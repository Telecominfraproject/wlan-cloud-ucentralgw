//
// Created by stephane bourque on 2021-02-15.
//

#include "uCentral.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IntValidator.h"
#include <iostream>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::Util::IntValidator;
using Poco::AutoPtr;

#include "TIPGWServer.h"
#include "uCentralRESTAPIServer.h"
#include "uCentralWebSocketServer.h"
#include "uStorageService.h"
#include "DeviceStatusServer.h"

#include "TIP/Api.h"
#include "TIP/Routing.h"

uCentral::uCentral():helpRequested_(false)
{
}

void uCentral::initialize(Application& self)
{
    // logging_channel_.assign(new FileChannel);
    loadConfiguration(); // load default configuration files, if present

    addSubsystem(TIPGWServer::instance());
    addSubsystem(uCentralRESTAPIServer::instance());
    addSubsystem(uCentralWebSocketServer::instance());
    addSubsystem(uStorageService::instance());

    ServerApplication::initialize(self);
    logger().information("Starting...");
    // add your own initialization code here
}

void uCentral::uninitialize()
{
    // add your own uninitialization code here
    Application::uninitialize();
}

void uCentral::reinitialize(Application& self)
{
    Application::reinitialize(self);
    // add your own reinitialization code here
}

void uCentral::defineOptions(OptionSet& options)
{
    Application::defineOptions(options);

    options.addOption(
            Option("help", "h", "display help information on command line arguments")
                    .required(false)
                    .repeatable(false)
                    .callback(OptionCallback<uCentral>(this, &uCentral::handleHelp)));

    options.addOption(
            Option("file", "f", "specify the configuration file")
                    .required(false)
                    .repeatable(true)
                    .argument("file")
                    .callback(OptionCallback<uCentral>(this, &uCentral::handleConfig)));

    options.addOption(
            Option("debug", "d", "run in debug mode")
                    .required(false)
                    .repeatable(true)
                    .callback(OptionCallback<uCentral>(this, &uCentral::handleDebug)));

    options.addOption(
            Option("port", "p", "bind to port")
                    .required(false)
                    .repeatable(false)
                    .argument("value")
                    .validator(new IntValidator(0,9999))
                    .callback(OptionCallback<uCentral>(this, &uCentral::handlePort)));
}

void uCentral::handleHelp(const std::string& name, const std::string& value)
{
    helpRequested_ = true;
    displayHelp();
    stopOptionsProcessing();
}

void uCentral::handleDebug(const std::string& name, const std::string& value)
{
    defineProperty(value);
}

void uCentral::handlePort(const std::string& name, const std::string& value)
{
    defineProperty(value);
}

void uCentral::handleConfig(const std::string& name, const std::string& value)
{
    std::cout << "Configuration file name: " << value << std::endl;
    loadConfiguration(value);
}

void uCentral::displayHelp()
{
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("A uCentral gateway implementation for TIP.");
    helpFormatter.format(std::cout);
}

void uCentral::defineProperty(const std::string& def)
{
    std::string name;
    std::string value;
    std::string::size_type pos = def.find('=');
    if (pos != std::string::npos)
    {
        name.assign(def, 0, pos);
        value.assign(def, pos + 1, def.length() - pos);
    }
    else name = def;
    config().setString(name, value);
}

int uCentral::main(const ArgVec& args)
{
    if (!helpRequested_)
    {
        Logger& logger = Logger::get("App");

        uStorageService::instance()->start();
        DeviceStatusServer::instance()->start();
        TIPGWServer::instance()->start();
        uCentralRESTAPIServer::instance()->start();
        uCentralWebSocketServer::instance()->start();

        waitForTerminationRequest();

        TIPGWServer::instance()->stop();
        uCentralRESTAPIServer::instance()->stop();
        uCentralWebSocketServer::instance()->stop();
        DeviceStatusServer::instance()->stop();
        uStorageService::instance()->stop();
    }

    return Application::EXIT_OK;
}
