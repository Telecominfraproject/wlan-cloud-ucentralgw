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

#include "uSubSystemServer.h"
#include "RESTAPI_objects.h"

namespace uCentral::CommandManager {

    int Start();
    void Stop();
    void WakeUp();
	void PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj);
	bool SendCommand(	const std::string &SerialNumber,
						 const std::string &Method,
						 const Poco::JSON::Object &Params,
						 std::shared_ptr<std::promise<Poco::JSON::Object::Ptr>> Promise,
						 const std::string &UUID);
	bool SendCommand(uCentral::Objects::CommandDetails & Command);
	void Janitor();

	class Manager : public Poco::Runnable {
		public:
			explicit Manager(Poco::Logger & Logger):
				Logger_(Logger)
			{}
			void run() override;
			void stop() { Stop_ = true; }
		private:
			bool 			Stop_ = false;
			Poco::Logger    & Logger_;
		};

    class Service : public uSubSystemServer {
	    public:
			Service() noexcept;

			friend int Start();
			friend void Stop();
			friend void WakeUp();
			friend void PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj);
			friend bool SendCommand(	const std::string &SerialNumber,
										const std::string &Method,
										const Poco::JSON::Object &Params,
										std::shared_ptr<std::promise<Poco::JSON::Object::Ptr>> Promise,
										const std::string &UUID);
			friend bool SendCommand( 	const std::string & SerialNumber,
								 const std::string & Method,
								 const Poco::JSON::Object &Params,
								 const std::string & UUID);
			friend bool SendCommand(uCentral::Objects::CommandDetails & Command);
			friend void Janitor();

			static Service *instance() {
				if (instance_ == nullptr) {
					instance_ = new Service;
				}
				return instance_;
			}

	    private:
			static Service *instance_;
			Manager         Manager_;
			Poco::Thread    ManagerThread;
			uint64_t 		Id_=1;

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
			bool SendCommand(uCentral::Objects::CommandDetails & Command);
			void Janitor();

		std::map< uint64_t , std::pair< std::shared_ptr<std::promise<Poco::JSON::Object::Ptr>>, std::string> >	OutStandingRequests_;
		std::map< uint64_t , uint64_t >		Age_;
	};

}  // namespace

#endif //UCENTRAL_UCOMMANDMANAGER_H
