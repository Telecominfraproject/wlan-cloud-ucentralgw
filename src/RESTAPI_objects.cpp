//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"

#include "RESTAPI_objects.h"
#include "RESTAPI_handler.h"
#include "uDeviceRegistry.h"
#include "uUtils.h"
#include "uCentral.h"

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
		Obj.set("createdTimestamp", uCentral::Utils::to_RFC3339(CreationTimestamp));
		Obj.set("lastConfigurationChange", uCentral::Utils::to_RFC3339(LastConfigurationChange));
		Obj.set("lastConfigurationDownload", uCentral::Utils::to_RFC3339(LastConfigurationDownload));
		Obj.set("lastFWUpdate", uCentral::Utils::to_RFC3339(LastFWUpdate));
		Obj.set("owner", Owner);
		Obj.set("location", Location);
		Obj.set("firmware", Firmware);
		Obj.set("compatible", Compatible);
		Obj.set("fwUpdatePolicy",FWUpdatePolicy);
	}

	void Device::to_json_with_status(Poco::JSON::Object &Obj) const {
		to_json(Obj);
		ConnectionState ConState;
		if (uCentral::DeviceRegistry::GetState(SerialNumber, ConState)) {
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
		Obj.set("recorded", uCentral::Utils::to_RFC3339(Recorded));
	}

	void Capabilities::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("capabilities", Obj, Capabilities);
		Obj.set("firstUpdate", uCentral::Utils::to_RFC3339(FirstUpdate));
		Obj.set("lastUpdate", uCentral::Utils::to_RFC3339(LastUpdate));
	}

	void DeviceLog::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("log", Log);
		Obj.set("severity", Severity);
		EmbedDocument("data", Obj, Data);
		Obj.set("recorded", uCentral::Utils::to_RFC3339(Recorded));
		Obj.set("logType", LogType);
		Obj.set("UUID", UUID);
	}

	void HealthCheck::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("UUID", UUID);
		EmbedDocument("values", Obj, Data);
		Obj.set("sanity", Sanity);
		Obj.set("recorded", uCentral::Utils::to_RFC3339(Recorded));
	}

	void DefaultConfiguration::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("name", Name);
		Obj.set("modelIds", Models);
		Obj.set("description", Description);
		EmbedDocument("configuration", Obj, Configuration);
		Obj.set("created", uCentral::Utils::to_RFC3339(Created));
		Obj.set("lastModified", uCentral::Utils::to_RFC3339(LastModified));
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
		Obj.set("submitted", uCentral::Utils::to_RFC3339(Submitted));
		Obj.set("executed", uCentral::Utils::to_RFC3339(Executed));
		Obj.set("completed", uCentral::Utils::to_RFC3339(Completed));
		Obj.set("when", uCentral::Utils::to_RFC3339(RunAt));
		Obj.set("errorCode", ErrorCode);
		Obj.set("custom", Custom);
		Obj.set("waitingForFile", WaitingForFile);
		Obj.set("attachFile", uCentral::Utils::to_RFC3339(AttachDate));
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
		Obj.set("created", uCentral::Utils::to_RFC3339(Created));
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
		Obj.set("lastContact", uCentral::Utils::to_RFC3339(LastContact));
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
		Obj.set("created",uCentral::Utils::to_RFC3339(created_));
		Obj.set("username",username_);
		Obj.set("aclTemplate",AclTemplateObj);
	}

	void PendingFirmwareUpgrade::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("serialNumber", SerialNumber);
		Obj.set("commandUUID", CommandUUID);
		Obj.set("newFirmware", NewFirmware);
		Obj.set("oldFirmware",OldFirmware);
		Obj.set("uti",URI);
		Obj.set("scheduledAt", uCentral::Utils::to_RFC3339(ScheduledAt));
		Obj.set("created", uCentral::Utils::to_RFC3339(Created));
		Obj.set("updateDone",uCentral::Utils::to_RFC3339(UpdateDone));
	};

	void RttySessionDetails::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("serialNumber", SerialNumber);
		Obj.set("server", Server);
		Obj.set("port", Port);
		Obj.set("token",Token);
		Obj.set("timeout", TimeOut);
		Obj.set("connectionId",ConnectionId);
		Obj.set("commandUUID",CommandUUID);
		Obj.set("started", uCentral::Utils::to_RFC3339(Started));
		Obj.set("viewport",ViewPort);
	}
}

