//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>
#include <mutex>

#include "Poco/Util/Application.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/PrivateKeyPassphraseHandler.h"

namespace OpenWifi {

	class MyPrivateKeyPassphraseHandler : public Poco::Net::PrivateKeyPassphraseHandler {
	  public:
		explicit MyPrivateKeyPassphraseHandler(const std::string &Password, Poco::Logger & Logger):
			PrivateKeyPassphraseHandler(true),
			Password_(Password),
			Logger_(Logger) {
		}

		void onPrivateKeyRequested([[maybe_unused]] const void * pSender,std::string & privateKey) {
			poco_information(Logger_,"Returning key passphrase.");
			privateKey = Password_;
		};
		inline Poco::Logger & Logger() { return Logger_; }
	  private:
		std::string Password_;
		Poco::Logger & Logger_;
	};

	class PropertiesFileServerEntry {
	  public:
		PropertiesFileServerEntry(std::string Address, uint32_t port, std::string Key_file,
								  std::string Cert_file, std::string RootCa, std::string Issuer,
								  std::string ClientCas, std::string Cas,
								  std::string Key_file_password = "", std::string Name = "",
								  Poco::Net::Context::VerificationMode M =
									  Poco::Net::Context::VerificationMode::VERIFY_RELAXED,
								  int backlog = 64);

		[[nodiscard]] inline const std::string &Address() const { return address_; };
		[[nodiscard]] inline uint32_t Port() const { return port_; };
		[[nodiscard]] inline auto KeyFile() const { return key_file_; };
		[[nodiscard]] inline auto CertFile() const { return cert_file_; };
		[[nodiscard]] inline auto RootCA() const { return root_ca_; };
		[[nodiscard]] inline auto KeyFilePassword() const { return key_file_password_; };
		[[nodiscard]] inline auto IssuerCertFile() const { return issuer_cert_file_; };
		[[nodiscard]] inline auto Name() const { return name_; };
		[[nodiscard]] inline int Backlog() const { return backlog_; }
		[[nodiscard]] inline auto Cas() const { return cas_; }

		[[nodiscard]] Poco::Net::SecureServerSocket CreateSecureSocket(Poco::Logger &L) const;
		[[nodiscard]] Poco::Net::ServerSocket CreateSocket([[maybe_unused]] Poco::Logger &L) const;
		void LogCertInfo(Poco::Logger &L, const Poco::Crypto::X509Certificate &C) const;
		void LogCert(Poco::Logger &L) const;
		void LogCas(Poco::Logger &L) const;

	  private:
		std::string address_;
		uint32_t port_;
		std::string cert_file_;
		std::string key_file_;
		std::string root_ca_;
		std::string key_file_password_;
		std::string issuer_cert_file_;
		std::string client_cas_;
		std::string cas_;
		std::string name_;
		int backlog_;
		Poco::Net::Context::VerificationMode level_;
	};

	class SubSystemServer : public Poco::Util::Application::Subsystem {
	  public:
		SubSystemServer(const std::string & Name, const std::string &LoggingPrefix,
						const std::string & SubSystemConfigPrefix);

		void initialize(Poco::Util::Application &self) override;
		inline void uninitialize() override {
		}
		inline void reinitialize([[maybe_unused]] Poco::Util::Application &self) override {
			poco_information(Logger_->L_,"Reloading of this subsystem is not supported.");
		}
		inline void defineOptions([[maybe_unused]] Poco::Util::OptionSet &options) override {
		}
		inline const std::string & Name() const { return Name_; };
		inline const char * name() const override { return Name_.c_str(); }

		inline const PropertiesFileServerEntry & Host(uint64_t index) { return ConfigServersList_[index]; };
		inline uint64_t HostSize() const { return ConfigServersList_.size(); }
		inline Poco::Logger & Logger() const { return Logger_->L_; }
		inline void SetLoggingLevel(const std::string & levelName) {
			Logger_->L_.setLevel(Poco::Logger::parseLevel(levelName));
		}
		inline int GetLoggingLevel() { return Logger_->L_.getLevel(); }

		virtual int Start() = 0;
		virtual void Stop() = 0;

		struct LoggerWrapper {
			Poco::Logger & L_;
			LoggerWrapper(Poco::Logger &L) :
 				L_(L) {
			}
		};

	  protected:
		std::recursive_mutex 			Mutex_;
		std::vector<PropertiesFileServerEntry> ConfigServersList_;

	  private:
		std::unique_ptr<LoggerWrapper>  Logger_;
		std::string 					Name_;
		std::string         			LoggerPrefix_;
		std::string 					SubSystemConfigPrefix_;
	};

	typedef std::vector<SubSystemServer *>          SubSystemVec;

} // namespace OpenWifi
