//
// Created by stephane bourque on 2021-03-01.
//

#include "SubSystemServer.h"

SubSystemServer::SubSystemServer( const std::string &Name,
                                  const std::string & LoggingPrefix,
                                  const std::string & SubSystemConfigPrefix )
    :   name_(Name),
        logger_(Logger::get(LoggingPrefix)),
        SubSystemConfigPrefix_(SubSystemConfigPrefix),
        servers_(SubSystemConfigPrefix)
    {

    }

void SubSystemServer::initialize(Application & self)
{
    logger_.information("Initializing...");
    servers_.initialize();
}

void SubSystemServer::uninitialize()
{
    // add your own uninitialization code here
}

void SubSystemServer::reinitialize(Application & self)
{
    // add your own reinitialization code here
}

void SubSystemServer::defineOptions(OptionSet& options)
{
}
