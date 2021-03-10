//
// Created by stephane bourque on 2021-02-15.
//

#ifndef UCENTRAL_UCENTRAL_H
#define UCENTRAL_UCENTRAL_H

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

#include <iostream>
#include <sstream>

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

    class Daemon : public Poco::Util::ServerApplication {

    public:
        Daemon();

    protected:
        void initialize(Application &self) override;
        void uninitialize() override;
        void reinitialize(Application &self) override;
        void defineOptions(OptionSet &options) override;
        void handleHelp(const std::string &name, const std::string &value);
        void handleDebug(const std::string &name, const std::string &value);
        void handlePort(const std::string &name, const std::string &value);
        void handleConfig(const std::string &name, const std::string &value);
        void displayHelp();
        void defineProperty(const std::string &def);
        int main(const ArgVec &args) override;

    private:
        bool helpRequested_;
        AutoPtr<FileChannel> logging_channel_;
    };

};

#endif //UCENTRAL_UCENTRAL_H
