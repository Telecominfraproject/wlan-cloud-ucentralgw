//
// Created by stephane bourque on 2023-05-23.
//

#include "AP_WS_ConfigAutoUpgrader.h"
#include <framework/utils.h>
#include <RESTObjects/RESTAPI_GWobjects.h>
#include <StorageService.h>

namespace OpenWifi {

	int AP_WS_ConfigAutoUpgradeAgent::Start() {
		poco_notice(Logger(), "Starting...");
		QueueManager_.start(*this);

		return 0;
	}

	void AP_WS_ConfigAutoUpgradeAgent::Stop() {
		poco_notice(Logger(), "Stopping...");
		Running_ = false;
		Queue_.wakeUpAll();
		QueueManager_.join();
		poco_notice(Logger(), "Stopped...");
	}

	void AP_WS_ConfigAutoUpgradeAgent::run() {
		Utils::SetThreadName("auto:cfgmgr");
		Running_ = true;

		while (Running_) {
			Poco::AutoPtr<Poco::Notification> NextMsg(Queue_.waitDequeueNotification());
			try {
				auto Entry = dynamic_cast<CheckConfiguration *>(NextMsg.get());
				if (Entry != nullptr) {
					GWObjects::Device DeviceInfo;
					std::string SerialNumber = Utils::IntToSerialNumber(Entry->serial_);
					if (StorageService()->GetDevice(SerialNumber, DeviceInfo)) {
						if(DeviceInfo.pendingUUID!=0 && Entry->uuid_==DeviceInfo.pendingUUID) {
							StorageService()->CompleteDeviceConfigurationChange(SerialNumber);
							SetDeviceCacheEntry(Entry->serial_, Utils::Now(), Entry->uuid_, 0);
							continue;
						}
						if(DeviceInfo.UUID==Entry->uuid_) {
							SetDeviceCacheEntry(Entry->serial_, Utils::Now(), Entry->uuid_, 0);
							continue;
						}
					}
				}
				return;
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			} catch (...) {
				poco_warning(Logger(), "Exception occurred during run.");
			}
		}
	}

} // namespace OpenWifi