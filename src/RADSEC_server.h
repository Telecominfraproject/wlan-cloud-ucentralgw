//
// Created by stephane bourque on 2022-08-15.
//

#pragma once

#include <iostream>
#include <fstream>

#include "framework/MicroService.h"

#include "RESTObjects/RESTAPI_GWobjects.h"

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/Context.h"
#include "Poco/Crypto/X509Certificate.h"
#include "Poco/Net/NetException.h"
#include "Poco/TemporaryFile.h"

#include "RADIUS_helpers.h"
#include "AP_WS_Server.h"

namespace OpenWifi {

	class RADSEC_server : public Poco::Runnable {
	  public:
		RADSEC_server(Poco::Net::SocketReactor & R, GWObjects::RadiusProxyServerEntry E) :
 			Reactor_(R),
			Server_(std::move(E)),
			Logger_(Poco::Logger::get(fmt::format("RADSEC: {}@{}:{}",
								Server_.name ,
								Server_.ip,
								Server_.port)))
		{
			ReconnectThread_.start(*this);
		}

		~RADSEC_server() {
			if(ReconnectThread_.isRunning()) {
				Stop();
			}
		}

		inline void Stop() {
			TryAgain_ = false;
			Disconnect();
			ReconnectThread_.wakeUp();
			ReconnectThread_.join();
		}

		inline void run() final {
			while(TryAgain_) {
				if(!Connected_) {
					std::unique_lock	G(Mutex_);
					Connect();
				}
				Poco::Thread::trySleep(3000);
			}
		}

		inline bool SendData(const std::string &serial_number, const unsigned char *buffer, int length) {
			try {
				if (Connected_) {
					RADIUS::RadiusPacket P(buffer, length);
					// std::cout << serial_number << "    Sending " << P.PacketType() << "  "  << length << " bytes" << std::endl;
					int sent_bytes;
					if (P.VerifyMessageAuthenticator(Server_.radsecSecret)) {
						poco_debug(Logger_,fmt::format("{}: {} Sending {} bytes", serial_number,
												  P.PacketType(), length));
						sent_bytes = Socket_->sendBytes(buffer, length);
					} else {
						poco_debug(Logger_,fmt::format("{}: {} Sending {} bytes", serial_number,
												  P.PacketType(), length));
						P.ComputeMessageAuthenticator(Server_.radsecSecret);
						sent_bytes = Socket_->sendBytes(P.Buffer(), length);
					}
					return (sent_bytes == length);
				}
			} catch (...) {

			}
			return false;
		}

		inline void onData([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
			unsigned char Buffer[4096];

			try {
				auto NumberOfReceivedBytes = Socket_->receiveBytes(Buffer,sizeof(Buffer));
				if(NumberOfReceivedBytes>40) {
					RADIUS::RadiusPacket P(Buffer,NumberOfReceivedBytes);
					if (P.IsAuthentication()) {
						auto SerialNumber = P.ExtractSerialNumberFromProxyState();
						if(!SerialNumber.empty()) {
							poco_debug(Logger_,
									   fmt::format("{}: {} Received {} bytes.", SerialNumber,
												   P.PacketType(), NumberOfReceivedBytes));
							AP_WS_Server()->SendRadiusAuthenticationData(SerialNumber, Buffer,
																		 NumberOfReceivedBytes);
						} else {
							poco_debug(Logger_,
									   fmt::format("Invalid AUTH packet received in proxy dropped. No serial number Source={}",
									Socket_->address().toString()));
						}
					} else if (P.IsAccounting()) {
						auto SerialNumber = P.ExtractSerialNumberFromProxyState();
						if(!SerialNumber.empty()) {
							poco_debug(Logger_,
									   fmt::format("{}: {} Received {} bytes.", SerialNumber,
												   P.PacketType(), NumberOfReceivedBytes));
							AP_WS_Server()->SendRadiusAccountingData(SerialNumber, Buffer,
																	 NumberOfReceivedBytes);
						} else {
							poco_debug(Logger_,
									   fmt::format("Invalid ACCT packet received in proxy dropped. No serial number Source={}",
												   Socket_->address().toString()));
						}
					} else if (P.IsAuthority()) {
					}
				} else {
					Disconnect();
				}
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
				Disconnect();
			}
		}

		inline void onError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
			std::cout << "onError" << std::endl;
			Disconnect();
		}

		inline void onShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
			std::cout << "onShutdown" << std::endl;
			Disconnect();
		}

