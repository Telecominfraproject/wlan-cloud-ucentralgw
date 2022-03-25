//
// Created by stephane bourque on 2022-02-21.
//

#pragma once

#include <string>
#include "Poco/String.h"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

namespace OpenWifi::RESTAPI::Errors {
	static const std::string MissingUUID{"Missing UUID."};
	static const std::string MissingSerialNumber{"Missing Serial Number."};
	static const std::string InternalError{"Internal error. Please try later."};
	static const std::string InvalidJSONDocument{"Invalid JSON document."};
	static const std::string UnsupportedHTTPMethod{"Unsupported HTTP Method"};
	static const std::string StillInUse{"Element still in use."};
	static const std::string CouldNotBeDeleted{"Element could not be deleted."};
	static const std::string NameMustBeSet{"The name property must be set."};
	static const std::string ConfigBlockInvalid{"Configuration block type invalid."};
	static const std::string UnknownId{"Unknown UUID."};
	static const std::string InvalidDeviceTypes{"Unknown or invalid device type(s)."};
	static const std::string RecordNotCreated{"Record could not be created."};
	static const std::string RecordNotUpdated{"Record could not be updated."};
	static const std::string UnknownManagementPolicyUUID{"Unknown management policy UUID."};
	static const std::string CannotDeleteRoot{"Root Entity cannot be removed, only modified."};
	static const std::string MustCreateRootFirst{"Root entity must be created first."};
	static const std::string ParentUUIDMustExist{"Parent UUID must exist."};
	static const std::string ConfigurationMustExist{"Configuration must exist."};
	static const std::string MissingOrInvalidParameters{"Invalid or missing parameters."};
	static const std::string UnknownSerialNumber{"Unknown Serial Number."};
	static const std::string InvalidSerialNumber{"Invalid Serial Number."};
	static const std::string SerialNumberExists{"Serial Number already exists."};
	static const std::string ValidNonRootUUID{"Must be a non-root, and valid UUID."};
	static const std::string VenueMustExist{"Venue does not exist."};
	static const std::string NotBoth{"You cannot specify both Entity and Venue"};
	static const std::string EntityMustExist{"Entity must exist."};
	static const std::string ParentOrEntityMustBeSet{"Parent or Entity must be set."};
	static const std::string ContactMustExist{"Contact must exist."};
	static const std::string LocationMustExist{"Location must exist."};
	static const std::string OnlyWSSupported{"This endpoint only supports WebSocket."};
	static const std::string SerialNumberMismatch{"Serial Number mismatch."};
	static const std::string InvalidCommand{"Invalid command."};
	static const std::string NoRecordsDeleted{"No records deleted."};
	static const std::string DeviceNotConnected{"Device is not currently connected."};
	static const std::string CannotCreateWS{"Telemetry system could not create WS endpoint. Please try again."};
	static const std::string BothDeviceTypeRevision{"Both deviceType and revision must be set."};
	static const std::string IdOrSerialEmpty{"SerialNumber and Id must not be empty."};
	static const std::string MissingUserID{"Missing user ID."};
	static const std::string IdMustBe0{"To create a user, you must set the ID to 0"};
	static const std::string InvalidUserRole{"Invalid userRole."};
	static const std::string InvalidEmailAddress{"Invalid email address."};
	static const std::string PasswordRejected{"Password was rejected. This maybe an old password."};
	static const std::string InvalidIPRanges{"Invalid IP range specifications."};
	static const std::string InvalidLOrderBy{"Invalid orderBy specification."};
	static const std::string NeedMobileNumber{"You must provide at least one validated phone number."};
	static const std::string BadMFAMethod{"MFA only supports sms or email."};
	static const std::string InvalidCredentials{"Invalid credentials (username/password)."};
	static const std::string InvalidPassword{"Password does not conform to basic password rules."};
	static const std::string UserPendingVerification{"User access denied pending email verification."};
	static const std::string PasswordMustBeChanged{"Password must be changed."};
	static const std::string UnrecognizedRequest{"Ill-formed request. Please consult documentation."};
	static const std::string MissingAuthenticationInformation{"Missing authentication information."};
	static const std::string InsufficientAccessRights{"Insufficient access rights to complete the operation."};
	static const std::string ExpiredToken{"Token has expired, user must login."};
	static const std::string SubscriberMustExist{"Subscriber must exist."};
	static const std::string AuthenticatorVerificationIncomplete{"Authenticator validation is not complete."};
	static const std::string SMSCouldNotBeSentRetry{"SMS could not be sent to validate device, try later or change the phone number."};
	static const std::string SMSCouldNotValidate{"Code and number could not be validated"};
	static const std::string InvalidDeviceClass{"Invalid device class. Must be: any, venue, entity, or subscriber"};
    static const std::string SerialNumberAlreadyProvisioned{"This device has already been provisioned to a subscriber."};
    static const std::string SerialNumberNotTheProperClass{"Device is not available to subscribers. It ahs been assigned to another class of devices."};
    static const std::string UserAlreadyExists{"Username already exists."};
    static const std::string NotImplemented{"Function not implemented."};
    static const std::string VariableMustExist{"Specified variable does not exist."};
}

