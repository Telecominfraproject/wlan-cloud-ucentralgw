//
// Created by stephane bourque on 2022-08-15.
//

#pragma once

#include <fstream>
#include <iostream>

#include "RESTObjects/RESTAPI_GWobjects.h"

#include "Poco/Crypto/X509Certificate.h"
#include "Poco/Crypto/RSAKey.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/TemporaryFile.h"

#include "framework/MicroServiceFuncs.h"

#include "fmt/format.h"

#include "AP_WS_Server.h"
#include "RADIUS_helpers.h"
#include <RESTObjects/RESTAPI_GWobjects.h>

namespace OpenWifi {

	class RADIUS_Destination : public Poco::Runnable {
	  public:
		RADIUS_Destination(Poco::Net::SocketReactor &R, const GWObjects::RadiusProxyPool &P)
			: Reactor_(R),
			  Logger_(Poco::Logger::get(
				  fmt::format("RADSEC: {}", P.name))),
			  Pool_(P)
		{
			Type_ = GWObjects::RadiusEndpointType(P.radsecPoolType);
			Start();
		}

		~RADIUS_Destination() override { Stop(); }

		const int SMALLEST_RADIUS_PACKET = 20 + 19 + 4;
		const int DEFAULT_RADIUS_AUTHENTICATION_PORT = 1812;
		const int DEFAULT_RADIUS_ACCOUNTING_PORT = 1813;
		const int DEFAULT_RADIUS_CoA_PORT = 3799;
		
		inline int Start() {
			ReconnectThread_.start(*this);
			return 0;
		}

		inline void Stop() {
			TryAgain_ = false;
			Disconnect();
			ReconnectThread_.wakeUp();
			ReconnectThread_.join();
		}

		inline void run() final {
			Poco::Thread::trySleep(5000);
			std::uint64_t CurrentDelay = 10, maxDelay=300, LastTry=0, LastKeepAlive=0;
			while (TryAgain_) {
				if (!Connected_) {
					if(!LastTry || (Utils::Now()-LastTry)>CurrentDelay) {
						LastTry = Utils::Now();
						if (!Connect()) {
							CurrentDelay *= 2;
							if(CurrentDelay>maxDelay) CurrentDelay=10;
						} else {
							CurrentDelay = 10;
						}
					}
				} else if ((Utils::Now() - LastKeepAlive) > Pool_.radsecKeepAlive) {
					RADIUS::RadiusOutputPacket P(Pool_.acctConfig.servers[ServerIndex_].radsecSecret);
					P.MakeStatusMessage();
					poco_trace(Logger_, fmt::format("{}: Keep-Alive message.", Pool_.acctConfig.servers[ServerIndex_].name));
					Socket_->sendBytes(P.Data(), P.Len());
					LastKeepAlive = Utils::Now();
				}
				Poco::Thread::trySleep(2000);
			}
		}

		inline bool SendData(const std::string &serial_number, const unsigned char *buffer,
							 int length) {
			try {
				if (Connected_) {
					RADIUS::RadiusPacket P(buffer, length);
					int sent_bytes;
					if (P.VerifyMessageAuthenticator(Pool_.acctConfig.servers[ServerIndex_].radsecSecret)) {
						poco_trace(Logger_, fmt::format("{}: {} Sending {} bytes", serial_number,
														P.PacketType(), length));
						sent_bytes = Socket_->sendBytes(buffer, length);
					} else {
						poco_trace(Logger_, fmt::format("{}: {} Sending {} bytes", serial_number,
														P.PacketType(), length));
						P.ComputeMessageAuthenticator(Pool_.acctConfig.servers[ServerIndex_].radsecSecret);
						sent_bytes = Socket_->sendBytes(P.Buffer(), length);
					}
					return (sent_bytes == length);
				}
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
			} catch (...) {
				poco_warning(Logger_, "Exception occurred: while sending data.");
			}
			return false;
		}

