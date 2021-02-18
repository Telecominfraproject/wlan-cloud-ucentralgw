//
// Created by stephane bourque on 2021-02-15.
//

#include "app.h"

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

#include "common.h"
#include "ucentralServer.h"
#include "TIP/Api.h"

App::App():helpRequested_(false)
{
}

void App::initialize(Application& self)
{
    // logging_channel_.assign(new FileChannel);
    addSubsystem(new UCentralGW);
    loadConfiguration(); // load default configuration files, if present
    Application::initialize(self);
    // add your own initialization code here
}

void App::uninitialize()
{
    // add your own uninitialization code here
    Application::uninitialize();
}

void App::reinitialize(Application& self)
{
    Application::reinitialize(self);
    // add your own reinitialization code here
}

void App::defineOptions(OptionSet& options)
{
    Application::defineOptions(options);

    options.addOption(
            Option("help", "h", "display help information on command line arguments")
                    .required(false)
                    .repeatable(false)
                    .callback(OptionCallback<App>(this, &App::handleHelp)));

    options.addOption(
            Option("file", "f", "specify the configuration file")
                    .required(false)
                    .repeatable(true)
                    .argument("file")
                    .callback(OptionCallback<App>(this, &App::handleConfig)));

    options.addOption(
            Option("debug", "d", "run in debug mode")
                    .required(false)
                    .repeatable(true)
                    .callback(OptionCallback<App>(this, &App::handleDebug)));

    options.addOption(
            Option("port", "p", "bind to port")
                    .required(false)
                    .repeatable(false)
                    .argument("value")
                    .validator(new IntValidator(0,9999))
                    .callback(OptionCallback<App>(this, &App::handlePort)));
}

void App::handleHelp(const std::string& name, const std::string& value)
{
    helpRequested_ = true;
    displayHelp();
    stopOptionsProcessing();
}

void App::handleDebug(const std::string& name, const std::string& value)
{
    defineProperty(value);
}

void App::handlePort(const std::string& name, const std::string& value)
{
    defineProperty(value);
}

void App::handleConfig(const std::string& name, const std::string& value)
{
    std::cout << "Configuration file name: " << value << std::endl;
    loadConfiguration(value);
}

void App::displayHelp()
{
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("A uCentral gateway implementation for TIP.");
    helpFormatter.format(std::cout);
}

void App::defineProperty(const std::string& def)
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

int App::main(const ArgVec& args)
{
    if (!helpRequested_)
    {
        // logging_channel_-> setProperty("path",App::instance().config().getString("logging.path"));
        // logging_channel_-> setProperty("rotation",App::instance().config().getString("logging.rotation"));
        // logging_channel_-> setProperty("archive", "timestamp");

        // AutoPtr<Formatter>  Ftr(new AppLogFormatter);

        // AutoPtr<FormattingChannel> FC( new FormattingChannel(Ftr,logging_channel_));


        // Logger::root().setChannel(FC);
        DBG
        Logger& logger = Logger::get("App");

        logger.information("Command line:");
        std::ostringstream ostr;
        for(auto &it: args)
        {
            // ostr << it << ' ';
        }
        logger.information(ostr.str());
        logger.information("Arguments to main():");
        for(auto &it: args)
        {
            // logger().information(it);
        }
        logger.information("Application properties:");
        printProperties("");

        TIP::API::API Tip;

        std::cout << "1st login" << std::endl;
        Tip.Login();

        std::vector<TIP::EquipmentGateway::EquipmentGatewayRecord> gateways;

        gateways = Tip.GetRoutingGatewaysByType();
        std::cout << "ID: " << gateways[0].id() << std::endl;

        TIP::EquipmentGateway::EquipmentGatewayRecord E = Tip.GetRoutingGateway(1691801527873612933);
        std::cout << "Hostname: " << E.hostname() << std::endl;

        gateways = Tip.GetRoutingGatewaysByHost("10.1.124.61");
        std::cout << "ID: " << gateways[0].id() << std::endl;

        TIP::EquipmentGateway::EquipmentGatewayRecord rr = gateways[0];

        rr.hostname("10.3.111.2");
        rr.ipAddr("10.3.111.2");
        rr.port( 9911);
        Tip.CreateRoutingGateway(rr);

        gateways = Tip.GetRoutingGatewaysByHost("10.3.111.2");

        std::cout << "ID: " << gateways[0].id() << "  Host: " << gateways[0].hostname() << std::endl;

    }

    return Application::EXIT_OK;
}

void App::printProperties(const std::string& base)
{
    AbstractConfiguration::Keys keys;
    config().keys(base, keys);
    if (keys.empty())
    {
        if (config().hasProperty(base))
        {
            std::string msg;
            msg.append(base);
            msg.append(" = ");
            msg.append(config().getString(base));
            logger().information(msg);
        }
    }
    else
    {
        for(auto &it: keys)
        {
            std::string fullKey = base;
            if (!fullKey.empty()) fullKey += '.';
            fullKey.append(it);
            printProperties(fullKey);
        }
    }
}
