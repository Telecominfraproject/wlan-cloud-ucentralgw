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
#include "uAuthService.h"

#include "TIP/Api.h"
#include "TIP/Routing.h"

namespace uCentral {

    Daemon::Daemon() : helpRequested_(false) {
    }

    void Daemon::initialize(Application &self) {
        // logging_channel_.assign(new FileChannel);
        loadConfiguration(); // load default configuration files, if present

        addSubsystem(uCentral::Storage::Service::instance());
        addSubsystem(uCentral::Auth::Service::instance());
        addSubsystem(uCentral::DeviceStatus::Service::instance());
        addSubsystem(uCentral::TIPGW::Service::instance());
        addSubsystem(uCentral::RESTAPI::Service::instance());
        addSubsystem(uCentral::WebSocket::Service::instance());

        ServerApplication::initialize(self);
        logger().information("Starting...");
        // add your own initialization code here
    }

    void Daemon::uninitialize() {
        // add your own uninitialization code here
        Application::uninitialize();
    }

    void Daemon::reinitialize(Application &self) {
        Application::reinitialize(self);
        // add your own reinitialization code here
    }

    void Daemon::defineOptions(OptionSet &options) {
        Application::defineOptions(options);

        options.addOption(
                Option("help", "h", "display help information on command line arguments")
                        .required(false)
                        .repeatable(false)
                        .callback(OptionCallback<Daemon>(this, &Daemon::handleHelp)));

        options.addOption(
                Option("file", "f", "specify the configuration file")
                        .required(false)
                        .repeatable(true)
                        .argument("file")
                        .callback(OptionCallback<Daemon>(this, &Daemon::handleConfig)));

        options.addOption(
                Option("debug", "d", "run in debug mode")
                        .required(false)
                        .repeatable(true)
                        .callback(OptionCallback<Daemon>(this, &Daemon::handleDebug)));

        options.addOption(
                Option("port", "p", "bind to port")
                        .required(false)
                        .repeatable(false)
                        .argument("value")
                        .validator(new IntValidator(0, 9999))
                        .callback(OptionCallback<Daemon>(this, &Daemon::handlePort)));
    }

    void Daemon::handleHelp(const std::string &name, const std::string &value) {
        helpRequested_ = true;
        displayHelp();
        stopOptionsProcessing();
    }

    void Daemon::handleDebug(const std::string &name, const std::string &value) {
        defineProperty(value);
    }

    void Daemon::handlePort(const std::string &name, const std::string &value) {
        defineProperty(value);
    }

    void Daemon::handleConfig(const std::string &name, const std::string &value) {
        std::cout << "Configuration file name: " << value << std::endl;
        loadConfiguration(value);
    }

    void Daemon::displayHelp() {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("A uCentral gateway implementation for TIP.");
        helpFormatter.format(std::cout);
    }

    void Daemon::defineProperty(const std::string &def) {
        std::string name;
        std::string value;
        std::string::size_type pos = def.find('=');
        if (pos != std::string::npos) {
            name.assign(def, 0, pos);
            value.assign(def, pos + 1, def.length() - pos);
        } else name = def;
        config().setString(name, value);
    }

