//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_UCENTRALPROTOCOL_H
#define UCENTRALGW_UCENTRALPROTOCOL_H

#include "Poco/String.h"

namespace uCentral::uCentralProtocol {

	const int SERIAL_NUMBER_LENGTH = 30;

	// vocabulary used in the PROTOCOL.md file
	static const char * JSONRPC = "jsonrpc";
	static const char * ID = "id";
	static const char * UUID = "uuid";
	static const char * JSONRPC_VERSION = "2.0";
	static const char * METHOD = "method";
	static const char * PARAMS = "params";
	static const char * SERIAL = "serial";
	static const char * FIRMWARE = "firmware";
	static const char * CONNECT = "connect";
	static const char * STATE = "state";
	static const char * HEALTHCHECK = "healthcheck";
	static const char * LOG = "log";
	static const char * CRASHLOG = "crashlog";
	static const char * PING = "ping";
	static const char * CFGPENDING = "cfgpending";
	static const char * RECOVERY = "recovery";
	static const char * COMPRESS_64 = "compress_64";
	static const char * CAPABILITIES = "capabilities";
	static const char * REQUEST_UUID = "request_uuid";
	static const char * SANITY = "sanity";
	static const char * DATA = "data";
	static const char * LOGLINES = "loglines";
	static const char * SEVERITY = "severity";
	static const char * ACTIVE = "active";
	static const char * REBOOT = "reboot";
	static const char * WHEN = "when";
	static const char * CONFIG = "config";
	static const char * EMPTY_JSON_DOC = "{}";
	static const char * RESULT = "result";
	static const char * REQUEST = "request";
	static const char * PERFORM = "perform";
	static const char * CONFIGURE = "configure";
	static const char * PENDING = "pending";
	static const char * SUBMITTED_BY_SYSTEM = "*system";
	static const char * URI = "uri";
	static const char * COMMAND = "command";
	static const char * PAYLOAD = "payload";
	static const char * KEEP_REDIRECTOR = "keep_redirector";
	static const char * DURATION = "duration";
	static const char * PATTERN = "pattern";
	static const char * LEDS = "leds";
	static const char * ON = "on";
	static const char * OFF = "off";
	static const char * BLINK = "blink";
	static const char * PACKETS = "packets";
	static const char * NETWORK = "network";
	static const char * INTERFACE = "interface";
	static const char * TRACE = "trace";
	static const char * WIFISCAN = "wifiscan";
	static const char * TYPES = "types";
	static const char * EVENT = "event";
	static const char * MESSAGE = "message";
	static const char * RTTY = "rtty";
	static const char * TOKEN = "token";
	static const char * SERVER = "server";
	static const char * PORT = "port";
	static const char * USER = "user";
	static const char * TIMEOUT = "timeout";
	static const char * UPGRADE = "upgrade";
	static const char * FACTORY = "factory";
	static const char * VERBOSE = "verbose";
	static const char * BANDS = "bands";
	static const char * CHANNELS = "channels";

	enum EVENT_MSG {
			ET_UNKNOWN,
			ET_CONNECT,
			ET_STATE,
			ET_HEALTHCHECK,
			ET_LOG,
			ET_CRASHLOG,
			ET_PING,
			ET_CFGPENDING,
			ET_RECOVERY
		};

	static EVENT_MSG EventFromString(const std::string & Method) {
		if (!Poco::icompare(Method, CONNECT)) {
			return ET_CONNECT;
		} else if (!Poco::icompare(Method, STATE)) {
			return ET_STATE;
		} else if (!Poco::icompare(Method, HEALTHCHECK)) {
			return ET_HEALTHCHECK;
		} else if (!Poco::icompare(Method, LOG)) {
			return ET_LOG;
		} else if (!Poco::icompare(Method, CRASHLOG)) {
			return ET_CRASHLOG;
		} else if (!Poco::icompare(Method, PING)) {
			return ET_PING;
		} else if (!Poco::icompare(Method, CFGPENDING)) {
			return ET_CFGPENDING;
		} else if (!Poco::icompare(Method, RECOVERY)) {
			return ET_RECOVERY;
		} else
			return ET_UNKNOWN;
	};
}

#endif // UCENTRALGW_UCENTRALPROTOCOL_H
