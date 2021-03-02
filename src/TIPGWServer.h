//
// Created by stephane bourque on 2021-02-12.
//

#ifndef UCENTRAL_TIPGWSERVER_H
#define UCENTRAL_TIPGWSERVER_H

#include "PropertiesFileServerList.h"

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

#include "SubSystemServer.h"

class TIPGWServer: public SubSystemServer
{
public:
    TIPGWServer() noexcept;

    int start();
    void stop();

    Logger & logger() { return SubSystemServer::logger(); };

    static TIPGWServer *instance() {
        if(instance_== nullptr) {
            instance_ = new TIPGWServer;
        }
        return instance_;
    }


private:
    static TIPGWServer *instance_;
};

#endif //UCENTRAL_TIPGWSERVER_H
