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
#include <shared_mutex>

#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Timer.h"
#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"

#include "framework/SubSystemServer.h"
#include "fmt/format.h"

#include "RESTObjects/RESTAPI_GWobjects.h"

namespace OpenWifi {

	class RPCResponseNotification: public Poco::Notification {
	  public:
		RPCResponseNotification(std::uint64_t ser,
								Poco::JSON::Object::Ptr pl) :
 			SerialNumber_(ser),
			Payload_(std::move(pl))
		{

		}
		std::uint64_t 				SerialNumber_;
		Poco::JSON::Object::Ptr		Payload_;
	};


	class CommandManager : public SubSystemServer, Poco::Runnable {
	    public:
		  	using objtype_t = Poco::JSON::Object::Ptr;
		  	using promise_type_t = std::promise<objtype_t>;

			struct CommandInfo {
				std::uint64_t 			Id=0;
				std::uint64_t 			SerialNumber=0;
				APCommands::Commands 	Command;
				std::string 			UUID;
				std::uint64_t 			State=1;
				std::chrono::time_point<std::chrono::high_resolution_clock> submitted = std::chrono::high_resolution_clock::now();
				std::shared_ptr<promise_type_t> rpc_entry;
				bool 					Deferred=false;
			};

			struct RPCResponse {
				std::uint64_t 				serialNumber;
				Poco::JSON::Object::Ptr		payload;

				explicit RPCResponse(std::uint64_t ser, Poco::JSON::Object::Ptr pl)
					:
						serialNumber(ser),
						payload(std::move(pl)) {
				}
			};

			int Start() override;
			void Stop() override;
			void WakeUp();
			inline void PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj) {
				ResponseQueue_.enqueueNotification(new RPCResponseNotification(Utils::SerialNumberToInt(SerialNumber),std::move(Obj)));
			}

			std::shared_ptr<promise_type_t> PostCommandOneWayDisk(uint64_t RPC_ID,
				APCommands::Commands Command,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool & Sent) {
					return 	PostCommand(RPC_ID,
								   Command,
								   SerialNumber,
									Method,
									Params,
									UUID,
								   	true, true, Sent , false);
			}

			std::shared_ptr<promise_type_t> PostCommandDisk(
				uint64_t RPC_ID,
				APCommands::Commands Command,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool & Sent) {
					return 	PostCommand(RPC_ID,
								   Command,
								   SerialNumber,
								   Method,
								   Params,
								   UUID,
								   false, true, Sent,
								   false );
			}

			std::shared_ptr<promise_type_t> PostCommand(
				uint64_t RPC_ID,
				APCommands::Commands Command,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool & Sent,
				bool rpc,
				bool Deferred) {
					return 	PostCommand(RPC_ID,
								   Command,
								   SerialNumber,
								   Method,
								   Params,
								   UUID,
								   false,
								   false, Sent,
								   rpc,
								   Deferred);
			}

			std::shared_ptr<promise_type_t> PostCommandOneWay(
				uint64_t RPC_ID,
				APCommands::Commands Command,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool & Sent) {
					return 	PostCommand(RPC_ID,
								   Command,
								   SerialNumber,
								   Method,
								   Params,
								   UUID,
								   true,
								   false, Sent,
								   false);
			}

			bool IsCommandRunning(const std::string &C);

			void run() override;

			static auto instance() {
			    static auto instance_ = new CommandManager;
				return instance_;
			}

			inline bool Running() const { return Running_; }
			void onJanitorTimer(Poco::Timer & timer);
			void onCommandRunnerTimer(Poco::Timer & timer);
			inline uint64_t Next_RPC_ID() { return ++Id_; }

			void RemovePendingCommand(std::uint64_t Id) {
				std::unique_lock	Lock(LocalMutex_);
				OutStandingRequests_.erase(Id);
			}

			inline bool CommandRunningForDevice(std::uint64_t SerialNumber, std::string & uuid, APCommands::Commands &command) {
				std::lock_guard	Lock(LocalMutex_);

				for(const auto &[Request,Command]:OutStandingRequests_) {
					if(Command.SerialNumber==SerialNumber) {
						uuid = Command.UUID;
						command = Command.Command;
						return true;
					}
				}
				return false;
			}

			inline void ClearQueue(std::uint64_t SerialNumber) {
				std::lock_guard	Lock(LocalMutex_);
				for(auto Request = OutStandingRequests_.begin(); Request != OutStandingRequests_.end() ; ) {
					if(Request->second.SerialNumber==SerialNumber)
						Request = OutStandingRequests_.erase(Request);
					else
						++Request;
				}
			}

			inline auto CommandTimeout() const { return commandTimeOut_; }
			inline auto CommandRetry() const { return commandRetry_; }

	    private:
		  	mutable std::recursive_mutex			LocalMutex_;
			std::atomic_bool 						Running_ = false;
			Poco::Thread    						ManagerThread;
			std::atomic_uint64_t 					Id_=3;	//	do not start @1. We ignore ID=1 & 0 is illegal..
			std::map<std::uint64_t , CommandInfo>	OutStandingRequests_;
			Poco::Timer                     		JanitorTimer_;
			std::unique_ptr<Poco::TimerCallback<CommandManager>>   JanitorCallback_;
			Poco::Timer                     		CommandRunnerTimer_;
			std::unique_ptr<Poco::TimerCallback<CommandManager>>   CommandRunnerCallback_;
			Poco::NotificationQueue					ResponseQueue_;
			std::uint64_t 							commandTimeOut_=0;
			std::uint64_t 							commandRetry_=0;
			std::uint64_t 							janitorInterval_=0;
			std::uint64_t 							queueInterval_=0;

			std::shared_ptr<promise_type_t> PostCommand(
				uint64_t RPCID,
				APCommands::Commands Command,
				const std::string &SerialNumber,
				const std::string &Method,
				const Poco::JSON::Object &Params,
				const std::string &UUID,
				bool oneway_rpc,
				bool disk_only,
				bool & Sent,
				bool rpc_call,
				bool Deferred=false);

			bool CompleteScriptCommand(CommandInfo &Command, const Poco::JSON::Object::Ptr &Payload, std::chrono::duration<double, std::milli> rpc_execution_time);
			bool CompleteTelemetryCommand(CommandInfo &Command, const Poco::JSON::Object::Ptr &Payload, std::chrono::duration<double, std::milli> rpc_execution_time);

			CommandManager() noexcept:
				SubSystemServer("CommandManager", "CMD-MGR", "command.manager") {
			}
	};

	inline auto CommandManager() { return CommandManager::instance(); }

}  // namespace

