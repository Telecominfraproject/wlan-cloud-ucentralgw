//
// Created by stephane bourque on 2021-03-01.
//

#ifndef UCENTRAL_UDEVICEREGISTRY_H
#define UCENTRAL_UDEVICEREGISTRY_H

#include "Poco/JSON/Object.h"

#include "SubSystemServer.h"
#include "RESTAPI_objects.h"

namespace uCentral::DeviceRegistry {

    struct ConnectionEntry {
        void             					* WSConn_;
		uCentral::Objects::ConnectionState 	* Conn_;
        std::string        					LastStats;
    };

    int Start();
    void Stop();
    bool GetStatistics(const std::string &SerialNumber, std::string & Statistics);
    void SetStatistics(const std::string &SerialNumber, const std::string &stats);
    bool GetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
    void SetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
	uCentral::Objects::ConnectionState *  Register(const std::string & SerialNumber, void *);
    void UnRegister(const std::string & SerialNumber, void *);
    bool SendCommand(uCentral::Objects::CommandDetails & Command);
    bool Connected(const std::string & SerialNumber);

    class Service : public SubSystemServer {
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
        friend bool GetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
        friend void SetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
        friend uCentral::Objects::ConnectionState * Register(const std::string & SerialNumber, void *);
        friend void UnRegister(const std::string & SerialNumber, void *);
        friend bool SendCommand(uCentral::Objects::CommandDetails & Command);
        friend bool Connected(const std::string & SerialNumber);
    private:
		static Service                          *instance_;
		std::map<std::string,ConnectionEntry>   Devices_;

        int Start() override;
        void Stop() override;
        bool GetStatistics(const std::string &SerialNumber, std::string & Statistics);
        void SetStatistics(const std::string &SerialNumber, const std::string &stats);
        bool GetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
        void SetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State);
		uCentral::Objects::ConnectionState *  Register(const std::string & SerialNumber, void *);
        void UnRegister(const std::string & SerialNumber, void *);
        bool SendCommand(uCentral::Objects::CommandDetails & Command);
        bool Connected(const std::string & SerialNumber);
    };

}  // namespace

#endif //UCENTRAL_UDEVICEREGISTRY_H
