//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <fstream>
#include "uStorageService.h"
#include "uCentral.h"
#include "uDeviceRegistry.h"
#include "Poco/Util/Application.h"
#include "utils.h"

namespace uCentral::Storage {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("Storage", "STORAGE-SVR", "storage")
    {
    }

    int Start() {
        return uCentral::Storage::Service::instance()->Start();
    }

    void Stop() {
        uCentral::Storage::Service::instance()->Stop();
    }

	std::string Service::ConvertParams(const std::string & S) const {
		std::string R;

		R.reserve(S.size()*2+1);

		if(dbType_==pgsql) {
			auto Idx=1;
			for(auto const & i:S)
			{
				if(i=='?') {
					R += '$';
					R.append(std::to_string(Idx++));
				} else {
					R += i;
				}
			}
		} else {
			R = S;
		}
		return R;
	}

    int Service::Start() {
		SubMutexGuard		Guard(Mutex_);

		Logger_.setLevel(Poco::Message::PRIO_NOTICE);
        Logger_.notice("Starting.");
        std::string DBType = uCentral::ServiceConfig::GetString("storage.type");

        if (DBType == "sqlite") {
            Setup_SQLite();
        } else if (DBType == "postgresql") {
            Setup_PostgreSQL();
        } else if (DBType == "mysql") {
            Setup_MySQL();
        } else if (DBType == "odbc") {
            Setup_ODBC();
        }

		Create_Tables();

		return 0;
    }

    void Service::Stop() {
        Logger_.notice("Stopping.");
    }
}
// namespace