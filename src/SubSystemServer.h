//
// Created by stephane bourque on 2021-03-01.
//

#ifndef UCENTRAL_SUBSYSTEMSERVER_H
#define UCENTRAL_SUBSYSTEMSERVER_H

#include <mutex>

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Logger.h"
#include "Poco/Net/SecureServerSocket.h"

class PropertiesFileServerEntry {
public:
    PropertiesFileServerEntry( std::string Address,
                               uint32_t port,
                               std::string Key_file,
                               std::string Cert_file,
                               std::string Key_file_password = "",
                               std::string Name="") :
            address_(std::move(Address)),
            port_(port),
            key_file_(std::move(Key_file)),
            cert_file_(std::move(Cert_file)),
            key_file_password_(std::move(Key_file_password)),
            name_(std::move(Name)){};
    
    [[nodiscard]] const std::string & address() const { return address_; };
    [[nodiscard]] uint32_t port() const { return port_; };
    [[nodiscard]] const std::string & key_file() const { return key_file_; };
    [[nodiscard]] const std::string & cert_file() const { return cert_file_; };
    [[nodiscard]] const std::string & key_file_password() const { return key_file_password_; };
    [[nodiscard]] const std::string & name() const { return name_; };
    [[nodiscard]] Poco::Net::SecureServerSocket CreateSecureSocket() const;

private:
    std::string     address_;
    std::string     cert_file_;
    std::string     key_file_;
    std::string     key_file_password_;
    uint32_t        port_;
    std::string     name_;
};

class SubSystemServer : public Poco::Util::Application::Subsystem {

public:
    SubSystemServer(std::string Name, const std::string & LoggingName, std::string SubSystemPrefix );
    virtual int Start() = 0;
    virtual void Stop() = 0;
    void initialize(Poco::Util::Application &self) override;
    void uninitialize() override;
    void reinitialize(Poco::Util::Application & self) override;
    void defineOptions(Poco::Util::OptionSet &options) override;
    const char *name() const override { return Name_.c_str(); };
    const PropertiesFileServerEntry & host(int index) { return ConfigServersList_[index]; };
    Poco::Logger                  & Logger() { return Logger_;};

protected:
    Poco::Logger            &Logger_;
    std::string             Name_;
    std::vector<PropertiesFileServerEntry> ConfigServersList_;
    std::string             SubSystemConfigPrefix_;
};

#endif //UCENTRAL_SUBSYSTEMSERVER_H
