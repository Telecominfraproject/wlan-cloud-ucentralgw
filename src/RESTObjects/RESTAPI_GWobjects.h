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
		std::string SerialNumber;
		std::string Address;
		uint64_t UUID = 0 ;
		uint64_t PendingUUID = 0 ;
		uint64_t TX = 0, RX = 0;
		uint64_t Associations_2G=0;
		uint64_t Associations_5G=0;
		bool Connected = false;
		uint64_t LastContact=0;
		std::string Firmware;
		CertificateValidation VerifiedCertificate = NO_CERTIFICATE;
		std::string Compatible;
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
		uint64_t UUID;
		uint64_t CreationTimestamp;
		uint64_t LastConfigurationChange;
		uint64_t LastConfigurationDownload;
		uint64_t LastFWUpdate;
		std::string Venue;
		std::string DevicePassword;
		void to_json(Poco::JSON::Object &Obj) const;
		void to_json_with_status(Poco::JSON::Object &Obj) const;
		bool from_json(Poco::JSON::Object::Ptr &Obj);
		void Print() const;
	};

	struct Statistics {
		std::string SerialNumber;
		uint64_t 	UUID;
		std::string Data;
		uint64_t 	Recorded;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct HealthCheck {
		std::string SerialNumber;
		uint64_t 	UUID;
		std::string Data;
		uint64_t 	Recorded;
		uint64_t 	Sanity;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct Capabilities {
		std::string Capabilities;
		uint64_t 	FirstUpdate;
		uint64_t 	LastUpdate;
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
		uint64_t 	Severity;
		uint64_t 	Recorded;
		uint64_t 	LogType;
		uint64_t 	UUID;
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
		bool 		from_json(Poco::JSON::Object::Ptr &Obj);
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
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct BlackListedDevice {
		std::string serialNumber;
		std::string reason;
		std::string author;
		uint64_t created;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(Poco::JSON::Object::Ptr &Obj);
	};

	struct RttySessionDetails {
		std::string SerialNumber;
		std::string Server;
		uint64_t 	Port;
		std::string Token;
		uint64_t 	TimeOut;
		std::string ConnectionId;
		uint64_t 	Started;
		std::string CommandUUID;
		uint64_t 	ViewPort;
		std::string DevicePassword;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct Dashboard {
		uint64_t 		  snapshot;
		uint64_t 		  numberOfDevices;
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
}
