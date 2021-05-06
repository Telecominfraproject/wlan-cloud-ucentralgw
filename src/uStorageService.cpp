//
// Created by stephane bourque on 2021-03-01.
//
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "uStorageService.h"
#include "uCentral.h"
#include "uDeviceRegistry.h"
#include "Poco/Data/RecordSet.h"
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

		if(IsPSQL_) {
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
        std::lock_guard<SubMutex> guard(Mutex_);

		Logger_.setLevel(Poco::Message::PRIO_NOTICE);
        Logger_.notice("Starting.");
        std::string DBType = uCentral::ServiceConfig::getString("storage.type");

        if (DBType == "sqlite") {
            return Setup_SQLite();
        } else if (DBType == "postgresql") {
            return Setup_PostgreSQL();
        } else if (DBType == "mysql") {
            return Setup_MySQL();
        } else if (DBType == "odbc") {
            return Setup_ODBC();
        }
        uCentral::instance()->exit(Poco::Util::Application::EXIT_CONFIG);
		return -1;
    }

    void Service::Stop() {
        Logger_.notice("Stopping.");
    }
}
// namespace