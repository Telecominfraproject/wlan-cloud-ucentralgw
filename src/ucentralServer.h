//
// Created by stephane bourque on 2021-02-12.
//

#ifndef UCENTRAL_UCENTRALSERVER_H
#define UCENTRAL_UCENTRALSERVER_H

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IntValidator.h"
#include "Poco/AutoPtr.h"
#include <iostream>
#include <sstream>


using Poco::Util::Application;
using Poco::Util::ServerApplication;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::Util::IntValidator;
using Poco::AutoPtr;


class UCentralGW: public Poco::Util::Subsystem
{
public:
    UCentralGW();

protected:
    const char *name() const { return "uCentralGW"; };
    void initialize(Application &self);
    void uninitialize();
    void reinitialize(Application& self);
    void defineOptions(OptionSet &options);

private:
    bool helpRequested_;
};

#endif //UCENTRAL_UCENTRALSERVER_H
