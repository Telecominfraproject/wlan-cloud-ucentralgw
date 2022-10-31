//
// Created by stephane bourque on 2021-08-11.
//

#pragma once

#include "framework/SubSystemServer.h"

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
		inline bool NumberExists(uint64_t SerialNumber) {
			std::lock_guard		G(Mutex_);
			return std::find(SNs_.begin(),SNs_.end(),SerialNumber)!=SNs_.end();
		}

		static inline std::string ReverseSerialNumber(const std::string &S) {
			std::string ReversedString;
			std::copy(rbegin(S),rend(S),std::back_inserter(ReversedString));
			return ReversedString;
		}

	  private:
		std::vector<uint64_t>		SNs_;
		std::vector<uint64_t>		Reverse_SNs_;

		void ReturnNumbers(const std::string &S, uint HowMany, const std::vector<uint64_t> & SNArr, std::vector<uint64_t> &A, bool ReverseResult);

		SerialNumberCache() noexcept:
			SubSystemServer("SerialNumberCache", "SNCACHE-SVR", "serialcache")
			{
				SNs_.reserve(2000);
			}
	};

	inline auto SerialNumberCache() { return SerialNumberCache::instance(); }

} // namespace OpenWiFi
