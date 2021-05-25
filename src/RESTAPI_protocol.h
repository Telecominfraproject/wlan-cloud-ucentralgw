//
// Created by stephane bourque on 2021-05-25.
//

#ifndef UCENTRALGW_RESTAPI_PROTOCOL_H
#define UCENTRALGW_RESTAPI_PROTOCOL_H

namespace uCentral::RESTAPI::Protocol {
	static const char * CAPABILITIES = "capabilities";
	static const char * LOGS = "logs";
	static const char * HEALTHCHECKS = "healthchecks";
	static const char * STATISTICS = "statistics";
	static const char * STATUS = "status";
	static const char * SERIALNUMBER = "serialNumber";
	static const char * PERFORM = "perform";
	static const char * CONFIGURE = "configure";
	static const char * UPGRADE = "upgrade";
	static const char * REBOOT = "reboot";
	static const char * FACTORY = "factory";
	static const char * LEDS = "leds";
	static const char * TRACE = "trace";
	static const char * REQUEST = "request";
	static const char * WIFISCAN = "wifiscan";
	static const char * EVENTQUEUE = "eventqueue";
	static const char * RTTY = "rtty";
	static const char * COMMAND = "command";
	static const char * STARTDATE = "startDate";
	static const char * ENDDATE = "endDate";
	static const char * OFFSET = "offset";
	static const char * LIMIT = "limit";
	static const char * LIFETIME = "lifetime";
}

#endif // UCENTRALGW_RESTAPI_PROTOCOL_H
