//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "DeviceRegistry.h"
#include "StorageService.h"

namespace uCentral {

	bool Storage::AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats) {

		DeviceRegistry()->SetStatistics(SerialNumber, NewStats);

		try {
			Logger_.information("Device:" + SerialNumber + " Stats size:" + std::to_string(NewStats.size()));

			uint64_t Now = time(nullptr);
			Poco::Data::Session Sess = Pool_->get();
	/*
						"SerialNumber VARCHAR(30), "
						"UUID INTEGER, "
						"Data TEXT, "
						"Recorded BIGINT)", now;

	 */
			Poco::Data::Statement   Insert(Sess);
			std::string St{"INSERT INTO Statistics (SerialNumber, UUID, Data, Recorded) VALUES(?,?,?,?)"};

			Insert << ConvertParams(St),
				Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(CfgUUID),
				Poco::Data::Keywords::use(NewStats),
				Poco::Data::Keywords::use(Now);
			Insert.execute();

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Storage::GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
									uint64_t HowMany,
									std::vector<GWObjects::Statistics> &Stats) {

		typedef Poco::Tuple<std::string, uint64_t, std::string, uint64_t> StatRecord;
		typedef std::vector<StatRecord> RecordList;

		// std::lock_guard<std::mutex> guard(Mutex_);

		try {
			RecordList              Records;
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);

			std::string Prefix{"SELECT SerialNumber, UUID, Data, Recorded FROM Statistics "};
			std::string StatementStr = SerialNumber.empty()
									   ? Prefix + std::string(DatesIncluded ? "WHERE " : "")
									   : Prefix + "WHERE SerialNumber='" + SerialNumber + "'" +
										 std::string(DatesIncluded ? " AND " : "");

			std::string DateSelector;
			if (FromDate && ToDate) {
				DateSelector = " Recorded>=" + std::to_string(FromDate) + " AND Recorded<=" + std::to_string(ToDate);
			} else if (FromDate) {
				DateSelector = " Recorded>=" + std::to_string(FromDate);
			} else if (ToDate) {
				DateSelector = " Recorded<=" + std::to_string(ToDate);
			}

			Select << StatementStr + DateSelector,
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::range(Offset, HowMany );
			Select.execute();

			for (auto i: Records) {
				GWObjects::Statistics R{
					.UUID = i.get<1>(),
					.Data = i.get<2>(),
					.Recorded = i.get<3>()};
				Stats.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Storage::GetNewestStatisticsData(std::string &SerialNumber, uint64_t HowMany, std::vector<GWObjects::Statistics> &Stats) {
		typedef Poco::Tuple<std::string, uint64_t, std::string, uint64_t> StatRecord;
		typedef std::vector<StatRecord> RecordList;

		try {
			RecordList              Records;
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber=? ORDER BY Recorded DESC"};

			Select << 	ConvertParams(St),
						Poco::Data::Keywords::into(Records),
						Poco::Data::Keywords::use(SerialNumber),
						Poco::Data::Keywords::limit(HowMany );
			Select.execute();

			for (auto i: Records) {
				GWObjects::Statistics R{
					.UUID = i.get<1>(),
					.Data = i.get<2>(),
					.Recorded = i.get<3>()};
				Stats.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

bool Storage::DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
		try {
			Poco::Data::Session Sess = Pool_->get();

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);

			std::string Prefix{"DELETE FROM Statistics "};
			std::string Statement = SerialNumber.empty()
									? Prefix + std::string(DatesIncluded ? "WHERE " : "")
									: Prefix + "WHERE SerialNumber='" + SerialNumber + "'" +
									  std::string(DatesIncluded ? " AND " : "");

			std::string DateSelector;
			if (FromDate && ToDate) {
				DateSelector = " Recorded>=" + std::to_string(FromDate) + " AND Recorded<=" + std::to_string(ToDate);
			} else if (FromDate) {
				DateSelector = " Recorded>=" + std::to_string(FromDate);
			} else if (ToDate) {
				DateSelector = " Recorded<=" + std::to_string(ToDate);
			}

			Poco::Data::Statement   Select(Sess);
			Select << Statement + DateSelector;
			Select.execute();

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Storage::RemoveStatisticsRecordsOlderThan(uint64_t Date) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			std::string St1{"delete from Statistics where recorded<?"};
			Delete << ConvertParams(St1), Poco::Data::Keywords::use(Date);
			Delete.execute();
			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

}