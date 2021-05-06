//
// Created by stephane bourque on 2021-05-06.
//

#include "uStorageService.h"

namespace uCentral::Storage {
	bool UpdateDeviceCapabilities(std::string &SerialNumber, std::string &State) {
		return uCentral::Storage::Service::instance()->UpdateDeviceCapabilities(SerialNumber, State);
	}

	bool GetDeviceCapabilities(std::string &SerialNumber, uCentralCapabilities &Capabilities) {
		return uCentral::Storage::Service::instance()->GetDeviceCapabilities(SerialNumber, Capabilities);
	}

	bool DeleteDeviceCapabilities(std::string &SerialNumber) {
		return uCentral::Storage::Service::instance()->DeleteDeviceCapabilities(SerialNumber);
	}

	bool Service::UpdateDeviceCapabilities(std::string &SerialNumber, std::string & Capabilities) {
		// std::lock_guard<std::mutex> guard(Mutex_);

		try {
			std::string SS;

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			uint64_t Now = time(nullptr);

			std::string St{"SELECT SerialNumber FROM Capabilities WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(SS),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

	/*
						"SerialNumber VARCHAR(30) PRIMARY KEY, "
						"Capabilities TEXT, "
						"FirstUpdate BIGINT, "
						"LastUpdate BIGINT"

	 */
			if (SS.empty()) {
				Logger_.information("Adding capabilities for " + SerialNumber);
				Poco::Data::Statement   Insert(Sess);

				std::string St2{"INSERT INTO Capabilities (SerialNumber, Capabilities, FirstUpdate, LastUpdate) "
								"VALUES(?,?,?,?)"};

				Insert  << ConvertParams(St2),
					Poco::Data::Keywords::use(SerialNumber),
					Poco::Data::Keywords::use(Capabilities),
					Poco::Data::Keywords::use(Now),
					Poco::Data::Keywords::use(Now);
				Insert.execute();

			} else {
				Logger_.information("Updating capabilities for " + SerialNumber);
				Poco::Data::Statement   Update(Sess);

				std::string St2{"UPDATE Capabilities SET Capabilities=?, LastUpdate=? WHERE SerialNumber=?"};

				Update  << 	ConvertParams(St2),
					Poco::Data::Keywords::use(Capabilities),
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

	bool Service::GetDeviceCapabilities(std::string &SerialNumber, uCentralCapabilities &Caps) {
		// std::lock_guard<std::mutex> guard(Mutex_);

		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string TmpSerialNumber;

			std::string St{"SELECT SerialNumber, Capabilities, FirstUpdate, LastUpdate FROM Capabilities WHERE SerialNumber=?"};

			Select  << ConvertParams(St),
				Poco::Data::Keywords::into(TmpSerialNumber),
				Poco::Data::Keywords::into(Caps.Capabilities),
				Poco::Data::Keywords::into(Caps.FirstUpdate),
				Poco::Data::Keywords::into(Caps.LastUpdate),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if (TmpSerialNumber.empty())
				return false;

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::DeleteDeviceCapabilities(std::string &SerialNumber) {
		// std::lock_guard<std::mutex> guard(Mutex_);

		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Delete(Sess);

			std::string St{"DELETE FROM Capabilities WHERE SerialNumber=?"};

			Delete << ConvertParams(St),
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