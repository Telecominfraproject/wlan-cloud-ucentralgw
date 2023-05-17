//
// Created by stephane bourque on 2023-02-03.
//

#pragma once

#include <string>

#include <Poco/JSON/Object.h>
#include <RESTObjects/RESTAPI_GWobjects.h>
#include <framework/KafkaManager.h>

namespace OpenWifi {

	class GWKafkaEvents {
	  public:
		GWKafkaEvents(std::uint64_t serialNumber, const std::string &type,
					  std::uint64_t timestamp)
			: serialNumber_(serialNumber), type_(type), timestamp_(timestamp) {
			payload_ = Poco::SharedPtr<Poco::JSON::Object>();
		}
		void Send();
		[[nodiscard]] inline std::uint64_t Serial() const { return serialNumber_;};

	  protected:
		std::uint64_t serialNumber_;
		std::string type_;
		std::uint64_t timestamp_ = 0;
		Poco::JSON::Object::Ptr payload_;
	};

	class DeviceFirmwareChangeKafkaEvent : public GWKafkaEvents {
	  public:
		DeviceFirmwareChangeKafkaEvent(std::uint64_t serialNumber, std::uint64_t timestamp,
									   const std::string &oldFirmware,
									   const std::string &newFirmware)
			: GWKafkaEvents(serialNumber, "unit.firmware_change", timestamp),
			  oldFirmware_(oldFirmware), newFirmware_(newFirmware) {}

		~DeviceFirmwareChangeKafkaEvent() {
			payload_->set("oldFirmware", oldFirmware_);
			payload_->set("newFirmware", newFirmware_);
			Send();
		}

	  private:
		std::string oldFirmware_, newFirmware_;
	};

	class DeviceConfigurationChangeKafkaEvent : public GWKafkaEvents {
	  public:
		DeviceConfigurationChangeKafkaEvent(std::uint64_t serialNumber,
											std::uint64_t timestamp, const std::string config)
			: GWKafkaEvents(serialNumber, "unit.configuration_change", timestamp), config_(config) {
		}

		~DeviceConfigurationChangeKafkaEvent() {
			payload_->set("configuration", config_);
			Send();
		}

	  private:
		std::string config_;
	};

	class DeviceBlacklistedKafkaEvent : public GWKafkaEvents {
	  public:
		explicit DeviceBlacklistedKafkaEvent(std::uint64_t serialNumber,
											 std::uint64_t timestamp, const std::string &reason,
											 const std::string &author, std::uint64_t created,
											 std::string &IP)
			: GWKafkaEvents(serialNumber, "blacklisted_device", timestamp), reason_(reason),
			  author_(author), created_(created), IP_(IP) {}

		~DeviceBlacklistedKafkaEvent() {
			payload_->set("reason", reason_);
			payload_->set("author", author_);
			payload_->set("created", created_);
			payload_->set("ipaddress", IP_);
			Send();
		}

	  private:
		std::string reason_, author_;
		std::uint64_t created_;
		std::string IP_;
	};

	class DeviceLogKafkaEvent : public GWKafkaEvents {
	  public:
		explicit DeviceLogKafkaEvent( const GWObjects::DeviceLog &L)
			: GWKafkaEvents(Utils::MACToInt(L.SerialNumber), "device_log", L.Recorded),
			  DL_(L)
		{
		}

		~DeviceLogKafkaEvent() {
			DL_.to_json(*payload_);
			Send();
		}

	  private:
		GWObjects::DeviceLog	DL_;
	};

} // namespace OpenWifi
