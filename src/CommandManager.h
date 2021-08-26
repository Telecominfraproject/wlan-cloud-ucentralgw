//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UCOMMANDMANAGER_H
#define UCENTRAL_UCOMMANDMANAGER_H

#include <chrono>
#include <future>
#include <map>
#include <utility>
#include <functional>

#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "RESTAPI_GWobjects.h"
#include "SubSystemServer.h"

namespace OpenWifi {

    class CommandManager : public SubSystemServer, Poco::Runnable {
	    public:
			int Start() override;
			void Stop() override;
			void WakeUp();
			void PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj);
			bool SendCommand(	const std::string &SerialNumber,
								 const std::string &Method,
								 const Poco::JSON::Object &Params,
								 std::shared_ptr<std::promise<Poco::JSON::Object::Ptr>> Promise,
								 const std::string &UUID);
			bool SendCommand( 	const std::string & SerialNumber,
								 const std::string & Method,
								 const Poco::JSON::Object &Params,
								 const std::string & UUID);
			bool SendCommand(GWObjects::CommandDetails & Command);
			void Janitor();
			void run() override;

			static CommandManager *instance() {
				if (instance_ == nullptr) {
					instance_ = new CommandManager;
				}
				return instance_;
			}

	    private:
			static CommandManager 		* instance_;
			std::atomic_bool 	Running_ = false;
			Poco::Thread    	ManagerThread;
			uint64_t 			Id_=1;
			std::map< uint64_t , std::pair< std::shared_ptr<std::promise<Poco::JSON::Object::Ptr>>, std::string> >	OutStandingRequests_;
			std::map< uint64_t , uint64_t >		Age_;

			CommandManager() noexcept;
	};

	inline CommandManager * CommandManager() { return CommandManager::instance(); }

}  // namespace

#endif //UCENTRAL_UCOMMANDMANAGER_H
