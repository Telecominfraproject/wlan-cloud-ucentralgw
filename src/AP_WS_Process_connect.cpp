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
		CN_ = SerialNumber_ = Serial;
		SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);
		Conn_ = DeviceRegistry()->Register(SerialNumberInt_, this, ConnectionId_);
		Conn_->Conn_.UUID = UUID;
		Conn_->Conn_.Firmware = Firmware;
		Conn_->Conn_.PendingUUID = 0;
		Conn_->Conn_.LastContact = OpenWifi::Now();
		Conn_->Conn_.Address = Utils::FormatIPv6(WS_->peerAddress().toString());
		CId_ = SerialNumber_ + "@" + CId_;
		//	We need to verify the certificate if we have one
		if ((!CN_.empty() && Utils::SerialNumberMatch(CN_, SerialNumber_)) ||
			AP_WS_Server()->IsSimSerialNumber(CN_)) {
			CertValidation_ = GWObjects::VERIFIED;
			poco_information(Logger(), fmt::format("CONNECT({}): Fully validated and authenticated device.", CId_));
		} else {
			if (CN_.empty())
				poco_information(Logger(), fmt::format("CONNECT({}): Not authenticated or validated.", CId_));
			else
				poco_information(Logger(), fmt::format(
											   "CONNECT({}): Authenticated but not validated. Serial='{}' CN='{}'", CId_,
											   Serial, CN_));
		}
		Conn_->Conn_.VerifiedCertificate = CertValidation_;
		auto IP = PeerAddress_.toString();
		if(IP.substr(0,7)=="::ffff:") {
			IP = IP.substr(7);
		}
		Conn_->Conn_.locale = FindCountryFromIP()->Get(IP);
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

			if(DeviceInfo.locale != Conn_->Conn_.locale) {
				DeviceInfo.locale = Conn_->Conn_.locale;
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
			Conn_->Conn_.UUID = UpgradedUUID;
		}
		Conn_->Conn_.Compatible = Compatible_;

		WebSocketClientNotificationDeviceConnected(SerialNumber_);

		if (KafkaManager()->Enabled()) {
			Poco::JSON::Stringifier Stringify;
			ParamsObj->set(uCentralProtocol::CONNECTIONIP, CId_);
			ParamsObj->set("locale", Conn_->Conn_.locale );
			ParamsObj->set(uCentralProtocol::TIMESTAMP, OpenWifi::Now());
			std::ostringstream OS;
			Stringify.condense(ParamsObj, OS);
			KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber_, OS.str());
		}
		Connected_ = true;
	} else {
		poco_warning(Logger(),fmt::format("INVALID-PROTOCOL({}): Missing one of uuid, firmware, or capabilities", CId_));
		Errors_++;
	}
}


}