		inline bool Connect() {
			if(TryAgain_) {

				Poco::TemporaryFile	CertFile_(MicroService::instance().DataDir());
				Poco::TemporaryFile	KeyFile_(MicroService::instance().DataDir());
				std::vector<Poco::TemporaryFile> CaCertFiles_;

				DecodeFile(CertFile_.path(), Server_.radsecCert);
				DecodeFile(KeyFile_.path(), Server_.radsecKey);

				for(auto &cert:Server_.radsecCacerts) {
					CaCertFiles_.emplace_back(Poco::TemporaryFile(MicroService::instance().DataDir()));
					DecodeFile(CaCertFiles_[CaCertFiles_.size()-1].path(), cert);
				}

				Poco::Net::Context::Ptr SecureContext = Poco::AutoPtr<Poco::Net::Context>(
					new Poco::Net::Context(Poco::Net::Context::TLS_CLIENT_USE,
										   KeyFile_.path(),
										   CertFile_.path(),""));

				for(const auto &ca:CaCertFiles_) {
					Poco::Crypto::X509Certificate	cert(ca.path());
					SecureContext->addCertificateAuthority(cert);
				}

				Socket_ = std::make_unique<Poco::Net::SecureStreamSocket>(SecureContext);

				Poco::Net::SocketAddress Destination(Server_.ip, Server_.port);

				try {
					poco_information(Logger_, "Attempting to connect");
					Socket_->connect(Destination, Poco::Timespan(100, 0));
					Socket_->completeHandshake();
					Socket_->verifyPeerCertificate();

					if(Socket_->havePeerCertificate()) {
						Peer_Cert_ = std::make_unique<Poco::Crypto::X509Certificate>(Socket_->peerCertificate());
					}

					Socket_->setBlocking(false);
					Socket_->setNoDelay(true);
					Socket_->setKeepAlive(true);
					Socket_->setReceiveTimeout(Poco::Timespan(1 * 60 * 60,0));

					Reactor_.addEventHandler(
						*Socket_,
						Poco::NObserver<RADSEC_server, Poco::Net::ReadableNotification>(
							*this, &RADSEC_server::onData));
					Reactor_.addEventHandler(
						*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ErrorNotification>(
										  *this, &RADSEC_server::onError));
					Reactor_.addEventHandler(
						*Socket_,
						Poco::NObserver<RADSEC_server, Poco::Net::ShutdownNotification>(
							*this, &RADSEC_server::onShutdown));
					Socket_->setBlocking(false);
					Socket_->setNoDelay(true);
					Socket_->setKeepAlive(true);

					Connected_ = true;
					poco_information(Logger_,fmt::format("Connected. CN={}",CommonName()));
					return true;
				} catch (const Poco::Net::NetException &E) {
					poco_information(Logger_,"Could not connect.");
					Logger_.log(E);
				} catch (const Poco::Exception &E) {
					poco_information(Logger_,"Could not connect.");
					Logger_.log(E);
				} catch (...) {
					poco_information(Logger_,"Could not connect.");
				}
			}
			return false;
		}

		inline void Disconnect() {
			if(Connected_) {
				std::unique_lock G(Mutex_);

				Reactor_.removeEventHandler(
					*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ReadableNotification>(
								  *this, &RADSEC_server::onData));
				Reactor_.removeEventHandler(
					*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ErrorNotification>(
								  *this, &RADSEC_server::onError));
				Reactor_.removeEventHandler(
					*Socket_, Poco::NObserver<RADSEC_server, Poco::Net::ShutdownNotification>(
								  *this, &RADSEC_server::onShutdown));
				Connected_ = false;
			}
			poco_information(Logger_,"Disconnecting.");
		}

		static void DecodeFile(const std::string &filename, const std::string &s) {
			std::ofstream sec_file(filename,std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
			std::stringstream is(s);
			Poco::Base64Decoder	ds(is);
			Poco::StreamCopier::copyStream(ds,sec_file);
			sec_file.close();
		}

		[[nodiscard]] inline std::string CommonName() {
			if(Peer_Cert_)
				return Peer_Cert_->commonName();
			return "";
		}

		[[nodiscard]] inline std::string IssuerName() {
			if(Peer_Cert_)
				return Peer_Cert_->issuerName();
			return "";
		}

		[[nodiscard]] inline std::string SubjectName() {
			if(Peer_Cert_)
				return Peer_Cert_->subjectName();
			return "";
		}

	  private:
		std::recursive_mutex								Mutex_;
		Poco::Net::SocketReactor							&Reactor_;
		GWObjects::RadiusProxyServerEntry					Server_;
		Poco::Logger										&Logger_;
		std::unique_ptr<Poco::Net::SecureStreamSocket>		Socket_;
		Poco::Thread										ReconnectThread_;
		std::unique_ptr<Poco::Crypto::X509Certificate>		Peer_Cert_;
		volatile bool 										Connected_=false;
		volatile bool 	 									TryAgain_=true;
	};
}