//
// Created by stephane bourque on 2021-02-15.
//

#ifndef UCENTRAL_UCENTRAL_H
#define UCENTRAL_UCENTRAL_H

#include <iostream>
#include <sstream>

#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IntValidator.h"
#include "Poco/AutoPtr.h"
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/Formatter.h"
#include "Poco/Message.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/ErrorHandler.h"

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::Util::IntValidator;
using Poco::Logger;
using Poco::SimpleFileChannel;
using Poco::FormattingChannel;
using Poco::Formatter;
using Poco::FileChannel;
using Poco::AutoPtr;
using Poco::Message;

#include "TIPGWServer.h"
#include "uCentralWebSocketServer.h"
#include "uCentralRESTAPIServer.h"


namespace uCentral {

    class ErrorHandler : public Poco::ErrorHandler {
    public:
        void exception(const Poco::Exception & E) override;
        void exception(const std::exception & E) override;
        void exception() override;
    private:

    };

    class Daemon : public Poco::Util::ServerApplication {

    public:
        Daemon();

        bool AutoProvisioning() { return AutoProvisioning_ ; }
        void initialize(Application &self) override;
        void uninitialize() override;
        void reinitialize(Application &self) override;
        void defineOptions(OptionSet &options) override;
        void handleHelp(const std::string &name, const std::string &value);
        void handleDebug(const std::string &name, const std::string &value);
        void handleLogs(const std::string &name, const std::string &value);
        void handleConfig(const std::string &name, const std::string &value);
        void displayHelp();
        void defineProperty(const std::string &def);
        std::string CreateUUID();
        int main(const ArgVec &args) override;

        std::string IdentifyDevice(const std::string & Id );

    private:
        bool                        helpRequested_;
        bool                        AutoProvisioning_;
        AutoPtr<FileChannel>        logging_channel_;
        std::map<std::string,std::vector<std::string>>    DeviceTypeIdentifications_;
        std::string                 ConfigFileName_;
        std::string                 LogDir_;
        bool                        DebugMode_;
        Poco::UUIDGenerator         UUIDGenerator_;
        ErrorHandler                AppErrorHandler_;

    };

    namespace ServiceConfig {
        uint64_t getInt(const std::string &Key,uint64_t Default);
        uint64_t getInt(const std::string &Key);
        std::string getString(const std::string &Key,const std::string & Default);
        std::string getString(const std::string &Key);
        uint64_t getBool(const std::string &Key,bool Default);
        uint64_t getBool(const std::string &Key);
        // std::string ReplaceEnvVar(const std::string &Key);
    }

    Daemon * instance();
};

#endif //UCENTRAL_UCENTRAL_H
