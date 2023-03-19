//
// Created by stephane bourque on 2022-08-15.
//

#pragma once

#include <fstream>
#include <iostream>

#include "RESTObjects/RESTAPI_GWobjects.h"

#include "Poco/Crypto/X509Certificate.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/TemporaryFile.h"

#include "framework/MicroServiceFuncs.h"

#include "fmt/format.h"

#include "AP_WS_Server.h"
#include "RADIUS_helpers.h"

namespace OpenWifi {

	class RADSEC_server : public Poco::Runnable {
	  public:
		RADSEC_server(Poco::Net::SocketReactor &R, GWObjects::RadiusProxyServerEntry E)
			: Reactor_(R), Server_(std::move(E)),
			  Logger_(Poco::Logger::get(
				  fmt::format("RADSEC: {}@{}:{}", Server_.name, Server_.ip, Server_.port))) {
			Start();
		}

		~RADSEC_server() { Stop(); }

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
			Poco::Thread::trySleep(3000);
			std::uint64_t LastStatus = 0;
			auto RadSecKeepAlive = MicroServiceConfigGetInt("radsec.keepalive", 120);
			while (TryAgain_) {
				if (!Connected_) {
					std::lock_guard G(LocalMutex_);
					LastStatus = Utils::Now();
					Connect();
				} else if ((Utils::Now() - LastStatus) > RadSecKeepAlive) {
					RADIUS::RadiusOutputPacket P(Server_.radsecSecret);
					P.MakeStatusMessage();
					poco_information(Logger_, "Keep-Alive message.");
					Socket_->sendBytes(P.Data(), P.Len());
					LastStatus = Utils::Now();
				}
				Poco::Thread::trySleep(!Connected_ ? 3000 : 10000);
			}
		}

		inline bool SendData(const std::string &serial_number, const unsigned char *buffer,
							 int length) {
			try {
				if (Connected_) {
					RADIUS::RadiusPacket P(buffer, length);
					int sent_bytes;
					if (P.VerifyMessageAuthenticator(Server_.radsecSecret)) {
						poco_debug(Logger_, fmt::format("{}: {} Sending {} bytes", serial_number,
														P.PacketType(), length));
						sent_bytes = Socket_->sendBytes(buffer, length);
					} else {
						poco_debug(Logger_, fmt::format("{}: {} Sending {} bytes", serial_number,
														P.PacketType(), length));
						P.ComputeMessageAuthenticator(Server_.radsecSecret);
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
							poco_debug(Logger_,
									   fmt::format("{}: {} Received {} bytes.", SerialNumber,
												   P.PacketType(), NumberOfReceivedBytes));
							AP_WS_Server()->SendRadiusAuthenticationData(SerialNumber, Buffer,
																		 NumberOfReceivedBytes);
						} else {
							poco_debug(Logger_, "AUTH packet dropped.");
						}
					} else if (P.IsAccounting()) {
						auto SerialNumber = P.ExtractSerialNumberFromProxyState();
						if (!SerialNumber.empty()) {
							poco_debug(Logger_,
									   fmt::format("{}: {} Received {} bytes.", SerialNumber,
												   P.PacketType(), NumberOfReceivedBytes));
							AP_WS_Server()->SendRadiusAccountingData(SerialNumber, Buffer,
																	 NumberOfReceivedBytes);
						} else {
							poco_debug(Logger_, "ACCT packet dropped.");
						}
					} else if (P.IsAuthority()) {
						auto SerialNumber = P.ExtractSerialNumberTIP();
						if (!SerialNumber.empty()) {
							poco_debug(Logger_,
									   fmt::format("{}: {} Received {} bytes.", SerialNumber,
												   P.PacketType(), NumberOfReceivedBytes));
							AP_WS_Server()->SendRadiusCoAData(SerialNumber, Buffer,
															  NumberOfReceivedBytes);
						} else {
							poco_debug(Logger_, "CoA/DM packet dropped.");
						}
					} else {
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

		inline bool Connect() {
			if (TryAgain_) {
				std::lock_guard G(LocalMutex_);

				Poco::TemporaryFile CertFile_(MicroServiceDataDirectory());
				Poco::TemporaryFile KeyFile_(MicroServiceDataDirectory());
				std::vector<std::unique_ptr<Poco::TemporaryFile>> CaCertFiles_;

				DecodeFile(CertFile_.path(), Server_.radsecCert);
				DecodeFile(KeyFile_.path(), Server_.radsecKey);

				for (auto &cert : Server_.radsecCacerts) {
					CaCertFiles_.emplace_back(
						std::make_unique<Poco::TemporaryFile>(MicroServiceDataDirectory()));
					DecodeFile(CaCertFiles_[CaCertFiles_.size() - 1]->path(), cert);
				}

				Poco::Net::Context::Ptr SecureContext =
					Poco::AutoPtr<Poco::Net::Context>(new Poco::Net::Context(
						Poco::Net::Context::TLS_CLIENT_USE, KeyFile_.path(), CertFile_.path(), ""));
				if (Server_.allowSelfSigned) {
					SecureContext->setSecurityLevel(Poco::Net::Context::SECURITY_LEVEL_NONE);
					SecureContext->enableExtendedCertificateVerification(false);
				}

				for (const auto &ca : CaCertFiles_) {
					Poco::Crypto::X509Certificate cert(ca->path());
					SecureContext->addCertificateAuthority(cert);
				}

				Socket_ = std::make_unique<Poco::Net::SecureStreamSocket>(SecureContext);

				Poco::Net::SocketAddress Destination(Server_.ip, Server_.port);

				try {
					poco_information(Logger_, "Attempting to connect");
					Socket_->connect(Destination, Poco::Timespan(100, 0));
					Socket_->completeHandshake();

					if (!Server_.allowSelfSigned) {
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
						*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ReadableNotification>(
									  *this, &RADSEC_server::onData));
					Reactor_.addEventHandler(
						*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ErrorNotification>(
									  *this, &RADSEC_server::onError));
					Reactor_.addEventHandler(
						*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ShutdownNotification>(
									  *this, &RADSEC_server::onShutdown));

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
			}
			return false;
		}

		inline void Disconnect() {
			if (Connected_) {
				std::lock_guard G(LocalMutex_);

				Reactor_.removeEventHandler(
					*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ReadableNotification>(
								  *this, &RADSEC_server::onData));
				Reactor_.removeEventHandler(
					*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ErrorNotification>(
								  *this, &RADSEC_server::onError));
				Reactor_.removeEventHandler(
					*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ShutdownNotification>(
								  *this, &RADSEC_server::onShutdown));
				Socket_->close();
				Connected_ = false;
			}
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

	  private:
		std::recursive_mutex LocalMutex_;
		Poco::Net::SocketReactor &Reactor_;
		GWObjects::RadiusProxyServerEntry Server_;
		Poco::Logger &Logger_;
		std::unique_ptr<Poco::Net::SecureStreamSocket> Socket_;
		Poco::Thread ReconnectThread_;
		std::unique_ptr<Poco::Crypto::X509Certificate> Peer_Cert_;
		volatile bool Connected_ = false;
		volatile bool TryAgain_ = true;
	};
} // namespace OpenWifi