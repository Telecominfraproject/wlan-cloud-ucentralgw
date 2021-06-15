//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"

#include "CentralConfig.h"

namespace uCentral {

	/*
	 *  Data model for DefaultConfigurations:

		Table name: DefaultConfigs

					"Name VARCHAR(30) PRIMARY KEY, "
					"Configuration TEXT, "
					"Models TEXT, "
					"Description TEXT, "
					"Created BIGINT , "
					"LastModified BIGINT)", now;
	 */

	bool Storage::CreateDefaultConfiguration(std::string &Name, uCentral::Objects::DefaultConfiguration &DefConfig) {
		try {

			std::string TmpName;

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select <<   "SELECT Name FROM DefaultConfigs WHERE Name='%s'",
				Poco::Data::Keywords::into(TmpName),
				Name;
			Select.execute();

			if (TmpName.empty()) {

				uCentral::Config::Config Cfg(DefConfig.Configuration);
	/*
						 "Name VARCHAR(30) PRIMARY KEY, "
						"Configuration TEXT, "
						"Models TEXT, "
						"Description TEXT, "
						"Created BIGINT , "
						"LastModified BIGINT)", now;

	 */

				if (Cfg.Valid()) {
					uint64_t Now = time(nullptr);
					Poco::Data::Statement   Insert(Sess);

					std::string St{"INSERT INTO DefaultConfigs (Name, Configuration, Models, Description, Created, LastModified) "
								   "VALUES(?,?,?,?,?,?)"};

					Insert  << ConvertParams(St),
						Poco::Data::Keywords::use(Name),
						Poco::Data::Keywords::use(DefConfig.Configuration),
						Poco::Data::Keywords::use(DefConfig.Models),
						Poco::Data::Keywords::use(DefConfig.Description),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(Now);

					Insert.execute();

					return true;
				} else {
					Logger_.warning("Cannot create device: invalid configuration.");
					return false;
				}
			} else {
				Logger_.warning("Default configuration already exists.");
			}
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), Name, E.displayText()));
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
			Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), Name, E.displayText()));
		}
		return false;
	}

	bool Storage::UpdateDefaultConfiguration(std::string &Name, uCentral::Objects::DefaultConfiguration &DefConfig) {
		try {

			Poco::Data::Session Sess = Pool_->get();

			uCentral::Config::Config Cfg(DefConfig.Configuration);

			if (Cfg.Valid()) {

				uint64_t Now = time(nullptr);
				Poco::Data::Statement   Update(Sess);

				std::string St{"UPDATE DefaultConfigs SET Configuration=?,  Models=?,  Description=?,  LastModified=?  WHERE Name=?"};

				Update << ConvertParams(St),
					Poco::Data::Keywords::use(DefConfig.Configuration),
					Poco::Data::Keywords::use(DefConfig.Models),
					Poco::Data::Keywords::use(DefConfig.Description),
					Poco::Data::Keywords::use(Now),
					Poco::Data::Keywords::use(Name);

				Update.execute();
				return true;
			} else {
				Logger_.warning(
					Poco::format("Default configuration: %s cannot be set to an invalid configuration.", Name));
			}
			return false;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), Name, E.displayText()));
		}
		return false;
	}

	bool Storage::GetDefaultConfiguration(std::string &Name, uCentral::Objects::DefaultConfiguration &DefConfig) {
		try {

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT "
						   "Name, "
						   "Configuration, "
						   "Models, "
						   "Description, "
						   "Created, "
						   "LastModified "
						   "FROM DefaultConfigs WHERE Name=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(DefConfig.Name),
				Poco::Data::Keywords::into(DefConfig.Configuration),
				Poco::Data::Keywords::into(DefConfig.Models),
				Poco::Data::Keywords::into(DefConfig.Description),
				Poco::Data::Keywords::into(DefConfig.Created),
				Poco::Data::Keywords::into(DefConfig.LastModified),
				Poco::Data::Keywords::use(Name);

			Select.execute();

			if (DefConfig.Name.empty())
				return false;

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), Name, E.displayText()));
		}
		return false;
	}

	bool Storage::GetDefaultConfigurations(uint64_t From, uint64_t HowMany,
										   std::vector<uCentral::Objects::DefaultConfiguration> &DefConfigs) {
		typedef Poco::Tuple<
			std::string,
			std::string,
			std::string,
			std::string,
			uint64_t,
			uint64_t> DeviceRecord;
		typedef std::vector<DeviceRecord> RecordList;

		RecordList Records;

		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select << "SELECT "
					  "Name, "
					  "Configuration, "
					  "Models, "
					  "Description, "
					  "Created, "
					  "LastModified "
					  "FROM DefaultConfigs",
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::range(From, From + HowMany );

			Select.execute();

			for (auto i: Records) {
				uCentral::Objects::DefaultConfiguration R{
					.Name           = i.get<0>(),
					.Configuration  = i.get<1>(),
					.Models         = i.get<2>(),
					.Description    = i.get<3>(),
					.Created        = i.get<4>(),
					.LastModified   = i.get<5>()};

				DefConfigs.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool FindInList(const std::string &Model, const std::string &List) {
		unsigned long P = 0;
		std::string Token;

		while (P < List.size()) {
			auto P2 = List.find_first_of(',', P);
			if (P2 == std::string::npos) {
				Token = List.substr(P);
				if (Model.find(Token) != std::string::npos)
					return true;
				return false;
			} else {
				Token = List.substr(P, P2);
				if (Model.find(Token) != std::string::npos)
					return true;
			}
			P = P2 + 1;
		}
		return false;
	}

	bool Storage::FindDefaultConfigurationForModel(const std::string &Model, uCentral::Objects::DefaultConfiguration &DefConfig) {
		try {
			typedef Poco::Tuple<
				std::string,
				std::string,
				std::string,
				std::string,
				uint64_t,
				uint64_t> DeviceRecord;
			typedef std::vector<DeviceRecord> RecordList;
			RecordList Records;

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select << "SELECT "
					  "Name, "
					  "Configuration, "
					  "Models, "
					  "Description, "
					  "Created, "
					  "LastModified "
					  "FROM DefaultConfigs",
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::range(0, 2);
			Select.execute();

			for (auto i: Records) {
				DefConfig.Models = i.get<2>();
				if (FindInList(Model, DefConfig.Models)) {
					DefConfig.Name = i.get<0>();
					Logger_.information(Poco::format("AUTO-PROVISIONING: found default configuration '%s' for model:%s",
													 DefConfig.Name, Model));
					DefConfig.Name = i.get<0>();
					DefConfig.Configuration = i.get<1>();
					DefConfig.Models = i.get<2>();
					DefConfig.Description = i.get<3>();
					DefConfig.Created = i.get<4>();
					DefConfig.LastModified = i.get<5>();
					return true;
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


}