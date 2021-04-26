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

            PropertiesFileServerEntry entry(   uCentral::ServiceConfig::getString(address,""),
                                               uCentral::ServiceConfig::getInt(port,0),
                                               uCentral::ServiceConfig::getString(key,""),
                                               uCentral::ServiceConfig::getString(cert,""),
											   uCentral::ServiceConfig::getString(rootca,""),
                                               uCentral::ServiceConfig::getString(key_password,""),
                                               uCentral::ServiceConfig::getString(name,""),
												uCentral::ServiceConfig::getBool(x509,false),
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
    if(address_=="*") {
		if(is_x509_) {
			Poco::Net::Context::Params	P;
			P.verificationMode = Poco::Net::Context::VERIFY_STRICT;
			P.verificationDepth = 9;
			P.loadDefaultCAs = true;
			P.caLocation = root_ca_;
			auto Context = new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, P);
			Poco::Crypto::X509Certificate	C(cert_file_);
			Context->useCertificate(C);
			return Poco::Net::SecureServerSocket(port_, backlog_, Context);
		} else {
			Poco::Net::Context::Params	P;

			P.verificationMode = Poco::Net::Context::VERIFY_STRICT;
			P.verificationDepth = 9;
			P.loadDefaultCAs = true;
			P.certificateFile = cert_file_;
			P.cipherList = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";
			P.dhUse2048Bits = true;

			auto Context = new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, P);
			Context->requireMinimumProtocol(Poco::Net::Context::PROTO_TLSV1_2);
			return Poco::Net::SecureServerSocket(port_, backlog_,Context);
		}
    }
    else
    {
		Poco::Net::IPAddress        Addr(address_);
		Poco::Net::SocketAddress    SockAddr(Addr,port_);
		if(is_x509_) {
			auto Context = new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, "", cert_file_, "");
			return Poco::Net::SecureServerSocket(SockAddr, backlog_, Context);
		} else {
			return Poco::Net::SecureServerSocket(
				SockAddr, backlog_,
				new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, key_file_, cert_file_,
									   ""));
		}
    }
}

