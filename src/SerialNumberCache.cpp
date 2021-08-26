//
// Created by stephane bourque on 2021-08-11.
//

#include "SerialNumberCache.h"
#include <mutex>

#include "Utils.h"
#include "StorageService.h"

namespace OpenWifi {

	class SerialNumberCache * SerialNumberCache::instance_ = nullptr;

	int SerialNumberCache::Start() {
		Storage()->UpdateSerialNumberCache();
		return 0;
	}

	void SerialNumberCache::Stop() {

	}

	void SerialNumberCache::AddSerialNumber(const std::string &S) {
		std::lock_guard		G(M_);

		uint64_t SN = std::stoull(S,0,16);
		if(std::find(SNs_.begin(),SNs_.end(),SN) == SNs_.end()) {
			if(SNs_.size()+1 == SNs_.capacity())
				SNs_.resize(SNs_.capacity()+2000);
			SNs_.push_back(SN);
			std::sort(SNs_.begin(),SNs_.end());
		}
	}

	void SerialNumberCache::DeleteSerialNumber(const std::string &S) {
		std::lock_guard		G(M_);

		uint64_t SN = std::stoull(S,0,16);
		auto It = std::find(SNs_.begin(),SNs_.end(),SN);
		if(It != SNs_.end()) {
			SNs_.erase(It);
		}
	}

	void SerialNumberCache::FindNumbers(const std::string &S, uint HowMany, std::vector<uint64_t> &A) {
		std::lock_guard		G(M_);

		if(S.length()==12) {
			uint64_t SN = std::stoull(S,0,16);
			auto It = std::find(SNs_.begin(),SNs_.end(),SN);
			if(It != SNs_.end()) {
				A.push_back(*It);
			}
		} else if (S.length()<12){
			std::string SS{S};
			SS.insert(SS.end(), 12 - SS.size(), '0');
			uint64_t SN = std::stoull(SS,0,16);

			auto LB = std::lower_bound(SNs_.begin(),SNs_.end(),SN);
			if(LB!=SNs_.end()) {
				for(;LB!=SNs_.end() && HowMany;++LB,--HowMany) {
					std::string TSN = Utils::int_to_hex(*LB);
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