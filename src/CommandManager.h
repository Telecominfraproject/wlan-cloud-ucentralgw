//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <chrono>
#include <future>
#include <map>
#include <utility>
#include <functional>

#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Timer.h"

#include "RESTObjects/RESTAPI_GWobjects.h"
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

	class RPCResponseNotification: public Poco::Notification {
	  public:
		RPCResponseNotification(const std::string &ser,
								const Poco::JSON::Object &pl) :
 			SerialNumber_(ser),
			Payload_(pl)
		{

		}
		std::string			SerialNumber_;
		Poco::JSON::Object	Payload_;
	};


	class CommandManager : public SubSystemServer, Poco::Runnable {
	    public:
		  	typedef Poco::JSON::Object 		objtype_t;
		  	typedef std::promise<objtype_t> promise_type_t;
			struct RpcObject {
				std::string uuid;
				std::chrono::time_point<std::chrono::high_resolution_clock> submitted = std::chrono::high_resolution_clock::now();
				std::shared_ptr<promise_type_t> rpc_entry;
			};

			struct RPCResponse {
				std::string 			serialNumber;
				Poco::JSON::Object		payload;

				explicit RPCResponse(const std::string &ser, const Poco::JSON::Object &pl)
					:
						serialNumber(ser),
						payload(pl) {
				}
			};

			int Start() override;
			void Stop() override;
			void WakeUp();
			inline void PostCommandResult(const std::string &SerialNumber, const Poco::JSON::Object &Obj) {
				std::lock_guard		G(Mutex_);
				// RPCResponseQueue_->Write(RPCResponse{.serialNumber=SerialNumber, .payload = Obj});
				ResponseQueue_.enqueueNotification(new RPCResponseNotification(SerialNumber,Obj));
			}

			std::shared_ptr<promise_type_t> PostCommandOneWayDisk(uint64_t RPCID,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool & Sent) {
					return 	PostCommand(RPCID, SerialNumber,
									Method,
									Params,
									UUID,
								   	true, true, Sent );
			}

			std::shared_ptr<promise_type_t> PostCommandDisk(
				uint64_t RPCID,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool & Sent) {
					return 	PostCommand(RPCID,
								   SerialNumber,
								   Method,
								   Params,
								   UUID,
								   false, true, Sent  );
			}

			std::shared_ptr<promise_type_t> PostCommand(
				uint64_t RPCID,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool & Sent) {
					return 	PostCommand(RPCID, SerialNumber,
								   Method,
								   Params,
								   UUID,
								   false,
								   false, Sent );
			}

			std::shared_ptr<promise_type_t> PostCommandOneWay(
				uint64_t RPCID,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool & Sent) {
					return 	PostCommand(RPCID,
								   SerialNumber,
								   Method,
								   Params,
								   UUID,
								   true,
								   false, Sent  );
			}

			void run() override;

			static auto instance() {
			    static auto instance_ = new CommandManager;
				return instance_;
			}

			inline bool Running() const { return Running_; }
			void onJanitorTimer(Poco::Timer & timer);
			void onCommandRunnerTimer(Poco::Timer & timer);
			void onRPCAnswer(bool& b);
			inline uint64_t NextRPCId() { return ++Id_; }
			void RemovePendingCommand(std::uint64_t Id) {
				std::lock_guard	G(Mutex_);

				for(auto hint=OutStandingRequests_.begin();hint!=OutStandingRequests_.end();++hint) {
					if(hint->first.Id==Id) {
						OutStandingRequests_.erase(hint);
						return;
					}
				}
			}

	    private:
			std::atomic_bool 						Running_ = false;
			Poco::Thread    						ManagerThread;
			std::atomic_uint64_t 					Id_=3;	//	do not start @1. We ignore ID=1 & 0 is illegal..
			std::map<CommandTagIndex,std::shared_ptr<RpcObject>>		OutStandingRequests_;
			std::set<std::string>					OutstandingUUIDs_;
			Poco::Timer                     		JanitorTimer_;
			std::unique_ptr<Poco::TimerCallback<CommandManager>>   JanitorCallback_;
			Poco::Timer                     		CommandRunnerTimer_;
			std::unique_ptr<Poco::TimerCallback<CommandManager>>   CommandRunnerCallback_;
			Poco::NotificationQueue					ResponseQueue_;

			std::shared_ptr<promise_type_t> PostCommand(
				uint64_t RPCID,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool oneway_rpc,
				bool disk_only,
				bool & Sent);

			CommandManager() noexcept:
				SubSystemServer("CommandManager", "CMD-MGR", "command.manager") {
			}
	};

	inline auto CommandManager() { return CommandManager::instance(); }

}  // namespace