    std::string default_config() {
        return std::string{
                "{\"uuid\":1613927736,\"steer\":{\"enabled\":1,\"network\":\"wan\",\"debug_level\":0},\"stats\":{\"interval\":60,\"neighbours\":1,\"traffic\":1,\"wifiiface\":1,\"wifistation\":1,\"pids\":1,\"serviceprobe\":1,\"lldp\":1,\"system\":1,\"poe\":1},\"phy\":[{\"band\":\"2\",\"cfg\":{\"disabled\":0,\"country\":\"DE\",\"channel\":6,\"txpower\":30,\"beacon_int\":100,\"htmode\":\"HE40\",\"hwmode\":\"11g\",\"chanbw\":20}},{\"band\":\"5\",\"cfg\":{\"mimo\":\"4x4\",\"disabled\":0,\"country\":\"DE\",\"channel\":0,\"htmode\":\"HE80\"}},{\"band\":\"5u\",\"cfg\":{\"disabled\":0,\"country\":\"DE\",\"channel\":100,\"htmode\":\"VHT80\"}},{\"band\":\"5l\",\"cfg\":{\"disabled\":0,\"country\":\"DE\",\"channel\":36,\"htmode\":\"VHT80\"}}],\"ssid\":[{\"band\":[\"2\"],\"cfg\":{\"ssid\":\"uCentral-Guest\",\"encryption\":\"psk2\",\"key\":\"OpenWifi\",\"mode\":\"ap\",\"isolate\":1,\"network\":\"guest\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f57\"}},{\"band\":[\"5l\",\"5\"],\"cfg\":{\"ssid\":\"uCentral-NAT.200\",\"encryption\":\"psk2\",\"key\":\"OpenWifi\",\"mode\":\"ap\",\"network\":\"nat200\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f51\"}},{\"band\":[\"5l\",\"5\"],\"cfg\":{\"ssid\":\"uCentral-EAP\",\"encryption\":\"wpa2\",\"server\":\"148.251.188.218\",\"port\":1812,\"auth_secret\":\"uSyncRad1u5\",\"mode\":\"ap\",\"network\":\"lan\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f51\"}},{\"band\":[\"5l\",\"5\"],\"cfg\":{\"ssid\":\"uCentral\",\"encryption\":\"psk2\",\"key\":\"OpenWifi\",\"mode\":\"ap\",\"network\":\"wan\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f51\"}}],\"network\":[{\"mode\":\"wan\",\"cfg\":{\"proto\":\"dhcp\"}},{\"mode\":\"gre\",\"cfg\":{\"vid\":\"50\",\"peeraddr\":\"50.210.104.108\"}},{\"mode\":\"nat\",\"vlan\":200,\"cfg\":{\"proto\":\"static\",\"ipaddr\":\"192.168.16.1\",\"netmask\":\"255.255.255.0\",\"mtu\":1500,\"ip6assign\":60,\"dhcp\":{\"start\":10,\"limit\":100,\"leasetime\":\"6h\"},\"leases\":[{\"ip\":\"192.168.100.2\",\"mac\":\"00:11:22:33:44:55\",\"hostname\":\"test\"},{\"ip\":\"192.168.100.3\",\"mac\":\"00:11:22:33:44:56\",\"hostname\":\"test2\"}]}},{\"mode\":\"guest\",\"cfg\":{\"proto\":\"static\",\"ipaddr\":\"192.168.12.11\",\"dhcp\":{\"start\":10,\"limit\":100,\"leasetime\":\"6h\"}}}],\"ntp\":{\"enabled\":1,\"enable_server\":1,\"server\":[\"0.openwrt.pool.ntp.org\",\"1.openwrt.pool.ntp.org\"]},\"ssh\":{\"enable\":1,\"Port\":22},\"system\":{\"timezone\":\"CET-1CEST,M3.5.0,M10.5.0/3\"},\"log\":{\"_log_proto\":\"udp\",\"_log_ip\":\"192.168.11.23\",\"_log_port\":12345,\"_log_hostname\":\"foo\",\"_log_size\":128},\"rtty\":{\"host\":\"websocket.usync.org\",\"token\":\"7049cb6b7949ba06c6b356d76f0f6275\",\"interface\":\"wan\"}}"};
    }

    void createTestRecord() {
        uint64_t Now = time(nullptr);
        std::string SerialNumber{"24f5a207a130"};

        uCentral::Storage::Service::instance()->DeleteDevice(SerialNumber);

        uCentralDevice D{.SerialNumber = SerialNumber,
                .DeviceType = "AP",
                .MACAddress = "24:f5:a2:07:a1:33",
                .Manufacturer = "LinkSys",
                .UUID = Now,
                .Configuration = default_config(),
                .Notes = "test device"};

        uCentral::Storage::Service::instance()->CreateDevice(D);

        std::vector<uCentralStatistics> Stats;

        if (uCentral::Storage::Service::instance()->GetStatisticsData(SerialNumber, 1, 100, Stats)) {
            std::cout << "Found " << std::to_string(Stats.size()) << " stats entries." << std::endl;
            // std::cout << "  entry(0): " << Stats[0].Data << std::endl;
        }
    }

    static bool path_match(const char *p, const char *r, std::map<std::string, std::string> &keys) {
        std::string param, value;

        while (*r) {
            if (*r == '{') {
                r++;
                while (*r != '}')
                    param += *r++;
                r++;
                while (*p != '/' && *p)
                    value += *p++;
                keys[param] = value;
                value.clear();
                param.clear();
            } else if (*p != *r) {
                return false;
            } else {
                r++;
                p++;
            }
        }

        return (*p == *r);
    }

    void show_kvs(std::map<std::string, std::string> &C) {
        for (auto &[key, value]:C)
            std::cout << "  " << key << ": " << value << std::endl;
    }

    void d(const char *p, const char *r) {
        std::map<std::string, std::string> KVs;

        if (path_match(p, r, KVs)) {
            std::cout << "Match: " << p << " >>> " << r << std::endl;
            show_kvs(KVs);
        } else
            std::cout << "NO Match: " << p << " >>> " << r << std::endl;
    }

    int Daemon::main(const ArgVec &args) {
        if (!helpRequested_) {
            Logger &logger = Logger::get("uCentral");

            std::cout << "Time: " << time(nullptr) << std::endl;
            uCentral::Storage::Service::instance()->start();
            uCentral::Auth::Service::instance()->start();

            createTestRecord();

            uCentral::DeviceStatus::Service::instance()->start();
            uCentral::TIPGW::Service::instance()->start();
            uCentral::RESTAPI::Service::instance()->start();
            uCentral::WebSocket::Service::instance()->start();

            waitForTerminationRequest();

            uCentral::TIPGW::Service::instance()->stop();
            uCentral::RESTAPI::Service::instance()->stop();
            uCentral::WebSocket::Service::instance()->stop();
            uCentral::DeviceStatus::Service::instance()->stop();
            uCentral::Auth::Service::instance()->stop();
            uCentral::Storage::Service::instance()->stop();
        }

        return Application::EXIT_OK;
    }

}; // end of namespace