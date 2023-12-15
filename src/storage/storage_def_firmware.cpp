//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
// Created by stephane bourque on 2023-07-11.
//	Arilia Wireless Inc.
//

#include "CentralConfig.h"
#include "StorageService.h"

#include "fmt/format.h"
#include "framework/RESTAPI_utils.h"

namespace OpenWifi {

	const static std::string DB_DefFirmware_SelectFields_ForCreation{
						"deviceType VARCHAR(128) PRIMARY KEY, "
						"uri TEXT, "
						"revision TEXT, "
						"Description TEXT, "
						"Created BIGINT , "
						"imageCreationDate BIGINT , "
						"LastModified BIGINT)" };

	const static std::string DB_DefFirmware_SelectFields{
						"deviceType, "
						"uri, "
						"revision, "
						"Description, "
						"Created, "
						"imageCreationDate, "
						"LastModified "};

	const static std::string DB_DefFirmware_InsertValues{"?,?,?,?,?,?,?"};

	typedef Poco::Tuple<std::string,
						std::string,
						std::string,
						std::string,
						uint64_t,
						uint64_t,
						uint64_t>
		DefFirmwareRecordTuple;
	typedef std::vector<DefFirmwareRecordTuple> DefFirmwareRecordList;

	void Convert(const DefFirmwareRecordTuple &R, GWObjects::DefaultFirmware &T) {
		T.deviceType = R.get<0>();
		T.uri = R.get<1>();
		T.revision = R.get<2>();
		T.Description = R.get<3>();
		T.Created = R.get<4>();
		T.imageCreationDate = R.get<5>();
		T.LastModified = R.get<6>();
	}

	void Convert(const GWObjects::DefaultFirmware &R, DefFirmwareRecordTuple &T) {
		T.set<0>(R.deviceType);
		T.set<1>(R.uri);
		T.set<2>(R.revision);
		T.set<3>(R.Description);
		T.set<4>(R.Created);
		T.set<5>(R.imageCreationDate);
		T.set<6>(R.LastModified);
	}

	bool Storage::CreateDefaultFirmware(GWObjects::DefaultFirmware &DefFirmware) {
		try {

			std::string TmpName;
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			Poco::toLowerInPlace(DefFirmware.deviceType);

			std::string St{"SELECT DeviceType FROM DefaultFirmwares WHERE deviceType=?"};
			Select << ConvertParams(St),
				Poco::Data::Keywords::into(TmpName),
				Poco::Data::Keywords::use(DefFirmware.deviceType);
			Select.execute();

			if (!TmpName.empty())
				return false;

			Sess.begin();
			Poco::Data::Statement Insert(Sess);

			std::string St2{"INSERT INTO DefaultFirmwares ( " + DB_DefFirmware_SelectFields +
						   " ) "
						   "VALUES(" +
						   DB_DefFirmware_InsertValues + ")"};

			DefFirmwareRecordTuple R;
			Convert(DefFirmware, R);
			Insert << ConvertParams(St2),
				Poco::Data::Keywords::use(R);
			Insert.execute();
			Sess.commit();
			return true;

		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::DeleteDefaultFirmware(std::string &deviceType) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Sess.begin();
			Poco::Data::Statement Delete(Sess);
			Poco::toLowerInPlace(deviceType);

			std::string St{"DELETE FROM DefaultFirmwares WHERE deviceType=?"};

			Delete << ConvertParams(St), Poco::Data::Keywords::use(deviceType);
			Delete.execute();
			Sess.commit();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::UpdateDefaultFirmware(GWObjects::DefaultFirmware &DefFirmware) {
		try {
			uint64_t Now = Utils::Now();
			Poco::Data::Session Sess = Pool_->get();
			Sess.begin();
			Poco::Data::Statement Update(Sess);
			DefFirmware.LastModified = Now;
			Poco::toLowerInPlace(DefFirmware.deviceType);


			std::string St{"UPDATE DefaultFirmwares SET deviceType=?, uri=?,  revision=?,  "
						   "Description=?,  Created=? , imageCreationDate=?, LastModified=?  WHERE deviceType=?"};

			DefFirmwareRecordTuple R;
			Convert(DefFirmware, R);

			Update << ConvertParams(St),
				Poco::Data::Keywords::use(R),
				Poco::Data::Keywords::use(DefFirmware.deviceType);
			Update.execute();
			Sess.commit();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::GetDefaultFirmware(std::string &deviceType,
										  GWObjects::DefaultFirmware &DefFirmware) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			Poco::toLowerInPlace(deviceType);

			std::string St{"SELECT " + DB_DefFirmware_SelectFields +
						   " FROM DefaultFirmwares WHERE deviceType=?"};

			DefFirmwareRecordTuple R;
			Select << ConvertParams(St),
				Poco::Data::Keywords::into(R),
				Poco::Data::Keywords::use(deviceType);
			Select.execute();

			if (Select.rowsExtracted() == 1) {
				Convert(R, DefFirmware);
				return true;
			}
			return false;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::DefaultFirmwareAlreadyExists(std::string &deviceType) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			Poco::toLowerInPlace(deviceType);

			std::string St{"SELECT " + DB_DefFirmware_SelectFields +
						   " FROM DefaultFirmwares WHERE deviceType=?"};

			DefFirmwareRecordTuple R;
			Select << ConvertParams(St), Poco::Data::Keywords::into(R),
				Poco::Data::Keywords::use(deviceType);
			Select.execute();

			return Select.rowsExtracted() == 1;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool
	Storage::GetDefaultFirmwares(uint64_t From, uint64_t HowMany,
									  std::vector<GWObjects::DefaultFirmware> &Firmwares) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			DefFirmwareRecordList Records;
			Select << "SELECT " + DB_DefFirmware_SelectFields +
						  " FROM DefaultFirmwares ORDER BY deviceType ASC " + ComputeRange(From, HowMany),
				Poco::Data::Keywords::into(Records);
			Select.execute();
			Firmwares.clear();
			for (const auto &i : Records) {
				GWObjects::DefaultFirmware R;
				Convert(i, R);
				Firmwares.push_back(R);
			}
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	uint64_t Storage::GetDefaultFirmwaresCount() {
		uint64_t Count = 0;
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			Select << "SELECT Count(*) from DefaultFirmwares", Poco::Data::Keywords::into(Count);
			Select.execute();
			return Count;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return Count;
	}

} // namespace OpenWifi