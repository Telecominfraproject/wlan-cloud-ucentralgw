//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "uStorageService.h"
#include "uStateProcessor.h"

/*
	Sess << "CREATE TABLE IF NOT EXISTS LifetimeStats ("
	"SerialNumber   VARCHAR(30) PRIMARY KEY, "
	"Statistics		TEXT, "
	"Created 		BIGINT, 		"
	"Updated		BIGINT			"
	") ",
	Poco::Data::Keywords::now;
*/

namespace uCentral::Storage {

	bool SetLifetimeStats(std::string & SerialNumber, std::string & Stats) {
		return Service::instance()->SetLifetimeStats(SerialNumber, Stats);
	}

	bool GetLifetimeStats(std::string & SerialNumber, std::string & Stats) {
		return Service::instance()->GetLifetimeStats(SerialNumber, Stats);
	}

	bool ResetLifetimeStats(std::string & SerialNumber) {
		return Service::instance()->ResetLifetimeStats(SerialNumber);
	}

	bool Service::SetLifetimeStats(std::string &SerialNumber, std::string &Stats) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string TmpSerial;
			std::string st1{"SELECT SerialNumber FROM LifetimeStats WHERE SerialNumber=?"};
			Select << 	ConvertParams(st1),
						Poco::Data::Keywords::into(TmpSerial),
						Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if(TmpSerial.empty()) {
				Poco::Data::Statement   Insert(Sess);
				uint64_t Now = time(nullptr);
				std::string st2{"INSERT INTO LifetimeStats (SerialNumber, Statistics, Created, Updated) VALUES(?,?,?,?)"};
				Insert << 	ConvertParams(st2),
							Poco::Data::Keywords::use(SerialNumber),
							Poco::Data::Keywords::use(Stats),
							Poco::Data::Keywords::use(Now),
							Poco::Data::Keywords::use(Now);
				Insert.execute();
			} else {
				Poco::Data::Statement   Update(Sess);
				uint64_t Now = time(nullptr);
				std::string st2{"UPDATE LifetimeStats SET Statistics=?, Updated=? WHERE SerialNumber=?"};
				Update << 	ConvertParams(st2),
							Poco::Data::Keywords::use(Stats),
							Poco::Data::Keywords::use(Now),
							Poco::Data::Keywords::use(SerialNumber);
				Update.execute();
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::GetLifetimeStats(std::string &SerialNumber, std::string &Stats) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string st1{"SELECT Statistics FROM LifetimeStats WHERE SerialNumber=?"};
			Select << 	ConvertParams(st1),
						Poco::Data::Keywords::into(Stats),
						Poco::Data::Keywords::use(SerialNumber);
			Select.execute();
			return !Stats.empty();
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::ResetLifetimeStats(std::string &SerialNumber) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Delete(Sess);

			std::string st1{"DELETE FROM LifetimeStats WHERE SerialNumber=?"};
			Delete << 	ConvertParams(st1),
						Poco::Data::Keywords::use(SerialNumber);
			Delete.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}
}