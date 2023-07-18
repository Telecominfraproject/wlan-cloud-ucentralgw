//
// Created by stephane bourque on 2023-07-12.
//

#pragma once

#include "framework/SubSystemServer.h"
#include "Poco/ExpireLRUCache.h"
#include "sdks/sdk_fms.h"

namespace OpenWifi {
	class FirmwareRevisionCache : public SubSystemServer {
	  public:
		static auto instance() {
			static auto instance_ = new FirmwareRevisionCache;
			return instance_;
		}

		inline int Start() override {
			poco_notice(Logger(), "Starting...");
			return 0;
		}

		inline void Stop() override {
			poco_notice(Logger(), "Stopping...");
			poco_notice(Logger(), "Stopped...");
		}

		inline bool DeviceMustUpgrade([[maybe_unused]] std::string &deviceType,
									  [[maybe_unused]] const std::string &firmware_string,
									  [[maybe_unused]] GWObjects::DefaultFirmware &Firmware) {
			return false;
			if(StorageService()->GetDefaultFirmware(deviceType,Firmware)) {

				std::string	key{ deviceType + Firmware.revision };

				if(!Cache_.has(key)) {
					FMSObjects::FirmwareAgeDetails	FAD;
					if(SDK::FMS::GetFirmwareAge(deviceType,Firmware.revision,FAD,Logger())) {
						Cache_.add(key,FAD);
					} else {
						//  if we cannot establish the age of the currently running firmware,
						//	then we assume it is too old.
						return true;
					}
				}
				auto FAD = Cache_.get(key);
				if(FAD->imageDate < Firmware.imageCreationDate) {
					return true;
				}
			}
			return false;
		}

	  private:
		Poco::ExpireLRUCache<std::string, FMSObjects::FirmwareAgeDetails> Cache_{
			512, 1200000};

		FirmwareRevisionCache() noexcept
			: SubSystemServer("FirmwareRevisionCache", "FWCACHE-SVR", "firmwarecache") {
		}
	};

	inline auto FirmwareRevisionCache() { return FirmwareRevisionCache::instance(); }

} // namespace OpenWifi
