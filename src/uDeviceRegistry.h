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
#include "uSubSystemServer.h"

// class uCentral::WebSocket::WSConnection;

namespace uCentral::DeviceRegistry {

    struct ConnectionEntry {
		void	*WSConn_;
		uCentral::Objects::ConnectionState 	Conn_;
        std::string        					LastStats;
		std::string 						LastHealthcheck;
    };

    int Start();
    void Stop();
    bool GetStatistics(const std::string &SerialNumber, std::string & Statistics);
    void SetStatistics(const std::string &SerialNumber, const std::string &stats);
	bool GetHealthcheck(const std::string &SerialNumber, std::string & Statistics);
	void SetHealthcheck(const std::string &SerialNumber, const std::string &stats);
    bool GetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
    void SetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
	uCentral::Objects::ConnectionState * Register(const std::string & SerialNumber, void *);
    void UnRegister(const std::string & SerialNumber, void *);
    // bool SendCommand(uCentral::Objects::CommandDetails & Command);
	bool SendFrame(const std::string & SerialNumber, const std::string & Payload);
    bool Connected(const std::string & SerialNumber);

    class Service : public uSubSystemServer {
    public:

        Service() noexcept;

        friend int Start();
        friend void Stop();

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

        friend bool GetStatistics(const std::string &SerialNumber, std::string & Statistics);
        friend void SetStatistics(const std::string &SerialNumber, const std::string &stats);
		friend bool GetHealthcheck(const std::string &SerialNumber, std::string & Statistics);
		friend void SetHealthcheck(const std::string &SerialNumber, const std::string &stats);
        friend bool GetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
        friend void SetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
        friend uCentral::Objects::ConnectionState * Register(const std::string & SerialNumber, void *);
        friend void UnRegister(const std::string & SerialNumber, void *);
        // friend bool SendCommand(uCentral::Objects::CommandDetails & Command);
        friend bool Connected(const std::string & SerialNumber);
		friend bool SendFrame(const std::string & SerialNumber, const std::string & Payload);

	  private:
		static Service                          *instance_;
		std::map<std::string,std::unique_ptr<ConnectionEntry>>   Devices_;

        int Start() override;
        void Stop() override;
        bool GetStatistics(const std::string &SerialNumber, std::string & Statistics);
        void SetStatistics(const std::string &SerialNumber, const std::string &stats);
        bool GetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
        void SetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
		bool GetHealthcheck(const std::string &SerialNumber, std::string & Statistics);
		void SetHealthcheck(const std::string &SerialNumber, const std::string &stats);
		uCentral::Objects::ConnectionState * Register(const std::string & SerialNumber, void *);
        void UnRegister(const std::string & SerialNumber, void *);
        bool SendCommand(uCentral::Objects::CommandDetails & Command);
        bool Connected(const std::string & SerialNumber);
		bool SendFrame(const std::string & SerialNumber, const std::string & Payload);

    };

}  // namespace

#endif //UCENTRAL_UDEVICEREGISTRY_H
