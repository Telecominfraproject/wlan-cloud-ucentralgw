//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "AP_WS_Server.h"
#include "CapabilitiesCache.h"
#include "CentralConfig.h"
#include "ConfigurationCache.h"
#include "Daemon.h"
#include "FindCountry.h"
#include "OUIServer.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Net/IPAddress.h"
#include "SDKcalls.h"
#include "SerialNumberCache.h"
#include "StateUtils.h"
#include "StorageService.h"

#include "framework/KafkaManager.h"
#include "framework/utils.h"

namespace OpenWifi {

	const static std::string DB_DeviceSelectFields{"SerialNumber,"
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
												   "DevicePassword, "
												   "subscriber, "
												   "entity, "
												   "modified, "
												   "locale,"
												   "restrictedDevice,"
												   "pendingConfiguration, "
												   "pendingConfigurationCmd, "
												   "restrictionDetails, "
												   "pendingUUID, "
												   "simulated,"
												   "lastRecordedContact,"
												   "certificateExpiryDate,"
												   "connectReason "
	};

	const static std::string DB_DeviceUpdateFields{"SerialNumber=?,"
												   "DeviceType=?, "
												   "MACAddress=?, "
												   "Manufacturer=?, "
												   "Configuration=?, "
												   "Notes=?, "
												   "Owner=?, "
												   "Location=?, "
												   "Firmware=?,"
												   "Compatible=?,"
												   "FWUpdatePolicy=?,"
												   "UUID=?,      "
												   "CreationTimestamp=?,   "
												   "LastConfigurationChange=?, "
												   "LastConfigurationDownload=?, "
												   "LastFWUpdate=?, "
												   "Venue=?,"
												   "DevicePassword=?, "
												   "subscriber=?, "
												   "entity=?, "
												   "modified=?, "
												   "locale=?, "
												   "restrictedDevice=?, "
												   "pendingConfiguration=?, "
												   "pendingConfigurationCmd=?, "
												   "restrictionDetails=?, "
												   "pendingUUID=?, "
												   "simulated=?,"
												   "lastRecordedContact=?, "
												   "certificateExpiryDate=?,"
												   "connectReason=? "
	};

	const static std::string DB_DeviceInsertValues{
		" VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) "};

	typedef Poco::Tuple<std::string, std::string, std::string, std::string, std::string,
						std::string, std::string, std::string, std::string, std::string,
						std::string, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, std::string,
						std::string, std::string, std::string, uint64_t, std::string, bool,
						std::string, std::string, std::string, std::uint64_t, bool, std::uint64_t,
						std::uint64_t, std::string>
		DeviceRecordTuple;
	typedef std::vector<DeviceRecordTuple> DeviceRecordList;

	void ConvertDeviceRecord(const DeviceRecordTuple &R, GWObjects::Device &D) {
		D.SerialNumber = R.get<0>();
		D.DeviceType = R.get<1>();
		D.MACAddress = R.get<2>();
		D.Manufacturer = R.get<3>();
		D.Configuration = R.get<4>();
		D.Notes = RESTAPI_utils::to_object_array<SecurityObjects::NoteInfo>(R.get<5>());
		D.Owner = R.get<6>();
		D.Location = R.get<7>();
		D.Firmware = R.get<8>();
		D.Compatible = R.get<9>();
		D.FWUpdatePolicy = R.get<10>();
		D.UUID = R.get<11>();
		D.CreationTimestamp = R.get<12>();
		D.LastConfigurationChange = R.get<13>();
		D.LastConfigurationDownload = R.get<14>();
		D.LastFWUpdate = R.get<15>();
		D.Venue = R.get<16>();
		D.DevicePassword = R.get<17>();
		D.subscriber = R.get<18>();
		D.entity = R.get<19>();
		D.modified = R.get<20>();
		D.locale = R.get<21>();
		D.restrictedDevice = R.get<22>();
		D.pendingConfiguration = R.get<23>();
		D.pendingConfigurationCmd = R.get<24>();
		D.restrictionDetails =
			RESTAPI_utils::to_object<OpenWifi::GWObjects::DeviceRestrictions>(R.get<25>());
		D.pendingUUID = R.get<26>();
		D.simulated = R.get<27>();
		D.lastRecordedContact = R.get<28>();
		D.certificateExpiryDate = R.get<29>();
		D.connectReason = R.get<30>();
	}

