//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"
#include "CentralConfig.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/Data/RecordSet.h"
#include "CapabilitiesCache.h"

namespace OpenWifi {

	bool Storage::UpdateDeviceCapabilities(std::string &SerialNumber, std::string & Capabilities, std::string & Compat) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   UpSert(Sess);

			uint64_t Now = std::time(nullptr);
			OpenWifi::Config::Capabilities	Caps(Capabilities);
			Compat = Caps.Compatible();
			if(!Caps.Compatible().empty() && !Caps.Platform().empty())
				CapabilitiesCache::instance()->Add(Caps.Compatible(),Caps.Platform());

			std::string St{"insert into Capabilities (SerialNumber, Capabilities, FirstUpdate, LastUpdate) values(?,?,?,?) on conflict (SerialNumber) do "
						   " update set Capabilities=?, LastUpdate=?"};
			UpSert << ConvertParams(St),
				Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(Capabilities),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Capabilities),
				Poco::Data::Keywords::use(Now);
			UpSert.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetDeviceCapabilities(std::string &SerialNumber, GWObjects::Capabilities &Caps) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string TmpSerialNumber;

			std::string St{"SELECT SerialNumber, Capabilities, FirstUpdate, LastUpdate FROM Capabilities WHERE SerialNumber=?"};

			Select  << ConvertParams(St),
				Poco::Data::Keywords::into(TmpSerialNumber),
				Poco::Data::Keywords::into(Caps.Capabilities),
				Poco::Data::Keywords::into(Caps.FirstUpdate),
				Poco::Data::Keywords::into(Caps.LastUpdate),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if (TmpSerialNumber.empty())
				return false;

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::DeleteDeviceCapabilities(std::string &SerialNumber) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Delete(Sess);

			std::string St{"DELETE FROM Capabilities WHERE SerialNumber=?"};

			Delete << ConvertParams(St),
				Poco::Data::Keywords::use(SerialNumber);
			Delete.execute();

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::InitCapabilitiesCache() {
		try {

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string st{"select capabilities from Capabilities"};
			Select << st;
			Select.execute();

			Poco::Data::RecordSet   RSet(Select);
			bool More = RSet.moveFirst();
			while(More) {
				auto Caps = RSet[0].convert<std::string>();
				try {
					Poco::JSON::Parser	P;
					auto RawObject = P.parse(Caps).extract<Poco::JSON::Object::Ptr>();
					std::string Compatible = RawObject->get("compatible").toString();
					CapsCache_[Compatible] = Caps;
				} catch (...) {

				}
				More = RSet.moveNext();
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}
}