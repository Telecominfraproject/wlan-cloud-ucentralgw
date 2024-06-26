//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "CentralConfig.h"
#include "StorageService.h"

#include "fmt/format.h"
#include "framework/RESTAPI_utils.h"

namespace OpenWifi {

	const static std::string DB_DefConfig_SelectFields_ForCreation{"Name VARCHAR(30) PRIMARY KEY, "
																   "Configuration TEXT, "
																   "Models TEXT, "
																   "Description TEXT, "
																   "Created BIGINT , "
																   "LastModified BIGINT, Platform TEXT )"};

	const static std::string DB_DefConfig_SelectFields{"Name, "
													   "Configuration, "
													   "Models, "
													   "Description, "
													   "Created, "
													   "LastModified, Platform "};

	const static std::string DB_DefConfig_InsertValues{"?,?,?,?,?,?,?"};

	typedef Poco::Tuple<std::string, std::string, std::string, std::string, uint64_t, uint64_t, std::string>
		DefConfigRecordTuple;
	typedef std::vector<DefConfigRecordTuple> DefConfigRecordList;

	void Convert(const DefConfigRecordTuple &R, GWObjects::DefaultConfiguration &T) {
		T.name = R.get<0>();
		T.configuration = R.get<1>();
		T.models = RESTAPI_utils::to_object_array(R.get<2>());
		T.description = R.get<3>();
		T.created = R.get<4>();
		T.lastModified = R.get<5>();
		T.platform = R.get<6>();
	}

	void Convert(const GWObjects::DefaultConfiguration &R, DefConfigRecordTuple &T) {
		T.set<0>(R.name);
		T.set<1>(R.configuration);
		T.set<2>(RESTAPI_utils::to_string(R.models));
		T.set<3>(R.description);
		T.set<4>(R.created);
		T.set<5>(R.lastModified);
		T.set<6>(R.platform);
	}

	bool Storage::CreateDefaultConfiguration(std::string &Name,
											 GWObjects::DefaultConfiguration &DefConfig) {
		try {

			std::string TmpName;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string St{"SELECT name FROM DefaultConfigs WHERE Name=?"};
			Select << ConvertParams(St), Poco::Data::Keywords::into(TmpName),
				Poco::Data::Keywords::use(Name);
			Select.execute();

			if (!TmpName.empty())
				return false;

			Config::Config Cfg(DefConfig.configuration);

			if (Cfg.Valid()) {
				Sess.begin();
				Poco::Data::Statement Insert(Sess);

				std::string St{"INSERT INTO DefaultConfigs ( " + DB_DefConfig_SelectFields +
							   " ) "
							   "VALUES(" +
							   DB_DefConfig_InsertValues + ")"};

				DefConfigRecordTuple R;
				Convert(DefConfig, R);
				Insert << ConvertParams(St), Poco::Data::Keywords::use(R);
				Insert.execute();
				Sess.commit();
				return true;
			} else {
				poco_warning(Logger(), "Cannot create device: invalid configuration.");
				return false;
			}
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::DeleteDefaultConfiguration(std::string &Name) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Sess.begin();
			Poco::Data::Statement Delete(Sess);

			std::string St{"DELETE FROM DefaultConfigs WHERE Name=?"};

			Delete << ConvertParams(St), Poco::Data::Keywords::use(Name);
			Delete.execute();
			Sess.commit();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::UpdateDefaultConfiguration(std::string &Name,
											 GWObjects::DefaultConfiguration &DefConfig) {
		try {
			uint64_t Now = Utils::Now();
			Poco::Data::Session Sess = Pool_->get();
			Sess.begin();
			Poco::Data::Statement Update(Sess);
			DefConfig.lastModified = Now;

			std::string St{"UPDATE DefaultConfigs SET Name=?, Configuration=?,  Models=?,  "
						   "Description=?,  Created=? , LastModified=? , Platform=?  WHERE Name=?"};

			DefConfigRecordTuple R;
			Convert(DefConfig, R);

			Update << ConvertParams(St), Poco::Data::Keywords::use(R),
				Poco::Data::Keywords::use(Name);
			Update.execute();
			Sess.commit();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::GetDefaultConfiguration(std::string &Name,
										  GWObjects::DefaultConfiguration &DefConfig) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string St{"SELECT " + DB_DefConfig_SelectFields +
						   " FROM DefaultConfigs WHERE Name=?"};

			DefConfigRecordTuple R;
			Select << ConvertParams(St), Poco::Data::Keywords::into(R),
				Poco::Data::Keywords::use(Name);
			Select.execute();

			if (Select.rowsExtracted() == 1) {
				Convert(R, DefConfig);
				return true;
			}
			return false;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	bool Storage::DefaultConfigurationAlreadyExists(std::string &Name) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string St{"SELECT " + DB_DefConfig_SelectFields +
						   " FROM DefaultConfigs WHERE Name=?"};

			DefConfigRecordTuple R;
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
	Storage::GetDefaultConfigurations(uint64_t From, uint64_t HowMany,
									  std::vector<GWObjects::DefaultConfiguration> &DefConfigs) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			DefConfigRecordList Records;
			Select << "SELECT " + DB_DefConfig_SelectFields +
						  " FROM DefaultConfigs ORDER BY NAME ASC " + ComputeRange(From, HowMany),
				Poco::Data::Keywords::into(Records);
			Select.execute();

			for (const auto &i : Records) {
				GWObjects::DefaultConfiguration R;
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

	bool Storage::FindDefaultConfigurationForModel(const std::string &DeviceModel, const std::string &Platform,
												   GWObjects::DefaultConfiguration &Config) {
		try {
			DefConfigRecordList DefConfigs;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			Select << "SELECT " + DB_DefConfig_SelectFields + " FROM DefaultConfigs",
				Poco::Data::Keywords::into(DefConfigs);
			Select.execute();

			for (const auto &DefConfig : DefConfigs) {
				GWObjects::DefaultConfiguration C;
				Convert(DefConfig, C);
				for (const auto &Model : C.models) {
					if ((Model == "*" || Model == DeviceModel) && (Config.platform == Platform)){
						Config = C;
						return true;
					}
				}
			}
			Logger().information(
				fmt::format("AUTO-PROVISIONING: no default configuration for model:{}", DeviceModel));
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return false;
	}

	uint64_t Storage::GetDefaultConfigurationsCount() {
		uint64_t Count = 0;
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			Select << "SELECT Count(*) from DefaultConfigs", Poco::Data::Keywords::into(Count);
			Select.execute();
			return Count;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("{}: Failed with: {}", std::string(__func__),
											   E.displayText()));
		}
		return Count;
	}

} // namespace OpenWifi