	void ConvertDeviceRecord(const GWObjects::Device &D, DeviceRecordTuple &R) {
		R.set<0>(D.SerialNumber);
		R.set<1>(D.DeviceType);
		R.set<2>(D.MACAddress);
		R.set<3>(D.Manufacturer);
		R.set<4>(D.Configuration);
		R.set<5>(RESTAPI_utils::to_string(D.Notes));
		R.set<6>(D.Owner);
		R.set<7>(D.Location);
		R.set<8>(D.Firmware);
		R.set<9>(D.Compatible);
		R.set<10>(D.FWUpdatePolicy);
		R.set<11>(D.UUID);
		R.set<12>(D.CreationTimestamp);
		R.set<13>(D.LastConfigurationChange);
		R.set<14>(D.LastConfigurationDownload);
		R.set<15>(D.LastFWUpdate);
		R.set<16>(D.Venue);
		R.set<17>(D.DevicePassword);
		R.set<18>(D.subscriber);
		R.set<19>(D.entity);
		R.set<20>(D.modified);
		R.set<21>(D.locale);
		R.set<22>(D.restrictedDevice);
		R.set<23>(D.pendingConfiguration);
		R.set<24>(D.pendingConfigurationCmd);
		R.set<25>(RESTAPI_utils::to_string(D.restrictionDetails));
		R.set<26>(D.pendingUUID);
		R.set<27>(D.simulated);
		R.set<28>(D.lastRecordedContact);
		R.set<29>(D.certificateExpiryDate);
		R.set<30>(D.connectReason);
	}

	bool Storage::GetDeviceCount(uint64_t &Count) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string st{"SELECT COUNT(*) FROM Devices"};

			Select << st, Poco::Data::Keywords::into(Count);
			Select.execute();
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany,
										 std::vector<std::string> &SerialNumbers,
										 const std::string &orderBy) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string st;
			if (orderBy.empty())
				st = "SELECT SerialNumber From Devices ORDER BY SerialNumber ASC ";
			else
				st = "SELECT SerialNumber From Devices " + orderBy;

			Select << st + ComputeRange(From, HowMany), Poco::Data::Keywords::into(SerialNumbers);
			Select.execute();
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

