//
// Created by stephane bourque on 2022-11-22.
//

#pragma once

#include <fstream>
#include <shared_mutex>

#include "framework/SubSystemServer.h"
#include "framework/MicroServiceFuncs.h"
#include "framework/utils.h"

#include "AP_restrictions.h"
#include "Poco/DigestStream.h"
#include "Poco/DigestEngine.h"
#include "Poco/Crypto/RSADigestEngine.h"
#include "Poco/StreamCopier.h"
#include "Poco/File.h"
#include "Poco/StringTokenizer.h"
#include "Poco/TemporaryFile.h"
#include "fmt/format.h"

namespace OpenWifi {

	class SignatureManager : public SubSystemServer {
	  public:
		inline static auto instance() {
			static auto instance_ = new SignatureManager;
			return instance_;
		}

		struct SignatureCacheEntry {
			std::string 	vendor_uri_hash;
			std::string 	signature;
		};

		inline int Start() final {
			poco_notice(Logger(),"Starting...");

			std::shared_lock L(KeyMutex_);

			CacheFilename_ = MicroServiceDataDirectory() + "/signature_cache";
			Poco::File	CacheFile(CacheFilename_);

			if(CacheFile.exists()) {
				std::fstream	CacheFileContent(CacheFilename_, std::ios_base::in);
				std::string 	line;
				while(std::getline(CacheFileContent, line)) {
					auto Tokens = Poco::StringTokenizer(line,":");
					if(Tokens.count()==2) {
						SignatureCache_[Tokens[0]] = Tokens[1];
					}
				}
			}
			poco_information(Logger(),fmt::format("Found {} entries in signature cache.", SignatureCache_.size()));

			// read all the key vendors.
			//		signature.manager.0.key.public
			//		signature.manager.0.key.private
			//		signature.manager.0.vendor
			int i=0;
			while(true) {
				auto Vendor = MicroServiceConfigGetString("signature.manager." + std::to_string(i) + ".vendor","");
				auto PrivateKey = MicroServiceConfigPath("signature.manager." + std::to_string(i) + ".key.private","");
				auto PublicKey = MicroServiceConfigPath("signature.manager." + std::to_string(i) + ".key.public","");
				if(Vendor.empty() || PrivateKey.empty() || PublicKey.empty()) {
					break;
				}
				Poco::File	PubKey(PublicKey), PrivKey(PrivateKey);
				if(PubKey.exists() && PrivKey.exists()) {
					Keys_[Vendor] = Poco::SharedPtr<Poco::Crypto::RSAKey>(
						new Poco::Crypto::RSAKey(PublicKey, PrivateKey, ""));
				}
				++i;
			}

			poco_information(Logger(),fmt::format("{} signatures in dictionary.", Keys_.size()));

			return 0;
		}

		inline void Stop() final {
			poco_notice(Logger(),"Stopping...");
			poco_notice(Logger(),"Stopped...");
		}

		inline std::string Sign(const AP_Restrictions &Restrictions, const std::string &Data) const {
			std::shared_lock L(KeyMutex_);
			try {
				if (Restrictions.algo() == "static") {
					return "aaaaaaaaaa";
				}
				auto Vendor = Keys_.find(Restrictions.vendor());
				if (Vendor == Keys_.end()) {
					poco_error( Logger(), fmt::format("{}: vendor unknown.", Vendor->first));
					return "";
				}

				if (Restrictions.algo() == "dgst-sha256") {
					Poco::Crypto::RSADigestEngine R(*Vendor->second, "SHA256");
					Poco::DigestOutputStream ostr(R);
					ostr << Data;
					ostr.flush();
					auto Signature = Utils::base64encode(
						(const unsigned char *)R.signature().data(), R.signature().size());
					return Signature;
				}
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			}
			return "";
		}

		inline std::string Sign(const AP_Restrictions &Restrictions, const Poco::URI &uri)  {
			std::shared_lock L(KeyMutex_);
			std::cout << __LINE__ << std::endl;
			try {
				std::cout << __LINE__ << std::endl;
				if (Restrictions.algo() == "static") {
					std::cout << __LINE__ << std::endl;
					return "aaaaaaaaaa";
				}

				std::cout << __LINE__ << std::endl;
				auto Vendor = Keys_.find(Restrictions.vendor());
				std::cout << __LINE__ << std::endl;
				if (Vendor == Keys_.end()) {
					std::cout << __LINE__ << std::endl;
					poco_error( Logger(), fmt::format("{}: vendor unknown.", Restrictions.vendor()));
					std::cout << __LINE__ << std::endl;
					return "";
				}

				std::cout << __LINE__ << std::endl;
				if (Restrictions.algo() == "dgst-sha256") {
					std::cout << __LINE__ << std::endl;
					auto FileHash =
						Utils::ComputeHash(Restrictions.vendor(), Restrictions.algo(), uri.getPathAndQuery());
					std::cout << __LINE__ << std::endl;
					auto CacheEntry = SignatureCache_.find(FileHash);
					std::cout << __LINE__ << std::endl;
					if (CacheEntry != end(SignatureCache_)) {
						std::cout << __LINE__ << std::endl;
						return CacheEntry->second;
					}

					std::cout << __LINE__ << std::endl;
					Poco::TemporaryFile TempDownloadedFile;
					std::cout << __LINE__ << std::endl;
					if (Utils::wgetfile(uri, TempDownloadedFile.path())) {
						std::cout << __LINE__ << std::endl;
						Poco::Crypto::RSADigestEngine R(*Vendor->second, "SHA256");
						std::cout << __LINE__ << std::endl;
						Poco::DigestOutputStream ofs(R);
						std::cout << __LINE__ << std::endl;
						std::fstream ifs(TempDownloadedFile.path(),
										 std::ios_base::in | std::ios_base::binary);
						std::cout << __LINE__ << std::endl;
						Poco::StreamCopier::copyStream(ifs, ofs);
						std::cout << __LINE__ << std::endl;
						ofs.flush();
						std::cout << __LINE__ << std::endl;
						auto Signature = Utils::base64encode((const unsigned char *)R.signature().data(),R.signature().size());
						std::cout << __LINE__ << std::endl;
						SignatureCache_[FileHash] = Signature;
						std::cout << __LINE__ << std::endl;
						SaveCache();
						std::cout << __LINE__ << std::endl;
						return Signature;
					}
				}
			} catch (const Poco::Exception &E) {
				std::cout << __LINE__ << std::endl;
				Logger().log(E);
			}
			std::cout << __LINE__ << std::endl;
			return "";
		}

		void SaveCache() {
			std::ofstream 	ofs(CacheFilename_, std::ios_base::trunc | std::ios_base::out);

			for(const auto &[hash,signature]:SignatureCache_) {
				ofs << hash << ":" << signature << std::endl;
			}
		}

	  private:
		mutable std::shared_mutex KeyMutex_;
		std::map<std::string, Poco::SharedPtr<Poco::Crypto::RSAKey>> Keys_;
		std::map<std::string,std::string>		SignatureCache_;
		std::string 							CacheFilename_;
		explicit SignatureManager() noexcept
			: SubSystemServer("SignatureManager", "SIGNATURE-MGR", "signature.manager") {}
	};

	inline auto SignatureManager() { return SignatureManager::instance(); }

}

