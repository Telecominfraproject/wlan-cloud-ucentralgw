//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Data/RecordSet.h"
#include "CentralConfig.h"
#include "StorageService.h"
#include "Utils.h"
#include "RESTAPI_utils.h"
#include "Daemon.h"
#include "DeviceRegistry.h"
#include "OUIServer.h"

namespace uCentral {

	bool Storage::GetDeviceCount(uint64_t &Count) {
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

	bool Storage::GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> &SerialNumbers) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string st{"SELECT SerialNumber From Devices"};

			Select << 	st,
				Poco::Data::Keywords::into(SerialNumbers),
				Poco::Data::Keywords::range(From, HowMany );
			Select.execute();
			return true;
		} catch (const Poco::Exception &E ) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t &NewUUID) {
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

			uint64_t Now = time(nullptr);
			NewUUID = CurrentUUID==Now ? Now + 1 : Now;

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

	bool Storage::CreateDevice(GWObjects::Device &DeviceDetails) {
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
									"LastFWUpdate, "
									"Venue "
									")"
									"VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"};

					auto NotesString = RESTAPI_utils::to_string(DeviceDetails.Notes);

					Insert  << ConvertParams(St2),
						Poco::Data::Keywords::use(DeviceDetails.SerialNumber),
						Poco::Data::Keywords::use(DeviceDetails.DeviceType),
						Poco::Data::Keywords::use(DeviceDetails.MACAddress),
						Poco::Data::Keywords::use(DeviceDetails.Manufacturer),
						Poco::Data::Keywords::use(DeviceDetails.Configuration),
						Poco::Data::Keywords::use(NotesString),
						Poco::Data::Keywords::use(DeviceDetails.Owner),
						Poco::Data::Keywords::use(DeviceDetails.Location),
						Poco::Data::Keywords::use(DeviceDetails.Firmware),
						Poco::Data::Keywords::use(DeviceDetails.Compatible),
						Poco::Data::Keywords::use(DeviceDetails.FWUpdatePolicy),
						Poco::Data::Keywords::use(DeviceDetails.UUID),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(DeviceDetails.Venue);
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

	bool Storage::CreateDefaultDevice(const std::string &SerialNumber, const std::string &Capabilities, std::string & Firmware, std::string &Compat) {

		GWObjects::Device D;
		Logger_.information(Poco::format("AUTO-CREATION(%s)", SerialNumber));
		uint64_t Now = time(nullptr);

		uCentral::Config::Capabilities Caps(Capabilities);
		GWObjects::DefaultConfiguration DefConfig;

		if (FindDefaultConfigurationForModel(Caps.Model(), DefConfig)) {
			uCentral::Config::Config NewConfig(DefConfig.Configuration);
			NewConfig.SetUUID(Now);
			D.Configuration = NewConfig.get();
		} else {
			uCentral::Config::Config NewConfig;
			NewConfig.SetUUID(Now);
			D.Configuration = NewConfig.get();
		}

		D.SerialNumber = Poco::toLower(SerialNumber);
		Compat = D.Compatible = Caps.Compatible();
		D.DeviceType = Daemon()->IdentifyDevice(D.Compatible);
		D.MACAddress = Utils::SerialToMAC(SerialNumber);
		D.Manufacturer = Caps.Model();
		D.Firmware = Firmware;
		D.UUID = Now;
		D.Notes = SecurityObjects::NoteInfoVec { SecurityObjects::NoteInfo{ (uint64_t)std::time(nullptr), "", "Auto-provisioned."}};
		D.CreationTimestamp = D.LastConfigurationDownload = D.LastConfigurationChange = Now;

		return CreateDevice(D);
	}

	bool Storage::SetLocation(std::string & SerialNumber, std::string & LocationUUID) {
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

	bool Storage::SetVenue(std::string & SerialNumber, std::string & VenueUUID) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);

			std::string St{"UPDATE Devices SET Venue=? WHERE SerialNumber=?"};

			Update  << ConvertParams(St) ,
				Poco::Data::Keywords::use(VenueUUID),
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

	bool Storage::GetDeviceFWUpdatePolicy(std::string &SerialNumber, std::string &Policy) {
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

	bool Storage::SetDevicePassword(std::string & SerialNumber, std::string & Password) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);
			std::string St{"UPDATE Devices SET DevicePassword=?  WHERE SerialNumber=?"};

			Update << ConvertParams(St) ,
				Poco::Data::Keywords::use(Password),
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

	bool Storage::SetOwner(std::string & SerialNumber, std::string & OwnerUUID) {
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

	bool Storage::SetConnectInfo(std::string &SerialNumber, std::string &Firmware) {
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

	bool Storage::DeleteDevice(std::string &SerialNumber) {
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

	bool Storage::GetDevice(std::string &SerialNumber, GWObjects::Device &DeviceDetails) {
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
						   "LastFWUpdate, "
						   "Venue, "
						   "DevicePassword "
						   "FROM Devices WHERE SerialNumber=?"};

			std::string NI;
			Select << ConvertParams(St),
				Poco::Data::Keywords::into(DeviceDetails.SerialNumber),
				Poco::Data::Keywords::into(DeviceDetails.DeviceType),
				Poco::Data::Keywords::into(DeviceDetails.MACAddress),
				Poco::Data::Keywords::into(DeviceDetails.Manufacturer),
				Poco::Data::Keywords::into(DeviceDetails.Configuration),
				Poco::Data::Keywords::into(NI),
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
				Poco::Data::Keywords::into(DeviceDetails.Venue),
				Poco::Data::Keywords::into(DeviceDetails.DevicePassword),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			DeviceDetails.Notes = RESTAPI_utils::to_object_array<SecurityObjects::NoteInfo>(NI);

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

	bool Storage::DeviceExists(std::string &SerialNumber) {
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

	bool Storage::GetDevicesWithoutFirmware(std::string &Compatible, std::string &Version, std::vector<std::string> &SerialNumbers) {
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

	bool Storage::UpdateDevice(GWObjects::Device &NewDeviceDetails) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);

			GWObjects::Device	ExistingDevice;
			if(!GetDevice(NewDeviceDetails.SerialNumber,ExistingDevice))
				return false;

			uint64_t Now = std::time(nullptr);
			if(!NewDeviceDetails.DeviceType.empty())
				ExistingDevice.DeviceType=NewDeviceDetails.DeviceType;
			if(!NewDeviceDetails.MACAddress.empty())
				ExistingDevice.MACAddress=NewDeviceDetails.MACAddress;
			if(!NewDeviceDetails.FWUpdatePolicy.empty())
				ExistingDevice.FWUpdatePolicy=NewDeviceDetails.FWUpdatePolicy;
			if(!NewDeviceDetails.DevicePassword.empty())
				ExistingDevice.DevicePassword=NewDeviceDetails.DevicePassword;
			if(!NewDeviceDetails.Notes.empty()) {
				for(const auto &i:NewDeviceDetails.Notes)
					ExistingDevice.Notes.push_back(i);
			}

			std::string NotesString = RESTAPI_utils::to_string(ExistingDevice.Notes);

			std::string St2{"UPDATE Devices SET "
							"DeviceType=?, "
							"MACAddress=?, "
							"Manufacturer=?, "
							"Notes=?, "
							"Owner=?, "
							"Location=?, "
							"FWUpdatePolicy=?,"
							"Venue=? "
							" WHERE SerialNumber=?"};
			auto NI = RESTAPI_utils::to_string(ExistingDevice.Notes);
			Update  << ConvertParams(St2),
				Poco::Data::Keywords::use(ExistingDevice.DeviceType),
				Poco::Data::Keywords::use(ExistingDevice.MACAddress),
				Poco::Data::Keywords::use(ExistingDevice.Manufacturer),
				Poco::Data::Keywords::use(NI),
				Poco::Data::Keywords::use(ExistingDevice.Owner),
				Poco::Data::Keywords::use(ExistingDevice.Location),
				Poco::Data::Keywords::use(ExistingDevice.FWUpdatePolicy),
				Poco::Data::Keywords::use(ExistingDevice.Venue),
				Poco::Data::Keywords::use(ExistingDevice.SerialNumber);
			Update.execute();
			GetDevice(ExistingDevice.SerialNumber,NewDeviceDetails);
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), NewDeviceDetails.SerialNumber,
										 E.displayText()));
		}

		return false;
	}

	bool Storage::GetDevices(uint64_t From, uint64_t HowMany, const std::string &Select, std::vector<GWObjects::Device> &Devices) {
		return false;
	}

	bool Storage::GetDevices(uint64_t From, uint64_t HowMany, std::vector<GWObjects::Device> &Devices) {

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
			uint64_t,
			std::string,
			std::string
		> DeviceRecord;
		typedef std::vector<DeviceRecord> RecordList;

		RecordList Records;

		std::cout << "Getting devices..." << std::endl;

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
					  "LastFWUpdate, "
					  "Venue, "
					  "DevicePassword "
					  "FROM Devices",
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::from(From),
				Poco::Data::Keywords::limit(HowMany,false);
			Select.execute();


			for (auto i: Records) {

				SecurityObjects::NoteInfoVec 	NI;
				NI = RESTAPI_utils::to_object_array<SecurityObjects::NoteInfo>(i.get<5>());

				GWObjects::Device R{
					.SerialNumber   = i.get<0>(),
					.DeviceType     = i.get<1>(),
					.MACAddress     = i.get<2>(),
					.Manufacturer   = i.get<3>(),
					.Configuration  = i.get<4>(),
					.Notes  		= NI,
					.Owner          = i.get<6>(),
					.Location 		= i.get<7>(),
					.Firmware 		= i.get<8>(),
					.Compatible 	= i.get<9>(),
					.FWUpdatePolicy = i.get<10>(),
					.UUID			= i.get<11>(),
					.CreationTimestamp = i.get<12>(),
					.LastConfigurationChange = i.get<13>(),
					.LastConfigurationDownload = i.get<14>(),
					.LastFWUpdate = i.get<15>(),
					.Venue = i.get<16>(),
					.DevicePassword = i.get<17>()};
				Devices.push_back(R);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t & NewUUID) {
		// std::lock_guard<std::mutex> guard(Mutex_);
		std::string SS;
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);
			uint64_t Now = time(nullptr);

			std::string St{"SELECT SerialNumber, UUID, Configuration FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(SS),
				Poco::Data::Keywords::into(NewUUID),
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

	bool Storage::SetDeviceCompatibility(std::string &SerialNumber, std::string &Compatible) {
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

	static std::string ComputeCertificateTag( GWObjects::CertificateValidation V) {
		switch(V) {
		case GWObjects::NO_CERTIFICATE: return "no certificate";
		case GWObjects::VALID_CERTIFICATE: return "non TIP certificate";
		case GWObjects::MISMATCH_SERIAL: return "serial mismatch";
		case GWObjects::VERIFIED: return "verified";
		}
		return "unknown";
	}

	static const uint64_t SECONDS_MONTH = 30*24*60*60;
	static const uint64_t SECONDS_WEEK = 7*24*60*60;
	static const uint64_t SECONDS_DAY = 1*24*60*60;
	static const uint64_t SECONDS_HOUR = 1*24*60*60;

	static std::string ComputeUpLastContactTag(uint64_t T1) {
		uint64_t T = T1 - std::time(nullptr);
		if( T>SECONDS_MONTH) return ">month";
		if( T>SECONDS_WEEK) return ">week";
		if( T>SECONDS_DAY) return ">day";
		if( T>SECONDS_HOUR) return ">hour";
		return "now";
	}

	static std::string ComputeSanityTag(uint64_t T) {
		if( T==100) return "100%";
		if( T>90) return ">90%";
		if( T>60) return ">60%";
		return "<60%";
	}

	static std::string ComputeUpTimeTag(uint64_t T) {
		if( T>SECONDS_MONTH) return ">month";
		if( T>SECONDS_WEEK) return ">week";
		if( T>SECONDS_DAY) return ">day";
		if( T>SECONDS_HOUR) return ">hour";
		return "now";
	}

	static std::string ComputeLoadTag(uint64_t T) {
		float V=100.0*((float)T/65536.0);
		if(V<5.0) return "< 5%";
		if(V<25.0) return "< 25%";
		if(V<50.0) return "< 50%";
		if(V<75.0) return "< 75%";
		return ">75%";
	}

	static std::string ComputeFreeMemoryTag(uint64_t Free, uint64_t Total) {
		float V = 100.0 * ((float)Free/(float(Total)));
		if(V<5.0) return "< 5%";
		if(V<25.0) return "< 25%";
		if(V<50.0) return "< 50%";
		if(V<75.0) return "< 75%";
		return ">75%";
	}

	int ChannelToBand(uint64_t C) {
		if(C>=1 && C<=16) return 2;
		return 5;
	}

	bool Storage::AnalyzeDevices(GWObjects::Dashboard &Dashboard) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select << "SELECT SerialNumber, Compatible, Firmware FROM Devices";
			Select.execute();

			Poco::Data::RecordSet   RSet(Select);

			bool More = RSet.moveFirst();
			while(More) {
				Dashboard.numberOfDevices++;
				auto SerialNumber = RSet[0].convert<std::string>();
				auto DeviceType = RSet[1].convert<std::string>();
				auto Revision = RSet[2].convert<std::string>();
				Types::UpdateCountedMap(Dashboard.vendors, OUIServer()->GetManufacturer(SerialNumber));
				Types::UpdateCountedMap(Dashboard.deviceType, DeviceType);

				GWObjects::ConnectionState	ConnState;
				if(DeviceRegistry()->GetState(SerialNumber, ConnState)) {
					Types::UpdateCountedMap(Dashboard.status, ConnState.Connected ? "connected" : "not connected");
					Types::UpdateCountedMap(Dashboard.certificates, ComputeCertificateTag(ConnState.VerifiedCertificate));
					Types::UpdateCountedMap(Dashboard.lastContact, ComputeUpLastContactTag(ConnState.LastContact));
					GWObjects::HealthCheck	HC;
					if(DeviceRegistry()->GetHealthcheck(SerialNumber,HC))
						Types::UpdateCountedMap(Dashboard.healths, ComputeSanityTag(HC.Sanity));
					std::string LastStats;
					if(DeviceRegistry()->GetStatistics(SerialNumber, LastStats) && !LastStats.empty()) {
						Poco::JSON::Parser	P;

						auto RawObject = P.parse(LastStats).extract<Poco::JSON::Object::Ptr>();

						if(RawObject->has("unit")) {
							auto Unit = RawObject->getObject("unit");
							if (Unit->has("uptime")) {
								Types::UpdateCountedMap(Dashboard.upTimes, ComputeUpTimeTag(Unit->get("uptime")));
							}
							if (Unit->has("memory")) {
								auto Memory = Unit->getObject("memory");
								uint64_t Free = Memory->get("free");
								uint64_t Total = Memory->get("total");
								Types::UpdateCountedMap(Dashboard.memoryUsed, ComputeFreeMemoryTag(Free, Total));
							}
							if (Unit->has("load")) {
								auto Load = Unit->getArray("load");
								Types::UpdateCountedMap(Dashboard.load1,
														ComputeLoadTag(Load->getElement<uint64_t>(0)));
								Types::UpdateCountedMap(Dashboard.load5,
														ComputeLoadTag(Load->getElement<uint64_t>(1)));
								Types::UpdateCountedMap(Dashboard.load15,
														ComputeLoadTag(Load->getElement<uint64_t>(2)));
							}
						}

						if(RawObject->isArray("radios") && RawObject->isArray("interfaces")) {
							auto RA = RawObject->getArray("radios");
							// map of phy to 2g/5g
							std::map<std::string,int>   RadioPHYs;
							//  parse radios and get the phy out with the band
							for(auto const &i:*RA) {
								Poco::JSON::Parser p2;
								auto RadioObj = i.extract<Poco::JSON::Object::Ptr>();
								if(RadioObj->has("phy") && RadioObj->has("channel")) {
									RadioPHYs[RadioObj->get("phy").toString()]= ChannelToBand(RadioObj->get("channel"));
								}
							}

							auto IA = RawObject->getArray("interfaces");
							for(auto const &i:*IA) {
								auto InterfaceObj = i.extract<Poco::JSON::Object::Ptr>();
								if(InterfaceObj->isArray("ssids")) {
									auto SSIDA = InterfaceObj->getArray("ssids");
									for(const auto &s:*SSIDA) {
										auto SSIDinfo = s.extract<Poco::JSON::Object::Ptr>();
										if(SSIDinfo->isArray("associations") && SSIDinfo->has("phy")) {
											auto PHY = SSIDinfo->get("phy").toString();
											int Radio = 2;
											auto Rit = RadioPHYs.find(PHY);
											if(Rit!=RadioPHYs.end())
												Radio = Rit->second;
											auto AssocA = SSIDinfo->getArray("associations");
											if(Radio==2)
												Types::UpdateCountedMap(Dashboard.associations, "2G", AssocA->size());
											else
												Types::UpdateCountedMap(Dashboard.associations, "5G", AssocA->size());
										}
									}
								}
							}
						}
					}
					Types::UpdateCountedMap(Dashboard.status, ConnState.Connected ? "connected" : "not connected");
				} else {
					Types::UpdateCountedMap(Dashboard.status, "not connected");
				}
				More = RSet.moveNext();
			}
			return true;
		} catch(const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}
}

