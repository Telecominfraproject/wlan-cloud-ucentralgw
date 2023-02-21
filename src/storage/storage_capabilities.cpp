//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "CapabilitiesCache.h"
#include "CentralConfig.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/JSON/Object.h"
#include "StorageService.h"
#include "framework/utils.h"

#include "fmt/format.h"

namespace OpenWifi {

	bool Storage::CreateDeviceCapabilities(std::string &SerialNumber,
										   const Config::Capabilities &Capabilities) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement UpSert(Sess);

			std::string TCaps{Capabilities.AsString()};
			uint64_t Now = Utils::Now();
			std::string St{"insert into Capabilities (SerialNumber, Capabilities, FirstUpdate, "
						   "LastUpdate) values(?,?,?,?) on conflict (SerialNumber) do "
						   " update set Capabilities=?, LastUpdate=?"};
			UpSert << ConvertParams(St), Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(TCaps), Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Now), Poco::Data::Keywords::use(TCaps),
				Poco::Data::Keywords::use(Now);
			UpSert.execute();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::UpdateDeviceCapabilities(std::string &SerialNumber,
										   const Config::Capabilities &Caps) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement UpSert(Sess);

			uint64_t Now = Utils::Now();
			if (!Caps.Compatible().empty() && !Caps.Platform().empty())
				CapabilitiesCache()->Add(Caps);

			std::string TCaps{Caps.AsString()};

			std::string St{"insert into Capabilities (SerialNumber, Capabilities, FirstUpdate, "
						   "LastUpdate) values(?,?,?,?) on conflict (SerialNumber) do "
						   " update set Capabilities=?, LastUpdate=?"};
			UpSert << ConvertParams(St), Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(TCaps), Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Now), Poco::Data::Keywords::use(TCaps),
				Poco::Data::Keywords::use(Now);
			UpSert.execute();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::GetDeviceCapabilities(std::string &SerialNumber, GWObjects::Capabilities &Caps) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string TmpSerialNumber;

			std::string St{"SELECT SerialNumber, Capabilities, FirstUpdate, LastUpdate FROM "
						   "Capabilities WHERE SerialNumber=?"};

			Select << ConvertParams(St), Poco::Data::Keywords::into(TmpSerialNumber),
				Poco::Data::Keywords::into(Caps.Capabilities),
				Poco::Data::Keywords::into(Caps.FirstUpdate),
				Poco::Data::Keywords::into(Caps.LastUpdate),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if (TmpSerialNumber.empty())
				return false;

			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::DeleteDeviceCapabilities(std::string &SerialNumber) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			std::string St{"DELETE FROM Capabilities WHERE SerialNumber=?"};

			Delete << ConvertParams(St), Poco::Data::Keywords::use(SerialNumber);
			Delete.execute();

			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

} // namespace OpenWifi