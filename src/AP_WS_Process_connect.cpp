//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "AP_WS_Server.h"
#include "CentralConfig.h"
#include "Daemon.h"
#include "FindCountry.h"
#include "StorageService.h"

#include "CommandManager.h"

#include "framework/KafkaManager.h"
#include "framework/utils.h"

#include "UI_GW_WebSocketNotifications.h"
#include <GWKafkaEvents.h>

namespace OpenWifi {

	[[maybe_unused]] static void SendKafkaFirmwareUpdate(const std::string &SerialNumber,
														 const std::string &OldFirmware,
														 const std::string &NewFirmware) {
		if (KafkaManager()->Enabled()) {
			Poco::JSON::Object EventDetails;
			EventDetails.set("oldFirmware", OldFirmware);
			EventDetails.set("newFirmware", NewFirmware);
			Poco::JSON::Object Event;
			Event.set("type", "device.firmware_change");
			Event.set("timestamp", Utils::Now());
			Event.set("payload", EventDetails);
			std::ostringstream OS;
			Event.stringify(OS);
			KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, SerialNumber, OS.str());
		}
	}

	[[maybe_unused]] static void SendKafkaDeviceNotProvisioned(	const std::string &SerialNumber,
														 		const std::string &Firmware,
															   	const std::string &DeviceType,
															   	const std::string &IP) {
		if (KafkaManager()->Enabled()) {
			Poco::JSON::Object EventDetails;
			EventDetails.set("firmware", Firmware);
			EventDetails.set("deviceType", DeviceType);
			EventDetails.set("IP", IP);
			Poco::JSON::Object Event;
			Event.set("type", "device.not_provisioned");
			Event.set("timestamp", Utils::Now());
			Event.set("payload", EventDetails);
			std::ostringstream OS;
			Event.stringify(OS);
			KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, SerialNumber, OS.str());
		}
	}

	void AP_WS_Connection::Process_connect(Poco::JSON::Object::Ptr ParamsObj,
										   const std::string &Serial) {
		if (ParamsObj->has(uCentralProtocol::UUID) && ParamsObj->has(uCentralProtocol::FIRMWARE) &&
			ParamsObj->has(uCentralProtocol::CAPABILITIES)) {
			uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
			auto Firmware = ParamsObj->get(uCentralProtocol::FIRMWARE).toString();
			auto Capabilities = ParamsObj->getObject(uCentralProtocol::CAPABILITIES);

			SerialNumber_ = Serial;
			SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);

			CommandManager()->ClearQueue(SerialNumberInt_);

			AP_WS_Server()->SetSessionDetails(State_.sessionId, SerialNumberInt_);

			std::lock_guard Lock(ConnectionMutex_);
			Config::Capabilities Caps(Capabilities);

			Compatible_ = Caps.Compatible();

			State_.UUID = UUID;
			State_.Firmware = Firmware;
			State_.PendingUUID = 0;
			State_.Address = Utils::FormatIPv6(WS_->peerAddress().toString());
			CId_ = SerialNumber_ + "@" + CId_;

			auto IP = PeerAddress_.toString();
			if (IP.substr(0, 7) == "::ffff:") {
				IP = IP.substr(7);
			}

			bool RestrictedDevice = false;
			if (Capabilities->has("restrictions")) {
				RestrictedDevice = true;
				Poco::JSON::Object::Ptr RestrictionObject = Capabilities->getObject("restrictions");
				Restrictions_.from_json(RestrictionObject);
			}

			if (Capabilities->has("developer")) {
				Restrictions_.developer = Capabilities->getValue<bool>("developer");
			}

			if(Capabilities->has("secure-rtty")) {
				RttyMustBeSecure_ = Capabilities->getValue<bool>("secure-rtty");
			}

			State_.locale = FindCountryFromIP()->Get(IP);
			GWObjects::Device DeviceInfo;
			auto DeviceExists = StorageService()->GetDevice(SerialNumber_, DeviceInfo);
			if (Daemon()->AutoProvisioning() && !DeviceExists) {
				StorageService()->CreateDefaultDevice(SerialNumber_, Caps, Firmware, PeerAddress_);
			} else if (!Daemon()->AutoProvisioning() && !DeviceExists) {
				SendKafkaDeviceNotProvisioned(SerialNumber_, Firmware, Compatible_, CId_);
				poco_warning(Logger(),fmt::format("Device {} is a {} from {} and cannot be provisioned.",SerialNumber_,Compatible_, CId_));
				return EndConnection();
			} else if (DeviceExists) {
				StorageService()->UpdateDeviceCapabilities(SerialNumber_, Caps);
				int Updated{0};
				if (!Firmware.empty()) {
					if (Firmware != DeviceInfo.Firmware) {
						DeviceFirmwareChangeKafkaEvent KEvent(SerialNumber_, Utils::Now(),
															  DeviceInfo.Firmware, Firmware);
						DeviceInfo.Firmware = Firmware;
						DeviceInfo.LastFWUpdate = Utils::Now();
						++Updated;

						GWWebSocketNotifications::SingleDeviceFirmwareChange_t Notification;
						Notification.content.serialNumber = SerialNumber_;
						Notification.content.newFirmware = Firmware;
						GWWebSocketNotifications::DeviceFirmwareUpdated(Notification);
					} else if (DeviceInfo.LastFWUpdate == 0) {
						DeviceInfo.LastFWUpdate = Utils::Now();
						++Updated;
					}
				}

				if (DeviceInfo.simulated && (State_.VerifiedCertificate!=GWObjects::SIMULATED)) {
					DeviceInfo.simulated = false;
					++Updated;
				}

				if (!DeviceInfo.simulated && (State_.VerifiedCertificate==GWObjects::SIMULATED)) {
					DeviceInfo.simulated = true;
					++Updated;
				}

				if (DeviceInfo.locale != State_.locale) {
					DeviceInfo.locale = State_.locale;
					++Updated;
				}

				if (Compatible_ != DeviceInfo.DeviceType) {
					DeviceInfo.DeviceType = Compatible_;
					++Updated;
				}

				if (RestrictedDevice != DeviceInfo.restrictedDevice) {
					DeviceInfo.restrictedDevice = RestrictedDevice;
					++Updated;
				}

				if (Restrictions_ != DeviceInfo.restrictionDetails) {
					DeviceInfo.restrictionDetails = Restrictions_;
					++Updated;
				}

				if (Updated) {
					StorageService()->UpdateDevice(DeviceInfo);
				}

				uint64_t UpgradedUUID = 0;
				LookForUpgrade(UUID, UpgradedUUID);
				State_.UUID = UpgradedUUID;
			}

			State_.Compatible = Compatible_;
			State_.Connected = true;
			ConnectionCompletionTime_ =
				std::chrono::high_resolution_clock::now() - ConnectionStart_;
			State_.connectionCompletionTime = ConnectionCompletionTime_.count();

			if (State_.VerifiedCertificate == GWObjects::VALID_CERTIFICATE) {
				if ((Utils::SerialNumberMatch(CN_, SerialNumber_,
											  (int)AP_WS_Server()->MismatchDepth())) ||
					AP_WS_Server()->IsSimSerialNumber(CN_)) {
					State_.VerifiedCertificate = GWObjects::VERIFIED;
					poco_information(Logger_,
									 fmt::format("CONNECT({}): Fully validated and authenticated "
												 "device. Session={} ConnectionCompletion Time={}",
												 CId_, State_.sessionId,
												 State_.connectionCompletionTime));
				} else {
					State_.VerifiedCertificate = GWObjects::MISMATCH_SERIAL;
					if (AP_WS_Server()->AllowSerialNumberMismatch()) {
						poco_information(
							Logger_,
							fmt::format("CONNECT({}): Serial number mismatch allowed. CN={} "
										"Serial={} Session={} ConnectionCompletion Time={}",
										CId_, CN_, SerialNumber_, State_.sessionId,
										State_.connectionCompletionTime));
					} else {
						poco_information(
							Logger_, fmt::format("CONNECT({}): Serial number mismatch disallowed. "
												 "Device rejected. CN={} Serial={} Session={}",
												 CId_, CN_, SerialNumber_, State_.sessionId));
						return EndConnection();
					}
				}
			} else {
				poco_information(Logger_,
								 fmt::format("CONNECT({}): Simulator device. "
											 "Session={} ConnectionCompletion Time={}",
											 CId_, State_.sessionId,
											 State_.connectionCompletionTime));
			}

			GWWebSocketNotifications::SingleDevice_t Notification;
			Notification.content.serialNumber = SerialNumber_;
			GWWebSocketNotifications::DeviceConnected(Notification);

			if (KafkaManager()->Enabled()) {
				Poco::JSON::Stringifier Stringify;

				ParamsObj->set(uCentralProtocol::CONNECTIONIP, CId_);
				ParamsObj->set("locale", State_.locale);
				ParamsObj->set(uCentralProtocol::TIMESTAMP, Utils::Now());
				std::ostringstream OS;
				Stringify.condense(ParamsObj, OS);
				KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber_, OS.str());
			}
		} else {
			poco_warning(
				Logger_,
				fmt::format("INVALID-PROTOCOL({}): Missing one of uuid, firmware, or capabilities",
							CId_));
			Errors_++;
		}
	}

} // namespace OpenWifi