//
// Created by stephane bourque on 2021-05-06.
//

#include "uStorageService.h"
#include "uCentralConfig.h"
#include "utils.h"

namespace uCentral::Storage {

	bool UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t &NewUUID) {
		return uCentral::Storage::Service::instance()->UpdateDeviceConfiguration(SerialNumber, Configuration, NewUUID);
	}

	bool CreateDevice(uCentralDevice &Device) {
		return uCentral::Storage::Service::instance()->CreateDevice(Device);
	}

	bool CreateDefaultDevice(const std::string &SerialNumber, const std::string &Capabilities) {
		return uCentral::Storage::Service::instance()->CreateDefaultDevice(SerialNumber, Capabilities);
	}

	bool GetDevice(std::string &SerialNumber, uCentralDevice &Device) {
		return uCentral::Storage::Service::instance()->GetDevice(SerialNumber, Device);
	}

	bool GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentralDevice> &Devices) {
		return uCentral::Storage::Service::instance()->GetDevices(From, HowMany, Devices);
	}

	bool GetDevices(uint64_t From, uint64_t HowMany, const std::string & Select, std::vector<uCentralDevice> &Devices) {
		return uCentral::Storage::Service::instance()->GetDevices(From, HowMany, Select, Devices);
	}

	bool DeleteDevice(std::string &SerialNumber) {
		return uCentral::Storage::Service::instance()->DeleteDevice(SerialNumber);
	}

	bool UpdateDevice(uCentralDevice &Device) {
		return uCentral::Storage::Service::instance()->UpdateDevice(Device);
	}

	bool SetOwner(std::string & SerialNumber, std::string & OwnerUUID) {
		return uCentral::Storage::Service::instance()->SetOwner(SerialNumber, OwnerUUID);
	}

	bool SetLocation(std::string & SerialNumber, std::string & LocationUUID) {
		return uCentral::Storage::Service::instance()->SetLocation(SerialNumber, LocationUUID);
	}

	bool SetFirmware(std::string & SerialNumber, std::string & Firmware ) {
		return uCentral::Storage::Service::instance()->SetFirmware(SerialNumber, Firmware);
	}

	bool GetDeviceCount( uint64_t & Count ) {
		return uCentral::Storage::Service::instance()->GetDeviceCount(Count);
	}

	bool GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> & SerialNumbers) {
		return uCentral::Storage::Service::instance()->GetDeviceSerialNumbers(From, HowMany, SerialNumbers);
	}

	bool DeviceExists(std::string &SerialNumber) {
		return uCentral::Storage::Service::instance()->DeviceExists(SerialNumber);
	}

	bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig,
							   uint64_t &NewerUUID) {
		return uCentral::Storage::Service::instance()->ExistingConfiguration(SerialNumber, CurrentConfig, NewConfig,
																			 NewerUUID);
	}

	bool Service::GetDeviceCount(uint64_t &Count) {
		try {
			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string st{"SELECT COUNT(*) FROM Devices"};

			Select << st ,
				Poco::Data::Keywords::into(Count);
			Select.execute();

			return true;

		} catch(const Poco::Exception & E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Service::GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> &SerialNumbers) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string st{"SELECT SerialNumber From Devices"};

			Select << 	st,
				Poco::Data::Keywords::into(SerialNumbers),
				Poco::Data::Keywords::range(From, From + HowMany );
			Select.execute();
			return true;
		} catch (const Poco::Exception &E ) {
			Logger_.log(E);
		}
		return false;
	}

	bool Service::UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t &NewUUID) {
		try {

			uCentral::Config::Config Cfg(Configuration);

			if (!Cfg.Valid()) {
				Logger_.warning(Poco::format("CONFIG-UPDATE(%s): Configuration was not valid", SerialNumber));
				return false;
			}

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			uint64_t CurrentUUID;

			std::string St{"SELECT UUID FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(CurrentUUID),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			uint64_t Now = time(nullptr);

			NewUUID = CurrentUUID==Now ? Now + 1 : Now;

			if (Cfg.SetUUID(CurrentUUID)) {
				std::string NewConfig = Cfg.get();

				Poco::Data::Statement   Update(Sess);
				std::string St2{"UPDATE Devices SET Configuration=? , UUID=?,  LastConfigurationChange=?  WHERE SerialNumber=?"};
				Update  << ConvertParams(St2),
					Poco::Data::Keywords::use(NewConfig),
					Poco::Data::Keywords::use(NewUUID),
					Poco::Data::Keywords::use(Now),
					Poco::Data::Keywords::use(SerialNumber);
				Update.execute();

				Logger_.information(Poco::format("CONFIG-UPDATE(%s): UUID is %Lu", SerialNumber, NewUUID));

				return true;
			}

			return false;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::CreateDevice(uCentralDevice &DeviceDetails) {
		// std::lock_guard<std::mutex> guard(Mutex_);

		std::string SerialNumber;
		try {

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT SerialNumber FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(SerialNumber),
				Poco::Data::Keywords::use(DeviceDetails.SerialNumber);
			Select.execute();

			if (SerialNumber.empty()) {
				uCentral::Config::Config Cfg(DeviceDetails.Configuration);

				if (Cfg.Valid() && Cfg.SetUUID(DeviceDetails.UUID)) {
					DeviceDetails.Configuration = Cfg.get();
					uint64_t Now = time(nullptr);

					// DeviceDetails.Print();
	/*
						 "SerialNumber  VARCHAR(30) UNIQUE PRIMARY KEY, "
						"DeviceType    VARCHAR(32), "
						"MACAddress    VARCHAR(30), "
						"Manufacturer  VARCHAR(64), "
						"UUID          BIGINT, "
						"Configuration TEXT, "
						"Notes         TEXT, "
						"CreationTimestamp BIGINT, "
						"LastConfigurationChange BIGINT, "
						"LastConfigurationDownload BIGINT, "
						"Owner 			VARCHAR(64),
						"Location		VARCHAR(64)"

	 */
					Poco::Data::Statement   Insert(Sess);

					std::string St2{"INSERT INTO Devices (SerialNumber, DeviceType, MACAddress, Manufacturer, UUID, "
									"Configuration, Notes, CreationTimestamp, LastConfigurationChange, LastConfigurationDownload,"
									"Owner, Location )"
									"VALUES(?,?,?,?,?,?,?,?,?,?,?,?)"};

					Insert  << ConvertParams(St2),
						Poco::Data::Keywords::use(DeviceDetails.SerialNumber),
						Poco::Data::Keywords::use(DeviceDetails.DeviceType),
						Poco::Data::Keywords::use(DeviceDetails.MACAddress),
						Poco::Data::Keywords::use(DeviceDetails.Manufacturer),
						Poco::Data::Keywords::use(DeviceDetails.UUID),
						Poco::Data::Keywords::use(DeviceDetails.Configuration),
						Poco::Data::Keywords::use(DeviceDetails.Notes),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(DeviceDetails.Owner),
						Poco::Data::Keywords::use(DeviceDetails.Location);
					Insert.execute();

					return true;
				} else {
					Logger_.warning("Cannot create device: invalid configuration.");
					return false;
				}
			}
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::CreateDefaultDevice(const std::string &SerialNumber, const std::string &Capabilities) {

		uCentralDevice D;
		Logger_.information(Poco::format("AUTO-CREATION(%s)", SerialNumber));
		uint64_t Now = time(nullptr);

		uCentral::Config::Capabilities Caps(Capabilities);
		uCentralDefaultConfiguration DefConfig;

		if (FindDefaultConfigurationForModel(Caps.ModelId(), DefConfig)) {
			uCentral::Config::Config NewConfig(DefConfig.Configuration);
			NewConfig.SetUUID(Now);
			D.Configuration = NewConfig.get();
		} else {
			uCentral::Config::Config NewConfig;
			NewConfig.SetUUID(Now);
			D.Configuration = NewConfig.get();
		}

		D.SerialNumber = SerialNumber;
		D.DeviceType = Caps.DeviceType();
		D.MACAddress = uCentral::Utils::SerialToMAC(SerialNumber);
		D.Manufacturer = Caps.Manufacturer();
		D.UUID = Now;
		D.Notes = "auto created device.";
		D.CreationTimestamp = D.LastConfigurationDownload = D.LastConfigurationChange = Now;

		return CreateDevice(D);
	}

	bool Service::SetLocation(std::string & SerialNumber, std::string & LocationUUID) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);

			std::string St{"UPDATE Devices SET Location=? WHERE SerialNumber=?"};

			Update  << ConvertParams(St) ,
				Poco::Data::Keywords::use(LocationUUID),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::SetOwner(std::string & SerialNumber, std::string & OwnerUUID) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);
			std::string St{"UPDATE Devices SET Owner=?  WHERE SerialNumber=?"};

			Update << ConvertParams(St) ,
				Poco::Data::Keywords::use(OwnerUUID),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::SetFirmware(std::string &SerialNumber, std::string &Firmware) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);
			std::string St{"UPDATE Devices SET Firmware=?  WHERE SerialNumber=?"};

			Update << ConvertParams(St) ,
				Poco::Data::Keywords::use(Firmware),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::DeleteDevice(std::string &SerialNumber) {
		// std::lock_guard<std::mutex> guard(Mutex_);

		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Delete(Sess);

			std::string St{"DELETE FROM Devices WHERE SerialNumber=?"};

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

	bool Service::GetDevice(std::string &SerialNumber, uCentralDevice &DeviceDetails) {
		// std::lock_guard<std::mutex> guard(Mutex_);

		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT "
						   "SerialNumber, "
						   "DeviceType, "
						   "MACAddress, "
						   "Manufacturer, "
						   "UUID, "
						   "Configuration, "
						   "Notes, "
						   "CreationTimestamp, "
						   "LastConfigurationChange, "
						   "LastConfigurationDownload, "
						   "Owner,"
						   "Location, "
						   "Firmware "
						   "FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(DeviceDetails.SerialNumber),
				Poco::Data::Keywords::into(DeviceDetails.DeviceType),
				Poco::Data::Keywords::into(DeviceDetails.MACAddress),
				Poco::Data::Keywords::into(DeviceDetails.Manufacturer),
				Poco::Data::Keywords::into(DeviceDetails.UUID),
				Poco::Data::Keywords::into(DeviceDetails.Configuration),
				Poco::Data::Keywords::into(DeviceDetails.Notes),
				Poco::Data::Keywords::into(DeviceDetails.CreationTimestamp),
				Poco::Data::Keywords::into(DeviceDetails.LastConfigurationChange),
				Poco::Data::Keywords::into(DeviceDetails.LastConfigurationDownload),
				Poco::Data::Keywords::into(DeviceDetails.Owner),
				Poco::Data::Keywords::into(DeviceDetails.Location),
				Poco::Data::Keywords::into(DeviceDetails.Firmware),
				Poco::Data::Keywords::use(SerialNumber);

			Select.execute();

			if (DeviceDetails.SerialNumber.empty())
				return false;

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::DeviceExists(std::string &SerialNumber) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string Serial;

			std::string St{"SELECT SerialNumber FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(Serial),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if (Serial.empty())
				return false;

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::UpdateDevice(uCentralDevice &NewConfig) {
		// std::lock_guard<std::mutex> guard(Mutex_);

		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);

			uint64_t Now = time(nullptr);

			std::string St{"UPDATE Devices SET Manufacturer=?, DeviceType=?, MACAddress=?, Notes=?, "
						   "LastConfigurationChange=? WHERE SerialNumber=?"};

			Update  << ConvertParams(St) ,
				Poco::Data::Keywords::use(NewConfig.Manufacturer),
				Poco::Data::Keywords::use(NewConfig.DeviceType),
				Poco::Data::Keywords::use(NewConfig.MACAddress),
				Poco::Data::Keywords::use(NewConfig.Notes),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(NewConfig.SerialNumber);

			Update.execute();

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), NewConfig.SerialNumber,
										 E.displayText()));
		}

		return false;
	}

	bool Service::GetDevices(uint64_t From, uint64_t HowMany, const std::string &Select, std::vector<uCentralDevice> &Devices) {
		return false;
	}

	bool Service::GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentralDevice> &Devices) {

		typedef Poco::Tuple<
			std::string,
			std::string,
			std::string,
			std::string,
			uint64_t,
			std::string,
			std::string,
			uint64_t,
			uint64_t,
			uint64_t,
			std::string,
			std::string,
			std::string> DeviceRecord;
		typedef std::vector<DeviceRecord> RecordList;

		RecordList Records;

		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select << "SELECT "
					  "SerialNumber, "
					  "DeviceType, "
					  "MACAddress, "
					  "Manufacturer, "
					  "UUID, "
					  "Configuration, "
					  "Notes, "
					  "CreationTimestamp, "
					  "LastConfigurationChange, "
					  "LastConfigurationDownload, "
					  "Owner, "
					  "Location, "
					  "Firmware "
					  "FROM Devices",
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::range(From, From + HowMany );
			Select.execute();

			for (auto i: Records) {
				uCentralDevice R{
					.SerialNumber   = i.get<0>(),
					.DeviceType     = i.get<1>(),
					.MACAddress     = i.get<2>(),
					.Manufacturer   = i.get<3>(),
					.UUID           = i.get<4>(),
					.Configuration  = i.get<5>(),
					.Notes          = i.get<6>(),
					.CreationTimestamp = i.get<7>(),
					.LastConfigurationChange = i.get<8>(),
					.LastConfigurationDownload = i.get<9>(),
					.Owner = i.get<10>(),
					.Location = i.get<11>(),
					.Firmware = i.get<12>()};

				Devices.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Service::ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig,
									uint64_t &UUID) {
		// std::lock_guard<std::mutex> guard(Mutex_);
		std::string SS;
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);
			uint64_t Now = time(nullptr);

			std::string St{"SELECT SerialNumber, UUID, Configuration FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(SS),
				Poco::Data::Keywords::into(UUID),
				Poco::Data::Keywords::into(NewConfig),
				Poco::Data::Keywords::use(SerialNumber);

			Select.execute();

			if (SS.empty()) {
				return false;
			}

			//  Let's update the last downloaded time
			Poco::Data::Statement   Update(Sess);
			std::string St2{"UPDATE Devices SET LastConfigurationDownload=?  WHERE SerialNumber=?"};

			Update << ConvertParams(St2),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}
}

