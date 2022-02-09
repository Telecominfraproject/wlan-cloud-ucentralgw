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

			auto R = ReverseSerialNumber(S);
			uint64_t RSN = std::stoull(R, nullptr, 16);
			auto rev_insert_point = std::lower_bound(Reverse_SNs_.begin(), Reverse_SNs_.end(), RSN);
			Reverse_SNs_.insert(rev_insert_point, RSN);
		}
	}

	void SerialNumberCache::DeleteSerialNumber(const std::string &S) {
		std::lock_guard		G(Mutex_);

		uint64_t SN = std::stoull(S,nullptr,16);
		auto It = std::find(SNs_.begin(),SNs_.end(),SN);
		if(It != SNs_.end()) {
			SNs_.erase(It);

			auto R = ReverseSerialNumber(S);
			uint64_t RSN = std::stoull(R, nullptr, 16);
			auto RIt = std::find(Reverse_SNs_.begin(),Reverse_SNs_.end(),RSN);
			if(RIt != Reverse_SNs_.end()) {
				Reverse_SNs_.erase(RIt);
			}
		}
	}

	uint64_t Reverse(uint64_t N) {
		uint64_t Res = 0;

		for (int i = 0; i < 16; i++) {
			Res = (Res << 4) + (N & 0x000000000000000f);
			N  >>= 4;
		}
		Res >>= 16;
		return Res;
	}

	void SerialNumberCache::ReturnNumbers(const std::string &S, uint HowMany, const std::vector<uint64_t> &SNArr, std::vector<uint64_t> &A, bool ReverseResult) {
		std::lock_guard G(Mutex_);

		if (S.length() == 12) {
			uint64_t SN = std::stoull(S, nullptr, 16);
			auto It = std::find(SNArr.begin(), SNArr.end(), SN);
			if (It != SNArr.end()) {
				A.push_back(ReverseResult ? Reverse(*It) : *It);
			}
		} else if (S.length() < 12) {
			std::string SS{S};
			SS.insert(SS.end(), 12 - SS.size(), '0');
			uint64_t SN = std::stoull(SS, nullptr, 16);

			auto LB = std::lower_bound(SNArr.begin(), SNArr.end(), SN);
			if (LB != SNArr.end()) {
				for (; LB != SNArr.end() && HowMany; ++LB, --HowMany) {
					if(ReverseResult) {
						const auto TSN = ReverseSerialNumber(Utils::IntToSerialNumber(Reverse(*LB)));
						if (S == TSN.substr(12-S.size())) {
							A.emplace_back(Reverse(*LB));
						} else {
							break;
						}
					} else {
						const auto TSN = Utils::IntToSerialNumber(*LB);
						if (S == TSN.substr(0, S.size())) {
							A.emplace_back(*LB);
						} else {
							break;
						}
					}
				}
			}
		}
	}

	void SerialNumberCache::FindNumbers(const std::string &S, uint HowMany, std::vector<uint64_t> &A) {
		if (S[0] == '*') {
			std::string R;
			std::copy(cbegin(S)+1,cend(S),std::back_inserter(R));
			return ReturnNumbers(R, HowMany, Reverse_SNs_, A, true);
		} else {
			return ReturnNumbers(S, HowMany, SNs_, A, false);
		}
	}
}