//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_CALLBACKMANAGER_H
#define UCENTRALGW_CALLBACKMANAGER_H

#include <queue>

#include "SubSystemServer.h"

namespace uCentral {

	class CallbackManager : public SubSystemServer, Poco::Runnable {
	  public:
		struct CallBackMessage {
			std::string Message;
			std::string JSONDoc;
		};

		struct CallbackHost {
			std::string Local;
			std::string LocalKey;
			std::string Remote;
			std::string RemoteKey;
			std::string Topics;
			uint64_t 	LastContact;
			uint64_t 	NextContact;
			bool 		Registered;
		};

		static CallbackManager *instance() {
			if (instance_ == nullptr) {
				instance_ = new CallbackManager;
			}
			return instance_;
		}

		void run() override;
		int Start() override;
		void Stop() override;
		bool AddMessage(const CallBackMessage &Msg);
		bool InitHosts();
		bool RegisterHosts();

	  private:
		static CallbackManager *instance_;
		Poco::Thread Mgr_;
		std::atomic_bool Running_ = false;
		std::queue<CallBackMessage> Calls_;
		std::string MyIDCallbackId_;
		std::vector<CallbackHost> Hosts_;

		CallbackManager() noexcept;
	};

	inline CallbackManager * CallbackManager() { return CallbackManager::instance(); }

} // end of namespace

#endif // UCENTRALGW_CALLBACKMANAGER_H
