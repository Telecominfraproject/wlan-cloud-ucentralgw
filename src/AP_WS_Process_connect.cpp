//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "AP_WS_Server.h"
#include "StorageService.h"
#include "FindCountry.h"
#include "Daemon.h"
#include "CentralConfig.h"

#include "CommandManager.h"

#include "framework/KafkaManager.h"
#include "framework/utils.h"

#include "UI_GW_WebSocketNotifications.h"

namespace OpenWifi {

	void AP_WS_Connection::Process_connect(Poco::JSON::Object::Ptr ParamsObj, const std::string &Serial) {
		if (ParamsObj->has(uCentralProtocol::UUID) &&
			ParamsObj->has(uCentralProtocol::FIRMWARE) &&
			ParamsObj->has(uCentralProtocol::CAPABILITIES)) {
			uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
			auto Firmware = ParamsObj->get(uCentralProtocol::FIRMWARE).toString();
			auto CapabilitiesString = ParamsObj->get(uCentralProtocol::CAPABILITIES).toString();

			SerialNumber_ = Serial;
			SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);

			CommandManager()->ClearQueue(SerialNumberInt_);

			std::unique_lock	Lock(LocalMutex_);
			Config::Capabilities Caps(CapabilitiesString);
			Compatible_ = Caps.Compatible();


			AP_WS_Server()->SetSessionDetails(State_.sessionId,SerialNumberInt_);
			State_.UUID = UUID;
			State_.Firmware = Firmware;
			State_.PendingUUID = 0;
			State_.Address = Utils::FormatIPv6(WS_->peerAddress().toString());
			CId_ = SerialNumber_ + "@" + CId_;

			auto IP = PeerAddress_.toString();
			if(IP.substr(0,7)=="::ffff:") {
				IP = IP.substr(7);
			}

			bool RestrictedDevice = false;
			if(ParamsObj->has("restricted")) {
				auto FCCValue = ParamsObj->get("restricted");
				if(FCCValue.isBoolean()) {
					RestrictedDevice = FCCValue;
				}
			}

			State_.locale = FindCountryFromIP()->Get(IP);
			GWObjects::Device	DeviceInfo;
			auto DeviceExists = StorageService()->GetDevice(SerialNumber_,DeviceInfo);
			if (Daemon()->AutoProvisioning() && !DeviceExists) {
				StorageService()->CreateDefaultDevice(SerialNumber_, CapabilitiesString, Firmware,
													  Compatible_, PeerAddress_);
			} else if (DeviceExists) {
				StorageService()->UpdateDeviceCapabilities(SerialNumber_, CapabilitiesString,
														   Compatible_);
				bool Updated = false;
				if(!Firmware.empty()) {
					if(Firmware!=DeviceInfo.Firmware) {
						DeviceInfo.Firmware = Firmware;
						DeviceInfo.LastFWUpdate = Utils::Now();
						Updated = true;

						GWWebSocketNotifications::SingleDeviceFirmwareChange_t	Notification;
						Notification.content.serialNumber = SerialNumber_;
						Notification.content.newFirmware = Firmware;
						GWWebSocketNotifications::DeviceFirmwareUpdated(Notification);
					} else if(DeviceInfo.LastFWUpdate==0) {
						DeviceInfo.LastFWUpdate = Utils::Now();
						Updated = true;
					}
				}

				if(DeviceInfo.locale != State_.locale) {
					DeviceInfo.locale = State_.locale;
					Updated = true;
				}

				if(Compatible_ != DeviceInfo.DeviceType) {
					DeviceInfo.DeviceType = Compatible_;
					Updated = true;
				}

				if(RestrictedDevice != DeviceInfo.restrictedDevice) {
					DeviceInfo.restrictedDevice = RestrictedDevice;
					Updated = true;
				}

				if(Updated) {
					StorageService()->UpdateDevice(DeviceInfo);
				}
				uint64_t UpgradedUUID=0;
				LookForUpgrade(UUID,UpgradedUUID);
				State_.UUID = UpgradedUUID;
			}

			State_.Compatible = Compatible_;
			State_.Connected = true;
			ConnectionCompletionTime_ = std::chrono::high_resolution_clock::now() - ConnectionStart_;
			State_.connectionCompletionTime = ConnectionCompletionTime_.count();

			if(State_.VerifiedCertificate == GWObjects::VALID_CERTIFICATE) {
				if ((	Utils::SerialNumberMatch(CN_, SerialNumber_, AP_WS_Server()->MismatchDepth())) ||
						AP_WS_Server()->IsSimSerialNumber(CN_)) {
					State_.VerifiedCertificate = GWObjects::VERIFIED;
					poco_information(Logger_, fmt::format("CONNECT({}): Fully validated and authenticated device. Session={} ConnectionCompletion Time={}",
														   CId_,
														   State_.sessionId,
														   State_.connectionCompletionTime ));
				} else {
					State_.VerifiedCertificate = GWObjects::MISMATCH_SERIAL;
					if(AP_WS_Server()->AllowSerialNumberMismatch()) {
						poco_information(
							Logger_, fmt::format("CONNECT({}): Serial number mismatch allowed. CN={} Serial={} Session={} ConnectionCompletion Time={}",
												 CId_, CN_, SerialNumber_, State_.sessionId,
												 State_.connectionCompletionTime));
					} else {
						poco_information(
							Logger_, fmt::format("CONNECT({}): Serial number mismatch disallowed. Device rejected. CN={} Serial={} Session={} ConnectionCompletion Time={}",
												 CId_, CN_, SerialNumber_, State_.sessionId,
												 State_.connectionCompletionTime));
						return EndConnection();
					}
				}
			}

			GWWebSocketNotifications::SingleDevice_t	Notification;
			Notification.content.serialNumber = SerialNumber_;
			GWWebSocketNotifications::DeviceConnected(Notification);

			// std::cout << "Serial: " << SerialNumber_ << "Session: " << State_.sessionId << std::endl;

			if (KafkaManager()->Enabled()) {
				Poco::JSON::Stringifier Stringify;
				ParamsObj->set(uCentralProtocol::CONNECTIONIP, CId_);
				ParamsObj->set("locale", State_.locale );
				ParamsObj->set(uCentralProtocol::TIMESTAMP, Utils::Now());
				std::ostringstream OS;
				Stringify.condense(ParamsObj, OS);
				KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber_, OS.str());
			}
		} else {
			poco_warning(Logger_,fmt::format("INVALID-PROTOCOL({}): Missing one of uuid, firmware, or capabilities", CId_));
			Errors_++;
		}
	}

}