namespace OpenWifi::RESTAPI::Protocol {
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
	static const char * UUID = "UUID";
	static const char * DATA = "data";
	static const char * CONFIGURATION = "configuration";
	static const char * WHEN = "when";
	static const char * URI = "uri";
	static const char * LOGTYPE = "logType";
	static const char * VALUES = "values";
	static const char * TYPES = "types";
	static const char * PAYLOAD = "payload";
	static const char * KEEPREDIRECTOR = "keepRedirector";
	static const char * NETWORK = "network";
	static const char * INTERFACE = "interface";
	static const char * BANDS = "bands";
	static const char * CHANNELS = "channels";
	static const char * VERBOSE = "verbose";
	static const char * MESSAGE = "message";
	static const char * STATE = "state";
	static const char * HEALTHCHECK = "healthcheck";
	static const char * PCAP_FILE_TYPE = "pcap";
	static const char * DURATION = "duration";
	static const char * NUMBEROFPACKETS = "numberOfPackets";
	static const char * FILTER = "filter";
	static const char * SELECT = "select";
	static const char * SERIALONLY = "serialOnly";
	static const char * COUNTONLY = "countOnly";
	static const char * DEVICEWITHSTATUS = "deviceWithStatus";
	static const char * DEVICESWITHSTATUS = "devicesWithStatus";
	static const char * DEVICES = "devices";
	static const char * COUNT = "count";
	static const char * SERIALNUMBERS = "serialNumbers";
	static const char * CONFIGURATIONS = "configurations";
	static const char * NAME = "name";
	static const char * COMMANDS = "commands";
	static const char * COMMANDUUID = "commandUUID";
	static const char * FIRMWARES = "firmwares";
	static const char * TOPIC = "topic";
	static const char * HOST = "host";
	static const char * OS = "os";
	static const char * HOSTNAME = "hostname";
	static const char * PROCESSORS = "processors";
	static const char * REASON = "reason";
	static const char * RELOAD = "reload";
	static const char * SUBSYSTEMS = "subsystems";
	static const char * FILEUUID = "uuid";
	static const char * USERID = "userId";
	static const char * PASSWORD = "password";
	static const char * TOKEN = "token";
	static const char * SETLOGLEVEL = "setloglevel";
	static const char * GETLOGLEVELS = "getloglevels";
	static const char * GETSUBSYSTEMNAMES = "getsubsystemnames";
	static const char * GETLOGLEVELNAMES = "getloglevelnames";
	static const char * STATS = "stats";
	static const char * PING = "ping";
	static const char * PARAMETERS = "parameters";
	static const char * VALUE = "value";
	static const char * LASTONLY = "lastOnly";
	static const char * NEWEST = "newest";
	static const char * ACTIVESCAN = "activeScan";
	static const char * OVERRIDEDFS = "override_dfs";
	static const char * LIST = "list";
	static const char * TAG = "tag";
	static const char * TAGLIST = "tagList";
	static const char * DESCRIPTION = "description";
	static const char * NOTES = "notes";
	static const char * DEVICETYPE = "deviceType";
	static const char * REVISION = "revision";
	static const char * AGES = "ages";
	static const char * REVISIONS = "revisions";
	static const char * DEVICETYPES = "deviceTypes";
	static const char * LATESTONLY = "latestOnly";
	static const char * IDONLY = "idOnly";
	static const char * REVISIONSET = "revisionSet";
	static const char * DEVICESET = "deviceSet";
	static const char * HISTORY = "history";
	static const char * ID = "id";
	static const char * VERSION = "version";
	static const char * TIMES = "times";
	static const char * UPTIME = "uptime";
	static const char * START = "start";

