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
#include "RESTAPI_utils.h"

using uCentral::RESTAPI_utils::field_to_json;
using uCentral::RESTAPI_utils::field_from_json;
using uCentral::RESTAPI_utils::EmbedDocument;

namespace uCentral::Objects {

	void Device::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"serialNumber", SerialNumber);
		field_to_json(Obj,"deviceType", uCentral::Daemon::instance()->IdentifyDevice(Compatible));
		field_to_json(Obj,"macAddress", MACAddress);
		field_to_json(Obj,"manufacturer", Manufacturer);
		field_to_json(Obj,"UUID", UUID);
		EmbedDocument("configuration", Obj, Configuration);

		field_to_json(Obj,"notes", Notes);
		field_to_json(Obj,"createdTimestamp", CreationTimestamp);
		field_to_json(Obj,"lastConfigurationChange", LastConfigurationChange);
		field_to_json(Obj,"lastConfigurationDownload", LastConfigurationDownload);
		field_to_json(Obj,"lastFWUpdate", LastFWUpdate);
		field_to_json(Obj,"owner", Owner);
		field_to_json(Obj,"location", Location);
		field_to_json(Obj,"venue", Venue);
		field_to_json(Obj,"firmware", Firmware);
		field_to_json(Obj,"compatible", Compatible);
		field_to_json(Obj,"fwUpdatePolicy", FWUpdatePolicy);
		field_to_json(Obj,"devicePassword", DevicePassword);
	}

	void Device::to_json_with_status(Poco::JSON::Object &Obj) const {
		to_json(Obj);
		ConnectionState ConState;
		if (DeviceRegistry()->GetState(SerialNumber, ConState)) {
			ConState.to_json(Obj);
		} else {
			field_to_json(Obj,"ipAddress", "N/A");
			field_to_json(Obj,"txBytes", (uint64_t) 0);
			field_to_json(Obj,"rxBytes", (uint64_t )0);
			field_to_json(Obj,"messageCount", (uint64_t )0);
			field_to_json(Obj,"connected", false);
			field_to_json(Obj,"lastContact", "N/A");
			field_to_json(Obj,"verifiedCertificate", "NO_CERTIFICATE");
		}
	}

	bool Device::from_json(Poco::JSON::Object::Ptr Obj) {
		try {
			field_from_json(Obj,"serialNumber",SerialNumber);
			field_from_json(Obj,"deviceType",DeviceType);
			field_from_json(Obj,"macAddress",MACAddress);
			field_from_json(Obj,"configuration",Configuration);
			field_from_json(Obj,"notes",Notes);
			field_from_json(Obj,"manufacturer",Manufacturer);
			field_from_json(Obj,"owner",Owner);
			field_from_json(Obj,"location",Location);
			field_from_json(Obj,"venue",Venue);
			field_from_json(Obj,"compatible",Compatible);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void Device::Print() const {
		std::cout << "Device: " << SerialNumber << " DeviceType:" << DeviceType << " MACAddress:" << MACAddress << " Manufacturer:"
				  << Manufacturer << " " << Configuration << std::endl;
	}

	void Statistics::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("data", Obj, Data);
		field_to_json(Obj,"UUID", UUID);
		field_to_json(Obj,"recorded", Recorded);
	}

	void Capabilities::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("capabilities", Obj, Capabilities);
		field_to_json(Obj,"firstUpdate", FirstUpdate);
		field_to_json(Obj,"lastUpdate", LastUpdate);
	}

	void DeviceLog::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("data", Obj, Data);
		field_to_json(Obj,"log", Log);
		field_to_json(Obj,"severity", Severity);
		field_to_json(Obj,"recorded", Recorded);
		field_to_json(Obj,"logType", LogType);
		field_to_json(Obj,"UUID", UUID);
	}

	void HealthCheck::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("values", Obj, Data);
		field_to_json(Obj,"UUID", UUID);
		field_to_json(Obj,"sanity", Sanity);
		field_to_json(Obj,"recorded", Recorded);
	}

	void DefaultConfiguration::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("configuration", Obj, Configuration);
		field_to_json(Obj,"name", Name);
		field_to_json(Obj,"modelIds", Models);
		field_to_json(Obj,"description", Description);
		field_to_json(Obj,"created", Created);
		field_to_json(Obj,"lastModified", LastModified);
	}

	void CommandDetails::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("details", Obj, Details);
		EmbedDocument("results", Obj, Results);
		field_to_json(Obj,"UUID", UUID);
		field_to_json(Obj,"serialNumber", SerialNumber);
		field_to_json(Obj,"command", Command);
		field_to_json(Obj,"errorText", ErrorText);
		field_to_json(Obj,"submittedBy", SubmittedBy);
		field_to_json(Obj,"status", Status);
		field_to_json(Obj,"submitted", Submitted);
		field_to_json(Obj,"executed", Executed);
		field_to_json(Obj,"completed", Completed);
		field_to_json(Obj,"when", RunAt);
		field_to_json(Obj,"errorCode", ErrorCode);
		field_to_json(Obj,"custom", Custom);
		field_to_json(Obj,"waitingForFile", WaitingForFile);
		field_to_json(Obj,"attachFile", AttachDate);
	}

	bool DefaultConfiguration::from_json(Poco::JSON::Object::Ptr Obj) {
		try {
			field_from_json(Obj,"name",Name);
			field_from_json(Obj,"configuration",Configuration);
			field_from_json(Obj,"modelIds",Models);
			field_from_json(Obj,"description",Description);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void BlackListedDevice::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"serialNumber", SerialNumber);
		field_to_json(Obj,"author", Author);
		field_to_json(Obj,"reason", Reason);
		field_to_json(Obj,"created", Created);
	}

	void ConnectionState::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"serialNumber", SerialNumber);
		field_to_json(Obj,"ipAddress", Address);
		field_to_json(Obj,"txBytes", TX);
		field_to_json(Obj,"rxBytes", RX);
		field_to_json(Obj,"messageCount", MessageCount);
		field_to_json(Obj,"UUID", UUID);
		field_to_json(Obj,"connected", Connected);
		field_to_json(Obj,"firmware", Firmware);
		field_to_json(Obj,"lastContact", LastContact);
		switch(VerifiedCertificate) {
			case NO_CERTIFICATE:
				field_to_json(Obj,"verifiedCertificate", "NO_CERTIFICATE"); break;
			case VALID_CERTIFICATE:
				field_to_json(Obj,"verifiedCertificate", "VALID_CERTIFICATE"); break;
			case MISMATCH_SERIAL:
				field_to_json(Obj,"verifiedCertificate", "MISMATCH_SERIAL"); break;
			case VERIFIED:
				field_to_json(Obj,"verifiedCertificate", "VERIFIED"); break;
			default:
				field_to_json(Obj,"verifiedCertificate", "NO_CERTIFICATE"); break;
		}
	}

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

