//
// Created by stephane bourque on 2022-08-15.
//

#pragma once

#include <iostream>
#include <fstream>

#include "RESTObjects/RESTAPI_GWobjects.h"

#include "framework/MicroService.h"

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/Context.h"
#include "Poco/Crypto/X509Certificate.h"
#include "Poco/Net/NetException.h"
#include "Poco/TemporaryFile.h"

#include "RADIUS_helpers.h"
#include "DeviceRegistry.h"

namespace OpenWifi {

	class RADSECserver : public Poco::Runnable {
	  public:
		RADSECserver(Poco::Net::SocketReactor & R, GWObjects::RadiusProxyServerEntry E, Poco::Logger &L) :
 			Reactor_(R),
			Server_(std::move(E)),
	  		Logger_(L) {

			MakeSecurityFiles();
			ReconnectorThr_.start(*this);
		}

		inline void run() {
			while(TryAgain_) {
				if(!Connected_) {
					Connect();
				}
				Poco::Thread::trySleep(1000);
			}
		}

		inline bool SendData(const unsigned char *buffer, int length) {
			if(Connected_) {
				return (Socket_->sendBytes(buffer,length) == length);
			}
			return false;
		}

		inline void onData([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
			Poco::Buffer<char> IncomingRadiusPacket(0);
			try {
				auto NumberOfReceivedBytes = Socket_->receiveBytes(IncomingRadiusPacket);
				auto *RP = (const OpenWifi::RADIUS::RawRadiusPacket *)(IncomingRadiusPacket.begin());
				Logger_.information(fmt::format("RADSEC: received {} bytes.", NumberOfReceivedBytes));
				RADIUS::RadiusPacket	P(IncomingRadiusPacket);
				if(RADIUS::IsAuthentication(RP->code) ) {
					auto SerialNumber = P.ExtractSerialNumberFromProxyState();
					DeviceRegistry()->SendRadiusAuthenticationData(SerialNumber, (const unsigned char *)IncomingRadiusPacket.begin(), IncomingRadiusPacket.size());
				} else if(RADIUS::IsAccounting(RP->code)) {
					auto SerialNumber = P.ExtractSerialNumberFromProxyState();
					DeviceRegistry()->SendRadiusAccountingData(SerialNumber, (const unsigned char *)IncomingRadiusPacket.begin(), IncomingRadiusPacket.size());
				} else if(RADIUS::IsAuthority(RP->code)) {

				}

			} catch (const Poco::Exception &E) {
				Logger_.log(E);
			}
		}

		inline void onError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
			Disconnect();
		}

		inline void onShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
			Disconnect();
		}

		inline bool Connect() {
			if(TryAgain_) {

				Poco::Net::Context::Ptr SecureContext = Poco::AutoPtr<Poco::Net::Context>(
					new Poco::Net::Context(Poco::Net::Context::CLIENT_USE,
										   KeyFile_.path(),
										   CertFile_.path(),
										   CacertFile_.path()));
				auto tmp_Socket_ = std::make_unique<Poco::Net::SecureStreamSocket>(SecureContext);

				Poco::Net::SocketAddress Destination(Server_.ip, Server_.port);

				try {
					tmp_Socket_->connect(Destination, Poco::Timespan(10, 0));

					Reactor_.addEventHandler(
						*tmp_Socket_,
						Poco::NObserver<RADSECserver, Poco::Net::ReadableNotification>(
							*this, &RADSECserver::onData));
					Reactor_.addEventHandler(
						*tmp_Socket_, Poco::NObserver<RADSECserver, Poco::Net::ErrorNotification>(
										  *this, &RADSECserver::onError));
					Reactor_.addEventHandler(
						*tmp_Socket_,
						Poco::NObserver<RADSECserver, Poco::Net::ShutdownNotification>(
							*this, &RADSECserver::onShutdown));
					Socket_ = std::move(tmp_Socket_);
					Connected_ = true;
					return true;
				} catch (const Poco::Net::NetException &E) {
					Logger_.log(E);
				} catch (const Poco::Exception &E) {
					Logger_.log(E);
				}
			}
			return false;
		}

		inline void Disconnect() {
			Reactor_.removeEventHandler(*Socket_,Poco::NObserver<RADSECserver, Poco::Net::ReadableNotification>(
												   *this, &RADSECserver::onData));
			Reactor_.removeEventHandler(*Socket_,Poco::NObserver<RADSECserver, Poco::Net::ErrorNotification>(
												   *this, &RADSECserver::onError));
			Reactor_.removeEventHandler(*Socket_,Poco::NObserver<RADSECserver, Poco::Net::ShutdownNotification>(
												   *this, &RADSECserver::onShutdown));
			Socket_->shutdown();
		}

		inline void Stop() {
			TryAgain_ = false;
			Disconnect();
			ReconnectorThr_.wakeUp();
			ReconnectorThr_.join();
		}

		static void Output_Crypto_Content(std::ofstream &of, const std::string &d) {
			constexpr std::size_t line_length = 64;
			std::size_t pos=0;

			while(pos<d.length()) {
				of << d.substr(pos,line_length) << std::endl;
				pos += line_length;
			}
		}

		inline void MakeSecurityFiles() {
			static const char * cert_header = "-----BEGIN CERTIFICATE-----";
			static const char * cert_footer = "-----END CERTIFICATE-----";
			static const char * key_header = "-----BEGIN PRIVATE KEY-----";
			static const char * key_footer = "-----END PRIVATE KEY-----";

			//	make the key file
			std::ofstream cert_file(CertFile_.path(),std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
			cert_file << cert_header << std::endl;
			Output_Crypto_Content(cert_file,Server_.radsec_cert);
			cert_file << cert_footer << std::endl;
			cert_file.close();

			std::ofstream key_file(KeyFile_.path(),std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
			key_file << key_header << std::endl;
			Output_Crypto_Content(cert_file,Server_.radsec_key);
			key_file << key_footer << std::endl;
			key_file.close();

			std::ofstream cacert_file(CacertFile_.path(),std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
			auto cert_list = Poco::StringTokenizer(Server_.radsec_cacerts,",");
			for(auto &cert:cert_list) {
				cert_file << cert_header << std::endl;
				Output_Crypto_Content(cert_file,cert);
				cert_file << cert_footer << std::endl;
			}
			cacert_file.close();
		}

	  private:
		Poco::Net::SocketReactor							&Reactor_;
		GWObjects::RadiusProxyServerEntry					Server_;
		Poco::Logger										&Logger_;
		std::atomic_bool 									Connected_=false;
		std::atomic_bool 									TryAgain_=true;
		std::unique_ptr<Poco::Net::SecureStreamSocket>		Socket_;
		Poco::TemporaryFile									CertFile_{MicroService::instance().DataDir()} ,
															KeyFile_{MicroService::instance().DataDir()},
															CacertFile_{MicroService::instance().DataDir()};
		Poco::Thread										ReconnectorThr_;
	};
}