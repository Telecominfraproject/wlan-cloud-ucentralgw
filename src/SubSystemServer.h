//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
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

namespace OpenWifi {
class PropertiesFileServerEntry {
  public:
	PropertiesFileServerEntry(std::string Address, uint32_t port, std::string Key_file,
							  std::string Cert_file, std::string RootCa, std::string Issuer,
							  std::string ClientCas, std::string Cas,
							  std::string Key_file_password = "", std::string Name = "",
							  Poco::Net::Context::VerificationMode M =
								  Poco::Net::Context::VerificationMode::VERIFY_RELAXED,
							  int backlog = 64)
		: address_(std::move(Address)), port_(port), key_file_(std::move(Key_file)),
		  cert_file_(std::move(Cert_file)), root_ca_(std::move(RootCa)),
		  issuer_cert_file_(std::move(Issuer)), client_cas_(std::move(ClientCas)),
		  cas_(std::move(Cas)), key_file_password_(std::move(Key_file_password)),
		  name_(std::move(Name)), level_(M), backlog_(backlog){};

	[[nodiscard]] const std::string &Address() const { return address_; };
	[[nodiscard]] uint32_t Port() const { return port_; };
	[[nodiscard]] const std::string &KeyFile() const { return key_file_; };
	[[nodiscard]] const std::string &CertFile() const { return cert_file_; };
	[[nodiscard]] const std::string &RootCA() const { return root_ca_; };
	[[nodiscard]] const std::string &KeyFilePassword() const { return key_file_password_; };
	[[nodiscard]] const std::string &IssuerCertFile() const { return issuer_cert_file_; };
	[[nodiscard]] const std::string &Name() const { return name_; };
	[[nodiscard]] Poco::Net::SecureServerSocket CreateSecureSocket(Poco::Logger &L) const;
	[[nodiscard]] int Backlog() const { return backlog_; }
	void LogCert(Poco::Logger &L) const;
	void LogCas(Poco::Logger &L) const;
	static void LogCertInfo(Poco::Logger &L, const Poco::Crypto::X509Certificate &C);

  private:
	std::string address_;
	std::string cert_file_;
	std::string key_file_;
	std::string root_ca_;
	std::string key_file_password_;
	std::string issuer_cert_file_;
	std::string client_cas_;
	std::string cas_;
	uint32_t port_;
	std::string name_;
	int backlog_;
	Poco::Net::Context::VerificationMode level_;
};

class SubSystemServer : public Poco::Util::Application::Subsystem {

  public:
	SubSystemServer(std::string Name, const std::string &LoggingName, std::string SubSystemPrefix);
	void initialize(Poco::Util::Application &self) override;
	void uninitialize() override;
	void reinitialize(Poco::Util::Application &self) override;
	void defineOptions(Poco::Util::OptionSet &options) override;
	inline const std::string & Name() const { return Name_; };
	const char * name() const override { return Name_.c_str(); }

	const PropertiesFileServerEntry & Host(uint64_t index) { return ConfigServersList_[index]; };
	uint64_t HostSize() const { return ConfigServersList_.size(); }
	Poco::Logger &Logger() { return Logger_; };
	void SetLoggingLevel(Poco::Message::Priority NewPriority) { Logger_.setLevel(NewPriority); }
	int GetLoggingLevel() { return Logger_.getLevel(); }
	virtual int Start() = 0;
	virtual void Stop() = 0;

  protected:
	std::recursive_mutex Mutex_;
	Poco::Logger 		&Logger_;
	std::string 		Name_;
	std::vector<PropertiesFileServerEntry> ConfigServersList_;
	std::string 		SubSystemConfigPrefix_;
};
}
#endif //UCENTRAL_SUBSYSTEMSERVER_H