/*	bool Storage::UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration,
											uint64_t &NewUUID) {
		try {

			Config::Config Cfg(Configuration);
			if (!Cfg.Valid()) {
				poco_warning(Logger(), fmt::format("CONFIG-UPDATE({}): Configuration was not valid",
												   SerialNumber));
				return false;
			}

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			GWObjects::Device D;
			if (!GetDevice(SerialNumber, D))
				return false;

			uint64_t Now = time(nullptr);
			D.UUID = NewUUID = D.LastConfigurationChange =
				(D.LastConfigurationChange == Now ? Now + 1 : Now);
			if (Cfg.SetUUID(NewUUID)) {
				Poco::Data::Statement Update(Sess);
				D.Configuration = Cfg.get();
				SetCurrentConfigurationID(SerialNumber, NewUUID);

				DeviceRecordTuple R;
				ConvertDeviceRecord(D, R);
				std::string St2{"UPDATE Devices SET " + DB_DeviceUpdateFields +
								" WHERE SerialNumber=?"};
				Update << ConvertParams(St2), Poco::Data::Keywords::use(R),
					Poco::Data::Keywords::use(SerialNumber);
				Update.execute();
				poco_information(Logger(),
								 fmt::format("DEVICE-CONFIGURATION-UPDATED({}): New UUID is {}",
											 SerialNumber, NewUUID));
				Configuration = D.Configuration;
				return true;
			}
			return false;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}
*/
	bool Storage::RollbackDeviceConfigurationChange(std::string & SerialNumber) {
		try {
			GWObjects::Device D;
			if (!GetDevice(SerialNumber, D))
				return false;
			D.pendingConfiguration.clear();
			D.pendingUUID = 0;
			D.LastConfigurationChange = Utils::Now();

			ConfigurationCache().Add(Utils::SerialNumberToInt(SerialNumber), D.UUID);

			Poco::Data::Session Sess = Pool_->get();
			Sess.begin();
			Poco::Data::Statement Update(Sess);

			DeviceRecordTuple R;
			ConvertDeviceRecord(D, R);
			std::string St2{"UPDATE Devices SET " + DB_DeviceUpdateFields +
							" WHERE SerialNumber=?"};
			Update << ConvertParams(St2), Poco::Data::Keywords::use(R),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();
			Sess.commit();
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::CompleteDeviceConfigurationChange(std::string & SerialNumber) {
		try {
			auto Session = Pool_->get();
			return CompleteDeviceConfigurationChange(Session, SerialNumber);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::CompleteDeviceConfigurationChange(Poco::Data::Session & Session, std::string & SerialNumber) {
		try {
			GWObjects::Device D;
			if (!GetDevice(SerialNumber, D))
				return false;

			if(D.pendingConfiguration.empty())
				return true;
			D.Configuration = D.pendingConfiguration;
			D.pendingConfiguration.clear();
			D.UUID = D.pendingUUID;
			D.pendingUUID = 0;
			D.LastConfigurationChange = Utils::Now();

			ConfigurationCache().Add(Utils::SerialNumberToInt(SerialNumber), D.UUID);

			Session.begin();
			Poco::Data::Statement Update(Session);

			DeviceRecordTuple R;
			ConvertDeviceRecord(D, R);
			std::string St2{"UPDATE Devices SET " + DB_DeviceUpdateFields +
							" WHERE SerialNumber=?"};
			Update << ConvertParams(St2), Poco::Data::Keywords::use(R),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();
			Session.commit();
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::SetPendingDeviceConfiguration(std::string &SerialNumber, std::string &Configuration,
									   uint64_t &NewUUID) {
		try {

			Config::Config Cfg(Configuration);
			if (!Cfg.Valid()) {
				poco_warning(Logger(), fmt::format("CONFIG-UPDATE({}): Configuration was not valid",
												   SerialNumber));
				return false;
			}


			GWObjects::Device D;
			if (!GetDevice(SerialNumber, D))
				return false;


			uint64_t Now = time(nullptr);
			if(NewUUID==0) {
				D.pendingUUID = NewUUID = (D.LastConfigurationChange == Now ? Now + 1 : Now);
			} else {
				D.pendingUUID = NewUUID;
			}

			if (Cfg.SetUUID(NewUUID)) {
				Poco::Data::Session Sess = Pool_->get();
				Sess.begin();
				Poco::Data::Statement Update(Sess);
				D.pendingConfiguration = Cfg.get();

				DeviceRecordTuple R;
				ConvertDeviceRecord(D, R);
				std::string St2{"UPDATE Devices SET " + DB_DeviceUpdateFields +
								" WHERE SerialNumber=?"};
				Update << ConvertParams(St2), Poco::Data::Keywords::use(R),
					Poco::Data::Keywords::use(SerialNumber);
				Update.execute();
				Sess.commit();
				poco_information(Logger(),
								 fmt::format("DEVICE-PENDING-CONFIGURATION-UPDATED({}): New UUID is {}",
											 SerialNumber, NewUUID));
				Configuration = D.pendingConfiguration;
				return true;
			}
			return false;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::SetDeviceLastRecordedContact(LockedDbSession &Session, std::string &SerialNumber, std::uint64_t lastRecordedContact) {
		try {
			std::lock_guard		Lock(Session.Mutex());
			return SetDeviceLastRecordedContact(Session.Session(), SerialNumber, lastRecordedContact);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::SetDeviceLastRecordedContact(Poco::Data::Session &Session, std::string &SerialNumber, std::uint64_t lastRecordedContact) {
		try {
			Session.begin();
			Poco::Data::Statement 	Update(Session);
			std::string St{"UPDATE Devices SET lastRecordedContact=?  WHERE SerialNumber=?"};

			Update << ConvertParams(St), Poco::Data::Keywords::use(lastRecordedContact),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();
			Session.commit();
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::SetDeviceLastRecordedContact(std::string &SerialNumber, std::uint64_t lastRecordedContact) {
		try {
			auto Session = Pool_->get();
			return SetDeviceLastRecordedContact(Session, SerialNumber, lastRecordedContact);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::CreateDevice(Poco::Data::Session &Sess, GWObjects::Device &DeviceDetails) {
		std::string SerialNumber;
		try {
			Config::Config Cfg(DeviceDetails.Configuration);
			uint64_t Now = Utils::Now();

			DeviceDetails.modified = Utils::Now();
			DeviceDetails.CreationTimestamp = DeviceDetails.LastConfigurationDownload =
				DeviceDetails.UUID = DeviceDetails.LastConfigurationChange = Now;

			if (Cfg.Valid() && Cfg.SetUUID(DeviceDetails.UUID)) {

				DeviceDetails.Configuration = Cfg.get();
				Sess.begin();
				Poco::Data::Statement Insert(Sess);

				std::string St2{"INSERT INTO Devices ( " + DB_DeviceSelectFields + " ) " +
								DB_DeviceInsertValues + " ON CONFLICT (SerialNumber) DO NOTHING"};

				SetCurrentConfigurationID(DeviceDetails.SerialNumber, DeviceDetails.UUID);
				DeviceRecordTuple R;
				ConvertDeviceRecord(DeviceDetails, R);
				Insert << ConvertParams(St2), Poco::Data::Keywords::use(R);
				Insert.execute();
				Sess.commit();
				SetCurrentConfigurationID(DeviceDetails.SerialNumber, DeviceDetails.UUID);
				SerialNumberCache()->AddSerialNumber(DeviceDetails.SerialNumber);
			} else {
				poco_warning(Logger(), "Cannot create device: invalid configuration.");
				return false;
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::CreateDevice(LockedDbSession &Session, GWObjects::Device &DeviceDetails) {
		try {
			std::lock_guard	Lock(Session.Mutex());
			return CreateDevice(Session.Session(), DeviceDetails);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

		bool Storage::CreateDevice(GWObjects::Device &DeviceDetails) {
		try {
			auto Session = Pool_->get();
			return CreateDevice(Session, DeviceDetails);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	static std::string InsertRadiosCountyRegulation(std::string &Config,
													const Poco::Net::IPAddress &IPAddress) {
		std::string FoundCountry;
		try {
			auto C = nlohmann::json::parse(Config);
			if (C.contains("radios") && C["radios"].is_array()) {
				auto Radios = C["radios"];
				for (auto &i : Radios) {
					if (i.contains("country") && !i["country"].empty() && i["country"] != "**")
						continue;
					if (FoundCountry.empty())
						FoundCountry = FindCountryFromIP()->Get(IPAddress);
					i["country"] = FoundCountry;
				}
				C["radios"] = Radios;
				Config = to_string(C);
			}
		} catch (...) {
		}
		return FoundCountry;
	}

	bool Storage::DeleteSimulatedDevice([[maybe_unused]] const std::string &SerialNumber) {

		std::vector<std::string> Statements =
		{
			"delete from commandlist using devices where commandlist.serialnumber=devices.serialnumber and devices.simulated=true;",
			"delete from healthchecks using devices where healthchecks.serialnumber=devices.serialnumber and devices.simulated=true;",
			"delete from statistics using devices where statistics.serialnumber=devices.serialnumber and devices.simulated=true;",
			"delete from devicelogs using devices where devicelogs.serialnumber=devices.serialnumber and devices.simulated=true;",
			"delete from capabilities using devices where capabilities.serialnumber=devices.serialnumber and devices.simulated=true;",
			"delete from devices where devices.simulated=true;"
		};
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Command(Sess);

			for (const auto &i : Statements) {
				try {
					Command << i, Poco::Data::Keywords::now;
				} catch (const Poco::Exception &E) {
					Logger().log(E);
				}
				Command.reset(Sess);
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return true;
	}

	bool Storage::CreateDefaultDevice(Poco::Data::Session &Session, std::string &SerialNumber, const Config::Capabilities &Caps,
									  std::string &Firmware,
									  const Poco::Net::IPAddress &IPAddress,
									  bool simulated) {

		GWObjects::Device D;

		// poco_information(Logger(), fmt::format("AUTO-CREATION({}): Start.", SerialNumber));
		uint64_t Now = Utils::Now();
		GWObjects::DefaultConfiguration DefConfig;

		if (!Caps.Platform().empty() && !Caps.Compatible().empty()) {
			CapabilitiesCache()->Add(Caps);
		}

		bool Found = false;
		std::string FoundConfig;
		if (AP_WS_Server()->UseProvisioning()) {
			if (SDKCalls::GetProvisioningConfiguration(SerialNumber, FoundConfig)) {
				if (FoundConfig != "none") {
					Found = true;
					Config::Config NewConfig(FoundConfig);
					NewConfig.SetUUID(Now);
					D.Configuration = NewConfig.get();
				}
			}
		}

		if (!Found && AP_WS_Server()->UseDefaults() &&
			FindDefaultConfigurationForModel(Caps.Compatible(), DefConfig)) {
			Config::Config NewConfig(DefConfig.Configuration);
			NewConfig.SetUUID(Now);
			D.Configuration = NewConfig.get();
		} else if (!Found) {
			Config::Config NewConfig;
			NewConfig.SetUUID(Now);
			D.Configuration = NewConfig.get();
		}

		//	We need to insert the country code according to the IP in the radios section...
		D.locale = InsertRadiosCountyRegulation(D.Configuration, IPAddress);
		D.SerialNumber = Poco::toLower(SerialNumber);
		D.Compatible = Caps.Compatible();
		D.DeviceType = Daemon()->IdentifyDevice(D.Compatible);
		D.MACAddress = Utils::SerialToMAC(SerialNumber);
		D.Manufacturer = Caps.Model();
		D.Firmware = Firmware;
		D.simulated = simulated;
		D.Notes = SecurityObjects::NoteInfoVec{
			SecurityObjects::NoteInfo{(uint64_t)Utils::Now(), "", "Auto-provisioned."}};

		CreateDeviceCapabilities(Session, SerialNumber, Caps);
		auto Result = CreateDevice(Session, D);
		poco_information(Logger(), fmt::format("AUTO-CREATION({}): Done, Result={}", SerialNumber, Result));
		return Result;
	}

/*	bool Storage::GetDeviceFWUpdatePolicy(std::string &SerialNumber, std::string &Policy) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string St{"SELECT FWUpdatePolicy FROM Devices WHERE SerialNumber=?"};
			Select << ConvertParams(St), Poco::Data::Keywords::into(Policy),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}
*/
	bool Storage::SetDevicePassword(LockedDbSession &Sess, std::string &SerialNumber, std::string &Password) {
		try {
			std::lock_guard		Lock(Sess.Mutex());
			Sess.Session().begin();

			Poco::Data::Statement Update(Sess.Session());
			std::string St{"UPDATE Devices SET DevicePassword=?  WHERE SerialNumber=?"};

			Update << ConvertParams(St), Poco::Data::Keywords::use(Password),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();
			Sess.Session().commit();
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::DeleteDevice(std::string &SerialNumber) {
		try {
			std::vector<std::string> TableNames{"Devices",		"Statistics",	"CommandList",
											"HealthChecks", "Capabilities", "DeviceLogs"};

			for (const auto &tableName : TableNames) {

				Poco::Data::Session Sess = Pool_->get();
				Sess.begin();
				Poco::Data::Statement Delete(Sess);

				std::string St = fmt::format("DELETE FROM {} WHERE SerialNumber='{}'", tableName, SerialNumber);
				try {
					Delete << St;
					Delete.execute();
					Sess.commit();
				} catch (...) {
				}
			}

			SerialNumberCache()->DeleteSerialNumber(SerialNumber);

			if (KafkaManager()->Enabled()) {
				Poco::JSON::Object Message;
				Message.set("command", "device_deleted");
				Message.set("timestamp", Utils::Now());
				KafkaManager()->PostMessage(KafkaTopics::COMMAND, SerialNumber, Message);
			}

			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	static void DeleteDeviceList(std::vector<std::string> &SerialNumbers, Poco::Logger &Logger) {
		for (auto &serialNumber:SerialNumbers) {
			poco_information(Logger,fmt::format("BATCH-DEVICE_DELETE: deleting {}", serialNumber));
			StorageService()->DeleteDevice(serialNumber);
		}
	}

	bool Storage::DeleteDevices(std::string &SerialPattern, bool SimulatedOnly) {
		try {
			std::vector<std::string>	SerialNumbers;
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement GetSerialNumbers(Sess);

			std::string SelectStatement = SimulatedOnly ?
					fmt::format("SELECT SerialNumber FROM Devices WHERE simulated and SerialNumber LIKE '{}' limit 10000",SerialPattern) :
					fmt::format("SELECT SerialNumber FROM Devices WHERE SerialNumber LIKE '{}' limit 10000", SerialPattern);

			GetSerialNumbers << SelectStatement,
				Poco::Data::Keywords::into(SerialNumbers);
			GetSerialNumbers.execute();

			poco_information(Logger(),fmt::format("BATCH-DEVICE_DELETE: Found {} devices that match the criteria {} to delete.", SerialNumbers.size(), SerialPattern));
			DeleteDeviceList(SerialNumbers, Logger());
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::DeleteDevices(std::uint64_t OlderContact, bool SimulatedOnly) {
		try {
			std::vector<std::string>	SerialNumbers;
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement GetSerialNumbers(Sess);

			std::string SelectStatement = SimulatedOnly ?
														fmt::format("SELECT SerialNumber FROM Devices WHERE simulated and lastRecordedContact!=0 and lastRecordedContact<{} limit 10000",OlderContact) :
														fmt::format("SELECT SerialNumber FROM Devices lastRecordedContact>0 and lastRecordedContact<{} limit 10000",OlderContact);
			GetSerialNumbers << SelectStatement,
				Poco::Data::Keywords::into(SerialNumbers);
			GetSerialNumbers.execute();

			poco_information(Logger(),fmt::format("BATCH-DEVICE_DELETE: Found {} devices that match with lastRecordedContact older than {} to delete.", SerialNumbers.size(), OlderContact));
			DeleteDeviceList(SerialNumbers, Logger());
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetDevice(Poco::Data::Session &Session, std::string &SerialNumber, GWObjects::Device &DeviceDetails) {
		try {
			Poco::Data::Statement Select(Session);
			std::string St{"SELECT " + DB_DeviceSelectFields +
						   " FROM Devices WHERE SerialNumber=?"};

			DeviceRecordTuple R;
			Select << ConvertParams(St), Poco::Data::Keywords::into(R),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if (Select.rowsExtracted() == 0)
				return false;
			ConvertDeviceRecord(R, DeviceDetails);
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetDevice(std::string &SerialNumber, GWObjects::Device &DeviceDetails) {
		try {
			auto Sess = Pool_->get();
			return GetDevice(Sess, SerialNumber, DeviceDetails);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetDevice(LockedDbSession &Session, std::string &SerialNumber, GWObjects::Device &DeviceDetails) {
		try {
			std::lock_guard		Lock(Session.Mutex());
			return GetDevice(Session.Session(), SerialNumber, DeviceDetails);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::DeviceExists(std::string &SerialNumber) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string Serial;

			std::string St{"SELECT SerialNumber FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St), Poco::Data::Keywords::into(Serial),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if (Select.rowsExtracted() == 0)
				return false;

			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::UpdateDevice(GWObjects::Device &NewDeviceDetails) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			return UpdateDevice(Sess, NewDeviceDetails);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::UpdateDevice(LockedDbSession &Session, GWObjects::Device &NewDeviceDetails) {
		try {
			std::lock_guard Lock(Session.Mutex());
			return UpdateDevice(Session.Session(), NewDeviceDetails);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::UpdateDevice(Poco::Data::Session &Sess, GWObjects::Device &NewDeviceDetails) {
		try {
			Sess.begin();
			Poco::Data::Statement Update(Sess);

			DeviceRecordTuple R;

			NewDeviceDetails.modified = Utils::Now();
			ConvertDeviceRecord(NewDeviceDetails, R);
			// NewDeviceDetails.LastConfigurationChange = Utils::Now();
			std::string St2{"UPDATE Devices SET " + DB_DeviceUpdateFields +
							" WHERE SerialNumber=?"};
			Update << ConvertParams(St2), Poco::Data::Keywords::use(R),
				Poco::Data::Keywords::use(NewDeviceDetails.SerialNumber);
			Update.execute();
			Sess.commit();
			// GetDevice(NewDeviceDetails.SerialNumber,NewDeviceDetails);
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetDevices(uint64_t From, uint64_t HowMany,
							 std::vector<GWObjects::Device> &Devices, const std::string &orderBy) {
		DeviceRecordList Records;
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			// std::string st{"SELECT " + DB_DeviceSelectFields + " FROM Devices " + orderBy.empty()
			// ? " ORDER BY SerialNumber ASC " + ComputeRange(From, HowMany)};
			std::string st = fmt::format("SELECT {} FROM Devices {} {}", DB_DeviceSelectFields,
										 orderBy.empty() ? " ORDER BY SerialNumber ASC " : orderBy,
										 ComputeRange(From, HowMany));

			Select << ConvertParams(st), Poco::Data::Keywords::into(Records);
			Select.execute();

			for (auto &i : Records) {
				GWObjects::Device D;
				ConvertDeviceRecord(i, D);
				Devices.push_back(D);
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::ExistingConfiguration(std::string &SerialNumber,
										[[maybe_unused]] uint64_t CurrentConfig,
										std::string &NewConfig, uint64_t &NewUUID) {
		std::string SS;
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			uint64_t Now = time(nullptr);

			std::string St{
				"SELECT SerialNumber, UUID, Configuration FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St), Poco::Data::Keywords::into(SS),
				Poco::Data::Keywords::into(NewUUID), Poco::Data::Keywords::into(NewConfig),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if (SS.empty()) {
				//	No configuration exists, so we should
				return false;
			}

			//  Let's update the last downloaded time
			Poco::Data::Statement Update(Sess);
			std::string St2{"UPDATE Devices SET LastConfigurationDownload=?  WHERE SerialNumber=?"};

			Update << ConvertParams(St2), Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::UpdateSerialNumberCache() {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			Select << "SELECT SerialNumber FROM Devices";
			Select.execute();

			Poco::Data::RecordSet RSet(Select);

			uint64_t NumberOfDevices = 0;

			bool More = RSet.moveFirst();
			while (More) {
				auto SerialNumber = RSet[0].convert<std::string>();
				SerialNumberCache()->AddSerialNumber(SerialNumber);
				NumberOfDevices++;
				More = RSet.moveNext();
			}
			Logger().information(fmt::format("Added {} serial numbers to cache.", NumberOfDevices));
			return true;

		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	static std::string ComputeCertificateTag(GWObjects::CertificateValidation V) {
		switch (V) {
		case GWObjects::NO_CERTIFICATE:
			return "no certificate";
		case GWObjects::VALID_CERTIFICATE:
			return "non TIP certificate";
		case GWObjects::MISMATCH_SERIAL:
			return "serial mismatch";
		case GWObjects::VERIFIED:
			return "verified";
		case GWObjects::SIMULATED:
			return "simulated";
		}
		return "unknown";
	}

	static const uint64_t SECONDS_MONTH = 30 * 24 * 60 * 60;
	static const uint64_t SECONDS_WEEK = 7 * 24 * 60 * 60;
	static const uint64_t SECONDS_DAY = 1 * 24 * 60 * 60;
	static const uint64_t SECONDS_HOUR = 60 * 60;

	static std::string ComputeUpLastContactTag(uint64_t T1) {
		uint64_t T = T1 - Utils::Now();
		if (T > SECONDS_MONTH)
			return ">month";
		if (T > SECONDS_WEEK)
			return ">week";
		if (T > SECONDS_DAY)
			return ">day";
		if (T > SECONDS_HOUR)
			return ">hour";
		return "now";
	}

	static std::string ComputeSanityTag(uint64_t T) {
		if (T == 100)
			return "100%";
		if (T > 90)
			return ">90%";
		if (T > 60)
			return ">60%";
		return "<60%";
	}

	static std::string ComputeUpTimeTag(uint64_t T) {
		if (T > SECONDS_MONTH)
			return ">month";
		if (T > SECONDS_WEEK)
			return ">week";
		if (T > SECONDS_DAY)
			return ">day";
		if (T > SECONDS_HOUR)
			return ">hour";
		return "now";
	}

	static std::string ComputeLoadTag(uint64_t T) {
		auto V = 100.0 * ((float)T / 65536.0);
		if (V < 5.0)
			return "< 5%";
		if (V < 25.0)
			return "< 25%";
		if (V < 50.0)
			return "< 50%";
		if (V < 75.0)
			return "< 75%";
		return ">75%";
	}

	static std::string ComputeUsedMemoryTag(uint64_t Free, uint64_t Total) {
		if (Total == 0)
			return "< 5%";
		auto V = 100.0 * ((float)(Total - Free) / (float(Total)));
		if (V < 5.0)
			return "< 5%";
		if (V < 25.0)
			return "< 25%";
		if (V < 50.0)
			return "< 50%";
		if (V < 75.0)
			return "< 75%";
		return ">75%";
	}

	bool Storage::AnalyzeDevices(GWObjects::Dashboard &Dashboard) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			Select << "SELECT SerialNumber, Compatible, Firmware FROM Devices";
			Select.execute();

			Poco::Data::RecordSet RSet(Select);

			bool More = RSet.moveFirst();
			while (More) {
				Dashboard.numberOfDevices++;
				auto SerialNumber = RSet[0].convert<std::string>();
				auto DeviceType = RSet[1].convert<std::string>();
				auto Revision = RSet[2].convert<std::string>();
				UpdateCountedMap(Dashboard.vendors, OUIServer()->GetManufacturer(SerialNumber));
				UpdateCountedMap(Dashboard.deviceType, DeviceType);

				GWObjects::ConnectionState ConnState;
				if (AP_WS_Server()->GetState(SerialNumber, ConnState)) {
					UpdateCountedMap(Dashboard.status,
									 ConnState.Connected ? "connected" : "not connected");
					UpdateCountedMap(Dashboard.certificates,
									 ComputeCertificateTag(ConnState.VerifiedCertificate));
					UpdateCountedMap(Dashboard.lastContact,
									 ComputeUpLastContactTag(ConnState.LastContact));
					GWObjects::HealthCheck HC;
					if (AP_WS_Server()->GetHealthcheck(SerialNumber, HC))
						UpdateCountedMap(Dashboard.healths, ComputeSanityTag(HC.Sanity));
					else
						UpdateCountedMap(Dashboard.healths, ComputeSanityTag(100));
					std::string LastStats;
					if (AP_WS_Server()->GetStatistics(SerialNumber, LastStats) &&
						!LastStats.empty()) {
						Poco::JSON::Parser P;

						auto RawObject = P.parse(LastStats).extract<Poco::JSON::Object::Ptr>();

						if (RawObject->has("unit")) {
							auto Unit = RawObject->getObject("unit");
							if (Unit->has("uptime")) {
								UpdateCountedMap(Dashboard.upTimes,
												 ComputeUpTimeTag(Unit->get("uptime")));
							}
							if (Unit->has("memory")) {
								auto Memory = Unit->getObject("memory");
								uint64_t Free = Memory->get("free");
								uint64_t Total = Memory->get("total");
								UpdateCountedMap(Dashboard.memoryUsed,
												 ComputeUsedMemoryTag(Free, Total));
							}
							if (Unit->has("load")) {
								auto Load = Unit->getArray("load");
								UpdateCountedMap(Dashboard.load1,
												 ComputeLoadTag(Load->getElement<uint64_t>(0)));
								UpdateCountedMap(Dashboard.load5,
												 ComputeLoadTag(Load->getElement<uint64_t>(1)));
								UpdateCountedMap(Dashboard.load15,
												 ComputeLoadTag(Load->getElement<uint64_t>(2)));
							}
						}

						uint64_t Associations_2G, Associations_5G, Associations_6G, uptime;
						StateUtils::ComputeAssociations(RawObject, Associations_2G, Associations_5G,
														Associations_6G, uptime);
						UpdateCountedMap(Dashboard.associations, "2G", Associations_2G);
						UpdateCountedMap(Dashboard.associations, "5G", Associations_5G);
						UpdateCountedMap(Dashboard.associations, "6G", Associations_6G);
					}
				} else {
					UpdateCountedMap(Dashboard.status, "not connected");
				}
				More = RSet.moveNext();
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	void Storage::GetDeviceDbFieldList(Types::StringVec &FieldList) {
		const auto fields =
			Poco::StringTokenizer(DB_DeviceSelectFields, ",", Poco::StringTokenizer::TOK_TRIM);
		for (const auto &field : fields)
			FieldList.push_back(field);
	}

} // namespace OpenWifi