		inline void
		onData([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
			unsigned char Buffer[4096];

			try {
				auto NumberOfReceivedBytes = Socket_->receiveBytes(Buffer, sizeof(Buffer));
				if (NumberOfReceivedBytes >= 20) {
					RADIUS::RadiusPacket P(Buffer, NumberOfReceivedBytes);
					if (P.IsAuthentication()) {
						auto SerialNumber = P.ExtractSerialNumberFromProxyState();
						if (!SerialNumber.empty()) {
							poco_trace(Logger_,
									   fmt::format("{}: {} Received {} bytes.", SerialNumber,
												   P.PacketType(), NumberOfReceivedBytes));
							AP_WS_Server()->SendRadiusAuthenticationData(SerialNumber, Buffer,
																		 NumberOfReceivedBytes);
						} else {
							poco_trace(Logger_, "AUTH packet dropped.");
						}
					} else if (P.IsAccounting()) {
						auto SerialNumber = P.ExtractSerialNumberFromProxyState();
						if (!SerialNumber.empty()) {
							poco_trace(Logger_,
									   fmt::format("{}: {} Received {} bytes.", SerialNumber,
												   P.PacketType(), NumberOfReceivedBytes));
							AP_WS_Server()->SendRadiusAccountingData(SerialNumber, Buffer,
																	 NumberOfReceivedBytes);
						} else {
							poco_trace(Logger_, "ACCT packet dropped.");
						}
					} else if (P.IsAuthority()) {
						auto SerialNumber = P.ExtractSerialNumberTIP();
						if (!SerialNumber.empty()) {
							poco_trace(Logger_,
									   fmt::format("{}: {} Received {} bytes.", SerialNumber,
												   P.PacketType(), NumberOfReceivedBytes));
							AP_WS_Server()->SendRadiusCoAData(SerialNumber, Buffer,
															  NumberOfReceivedBytes);
						} else {
							poco_trace(Logger_, "CoA/DM packet dropped.");
						}
					} else {
						DBGLINE
						poco_warning(Logger_,
									 fmt::format("Unknown packet: Type: {} (type={}) Length={}",
												 P.PacketType(), P.PacketTypeInt(), P.BufferLen()));
					}
				} else {
					poco_warning(Logger_, "Invalid packet received. Resetting the connection.");
					Disconnect();
				}
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
				Disconnect();
			} catch (...) {
				Disconnect();
				poco_warning(Logger_, "Exception occurred. Resetting the connection.");
			}
		}

		inline void
		onError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
			poco_warning(Logger_, "Socker error. Terminating connection.");
			Disconnect();
		}

		inline void
		onShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
			poco_warning(Logger_, "Socker socket shutdown. Terminating connection.");
			Disconnect();
		}

		inline void OnAccountingSocketReadable(
			const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
			Poco::Net::SocketAddress Sender;
			RADIUS::RadiusPacket P;

			DBGLINE
			auto ReceiveSize = pNf->socket().impl()->receiveBytes(P.Buffer(), P.BufferLen());
			if (ReceiveSize < SMALLEST_RADIUS_PACKET) {
				poco_warning(Logger_, "Accounting: bad packet received.");
				DBGLINE
				return;
			}
			P.Evaluate(ReceiveSize);
			auto SerialNumber = P.ExtractSerialNumberFromProxyState();
			DBGLINE
			if (SerialNumber.empty()) {
				poco_warning(Logger_, "Accounting: missing serial number. Dropping request.");
				DBGLINE
				return;
			}
			poco_debug(
				Logger_,
				fmt::format(
					"Accounting Packet Response received for {}", SerialNumber ));
			DBGLINE
			AP_WS_Server()->SendRadiusAccountingData(SerialNumber, P.Buffer(), P.Size());
			DBGLINE
		}

