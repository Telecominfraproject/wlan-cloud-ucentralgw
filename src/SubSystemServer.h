//
// Created by stephane bourque on 2021-03-01.
//

#ifndef UCENTRAL_SUBSYSTEMSERVER_H
#define UCENTRAL_SUBSYSTEMSERVER_H

#include <mutex>

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IntValidator.h"
#include "Poco/AutoPtr.h"
#include "Poco/Logger.h"
#include "Poco/Net/SecureServerSocket.h"

using Poco::Util::Application;
using Poco::Util::ServerApplication;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::Util::IntValidator;
using Poco::AutoPtr;
using Poco::Logger;

class PropertiesFileServerEntry {
public:
    PropertiesFileServerEntry( const std::string &address,
                               uint32_t port,
                               const std::string &key_file,
                               const std::string &cert_file,
                               const std::string &key_file_password = "" ) :
            address_(address),
            port_(port),
            key_file_(key_file),
            cert_file_(cert_file),
            key_file_password_(key_file_password) {};

    [[nodiscard]] const std::string & address() const { return address_; };
    [[nodiscard]] uint32_t port() const { return port_; };
    [[nodiscard]] const std::string & key_file() const { return key_file_; };
    [[nodiscard]] const std::string & cert_file() const { return cert_file_; };
    [[nodiscard]] const std::string & key_file_password() const { return key_file_password_; };
    [[nodiscard]] std::shared_ptr<Poco::Net::SecureServerSocket> CreateSecureSocket() const;

private:
    std::string     address_;
    std::string     cert_file_;
    std::string     key_file_;
    std::string     key_file_password_;
    uint32_t        port_;
};

class SubSystemServer : public Poco::Util::Application::Subsystem {

public:
    SubSystemServer(const std::string &name, const std::string & LoggingName, const std::string & SubSystemPrefix );
    virtual int Start() = 0;
    virtual void Stop() = 0;
    void initialize(Application &self) override;
    void uninitialize() override;
    void reinitialize(Application & self) override;
    void defineOptions(OptionSet &options) override;
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
