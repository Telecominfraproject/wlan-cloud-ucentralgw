//
// Created by stephane bourque on 2021-03-01.
//

#include "SubSystemServer.h"
#include "uCentral.h"

SubSystemServer::SubSystemServer( std::string Name,
                                  const std::string & LoggingPrefix,
                                  std::string SubSystemConfigPrefix )
    :   Name_(std::move(Name)),
        Logger_(Poco::Logger::get(LoggingPrefix)),
        SubSystemConfigPrefix_(std::move(SubSystemConfigPrefix))
{
	Logger_.setLevel(Poco::Message::PRIO_NOTICE);
}

void SubSystemServer::initialize(Poco::Util::Application & self)
{
    Logger_.notice("Initializing...");
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
			std::string x509{root+"x509"};
			std::string backlog{root+"backlog"};
			std::string rootca{root+"rootca"};

			std::string level{root+"security"};
			Poco::Net::Context::VerificationMode	M=Poco::Net::Context::VERIFY_RELAXED;

			auto L = uCentral::ServiceConfig::getString(level,"");

			if(L=="strict") {
				M=Poco::Net::Context::VERIFY_STRICT;
			} else if(L=="none") {
				M=Poco::Net::Context::VERIFY_NONE;
			} else if(L=="relaxed") {
				M=Poco::Net::Context::VERIFY_RELAXED;
			} else if(L=="once")
				M=Poco::Net::Context::VERIFY_ONCE;

            PropertiesFileServerEntry entry(   uCentral::ServiceConfig::getString(address,""),
                                               uCentral::ServiceConfig::getInt(port,0),
                                               uCentral::ServiceConfig::getString(key,""),
                                               uCentral::ServiceConfig::getString(cert,""),
											   uCentral::ServiceConfig::getString(rootca,""),
                                               uCentral::ServiceConfig::getString(key_password,""),
                                               uCentral::ServiceConfig::getString(name,""),
												uCentral::ServiceConfig::getBool(x509,false),
												M,
											   (int) uCentral::ServiceConfig::getInt(backlog,64));
            ConfigServersList_.push_back(entry);
            i++;
        }
    }
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
	Poco::Net::Context::Params	P;

	P.verificationMode = level_;
	P.verificationDepth = 9;
	P.loadDefaultCAs = true;
	P.certificateFile = cert_file_;
	P.cipherList = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";
	P.dhUse2048Bits = true;
	P.privateKeyFile = key_file_;
	P.caLocation = root_ca_;
	auto Context = new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, P);
	Context->disableStatelessSessionResumption();
	if(address_=="*")
		return Poco::Net::SecureServerSocket(port_, backlog_,Context);
	else {
		Poco::Net::IPAddress        Addr(address_);
		Poco::Net::SocketAddress    SockAddr(Addr,port_);

		return Poco::Net::SecureServerSocket(
			SockAddr, backlog_,
			new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE,P));
	}
}

