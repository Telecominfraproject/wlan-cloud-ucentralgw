//
// Created by stephane bourque on 2021-05-12.
//

#ifndef UCENTRALGW_UFIRMWAREMANAGER_H
#define UCENTRALGW_UFIRMWAREMANAGER_H
#include "uSubSystemServer.h"

namespace uCentral::FirmwareManager {
	int Start();
	void Stop();
	bool SetManifest(const std::string & Manifest);

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

	class Service : public uSubSystemServer, Poco::Runnable {
	  public:
		Service() noexcept;

		friend int Start();
		friend void Stop();
		friend bool SetManifest(const std::string & Manifest);

		static Service *instance() {
			if (instance_ == nullptr) {
				instance_ = new Service;
			}
			return instance_;
		}

		void run() override;

	  private:
		static Service 		*instance_;
		Poco::Thread 		Mgr_;
		std::atomic_bool 	Running_ = false;
		std::string 		Manifest_;
		std::string 		ManifestFileName_;
		std::map<std::string, FirmwareEntry>	Firmwares_;
		std::string 		DefaultPolicy_;

		int Start() override;
		void Stop() override;
		bool SetManifest(const std::string & Manifest);
		bool DoUpgrade(const FirmwareEntry &Entry, const std::vector<std::string> &SerialNumbers);
		bool ParseManifest();
		uint64_t CalculateWhen(std::string & SerialNumber);
	};

}

#endif // UCENTRALGW_UFIRMWAREMANAGER_H