		inline void OnAuthenticationSocketReadable(
			const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
			Poco::Net::SocketAddress Sender;
			RADIUS::RadiusPacket P;

			auto ReceiveSize = pNf->socket().impl()->receiveBytes(P.Buffer(), P.BufferLen());
			if (ReceiveSize < SMALLEST_RADIUS_PACKET) {
				poco_warning(Logger_, "Authentication: bad packet received.");
				return;
			}
			P.Evaluate(ReceiveSize);

			if(Logger_.trace()) {
				P.Log(std::cout);
			}
			auto SerialNumber = P.ExtractSerialNumberFromProxyState();
			if (SerialNumber.empty()) {
				poco_warning(Logger_, "Authentication: missing serial number. Dropping request.");
				return;
			}
			auto CallingStationID = P.ExtractCallingStationID();
			auto CalledStationID = P.ExtractCalledStationID();

			poco_debug(
				Logger_,
				fmt::format(
					"Authentication Packet received for {}, CalledStationID: {}, CallingStationID:{}",
					SerialNumber, CalledStationID, CallingStationID));
			AP_WS_Server()->SendRadiusAuthenticationData(SerialNumber, P.Buffer(), P.Size());
		}

		inline void OnCoASocketReadable(
			const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
			Poco::Net::SocketAddress Sender;
			RADIUS::RadiusPacket P;

			auto ReceiveSize = pNf.get()->socket().impl()->receiveBytes(P.Buffer(), P.BufferLen());
			if (ReceiveSize < SMALLEST_RADIUS_PACKET) {
				poco_warning(Logger_, "CoA/DM: bad packet received.");
				return;
			}

			P.Evaluate(ReceiveSize);
			auto SerialNumber = P.ExtractSerialNumberTIP();
			if (SerialNumber.empty()) {
				poco_warning(Logger_, "CoA/DM: missing serial number. Dropping request.");
				return;
			}
			auto CallingStationID = P.ExtractCallingStationID();
			auto CalledStationID = P.ExtractCalledStationID();

			poco_debug(
				Logger_,
				fmt::format("CoA Packet received for {}, CalledStationID: {}, CallingStationID:{}",
							SerialNumber, CalledStationID, CallingStationID));
			AP_WS_Server()->SendRadiusCoAData(SerialNumber, P.Buffer(), P.Size());
		}
		
		static inline bool IsExpired(const Poco::Crypto::X509Certificate &C) {
			return C.expiresOn().timestamp().epochTime() < (std::time_t)Utils::Now();
		}

		static inline void Cat(const std::string &F1, const std::string & F2, const std::string &F) {
			std::ofstream of(F.c_str(),std::ios_base::trunc|std::ios_base::out|std::ios_base::binary);
			std::ifstream if1(F1.c_str(),std::ios_base::binary|std::ios_base::in);
			Poco::StreamCopier::copyStream(if1,of);
			of << std::endl;
			std::ifstream if2(F2.c_str(),std::ios_base::binary|std::ios_base::in);
			Poco::StreamCopier::copyStream(if2,of);
			of << std::endl;
			of.close();
		}

