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

#include "RESTAPI_objects.h"
#include "SubSystemServer.h"

// class uCentral::WebSocket::WSConnection;

namespace uCentral {

	class WSConnection;
    class DeviceRegistry : public SubSystemServer {
    public:
		struct ConnectionEntry {
			WSConnection 				*WSConn_;
			Objects::ConnectionState 	Conn_;
			std::string        			LastStats;
			std::string 				LastHealthcheck;
		};

        static DeviceRegistry *instance() {
            if (instance_ == nullptr) {
                instance_ = new DeviceRegistry;
            }
            return instance_;
        }

		int Start() override;
		void Stop() override;
		bool GetStatistics(const std::string &SerialNumber, std::string & Statistics);
		void SetStatistics(const std::string &SerialNumber, const std::string &stats);
		bool GetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
		void SetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
		bool GetHealthcheck(const std::string &SerialNumber, std::string & Statistics);
		void SetHealthcheck(const std::string &SerialNumber, const std::string &stats);
		uCentral::Objects::ConnectionState * Register(const std::string & SerialNumber, WSConnection *);
		void UnRegister(const std::string & SerialNumber, WSConnection *);
		bool SendCommand(uCentral::Objects::CommandDetails & Command);
		bool Connected(const std::string & SerialNumber);
		bool SendFrame(const std::string & SerialNumber, const std::string & Payload);
		void SetPendingUUID(const std::string & SerialNumber, uint64_t PendingUUID);

	  private:
		static DeviceRegistry                          *instance_;
		std::map<std::string,std::unique_ptr<ConnectionEntry>>   Devices_;

		DeviceRegistry() noexcept;
	};

	inline DeviceRegistry * DeviceRegistry() { return DeviceRegistry::instance(); }

}  // namespace

#endif //UCENTRAL_UDEVICEREGISTRY_H
