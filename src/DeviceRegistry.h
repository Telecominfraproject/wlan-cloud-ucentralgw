//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UDEVICEREGISTRY_H
#define UCENTRAL_UDEVICEREGISTRY_H

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
		};

        static DeviceRegistry *instance() {
			static DeviceRegistry instance;
            return &instance;
        }

		int Start() override;
		void Stop() override;
		bool GetStatistics(const std::string &SerialNumber, std::string & Statistics);
		void SetStatistics(const std::string &SerialNumber, const std::string &stats);
		bool GetState(const std::string & SerialNumber, GWObjects::ConnectionState & State);
		void SetState(const std::string & SerialNumber, GWObjects::ConnectionState & State);
		bool GetHealthcheck(const std::string &SerialNumber, GWObjects::HealthCheck & CheckData);
		void SetHealthcheck(const std::string &SerialNumber, const GWObjects::HealthCheck &H);
		std::shared_ptr<ConnectionEntry> Register(const std::string & SerialNumber, WSConnection *);
		void UnRegister(const std::string & SerialNumber, WSConnection *);
		bool SendCommand(GWObjects::CommandDetails & Command);
		bool Connected(const std::string & SerialNumber);
		bool SendFrame(const std::string & SerialNumber, const std::string & Payload);
		void SetPendingUUID(const std::string & SerialNumber, uint64_t PendingUUID);

		bool AnalyzeRegistry(GWObjects::Dashboard &D);

	  private:
		std::map<std::string,std::shared_ptr<ConnectionEntry>>   Devices_;

		DeviceRegistry() noexcept;
	};

	inline DeviceRegistry * DeviceRegistry() { return DeviceRegistry::instance(); }

}  // namespace

#endif //UCENTRAL_UDEVICEREGISTRY_H
