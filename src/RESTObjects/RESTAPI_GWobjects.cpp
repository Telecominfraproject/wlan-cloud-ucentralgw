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
#ifdef	TIP_GATEWAY_SERVICE
#include "DeviceRegistry.h"
#include "CapabilitiesCache.h"
#endif

#include "RESTAPI_GWobjects.h"
#include "framework/MicroService.h"

using OpenWifi::RESTAPI_utils::field_to_json;
using OpenWifi::RESTAPI_utils::field_from_json;
using OpenWifi::RESTAPI_utils::EmbedDocument;

namespace OpenWifi::GWObjects {

	void Device::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"serialNumber", SerialNumber);
#ifdef TIP_GATEWAY_SERVICE
		field_to_json(Obj,"deviceType", CapabilitiesCache::instance()->Get(Compatible));
#endif
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

#ifdef TIP_GATEWAY_SERVICE
		ConnectionState ConState;

		if (DeviceRegistry()->GetState(SerialNumber, ConState)) {
			ConState.to_json(Obj);
		} else {
			field_to_json(Obj,"ipAddress", "");
			field_to_json(Obj,"txBytes", (uint64_t) 0);
			field_to_json(Obj,"rxBytes", (uint64_t )0);
			field_to_json(Obj,"messageCount", (uint64_t )0);
			field_to_json(Obj,"connected", false);
			field_to_json(Obj,"lastContact", "");
			field_to_json(Obj,"verifiedCertificate", "NO_CERTIFICATE");
			field_to_json(Obj,"associations_2G", (uint64_t) 0);
			field_to_json(Obj,"associations_5G", (uint64_t) 0);
		}
#endif
	}

	bool Device::from_json(Poco::JSON::Object::Ptr &Obj) {
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

	bool DefaultConfiguration::from_json(Poco::JSON::Object::Ptr &Obj) {
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
		field_to_json(Obj,"serialNumber", serialNumber);
		field_to_json(Obj,"author", author);
		field_to_json(Obj,"reason", reason);
		field_to_json(Obj,"created", created);
	}

	bool BlackListedDevice::from_json(Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"serialNumber",serialNumber);
			field_from_json(Obj,"author",author);
			field_from_json(Obj,"reason",reason);
			field_from_json(Obj,"created",created);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
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
		field_to_json(Obj,"associations_2G", Associations_2G);
		field_to_json(Obj,"associations_5G", Associations_5G);

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
		field_to_json(Obj,"serialNumber", SerialNumber);
		field_to_json(Obj,"server", Server);
		field_to_json(Obj,"port", Port);
		field_to_json(Obj,"token",Token);
		field_to_json(Obj,"timeout", TimeOut);
		field_to_json(Obj,"connectionId",ConnectionId);
		field_to_json(Obj,"commandUUID",CommandUUID);
		field_to_json(Obj,"started", Started);
		field_to_json(Obj,"viewport",ViewPort);
		field_to_json(Obj,"password",DevicePassword);
	}

	void Dashboard::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"commands",commands);
		field_to_json(Obj,"upTimes",upTimes);
		field_to_json(Obj,"memoryUsed",memoryUsed);
		field_to_json(Obj,"load1",load1);
		field_to_json(Obj,"load5",load5);
		field_to_json(Obj,"load15",load15);
		field_to_json(Obj,"vendors",vendors);
		field_to_json(Obj,"status",status);
		field_to_json(Obj,"deviceType",deviceType);
		field_to_json(Obj,"healths",healths);
		field_to_json(Obj,"certificates",certificates);
		field_to_json(Obj,"lastContact",lastContact);
		field_to_json(Obj,"associations",associations);
		field_to_json(Obj,"snapshot",snapshot);
		field_to_json(Obj,"numberOfDevices",numberOfDevices);
	}

	void Dashboard::reset()  {
		commands.clear();
		upTimes.clear();
		memoryUsed.clear();
		load1.clear();
		load5.clear();
		load15.clear();
		vendors.clear();
		status.clear();
		deviceType.clear();
		healths.clear();
		certificates.clear();
		lastContact.clear();
		associations.clear();
		numberOfDevices = 0 ;
		snapshot = std::time(nullptr);
	}

	void CapabilitiesModel::to_json(Poco::JSON::Object &Obj) const{
		field_to_json(Obj,"deviceType", deviceType);
		field_to_json(Obj,"capabilities", capabilities);
	};

}

