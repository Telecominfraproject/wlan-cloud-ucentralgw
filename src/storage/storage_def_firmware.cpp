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
						"Name VARCHAR(64) PRIMARY KEY, "
						"uri TEXT, "
						"revision TEXT, "
						"Models TEXT, "
						"Description TEXT, "
						"Created BIGINT , "
						"imageCreationDate BIGINT , "
						"LastModified BIGINT)" };

	const static std::string DB_DefFirmware_SelectFields{
						"Name, "
						"uri, "
						"revision, "
						"Models, "
						"Description, "
						"Created, "
						"imageCreationDate, "
						"LastModified "};

	const static std::string DB_DefFirmware_InsertValues{"?,?,?,?,?,?,?,?"};

	typedef Poco::Tuple<std::string,
						std::string,
						std::string,
						std::string,
						std::string,
						uint64_t,
						uint64_t,
						uint64_t>
		DefFirmwareRecordTuple;
	typedef std::vector<DefFirmwareRecordTuple> DefFirmwareRecordList;

	void Convert(const DefFirmwareRecordTuple &R, GWObjects::DefaultFirmware &T) {
		T.Name = R.get<0>();
		T.uri = R.get<1>();
		T.revision = R.get<2>();
		T.Models = RESTAPI_utils::to_object_array(R.get<3>());
		T.Description = R.get<4>();
		T.Created = R.get<5>();
		T.imageCreationDate = R.get<6>();
		T.LastModified = R.get<7>();
	}

	void Convert(const GWObjects::DefaultFirmware &R, DefFirmwareRecordTuple &T) {
		T.set<0>(R.Name);
		T.set<1>(R.uri);
		T.set<2>(R.revision);
		T.set<3>(RESTAPI_utils::to_string(R.Models));
		T.set<4>(R.Description);
		T.set<5>(R.Created);
		T.set<6>(R.imageCreationDate);
		T.set<7>(R.LastModified);
	}

	bool Storage::CreateDefaultFirmware(std::string &Name,
											 GWObjects::DefaultFirmware &DefFirmware) {
		try {

			std::string TmpName;
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			Poco::toLowerInPlace(DefFirmware.Name);

			std::string St{"SELECT name FROM DefaultFirmwares WHERE Name=?"};
			Select << ConvertParams(St), Poco::Data::Keywords::into(TmpName),
				Poco::Data::Keywords::use(Name);
			Select.execute();

			if (!TmpName.empty())
				return false;

			Poco::Data::Statement Insert(Sess);

			std::string St2{"INSERT INTO DefaultFirmwares ( " + DB_DefFirmware_SelectFields +
						   " ) "
						   "VALUES(" +
						   DB_DefFirmware_InsertValues + ")"};

			DefFirmwareRecordTuple R;
			Convert(DefFirmware, R);
			Insert << ConvertParams(St2), Poco::Data::Keywords::use(R);
			Insert.execute();
			return true;

		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::DeleteDefaultFirmware(std::string &Name) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);
			Poco::toLowerInPlace(Name);

			std::string St{"DELETE FROM DefaultFirmwares WHERE Name=?"};

			Delete << ConvertParams(St), Poco::Data::Keywords::use(Name);
			Delete.execute();

			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::UpdateDefaultFirmware(std::string &Name,
											 GWObjects::DefaultFirmware &DefFirmware) {
		try {
			Poco::Data::Session Sess = Pool_->get();

			uint64_t Now = time(nullptr);
			Poco::Data::Statement Update(Sess);
			DefFirmware.LastModified = Now;
			Poco::toLowerInPlace(DefFirmware.Name);

			std::string St{"UPDATE DefaultFirmwares SET Name=?, Configuration=?,  Models=?,  "
						   "Description=?,  Created=? , LastModified=?  WHERE Name=?"};

			DefFirmwareRecordTuple R;
			Convert(DefFirmware, R);

			Update << ConvertParams(St), Poco::Data::Keywords::use(R),
				Poco::Data::Keywords::use(Name);
			Update.execute();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::GetDefaultFirmware(std::string &Name,
										  GWObjects::DefaultFirmware &DefFirmware) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			Poco::toLowerInPlace(DefFirmware.Name);

			std::string St{"SELECT " + DB_DefFirmware_SelectFields +
						   " FROM DefaultFirmwares WHERE Name=?"};

			DefFirmwareRecordTuple R;
			Select << ConvertParams(St), Poco::Data::Keywords::into(R),
				Poco::Data::Keywords::use(Name);
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

	bool Storage::DefaultFirmwareAlreadyExists(std::string &Name) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			Poco::toLowerInPlace(Name);

			std::string St{"SELECT " + DB_DefFirmware_SelectFields +
						   " FROM DefaultFirmwares WHERE Name=?"};

			DefFirmwareRecordTuple R;
			Select << ConvertParams(St), Poco::Data::Keywords::into(R),
				Poco::Data::Keywords::use(Name);
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
									  std::vector<GWObjects::DefaultFirmware> &DefConfigs) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			DefFirmwareRecordList Records;
			Select << "SELECT " + DB_DefFirmware_SelectFields +
						  " FROM DefaultFirmwares ORDER BY NAME ASC " + ComputeRange(From, HowMany),
				Poco::Data::Keywords::into(Records);
			Select.execute();

			for (const auto &i : Records) {
				GWObjects::DefaultFirmware R;
				Convert(i, R);
				DefConfigs.push_back(R);
			}
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::FindDefaultFirmwareForModel(const std::string &Model,
												   GWObjects::DefaultFirmware &DefConfig) {
		try {
			DefFirmwareRecordList Records;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			Select << "SELECT " + DB_DefFirmware_SelectFields + " FROM DefaultFirmwares",
				Poco::Data::Keywords::into(Records);
			Select.execute();

			for (const auto &i : Records) {
				GWObjects::DefaultFirmware Config;
				Convert(i, Config);
				for (const auto &j : Config.Models) {
					if (j == "*" || j == Model) {
						DefConfig = Config;
						return true;
					}
				}
			}
			return false;
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