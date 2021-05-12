//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "uStorageService.h"

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"

namespace uCentral::Storage {
	bool UpdateDeviceCapabilities(std::string &SerialNumber, std::string &Capabilities) {
		return uCentral::Storage::Service::instance()->UpdateDeviceCapabilities(SerialNumber, Capabilities);
	}

	bool GetDeviceCapabilities(std::string &SerialNumber, uCentral::Objects::Capabilities &Capabilities) {
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

			//	Find compatible in the capabilities...
			std::string Compatible;

			Poco::JSON::Parser	P;
			Poco::JSON::Parser      IncomingParser;
			Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Capabilities).extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *Obj;

			if(ds.contains("compatible"))
				Compatible = ds["compatible"].toString();
			else
				Compatible = "unknown";

			std::cout << SerialNumber << " is a " << Compatible << std::endl;

			std::string St{"SELECT SerialNumber FROM Capabilities WHERE SerialNumber=?"};
			Select << ConvertParams(St),
				Poco::Data::Keywords::into(SS),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

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

			uCentral::Storage::SetDeviceCompatibility(SerialNumber, Compatible);
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::GetDeviceCapabilities(std::string &SerialNumber, uCentral::Objects::Capabilities &Caps) {
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