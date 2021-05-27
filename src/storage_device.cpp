//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "uCentralConfig.h"
#include "uStorageService.h"
#include "uUtils.h"

namespace uCentral::Storage {

	bool UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t &NewUUID) {
		return Service::instance()->UpdateDeviceConfiguration(SerialNumber, Configuration, NewUUID);
	}

	bool CreateDevice(uCentral::Objects::Device &Device) {
		return Service::instance()->CreateDevice(Device);
	}

	bool CreateDefaultDevice(const std::string &SerialNumber, const std::string &Capabilities) {
		return Service::instance()->CreateDefaultDevice(SerialNumber, Capabilities);
	}

	bool GetDevice(std::string &SerialNumber, uCentral::Objects::Device &Device) {
		return Service::instance()->GetDevice(SerialNumber, Device);
	}

	bool GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::Device> &Devices) {
		return Service::instance()->GetDevices(From, HowMany, Devices);
	}

	bool GetDevices(uint64_t From, uint64_t HowMany, const std::string & Select, std::vector<uCentral::Objects::Device> &Devices) {
		return Service::instance()->GetDevices(From, HowMany, Select, Devices);
	}

	bool DeleteDevice(std::string &SerialNumber) {
		return Service::instance()->DeleteDevice(SerialNumber);
	}

	bool UpdateDevice(uCentral::Objects::Device &Device) {
		return Service::instance()->UpdateDevice(Device);
	}

	bool SetOwner(std::string & SerialNumber, std::string & OwnerUUID) {
		return Service::instance()->SetOwner(SerialNumber, OwnerUUID);
	}

	bool SetLocation(std::string & SerialNumber, std::string & LocationUUID) {
		return Service::instance()->SetLocation(SerialNumber, LocationUUID);
	}

	bool SetFirmware(std::string & SerialNumber, std::string & Firmware ) {
		return Service::instance()->SetFirmware(SerialNumber, Firmware);
	}

	bool GetDevicesWithoutFirmware(std::string &DeviceType, std::string &Version, std::vector<std::string> & SerialNumbers) {
		return Service::instance()->GetDevicesWithoutFirmware(DeviceType,Version, SerialNumbers);
	}

	bool GetDeviceCount( uint64_t & Count ) {
		return Service::instance()->GetDeviceCount(Count);
	}

	bool GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> & SerialNumbers) {
		return Service::instance()->GetDeviceSerialNumbers(From, HowMany, SerialNumbers);
	}

	bool DeviceExists(std::string &SerialNumber) {
		return Service::instance()->DeviceExists(SerialNumber);
	}

	bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig,
							   uint64_t &NewerUUID) {
		return Service::instance()->ExistingConfiguration(SerialNumber, CurrentConfig, NewConfig,
																			 NewerUUID);
	}

	bool GetDeviceFWUpdatePolicy(std::string & SerialNumber, std::string & Policy) {
		return Service::instance()->GetDeviceFWUpdatePolicy(SerialNumber, Policy);
	}


