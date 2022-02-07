//
// Created by stephane bourque on 2021-08-11.
//

#include "SerialNumberCache.h"
#include <mutex>

#include "StorageService.h"

namespace OpenWifi {

	int SerialNumberCache::Start() {
		StorageService()->UpdateSerialNumberCache();
		return 0;
	}

	void SerialNumberCache::Stop() {

	}

	void SerialNumberCache::AddSerialNumber(const std::string &S) {
		std::lock_guard		G(Mutex_);
		uint64_t SN = std::stoull(S, nullptr, 16);
		if(std::find(std::begin(SNs_),std::end(SNs_),SN) == std::end(SNs_)) {
			auto insert_point = std::lower_bound(SNs_.begin(), SNs_.end(), SN);
			SNs_.insert(insert_point, SN);
		}
	}

	void SerialNumberCache::DeleteSerialNumber(const std::string &S) {
		std::lock_guard		G(Mutex_);

		uint64_t SN = std::stoull(S,nullptr,16);
		auto It = std::find(SNs_.begin(),SNs_.end(),SN);
		if(It != SNs_.end()) {
			SNs_.erase(It);
		}
	}

	void SerialNumberCache::FindNumbers(const std::string &S, uint HowMany, std::vector<uint64_t> &A) {
		std::lock_guard		G(Mutex_);

		if(S.length()==12) {
			uint64_t SN = std::stoull(S,nullptr,16);
			auto It = std::find(SNs_.begin(),SNs_.end(),SN);
			if(It != SNs_.end()) {
				A.push_back(*It);
			}
		} else if (S.length()<12){
			std::string SS{S};
			SS.insert(SS.end(), 12 - SS.size(), '0');
			uint64_t SN = std::stoull(SS,nullptr,16);

			auto LB = std::lower_bound(SNs_.begin(),SNs_.end(),SN);
			if(LB!=SNs_.end()) {
				for(;LB!=SNs_.end() && HowMany;++LB,--HowMany) {
					const auto TSN = Utils::IntToSerialNumber(*LB);
					if(S == TSN.substr(0,S.size())) {
						A.emplace_back(*LB);
					} else {
						break;
					}
				}
			}
		}
	}
}