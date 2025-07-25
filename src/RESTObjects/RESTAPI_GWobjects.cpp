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
#ifdef TIP_GATEWAY_SERVICE
#include "AP_WS_Server.h"
#include "StorageService.h"
#include "CapabilitiesCache.h"
#include "RADIUSSessionTracker.h"
#endif

#include "RESTAPI_GWobjects.h"
#include "framework/RESTAPI_utils.h"
#include "framework/utils.h"

using OpenWifi::RESTAPI_utils::EmbedDocument;
using OpenWifi::RESTAPI_utils::field_from_json;
using OpenWifi::RESTAPI_utils::field_to_json;

namespace OpenWifi::GWObjects {

	void Device::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "serialNumber", SerialNumber);
#ifdef TIP_GATEWAY_SERVICE
		field_to_json(Obj, "deviceType", StorageService()->GetPlatform(SerialNumber));
		field_to_json(Obj, "blackListed", StorageService()->IsBlackListed(Utils::MACToInt(SerialNumber)));
#endif
		field_to_json(Obj, "macAddress", MACAddress);
		field_to_json(Obj, "manufacturer", Manufacturer);
		field_to_json(Obj, "UUID", UUID);
		EmbedDocument("configuration", Obj, Configuration);
		field_to_json(Obj, "notes", Notes);
		field_to_json(Obj, "createdTimestamp", CreationTimestamp);
		field_to_json(Obj, "lastConfigurationChange", LastConfigurationChange);
		field_to_json(Obj, "lastConfigurationDownload", LastConfigurationDownload);
		field_to_json(Obj, "lastFWUpdate", LastFWUpdate);
		field_to_json(Obj, "owner", Owner);
		field_to_json(Obj, "location", Location);
		field_to_json(Obj, "venue", Venue);
		field_to_json(Obj, "firmware", Firmware);
		field_to_json(Obj, "compatible", Compatible);
		field_to_json(Obj, "fwUpdatePolicy", FWUpdatePolicy);
		field_to_json(Obj, "devicePassword", DevicePassword);
		field_to_json(Obj, "subscriber", subscriber);
		field_to_json(Obj, "entity", entity);
		field_to_json(Obj, "modified", modified);
		field_to_json(Obj, "locale", locale);
		field_to_json(Obj, "restrictedDevice", restrictedDevice);
		field_to_json(Obj, "pendingConfiguration", pendingConfiguration);
		field_to_json(Obj, "pendingConfigurationCmd", pendingConfigurationCmd);
		field_to_json(Obj, "restrictionDetails", restrictionDetails);
		field_to_json(Obj, "pendingUUID", pendingUUID);
		field_to_json(Obj, "simulated", simulated);
		field_to_json(Obj, "lastRecordedContact", lastRecordedContact);
		field_to_json(Obj, "certificateExpiryDate", certificateExpiryDate);
		field_to_json(Obj, "connectReason", connectReason);
	}

	void Device::to_json_with_status(Poco::JSON::Object &Obj) const {
		to_json(Obj);

#ifdef TIP_GATEWAY_SERVICE
		ConnectionState ConState;
#ifdef USE_MEDUSA_CLIENT
        auto Res = GS()->GetState(SerialNumber);
        if (Res.has_value()) {
            Res.value().to_json(SerialNumber,Obj);
#else
        if (AP_WS_Server()->GetState(SerialNumber, ConState)) {
			ConState.to_json(SerialNumber,Obj);
#endif
		} else {
			field_to_json(Obj, "ipAddress", "");
			field_to_json(Obj, "txBytes", (uint64_t)0);
			field_to_json(Obj, "rxBytes", (uint64_t)0);
			field_to_json(Obj, "messageCount", (uint64_t)0);
			field_to_json(Obj, "connected", false);
			field_to_json(Obj, "lastContact", "");
			field_to_json(Obj, "verifiedCertificate", "NO_CERTIFICATE");
			field_to_json(Obj, "associations_2G", (uint64_t)0);
			field_to_json(Obj, "associations_5G", (uint64_t)0);
			field_to_json(Obj, "associations_6G", (uint64_t)0);
			field_to_json(Obj, "hasRADIUSSessions", false);
			field_to_json(Obj, "hasGPS", ConState.hasGPS);
			field_to_json(Obj, "sanity", ConState.sanity);
			field_to_json(Obj, "memoryUsed", ConState.memoryUsed);
			field_to_json(Obj, "sanity", ConState.sanity);
			field_to_json(Obj, "load", ConState.load);
			field_to_json(Obj, "temperature", ConState.temperature);
		}
#endif
	}

	bool Device::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "serialNumber", SerialNumber);
			field_from_json(Obj, "deviceType", DeviceType);
			field_from_json(Obj, "macAddress", MACAddress);
			field_from_json(Obj, "manufacturer", Manufacturer);
			field_from_json(Obj, "UUID", UUID);
			field_from_json(Obj, "configuration", Configuration);
			field_from_json(Obj, "notes", Notes);
			field_from_json(Obj, "createdTimestamp", CreationTimestamp);
			field_from_json(Obj, "lastConfigurationChange", LastConfigurationChange);
			field_from_json(Obj, "lastConfigurationDownload", LastConfigurationDownload);
			field_from_json(Obj, "lastFWUpdate", LastFWUpdate);
			field_from_json(Obj, "owner", Owner);
			field_from_json(Obj, "location", Location);
			field_from_json(Obj, "venue", Venue);
			field_from_json(Obj, "firmware", Firmware);
			field_from_json(Obj, "compatible", Compatible);
			field_from_json(Obj, "fwUpdatePolicy", FWUpdatePolicy);
			field_from_json(Obj, "devicePassword", DevicePassword);
			field_from_json(Obj, "subscriber", subscriber);
			field_from_json(Obj, "entity", entity);
			field_from_json(Obj, "modified", modified);
			field_from_json(Obj, "locale", locale);
			field_from_json(Obj, "restrictedDevice", restrictedDevice);
			field_from_json(Obj, "pendingConfiguration", pendingConfiguration);
			field_from_json(Obj, "pendingConfigurationCmd", pendingConfigurationCmd);
			field_from_json(Obj, "restrictionDetails", restrictionDetails);
			field_from_json(Obj, "pendingUUID", pendingUUID);
			field_from_json(Obj, "simulated", simulated);
			field_from_json(Obj, "lastRecordedContact", lastRecordedContact);
			field_from_json(Obj, "certificateExpiryDate", certificateExpiryDate);
			field_from_json(Obj, "connectReason", connectReason);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void Device::Print() const {
		std::cout << "Device: " << SerialNumber << " DeviceType:" << DeviceType
				  << " MACAddress:" << MACAddress << " Manufacturer:" << Manufacturer << " "
				  << Configuration << std::endl;
	}

	void Statistics::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("data", Obj, Data);
		field_to_json(Obj, "UUID", UUID);
		field_to_json(Obj, "recorded", Recorded);
	}

	void Capabilities::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("capabilities", Obj, Capabilities);
		field_to_json(Obj, "firstUpdate", FirstUpdate);
		field_to_json(Obj, "lastUpdate", LastUpdate);
	}

	void DeviceLog::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("data", Obj, Data);
		field_to_json(Obj, "log", Log);
		field_to_json(Obj, "severity", Severity);
		field_to_json(Obj, "recorded", Recorded);
		field_to_json(Obj, "logType", LogType);
		field_to_json(Obj, "UUID", UUID);
	}

	void HealthCheck::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("values", Obj, Data);
		field_to_json(Obj, "UUID", UUID);
		field_to_json(Obj, "sanity", Sanity);
		field_to_json(Obj, "recorded", Recorded);
	}

    bool HealthCheck::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "UUID", UUID);
            field_from_json(Obj, "sanity", Sanity);
            field_from_json(Obj, "recorded", Recorded);
            return true;
        } catch(...) {

        }
        return false;
    }

	void DefaultFirmware::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "deviceType", deviceType);
		field_to_json(Obj, "description", Description);
		field_to_json(Obj, "uri", uri);
		field_to_json(Obj, "revision", revision);
		field_to_json(Obj, "imageCreationDate", imageCreationDate);
		field_to_json(Obj, "created", Created);
		field_to_json(Obj, "lastModified", LastModified);
	}

	bool DefaultFirmware::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "deviceType", deviceType);
			field_from_json(Obj, "description", Description);
			field_from_json(Obj, "uri", uri);
			field_from_json(Obj, "revision", revision);
			field_from_json(Obj, "imageCreationDate", imageCreationDate);
			field_from_json(Obj, "created", Created);
			field_from_json(Obj, "lastModified", LastModified);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void CommandDetails::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("details", Obj, Details);
		EmbedDocument("results", Obj, Results);
		field_to_json(Obj, "UUID", UUID);
		field_to_json(Obj, "serialNumber", SerialNumber);
		field_to_json(Obj, "command", Command);
		field_to_json(Obj, "errorText", ErrorText);
		field_to_json(Obj, "submittedBy", SubmittedBy);
		field_to_json(Obj, "status", Status);
		field_to_json(Obj, "submitted", Submitted);
		field_to_json(Obj, "executed", Executed);
		field_to_json(Obj, "completed", Completed);
		field_to_json(Obj, "when", RunAt);
		field_to_json(Obj, "errorCode", ErrorCode);
		field_to_json(Obj, "custom", Custom);
		field_to_json(Obj, "waitingForFile", WaitingForFile);
		field_to_json(Obj, "attachFile", AttachDate);
		field_to_json(Obj, "executionTime", executionTime);
		field_to_json(Obj, "lastTry", lastTry);
		field_to_json(Obj, "deferred", deferred);
	}

	void DefaultConfiguration::to_json(Poco::JSON::Object &Obj) const {
		EmbedDocument("configuration", Obj, configuration);
		field_to_json(Obj, "name", name);
		field_to_json(Obj, "modelIds", models);
		field_to_json(Obj, "description", description);
		field_to_json(Obj, "created", created);
		field_to_json(Obj, "lastModified", lastModified);
		field_to_json(Obj, "platform", platform);
	}

	bool DefaultConfiguration::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "configuration", configuration);
			field_from_json(Obj, "name", name);
			field_from_json(Obj, "modelIds", models);
			field_from_json(Obj, "description", description);
			field_from_json(Obj, "created", created);
			field_from_json(Obj, "lastModified", lastModified);
			field_from_json(Obj, "platform", platform);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void BlackListedDevice::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "serialNumber", serialNumber);
		field_to_json(Obj, "author", author);
		field_to_json(Obj, "reason", reason);
		field_to_json(Obj, "created", created);
	}

	bool BlackListedDevice::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "serialNumber", serialNumber);
			field_from_json(Obj, "author", author);
			field_from_json(Obj, "reason", reason);
			field_from_json(Obj, "created", created);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void ConnectionState::to_json([[maybe_unused]] const std::string &SerialNumber, Poco::JSON::Object &Obj)  {
		field_to_json(Obj, "ipAddress", Address);
		field_to_json(Obj, "txBytes", TX);
		field_to_json(Obj, "rxBytes", RX);
		field_to_json(Obj, "messageCount", MessageCount);
		field_to_json(Obj, "UUID", UUID);
		field_to_json(Obj, "connected", Connected);
		field_to_json(Obj, "firmware", Firmware);
		field_to_json(Obj, "lastContact", LastContact);
		field_to_json(Obj, "associations_2G", Associations_2G);
		field_to_json(Obj, "associations_5G", Associations_5G);
		field_to_json(Obj, "associations_6G", Associations_6G);
		field_to_json(Obj, "webSocketClients", webSocketClients);
		field_to_json(Obj, "websocketPackets", websocketPackets);
		field_to_json(Obj, "kafkaClients", kafkaClients);
		field_to_json(Obj, "kafkaPackets", kafkaPackets);
		field_to_json(Obj, "locale", locale);
		field_to_json(Obj, "started", started);
		field_to_json(Obj, "sessionId", sessionId);
		field_to_json(Obj, "connectionCompletionTime", connectionCompletionTime);
		field_to_json(Obj, "totalConnectionTime", Utils::Now() - started);
		field_to_json(Obj, "certificateExpiryDate", certificateExpiryDate);
		field_to_json(Obj, "certificateIssuerName", certificateIssuerName);
		field_to_json(Obj, "connectReason", connectReason);
		field_to_json(Obj, "uptime", uptime);
        field_to_json(Obj, "compatible", Compatible);

#ifdef TIP_GATEWAY_SERVICE
		hasRADIUSSessions = RADIUSSessionTracker()->HasSessions(SerialNumber);
#endif
		field_to_json(Obj, "hasRADIUSSessions", hasRADIUSSessions );
		field_to_json(Obj, "hasGPS", hasGPS);
		field_to_json(Obj, "sanity", sanity);
		field_to_json(Obj, "memoryUsed", memoryUsed);
		field_to_json(Obj, "sanity", sanity);
		field_to_json(Obj, "load", load);
		field_to_json(Obj, "temperature", temperature);

		switch (VerifiedCertificate) {
		case NO_CERTIFICATE:
			field_to_json(Obj, "verifiedCertificate", "NO_CERTIFICATE");
			break;
		case VALID_CERTIFICATE:
			field_to_json(Obj, "verifiedCertificate", "VALID_CERTIFICATE");
			break;
		case MISMATCH_SERIAL:
			field_to_json(Obj, "verifiedCertificate", "MISMATCH_SERIAL");
			break;
		case VERIFIED:
			field_to_json(Obj, "verifiedCertificate", "VERIFIED");
			break;
		case SIMULATED:
			field_to_json(Obj, "verifiedCertificate", "SIMULATED");
			break;
		default:
			field_to_json(Obj, "verifiedCertificate", "NO_CERTIFICATE");
			break;
		}
	}

    bool ConnectionState::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "compatible", Compatible);
            field_from_json(Obj, "ipAddress", Address);
            field_from_json(Obj, "txBytes", TX);
            field_from_json(Obj, "rxBytes", RX);
            field_from_json(Obj, "messageCount", MessageCount);
            field_from_json(Obj, "UUID", UUID);
            field_from_json(Obj, "connected", Connected);
            field_from_json(Obj, "firmware", Firmware);
            field_from_json(Obj, "lastContact", LastContact);
            field_from_json(Obj, "associations_2G", Associations_2G);
            field_from_json(Obj, "associations_5G", Associations_5G);
            field_from_json(Obj, "associations_6G", Associations_6G);
            field_from_json(Obj, "webSocketClients", webSocketClients);
            field_from_json(Obj, "websocketPackets", websocketPackets);
            field_from_json(Obj, "kafkaClients", kafkaClients);
            field_from_json(Obj, "kafkaPackets", kafkaPackets);
            field_from_json(Obj, "locale", locale);
            field_from_json(Obj, "started", started);
            field_from_json(Obj, "sessionId", sessionId);
            field_from_json(Obj, "connectionCompletionTime", connectionCompletionTime);
            field_from_json(Obj, "totalConnectionTime", totalConnectionTime);
            field_from_json(Obj, "certificateExpiryDate", certificateExpiryDate);
			field_from_json(Obj, "certificateIssuerName", certificateIssuerName);
            field_from_json(Obj, "connectReason", connectReason);
            field_from_json(Obj, "uptime", uptime);
            field_from_json(Obj, "hasRADIUSSessions", hasRADIUSSessions );
            field_from_json(Obj, "hasGPS", hasGPS);
            field_from_json(Obj, "sanity", sanity);
            field_from_json(Obj, "memoryUsed", memoryUsed);
            field_from_json(Obj, "sanity", sanity);
            field_from_json(Obj, "load", load);
            field_from_json(Obj, "temperature", temperature);
            return true;
        } catch(const Poco::Exception &E) {
        }
        return false;
    }

	void DeviceConnectionStatistics::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "averageConnectionTime", averageConnectionTime);
		field_to_json(Obj, "connectedDevices", connectedDevices);
		field_to_json(Obj, "connectingDevices", connectingDevices);
	}

	bool DeviceConnectionStatistics::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "averageConnectionTime", averageConnectionTime);
			field_from_json(Obj, "connectedDevices", connectedDevices);
			field_from_json(Obj, "connectingDevices", connectingDevices);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RttySessionDetails::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "serialNumber", SerialNumber);
		field_to_json(Obj, "server", Server);
		field_to_json(Obj, "port", Port);
		field_to_json(Obj, "token", Token);
		field_to_json(Obj, "timeout", TimeOut);
		field_to_json(Obj, "connectionId", ConnectionId);
		field_to_json(Obj, "commandUUID", CommandUUID);
		field_to_json(Obj, "started", Started);
		field_to_json(Obj, "viewport", ViewPort);
		field_to_json(Obj, "password", DevicePassword);
	}

	void Dashboard::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "commands", commands);
		field_to_json(Obj, "upTimes", upTimes);
		field_to_json(Obj, "memoryUsed", memoryUsed);
		field_to_json(Obj, "load1", load1);
		field_to_json(Obj, "load5", load5);
		field_to_json(Obj, "load15", load15);
		field_to_json(Obj, "vendors", vendors);
		field_to_json(Obj, "status", status);
		field_to_json(Obj, "deviceType", deviceType);
		field_to_json(Obj, "healths", healths);
		field_to_json(Obj, "certificates", certificates);
		field_to_json(Obj, "lastContact", lastContact);
		field_to_json(Obj, "associations", associations);
		field_to_json(Obj, "snapshot", snapshot);
		field_to_json(Obj, "numberOfDevices", numberOfDevices);
	}

	void Dashboard::reset() {
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
		numberOfDevices = 0;
		snapshot = Utils::Now();
	}

	void CapabilitiesModel::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "deviceType", deviceType);
		field_to_json(Obj, "capabilities", capabilities);
	};

	void ScriptRequest::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "serialNumber", serialNumber);
		field_to_json(Obj, "timeout", timeout);
		field_to_json(Obj, "type", type);
		field_to_json(Obj, "scriptId", scriptId);
		field_to_json(Obj, "script", script);
		field_to_json(Obj, "when", when);
		field_to_json(Obj, "signature", signature);
		field_to_json(Obj, "deferred", deferred);
		field_to_json(Obj, "uri", uri);
	}

	bool ScriptRequest::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "serialNumber", serialNumber);
			field_from_json(Obj, "timeout", timeout);
			field_from_json(Obj, "type", type);
			field_from_json(Obj, "script", script);
			field_from_json(Obj, "scriptId", scriptId);
			field_from_json(Obj, "when", when);
			field_from_json(Obj, "signature", signature);
			field_from_json(Obj, "deferred", deferred);
			field_from_json(Obj, "uri", uri);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RadiusProxyPoolList::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "pools", pools);
	}

	bool RadiusProxyPoolList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "pools", pools);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RadiusProxyPool::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "name", name);
		field_to_json(Obj, "description", description);
		field_to_json(Obj, "authConfig", authConfig);
		field_to_json(Obj, "acctConfig", acctConfig);
		field_to_json(Obj, "coaConfig", coaConfig);
		field_to_json(Obj, "useByDefault", useByDefault);
		field_to_json(Obj, "radsecKeepAlive", radsecKeepAlive);
		field_to_json(Obj, "poolProxyIp", poolProxyIp);
		field_to_json(Obj, "radsecPoolType", radsecPoolType);
		field_to_json(Obj, "enabled", enabled);
	}

	bool RadiusProxyPool::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "name", name);
			field_from_json(Obj, "description", description);
			field_from_json(Obj, "authConfig", authConfig);
			field_from_json(Obj, "acctConfig", acctConfig);
			field_from_json(Obj, "coaConfig", coaConfig);
			field_from_json(Obj, "useByDefault", useByDefault);
			field_from_json(Obj, "radsecKeepAlive", radsecKeepAlive);
			field_from_json(Obj, "poolProxyIp", poolProxyIp);
			field_from_json(Obj, "radsecPoolType", radsecPoolType);
			field_from_json(Obj, "enabled", enabled);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RadiusProxyServerConfig::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "strategy", strategy);
		field_to_json(Obj, "monitor", monitor);
		field_to_json(Obj, "monitorMethod", monitorMethod);
		field_to_json(Obj, "methodParameters", methodParameters);
		field_to_json(Obj, "servers", servers);
	}

	bool RadiusProxyServerConfig::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "strategy", strategy);
			field_from_json(Obj, "monitor", monitor);
			field_from_json(Obj, "monitorMethod", monitorMethod);
			field_from_json(Obj, "methodParameters", methodParameters);
			field_from_json(Obj, "servers", servers);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RadiusProxyServerEntry::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "name", name);
		field_to_json(Obj, "ip", ip);
		field_to_json(Obj, "port", port);
		field_to_json(Obj, "weight", weight);
		field_to_json(Obj, "secret", secret);
		field_to_json(Obj, "certificate", certificate);
		field_to_json(Obj, "radsec", radsec);
		field_to_json(Obj, "allowSelfSigned", allowSelfSigned);
		field_to_json(Obj, "radsecPort", radsecPort);
		field_to_json(Obj, "radsecSecret", radsecSecret);
		field_to_json(Obj, "radsecCacerts", radsecCacerts);
		field_to_json(Obj, "radsecCert", radsecCert);
		field_to_json(Obj, "radsecKey", radsecKey);
		field_to_json(Obj, "radsecRealms", radsecRealms);
		field_to_json(Obj, "ignore", ignore);
	}

	bool RadiusProxyServerEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "name", name);
			field_from_json(Obj, "ip", ip);
			field_from_json(Obj, "port", port);
			field_from_json(Obj, "weight", weight);
			field_from_json(Obj, "secret", secret);
			field_from_json(Obj, "certificate", certificate);
			field_from_json(Obj, "radsec", radsec);
			field_from_json(Obj, "allowSelfSigned", allowSelfSigned);
			field_from_json(Obj, "radsecSecret", radsecSecret);
			field_from_json(Obj, "radsecPort", radsecPort);
			field_from_json(Obj, "radsecCacerts", radsecCacerts);
			field_from_json(Obj, "radsecCert", radsecCert);
			field_from_json(Obj, "radsecKey", radsecKey);
			field_from_json(Obj, "radsecRealms", radsecRealms);
			field_from_json(Obj, "ignore", ignore);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void ScriptEntry::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "id", id);
		field_to_json(Obj, "name", name);
		field_to_json(Obj, "description", description);
		field_to_json(Obj, "uri", uri);
		field_to_json(Obj, "content", content);
		field_to_json(Obj, "version", version);
		field_to_json(Obj, "type", type);
		field_to_json(Obj, "created", created);
		field_to_json(Obj, "modified", modified);
		field_to_json(Obj, "author", author);
		field_to_json(Obj, "restricted", restricted);
		field_to_json(Obj, "deferred", deferred);
		field_to_json(Obj, "timeout", timeout);
		field_to_json(Obj, "defaultUploadURI", defaultUploadURI);
	}

	bool ScriptEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "id", id);
			field_from_json(Obj, "name", name);
			field_from_json(Obj, "description", description);
			field_from_json(Obj, "uri", uri);
			field_from_json(Obj, "content", content);
			field_from_json(Obj, "version", version);
			field_from_json(Obj, "type", type);
			field_from_json(Obj, "created", created);
			field_from_json(Obj, "modified", modified);
			field_from_json(Obj, "author", author);
			field_from_json(Obj, "restricted", restricted);
			field_from_json(Obj, "deferred", deferred);
			field_from_json(Obj, "timeout", timeout);
			field_from_json(Obj, "defaultUploadURI", defaultUploadURI);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void ScriptEntryList::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "scripts", scripts);
	}

	bool ScriptEntryList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "scripts", scripts);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void RangeOptions::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "NO_IR", NO_IR);
		field_to_json(Obj, "AUTO_BW", AUTO_BW);
		field_to_json(Obj, "DFS", DFS);
		field_to_json(Obj, "NO_OUTDOOR", NO_OUTDOOR);
		field_to_json(Obj, "wmmrule_ETSI", wmmrule_ETSI);
		field_to_json(Obj, "NO_OFDM", NO_OFDM);
	}

	void FrequencyRange::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "from", from);
		field_to_json(Obj, "to", to);
		field_to_json(Obj, "channelWidth", channelWidth);
		field_to_json(Obj, "powerDb", powerDb);
		field_to_json(Obj, "options", options);
	}

	void RegulatoryCountryInfo::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "country", country);
		field_to_json(Obj, "domain", domain);
		field_to_json(Obj, "ranges", ranges);
	}

	void DeviceRestrictionsKeyInfo::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "vendor", vendor);
		field_to_json(Obj, "algo", algo);
	}

	bool DeviceRestrictionsKeyInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "vendor", vendor);
			field_from_json(Obj, "algo", algo);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	void DeviceRestrictions::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "dfs", dfs);
		field_to_json(Obj, "ssh", ssh);
		field_to_json(Obj, "rtty", rtty);
		field_to_json(Obj, "tty", tty);
		field_to_json(Obj, "developer", developer);
		field_to_json(Obj, "upgrade", upgrade);
		field_to_json(Obj, "commands", commands);
		field_to_json(Obj, "country", country);
		field_to_json(Obj, "key_info", key_info);
	}

	bool DeviceRestrictions::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "dfs", dfs);
			field_from_json(Obj, "ssh", ssh);
			field_from_json(Obj, "rtty", rtty);
			field_from_json(Obj, "tty", tty);
			field_from_json(Obj, "developer", developer);
			field_from_json(Obj, "upgrade", upgrade);
			field_from_json(Obj, "commands", commands);
			field_from_json(Obj, "country", country);
			field_from_json(Obj, "key_info", key_info);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	bool DeviceRestrictionsKeyInfo::operator!=(
		const OpenWifi::GWObjects::DeviceRestrictionsKeyInfo &T) const {
		return (T.algo != algo) || (T.vendor != vendor);
	}

	bool DeviceRestrictions::operator!=(const OpenWifi::GWObjects::DeviceRestrictions &T) const {
		return ((T.dfs != dfs) || (T.rtty != rtty) || (T.upgrade != upgrade) ||
				(T.commands != commands) || (T.developer != developer) || (T.ssh != ssh) ||
				(T.key_info != key_info) || (T.country != country));
	}

	void RADIUSSession::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "started", started);
		field_to_json(Obj, "lastTransaction", lastTransaction);
		field_to_json(Obj, "destination", destination);
		field_to_json(Obj, "serialNumber", serialNumber);
		field_to_json(Obj, "userName", userName);
		field_to_json(Obj, "accountingSessionId", accountingSessionId);
		field_to_json(Obj, "accountingMultiSessionId", accountingMultiSessionId);
		field_to_json(Obj, "inputPackets", inputPackets);
		field_to_json(Obj, "outputPackets", outputPackets);
		field_to_json(Obj, "inputOctets", inputOctets);
		field_to_json(Obj, "outputOctets", outputOctets);
		field_to_json(Obj, "inputGigaWords", inputGigaWords);
		field_to_json(Obj, "outputGigaWords", outputGigaWords);
		field_to_json(Obj, "sessionTime", sessionTime);
		field_to_json(Obj, "callingStationId", callingStationId);
		field_to_json(Obj, "chargeableUserIdentity", chargeableUserIdentity);
		field_to_json(Obj, "interface", interface);
		field_to_json(Obj, "secret", secret);
		field_to_json(Obj, "nasId", nasId);
		field_to_json(Obj, "calledStationId", calledStationId);
	}

	void RADIUSSessionList::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "sessions", sessions);
	}

	void RadiusCoADMParameters::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "accountingSessionId", accountingSessionId);
		field_to_json(Obj, "accountingMultiSessionId", accountingMultiSessionId);
		field_to_json(Obj, "callingStationId", callingStationId);
		field_to_json(Obj, "chargeableUserIdentity", chargeableUserIdentity);
		field_to_json(Obj, "userName", userName);
	}

	bool RadiusCoADMParameters::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "accountingSessionId", accountingSessionId);
			field_from_json(Obj, "accountingMultiSessionId", accountingMultiSessionId);
			field_from_json(Obj, "callingStationId", callingStationId);
			field_from_json(Obj, "chargeableUserIdentity", chargeableUserIdentity);
			field_from_json(Obj, "userName", userName);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	bool DeviceTransferRequest::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "serialNumber", serialNumber);
			field_from_json(Obj, "server", server);
			field_from_json(Obj, "port", port);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	bool DeviceCertificateUpdateRequest::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "serial", serialNumber);
			field_from_json(Obj, "encodedCertificate", encodedCertificate);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	bool PowerCyclePort::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "name", name);
			field_from_json(Obj, "cycle", cycle);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	bool PowerCycleRequest::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "serial", serialNumber);
			field_from_json(Obj, "when", when);
			field_from_json(Obj, "ports", ports);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	bool FixedConfig::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "serial", serialNumber);
			field_from_json(Obj, "country", country);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	bool CableDiagnostics::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "serial", serialNumber);
			field_from_json(Obj, "when", when);
			field_from_json(Obj, "ports", ports);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}

	bool ReEnroll::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "serial", serialNumber);
			field_from_json(Obj, "when", when);
			return true;
		} catch (const Poco::Exception &E) {
		}
		return false;
	}
} // namespace OpenWifi::GWObjects
