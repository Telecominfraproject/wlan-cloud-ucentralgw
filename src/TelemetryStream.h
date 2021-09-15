//
// Created by stephane bourque on 2021-09-07.
//

#ifndef OWGW_TELEMETRYSTREAM_H
#define OWGW_TELEMETRYSTREAM_H

#include "SubSystemServer.h"

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/ParallelSocketAcceptor.h"
#include "Poco/Net/WebSocket.h"

namespace OpenWifi {

	class TelemetryClient {
		static constexpr int BufSize = 64000;
	  public:
		TelemetryClient(Poco::Net::StreamSocket& Socket, Poco::Net::SocketReactor& Reactor);
		~TelemetryClient();

		void OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
		void OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);
		bool Send(const std::string &Payload);
		void ProcessIncomingFrame();
	  private:
		std::recursive_mutex        			Mutex_;
		Poco::Logger               				&Logger_;
		Poco::Net::StreamSocket     			Socket_;
		Poco::Net::SocketReactor				&Reactor_;
		std::string 							CId_;
		std::unique_ptr<Poco::Net::WebSocket>	WS_;
		std::string 							UUID_;
		std::string 							SerialNumber_;
		bool 									Registered_=false;
		void SendTelemetryShutdown();
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
		static TelemetryStream 	* 	instance_;
		Poco::Thread				ReactorThread_;
		Poco::Net::SocketReactor	Reactor_;
		std::map<std::string, TelemetryClient *>	Clients_;			// 	uuid -> client
		std::map<std::string, std::string>			SerialNumbers_;		//	serialNumber -> uuid
		std::vector<std::unique_ptr<Poco::Net::ParallelSocketAcceptor<TelemetryClient, Poco::Net::SocketReactor>>>	Acceptors_;

		TelemetryStream() noexcept:
			SubSystemServer("TelemetryServer", "TELEMETRY-SVR", "openwifi.telemetry")
		{
		}
	};

	inline TelemetryStream * TelemetryStream() { return TelemetryStream::instance(); }

	}
#endif // OWGW_TELEMETRYSTREAM_H
