//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"

/*
	Sess << "CREATE TABLE IF NOT EXISTS BlackList ("
				"SerialNumber	VARCHAR(30) PRIMARY KEY, "
				"Reason			TEXT, "
				"Created		BIGINT, "
				"Author			VARCHAR(64)"
				")", Poco::Data::Keywords::now;
 */

#include "RESTAPI_GWobjects.h"

namespace uCentral {

	bool Storage::AddBlackListDevices(std::vector<GWObjects::BlackListedDevice> &Devices) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Insert(Sess);

			for (auto &i : Devices) {
				std::string St{"INSERT INTO BlackList (SerialNumber, Reason, Author, Created) "
							   "VALUES(?,?,?,?)"};
				Insert << ConvertParams(St), Poco::Data::Keywords::use(i.SerialNumber),
					Poco::Data::Keywords::use(i.Reason), Poco::Data::Keywords::use(i.Author),
					Poco::Data::Keywords::use(i.Created);
				Insert.execute();
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

			Delete << ConvertParams(St), Poco::Data::Keywords::use(SerialNumber);
			Delete.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::GetBlackListDevices(uint64_t Offset, uint64_t HowMany,
									  std::vector<GWObjects::BlackListedDevice> &Devices) {
		try {
			using tuple_list = Poco::Tuple<std::string, std::string, std::string, uint64_t>;
			using record_list = std::vector<tuple_list>;

			record_list Records;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			Select << "SELECT SerialNumber, Reason, Author, Created FROM BlackList"
						+ ComputeRange(Offset,HowMany),
				Poco::Data::Keywords::into(Records);
			Select.execute();

			for (auto i : Records) {
				GWObjects::BlackListedDevice R{.SerialNumber = i.get<0>(),
											.Reason = i.get<1>(),
											.Author = i.get<2>(),
											.Created = i.get<3>()};
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

			Select << ConvertParams(St), Poco::Data::Keywords::into(TmpSerialNumber),
				Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			return !TmpSerialNumber.empty();

		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}
}