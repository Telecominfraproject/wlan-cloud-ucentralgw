//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"

#include "CentralConfig.h"

namespace OpenWifi {

	const static std::string	DB_DefConfig_SelectFields_ForCreation{
		"Name VARCHAR(30) PRIMARY KEY, "
		"Configuration TEXT, "
		"Models TEXT, "
		"Description TEXT, "
		"Created BIGINT , "
		"LastModified BIGINT)"};

	const static std::string	DB_DefConfig_SelectFields{
		"Name, "
		"Configuration, "
		"Models, "
		"Description, "
		"Created, "
		"LastModified "};

	const static std::string 	DB_DefConfig_InsertValues{"?,?,?,?,?,?"};

	typedef Poco::Tuple<
		std::string,
		std::string,
		std::string,
		std::string,
		uint64_t,
		uint64_t
	> DefConfigRecordTuple;
	typedef std::vector<DefConfigRecordTuple> DefConfigRecordList;

	void Convert(const DefConfigRecordTuple &R, GWObjects::DefaultConfiguration & T) {
		T.Name = R.get<0>();
		T.Configuration = R.get<1>();
		T.Models = RESTAPI_utils::to_object_array(R.get<2>());
		T.Description = R.get<3>();
		T.Created = R.get<4>();
		T.LastModified = R.get<5>();
	}

	void Convert(const GWObjects::DefaultConfiguration & R, DefConfigRecordTuple &T) {
		T.set<0>(R.Name);
		T.set<1>(R.Configuration);
		T.set<2>(RESTAPI_utils::to_string(R.Models));
		T.set<3>(R.Description);
		T.set<4>(R.Created);
		T.set<5>(R.LastModified);
	}

	bool Storage::CreateDefaultConfiguration(std::string &Name, GWObjects::DefaultConfiguration &DefConfig) {
		try {

			std::string TmpName;

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select <<   "SELECT Name FROM DefaultConfigs WHERE Name='%s'",
				Poco::Data::Keywords::into(TmpName),
				Name;
			Select.execute();

			if (!TmpName.empty())
				return false;

			Config::Config Cfg(DefConfig.Configuration);

			if (Cfg.Valid()) {
				uint64_t Now = std::time(nullptr);
				Poco::Data::Statement   Insert(Sess);

				std::string St{"INSERT INTO DefaultConfigs ( " + DB_DefConfig_SelectFields + " ) "
						"VALUES(" + DB_DefConfig_InsertValues + ")"};

				DefConfigRecordTuple R;
				Convert(DefConfig,R);
				Insert  << ConvertParams(St),
					Poco::Data::Keywords::use(R);
				Insert.execute();
				return true;
			} else {
				Logger_.warning("Cannot create device: invalid configuration.");
				return false;
			}
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::DeleteDefaultConfiguration(std::string &Name) {
		try {

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Delete(Sess);

			std::string St{"DELETE FROM DefaultConfigs WHERE Name=?"};

			Delete << ConvertParams(St),
				Poco::Data::Keywords::use(Name);
			Delete.execute();

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::UpdateDefaultConfiguration(std::string &Name, GWObjects::DefaultConfiguration &DefConfig) {
		try {
			Poco::Data::Session Sess = Pool_->get();

			uint64_t Now = time(nullptr);
			Poco::Data::Statement   Update(Sess);
			DefConfig.LastModified = Now;

			std::string St{"UPDATE DefaultConfigs SET Name=?, Configuration=?,  Models=?,  Description=?,  Created=? , LastModified=?  WHERE Name=?"};

			DefConfigRecordTuple R;
			Convert(DefConfig, R);

			Update << ConvertParams(St),
				Poco::Data::Keywords::use(R),
				Poco::Data::Keywords::use(Name);
			Update.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetDefaultConfiguration(std::string &Name, GWObjects::DefaultConfiguration &DefConfig) {
		try {

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT " + DB_DefConfig_SelectFields +
						   " FROM DefaultConfigs WHERE Name=?"};

			DefConfigRecordTuple R;
			Select << ConvertParams(St),
				Poco::Data::Keywords::into(R),
				Poco::Data::Keywords::use(Name);
			Select.execute();

			if (Select.rowsExtracted()==1) {
				Convert(R,DefConfig);
				return true;
			}
			return false;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::DefaultConfigurationAlreadyExists(std::string &Name) {
		try {

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT " + DB_DefConfig_SelectFields +
			" FROM DefaultConfigs WHERE Name=?"};

			DefConfigRecordTuple R;
			Select << ConvertParams(St),
				Poco::Data::Keywords::into(R),
				Poco::Data::Keywords::use(Name);
			Select.execute();

			return Select.rowsExtracted()==1;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetDefaultConfigurations(uint64_t From, uint64_t HowMany,
										   std::vector<GWObjects::DefaultConfiguration> &DefConfigs) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			DefConfigRecordList Records;
			Select << "SELECT "  + DB_DefConfig_SelectFields +
					  " FROM DefaultConfigs ORDER BY NAME ASC " + ComputeRange(From, HowMany) ,
					Poco::Data::Keywords::into(Records);
			Select.execute();

			for (const auto &i: Records) {
				GWObjects::DefaultConfiguration R;
				Convert(i,R);
				DefConfigs.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::FindDefaultConfigurationForModel(const std::string &Model, GWObjects::DefaultConfiguration &DefConfig) {
		try {
			DefConfigRecordList Records;

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select << "SELECT "  + DB_DefConfig_SelectFields + " FROM DefaultConfigs",
				Poco::Data::Keywords::into(Records);
			Select.execute();

			for (const auto &i: Records) {
				GWObjects::DefaultConfiguration Config;
				Convert(i,Config);
				for(const auto &j:Config.Models) {
					if (j == "*" || j == Model) {
						DefConfig = Config;
						return true;
					}
				}
			}
			Logger_.information(Poco::format("AUTO-PROVISIONING: no default configuration for model:%s", Model));
			return false;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	uint64_t Storage::GetDefaultConfigurationsCount() {
		uint64_t Count = 0;
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);
			Select << "SELECT Count(*) from DefaultConfigs" ,
				Poco::Data::Keywords::into(Count);
			Select.execute();
			return Count;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return Count;
	}

}