		inline bool Connect_GlobalReach() {

			if (TryAgain_) {
				std::lock_guard G(LocalMutex_);

				Poco::TemporaryFile CertFile_(MicroServiceDataDirectory());
				Poco::TemporaryFile KeyFile_(MicroServiceDataDirectory());
				Poco::TemporaryFile OpenRoamingRootCertFile_(MicroServiceDataDirectory());
				Poco::TemporaryFile Intermediate0(MicroServiceDataDirectory());
				Poco::TemporaryFile Intermediate1(MicroServiceDataDirectory());

				DecodeFile(KeyFile_.path(), Pool_.acctConfig.servers[ServerIndex_].radsecKey);
				DecodeFile(CertFile_.path(), Pool_.acctConfig.servers[ServerIndex_].radsecCert);
				DecodeFile(Intermediate0.path(), Pool_.acctConfig.servers[ServerIndex_].radsecCacerts[0]);
				DecodeFile(Intermediate1.path(), Pool_.acctConfig.servers[ServerIndex_].radsecCacerts[1]);

				const static std::string OpenRoamingRootCert{
					"-----BEGIN CERTIFICATE-----\n"
					"MIIClDCCAhugAwIBAgIUF1f+h+uJNHyr+ZqTpwew8LYRAW0wCgYIKoZIzj0EAwMw\n"
					"gYkxCzAJBgNVBAYTAkdCMQ8wDQYDVQQIEwZMb25kb24xDzANBgNVBAcTBkxvbmRv\n"
					"bjEsMCoGA1UEChMjR2xvYmFsUmVhY2ggVGVjaG5vbG9neSBFTUVBIExpbWl0ZWQx\n"
					"KjAoBgNVBAMTIUdsb2JhbFJlYWNoIENlcnRpZmljYXRlIEF1dGhvcml0eTAeFw0y\n"
					"MzA3MTQwOTMyMDBaFw00MzA3MDkwOTMyMDBaMIGJMQswCQYDVQQGEwJHQjEPMA0G\n"
					"A1UECBMGTG9uZG9uMQ8wDQYDVQQHEwZMb25kb24xLDAqBgNVBAoTI0dsb2JhbFJl\n"
					"YWNoIFRlY2hub2xvZ3kgRU1FQSBMaW1pdGVkMSowKAYDVQQDEyFHbG9iYWxSZWFj\n"
					"aCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAARy\n"
					"f02umFNy5W/TtM5nfMaLhRF61vLxhT8iNQHR1mXiRmNdME3ArForBcAm2eolHPcJ\n"
					"RH9DcXs59d2zzoPEaBjXADTCjUts3F7G6fjqvfki2e/txx/xfUopQO8G54XcFWqj\n"
					"QjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBRS\n"
					"tNe7MgAFwTaMZKUtS1/8pVoBqjAKBggqhkjOPQQDAwNnADBkAjA7VKHTybtSMBcN\n"
					"717jGYvkWlcj4c9/LzPtkHO053wGsPigaq+1SjY7tDhS/g9oUQACMA6UqH2e8cfn\n"
					"cZqmBNVNN3DBjIb4anug7F+FnYOQF36ua6MLBeGn3aKxvu1aO+hjPg==\n"
					"-----END CERTIFICATE-----\n"};

				std::ofstream ofs{OpenRoamingRootCertFile_.path().c_str(),
								  std::ios_base::trunc | std::ios_base::out |
									  std::ios_base::binary};
				ofs << OpenRoamingRootCert;
				ofs.close();

				Poco::Net::Context::Ptr SecureContext = Poco::AutoPtr<Poco::Net::Context>(
					new Poco::Net::Context(Poco::Net::Context::TLS_CLIENT_USE, ""));

				if (Pool_.acctConfig.servers[ServerIndex_].allowSelfSigned) {
					SecureContext->setSecurityLevel(Poco::Net::Context::SECURITY_LEVEL_NONE);
					SecureContext->enableExtendedCertificateVerification(false);
				}

				SecureContext->usePrivateKey(Poco::Crypto::RSAKey("", KeyFile_.path(), ""));
				Poco::Crypto::X509Certificate Cert(CertFile_.path());
				if (!IsExpired(Cert)) {
					SecureContext->useCertificate(Poco::Crypto::X509Certificate(CertFile_.path()));
				} else {
					poco_error(
						Logger_,
						fmt::format(
							"Certificate for {} has expired. We cannot connect to this server.",
							Pool_.acctConfig.servers[ServerIndex_].name));
					return false;
				}

				SecureContext->addCertificateAuthority(
					Poco::Crypto::X509Certificate(OpenRoamingRootCertFile_.path()));
				SecureContext->addChainCertificate(
					Poco::Crypto::X509Certificate(Intermediate0.path()));
				SecureContext->addChainCertificate(
					Poco::Crypto::X509Certificate(Intermediate1.path()));
				SecureContext->enableExtendedCertificateVerification(false);

				Socket_ = std::make_unique<Poco::Net::SecureStreamSocket>(SecureContext);
				ServerIndex_ = 0 ;
				for (const auto &PoolEntryServer : Pool_.acctConfig.servers) {
					Poco::Net::SocketAddress Destination(PoolEntryServer.ip, PoolEntryServer.port);
					try {
						poco_information(Logger_, fmt::format("Attempting to connect to {}", CommonName()));
						Socket_->connect(Destination, Poco::Timespan(20, 0));
						Socket_->completeHandshake();

						if (!Pool_.acctConfig.servers[ServerIndex_].allowSelfSigned) {
							Socket_->verifyPeerCertificate();
						}

						if (Socket_->havePeerCertificate()) {
							Peer_Cert_ = std::make_unique<Poco::Crypto::X509Certificate>(
								Socket_->peerCertificate());
						}

						Socket_->setBlocking(false);
						Socket_->setNoDelay(true);
						Socket_->setKeepAlive(true);
						Socket_->setReceiveTimeout(Poco::Timespan(1 * 60 * 60, 0));

						Reactor_.addEventHandler(
							*Socket_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
								*this, &RADIUS_Destination::onData));
						Reactor_.addEventHandler(
							*Socket_, Poco::NObserver<RADIUS_Destination, Poco::Net::ErrorNotification>(
										  *this, &RADIUS_Destination::onError));
						Reactor_.addEventHandler(
							*Socket_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ShutdownNotification>(
								*this, &RADIUS_Destination::onShutdown));

						Connected_ = true;
						poco_information(Logger_, fmt::format("Connected. CN={}", CommonName()));
						return true;
					} catch (const Poco::Net::NetException &E) {
						poco_warning(Logger_, "NetException: Could not connect.");
						Logger_.log(E);
					} catch (const Poco::Exception &E) {
						poco_warning(Logger_, "Exception: Could not connect.");
						Logger_.log(E);
					} catch (...) {
						poco_warning(Logger_, "Could not connect.");
					}
					ServerIndex_++;
				}
			}
			ServerIndex_=0;
			return false;
		}

		inline bool Connect_Orion() {
			if (TryAgain_) {
				std::lock_guard G(LocalMutex_);

				Poco::TemporaryFile CertFile_(MicroServiceDataDirectory());
				Poco::TemporaryFile KeyFile_(MicroServiceDataDirectory());
				std::vector<std::unique_ptr<Poco::TemporaryFile>> CaCertFiles_;

				DecodeFile(CertFile_.path(), Pool_.acctConfig.servers[ServerIndex_].radsecCert);
				DecodeFile(KeyFile_.path(), Pool_.acctConfig.servers[ServerIndex_].radsecKey);

				Poco::Crypto::X509Certificate	Cert(CertFile_.path());
				if(IsExpired(Cert)) {
					poco_error(Logger_, fmt::format("Certificate for {} has expired. We cannot connect to this server.", Pool_.acctConfig.servers[ServerIndex_].name));
					return false;
				}

				for (auto &cert : Pool_.acctConfig.servers[ServerIndex_].radsecCacerts) {
					CaCertFiles_.emplace_back(
						std::make_unique<Poco::TemporaryFile>(MicroServiceDataDirectory()));
					DecodeFile(CaCertFiles_[CaCertFiles_.size() - 1]->path(), cert);
				}

				Poco::Net::Context::Ptr SecureContext =
					Poco::AutoPtr<Poco::Net::Context>(new Poco::Net::Context(
						Poco::Net::Context::TLS_CLIENT_USE, KeyFile_.path(), CertFile_.path(), ""));
				if (Pool_.acctConfig.servers[ServerIndex_].allowSelfSigned) {
					SecureContext->setSecurityLevel(Poco::Net::Context::SECURITY_LEVEL_NONE);
					SecureContext->enableExtendedCertificateVerification(false);
				}

				for (const auto &ca : CaCertFiles_) {
					Poco::Crypto::X509Certificate cert(ca->path());
					SecureContext->addCertificateAuthority(cert);
				}

				Socket_ = std::make_unique<Poco::Net::SecureStreamSocket>(SecureContext);
				ServerIndex_ = 0 ;
				for (const auto &PoolEntryServer : Pool_.acctConfig.servers) {
					Poco::Net::SocketAddress Destination(PoolEntryServer.ip, PoolEntryServer.port);
					try {
						poco_information(Logger_, "Attempting to connect");
						Socket_->connect(Destination, Poco::Timespan(100, 0));
						Socket_->completeHandshake();

						if (!Pool_.acctConfig.servers[ServerIndex_].allowSelfSigned) {
							Socket_->verifyPeerCertificate();
						}

						if (Socket_->havePeerCertificate()) {
							Peer_Cert_ = std::make_unique<Poco::Crypto::X509Certificate>(
								Socket_->peerCertificate());
						}

						Socket_->setBlocking(false);
						Socket_->setNoDelay(true);
						Socket_->setKeepAlive(true);
						Socket_->setReceiveTimeout(Poco::Timespan(1 * 60 * 60, 0));

						Reactor_.addEventHandler(
							*Socket_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
								*this, &RADIUS_Destination::onData));
						Reactor_.addEventHandler(
							*Socket_, Poco::NObserver<RADIUS_Destination, Poco::Net::ErrorNotification>(
										  *this, &RADIUS_Destination::onError));
						Reactor_.addEventHandler(
							*Socket_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ShutdownNotification>(
								*this, &RADIUS_Destination::onShutdown));

						Connected_ = true;
						poco_information(Logger_, fmt::format("Connected. CN={}", CommonName()));
						return true;
					} catch (const Poco::Net::NetException &E) {
						poco_information(Logger_, "Could not connect.");
						Logger_.log(E);
					} catch (const Poco::Exception &E) {
						poco_information(Logger_, "Could not connect.");
						Logger_.log(E);
					} catch (...) {
						poco_information(Logger_, "Could not connect.");
					}
					ServerIndex_++;
				}
			}
			ServerIndex_=0;
			return false;
		}

		inline bool Connect_Generic() {
			if (TryAgain_) {
				std::lock_guard G(LocalMutex_);

				Poco::Net::SocketAddress AuthSockAddrV4(
					Poco::Net::AddressFamily::IPv4,
					MicroServiceConfigGetInt("radius.proxy.authentication.port",
											 DEFAULT_RADIUS_AUTHENTICATION_PORT));
				AuthenticationSocketV4_ =
					std::make_unique<Poco::Net::DatagramSocket>(AuthSockAddrV4, true, true);

				Poco::Net::SocketAddress AcctSockAddrV4(
					Poco::Net::AddressFamily::IPv4,
					MicroServiceConfigGetInt("radius.proxy.accounting.port",
											 DEFAULT_RADIUS_ACCOUNTING_PORT));
				AccountingSocketV4_ =
					std::make_unique<Poco::Net::DatagramSocket>(AcctSockAddrV4, true, true);

				Poco::Net::SocketAddress CoASockAddrV4(
					Poco::Net::AddressFamily::IPv4,
					MicroServiceConfigGetInt("radius.proxy.coa.port", DEFAULT_RADIUS_CoA_PORT));
				CoASocketV4_ = std::make_unique<Poco::Net::DatagramSocket>(CoASockAddrV4, true, true);

/*
				AuthenticationSocketV6_ =
					std::make_unique<Poco::Net::DatagramSocket>(AuthSockAddrV6, true, true);
 				Poco::Net::SocketAddress AuthSockAddrV6(
					Poco::Net::AddressFamily::IPv6,
					MicroServiceConfigGetInt("radius.proxy.authentication.port",
											 DEFAULT_RADIUS_AUTHENTICATION_PORT));

				Poco::Net::SocketAddress AcctSockAddrV6(
					Poco::Net::AddressFamily::IPv6,
					MicroServiceConfigGetInt("radius.proxy.accounting.port",
											 DEFAULT_RADIUS_ACCOUNTING_PORT));
				AccountingSocketV6_ =
					std::make_unique<Poco::Net::DatagramSocket>(AcctSockAddrV6, true, true);

				Poco::Net::SocketAddress CoASockAddrV6(
					Poco::Net::AddressFamily::IPv6,
					MicroServiceConfigGetInt("radius.proxy.coa.port", DEFAULT_RADIUS_CoA_PORT));
				CoASocketV6_ = std::make_unique<Poco::Net::DatagramSocket>(CoASockAddrV6, true, true);
*/
				Reactor_.addEventHandler(
					*AuthenticationSocketV4_,
					Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
						*this, &RADIUS_Destination::OnAuthenticationSocketReadable));
				Reactor_.addEventHandler(
					*AccountingSocketV4_,
					Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
						*this, &RADIUS_Destination::OnAccountingSocketReadable));
				Reactor_.addEventHandler(
					*CoASocketV4_, Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
									   *this, &RADIUS_Destination::OnCoASocketReadable));
