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

#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/MicroService.h"

namespace OpenWifi {

	struct CommandTagIndex {
		uint64_t 	Id=0;
		std::string SerialNumber;
	};

	inline bool operator <(const CommandTagIndex& lhs, const CommandTagIndex& rhs) {
		if(lhs.Id<rhs.Id)
			return true;
		if(lhs.Id>rhs.Id)
			return false;
		return lhs.SerialNumber<rhs.SerialNumber;
	}

	inline bool operator ==(const CommandTagIndex& lhs, const CommandTagIndex& rhs) {
		if(lhs.Id == rhs.Id && lhs.SerialNumber == rhs.SerialNumber)
			return true;
		return false;
	}

	struct CommandTag {
		std::string 			UUID;
		Poco::JSON::Object::Ptr Result;
		uint64_t 				Submitted=0;
		uint64_t 				Completed=0;
	};

    class CommandManager : public SubSystemServer, Poco::Runnable {
	    public:
			int Start() override;
			void Stop() override;
			void WakeUp();
			void PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj);
			bool SendCommand(	const std::string &SerialNumber,
								const std::string &Method,
								const Poco::JSON::Object &Params,
								const std::string &UUID,
							 	uint64_t & Id,
							 	bool oneway_rpc=false);
			void Janitor();
			void run() override;

			bool GetCommand(uint64_t Id, const std::string & SerialNumber, CommandTag &T);

			static CommandManager *instance() {
			    static CommandManager *instance_ = new CommandManager;
				return instance_;
			}
			inline bool Running() const { return Running_; }

	    private:
			std::atomic_bool 			Running_ = false;
			Poco::Thread    			ManagerThread;
			uint64_t 					Id_=2;	//	do not start @1. We ignore ID=1 & 0 is illegal..
			std::map<CommandTagIndex,CommandTag>	OutStandingRequests_;

			CommandManager() noexcept:
				SubSystemServer("CommandManager", "CMD-MGR", "command.manager")
				{
				}
	};

	inline CommandManager * CommandManager() { return CommandManager::instance(); }

}  // namespace

#endif //UCENTRAL_UCOMMANDMANAGER_H
