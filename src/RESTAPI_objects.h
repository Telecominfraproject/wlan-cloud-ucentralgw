//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_OBJECTS_H
#define UCENTRAL_RESTAPI_OBJECTS_H

#include "Poco/JSON/Object.h"

namespace uCentral::Objects {

	struct AclTemplate {
		bool Read_ = true ;
		bool ReadWrite_ = true ;
		bool ReadWriteCreate_ = true ;
		bool Delete_ = true ;
		bool PortalLogin_ = true ;
		void to_json(Poco::JSON::Object &Obj) const ;
	};

	struct WebToken {
		std::string access_token_;
		std::string refresh_token_;
		std::string id_token_;
		std::string token_type_;
		std::string username_;
		unsigned int expires_in_;
		unsigned int idle_timeout_;
		AclTemplate acl_template_;
		uint64_t    created_;
		void to_json(Poco::JSON::Object &Obj) const ;
	};

	struct ConnectionState {
		uint64_t MessageCount;
		std::string SerialNumber;
		std::string Address;
		uint64_t UUID;
		uint64_t PendingUUID;
		uint64_t TX, RX;
		bool Connected;
		uint64_t LastContact;
		std::string Firmware;
		bool VerifiedCertificate;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct Device {
		std::string SerialNumber;
		std::string DeviceType;
		std::string MACAddress;
		std::string Manufacturer;
		uint64_t UUID;
		std::string Configuration;
		std::string Notes;
		uint64_t CreationTimestamp;
		uint64_t LastConfigurationChange;
		uint64_t LastConfigurationDownload;
		std::string Owner;
		std::string Location;
		std::string Firmware;
		void to_json(Poco::JSON::Object &Obj) const;
		void to_json_with_status(Poco::JSON::Object &Obj) const;
		bool from_json(Poco::JSON::Object::Ptr Obj);
		void Print() const;
	};

	struct Statistics {
		uint64_t 	UUID;
		std::string Data;
		uint64_t 	Recorded;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct HealthCheck {
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
		std::string Log;
		std::string Data;
		uint64_t 	Severity;
		uint64_t 	Recorded;
		uint64_t 	LogType;
		void 		to_json(Poco::JSON::Object &Obj) const;
	};

	struct DefaultConfiguration {
		std::string Name;
		std::string Configuration;
		std::string Models;
		std::string Description;
		uint64_t 	Created;
		uint64_t 	LastModified;
		void 		to_json(Poco::JSON::Object &Obj) const;
		bool 		from_json(Poco::JSON::Object::Ptr Obj);
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
		uint64_t Submitted;
		uint64_t Executed;
		uint64_t Completed;
		uint64_t RunAt;
		uint64_t ErrorCode;
		uint64_t Custom;
		uint64_t WaitingForFile;
		uint64_t AttachDate;
		uint64_t AttachSize;
		std::string AttachType;
		void to_json(Poco::JSON::Object &Obj) const;
	};

	struct BlackListedDevice {
		std::string SerialNumber;
		std::string Reason;
		std::string Author;
		uint64_t Created;
		void to_json(Poco::JSON::Object &Obj) const;
	};
}

#endif //UCENTRAL_RESTAPI_OBJECTS_H
