//
// Created by stephane bourque on 2021-11-20.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/Net/IPAddress.h"
#include "nlohmann/json.hpp"

namespace OpenWifi {

	class FindCountryFromIP : public SubSystemServer {
	  public:
		static auto *instance() {
			static auto instance_ = new FindCountryFromIP;
			return instance_;
		}

		inline int Start() final {
			Token_ = MicroService::instance().ConfigGetString("iptocountry.ipinfo.token","");
			return 0;
		}

		inline void Stop() final {
		}

		[[nodiscard]] inline std::string ReformatAddress(const std::string & I )
		{
			if(I.substr(0,8) == "[::ffff:")
			{
				unsigned long PClosingBracket = I.find_first_of(']');

				std::string ip = I.substr(8, PClosingBracket-8);
				return ip;
			}
			return I;
		}

		inline std::string Get(const Poco::Net::IPAddress & IP) {
			if(Token_.empty())
				return "";

			try {
				std::string URL = "https://ipinfo.io/" + ReformatAddress(IP.toString()) + "?token=" + Token_;
				std::cout << "URL: " << URL << std::endl;
				std::string Response;
				if (Utils::wgets(URL, Response)) {
					nlohmann::json 		IPInfo = nlohmann::json::parse(Response);
					if(IPInfo.contains("country") && IPInfo["country"].is_string()) {
						std::cout << "Country is " << to_string(IPInfo["country"]) << std::endl;
						return IPInfo["country"];
					}
					std::cout << "1 - Could not wget" << std::endl;
				} else {
					std::cout << "2 - Could not wget" << std::endl;
					return "";
				}
			} catch(...) {
				std::cout << "3 - Could not wget" << std::endl;
			}
			return "";
		}

	  private:
		std::string Token_;

		FindCountryFromIP() noexcept:
			SubSystemServer("IpToCountry", "IPTOC-SVR", "iptocountry")
		{
		}
	};

	inline FindCountryFromIP * FindCountryFromIP() { return FindCountryFromIP::instance(); }

}
