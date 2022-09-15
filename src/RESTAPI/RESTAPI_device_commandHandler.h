//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "framework/MicroService.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	class RESTAPI_device_commandHandler : public RESTAPIHandler {
	  public:
		RESTAPI_device_commandHandler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, uint64_t TransactionId, bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{
								 Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
								 Poco::Net::HTTPRequest::HTTP_DELETE,
								 Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server,
							 TransactionId,
							 Internal) {}

		void GetCapabilities();
		void DeleteCapabilities();
		void GetLogs();
		void DeleteLogs();
		void GetStatistics();
		void DeleteStatistics();
		void GetStatus();
		void GetChecks();
		void DeleteChecks();
		void ExecuteCommand(const std::string &UUID, uint64_t RPC);
		void Configure(const std::string &UUID, uint64_t RPC);
		void Upgrade(const std::string &UUID, uint64_t RPC);
		void Reboot(const std::string &UUID, uint64_t RPC);
		void Factory(const std::string &UUID, uint64_t RPC);
		void LEDs(const std::string &UUID, uint64_t RPC);
		void Trace(const std::string &UUID, uint64_t RPC);
		void MakeRequest(const std::string &UUID, uint64_t RPC);
		void WifiScan(const std::string &UUID, uint64_t RPC);
		void EventQueue(const std::string &UUID, uint64_t RPC);
		void Rtty(const std::string &UUID, uint64_t RPC);
		void Telemetry(const std::string &UUID, uint64_t RPC);
		void Ping(const std::string &UUID, uint64_t RPC);
		void Script(const std::string &UUID, uint64_t RPC);

		static auto PathName() { return std::list<std::string>{"/api/v1/device/{serialNumber}/{command}"}; };
		void DoGet() final;
		void DoDelete() final;
		void DoPost() final;
		void DoPut() final {};

		void CallCanceled(const char * Cmd,const std::string &UUID, uint64_t RPC, const OpenWifi::RESTAPI::Errors::msg & Err);
		void CallCanceled(const char * Cmd, const OpenWifi::RESTAPI::Errors::msg &Err);

		inline bool ValidateParameters() {
			Command_ =  GetBinding(RESTAPI::Protocol::COMMAND, "");
			if (Command_.empty()) {
				return false;
			}
			SerialNumber_ = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");
			if (SerialNumber_.empty()) {
				return false;
			}
			return true;
		}

	  private:
		std::string 	SerialNumber_,
						Command_;
		std::uint64_t 	SerialNumberInt_=0;
	};
}
