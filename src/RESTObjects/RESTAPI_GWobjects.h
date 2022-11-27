//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "Poco/JSON/Object.h"
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi::GWObjects {

	enum CertificateValidation {
		NO_CERTIFICATE,
		VALID_CERTIFICATE,
		MISMATCH_SERIAL,
		VERIFIED
	};

	struct ConnectionState {
		uint64_t MessageCount = 0 ;
		std::string Address;
		uint64_t UUID = 0 ;
		uint64_t PendingUUID = 0 ;
		uint64_t TX = 0, RX = 0;
		uint64_t Associations_2G=0;
		uint64_t Associations_5G=0;
		uint64_t Associations_6G=0;
		bool Connected = false;
		uint64_t LastContact=0;
		std::string Firmware;
		CertificateValidation VerifiedCertificate = NO_CERTIFICATE;
		std::string 	Compatible;
		uint64_t 		kafkaClients=0;
		uint64_t 		webSocketClients=0;
		uint64_t 		kafkaPackets=0;
		uint64_t 		websocketPackets=0;
		std::string 	locale;
		uint64_t 		started=0;
		uint64_t 		sessionId=0;
		double      	connectionCompletionTime=0.0;
		std::uint64_t	certificateExpiryDate=0;

		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct Device {
		std::string SerialNumber;
		std::string DeviceType;
		std::string MACAddress;
		std::string Manufacturer;
		std::string Configuration;
		SecurityObjects::NoteInfoVec 	Notes;
		std::string Owner;
		std::string Location;
		std::string Firmware;
		std::string Compatible;
		std::string FWUpdatePolicy;
		uint64_t UUID = 0 ;
		uint64_t CreationTimestamp = 0 ;
		uint64_t LastConfigurationChange = 0 ;
		uint64_t LastConfigurationDownload = 0 ;
		uint64_t LastFWUpdate = 0 ;
		std::string Venue;
		std::string DevicePassword;
		std::string subscriber;
		std::string entity;
		uint64_t 	modified=0;
		std::string locale;
		bool 		restrictedDevice=false;

		void to_json(Poco::JSON::Object &Obj) const;
		void to_json_with_status(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
		void Print() const;
	};

	struct DeviceConnectionStatistics {
		std::uint64_t connectedDevices = 0;
		std::uint64_t averageConnectionTime = 0;
		std::uint64_t connectingDevices = 0;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct Statistics {
		std::string SerialNumber;
		uint64_t 	UUID = 0 ;
		std::string Data;
		uint64_t 	Recorded = 0;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct HealthCheck {
		std::string SerialNumber;
		uint64_t 	UUID = 0 ;
		std::string Data;
		uint64_t 	Recorded = 0 ;
		uint64_t 	Sanity = 0 ;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct Capabilities {
		std::string Capabilities;
		uint64_t 	FirstUpdate = 0 ;
		uint64_t 	LastUpdate = 0 ;
		void 		to_json(Poco::JSON::Object &Obj) const;
	};

	struct DeviceLog {
		enum Level {
			LOG_EMERG = 0,	 /* system is unusable */
			LOG_ALERT = 1,	 /* action must be taken immediately */
			LOG_CRIT = 2,	 /* critical conditions */
			LOG_ERR = 3,	 /* error conditions */
			LOG_WARNING = 4, /* warning conditions */
			LOG_NOTICE = 5,	 /* normal but significant condition */
			LOG_INFO = 6,	 /* informational */
			LOG_DEBUG = 7	 /* debug-level messages */
		};
		std::string SerialNumber;
		std::string Log;
		std::string Data;
		uint64_t 	Severity = 0 ;
		uint64_t 	Recorded = 0 ;
		uint64_t 	LogType = 0 ;
		uint64_t 	UUID = 0 ;
		void 		to_json(Poco::JSON::Object &Obj) const;
	};

	struct DefaultConfiguration {
		std::string Name;
		std::string Configuration;
		Types::StringVec Models;
		std::string Description;
		uint64_t 	Created;
		uint64_t 	LastModified;
		void 		to_json(Poco::JSON::Object &Obj) const;
		bool 		from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct CommandDetails {
		std::string UUID;
		std::string SerialNumber;
		std::string Command;
		std::string Status;
		std::string SubmittedBy;
		std::string Results;
		std::string Details;
		std::string ErrorText;
		uint64_t Submitted = time(nullptr);
		uint64_t Executed = 0;
		uint64_t Completed = 0 ;
		uint64_t RunAt = 0 ;
		uint64_t ErrorCode = 0 ;
		uint64_t Custom = 0 ;
		uint64_t WaitingForFile = 0 ;
		uint64_t AttachDate = 0 ;
		uint64_t AttachSize = 0 ;
		std::string AttachType;
		double 		executionTime = 0.0;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct BlackListedDevice {
		std::string serialNumber;
		std::string reason;
		std::string author;
		uint64_t created;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct RttySessionDetails {
		std::string SerialNumber;
		std::string Server;
		uint64_t 	Port = 0 ;
		std::string Token;
		uint64_t 	TimeOut = 0 ;
		std::string ConnectionId;
		uint64_t 	Started = 0 ;
		std::string CommandUUID;
		uint64_t 	ViewPort = 0 ;
		std::string DevicePassword;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct Dashboard {
		uint64_t 		  snapshot = 0 ;
		uint64_t 		  numberOfDevices = 0 ;
		Types::CountedMap commands;
		Types::CountedMap upTimes;
		Types::CountedMap memoryUsed;
		Types::CountedMap load1;
		Types::CountedMap load5;
		Types::CountedMap load15;
		Types::CountedMap vendors;
		Types::CountedMap status;
		Types::CountedMap deviceType;
		Types::CountedMap healths;
		Types::CountedMap certificates;
		Types::CountedMap lastContact;
		Types::CountedMap associations;
		void to_json(Poco::JSON::Object &Obj) const;
		void reset();
	};

	struct CapabilitiesModel {
		std::string deviceType;
		std::string capabilities;

		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct ScriptEntry {
		std::string 		id;
		std::string 		name;
		std::string 		description;
		std::string 		uri;
		std::string 		content;
		std::string 		version;
		std::string 		type;
		std::uint64_t 		created;
		std::uint64_t 		modified;
		std::string 		author;
		Types::StringVec 	restricted;
		bool				deferred=false;
		std::uint64_t 		timeout=30;
		std::string 		defaultUploadURI;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct ScriptEntryList {
		std::vector<ScriptEntry>	scripts;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct ScriptRequest {
		std::string serialNumber;
		uint64_t 	timeout=30;
		std::string type;
		std::string script;
		std::string scriptId;
		std::uint64_t when;
		std::string signature;
		bool deferred;
		std::string uri;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct RadiusProxyServerEntry {
		std::string name;
		std::string ip;
		uint16_t 	port=0;
		uint64_t 	weight=0;
		std::string secret;
		std::string certificate;
		bool 		radsec=false;
		uint16_t 	radsecPort=2083;
		std::string radsecSecret;
		std::string radsecKey;
		std::string radsecCert;
		std::vector<std::string> 	radsecCacerts;
		std::vector<std::string>	radsecRealms;
		bool 		ignore=false;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct RadiusProxyServerConfig {
		std::string 	strategy;
		bool 			monitor=false;
		std::string 	monitorMethod;
		std::vector<std::string>	methodParameters;
		std::vector<RadiusProxyServerEntry>	servers;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct 	RadiusProxyPool {
		std::string name;
		std::string description;
		RadiusProxyServerConfig	authConfig;
		RadiusProxyServerConfig	acctConfig;
		RadiusProxyServerConfig	coaConfig;
		bool 		useByDefault=false;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct RadiusProxyPoolList {
		std::vector<RadiusProxyPool>	pools;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};
}
