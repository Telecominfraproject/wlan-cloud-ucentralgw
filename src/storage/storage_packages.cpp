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
#include "StorageService.h"
#include "framework/utils.h"

#include "fmt/format.h"

namespace OpenWifi {

	const static std::string DB_PackageSelectField{"SerialNumber, PackageName, PackageVersion, FirstUpdate, LastUpdate"};

	// 					serial		 pkgName	  pkgVer	   f_update  l_update
	typedef Poco::Tuple<std::string, std::string, std::string, uint64_t, uint64_t> PackageTuple;
	typedef std::vector<PackageTuple> PackageList;

	void ConvertPackagesRecord(const PackageTuple &R, GWObjects::Package &Pkg) {
		Pkg.serialNumber = R.get<0>();
		Pkg.pkgName = R.get<1>();
		Pkg.pkgVersion = R.get<2>();
		Pkg.FirstUpdate = R.get<3>();
		Pkg.LastUpdate = R.get<4>();
	}

	void ConvertPackagesRecord(const GWObjects::Package &Pkg, PackageTuple &R) {
		R.set<0>(Pkg.serialNumber);
		R.set<1>(Pkg.pkgName);
		R.set<2>(Pkg.pkgVersion);
		R.set<3>(Pkg.FirstUpdate);
		R.set<4>(Pkg.LastUpdate);
	}

	bool Storage::CreateDeviceInstalledPackages(std::string &SerialNumber, std::vector<GWObjects::Package> &pkgList) {
		try {
			Session.begin();
			Poco::Data::Statement UpSert(Session);

			uint64_t Now = Utils::Now();

			std::string St{"INSERT INTO Packages (SerialNumber, PackageName, PackageVersion, FirstUpdate, LastUpdate) "
                         "VALUES (?,?,?,?,?) "
                         "ON CONFLICT (SerialNumber, PackageName) DO "
                         "UPDATE SET PackageVersion = ?, LastUpdate = ?"};
			
			std::vector<string> PackageNames;
			std::vector<string> PackageVersions;
			
			for (const auto& pkg : packages) {
				PackageNames.push_back(pkg.pkgName);
				PackageVersions.push_back(pkg.pkgVersion);
			}

			UpSert << ConvertParams(St), Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(PackageNames), Poco::Data::Keywords::use(PackageVersions),
				Poco::Data::Keywords::use(Now), Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(PackageVersions), Poco::Data::Keywords::use(Now);
			UpSert.execute();
			Session.commit();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::UpdateDeviceInstalledPackages(Poco::Data::Session &Session, std::string &SerialNumber,
										   GWObjects::PackageList &pkgList) {
		try {
			Session.begin();
			// Poco::Data::Statement UpSert(Session);

			// uint64_t Now = Utils::Now();

			// std::string St{"insert into Packages (SerialNumber, Packages, FirstUpdate, "
			// 			   "LastUpdate) values(?,?,?,?) on conflict (SerialNumber) do "
			// 			   " update set Packages=?, LastUpdate=?"};
			// UpSert << ConvertParams(St), Poco::Data::Keywords::use(SerialNumber),
			// 	Poco::Data::Keywords::use(pkgList.packages), Poco::Data::Keywords::use(Now),
			// 	Poco::Data::Keywords::use(Now), Poco::Data::Keywords::use(pkgList.packages),
			// 	Poco::Data::Keywords::use(Now);
			// UpSert.execute();
			// Session.commit();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}	
		return false;
	}

	bool Storage::GetDeviceInstalledPackages(std::string &SerialNumber, std::vector<GWObjects::Package> &PkgList) {
		try {
			Poco::Data::Session Sess(Pool_->get());
			Poco::Data::Statement Select(Sess);

			PackageList Pkgs;

			std::string St{"SELECT " + DB_PackageSelectField + " FROM Packages WHERE SerialNumber=?"};

			Select << ConvertParams(St), Poco::Data::Keywords::into(Pkgs), Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			for (const auto &i : Pkgs) {
				GWObjects::Package R;
				ConvertPackagesRecord(i, R);
				PkgList.emplace_back(R);
			}
			Select.reset(Sess);
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}

		return false;
	}

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