//
// Created by stephane bourque on 2021-03-01.
//

#include "SubSystemServer.h"
#include "uCentral.h"

#include "Poco/Net/X509Certificate.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"

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
	P.loadDefaultCAs = root_ca_.empty();
	P.certificateFile = cert_file_;
	P.cipherList = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";
	P.dhUse2048Bits = true;
	P.privateKeyFile = key_file_;
	P.caLocation = root_ca_;
	auto Context = new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, P);

	Poco::Crypto::X509Certificate   Cert(cert_file_);
	Poco::Crypto::X509Certificate	Issueing( root_ca_ + "/issueing.pem");
	Poco::Crypto::X509Certificate	Root( root_ca_ + "/root.pem");

	Context->useCertificate(Cert);
	Context->addChainCertificate(Issueing);
	Context->addChainCertificate(Root);

	Poco::Crypto::RSAKey            Key("",key_file_,"");
	Context->usePrivateKey(Key);

	Context->addChainCertificate(Issueing);

	Context->disableStatelessSessionResumption();
	Context->enableExtendedCertificateVerification();

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

void PropertiesFileServerEntry::log_cert_info(Poco::Logger &L, const Poco::Crypto::X509Certificate &C) {

	L.information("=============================================================================================");
	L.information(Poco::format(">          Issuer: %s",C.issuerName()));
	L.information("---------------------------------------------------------------------------------------------");
	L.information(Poco::format(">     Common Name: %s",C.issuerName(Poco::Crypto::X509Certificate::NID_COMMON_NAME)));
	L.information(Poco::format(">         Country: %s",C.issuerName(Poco::Crypto::X509Certificate::NID_COUNTRY)));
	L.information(Poco::format(">        Locality: %s",C.issuerName(Poco::Crypto::X509Certificate::NID_LOCALITY_NAME)));
	L.information(Poco::format(">      State/Prov: %s",C.issuerName(Poco::Crypto::X509Certificate::NID_STATE_OR_PROVINCE)));
	L.information(Poco::format(">        Org name: %s",C.issuerName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_NAME)));
	L.information(Poco::format(">        Org unit: %s",C.issuerName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_UNIT_NAME)));
	L.information(Poco::format(">           Email: %s",C.issuerName(Poco::Crypto::X509Certificate::NID_PKCS9_EMAIL_ADDRESS)));
	L.information(Poco::format(">         Serial#: %s",C.issuerName(Poco::Crypto::X509Certificate::NID_SERIAL_NUMBER)));
	L.information("---------------------------------------------------------------------------------------------");
	L.information(Poco::format(">         Subject: %s",C.subjectName()));
	L.information("---------------------------------------------------------------------------------------------");
	L.information(Poco::format(">     Common Name: %s",C.subjectName(Poco::Crypto::X509Certificate::NID_COMMON_NAME)));
	L.information(Poco::format(">         Country: %s",C.subjectName(Poco::Crypto::X509Certificate::NID_COUNTRY)));
	L.information(Poco::format(">        Locality: %s",C.subjectName(Poco::Crypto::X509Certificate::NID_LOCALITY_NAME)));
	L.information(Poco::format(">      State/Prov: %s",C.subjectName(Poco::Crypto::X509Certificate::NID_STATE_OR_PROVINCE)));
	L.information(Poco::format(">        Org name: %s",C.subjectName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_NAME)));
	L.information(Poco::format(">        Org unit: %s",C.subjectName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_UNIT_NAME)));
	L.information(Poco::format(">           Email: %s",C.subjectName(Poco::Crypto::X509Certificate::NID_PKCS9_EMAIL_ADDRESS)));
	L.information(Poco::format(">         Serial#: %s",C.subjectName(Poco::Crypto::X509Certificate::NID_SERIAL_NUMBER)));
	L.information("---------------------------------------------------------------------------------------------");
	L.information(Poco::format(">  Signature Algo: %s",C.signatureAlgorithm()));
	auto From = Poco::DateTimeFormatter::format(C.validFrom(),Poco::DateTimeFormat::HTTP_FORMAT);
	L.information(Poco::format(">      Valid from: %s",From));
	auto Expires = Poco::DateTimeFormatter::format(C.expiresOn(),Poco::DateTimeFormat::HTTP_FORMAT);
	L.information(Poco::format(">      Expires on: %s",Expires));
	L.information(Poco::format(">         Version: %d",(int)C.version()));
	L.information(Poco::format(">        Serial #: %s",C.serialNumber()));
	L.information("=============================================================================================");


}

void PropertiesFileServerEntry::log_cert(Poco::Logger & L) const {
	try {
		Poco::Crypto::X509Certificate C(cert_file_);
		L.information("=============================================================================================");
		L.information("=============================================================================================");
		L.information(Poco::format("Certificate Filename: %s",cert_file_));
		log_cert_info(L,C);
		L.information("=============================================================================================");
	} catch( const Poco::Exception & E) {
		L.log(E);
	}
}

void PropertiesFileServerEntry::log_cas(Poco::Logger & L) const {
	try {
		std::vector<Poco::Crypto::X509Certificate> Certs=Poco::Net::X509Certificate::readPEM(root_ca_);

		L.information("=============================================================================================");
		L.information("=============================================================================================");
		L.information(Poco::format("CA Filename: %s",root_ca_));
		L.information("=============================================================================================");
		auto i=1;
		for(const auto & C : Certs)
		{
			L.information(Poco::format(" Index: %d",i));
			L.information("=============================================================================================");
			log_cert_info(L, C);
			i++;
		}
		L.information("=============================================================================================");
	} catch ( const Poco::Exception & E ) {
		L.log(E);
	}
}