	static const char * NEWPASSWORD = "newPassword";
	static const char * USERS = "users";
	static const char * WITHEXTENDEDINFO = "withExtendedInfo";

	static const char * ERRORTEXT = "errorText";
	static const char * ERRORCODE = "errorCode";
	static const char * AVATARID = "avatarId";
	static const char * UNNAMED = "(unnamed)";
	static const char * UNSPECIFIED = "(unspecified)";
	static const char * CONTENTDISPOSITION = "Content-Disposition";
	static const char * CONTENTTYPE = "Content-Type";

	static const char * REQUIREMENTS = "requirements";
	static const char * PASSWORDPATTERN = "passwordPattern";
	static const char * ACCESSPOLICY = "accessPolicy";
	static const char * PASSWORDPOLICY = "passwordPolicy";
	static const char * FORGOTPASSWORD = "forgotPassword";
	static const char * RESENDMFACODE = "resendMFACode";
	static const char * COMPLETEMFACHALLENGE = "completeMFAChallenge";
	static const char * ME = "me";
	static const char * TELEMETRY = "telemetry";
	static const char * INTERVAL = "interval";
	static const char * UI = "UI";
	static const char * BANDWIDTH = "bandwidth";
}

namespace OpenWifi::uCentralProtocol {

    const int SERIAL_NUMBER_LENGTH = 30;

    // vocabulary used in the PROTOCOL.md file
    static const char *JSONRPC = "jsonrpc";
    static const char *ID = "id";
    static const char *UUID = "uuid";
    static const char *JSONRPC_VERSION = "2.0";
    static const char *METHOD = "method";
    static const char *PARAMS = "params";
    static const char *SERIAL = "serial";
    static const char *FIRMWARE = "firmware";
    static const char *CONNECT = "connect";
    static const char *STATE = "state";
    static const char *STATUS = "status";
    static const char *ERROR = "error";
    static const char *TEXT = "text";
    static const char *HEALTHCHECK = "healthcheck";
    static const char *LOG = "log";
    static const char *CRASHLOG = "crashlog";
    static const char *PING = "ping";
    static const char *CFGPENDING = "cfgpending";
    static const char *RECOVERY = "recovery";
    static const char *COMPRESS_64 = "compress_64";
    static const char *CAPABILITIES = "capabilities";
    static const char *REQUEST_UUID = "request_uuid";
    static const char *SANITY = "sanity";
    static const char *DATA = "data";
    static const char *LOGLINES = "loglines";
    static const char *SEVERITY = "severity";
    static const char *ACTIVE = "active";
    static const char *OVERRIDEDFS = "override_dfs";
    static const char *REBOOT = "reboot";
    static const char *WHEN = "when";
    static const char *CONFIG = "config";
    static const char *EMPTY_JSON_DOC = "{}";
    static const char *RESULT = "result";
    static const char *REQUEST = "request";
    static const char *PERFORM = "perform";
    static const char *CONFIGURE = "configure";
    static const char *PENDING = "pending";
    static const char *SUBMITTED_BY_SYSTEM = "*system";
    static const char *URI = "uri";
    static const char *COMMAND = "command";
    static const char *PAYLOAD = "payload";
    static const char *KEEP_REDIRECTOR = "keep_redirector";
    static const char *DURATION = "duration";
    static const char *PATTERN = "pattern";
    static const char *LEDS = "leds";
    static const char *ON = "on";
    static const char *OFF = "off";
    static const char *BLINK = "blink";
    static const char *PACKETS = "packets";
    static const char *NETWORK = "network";
    static const char *INTERFACE = "interface";
    static const char *TRACE = "trace";
    static const char *WIFISCAN = "wifiscan";
    static const char *TYPES = "types";
    static const char *EVENT = "event";
    static const char *MESSAGE = "message";
    static const char *RTTY = "rtty";
    static const char *TOKEN = "token";
    static const char *SERVER = "server";
    static const char *PORT = "port";
    static const char *USER = "user";
    static const char *TIMEOUT = "timeout";
    static const char *UPGRADE = "upgrade";
    static const char *FACTORY = "factory";
    static const char *VERBOSE = "verbose";
    static const char *BANDS = "bands";
    static const char *CHANNELS = "channels";
    static const char *PASSWORD = "password";
    static const char *DEVICEUPDATE = "deviceupdate";

