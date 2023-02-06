//
// Created by stephane bourque on 2023-02-03.
//

#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include <framework/KafkaManager.h>

namespace OpenWifi {

	class GWKafkaEvents {
	  public:
		GWKafkaEvents(const std::string &serialNumber, const std::string &type, std::uint64_t timestamp) :
			serialNumber_(serialNumber),
			type_(type),
			timestamp_(timestamp) {
		}

		inline void SetPayload(Poco::JSON::Object::Ptr payload) {
			payload_ = std::move(payload);
		}
		void Send();

	  private:
		std::string 	serialNumber_;
		std::string 	type_;
		std::uint64_t 	timestamp_=0;
		Poco::JSON::Object::Ptr payload_;
	};

	class DeviceFirmwareChangeKafkaEvent : public GWKafkaEvents {
	  public:
		DeviceFirmwareChangeKafkaEvent( const std::string &serialNumber, std::uint64_t timestamp, const std::string &oldFirmware, const std::string &newFirmware) :
			GWKafkaEvents(serialNumber,"device.firmware_change", timestamp),
			oldFirmware_(oldFirmware),
			newFirmware_(newFirmware) {
		}

		~DeviceFirmwareChangeKafkaEvent() {
			Poco::JSON::Object::Ptr payload = new Poco::JSON::Object;
			payload->set("oldFirmware", oldFirmware_);
			payload->set("newFirmware",newFirmware_);
			SetPayload(payload);
			Send();
		}

		private:
			std::string oldFirmware_, newFirmware_;
	};

	class DeviceConfigurationChangeKafkaEvent : public GWKafkaEvents {
	  public:
		DeviceConfigurationChangeKafkaEvent( const std::string &serialNumber, std::uint64_t timestamp, const std::string config) :
			GWKafkaEvents(serialNumber,"device.configuration_change", timestamp),
			config_(config) {
		}

		~DeviceConfigurationChangeKafkaEvent() {
			Poco::JSON::Object::Ptr payload = new Poco::JSON::Object;
			payload->set("configuration", config_);
			SetPayload(payload);
			Send();
		}

	  private:
		std::string config_;
	};

	class DeviceBlacklistedKafkaEvent : public GWKafkaEvents {
	  public:
		explicit DeviceBlacklistedKafkaEvent( const std::string &serialNumber, std::uint64_t timestamp, const std::string &reason, const std::string &author, std::uint64_t created, std::string &IP) :
		  GWKafkaEvents(serialNumber,"blacklisted_device", timestamp),
		  reason_(reason), author_(author), created_(created), IP_(IP)
		{
		}

		~DeviceBlacklistedKafkaEvent() {
			Poco::JSON::Object::Ptr payload = new Poco::JSON::Object;
			payload->set("reason", reason_);
			payload->set("author", author_);
			payload->set("created", created_);
			payload->set("ipaddress", IP_);
			SetPayload(payload);
			Send();
		}

	  private:
		std::string 	reason_,author_;
		std::uint64_t 	created_;
		std::string 	IP_;
	};

} // namespace OpenWifi