/*
				Reactor_.addEventHandler(
					*AuthenticationSocketV6_,
					Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
						*this, &RADIUS_Destination::OnAuthenticationSocketReadable));
				Reactor_.addEventHandler(
					*AccountingSocketV6_,
					Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
						*this, &RADIUS_Destination::OnAccountingSocketReadable));

				Reactor_.addEventHandler(
					*CoASocketV6_, Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
									   *this, &RADIUS_Destination::OnCoASocketReadable));
*/
			}
			return true;
		}

		inline bool Connect_Radsec() {
			if (TryAgain_) {
				std::lock_guard G(LocalMutex_);
			}
			return true;
		}

		inline bool Connect() {
			switch(Type_) {
			case GWObjects::RadiusEndpointType::orion: return Connect_Orion();
			case GWObjects::RadiusEndpointType::globalreach: return Connect_GlobalReach();
			case GWObjects::RadiusEndpointType::radsec: return Connect_Radsec();
			default:
				return Connect_Generic();
			}
		}

		inline void Disconnect() {
			if (Connected_) {
				std::lock_guard G(LocalMutex_);
				if(Type_==GWObjects::RadiusEndpointType::generic) {
					if(AuthenticationSocketV4_) {
						Reactor_.removeEventHandler(
							*AuthenticationSocketV4_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
								*this, &RADIUS_Destination::OnAuthenticationSocketReadable));
						AuthenticationSocketV4_->close();
						AuthenticationSocketV4_.reset();
					}

					if(AccountingSocketV4_) {
						Reactor_.removeEventHandler(
							*AccountingSocketV4_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
								*this, &RADIUS_Destination::OnAccountingSocketReadable));
						AccountingSocketV4_->close();
						AccountingSocketV4_.reset();
					}

					if(CoASocketV4_) {
						Reactor_.removeEventHandler(
							*CoASocketV4_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
								*this, &RADIUS_Destination::OnCoASocketReadable));
						CoASocketV4_->close();
						CoASocketV4_.reset();
					}

