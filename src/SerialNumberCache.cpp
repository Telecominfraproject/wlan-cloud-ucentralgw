//
// Created by stephane bourque on 2021-08-11.
//

#include "SerialNumberCache.h"
#include <mutex>

namespace OpenWiFi {

	class SerialNumberCache * SerialNumberCache::instance_ = nullptr;

	int SerialNumberCache::Start() {

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

		assert(HowMany>0);

		if(S.length()==12) {
			uint64_t SN = std::stoull(S,0,16);
			auto It = std::find(SNs_.begin(),SNs_.end(),SN);
			if(It != SNs_.end()) {
				A.push_back(*It);
			}
		} else if (S.length()<12){
			std::string SS{S};
			SS.insert(0, 12 - SS.size(), '0');
			uint64_t SN = std::stoull(SS,0,16);

			auto LB = std::lower_bound(SNs_.begin(),SNs_.end(),SN);
			if(LB!=SNs_.end()) {
				for(;LB!=SNs_.end() && HowMany;++LB,--HowMany)
					A.emplace_back(*LB);
			}
		}
	}
}