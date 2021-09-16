//
// Created by stephane bourque on 2021-09-07.
//

#ifndef OWGW_TELEMETRYSTREAM_H
#define OWGW_TELEMETRYSTREAM_H

#include <iostream>

#include "SubSystemServer.h"

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/ParallelSocketAcceptor.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Timespan.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPServer.h"

namespace OpenWifi {

	class TelemetryClient {
		static constexpr int BufSize = 64000;
	  public:
		TelemetryClient(
			std::string UUID, std::string SerialNumber,
			std::shared_ptr<Poco::Net::WebSocket> WSock,
			Poco::Net::SocketReactor& Reactor, Poco::Logger &Logger);
		~TelemetryClient();

		void OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
		void OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);
		bool Send(const std::string &Payload);
		void ProcessIncomingFrame();
	  private:
		std::recursive_mutex        			Mutex_;
		std::string 							UUID_;
		std::string 							SerialNumber_;
		Poco::Net::SocketReactor				&Reactor_;
		Poco::Logger               				&Logger_;
		Poco::Net::StreamSocket     			Socket_;
		std::string 							CId_;
		std::shared_ptr<Poco::Net::WebSocket>	WS_;
		bool 									Registered_=false;
		void SendTelemetryShutdown();
		void CompleteStartup();
	};

	class TelemetryRequestHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		explicit TelemetryRequestHandler( const std::string & UUID, const std::string & SerialNumber, Poco::Net::SocketReactor &Reactor, Poco::Logger &Logger) :
 			UUID_(UUID), SerialNumber_(SerialNumber), Reactor_(Reactor), Logger_(Logger) {}

		void handleRequest(Poco::Net::HTTPServerRequest & Request, Poco::Net::HTTPServerResponse & Response) final {
			std::cout << __LINE__ << std::endl;
			try {
				auto WS = std::make_shared<Poco::Net::WebSocket>(Request, Response);
				new TelemetryClient(UUID_, SerialNumber_, WS, Reactor_, Logger_);
			} catch (const Poco::Exception &E) {
				std::cout << E.what() << " " << E.name() << " " << E.displayText() << std::endl;
			}
			std::cout << __LINE__ << std::endl;
		}
	  private:
		const std::string 			&UUID_;
		const std::string 			&SerialNumber_;
		Poco::Net::SocketReactor	&Reactor_;
		Poco::Logger				&Logger_;
	};

	class TelemetryRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	  public:
		explicit TelemetryRequestHandlerFactory(Poco::Net::SocketReactor & Reactor, Poco::Logger & Logger) :
			Reactor_(Reactor), Logger_(Logger)
		{}

		inline Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest & Request) final {
			std::cout << __LINE__ << std::endl;
			Poco::URI U(Request.getURI());
			std::string UUID_ = U.getPath().substr(1);
			std::string SerialNumber_;
			auto Parameters = U.getQueryParameters();
			for (const auto &i : Parameters) {
				if (i.first == "serialNumber") {
					SerialNumber_ = i.second;
					break;
				}
			}
			std::cout << "Serial:" << SerialNumber_ << " UUID:" << UUID_ << std::endl;
			return new TelemetryRequestHandler( UUID_, SerialNumber_, Reactor_, Logger_);
		}

	  private:
		Poco::Net::SocketReactor	& Reactor_;
		Poco::Logger 				& Logger_;
	};

	class TelemetryStream : public SubSystemServer {
	  public:
		static TelemetryStream *instance() {
			if (instance_ == nullptr) {
				instance_ = new TelemetryStream;
			}
			return instance_;
		}

		int Start() override;
		void Stop() override;

		bool CreateEndpoint(const std::string &SerialNumber, std::string &EndPoint, std::string &UUID);
		void DeleteEndPoint(const std::string &SerialNumber);
		void UpdateEndPoint(const std::string &SerialNumber, const std::string &PayLoad);
		bool RegisterClient(const std::string &UUID, TelemetryClient *Client);
		void DeRegisterClient(const std::string &UUID);

	  private:
		static TelemetryStream 					* 	instance_;
		Poco::Thread								ReactorThread_;
		Poco::Net::SocketReactor					Reactor_;
		std::map<std::string, TelemetryClient *>	Clients_;			// 	uuid -> client
		std::map<std::string, std::string>			SerialNumbers_;		//	serialNumber -> uuid
		Poco::ThreadPool							Pool_;
		std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   TelemetryServers_;

		TelemetryStream() noexcept:
			SubSystemServer("TelemetryServer", "TELEMETRY-SVR", "openwifi.telemetry")
		{
		}
	};

	inline TelemetryStream * TelemetryStream() { return TelemetryStream::instance(); }

	}
#endif // OWGW_TELEMETRYSTREAM_H
