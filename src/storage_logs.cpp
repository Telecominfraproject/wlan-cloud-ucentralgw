//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"

namespace uCentral {
	typedef Poco::Tuple<
		std::string,
		std::string,
		std::string,
		uint64_t,
		uint64_t,
		uint64_t,
		uint64_t > DeviceLogsRecordTuple;

	bool Storage::AddLog(std::string &SerialNumber, uCentral::Objects::DeviceLog &Log, bool CrashLog) {

		try {
	/*
						"SerialNumber   VARCHAR(30), "
						"Log            TEXT, "
						"Data           TEXT, "
						"Severity       BIGINT, "
						"Recorded       BIGINT, "
						"LogType        BIGINT"
	 */
			uint64_t LogType = CrashLog ? 1 : 0 ;
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Insert(Sess);

			std::string St{"INSERT INTO DeviceLogs (SerialNumber, Log, Data, Severity, Recorded, LogType, UUID ) VALUES(?,?,?,?,?,?,?)"};

			Insert << ConvertParams(St) ,
				Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(Log.Log),
				Poco::Data::Keywords::use(Log.Data),
				Poco::Data::Keywords::use(Log.Severity),
				Poco::Data::Keywords::use(Log.Recorded),
				Poco::Data::Keywords::use(LogType),
				Poco::Data::Keywords::use(Log.UUID);
			Insert.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Storage::AddLog(std::string &SerialNumber, uint64_t UUID, const std::string &Log) {
		uCentral::Objects::DeviceLog DeviceLog;

		DeviceLog.Log = Log;
		DeviceLog.Data = "";
		DeviceLog.Severity = uCentral::Objects::DeviceLog::Level::LOG_INFO;
		DeviceLog.Recorded = time(nullptr);
		DeviceLog.UUID = UUID;

		return AddLog(SerialNumber, DeviceLog, false);
	}

	bool Storage::GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
							 uint64_t HowMany,
							 std::vector<uCentral::Objects::DeviceLog> &Stats, uint64_t Type ) {

	/*
						"SerialNumber   VARCHAR(30), "
						"Log            TEXT, "
						"Data           TEXT, "
						"Severity       BIGINT, "
						"Recorded       BIGINT, "
						"LogType        BIGINT, "
						"UUID			BIGINT
	 */

		typedef std::vector<DeviceLogsRecordTuple> RecordList;

		try {
			RecordList Records;
			Poco::Data::Session Sess = Pool_->get();

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);
			bool HasWhere = DatesIncluded || !SerialNumber.empty();

			std::string Prefix{"SELECT SerialNumber, Log, Data, Severity, Recorded, LogType, UUID FROM DeviceLogs  "};
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

			Select << Statement + DateSelector + TypeSelector,
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::range(Offset, Offset + HowMany );

			Select.execute();

			for (auto i: Records) {
				uCentral::Objects::DeviceLog R{
					.Log = i.get<1>(),
					.Data = i.get<2>(),
					.Severity = i.get<3>(),
					.Recorded = i.get<4>(),
					.LogType = i.get<5>(),
					.UUID = i.get<6>()};
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

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Storage::GetNewestLogData(std::string &SerialNumber, uint64_t HowMany, std::vector<uCentral::Objects::DeviceLog> &Stats, uint64_t Type) {
		typedef std::vector<DeviceLogsRecordTuple> RecordList;

		try {
			RecordList 				Records;
			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);


			std::string st{"SELECT SerialNumber, Log, Data, Severity, Recorded, LogType, UUID FROM DeviceLogs WHERE SerialNumber=? AND LogType=? ORDER BY Recorded DESC"};

			Select << 	ConvertParams(st),
						Poco::Data::Keywords::into(Records),
						Poco::Data::Keywords::use(SerialNumber),
						Poco::Data::Keywords::use(Type),
						Poco::Data::Keywords::limit(HowMany);
			Select.execute();

			for (auto i: Records) {
				uCentral::Objects::DeviceLog R{
					.Log = i.get<1>(),
					.Data = i.get<2>(),
					.Severity = i.get<3>(),
					.Recorded = i.get<4>(),
					.LogType = i.get<5>(),
					.UUID = i.get<6>()};
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

}

