//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"
#include "fmt/format.h"

namespace OpenWifi {
	const static std::string DB_LogsSelectFields{"SerialNumber, Log, Data, Severity, Recorded, LogType, UUID"};
	const static std::string DB_LogsInsertValues{"?,?,?,?,?,?,?"};

	typedef Poco::Tuple<
		std::string,
		std::string,
		std::string,
		uint64_t,
		uint64_t,
		uint64_t,
		uint64_t
	> DeviceLogsRecordTuple;
	typedef std::vector<DeviceLogsRecordTuple >	DeviceLogsRecordList;

	void ConvertLogsRecord( const DeviceLogsRecordTuple & R, GWObjects::DeviceLog & Log ) {
		Log.SerialNumber = R.get<0>();
		Log.Log = R.get<1>();
		Log.Data = R.get<2>();
		Log.Severity = R.get<3>();
		Log.Recorded = R.get<4>();
		Log.LogType = R.get<5>();
		Log.UUID = R.get<6>();
	}

	void ConvertLogsRecord( const GWObjects::DeviceLog & Log , DeviceLogsRecordTuple & R ) {
		R.set<0>(Log.SerialNumber);
		R.set<1>(Log.Log);
		R.set<2>(Log.Data);
		R.set<3>(Log.Severity);
		R.set<4>(Log.Recorded);
		R.set<5>(Log.LogType);
		R.set<6>(Log.UUID);
	}

	bool Storage::AddLog(const GWObjects::DeviceLog & Log) {
		try {

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Insert(Sess);

			std::string St{"INSERT INTO DeviceLogs (" +
				DB_LogsSelectFields +
				") values( " +
				DB_LogsInsertValues + " )"};

			DeviceLogsRecordTuple	R;
			ConvertLogsRecord(Log, R);

			Insert << ConvertParams(St) ,
				Poco::Data::Keywords::use(R);
			Insert.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			poco_warning(Logger(),fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
							 uint64_t HowMany,
							 std::vector<GWObjects::DeviceLog> &Stats, uint64_t Type ) {
		try {
			DeviceLogsRecordList Records;
			Poco::Data::Session Sess = Pool_->get();

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);
			bool HasWhere = DatesIncluded || !SerialNumber.empty();

			std::string Prefix{"SELECT " + DB_LogsSelectFields + " FROM DeviceLogs  "};
			std::string Statement = SerialNumber.empty()
									? Prefix + std::string(DatesIncluded ? "WHERE " : "")
									: Prefix + "WHERE SerialNumber='" + SerialNumber + "'" +
									  std::string(DatesIncluded ? " AND " : "") ;

			std::string DateSelector;
			if (FromDate && ToDate) {
				DateSelector = " Recorded>=" + std::to_string(FromDate) + " AND Recorded<=" + std::to_string(ToDate);
			} else if (FromDate) {
				DateSelector = " Recorded>=" + std::to_string(FromDate);
			} else if (ToDate) {
				DateSelector = " Recorded<=" + std::to_string(ToDate);
			}

			std::string TypeSelector;
			TypeSelector = (HasWhere ? " AND LogType=" : " WHERE LogType=" ) + std::to_string(Type);
			Poco::Data::Statement   Select(Sess);

			Select << Statement + DateSelector + TypeSelector + " ORDER BY Recorded DESC " + ComputeRange(Offset, HowMany),
				Poco::Data::Keywords::into(Records);
			Select.execute();

			for (const auto &i: Records) {
				GWObjects::DeviceLog R;
				ConvertLogsRecord(i,R);
				Stats.push_back(R);
			}
			Select.reset(Sess);
			return true;
		}
		catch (const Poco::Exception &E) {
			poco_warning(Logger(),fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Type) {
		try {
			Poco::Data::Session Sess = Pool_->get();

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);
			bool HasWhere = DatesIncluded || !SerialNumber.empty();

			std::string Prefix{"DELETE FROM DeviceLogs "};
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

			std::string TypeSelector;
			TypeSelector = (HasWhere ? " AND LogType=" : " WHERE LogType=" ) + std::to_string(Type);

			Poco::Data::Statement   Delete(Sess);
			Delete << StatementStr + DateSelector + TypeSelector;

			Delete.execute();
			Delete.reset(Sess);

			return true;
		}
		catch (const Poco::Exception &E) {
			poco_warning(Logger(),fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetNewestLogData(std::string &SerialNumber, uint64_t HowMany, std::vector<GWObjects::DeviceLog> &Stats, uint64_t Type) {
		try {
			DeviceLogsRecordList 	Records;
			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);


			std::string st{"SELECT " + DB_LogsSelectFields + " FROM DeviceLogs WHERE SerialNumber=? AND LogType=? ORDER BY Recorded DESC " + ComputeRange(0, HowMany)};
			Select << 	ConvertParams(st),
						Poco::Data::Keywords::into(Records),
						Poco::Data::Keywords::use(SerialNumber),
						Poco::Data::Keywords::use(Type);
			Select.execute();

			for (const auto &i: Records) {
				GWObjects::DeviceLog R;
				ConvertLogsRecord(i,R);
				Stats.push_back(R);
			}
			Select.reset(Sess);
			return true;
		}
		catch (const Poco::Exception &E) {
			poco_warning(Logger(),fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::RemoveDeviceLogsRecordsOlderThan(uint64_t Date) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			std::string St1{"delete from DeviceLogs where recorded<?"};
			Delete << ConvertParams(St1), Poco::Data::Keywords::use(Date);
			Delete.execute();
			return true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(),fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

}

