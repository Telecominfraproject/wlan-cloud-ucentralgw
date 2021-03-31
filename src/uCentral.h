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
#include "Poco/UUIDGenerator.h"
#include "Poco/ErrorHandler.h"

namespace uCentral {

    class MyErrorHandler : public Poco::ErrorHandler {
    public:
        void exception(const Poco::Exception & E) override;
        void exception(const std::exception & E) override;
        void exception() override;
    private:

    };

    class Daemon : public Poco::Util::ServerApplication {

    public:
        Daemon() :
            helpRequested_(false),
            AutoProvisioning_(false),
            DebugMode_(false),
            ID_(1) {};

        void initialize(Application &self) override;
        void uninitialize() override;
        void reinitialize(Application &self) override;
        void defineOptions(Poco::Util::OptionSet &options) override;
        void handleHelp(const std::string &name, const std::string &value);
        void handleDebug(const std::string &name, const std::string &value);
        void handleLogs(const std::string &name, const std::string &value);
        void handleConfig(const std::string &name, const std::string &value);
        void displayHelp();
        std::string CreateUUID();
        int main(const ArgVec &args) override;

        std::string IdentifyDevice(const std::string & Id ) const;

        bool AutoProvisioning() const { return AutoProvisioning_ ; }
        bool Debug() const { return DebugMode_; }
        uint64_t ID() const { return ID_; }

    private:
        bool                        helpRequested_;
        bool                        AutoProvisioning_;
        std::map<std::string,std::vector<std::string>>    DeviceTypeIdentifications_;
        std::string                 ConfigFileName_;
        std::string                 LogDir_;
        bool                        DebugMode_;
        uint64_t                    ID_;
        Poco::UUIDGenerator         UUIDGenerator_;
        MyErrorHandler              AppErrorHandler_;

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