bool SetDeviceCompatibility(std::string & SerialNumber, std::string & Compatible) {
		return Service::instance()->SetDeviceCompatibility(SerialNumber, Compatible);
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

			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			uint64_t CurrentUUID;

			std::string St{"SELECT UUID FROM Devices WHERE SerialNumber=?"};

			Select << 	ConvertParams(St),
						Poco::Data::Keywords::into(CurrentUUID),
						Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			std::cout << "CurrentUUID" << CurrentUUID << std::endl;
			uint64_t Now = time(nullptr);
			NewUUID = CurrentUUID==Now ? Now + 1 : Now;

			std::cout << "NewUUID" << NewUUID << std::endl;

			if (Cfg.SetUUID(NewUUID)) {
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

				Configuration = NewConfig;

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

	bool Service::CreateDevice(uCentral::Objects::Device &DeviceDetails) {
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

					Poco::Data::Statement   Insert(Sess);

					std::string St2{"INSERT INTO Devices ("
									"SerialNumber,"
									"DeviceType, "
									"MACAddress, "
									"Manufacturer, "
									"Configuration, "
									"Notes, "
									"Owner, "
									"Location, "
									"Firmware,"
									"Compatible,"
									"FWUpdatePolicy,"
									"UUID,      "
									"CreationTimestamp,   "
									"LastConfigurationChange, "
									"LastConfigurationDownload, "
									"LastFWUpdate "
									")"
									"VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"};

					Insert  << ConvertParams(St2),
						Poco::Data::Keywords::use(DeviceDetails.SerialNumber),
						Poco::Data::Keywords::use(DeviceDetails.DeviceType),
						Poco::Data::Keywords::use(DeviceDetails.MACAddress),
						Poco::Data::Keywords::use(DeviceDetails.Manufacturer),
						Poco::Data::Keywords::use(DeviceDetails.Configuration),
						Poco::Data::Keywords::use(DeviceDetails.Notes),
						Poco::Data::Keywords::use(DeviceDetails.Owner),
						Poco::Data::Keywords::use(DeviceDetails.Location),
						Poco::Data::Keywords::use(DeviceDetails.Firmware),
						Poco::Data::Keywords::use(DeviceDetails.Compatible),
						Poco::Data::Keywords::use(DeviceDetails.FWUpdatePolicy),
						Poco::Data::Keywords::use(DeviceDetails.UUID),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(Now);
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

		uCentral::Objects::Device D;
		Logger_.information(Poco::format("AUTO-CREATION(%s)", SerialNumber));
		uint64_t Now = time(nullptr);

		uCentral::Config::Capabilities Caps(Capabilities);
		uCentral::Objects::DefaultConfiguration DefConfig;

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

	bool Service::GetDeviceFWUpdatePolicy(std::string &SerialNumber, std::string &Policy) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT FWUpdatePolicy FROM Devices WHERE SerialNumber=?"};
			Select  << ConvertParams(St) ,
				Poco::Data::Keywords::into(Policy),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();
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
			Poco::Data::Statement   Select(Sess);

			//	Get the old version and if they do not match, set the last date
			std::string St{"SELECT Firmware FROM Devices  WHERE SerialNumber=?"};
			std::string TmpFirmware;
			Select << ConvertParams(St) ,
				Poco::Data::Keywords::into(TmpFirmware),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if(TmpFirmware != Firmware) {
				Poco::Data::Statement	Update(Sess);
				std::string St2{"UPDATE Devices SET Firmware=?, LastFWUpdate=? WHERE SerialNumber=?"};
				uint64_t 	Now = time(nullptr);

				Update << 	ConvertParams(St2),
							Poco::Data::Keywords::use(Firmware),
							Poco::Data::Keywords::use(Now),
							Poco::Data::Keywords::use(SerialNumber);
				Update.execute();
				return true;
			}
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

	bool Service::GetDevice(std::string &SerialNumber, uCentral::Objects::Device &DeviceDetails) {
		// std::lock_guard<std::mutex> guard(Mutex_);

		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT "
						   "SerialNumber,"
						   "DeviceType, "
						   "MACAddress, "
						   "Manufacturer, "
						   "Configuration, "
						   "Notes, "
						   "Owner, "
						   "Location, "
						   "Firmware,"
						   "Compatible,"
						   "FWUpdatePolicy,"
						   "UUID,      "
						   "CreationTimestamp,   "
						   "LastConfigurationChange, "
						   "LastConfigurationDownload, "
						   "LastFWUpdate "
						   "FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(DeviceDetails.SerialNumber),
				Poco::Data::Keywords::into(DeviceDetails.DeviceType),
				Poco::Data::Keywords::into(DeviceDetails.MACAddress),
				Poco::Data::Keywords::into(DeviceDetails.Manufacturer),
				Poco::Data::Keywords::into(DeviceDetails.Configuration),
				Poco::Data::Keywords::into(DeviceDetails.Notes),
				Poco::Data::Keywords::into(DeviceDetails.Owner),
				Poco::Data::Keywords::into(DeviceDetails.Location),
				Poco::Data::Keywords::into(DeviceDetails.Firmware),
				Poco::Data::Keywords::into(DeviceDetails.Compatible),
				Poco::Data::Keywords::into(DeviceDetails.FWUpdatePolicy),
				Poco::Data::Keywords::into(DeviceDetails.UUID),
				Poco::Data::Keywords::into(DeviceDetails.CreationTimestamp),
				Poco::Data::Keywords::into(DeviceDetails.LastConfigurationChange),
				Poco::Data::Keywords::into(DeviceDetails.LastConfigurationDownload),
				Poco::Data::Keywords::into(DeviceDetails.LastFWUpdate),
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

	bool Service::GetDevicesWithoutFirmware(std::string &Compatible, std::string &Version, std::vector<std::string> &SerialNumbers) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT SerialNumber FROM Devices WHERE Compatible=? AND Firmware!=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(SerialNumbers),
				Poco::Data::Keywords::use(Compatible),
				Poco::Data::Keywords::use(Version);
			Select.execute();

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Service::UpdateDevice(uCentral::Objects::Device &NewConfig) {
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

	bool Service::GetDevices(uint64_t From, uint64_t HowMany, const std::string &Select, std::vector<uCentral::Objects::Device> &Devices) {
		return false;
	}

	bool Service::GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::Device> &Devices) {

		typedef Poco::Tuple<
			std::string,
			std::string,
			std::string,
			std::string,
			std::string,
			std::string,
			std::string,
			std::string,
			std::string,
			std::string,
			std::string,
			uint64_t,
			uint64_t,
			uint64_t,
			uint64_t,
			uint64_t
		> DeviceRecord;
		typedef std::vector<DeviceRecord> RecordList;

		RecordList Records;

		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select << "SELECT "
					  "SerialNumber,"
					  "DeviceType, "
					  "MACAddress, "
					  "Manufacturer, "
					  "Configuration, "
					  "Notes, "
					  "Owner, "
					  "Location, "
					  "Firmware,"
					  "Compatible,"
					  "FWUpdatePolicy,"
					  "UUID,      "
					  "CreationTimestamp,   "
					  "LastConfigurationChange, "
					  "LastConfigurationDownload, "
					  "LastFWUpdate "
					  "FROM Devices",
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::range(From, From + HowMany );
			Select.execute();

			for (auto i: Records) {
				uCentral::Objects::Device R{
					.SerialNumber   = i.get<0>(),
					.DeviceType     = i.get<1>(),
					.MACAddress     = i.get<2>(),
					.Manufacturer   = i.get<3>(),
					.Configuration  = i.get<4>(),
					.Notes  		= i.get<5>(),
					.Owner          = i.get<6>(),
					.Location 		= i.get<7>(),
					.Firmware 		= i.get<8>(),
					.Compatible 	= i.get<9>(),
					.FWUpdatePolicy = i.get<10>(),
					.UUID			= i.get<11>(),
					.CreationTimestamp = i.get<12>(),
					.LastConfigurationChange = i.get<13>(),
					.LastConfigurationDownload = i.get<14>(),
					.LastFWUpdate = i.get<15>()};
				Devices.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Service::ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t & UUID) {
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
				//	No configuration exists, so we should
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

	bool Service::SetDeviceCompatibility(std::string &SerialNumber, std::string &Compatible) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);

			std::string st{"UPDATE Devices SET Compatible=? WHERE SerialNumber=?"};

			Update << 	ConvertParams(st),
						Poco::Data::Keywords::use(Compatible),
						Poco::Data::Keywords::use(SerialNumber);
			Update.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}
}

