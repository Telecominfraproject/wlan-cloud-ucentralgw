//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Euphokumiko on 2025-05-20.
//	Accton Corp.
//

#include "CentralConfig.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "StorageService.h"
#include "framework/utils.h"

#include "fmt/format.h"

namespace OpenWifi {

	const static std::string DB_PackageSelectField{"SerialNumber, Packages, FirstUpdate, LastUpdate "};

	const static std::string DB_MYSQL_JSON_QUERY{"WHERE JSON_SEARCH(Packages, 'one', ?, NULL, '$[*].name') IS NOT NULL"};
	const static std::string DB_POSTGRES_JSON_QUERY{"WHERE Packages::jsonb @> '[{\"name\": \"?\"}]'"};

	// 					serial		 pkgs		  f_update  l_update
	typedef Poco::Tuple<std::string, std::string, uint64_t, uint64_t> PackageTuple;


	bool Storage::CreateDeviceInstalledPackages(std::string &SerialNumber,
												GWObjects::PackageList &Pkgs) {
		try {
			Poco::Data::Session Sess(Pool_->get());
			Poco::Data::Statement UpSert(Sess);

			uint64_t Now = Utils::Now();

			std::string St{
				"INSERT INTO DevicePackages ("
				+ DB_PackageSelectField + ") "
				"VALUES (?,?,?,?) "
				"ON CONFLICT (SerialNumber) DO "
				"UPDATE SET Packages = ?, LastUpdate = ?"};

			UpSert << ConvertParams(St), Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(Pkgs.packageStringArray),
				Poco::Data::Keywords::use(Now), Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Pkgs.packageStringArray),
				Poco::Data::Keywords::use(Now);
			UpSert.execute();
			Sess.commit();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::UpdateDeviceInstalledPackages(std::string &SerialNumber, GWObjects::PackageList &Pkgs) {
		return CreateDeviceInstalledPackages(SerialNumber, Pkgs);
	}

	bool Storage::GetDeviceInstalledPackages(std::string &SerialNumber,
											 GWObjects::PackageList &DevicePackages) {
		try {
			Poco::Data::Session Sess(Pool_->get());
			Poco::Data::Statement Select(Sess);

			PackageTuple packageTuple;
			std::string TmpSerialNumber;
			std::string packageStringArray;
			std::string St{"SELECT " + DB_PackageSelectField +
						   "FROM DevicePackages WHERE SerialNumber=?"};

			Select << ConvertParams(St), Poco::Data::Keywords::into(TmpSerialNumber),
				Poco::Data::Keywords::into(packageStringArray),
				Poco::Data::Keywords::into(DevicePackages.FirstUpdate),
				Poco::Data::Keywords::into(DevicePackages.LastUpdate),
				Poco::Data::Keywords::use(SerialNumber);

			Select.execute();

			if (!TmpSerialNumber.empty()) {
				Poco::JSON::Parser parser;
				Poco::Dynamic::Var result = parser.parse(packageStringArray);
				Poco::JSON::Array::Ptr jsonArray = result.extract<Poco::JSON::Array::Ptr>();
				DevicePackages.serialNumber = TmpSerialNumber;
				DevicePackages.packageArray.clear();

				for (const auto &item : *jsonArray) {
					Poco::JSON::Object::Ptr obj = item.extract<Poco::JSON::Object::Ptr>();
					GWObjects::PackageInfo pkg;
					pkg.name = obj->getValue<std::string>("name");
					pkg.version = obj->getValue<std::string>("version");
					DevicePackages.packageArray.emplace_back(pkg);
				}
			} else {
				DevicePackages.packageArray.clear();
			}

			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}

		return false;
	}

	// bool Storage::CheckIfPackageAlreadyInstalled(std::string &SerialNumber, std::string packageName) {
	// 	try {
	// 		Poco::Data::Session Sess(Pool_->get());
	// 		Poco::Data::Statement Delete(Sess);

	// 		std::string St{ "SELECT 1 AS is_present FROM DevicePackages "
	// 						"WHERE SerialNumber = ? "
	// 						"AND "
	// 						+  +

	// 					};

	// 		Delete << ConvertParams(St), Poco::Data::Keywords::use(SerialNumber);
	// 		Delete.execute();
	// 		Sess.commit();
	// 		return true;
	// 	} catch (const Poco::Exception &E) {
	// 		poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
	// 										   E.displayText()));
	// 	}
	// 	return false;
	// }

	bool Storage::DeleteDeviceInstalledPackages(std::string &SerialNumber) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Sess.begin();
			Poco::Data::Statement Delete(Sess);

			std::string St{"DELETE FROM Packages WHERE SerialNumber=?"};

			Delete << ConvertParams(St), Poco::Data::Keywords::use(SerialNumber);
			Delete.execute();
			Sess.commit();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}
} // namespace OpenWifi