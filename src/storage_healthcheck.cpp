//
// Created by stephane bourque on 2021-05-06.
//
#include "uStorageService.h"

namespace uCentral::Storage {

	bool AddHealthCheckData(std::string &SerialNumber, uCentral::Objects::HealthCheck &Check) {
		return uCentral::Storage::Service::instance()->AddHealthCheckData(SerialNumber, Check);
	}

	bool
	GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
					   std::vector<uCentral::Objects::HealthCheck> &Checks) {
		return uCentral::Storage::Service::instance()->GetHealthCheckData(SerialNumber, FromDate, ToDate, Offset,
																		  HowMany,
																		  Checks);
	}

	bool DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
		return uCentral::Storage::Service::instance()->DeleteHealthCheckData(SerialNumber, FromDate, ToDate);
	}


	bool Service::AddHealthCheckData(std::string &SerialNumber, uCentral::Objects::HealthCheck &Check) {
		try {
			Logger_.information("Device:" + SerialNumber + " HealthCheck: sanity " + std::to_string(Check.Sanity));

			Poco::Data::Session Sess = Pool_->get();

	/*          "SerialNumber VARCHAR(30), "
				"UUID          BIGINT, "
				"Data TEXT, "
				"Sanity BIGINT , "
				"Recorded BIGINT) ", now;
	*/
			Poco::Data::Statement   Insert(Sess);
			std::string St{"INSERT INTO HealthChecks (SerialNumber, UUID, Data, Sanity, Recorded) VALUES(?,?,?,?,?)"};

			Insert  << 	ConvertParams(St),
				Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(Check.UUID),
				Poco::Data::Keywords::use(Check.Data),
				Poco::Data::Keywords::use(Check.Sanity),
				Poco::Data::Keywords::use(Check.Recorded);
			Insert.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
									 uint64_t HowMany,
									 std::vector<uCentral::Objects::HealthCheck> &Checks) {

		typedef Poco::Tuple<std::string, uint64_t, std::string, uint64_t, uint64_t> Record;
		typedef std::vector<Record> RecordList;

		// std::lock_guard<std::mutex> guard(Mutex_);
		try {
			RecordList Records;
			Poco::Data::Session Sess = Pool_->get();

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);

			std::string Prefix{"SELECT SerialNumber, UUID, Data, Sanity, Recorded FROM HealthChecks "};
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

			Select << Statement + DateSelector,
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::range(Offset, Offset + HowMany );
			Select.execute();

			for (auto i: Records) {
				uCentral::Objects::HealthCheck R;

				R.UUID = i.get<1>();
				R.Data = i.get<2>();
				R.Sanity = i.get<3>();
				R.Recorded = i.get<4>();

				Checks.push_back(R);
			}

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
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
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}
}
