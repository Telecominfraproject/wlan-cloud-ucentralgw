//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_FIRMWAREMANAGER_H
#define UCENTRALGW_FIRMWAREMANAGER_H
#include "SubSystemServer.h"

namespace uCentral {

	class FirmwareManager : public SubSystemServer, Poco::Runnable {
	  public:
		struct FirmwareEntry {
			std::string Compatible;
			std::string Uploader;
			std::string Version;
			std::string URI;
			uint64_t 	Uploaded;
			uint64_t 	Size;
			uint64_t 	FirmwareDate;
			bool 		Latest;
		};

		static FirmwareManager *instance() {
			if (instance_ == nullptr) {
				instance_ = new FirmwareManager;
			}
			return instance_;
		}

		void run() override;
		int Start() override;
		void Stop() override;
		bool SetManifest(const std::string & Manifest);
		bool DoUpgrade(const FirmwareEntry &Entry, const std::vector<std::string> &SerialNumbers);
		bool ParseManifest();
		uint64_t CalculateWhen(std::string & SerialNumber);

	  private:
		static FirmwareManager 		*instance_;
		Poco::Thread 		Mgr_;
		std::atomic_bool 	Running_ = false;
		std::string 		Manifest_;
		std::string 		ManifestFileName_;
		std::map<std::string, FirmwareEntry>	Firmwares_;
		std::string 		DefaultPolicy_;

		FirmwareManager() noexcept;
	};

	inline FirmwareManager * FirmwareManager() { return FirmwareManager::instance(); }
}

#endif // UCENTRALGW_FIRMWAREMANAGER_H
