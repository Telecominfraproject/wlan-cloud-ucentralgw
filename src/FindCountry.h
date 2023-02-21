//
// Created by stephane bourque on 2021-11-20.
//

#pragma once

#include "Poco/Net/IPAddress.h"

#include "framework/MicroServiceFuncs.h"
#include "framework/SubSystemServer.h"

#include "nlohmann/json.hpp"

namespace OpenWifi {

	class IPToCountryProvider {
	  public:
		virtual bool Init() = 0;
		virtual Poco::URI URI(const std::string &IPAddress) = 0;
		virtual std::string Country(const std::string &Response) = 0;
		virtual ~IPToCountryProvider(){};
	};

	class IPInfo : public IPToCountryProvider {
	  public:
		static std::string Name() { return "ipinfo"; }
		inline bool Init() override {
			Key_ = MicroServiceConfigGetString("iptocountry.ipinfo.token", "");
			return !Key_.empty();
		}

		[[nodiscard]] inline Poco::URI URI(const std::string &IPAddress) override {
			Poco::URI U("https://ipinfo.io");
			U.setPath("/" + IPAddress);
			U.addQueryParameter("token", Key_);
			return U;
		}

		inline std::string Country(const std::string &Response) override {
			try {
				nlohmann::json IPInfo = nlohmann::json::parse(Response);
				if (IPInfo.contains("country") && IPInfo["country"].is_string()) {
					return IPInfo["country"];
				}
			} catch (...) {
			}
			return "";
		}

	  private:
		std::string Key_;
	};

	class IPData : public IPToCountryProvider {
	  public:
		static std::string Name() { return "ipdata"; }
		inline bool Init() override {
			Key_ = MicroServiceConfigGetString("iptocountry.ipdata.apikey", "");
			return !Key_.empty();
		}

		[[nodiscard]] inline Poco::URI URI(const std::string &IPAddress) override {
			Poco::URI U("https://api.ipdata.co");
			U.setPath("/" + IPAddress);
			U.addQueryParameter("api-key", Key_);
			return U;
		}

		inline std::string Country(const std::string &Response) override {
			try {
				nlohmann::json IPInfo = nlohmann::json::parse(Response);
				if (IPInfo.contains("country_code") && IPInfo["country_code"].is_string()) {
					return IPInfo["country_code"];
				}
			} catch (...) {
			}
			return "";
		}

	  private:
		std::string Key_;
	};

	class IP2Location : public IPToCountryProvider {
	  public:
		static std::string Name() { return "ip2location"; }
		inline bool Init() override {
			Key_ = MicroServiceConfigGetString("iptocountry.ip2location.apikey", "");
			return !Key_.empty();
		}

		[[nodiscard]] inline Poco::URI URI(const std::string &IPAddress) override {
			Poco::URI U("https://api.ip2location.com/v2");
			U.setPath("/");
			U.addQueryParameter("ip", IPAddress);
			U.addQueryParameter("package", "WS1");
			U.addQueryParameter("key", Key_);
			return U;
		}

		inline std::string Country(const std::string &Response) override {
			try {
				nlohmann::json IPInfo = nlohmann::json::parse(Response);
				if (IPInfo.contains("country_code") && IPInfo["country_code"].is_string()) {
					return IPInfo["country_code"];
				}
			} catch (...) {
			}
			return "";
		}

	  private:
		std::string Key_;
	};

	template <typename BaseClass, typename T, typename... Args>
	std::unique_ptr<BaseClass> IPLocationProvider(const std::string &RequestProvider) {
		if (T::Name() == RequestProvider) {
			return std::make_unique<T>();
		}
		if constexpr (sizeof...(Args) == 0) {
			return nullptr;
		} else {
			return IPLocationProvider<BaseClass, Args...>(RequestProvider);
		}
	}

	class FindCountryFromIP : public SubSystemServer {
	  public:
		static auto instance() {
			static auto instance_ = new FindCountryFromIP;
			return instance_;
		}

		inline int Start() final {
			poco_notice(Logger(), "Starting...");
			ProviderName_ = MicroServiceConfigGetString("iptocountry.provider", "");
			if (!ProviderName_.empty()) {
				Provider_ = IPLocationProvider<IPToCountryProvider, IPInfo, IPData, IP2Location>(
					ProviderName_);
				if (Provider_ != nullptr) {
					Enabled_ = Provider_->Init();
				}
			}
			Default_ = MicroServiceConfigGetString("iptocountry.default", "US");
			return 0;
		}

		inline void Stop() final {
			poco_notice(Logger(), "Stopping...");
			//	Nothing to do - just to provide the same look at the others.
			poco_notice(Logger(), "Stopped...");
		}

		[[nodiscard]] static inline std::string ReformatAddress(const std::string &I) {
			if (I.substr(0, 7) == "::ffff:") {
				std::string ip = I.substr(7);
				return ip;
			}
			return I;
		}

		inline std::string Get(const Poco::Net::IPAddress &IP) {
			if (!Enabled_)
				return Default_;
			return Get(ReformatAddress(IP.toString()));
		}

		inline std::string Get(const std::string &IP) {
			if (!Enabled_)
				return Default_;
			try {
				std::string URL = Provider_->URI(IP).toString();
				std::string Response;
				if (Utils::wgets(URL, Response)) {
					auto Answer = Provider_->Country(Response);
					if (!Answer.empty())
						return Answer;
				}
			} catch (...) {
			}
			return Default_;
		}

		inline auto Enabled() const { return Enabled_; }

	  private:
		bool Enabled_ = false;
		std::string Default_;
		std::unique_ptr<IPToCountryProvider> Provider_;
		std::string ProviderName_;

		FindCountryFromIP() noexcept : SubSystemServer("IpToCountry", "IPTOC-SVR", "iptocountry") {}
	};

	inline auto FindCountryFromIP() { return FindCountryFromIP::instance(); }

} // namespace OpenWifi
