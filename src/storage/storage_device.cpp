//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "CentralConfig.h"
#include "ConfigurationCache.h"
#include "Daemon.h"
#include "DeviceRegistry.h"
#include "OUIServer.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Net/IPAddress.h"
#include "SerialNumberCache.h"
#include "StorageService.h"
#include "framework/MicroService.h"
#include "CapabilitiesCache.h"
#include "FindCountry.h"
#include "WS_Server.h"
#include "SDKcalls.h"
#include "StateUtils.h"

namespace OpenWifi {

	const static std::string DB_DeviceSelectFields{
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
		"DevicePassword, "
		"subscriber, "
		"entity, "
		"modified "
	};

	const static std::string DB_DeviceUpdateFields{
		"SerialNumber=?,"
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
		"modified=? "
	};

	const static std::string DB_DeviceInsertValues{" VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) "};

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
			std::string,
			std::string,
			std::string,
			uint64_t
		> DeviceRecordTuple;
		typedef std::vector<DeviceRecordTuple> DeviceRecordList;

	void ConvertDeviceRecord(const DeviceRecordTuple & R, GWObjects::Device &D) {
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
	}

	void ConvertDeviceRecord(const GWObjects::Device &D, DeviceRecordTuple & R) {
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
	}

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
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> &SerialNumbers) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string st{"SELECT SerialNumber From Devices ORDER BY SerialNumber ASC " };

			Select << 	st + ComputeRange(From, HowMany),
				Poco::Data::Keywords::into(SerialNumbers);
			Select.execute();
			return true;
		} catch (const Poco::Exception &E ) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t &NewUUID) {
		try {

			Config::Config Cfg(Configuration);
			if (!Cfg.Valid()) {
				poco_warning(Logger(),Poco::format("CONFIG-UPDATE(%s): Configuration was not valid", SerialNumber));
				return false;
			}

			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			GWObjects::Device	D;
			if(!GetDevice(SerialNumber,D))
				return false;

			uint64_t Now = time(nullptr);
			D.UUID = NewUUID = D.LastConfigurationChange = (D.LastConfigurationChange==Now ? Now + 1 : Now);
			if (Cfg.SetUUID(NewUUID)) {
				Poco::Data::Statement   Update(Sess);
				D.Configuration = Cfg.get();
				SetCurrentConfigurationID(SerialNumber, NewUUID);

				DeviceRecordTuple R;
				ConvertDeviceRecord(D,R);
				std::string St2{"UPDATE Devices SET " + DB_DeviceUpdateFields + " WHERE SerialNumber=?"};
				Update  << ConvertParams(St2),
					Poco::Data::Keywords::use(R),
					Poco::Data::Keywords::use(SerialNumber);
				Update.execute();
				poco_information(Logger(),Poco::format("DEVICE-CONFIGURATION-UPDATED(%s): New UUID is %Lu", SerialNumber, NewUUID));
				Configuration = D.Configuration;
				return true;
			}
			return false;
		}
		catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::CreateDevice(GWObjects::Device &DeviceDetails) {
		std::string SerialNumber;
		try {

			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT SerialNumber FROM Devices WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(SerialNumber),
				Poco::Data::Keywords::use(DeviceDetails.SerialNumber);
			Select.execute();

			if (Select.rowsExtracted()==0) {
				Config::Config Cfg(DeviceDetails.Configuration);
				uint64_t Now = std::time(nullptr);

				DeviceDetails.modified = OpenWifi::Now();
				DeviceDetails.CreationTimestamp = DeviceDetails.LastConfigurationDownload =
				DeviceDetails.UUID = DeviceDetails.LastConfigurationChange = Now;

				if (Cfg.Valid() && Cfg.SetUUID(DeviceDetails.UUID)) {

					DeviceDetails.Configuration = Cfg.get();
					Poco::Data::Statement   Insert(Sess);

					std::string St2{"INSERT INTO Devices ( " +
										DB_DeviceSelectFields + " ) " +
										DB_DeviceInsertValues };

					SetCurrentConfigurationID(DeviceDetails.SerialNumber, DeviceDetails.UUID);
					DeviceRecordTuple R;
					ConvertDeviceRecord(DeviceDetails, R);
					Insert  << ConvertParams(St2),
						Poco::Data::Keywords::use(R);
					Insert.execute();
					SetCurrentConfigurationID(DeviceDetails.SerialNumber, DeviceDetails.UUID);
					SerialNumberCache()->AddSerialNumber(DeviceDetails.SerialNumber);
					return true;
				} else {
					poco_warning(Logger(),"Cannot create device: invalid configuration.");
					return false;
				}
			} else {
				poco_warning(Logger(),Poco::format("Device %s already exists.", SerialNumber));
				return false;
			}

		}
		catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	static void InsertRadiosCountyRegulation(std::string &Config, const Poco::Net::IPAddress & IPAddress) {
		try {
			auto C = nlohmann::json::parse(Config);
			if(C.contains("radios") && C["radios"].is_array()) {
				auto Radios = C["radios"];
				std::string FoundCountry;
				for(auto &i:Radios) {
					if(i.contains("country") && !i["country"].empty() && i["country"]!="**")
						continue;
					if(FoundCountry.empty())
						FoundCountry = FindCountryFromIP()->Get(IPAddress);
					i["country"] = FoundCountry;
				}
				C["radios"] = Radios;
				Config = to_string(C);
			}
		} catch (...) {

		}
	}

// #define 	__DBGLOG__ Logger().information(Poco::format("-->%u", (uint32_t) __LINE__));

	bool Storage::CreateDefaultDevice(std::string &SerialNumber, std::string &Capabilities, std::string & Firmware, std::string &Compat, const Poco::Net::IPAddress & IPAddress) {

		GWObjects::Device D;
		poco_information(Logger(),Poco::format("AUTO-CREATION(%s)", SerialNumber));
		uint64_t Now = time(nullptr);
		Config::Capabilities 			Caps(Capabilities);
		GWObjects::DefaultConfiguration DefConfig;

		if(!Caps.Platform().empty() && !Caps.Compatible().empty()) {
			CapabilitiesCache::instance()->Add(Caps.Compatible(), Caps.Platform(), Capabilities);
		}

		bool 			Found = false;
		std::string 	FoundConfig;
		if(WebSocketServer()->UseProvisioning()) {
			if(SDKCalls::GetProvisioningConfiguration(SerialNumber, FoundConfig)) {
				if(FoundConfig != "none") {
					Found = true;
					Config::Config NewConfig(FoundConfig);
					NewConfig.SetUUID(Now);
					D.Configuration = NewConfig.get();
				}
			}
		}

		if (!Found && WebSocketServer()->UseDefaults() && FindDefaultConfigurationForModel(Compat, DefConfig)) {
			Config::Config NewConfig(DefConfig.Configuration);
			NewConfig.SetUUID(Now);
			D.Configuration = NewConfig.get();
		} else if(!Found) {
			Config::Config NewConfig;
			NewConfig.SetUUID(Now);
			D.Configuration = NewConfig.get();
		}

		//	We need to insert the country code according to the IP in the radios section...
		InsertRadiosCountyRegulation(D.Configuration, IPAddress);

		D.SerialNumber = Poco::toLower(SerialNumber);
		Compat = D.Compatible = Caps.Compatible();
		D.DeviceType = Daemon()->IdentifyDevice(D.Compatible);
		D.MACAddress = Utils::SerialToMAC(SerialNumber);
		D.Manufacturer = Caps.Model();
		D.Firmware = Firmware;
		D.Notes = SecurityObjects::NoteInfoVec { SecurityObjects::NoteInfo{ (uint64_t)std::time(nullptr), "", "Auto-provisioned."}};

		CreateDeviceCapabilities(SerialNumber, Capabilities);

		return CreateDevice(D);
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
			Logger().log(E);
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
			Logger().log(E);
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
				uint64_t 	Now = std::time(nullptr);

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
			Logger().log(E);
		}
		return false;
	}

	bool Storage::DeleteDevice(std::string &SerialNumber) {
		try {
			std::vector<std::string>	DBList{"Devices", "Statistics", "CommandList", "HealthChecks", "LifetimeStats", "Capabilities", "DeviceLogs"};

			for(const auto &i:DBList) {

				Poco::Data::Session     Sess = Pool_->get();
				Poco::Data::Statement   Delete(Sess);

				std::string St{"DELETE FROM " + i + " WHERE SerialNumber=?"};
				try {
					Delete << ConvertParams(St),
						Poco::Data::Keywords::use(SerialNumber);
					Delete.execute();
				} catch (...) {

				}
			}

			SerialNumberCache()->DeleteSerialNumber(SerialNumber);

			if(KafkaManager()->Enabled()) {
				nlohmann::json 	Message;
				Message["command"] = "delete_device";
				Message["payload"]["serialNumber"] = SerialNumber;
				KafkaManager()->PostMessage(KafkaTopics::COMMAND, SerialNumber, to_string(Message));
			}

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetDevice(std::string &SerialNumber, GWObjects::Device &DeviceDetails) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT " + DB_DeviceSelectFields +
						   " FROM Devices WHERE SerialNumber=?"};

			DeviceRecordTuple R;
			Select << 	ConvertParams(St),
						Poco::Data::Keywords::into(R),
						Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			if (Select.rowsExtracted()==0)
				return false;
			ConvertDeviceRecord(R,DeviceDetails);
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger().log(E);
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

			if (Select.rowsExtracted()==0)
				return false;

			return true;
		}
		catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::UpdateDevice(GWObjects::Device &NewDeviceDetails) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);

			DeviceRecordTuple R;

			NewDeviceDetails.modified = OpenWifi::Now();
			ConvertDeviceRecord(NewDeviceDetails,R);

			NewDeviceDetails.LastConfigurationChange = std::time(nullptr);
			std::string St2{"UPDATE Devices SET " +
									DB_DeviceUpdateFields +
							" WHERE SerialNumber=?"};
			Update  << ConvertParams(St2),
				Poco::Data::Keywords::use(R),
				Poco::Data::Keywords::use(NewDeviceDetails.SerialNumber);
			Update.execute();
			GetDevice(NewDeviceDetails.SerialNumber,NewDeviceDetails);
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetDevices(uint64_t From, uint64_t HowMany, std::vector<GWObjects::Device> &Devices) {
		DeviceRecordList Records;
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			std::string st{"SELECT " + DB_DeviceSelectFields + " FROM Devices ORDER BY SerialNumber ASC " + ComputeRange(From, HowMany)};
			Select << 	ConvertParams(st),
						Poco::Data::Keywords::into(Records);
			Select.execute();

			for (auto &i: Records) {
				GWObjects::Device D;
				ConvertDeviceRecord(i, D);
				Devices.push_back(D);
			}
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t & NewUUID) {
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
			Logger().log(E);
		}
		return false;
	}

	bool Storage::UpdateSerialNumberCache() {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select << "SELECT SerialNumber FROM Devices";
			Select.execute();

			Poco::Data::RecordSet   RSet(Select);

			uint64_t NumberOfDevices = 0 ;

			bool More = RSet.moveFirst();
			while(More) {
				auto SerialNumber = RSet[0].convert<std::string>();
				SerialNumberCache()->AddSerialNumber(SerialNumber);
				NumberOfDevices++;
				More = RSet.moveNext();
			}
			Logger().information(Poco::format("Added %lu serial numbers to cache.", NumberOfDevices));
			return true;

		} catch(const Poco::Exception &E) {
			Logger().log(E);
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
	static const uint64_t SECONDS_HOUR = 60*60;

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
		auto V=100.0*((float)T/65536.0);
		if(V<5.0) return "< 5%";
		if(V<25.0) return "< 25%";
		if(V<50.0) return "< 50%";
		if(V<75.0) return "< 75%";
		return ">75%";
	}

	static std::string ComputeFreeMemoryTag(uint64_t Free, uint64_t Total) {
		auto V = 100.0 * ((float)Free/(float(Total)));
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
				UpdateCountedMap(Dashboard.vendors, OUIServer()->GetManufacturer(SerialNumber));
				UpdateCountedMap(Dashboard.deviceType, DeviceType);

				GWObjects::ConnectionState	ConnState;
				if(DeviceRegistry()->GetState(SerialNumber, ConnState)) {
					UpdateCountedMap(Dashboard.status, ConnState.Connected ? "connected" : "not connected");
					UpdateCountedMap(Dashboard.certificates, ComputeCertificateTag(ConnState.VerifiedCertificate));
					UpdateCountedMap(Dashboard.lastContact, ComputeUpLastContactTag(ConnState.LastContact));
					GWObjects::HealthCheck	HC;
					if(DeviceRegistry()->GetHealthcheck(SerialNumber,HC))
						UpdateCountedMap(Dashboard.healths, ComputeSanityTag(HC.Sanity));
					else
						UpdateCountedMap(Dashboard.healths, ComputeSanityTag(100));
					std::string LastStats;
					if(DeviceRegistry()->GetStatistics(SerialNumber, LastStats) && !LastStats.empty()) {
						Poco::JSON::Parser	P;

						auto RawObject = P.parse(LastStats).extract<Poco::JSON::Object::Ptr>();

						if(RawObject->has("unit")) {
							auto Unit = RawObject->getObject("unit");
							if (Unit->has("uptime")) {
								UpdateCountedMap(Dashboard.upTimes, ComputeUpTimeTag(Unit->get("uptime")));
							}
							if (Unit->has("memory")) {
								auto Memory = Unit->getObject("memory");
								uint64_t Free = Memory->get("free");
								uint64_t Total = Memory->get("total");
								UpdateCountedMap(Dashboard.memoryUsed, ComputeFreeMemoryTag(Free, Total));
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

						uint64_t 	Associations_2G, Associations_5G;
						StateUtils::ComputeAssociations(RawObject, Associations_2G, Associations_5G);
						UpdateCountedMap(Dashboard.associations, "2G", Associations_2G);
						UpdateCountedMap(Dashboard.associations, "5G", Associations_5G);
					}
					UpdateCountedMap(Dashboard.status, ConnState.Connected ? "connected" : "not connected");
				} else {
					UpdateCountedMap(Dashboard.status, "not connected");
				}
				More = RSet.moveNext();
			}
			return true;
		} catch(const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}
}

