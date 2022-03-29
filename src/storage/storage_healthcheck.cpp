//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"

namespace OpenWifi {

	const static std::string DB_HealthCheckSelectFields{"SerialNumber, UUID, Data, Sanity, Recorded"};
	const static std::string DB_HealthCheckInsertValues{"?,?,?,?,?"};

	typedef Poco::Tuple<
		std::string,
		uint64_t,
		std::string,
		uint64_t,
		uint64_t
	> HealthCheckRecordTuple;
	typedef std::vector<HealthCheckRecordTuple> HealthCheckRecordList;

	void ConvertHealthCheckRecord(const HealthCheckRecordTuple &R, GWObjects::HealthCheck &H) {
		H.SerialNumber = R.get<0>();
		H.UUID = R.get<1>();
		H.Data = R.get<2>();
		H.Sanity = R.get<3>();
		H.Recorded = R.get<4>();
	}

	void ConvertHealthCheckRecord(const GWObjects::HealthCheck &H, HealthCheckRecordTuple &R) {
		R.set<0>(H.SerialNumber);
		R.set<1>(H.UUID);
		R.set<2>(H.Data);
		R.set<3>(H.Sanity);
		R.set<4>(H.Recorded);
	}

	bool Storage::AddHealthCheckData(const GWObjects::HealthCheck &Check) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement   Insert(Sess);

			std::string St{"INSERT INTO HealthChecks ( " +
				DB_HealthCheckSelectFields +
				" ) VALUES( " +
				DB_HealthCheckInsertValues +
				" )"};

			HealthCheckRecordTuple 		R;
			ConvertHealthCheckRecord(Check, R);
			Insert  << 	ConvertParams(St),
				Poco::Data::Keywords::use(R);
			Insert.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
									 uint64_t HowMany,
									 std::vector<GWObjects::HealthCheck> &Checks) {
		try {
			HealthCheckRecordList Records;
			Poco::Data::Session Sess = Pool_->get();

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);

			std::string Prefix{"SELECT " + DB_HealthCheckSelectFields + " FROM HealthChecks "};
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

			Select << Statement + DateSelector + " ORDER BY Recorded ASC " + ComputeRange(Offset,HowMany),
				Poco::Data::Keywords::into(Records);
			Select.execute();

			for (const auto &i: Records) {
				GWObjects::HealthCheck R;
				ConvertHealthCheckRecord(i,R);
				Checks.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetNewestHealthCheckData(std::string &SerialNumber, uint64_t HowMany, std::vector<GWObjects::HealthCheck> &Checks) {

		try {
			HealthCheckRecordList 	Records;
			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string st{"SELECT " + DB_HealthCheckSelectFields + " FROM HealthChecks WHERE SerialNumber=? ORDER BY Recorded DESC "};

			Select << 	ConvertParams(st) + ComputeRange(0,HowMany),
						Poco::Data::Keywords::into(Records),
						Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			for (const auto &i: Records) {
				GWObjects::HealthCheck R;
				ConvertHealthCheckRecord(i,R);
				Checks.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
		try {
			Poco::Data::Session Sess = Pool_->get();

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);

			std::string Prefix{"DELETE FROM HealthChecks "};
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

			Poco::Data::Statement   Delete(Sess);

			Delete << Statement + DateSelector;

			Delete.execute();

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::RemoveHealthChecksRecordsOlderThan(uint64_t Date) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			std::string St1{"delete from HealthChecks where recorded<?"};
			Delete << ConvertParams(St1),
				Poco::Data::Keywords::use(Date);
			Delete.execute();
			return true;
		} catch (const Poco::Exception &E) {
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

}
