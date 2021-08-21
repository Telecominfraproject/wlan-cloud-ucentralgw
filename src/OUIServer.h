//
// Created by stephane bourque on 2021-06-17.
//

#ifndef UCENTRALGW_OUISERVER_H
#define UCENTRALGW_OUISERVER_H

#include "SubSystemServer.h"

namespace OpenWifi {

	class OUIServer : public SubSystemServer {
	  public:

		typedef std::map<uint64_t,std::string>	OUIMap;

		static OUIServer *instance() {
			if (instance_ == nullptr) {
				instance_ = new OUIServer;
			}
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void Update();
		void UpdateImpl();
		[[nodiscard]] std::string GetManufacturer(const std::string &MAC);
		[[nodiscard]] bool GetFile(const std::string &FileName);
		[[nodiscard]] bool ProcessFile(const std::string &FileName, OUIMap &Map);

	  private:
		static OUIServer 	* instance_;
		uint64_t 			LastUpdate_ = 0 ;
		bool 				ValidFile_=false;
		OUIMap 				OUIs_;
		std::atomic_bool 	Updating_=false;
		std::atomic_bool 	Running_=false;

		OUIServer() noexcept:
			SubSystemServer("OUIServer", "OUI-SVR", "ouiservr")
		{
		}
	};

	inline OUIServer * OUIServer() { return OUIServer::instance(); }

}

#endif // UCENTRALGW_OUISERVER_H
