//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "framework/MicroService.h"

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
		void ExecuteCommand();
		void Configure();
		void GetChecks();
		void DeleteChecks();
		void Upgrade();
		void Reboot();
		void Factory();
		void LEDs();
		void Trace();
		void LogDump();
		void MakeRequest();
		void WifiScan();
		void EventQueue();
		void Rtty();
		void Telemetry();
		void Ping();
		void Script();

		static auto PathName() { return std::list<std::string>{"/api/v1/device/{serialNumber}/{command}"}; };
		void DoGet() final;
		void DoDelete() final;
		void DoPost() final;
		void DoPut() final {};

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
		std::string SerialNumber_, Command_;
	};
}
