//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "Poco/JSON/Object.h"

#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/MicroService.h"

// class uCentral::WebSocket::WSConnection;

namespace OpenWifi {

	class WSConnection;
    class DeviceRegistry : public SubSystemServer {
    public:
		struct ConnectionEntry {
			WSConnection 				*WSConn_;
			GWObjects::ConnectionState 	Conn_;
			std::string        			LastStats;
			GWObjects::HealthCheck		LastHealthcheck;
			uint64_t 					ConnectionId=0;
		};

        static auto instance() {
            static auto instance_ = new DeviceRegistry;
            return instance_;
        }

		int Start() override;
		void Stop() override;
		bool GetStatistics(const std::string &SerialNumber, std::string & Statistics);
		void SetStatistics(const std::string &SerialNumber, const std::string &stats);
		bool GetState(const std::string & SerialNumber, GWObjects::ConnectionState & State);
		void SetState(const std::string & SerialNumber, GWObjects::ConnectionState & State);
		bool GetHealthcheck(const std::string &SerialNumber, GWObjects::HealthCheck & CheckData);
		void SetHealthcheck(const std::string &SerialNumber, const GWObjects::HealthCheck &H);
		std::shared_ptr<ConnectionEntry> Register(const std::string & SerialNumber, WSConnection *, uint64_t & ConnectionId);
		void UnRegister(const std::string & SerialNumber, uint64_t ConnectionId);
		bool SendCommand(GWObjects::CommandDetails & Command);
		bool Connected(const std::string & SerialNumber);
		bool SendFrame(const std::string & SerialNumber, const std::string & Payload);
		void SetPendingUUID(const std::string & SerialNumber, uint64_t PendingUUID);
		bool AnalyzeRegistry(GWObjects::Dashboard &D);

	  private:
		inline static std::atomic_uint64_t 						Id_=1;
		std::map<std::string,std::shared_ptr<ConnectionEntry>>  Devices_;

		DeviceRegistry() noexcept:
    		SubSystemServer("DeviceRegistry", "DevStatus", "devicestatus") {
		}
	};

	inline auto DeviceRegistry() { return DeviceRegistry::instance(); }

}  // namespace

