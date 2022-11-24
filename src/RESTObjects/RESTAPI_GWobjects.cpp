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
#include "AP_WS_Server.h"
#include "CapabilitiesCache.h"
#endif

#include "RESTAPI_GWobjects.h"
#include "framework/RESTAPI_utils.h"
#include "framework/utils.h"

using OpenWifi::RESTAPI_utils::field_to_json;
using OpenWifi::RESTAPI_utils::field_from_json;
using OpenWifi::RESTAPI_utils::EmbedDocument;

namespace OpenWifi::GWObjects {

	void Device::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"serialNumber", SerialNumber);
#ifdef TIP_GATEWAY_SERVICE
		field_to_json(Obj,"deviceType", CapabilitiesCache::instance()->GetPlatform(Compatible));
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
		field_to_json(Obj,"subscriber", subscriber);
		field_to_json(Obj,"entity", entity);
		field_to_json(Obj,"modified", modified);
		field_to_json(Obj,"locale", locale);
		field_to_json(Obj,"restrictedDevice", restrictedDevice);

	}

	void Device::to_json_with_status(Poco::JSON::Object &Obj) const {
		to_json(Obj);

#ifdef TIP_GATEWAY_SERVICE
		ConnectionState ConState;

		if (AP_WS_Server()->GetState(SerialNumber, ConState)) {
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
			field_to_json(Obj,"associations_6G", (uint64_t) 0);
		}
#endif
	}

	bool Device::from_json(const Poco::JSON::Object::Ptr &Obj) {
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
			field_from_json(Obj,"subscriber", subscriber);
			field_from_json(Obj,"entity", entity);
			field_from_json(Obj,"locale", locale);
			field_from_json(Obj,"restrictedDevice", restrictedDevice);
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
		field_to_json(Obj,"executionTime", executionTime);
	}

	bool DefaultConfiguration::from_json(const Poco::JSON::Object::Ptr &Obj) {
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

	bool BlackListedDevice::from_json(const Poco::JSON::Object::Ptr &Obj) {
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
		field_to_json(Obj,"associations_6G", Associations_6G);
		field_to_json(Obj,"webSocketClients", webSocketClients);
		field_to_json(Obj,"websocketPackets", websocketPackets);
		field_to_json(Obj,"kafkaClients", kafkaClients);
		field_to_json(Obj,"kafkaPackets", kafkaPackets);
		field_to_json(Obj,"locale", locale);
		field_to_json(Obj,"started", started);
		field_to_json(Obj,"sessionId", sessionId);
		field_to_json(Obj,"connectionCompletionTime", connectionCompletionTime);
		field_to_json(Obj,"totalConnectionTime", Utils::Now() - started);
		field_to_json(Obj,"certificateExpiryDate", certificateExpiryDate);

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

	void DeviceConnectionStatistics::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"averageConnectionTime", averageConnectionTime);
		field_to_json(Obj,"connectedDevices", connectedDevices );
		field_to_json(Obj,"connectingDevices", connectingDevices );
	}

	bool DeviceConnectionStatistics::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"averageConnectionTime", averageConnectionTime);
			field_from_json(Obj,"connectedDevices", connectedDevices );
			field_from_json(Obj,"connectingDevices", connectingDevices );
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
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
		snapshot = Utils::Now();
	}

	void CapabilitiesModel::to_json(Poco::JSON::Object &Obj) const{
		field_to_json(Obj,"deviceType", deviceType);
		field_to_json(Obj,"capabilities", capabilities);
	};

	void ScriptRequest::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"serialNumber",serialNumber);
		field_to_json(Obj,"timeout",timeout);
		field_to_json(Obj,"type",type);
		field_to_json(Obj,"scriptId",scriptId);
		field_to_json(Obj,"script",script);
		field_to_json(Obj,"when",when);
		field_to_json(Obj,"signature", signature);
		field_to_json(Obj,"deferred", deferred);
		field_to_json(Obj,"uri", uri);
	}

	bool ScriptRequest::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"serialNumber",serialNumber);
			field_from_json(Obj,"timeout",timeout);
			field_from_json(Obj,"type",type);
			field_from_json(Obj,"script",script);
			field_from_json(Obj,"scriptId",scriptId);
			field_from_json(Obj,"when",when);
			field_from_json(Obj,"signature", signature);
			field_from_json(Obj,"deferred", deferred);
			field_from_json(Obj,"uri", uri);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RadiusProxyPoolList::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"pools",pools);
	}

	bool RadiusProxyPoolList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"pools",pools);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RadiusProxyPool::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"name",name);
		field_to_json(Obj,"description",description);
		field_to_json(Obj,"authConfig",authConfig);
		field_to_json(Obj,"acctConfig",acctConfig);
		field_to_json(Obj,"coaConfig",coaConfig);
		field_to_json(Obj,"useByDefault",useByDefault);
	}

	bool RadiusProxyPool::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"name",name);
			field_from_json(Obj,"description",description);
			field_from_json(Obj,"authConfig",authConfig);
			field_from_json(Obj,"acctConfig",acctConfig);
			field_from_json(Obj,"coaConfig",coaConfig);
			field_from_json(Obj,"useByDefault",useByDefault);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RadiusProxyServerConfig::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"strategy",strategy);
		field_to_json(Obj,"monitor",monitor);
		field_to_json(Obj,"monitorMethod",monitorMethod);
		field_to_json(Obj,"methodParameters",methodParameters);
		field_to_json(Obj,"servers",servers);
	}

	bool RadiusProxyServerConfig::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"strategy",strategy);
			field_from_json(Obj,"monitor",monitor);
			field_from_json(Obj,"monitorMethod",monitorMethod);
			field_from_json(Obj,"methodParameters",methodParameters);
			field_from_json(Obj,"servers",servers);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RadiusProxyServerEntry::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"name",name);
		field_to_json(Obj,"ip",ip);
		field_to_json(Obj,"port",port);
		field_to_json(Obj,"weight",weight);
		field_to_json(Obj,"secret",secret);
		field_to_json(Obj,"certificate",certificate);
		field_to_json(Obj,"radsec",radsec);
		field_to_json(Obj,"radsecPort",radsecPort);
		field_to_json(Obj,"radsecSecret",radsecSecret);
		field_to_json(Obj,"radsecCacerts",radsecCacerts);
		field_to_json(Obj,"radsecCert",radsecCert);
		field_to_json(Obj,"radsecKey",radsecKey);
		field_to_json(Obj,"radsecRealms",radsecRealms);
		field_to_json(Obj,"ignore",ignore);
	}

	bool RadiusProxyServerEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"name",name);
			field_from_json(Obj,"ip",ip);
			field_from_json(Obj,"port",port);
			field_from_json(Obj,"weight",weight);
			field_from_json(Obj,"secret",secret);
			field_from_json(Obj,"certificate",certificate);
			field_from_json(Obj,"radsec",radsec);
			field_from_json(Obj,"radsecSecret",radsecSecret);
			field_from_json(Obj,"radsecPort",radsecPort);
			field_from_json(Obj,"radsecCacerts",radsecCacerts);
			field_from_json(Obj,"radsecCert",radsecCert);
			field_from_json(Obj,"radsecKey",radsecKey);
			field_from_json(Obj,"radsecRealms",radsecRealms);
			field_from_json(Obj,"ignore",ignore);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void ScriptEntry::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"id", id);
		field_to_json(Obj,"name", name);
		field_to_json(Obj,"description", description);
		field_to_json(Obj,"uri", uri);
		field_to_json(Obj,"content", content);
		field_to_json(Obj,"version", version);
		field_to_json(Obj,"type", type);
		field_to_json(Obj,"created", created);
		field_to_json(Obj,"modified", modified);
		field_to_json(Obj,"author", author);
		field_to_json(Obj,"restricted", restricted);
		field_to_json(Obj,"deferred", deferred);
		field_to_json(Obj,"timeout", timeout);
		field_to_json(Obj,"defaultUploadURI", defaultUploadURI);
	}

	bool ScriptEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"id", id);
			field_from_json(Obj,"name", name);
			field_from_json(Obj,"description", description);
			field_from_json(Obj,"uri", uri);
			field_from_json(Obj,"content", content);
			field_from_json(Obj,"version", version);
			field_from_json(Obj,"type", type);
			field_from_json(Obj,"created", created);
			field_from_json(Obj,"modified", modified);
			field_from_json(Obj,"author", author);
			field_from_json(Obj,"restricted", restricted);
			field_from_json(Obj,"deferred", deferred);
			field_from_json(Obj,"timeout", timeout);
			field_from_json(Obj,"defaultUploadURI", defaultUploadURI);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void ScriptEntryList::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"scripts",scripts);
	}

	bool ScriptEntryList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"scripts",scripts);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

}

