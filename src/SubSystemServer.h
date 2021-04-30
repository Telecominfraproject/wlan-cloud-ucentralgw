//
// Created by stephane bourque on 2021-03-01.
//

#ifndef UCENTRAL_SUBSYSTEMSERVER_H
#define UCENTRAL_SUBSYSTEMSERVER_H

#include <mutex>

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Logger.h"
#include "Poco/Net/SecureServerSocket.h"

#include "Poco/Net/X509Certificate.h"

class PropertiesFileServerEntry {
public:
    PropertiesFileServerEntry( 	std::string Address,
                               	uint32_t port,
                               	std::string Key_file,
                               	std::string Cert_file,
								std::string RootCa,
								std::string Issuer,
								std::string ClientCas,
                               	std::string Key_file_password = "",
                               	std::string Name="",
								bool x509=false,
								Poco::Net::Context::VerificationMode M=Poco::Net::Context::VerificationMode::VERIFY_RELAXED,
								int backlog=64) :
            address_(std::move(Address)),
            port_(port),
            key_file_(std::move(Key_file)),
            cert_file_(std::move(Cert_file)),
			root_ca_(std::move(RootCa)),
			issuer_cert_file_(std::move(Issuer)),
			client_cas_(std::move(ClientCas)),
            key_file_password_(std::move(Key_file_password)),
            name_(std::move(Name)),
		    is_x509_(x509),
			level_(M),
			backlog_(backlog){};
    
    [[nodiscard]] const std::string & address() const { return address_; };
    [[nodiscard]] uint32_t port() const { return port_; };
    [[nodiscard]] const std::string & key_file() const { return key_file_; };
    [[nodiscard]] const std::string & cert_file() const { return cert_file_; };
	[[nodiscard]] const std::string & root_ca() const { return root_ca_; };
    [[nodiscard]] const std::string & key_file_password() const { return key_file_password_; };
	[[nodiscard]] const std::string & issuer_cert_file() const { return issuer_cert_file_; };
    [[nodiscard]] const std::string & name() const { return name_; };
    [[nodiscard]] Poco::Net::SecureServerSocket CreateSecureSocket(Poco::Logger &L) const;
	[[nodiscard]] bool is_x509() const { return is_x509_; }
	[[nodiscard]] int backlog() const { return backlog_; }
	void log_cert( Poco::Logger & L ) const;
	void log_cas( Poco::Logger & L ) const;
	static void log_cert_info(Poco::Logger &L, const Poco::Crypto::X509Certificate &C);

private:
    std::string     address_;
    std::string     cert_file_;
    std::string     key_file_;
	std::string 	root_ca_;
    std::string     key_file_password_;
	std::string 	issuer_cert_file_;
	std::string 	client_cas_;
    uint32_t        port_;
    std::string     name_;
	bool 			is_x509_;
	int 			backlog_;
	Poco::Net::Context::VerificationMode 			level_;
};

class SubSystemServer : public Poco::Util::Application::Subsystem {

public:
    SubSystemServer(std::string Name, const std::string & LoggingName, std::string SubSystemPrefix );
    virtual int Start() = 0;
    virtual void Stop() = 0;
    void initialize(Poco::Util::Application &self) override;
    void uninitialize() override;
    void reinitialize(Poco::Util::Application & self) override;
    void defineOptions(Poco::Util::OptionSet &options) override;
    const char *name() const override { return Name_.c_str(); };
    const PropertiesFileServerEntry & host(int index) { return ConfigServersList_[index]; };
    Poco::Logger                  & Logger() { return Logger_;};
	void SetLoggingLevel(Poco::Message::Priority NewPriority) { Logger_.setLevel(NewPriority); }

protected:
  	std::mutex				Mutex_{};
    Poco::Logger            & Logger_;
    std::string             Name_;
    std::vector<PropertiesFileServerEntry> ConfigServersList_;
    std::string             SubSystemConfigPrefix_;
};

#endif //UCENTRAL_SUBSYSTEMSERVER_H