    static const char *SERIALNUMBER = "serialNumber";
    static const char *COMPATIBLE = "compatible";
    static const char *DISCONNECTION = "disconnection";
    static const char *TIMESTAMP = "timestamp";
    static const char *SYSTEM = "system";
    static const char *HOST = "host";
    static const char *CONNECTIONIP = "connectionIp";
    static const char *TELEMETRY = "telemetry";
    static const char *BANDWIDTH = "bandwidth";
}

namespace OpenWifi::uCentralProtocol::Events {

    static const char *CONNECT = "connect";
    static const char *STATE = "state";
    static const char *HEALTHCHECK = "healthcheck";
    static const char *LOG = "log";
    static const char *CRASHLOG = "crashlog";
    static const char *PING = "ping";
    static const char *CFGPENDING = "cfgpending";
    static const char *RECOVERY = "recovery";
    static const char *TELEMETRY = "telemetry";
    static const char *DEVICEUPDATE = "deviceupdate";

    enum EVENT_MSG {
		ET_UNKNOWN,
		ET_CONNECT,
		ET_STATE,
		ET_HEALTHCHECK,
		ET_LOG,
		ET_CRASHLOG,
		ET_PING,
		ET_CFGPENDING,
		ET_RECOVERY,
		ET_DEVICEUPDATE,
		ET_TELEMETRY
	};

	inline static EVENT_MSG EventFromString(const std::string & Method) {
        static std::vector<std::pair<const char *,EVENT_MSG>>   Values{
                { CFGPENDING , ET_CFGPENDING },
                { CONNECT, ET_CONNECT },
                { CRASHLOG, ET_CRASHLOG },
                { DEVICEUPDATE, ET_DEVICEUPDATE },
                { HEALTHCHECK, ET_HEALTHCHECK },
                { LOG, ET_LOG },
                { PING, ET_PING },
                { RECOVERY, ET_RECOVERY },
                { STATE, ET_STATE },
                { TELEMETRY, ET_TELEMETRY }
        };

        std::string L = Poco::toLower(Method);
        auto hint = std::find_if(cbegin(Values),cend(Values),[&](const std::pair<const char *,EVENT_MSG> &v) ->bool { return strcmp(v.first,L.c_str())==0; });
        if(hint == cend(Values))
            return ET_UNKNOWN;
        return hint->second;
	};
}

namespace OpenWifi::Provisioning::DeviceClass {

    static const char * ANY = "any";
    static const char * SUBSCRIBER = "subscriber";
    static const char * VENUE = "venue";
    static const char * ENTITY = "entity";

    inline bool Validate(const char *s) {
        static std::vector<std::string> Values{ ANY, ENTITY, SUBSCRIBER, VENUE };
        return std::find(cbegin(Values), cend(Values), s) != cend(Values);
    }

}

#if defined(__GNUC__ )
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif


