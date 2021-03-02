//
// Created by stephane bourque on 2021-03-01.
//

#ifndef UCENTRAL_SUBSYSTEMSERVER_H
#define UCENTRAL_SUBSYSTEMSERVER_H

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IntValidator.h"
#include "Poco/AutoPtr.h"
#include "Poco/Logger.h"

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

#include "PropertiesFileServerList.h"

class SubSystemServer : public Poco::Util::Application::Subsystem {
public:
    SubSystemServer(const std::string &name, const std::string & LoggingName, const std::string & SubSystemPrefix );
    virtual int start() = 0;
    virtual void stop() = 0;
    void initialize(Application &self);
    void uninitialize();
    void reinitialize(Application & self);
    void defineOptions(OptionSet &options);
    const char *name() const { return name_.c_str(); };
    const PropertiesFileServerEntry & host(int index) { return servers_[index]; };
    Logger                  & logger() { return logger_;};

private:
    std::string                 name_;
    Logger                  &   logger_;
    PropertiesFileServerList    servers_;
};

#endif //UCENTRAL_SUBSYSTEMSERVER_H