/*					if(AuthenticationSocketV6_) {
						Reactor_.removeEventHandler(
							*AuthenticationSocketV6_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
								*this, &RADIUS_Destination::OnAuthenticationSocketReadable));
						AuthenticationSocketV6_->close();
						AuthenticationSocketV6_.reset();
					}

					if(AccountingSocketV6_) {
						Reactor_.removeEventHandler(
							*AccountingSocketV6_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
								*this, &RADIUS_Destination::OnAccountingSocketReadable));
						AccountingSocketV6_->close();
						AccountingSocketV6_.reset();
					}

					if(CoASocketV6_) {
						Reactor_.removeEventHandler(
							*CoASocketV6_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
								*this, &RADIUS_Destination::OnCoASocketReadable));
						CoASocketV6_->close();
						CoASocketV6_.reset();
					}
*/
				} else {
					if(Socket_!=nullptr) {
						std::lock_guard G(LocalMutex_);
						Reactor_.removeEventHandler(
							*Socket_, Poco::NObserver<RADIUS_Destination, Poco::Net::ReadableNotification>(
										  *this, &RADIUS_Destination::onData));
						Reactor_.removeEventHandler(
							*Socket_, Poco::NObserver<RADIUS_Destination, Poco::Net::ErrorNotification>(
										  *this, &RADIUS_Destination::onError));
						Reactor_.removeEventHandler(
							*Socket_,
							Poco::NObserver<RADIUS_Destination, Poco::Net::ShutdownNotification>(
								*this, &RADIUS_Destination::onShutdown));
						Socket_->close();
					}
				}
			}
			Connected_ = false;
			poco_information(Logger_, "Disconnecting.");
		}

		static void DecodeFile(const std::string &filename, const std::string &s) {
			std::ofstream sec_file(filename, std::ios_base::out | std::ios_base::trunc |
												 std::ios_base::binary);
			std::stringstream is(s);
			Poco::Base64Decoder ds(is);
			Poco::StreamCopier::copyStream(ds, sec_file);
			sec_file.close();
		}

		[[nodiscard]] inline std::string CommonName() {
			if (Peer_Cert_)
				return Peer_Cert_->commonName();
			return "";
		}

		[[nodiscard]] inline std::string IssuerName() {
			if (Peer_Cert_)
				return Peer_Cert_->issuerName();
			return "";
		}

		[[nodiscard]] inline std::string SubjectName() {
			if (Peer_Cert_)
				return Peer_Cert_->subjectName();
			return "";
		}

		const auto &Pool() const { return Pool_; }
		auto ServerType() const { return Type_; }

		inline bool SendRadiusDataAuthData(const std::string &serialNumber, const unsigned char *buffer, std::size_t  size) {
			poco_trace(Logger_, fmt::format("{}: Sending RADIUS Auth {} bytes.", serialNumber, size));
			std::cout << "Sending RADIUS Auth " << size << " bytes to " << Pool_.authConfig.servers[0].ip <<
				":" << Pool_.authConfig.servers[0].port << std::endl;
			AuthenticationSocketV4_->sendTo(buffer, size, Poco::Net::SocketAddress(Pool_.authConfig.servers[0].ip, Pool_.authConfig.servers[0].port));
			return true;
		}

		inline bool SendRadiusDataAcctData(const std::string &serialNumber, const unsigned char *buffer, std::size_t  size) {
			poco_trace(Logger_, fmt::format("{}: Sending RADIUS Acct {} bytes.", serialNumber, size));
			AccountingSocketV4_->sendTo(buffer, size, Poco::Net::SocketAddress(Pool_.acctConfig.servers[0].ip, Pool_.acctConfig.servers[0].port));
			return true;
		}

		inline bool SendRadiusDataCoAData(const std::string &serialNumber, const unsigned char *buffer, std::size_t  size) {
			poco_trace(Logger_, fmt::format("{}: Sending RADIUS CoA {} bytes.", serialNumber, size));
			CoASocketV4_->sendTo(buffer, size, Poco::Net::SocketAddress(Pool_.coaConfig.servers[0].ip, Pool_.coaConfig.servers[0].port));
			return true;
		}

	  private:
		std::recursive_mutex 							LocalMutex_;
		Poco::Net::SocketReactor 						&Reactor_;
		Poco::Logger 									&Logger_;

		std::unique_ptr<Poco::Net::SecureStreamSocket> 	Socket_;

		std::unique_ptr<Poco::Net::DatagramSocket> 		AccountingSocketV4_;
		std::unique_ptr<Poco::Net::DatagramSocket> 		AuthenticationSocketV4_;
		std::unique_ptr<Poco::Net::DatagramSocket> 		CoASocketV4_;

/*		std::unique_ptr<Poco::Net::DatagramSocket> 		CoASocketV6_;
		std::unique_ptr<Poco::Net::DatagramSocket> 		AccountingSocketV6_;
		std::unique_ptr<Poco::Net::DatagramSocket> 		AuthenticationSocketV6_;
*/

		Poco::Thread 									ReconnectThread_;
		std::unique_ptr<Poco::Crypto::X509Certificate> 	Peer_Cert_;
		volatile bool 									Connected_ = false;
		volatile bool 									TryAgain_ = true;
		enum GWObjects::RadiusEndpointType				Type_{GWObjects::RadiusEndpointType::unknown};
		GWObjects::RadiusProxyPool						Pool_;
		uint64_t 										ServerIndex_=0;
	};
} // namespace OpenWifi