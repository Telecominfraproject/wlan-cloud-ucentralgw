//
// Created by stephane bourque on 2021-03-01.
//

#include "SubSystemServer.h"
#include "uCentral.h"

SubSystemServer::SubSystemServer( const std::string & Name,
                                  const std::string & LoggingPrefix,
                                  const std::string & SubSystemConfigPrefix )
    :   Name_(Name),
        Logger_(Poco::Logger::get(LoggingPrefix)),
        SubSystemConfigPrefix_(SubSystemConfigPrefix)
{

}

void SubSystemServer::initialize(Poco::Util::Application & self)
{
    Logger_.information("Initializing...");
    auto i=0;
    bool good=true;

    while(good) {
        std::string root{SubSystemConfigPrefix_ + ".host." + std::to_string(i) + "."};

        std::string address{root + "address"};
        if(uCentral::ServiceConfig::getString(address,"").empty()) {
            good = false;
        }
        else {
            std::string port{root + "port"};
            std::string key{root + "key"};
            std::string key_password{root + "key.password"};
            std::string cert{root + "cert"};
            std::string name{root + "name"};

            PropertiesFileServerEntry entry(   uCentral::ServiceConfig::getString(address,""),
                                               uCentral::ServiceConfig::getInt(port,0),
                                               uCentral::ServiceConfig::getString(key,""),
                                               uCentral::ServiceConfig::getString(cert,""),
                                               uCentral::ServiceConfig::getString(key_password,""),
                                               uCentral::ServiceConfig::getString(name,""));
            ConfigServersList_.push_back(entry);
            i++;
        }
    }
    // uCentral::instance().config().
}

void SubSystemServer::uninitialize()
{
    // add your own uninitialization code here
}

void SubSystemServer::reinitialize(Poco::Util::Application & self)
{
    // add your own reinitialization code here
}

void SubSystemServer::defineOptions(Poco::Util::OptionSet& options)
{
}

Poco::Net::SecureServerSocket PropertiesFileServerEntry::CreateSecureSocket() const
{
    if(address_=="*") {
        return Poco::Net::SecureServerSocket(port_,64, new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, key_file_, cert_file_, ""));
    }
    else
    {
        Poco::Net::IPAddress        Addr(address_);
        Poco::Net::SocketAddress    SockAddr(Addr,port_);
        return Poco::Net::SecureServerSocket(SockAddr,64, new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, key_file_, cert_file_, ""));
    }
}

