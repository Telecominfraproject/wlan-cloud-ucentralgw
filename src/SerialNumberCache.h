//
// Created by stephane bourque on 2021-08-11.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
	class SerialNumberCache : public SubSystemServer {
		public:

		static auto instance() {
		    static auto instance_ = new SerialNumberCache;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void AddSerialNumber(const std::string &SerialNumber);
		void DeleteSerialNumber(const std::string &SerialNumber);
		void FindNumbers(const std::string &SerialNumber, uint HowMany, std::vector<uint64_t> &A);
/*		inline bool NumberExists(const std::string &SerialNumber) {
			return NumberExists(Utils::SerialNumberToInt(SerialNumber));
		}
*/
		inline bool NumberExists(uint64_t SerialNumber) {
			std::lock_guard		G(Mutex_);
			return std::find(SNs_.begin(),SNs_.end(),SerialNumber)!=SNs_.end();
		}

	  private:
		uint64_t 					LastUpdate_ = 0 ;
		std::vector<uint64_t>		SNs_;

		SerialNumberCache() noexcept:
			SubSystemServer("SerialNumberCache", "SNCACHE-SVR", "serialcache")
			{
				SNs_.reserve(2000);
			}
	};

	inline auto SerialNumberCache() { return SerialNumberCache::instance(); }

} // namespace OpenWiFi
