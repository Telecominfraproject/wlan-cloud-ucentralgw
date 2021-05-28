//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_UCALLBACKMANAGER_H
#define UCENTRALGW_UCALLBACKMANAGER_H

#include <queue>

#include "uSubSystemServer.h"

namespace uCentral::CallbackManager {

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

	int Start();
	void Stop();
	bool AddMessage(const CallBackMessage &Msg);

	class Service : public uSubSystemServer, Poco::Runnable {
	  public:
		Service() noexcept;

		friend int Start();
		friend void Stop();
		friend bool AddMessage(const CallBackMessage &Msg);

		static Service *instance() {
			if (instance_ == nullptr) {
				instance_ = new Service;
			}
			return instance_;
		}

		void run() override;

	  private:
		static Service *instance_;
		Poco::Thread Mgr_;
		std::atomic_bool Running_ = false;
		std::queue<CallBackMessage> Calls_;
		std::string MyIDCallbackId_;
		std::vector<CallbackHost> Hosts_;

		int Start() override;
		void Stop() override;
		bool AddMessage(const CallBackMessage &Msg);
		bool InitHosts();
		bool RegisterHosts();
	};

} // end of namespace

#endif // UCENTRALGW_UCALLBACKMANAGER_H
