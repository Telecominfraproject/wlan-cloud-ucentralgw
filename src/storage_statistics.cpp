//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "DeviceRegistry.h"
#include "StorageService.h"

namespace OpenWifi {

	const static std::string DB_StatsSelectFields{" SerialNumber, UUID, Data, Recorded "};
	const static std::string DB_StatsInsertValues{ "?,?,?,?"};

	typedef Poco::Tuple<
			std::string,
			uint64_t,
			std::string,
			uint64_t> StatsRecordTuple;
	typedef std::vector<StatsRecordTuple> StatsRecordList;

	void ConvertStatsRecord(const StatsRecordTuple &R, GWObjects::Statistics & Stats) {
		Stats.SerialNumber = R.get<0>();
		Stats.UUID = R.get<1>();
		Stats.Data = R.get<2>();
		Stats.Recorded = R.get<3>();
	}

	void ConvertStatsRecord(const GWObjects::Statistics & Stats, StatsRecordTuple & R) {
		R.set<0>(Stats.SerialNumber);
		R.set<1>(Stats.UUID);
		R.set<2>(Stats.Data);
		R.set<3>(Stats.Recorded);
	}

	bool Storage::AddStatisticsData(const GWObjects::Statistics & Stats) {
		DeviceRegistry()->SetStatistics(Stats.SerialNumber, Stats.Data);
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement   Insert(Sess);

			uint64_t Now = time(nullptr);
			Logger_.information("Device:" + Stats.SerialNumber + " Stats size:" + std::to_string(Stats.Data.size()));
			std::string St{"INSERT INTO Statistics ( " +
								DB_StatsSelectFields +
								" ) VALUES ( " +
								DB_StatsInsertValues + " )"};
			StatsRecordTuple R;
			ConvertStatsRecord(Stats, R);
			Insert << ConvertParams(St),
				Poco::Data::Keywords::use(R);
			Insert.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
									uint64_t HowMany,
									std::vector<GWObjects::Statistics> &Stats) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			StatsRecordList         Records;

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);

			std::string Prefix{"SELECT " + DB_StatsSelectFields + " FROM Statistics "};
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

			Select << StatementStr + DateSelector + " ORDER BY Recorded DESC " + ComputeRange(Offset, HowMany),
				Poco::Data::Keywords::into(Records);
			Select.execute();

			for (const auto &i: Records) {
				GWObjects::Statistics R;
				ConvertStatsRecord(i,R);
				Stats.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::GetNewestStatisticsData(std::string &SerialNumber, uint64_t HowMany, std::vector<GWObjects::Statistics> &Stats) {
		try {
			StatsRecordList         Records;
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT " +
						   		DB_StatsSelectFields +
						   		" FROM Statistics WHERE SerialNumber=? ORDER BY Recorded DESC "};
			Select << 	ConvertParams(St) + ComputeRange(1, HowMany),
						Poco::Data::Keywords::into(Records),
						Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			for (const auto &i: Records) {
				GWObjects::Statistics R;
				ConvertStatsRecord(i,R);
				Stats.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.log(E);
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
			Logger_.log(E);
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