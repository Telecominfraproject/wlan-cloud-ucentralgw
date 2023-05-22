//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Array.h"
#include "Poco/JSON/Stringifier.h"

#include "AP_WS_Server.h"
#include "Poco/StringTokenizer.h"
#include "RESTAPI/RESTAPI_device_helper.h"
#include "RESTAPI_devices_handler.h"
#include "StorageService.h"
#include "framework/orm.h"
#include "framework/ow_constants.h"

namespace OpenWifi {

	bool PrepareOrderBy(const std::string &OrderByListRaw, std::string &OrderByString) {
		auto OrderByList = ORM::Escape(OrderByListRaw);
		auto items = Poco::StringTokenizer(OrderByList, ",");
		std::string ItemList;

		Types::StringVec Fields;
		Storage::GetDeviceDbFieldList(Fields);

		std::set<std::string> FieldNames;
		for (const auto &field : Fields)
			FieldNames.insert(Poco::toLower(field));

		for (const auto &i : items) {
			auto T = Poco::StringTokenizer(i, ":");
			if (T.count() != 2) {
				return false;
			}
			if (T[1] != "a" && T[1] != "d") {
				return false;
			}
			if (!ItemList.empty())
				ItemList += " , ";
			auto hint = FieldNames.find(Poco::toLower(T[0]));
			if (hint == FieldNames.end()) {
				return false;
			}
			ItemList += T[0] + (T[1] == "a" ? " ASC" : " DESC");
		}

		if (!ItemList.empty()) {
			OrderByString = " ORDER BY " + ItemList;
		}
		return true;
	}

	void RESTAPI_devices_handler::DoGet() {

		if (GetBoolParameter("connectionStatistics")) {
			GWObjects::DeviceConnectionStatistics DCS;
			Poco::JSON::Object Answer;

			AP_WS_Server()->AverageDeviceStatistics(DCS.connectedDevices, DCS.averageConnectionTime,
													DCS.connectingDevices);
			DCS.to_json(Answer);
			return ReturnObject(Answer);
		}

		if (GetBoolParameter("orderSpec")) {
			Types::StringVec Fields;
			Storage::GetDeviceDbFieldList(Fields);
			std::sort(Fields.begin(), Fields.end());
			Poco::JSON::Object Answer;
			RESTAPI_utils::field_to_json(Answer, "list", Fields);
			return ReturnObject(Answer);
		}

		std::string OrderBy{" ORDER BY serialNumber ASC "}, Arg;
		if (HasParameter("orderBy", Arg)) {
			if (!PrepareOrderBy(Arg, OrderBy)) {
				return BadRequest(RESTAPI::Errors::InvalidLOrderBy);
			}
		}

		auto serialOnly = GetBoolParameter(RESTAPI::Protocol::SERIALONLY, false);
		auto deviceWithStatus = GetBoolParameter(RESTAPI::Protocol::DEVICEWITHSTATUS, false);
		auto completeInfo = GetBoolParameter("completeInfo", false);

		Poco::JSON::Object RetObj;
		if (!QB_.Select.empty()) {
			Poco::JSON::Array Objects;
			for (auto &i : SelectedRecords()) {
				auto SerialNumber = i;
				if (!Utils::ValidSerialNumber(i))
					continue;
				GWObjects::Device D;
				if (StorageService()->GetDevice(SerialNumber, D)) {
					if (completeInfo) {
						Poco::JSON::Object FullDeviceInfo;
						CompleteDeviceInfo(D, FullDeviceInfo);
						Objects.add(FullDeviceInfo);
					} else {
						Poco::JSON::Object Obj;
						if (deviceWithStatus)
							D.to_json_with_status(Obj);
						else
							D.to_json(Obj);
						Objects.add(Obj);
					}
				} else {
					Logger_.error(fmt::format("DEVICE({}): device in select cannot be found.", i));
				}
			}
			if (deviceWithStatus)
				RetObj.set(RESTAPI::Protocol::DEVICESWITHSTATUS, Objects);
			else
				RetObj.set(RESTAPI::Protocol::DEVICES, Objects);

		} else if (QB_.CountOnly == true) {
			uint64_t Count = 0;
			if (StorageService()->GetDeviceCount(Count)) {
				return ReturnCountOnly(Count);
			}
		} else if (serialOnly) {
			std::vector<std::string> SerialNumbers;
			StorageService()->GetDeviceSerialNumbers(QB_.Offset, QB_.Limit, SerialNumbers, OrderBy);
			Poco::JSON::Array Objects;
			for (const auto &i : SerialNumbers) {
				Objects.add(i);
			}
			RetObj.set(RESTAPI::Protocol::SERIALNUMBERS, Objects);
		} else if (GetBoolParameter("health")) {
			auto lowLimit = GetParameter("lowLimit",30);
			auto highLimit = GetParameter("highLimit",80);
			std::vector<std::string>	SerialNumbers;
			AP_WS_Server()->GetHealthDevices(lowLimit,highLimit,SerialNumbers);

			Poco::JSON::Array Objects;
			for(const auto &s:SerialNumbers)
				Objects.add(s);
			RetObj.set("serialNumbers", Objects);
		} else {
			std::vector<GWObjects::Device> Devices;
			StorageService()->GetDevices(QB_.Offset, QB_.Limit, Devices, OrderBy);
			Poco::JSON::Array Objects;
			for (const auto &i : Devices) {
				Poco::JSON::Object Obj;
				if (deviceWithStatus)
					i.to_json_with_status(Obj);
				else
					i.to_json(Obj);
				Objects.add(Obj);
			}
			if (deviceWithStatus)
				RetObj.set(RESTAPI::Protocol::DEVICESWITHSTATUS, Objects);
			else
				RetObj.set(RESTAPI::Protocol::DEVICES, Objects);
		}
		ReturnObject(RetObj);
	}

	static bool ValidMacPatternOnlyChars(const std::string &s) {
		return std::for_each(s.begin(),s.end(),[](const char c) {
			if(c=='%') return true;
			if(c>='0' && c<='9') return true;
			if(c>='a' && c<='f') return true;
			return false;
 		});
	}

	void RESTAPI_devices_handler::DoDelete() {

		if(!RESTAPI_utils::IsRootOrAdmin(UserInfo_.userinfo)) {
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		auto macPattern = GetParameter("macPattern","");
		if(macPattern.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		//	rules out wrong values.
		Poco::toLowerInPlace(macPattern);
		Poco::replaceInPlace(macPattern,"*","%");
		if(!ValidMacPatternOnlyChars(macPattern)) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		auto SimulatedOnly = GetBoolParameter("simulatedOnly",false);
		StorageService()->DeleteDevices(macPattern, SimulatedOnly);
		return OK();
	}

} // namespace OpenWifi