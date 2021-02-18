//
// Created by stephane bourque on 2021-02-15.
//

#ifndef UCENTRAL_APP_H
#define UCENTRAL_APP_H

#include "Poco/Util/Application.h"
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

class AppLogFormatter : public Formatter {
public:
    void format(const Message &msg, std::string & text )
    {
        std::cout << "Logging message: " << msg.getText() << std::endl;
    }
private:
};
class App: public Application
{
public:
    App();

protected:
    void initialize(Application& self);
    void uninitialize();
    void reinitialize(Application& self);
    void defineOptions(OptionSet& options);
    void handleHelp(const std::string& name, const std::string& value);
    void handleDebug(const std::string& name, const std::string& value);
    void handlePort(const std::string& name, const std::string& value);
    void handleConfig(const std::string& name, const std::string& value);
    void displayHelp();
    void defineProperty(const std::string& def);
    int main(const ArgVec& args);
    void printProperties(const std::string& base);

private:
    bool helpRequested_;
    AutoPtr<FileChannel>  logging_channel_;
};


#endif //UCENTRAL_APP_H
