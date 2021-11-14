//
// Created by stephane bourque on 2021-08-11.
//

#ifndef UCENTRALGW_SERIALNUMBERCACHE_H
#define UCENTRALGW_SERIALNUMBERCACHE_H

#include "framework/MicroService.h"

namespace OpenWifi {
	class SerialNumberCache : public SubSystemServer {
		public:

		static SerialNumberCache * instance() {
		    static SerialNumberCache * instance_ = new SerialNumberCache;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void AddSerialNumber(const std::string &S);
		void DeleteSerialNumber(const std::string &S);
		void FindNumbers(const std::string &S, uint HowMany, std::vector<uint64_t> &A);

	  private:
		uint64_t 					LastUpdate_ = 0 ;
		std::vector<uint64_t>		SNs_;
		std::mutex					M_;

		SerialNumberCache() noexcept:
			SubSystemServer("SerialNumberCache", "SNCACHE-SVR", "serialcache")
			{
				SNs_.reserve(2000);
			}
	};

	inline SerialNumberCache * SerialNumberCache() { return SerialNumberCache::instance(); }

} // namespace OpenWiFi

#endif // UCENTRALGW_SERIALNUMBERCACHE_H
