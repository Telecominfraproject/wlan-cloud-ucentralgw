//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "AP_WS_Server.h"
#include "StorageService.h"
#include "FindCountry.h"
#include "framework/WebSocketClientNotifications.h"
#include "Daemon.h"

namespace OpenWifi {

void AP_WS_Connection::Process_connect(Poco::JSON::Object::Ptr ParamsObj, const std::string &Serial) {
	if (ParamsObj->has(uCentralProtocol::UUID) &&
		ParamsObj->has(uCentralProtocol::FIRMWARE) &&
		ParamsObj->has(uCentralProtocol::CAPABILITIES)) {
		uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
		auto Firmware = ParamsObj->get(uCentralProtocol::FIRMWARE).toString();
		auto Capabilities = ParamsObj->get(uCentralProtocol::CAPABILITIES).toString();

		//// change this
		SerialNumber_ = Serial;
		SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);
		DeviceRegistry()->SetSessionDetails(ConnectionId_,this,SerialNumberInt_);
		State_.UUID = UUID;
		State_.Firmware = Firmware;
		State_.PendingUUID = 0;
		State_.LastContact = OpenWifi::Now();
		State_.Address = Utils::FormatIPv6(WS_->peerAddress().toString());
		CId_ = SerialNumber_ + "@" + CId_;
		//	We need to verify the certificate if we have one
		if(State_.VerifiedCertificate == GWObjects::VALID_CERTIFICATE) {
			if ((	Utils::SerialNumberMatch(CN_, SerialNumber_)) ||
					AP_WS_Server()->IsSimSerialNumber(CN_)) {
				State_.VerifiedCertificate = GWObjects::VERIFIED;
				poco_information(Logger(), fmt::format("CONNECT({}): Fully validated and authenticated device.", CId_));
			} else {
				State_.VerifiedCertificate = GWObjects::MISMATCH_SERIAL;
				poco_information(Logger(),
								 fmt::format("CONNECT({}): Serial number mismatch. CN={} Serial={}", CId_, CN_, SerialNumber_));
			}
		}

		auto IP = PeerAddress_.toString();
		if(IP.substr(0,7)=="::ffff:") {
			IP = IP.substr(7);
		}
		State_.locale = FindCountryFromIP()->Get(IP);
		GWObjects::Device	DeviceInfo;
		auto DeviceExists = StorageService()->GetDevice(SerialNumber_,DeviceInfo);
		// std::cout << "Connecting: " << SerialNumber_ << std::endl;
		if (Daemon()->AutoProvisioning() && !DeviceExists) {
			StorageService()->CreateDefaultDevice(SerialNumber_, Capabilities, Firmware,
												  Compatible_, PeerAddress_);
		} else if (DeviceExists) {
			StorageService()->UpdateDeviceCapabilities(SerialNumber_, Capabilities,
													   Compatible_);
			bool Updated = false;
			if(!Firmware.empty() && Firmware!=DeviceInfo.Firmware) {
				DeviceInfo.Firmware = Firmware;
				Updated = true;
				WebSocketClientNotificationDeviceFirmwareUpdated(SerialNumber_, Firmware);
			}

			if(DeviceInfo.locale != State_.locale) {
				DeviceInfo.locale = State_.locale;
				Updated = true;
			}

			if(Compatible_ != DeviceInfo.DeviceType) {
				DeviceInfo.DeviceType = Compatible_;
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

		WebSocketClientNotificationDeviceConnected(SerialNumber_);

		if (KafkaManager()->Enabled()) {
			Poco::JSON::Stringifier Stringify;
			ParamsObj->set(uCentralProtocol::CONNECTIONIP, CId_);
			ParamsObj->set("locale", State_.locale );
			ParamsObj->set(uCentralProtocol::TIMESTAMP, OpenWifi::Now());
			std::ostringstream OS;
			Stringify.condense(ParamsObj, OS);
			KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber_, OS.str());
		}

		std::cout << "Finished process connect" << std::endl;

	} else {
		poco_warning(Logger(),fmt::format("INVALID-PROTOCOL({}): Missing one of uuid, firmware, or capabilities", CId_));
		Errors_++;
		std::cout << "Error Finished process connect" << std::endl;
	}
}


}