//
// Created by stephane bourque on 2023-11-28.
//

#pragma once

#include <mutex>
#include <Poco/Data/Session.h>

namespace OpenWifi {

	struct LockedDbSession {
		std::unique_ptr<Poco::Data::Session> Session;
		std::unique_ptr<std::mutex> Mutex;

		inline Poco::Data::Session & operator *() { return *Session; }
	};

}