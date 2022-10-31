//
// Created by stephane bourque on 2022-10-25.
//

#include "fmt/format.h"

#include "framework/SubSystemServer.h"

#include "Poco/Net/SSLManager.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"

#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {

	PropertiesFileServerEntry::PropertiesFileServerEntry(std::string Address, uint32_t port, std::string Key_file,
							  std::string Cert_file, std::string RootCa, std::string Issuer,
							  std::string ClientCas, std::string Cas,
							  std::string Key_file_password, std::string Name,
							  Poco::Net::Context::VerificationMode M,
							  int backlog)
		: address_(std::move(Address)),
		  port_(port),
		  cert_file_(std::move(Cert_file)),
		  key_file_(std::move(Key_file)),
		  root_ca_(std::move(RootCa)),
		  key_file_password_(std::move(Key_file_password)),
		  issuer_cert_file_(std::move(Issuer)),
		  client_cas_(std::move(ClientCas)),
		  cas_(std::move(Cas)),
		  name_(std::move(Name)),
		  backlog_(backlog),
		  level_(M) {

	  };

	[[nodiscard]] Poco::Net::SecureServerSocket PropertiesFileServerEntry::CreateSecureSocket(Poco::Logger &L) const {
		Poco::Net::Context::Params P;

		P.verificationMode = level_;
		P.verificationDepth = 9;
		P.loadDefaultCAs = root_ca_.empty();
		P.cipherList = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";
		P.dhUse2048Bits = true;
		P.caLocation = cas_;

		auto Context = Poco::AutoPtr<Poco::Net::Context>(new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, P));

		if(!key_file_password_.empty()) {
			auto PassphraseHandler = Poco::SharedPtr<MyPrivateKeyPassphraseHandler>( new MyPrivateKeyPassphraseHandler(key_file_password_,L));
			Poco::Net::SSLManager::instance().initializeServer(PassphraseHandler, nullptr,Context);
		}

		if (!cert_file_.empty() && !key_file_.empty()) {
			Poco::Crypto::X509Certificate Cert(cert_file_);
			Poco::Crypto::X509Certificate Root(root_ca_);

			Context->useCertificate(Cert);
			Context->addChainCertificate(Root);

			Context->addCertificateAuthority(Root);

			if (level_ == Poco::Net::Context::VERIFY_STRICT) {
				if (issuer_cert_file_.empty()) {
					L.fatal("In strict mode, you must supply ans issuer certificate");
				}
				if (client_cas_.empty()) {
					L.fatal("In strict mode, client cas must be supplied");
				}
				Poco::Crypto::X509Certificate Issuing(issuer_cert_file_);
				Context->addChainCertificate(Issuing);
				Context->addCertificateAuthority(Issuing);
			}

			Poco::Crypto::RSAKey Key("", key_file_, key_file_password_);
			Context->usePrivateKey(Key);

			SSL_CTX *SSLCtx = Context->sslContext();
			if (!SSL_CTX_check_private_key(SSLCtx)) {
				L.fatal(fmt::format("Wrong Certificate({}) for Key({})", cert_file_, key_file_));
			}

			SSL_CTX_set_verify(SSLCtx, SSL_VERIFY_PEER, nullptr);

			if (level_ == Poco::Net::Context::VERIFY_STRICT) {
				SSL_CTX_set_client_CA_list(SSLCtx, SSL_load_client_CA_file(client_cas_.c_str()));
			}
			SSL_CTX_enable_ct(SSLCtx, SSL_CT_VALIDATION_STRICT);
			SSL_CTX_dane_enable(SSLCtx);

			Context->enableSessionCache();
			Context->setSessionCacheSize(0);
			Context->setSessionTimeout(60);
			Context->enableExtendedCertificateVerification(true);
			Context->disableStatelessSessionResumption();
		}

		if (address_ == "*") {
			Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
				Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
												  : Poco::Net::AddressFamily::IPv4));
			Poco::Net::SocketAddress SockAddr(Addr, port_);

			return Poco::Net::SecureServerSocket(SockAddr, backlog_, Context);
		} else {
			Poco::Net::IPAddress Addr(address_);
			Poco::Net::SocketAddress SockAddr(Addr, port_);

			return Poco::Net::SecureServerSocket(SockAddr, backlog_, Context);
		}
	}

	[[nodiscard]] Poco::Net::ServerSocket PropertiesFileServerEntry::CreateSocket([[maybe_unused]] Poco::Logger &L) const {
		Poco::Net::Context::Params P;

		if (address_ == "*") {
			Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
				Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
												  : Poco::Net::AddressFamily::IPv4));
			Poco::Net::SocketAddress SockAddr(Addr, port_);
			return Poco::Net::ServerSocket(SockAddr, backlog_);
		} else {
			Poco::Net::IPAddress Addr(address_);
			Poco::Net::SocketAddress SockAddr(Addr, port_);
			return Poco::Net::ServerSocket(SockAddr, backlog_);
		}
	}

	void PropertiesFileServerEntry::LogCertInfo(Poco::Logger &L, const Poco::Crypto::X509Certificate &C) const {
		L.information("=============================================================================================");
		L.information(fmt::format(">          Issuer: {}", C.issuerName()));
		L.information("---------------------------------------------------------------------------------------------");
		L.information(fmt::format(">     Common Name: {}",
								  C.issuerName(Poco::Crypto::X509Certificate::NID_COMMON_NAME)));
		L.information(fmt::format(">         Country: {}",
								  C.issuerName(Poco::Crypto::X509Certificate::NID_COUNTRY)));
		L.information(fmt::format(">        Locality: {}",
								  C.issuerName(Poco::Crypto::X509Certificate::NID_LOCALITY_NAME)));
		L.information(fmt::format(">      State/Prov: {}",
								  C.issuerName(Poco::Crypto::X509Certificate::NID_STATE_OR_PROVINCE)));
		L.information(fmt::format(">        Org name: {}",
								  C.issuerName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_NAME)));
		L.information(
			fmt::format(">        Org unit: {}",
						C.issuerName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_UNIT_NAME)));
		L.information(
			fmt::format(">           Email: {}",
						C.issuerName(Poco::Crypto::X509Certificate::NID_PKCS9_EMAIL_ADDRESS)));
		L.information(fmt::format(">         Serial#: {}",
								  C.issuerName(Poco::Crypto::X509Certificate::NID_SERIAL_NUMBER)));
		L.information("---------------------------------------------------------------------------------------------");
		L.information(fmt::format(">         Subject: {}", C.subjectName()));
		L.information("---------------------------------------------------------------------------------------------");
		L.information(fmt::format(">     Common Name: {}",
								  C.subjectName(Poco::Crypto::X509Certificate::NID_COMMON_NAME)));
		L.information(fmt::format(">         Country: {}",
								  C.subjectName(Poco::Crypto::X509Certificate::NID_COUNTRY)));
		L.information(fmt::format(">        Locality: {}",
								  C.subjectName(Poco::Crypto::X509Certificate::NID_LOCALITY_NAME)));
		L.information(
			fmt::format(">      State/Prov: {}",
						C.subjectName(Poco::Crypto::X509Certificate::NID_STATE_OR_PROVINCE)));
		L.information(
			fmt::format(">        Org name: {}",
						C.subjectName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_NAME)));
		L.information(
			fmt::format(">        Org unit: {}",
						C.subjectName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_UNIT_NAME)));
		L.information(
			fmt::format(">           Email: {}",
						C.subjectName(Poco::Crypto::X509Certificate::NID_PKCS9_EMAIL_ADDRESS)));
		L.information(fmt::format(">         Serial#: {}",
								  C.subjectName(Poco::Crypto::X509Certificate::NID_SERIAL_NUMBER)));
		L.information("---------------------------------------------------------------------------------------------");
		L.information(fmt::format(">  Signature Algo: {}", C.signatureAlgorithm()));
		auto From = Poco::DateTimeFormatter::format(C.validFrom(), Poco::DateTimeFormat::HTTP_FORMAT);
		L.information(fmt::format(">      Valid from: {}", From));
		auto Expires =
			Poco::DateTimeFormatter::format(C.expiresOn(), Poco::DateTimeFormat::HTTP_FORMAT);
		L.information(fmt::format(">      Expires on: {}", Expires));
		L.information(fmt::format(">         Version: {}", (int)C.version()));
		L.information(fmt::format(">        Serial #: {}", C.serialNumber()));
		L.information("=============================================================================================");
	}

	void PropertiesFileServerEntry::LogCert(Poco::Logger &L) const {
		try {
			Poco::Crypto::X509Certificate C(cert_file_);
			L.information("=============================================================================================");
			L.information("=============================================================================================");
			L.information(fmt::format("Certificate Filename: {}", cert_file_));
			LogCertInfo(L, C);
			L.information("=============================================================================================");

			if (!issuer_cert_file_.empty()) {
				Poco::Crypto::X509Certificate C1(issuer_cert_file_);
				L.information("=============================================================================================");
				L.information("=============================================================================================");
				L.information(fmt::format("Issues Certificate Filename: {}", issuer_cert_file_));
				LogCertInfo(L, C1);
				L.information("=============================================================================================");
			}

			if (!client_cas_.empty()) {
				std::vector<Poco::Crypto::X509Certificate> Certs =
					Poco::Net::X509Certificate::readPEM(client_cas_);

				L.information("=============================================================================================");
				L.information("=============================================================================================");
				L.information(fmt::format("Client CAs Filename: {}", client_cas_));
				L.information("=============================================================================================");
				auto i = 1;
				for (const auto &C3 : Certs) {
					L.information(fmt::format(" Index: {}", i));
					L.information("=============================================================================================");
					LogCertInfo(L, C3);
					i++;
				}
				L.information("=============================================================================================");
			}

		} catch (const Poco::Exception &E) {
			L.log(E);
		}
	}

	void PropertiesFileServerEntry::LogCas(Poco::Logger &L) const {
		try {
			std::vector<Poco::Crypto::X509Certificate> Certs =
				Poco::Net::X509Certificate::readPEM(root_ca_);

			L.information("=============================================================================================");
			L.information("=============================================================================================");
			L.information(fmt::format("CA Filename: {}", root_ca_));
			L.information("=============================================================================================");
			auto i = 1;
			for (const auto &C : Certs) {
				L.information(fmt::format(" Index: {}", i));
				L.information("=============================================================================================");
				LogCertInfo(L, C);
				i++;
			}
			L.information("=============================================================================================");
		} catch (const Poco::Exception &E) {
			L.log(E);
		}
	}

	SubSystemServer::SubSystemServer(const std::string &Name, const std::string &LoggingPrefix,
											const std::string &SubSystemConfigPrefix):
		Name_(Name),
		LoggerPrefix_(LoggingPrefix),
		SubSystemConfigPrefix_(SubSystemConfigPrefix) {
	}

	void SubSystemServer::initialize([[maybe_unused]] Poco::Util::Application &self) {
		auto i = 0;
		bool good = true;

		auto NewLevel = MicroServiceConfigGetString("logging.level." + Name_, "");
		if(NewLevel.empty())
			Logger_ = std::make_unique<LoggerWrapper>(Poco::Logger::create(LoggerPrefix_, Poco::Logger::root().getChannel(), Poco::Logger::root().getLevel()));
		else
			Logger_ = std::make_unique<LoggerWrapper>(Poco::Logger::create(LoggerPrefix_, Poco::Logger::root().getChannel(), Poco::Logger::parseLevel(NewLevel)));

		ConfigServersList_.clear();
		while (good) {
			std::string root{SubSystemConfigPrefix_ + ".host." + std::to_string(i) + "."};

			std::string address{root + "address"};
			if (MicroServiceConfigGetString(address, "").empty()) {
				good = false;
			} else {
				std::string port{root + "port"};
				std::string key{root + "key"};
				std::string key_password{root + "key.password"};
				std::string cert{root + "cert"};
				std::string name{root + "name"};
				std::string backlog{root + "backlog"};
				std::string rootca{root + "rootca"};
				std::string issuer{root + "issuer"};
				std::string clientcas(root + "clientcas");
				std::string cas{root + "cas"};

				std::string level{root + "security"};
				Poco::Net::Context::VerificationMode M = Poco::Net::Context::VERIFY_RELAXED;

				auto L = MicroServiceConfigGetString(level, "");

				if (L == "strict") {
					M = Poco::Net::Context::VERIFY_STRICT;
				} else if (L == "none") {
					M = Poco::Net::Context::VERIFY_NONE;
				} else if (L == "relaxed") {
					M = Poco::Net::Context::VERIFY_RELAXED;
				} else if (L == "once")
					M = Poco::Net::Context::VERIFY_ONCE;

				PropertiesFileServerEntry entry(MicroServiceConfigGetString(address, ""),
												MicroServiceConfigGetInt(port, 0),
												MicroServiceConfigPath(key, ""),
												MicroServiceConfigPath(cert, ""),
												MicroServiceConfigPath(rootca, ""),
												MicroServiceConfigPath(issuer, ""),
												MicroServiceConfigPath(clientcas, ""),
												MicroServiceConfigPath(cas, ""),
												MicroServiceConfigGetString(key_password, ""),
												MicroServiceConfigGetString(name, ""), M,
												(int)MicroServiceConfigGetInt(backlog, 64));
				ConfigServersList_.push_back(entry);
				i++;
			}
		}
	}




} // namespace OpenWifi