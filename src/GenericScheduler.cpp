//
// Created by stephane bourque on 2023-04-19.
//

#include "GenericScheduler.h"

namespace OpenWifi {

	int GenericScheduler::Start() {
		poco_information(Logger(),"Starting...");
		return 0;
	}

	void GenericScheduler::Stop() {
		poco_information(Logger(),"Stopping...");
		poco_information(Logger(),"Stopped...");
	}

} // namespace OpenWifi