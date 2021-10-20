//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI/RESTAPI_GWobjects.h"
#include "StorageService.h"
namespace OpenWifi {

	/*
		Sess << "CREATE TABLE IF NOT EXISTS BlackList ("
					"SerialNumber	VARCHAR(30) PRIMARY KEY, "
					"Reason			TEXT, "
					"Created		BIGINT, "
					"Author			VARCHAR(64)"
					")", Poco::Data::Keywords::now;
	 */

	const static std::string DB_BlackListDeviceSelectFields{	"SerialNumber,"
																   "Reason, "
																   "Created, "
																   "Author "};

	const static std::string DB_BlackListDeviceUpdateFields{	"SerialNumber=?,"
																   "Reason=?, "
																   "Created=?, "
																   "Author=? "};

	const static std::string DB_BlackListDeviceInsertValues{" VALUES(?,?,?,?) "};

	typedef Poco::Tuple<
		std::string,
		std::string,
		uint64_t,
		std::string
	> BlackListDeviceRecordTuple;
	typedef std::vector<BlackListDeviceRecordTuple> BlackListDeviceRecordList;

	void ConvertBlackListDeviceRecord(const BlackListDeviceRecordTuple & R, GWObjects::BlackListedDevice &D) {
		D.serialNumber = Poco::toLower(R.get<0>());
		D.reason = R.get<1>();
		D.created = R.get<2>();
		D.author = R.get<3>();
	}

	void ConvertBlackListDeviceRecord(const GWObjects::BlackListedDevice &D, BlackListDeviceRecordTuple & R) {
		R.set<0>(Poco::toLower(D.serialNumber));
		R.set<1>(D.reason);
		R.set<2>(D.created);
		R.set<3>(D.author);
	}

	bool Storage::AddBlackListDevice(GWObjects::BlackListedDevice &  Device) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Insert(Sess);

			std::string St{"INSERT INTO BlackList (" + DB_BlackListDeviceSelectFields + ") " + DB_BlackListDeviceInsertValues };

			BlackListDeviceRecordTuple T;
			ConvertBlackListDeviceRecord(Device,T);
			Insert << ConvertParams(St),
				Poco::Data::Keywords::use(T);
			Insert.execute();
			return true;

		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::AddBlackListDevices(std::vector<GWObjects::BlackListedDevice> &Devices) {
		try {
			for (auto &i : Devices) {
				AddBlackListDevice(i);
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::DeleteBlackListDevice(std::string &SerialNumber) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			std::string St{"DELETE FROM BlackList WHERE SerialNumber=?"};

			Poco::toLowerInPlace(SerialNumber);
			Delete << ConvertParams(St),
				Poco::Data::Keywords::use(SerialNumber);
			Delete.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::GetBlackListDevice(std::string & SerialNumber, GWObjects::BlackListedDevice &Device) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			Poco::toLowerInPlace(SerialNumber);
			std::string st{"SELECT " + DB_BlackListDeviceSelectFields +  " FROM BlackList where SerialNumber=? "};

			BlackListDeviceRecordTuple	T;
			Select << 	ConvertParams(st),
						Poco::Data::Keywords::into(T),
						Poco::Data::Keywords::use(SerialNumber);
			Select.execute();
			if(Select.rowsExtracted()==1)
				ConvertBlackListDeviceRecord(T,Device);

			return Select.rowsExtracted()==1;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::UpdateBlackListDevice(std::string & SerialNumber, GWObjects::BlackListedDevice & Device) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Update(Sess);

			std::string St{"UPDATE BlackList " + DB_BlackListDeviceUpdateFields + " where serialNumber=?" };

			BlackListDeviceRecordTuple T;
			ConvertBlackListDeviceRecord(Device,T);
			Update << ConvertParams(St),
				Poco::Data::Keywords::use(T),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();

			return true;

		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::GetBlackListDevices(uint64_t Offset, uint64_t HowMany,
									  std::vector<GWObjects::BlackListedDevice> &Devices) {
		try {
			BlackListDeviceRecordList Records;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			Select << "SELECT " + DB_BlackListDeviceSelectFields + " FROM BlackList ORDER BY SerialNumber ASC "
						+ ComputeRange(Offset,HowMany),
				Poco::Data::Keywords::into(Records);
			Select.execute();

			for (auto i : Records) {
				GWObjects::BlackListedDevice R;
				ConvertBlackListDeviceRecord(i,R);
				Devices.push_back(R);
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::IsBlackListed(std::string &SerialNumber) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string TmpSerialNumber;

			std::string St{"SELECT SerialNumber FROM BlackList WHERE SerialNumber=?"};
			Poco::toLowerInPlace(SerialNumber);
			Select << ConvertParams(St),
				Poco::Data::Keywords::into(TmpSerialNumber),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			return !(Select.rowsExtracted()!=1);

		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}
}