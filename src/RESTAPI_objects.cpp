//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"

#include "Daemon.h"
#include "DeviceRegistry.h"
#include "RESTAPI_handler.h"
#include "RESTAPI_objects.h"
#include "Utils.h"

namespace uCentral::Objects {

	void EmbedDocument(const std::string & ObjName, Poco::JSON::Object & Obj, const std::string &ObjStr) {
		std::string D = ObjStr.empty() ? "{}" : ObjStr;
		Poco::JSON::Parser P;
		Poco::Dynamic::Var result = P.parse(D);
		const auto &DetailsObj = result.extract<Poco::JSON::Object::Ptr>();
		Obj.set(ObjName, DetailsObj);
	}

	void Device::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("serialNumber", SerialNumber);
		Obj.set("deviceType", uCentral::Daemon::instance()->IdentifyDevice(Compatible));
		Obj.set("macAddress", MACAddress);
		Obj.set("manufacturer", Manufacturer);
		Obj.set("UUID", UUID);

		EmbedDocument("configuration", Obj, Configuration);

		Obj.set("notes", Notes);
		Obj.set("createdTimestamp", CreationTimestamp);
		Obj.set("lastConfigurationChange", LastConfigurationChange);
		Obj.set("lastConfigurationDownload", LastConfigurationDownload);
		Obj.set("lastFWUpdate", LastFWUpdate);
		Obj.set("owner", Owner);
		Obj.set("location", Location);
		Obj.set("location", Venue);
		Obj.set("firmware", Firmware);
		Obj.set("compatible", Compatible);
		Obj.set("fwUpdatePolicy",FWUpdatePolicy);
		Obj.set("devicePassword",DevicePassword);
	}

	void Device::to_json_with_status(Poco::JSON::Object &Obj) const {
		to_json(Obj);
		ConnectionState ConState;
		if (DeviceRegistry()->GetState(SerialNumber, ConState)) {
			ConState.to_json(Obj);
		} else {
			Obj.set("ipAddress", "N/A");
			Obj.set("txBytes", 0);
			Obj.set("rxBytes", 0);
			Obj.set("messageCount", 0);
			Obj.set("connected", false);
			Obj.set("lastContact", "N/A");
			Obj.set("verifiedCertificate", "NO_CERTIFICATE");
		}
	}

	bool Device::from_json(Poco::JSON::Object::Ptr Obj) {

		try {
			Poco::DynamicStruct ds = *Obj;

			SerialNumber = ds["serialNumber"].toString();
			DeviceType = ds["deviceType"].toString();
			MACAddress = ds["macAddress"].toString();
			UUID = ds["UUID"];
			Configuration = ds["configuration"].toString();
			if (ds.contains("notes"))
				Notes = ds["notes"].toString();
			if (ds.contains("manufacturer"))
				Manufacturer = ds["manufacturer"].toString();
			if (ds.contains("owner"))
				Owner = ds["owner"].toString();
			if (ds.contains("location"))
				Location = ds["location"].toString();
			if (ds.contains("venue"))
				Owner = ds["venue"].toString();
			if (ds.contains("compatible"))
				Compatible = ds["compatible"].toString();
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void Device::Print() const {
		std::cout << "Device: " << SerialNumber << " " << DeviceType << " " << MACAddress << " "
				  << Manufacturer << " " << Configuration << std::endl;
	}

	void Statistics::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("data", Obj, Data);
		Obj.set("UUID", UUID);
		Obj.set("recorded", Recorded);
	}

	void Capabilities::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("capabilities", Obj, Capabilities);
		Obj.set("firstUpdate", FirstUpdate);
		Obj.set("lastUpdate", LastUpdate);
	}

	void DeviceLog::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("log", Log);
		Obj.set("severity", Severity);
		EmbedDocument("data", Obj, Data);
		Obj.set("recorded", Recorded);
		Obj.set("logType", LogType);
		Obj.set("UUID", UUID);
	}

	void HealthCheck::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("UUID", UUID);
		EmbedDocument("values", Obj, Data);
		Obj.set("sanity", Sanity);
		Obj.set("recorded", Recorded);
	}

	void DefaultConfiguration::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("name", Name);
		Obj.set("modelIds", Models);
		Obj.set("description", Description);
		EmbedDocument("configuration", Obj, Configuration);
		Obj.set("created", Created);
		Obj.set("lastModified", LastModified);
	}

	void CommandDetails::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("UUID", UUID);
		Obj.set("serialNumber", SerialNumber);
		Obj.set("command", Command);
		EmbedDocument("details", Obj, Details);
		EmbedDocument("results", Obj, Results);
		Obj.set("errorText", ErrorText);
		Obj.set("submittedBy", SubmittedBy);
		Obj.set("status", Status);
		Obj.set("submitted", Submitted);
		Obj.set("executed", Executed);
		Obj.set("completed", Completed);
		Obj.set("when", RunAt);
		Obj.set("errorCode", ErrorCode);
		Obj.set("custom", Custom);
		Obj.set("waitingForFile", WaitingForFile);
		Obj.set("attachFile", AttachDate);
	}

	bool DefaultConfiguration::from_json(Poco::JSON::Object::Ptr Obj) {
		Poco::DynamicStruct ds = *Obj;

		try {
			Name = ds["name"].toString();
			Configuration = ds["configuration"].toString();
			Models = ds["modelIds"].toString();
			if (ds.contains("description"))
				Description = ds["description"].toString();
			return true;
		} catch (const Poco::Exception &E) {
		}

		return false;
	}

	void BlackListedDevice::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("serialNumber", SerialNumber);
		Obj.set("author", Author);
		Obj.set("reason", Reason);
		Obj.set("created", Created);
	}

	void ConnectionState::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("serialNumber", SerialNumber);
		Obj.set("ipAddress", Address);
		Obj.set("txBytes", TX);
		Obj.set("rxBytes", RX);
		Obj.set("messageCount", MessageCount);
		Obj.set("UUID", UUID);
		Obj.set("connected", Connected);
		Obj.set("firmware", Firmware);
		Obj.set("lastContact", LastContact);
		switch(VerifiedCertificate) {
		case NO_CERTIFICATE:
			Obj.set("verifiedCertificate", "NO_CERTIFICATE"); break;
		case VALID_CERTIFICATE:
			Obj.set("verifiedCertificate", "VALID_CERTIFICATE"); break;
		case MISMATCH_SERIAL:
			Obj.set("verifiedCertificate", "MISMATCH_SERIAL"); break;
		case VERIFIED:
			Obj.set("verifiedCertificate", "VERIFIED"); break;
		default:
			Obj.set("verifiedCertificate", "NO_CERTIFICATE"); break;
		}
	}

	void AclTemplate::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("Read",Read_);
		Obj.set("ReadWrite",ReadWrite_);
		Obj.set("ReadWriteCreate",ReadWriteCreate_);
		Obj.set("Delete",Delete_);
		Obj.set("PortalLogin",PortalLogin_);
	}

	void WebToken::to_json(Poco::JSON::Object & Obj) const {
		Poco::JSON::Object  AclTemplateObj;
		acl_template_.to_json(AclTemplateObj);
		Obj.set("access_token",access_token_);
		Obj.set("refresh_token",refresh_token_);
		Obj.set("token_type",token_type_);
		Obj.set("expires_in",expires_in_);
		Obj.set("idle_timeout",idle_timeout_);
		Obj.set("created",created_);
		Obj.set("username",username_);
		Obj.set("aclTemplate",AclTemplateObj);
	}

	void PendingFirmwareUpgrade::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("serialNumber", SerialNumber);
		Obj.set("commandUUID", CommandUUID);
		Obj.set("newFirmware", NewFirmware);
		Obj.set("oldFirmware",OldFirmware);
		Obj.set("uti",URI);
		Obj.set("scheduledAt", ScheduledAt);
		Obj.set("created", Created);
		Obj.set("updateDone",UpdateDone);
	};

	void RttySessionDetails::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("serialNumber", SerialNumber);
		Obj.set("server", Server);
		Obj.set("port", Port);
		Obj.set("token",Token);
		Obj.set("timeout", TimeOut);
		Obj.set("connectionId",ConnectionId);
		Obj.set("commandUUID",CommandUUID);
		Obj.set("started", Started);
		Obj.set("viewport",ViewPort);
		Obj.set("password",DevicePassword);
	}
}

