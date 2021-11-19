//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StateProcessor.h"
#include "StorageService.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"

/*
	Sess << "CREATE TABLE IF NOT EXISTS LifetimeStats ("
	"SerialNumber   VARCHAR(30) PRIMARY KEY, "
	"Statistics		TEXT, "
	"Created 		BIGINT, 		"
	"Updated		BIGINT			"
	") ",
	Poco::Data::Keywords::now;
*/

namespace OpenWifi {

	bool Storage::SetLifetimeStats(std::string &SerialNumber, std::string &Stats) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   InsertOrUpdate(Sess);

			std::string InsertOrReplace{
				"insert into LifetimeStats (SerialNumber, Statistics, Created, Updated) VALUES(?,?,?,?) on conflict(SerialNumber) do  "
				"update set Statistics=?, Updated=?"
			};

			uint64_t Now = time(nullptr);
			InsertOrUpdate << 	ConvertParams(InsertOrReplace),
				Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(Stats),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Stats),
				Poco::Data::Keywords::use(Now);
			InsertOrUpdate.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetLifetimeStats(std::string &SerialNumber, std::string &Stats) {
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
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::ResetLifetimeStats(std::string &SerialNumber) {
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
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

}