//
// Created by stephane bourque on 2022-02-21.
//

#pragma once

#include <string>
#include <cstring>
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
    struct msg { uint64_t err_num; std::string err_txt; };
    static const struct msg Error404{404,"Resource does not exist."};

    static const struct msg SUCCESS{0,"No error."};
    static const struct msg PASSWORD_CHANGE_REQUIRED{1,"Password change required"};
    static const struct msg INVALID_CREDENTIALS{2,"Invalid credentials."};
    static const struct msg PASSWORD_ALREADY_USED{3,"Password already used."};
    static const struct msg USERNAME_PENDING_VERIFICATION{4,"Username pending verification."};
    static const struct msg PASSWORD_INVALID{5,"Password is invalid"};
    static const struct msg INTERNAL_ERROR{6,"Internal error."};
    static const struct msg ACCESS_DENIED{7,"Access denied."};
    static const struct msg INVALID_TOKEN{8,"Invalid token."};
    static const struct msg EXPIRED_TOKEN{9,"Expired token."};
    static const struct msg RATE_LIMIT_EXCEEDED{10,"Rate limit exceeded."};
    static const struct msg BAD_MFA_TRANSACTION{11,"Bad MFA transaction."};
    static const struct msg MFA_FAILURE{12,"MFA failure."};
    static const struct msg SECURITY_SERVICE_UNREACHABLE{13,"Security service is unreachable, try again later."};
    static const struct msg CANNOT_REFRESH_TOKEN{14,"Cannot refresh token."};

    static const struct msg MissingUUID{1000,"Missing UUID."};
    static const struct msg MissingSerialNumber{1001,"Missing Serial Number."};
    static const struct msg InternalError{1002,"Internal error. Please try later."};
    static const struct msg InvalidJSONDocument{1003,"Invalid JSON document."};
    static const struct msg UnsupportedHTTPMethod{1004,"Unsupported HTTP Method"};
    static const struct msg StillInUse{1005,"Element still in use."};
    static const struct msg CouldNotBeDeleted{1006,"Element could not be deleted."};
    static const struct msg NameMustBeSet{1007,"The name property must be set."};
    static const struct msg ConfigBlockInvalid{1008,"Configuration block type invalid."};
    static const struct msg UnknownId{1009,"Unknown UUID."};
    static const struct msg InvalidDeviceTypes{1010,"Unknown or invalid device type(s)."};
    static const struct msg RecordNotCreated{1011,"Record could not be created."};
    static const struct msg RecordNotUpdated{1012,"Record could not be updated."};
    static const struct msg UnknownManagementPolicyUUID{1013,"Unknown management policy UUID."};
    static const struct msg CannotDeleteRoot{1014,"Root Entity cannot be removed, only modified."};
    static const struct msg MustCreateRootFirst{1015,"Root entity must be created first."};
    static const struct msg ParentUUIDMustExist{1016,"Parent UUID must exist."};
    static const struct msg ConfigurationMustExist{1017,"Configuration must exist."};
    static const struct msg MissingOrInvalidParameters{1018,"Invalid or missing parameters."};
    static const struct msg UnknownSerialNumber{1019,"Unknown Serial Number."};
    static const struct msg InvalidSerialNumber{1020,"Invalid Serial Number."};
    static const struct msg SerialNumberExists{1021,"Serial Number already exists."};
    static const struct msg ValidNonRootUUID{1022,"Must be a non-root, and valid UUID."};
    static const struct msg VenueMustExist{1023,"Venue does not exist."};
    static const struct msg NotBoth{1024,"You cannot specify both Entity and Venue"};
    static const struct msg EntityMustExist{1025,"Entity must exist."};
    static const struct msg ParentOrEntityMustBeSet{1026,"Parent or Entity must be set."};
    static const struct msg ContactMustExist{1027,"Contact must exist."};
    static const struct msg LocationMustExist{1028,"Location must exist."};
    static const struct msg OnlyWSSupported{1029,"This endpoint only supports WebSocket."};
    static const struct msg SerialNumberMismatch{1030,"Serial Number mismatch."};
    static const struct msg InvalidCommand{1031,"Invalid command."};
    static const struct msg NoRecordsDeleted{1032,"No records deleted."};
    static const struct msg DeviceNotConnected{1033,"Device is not currently connected."};
    static const struct msg CannotCreateWS{1034,"Telemetry system could not create WS endpoint. Please try again."};
    static const struct msg BothDeviceTypeRevision{1035,"Both deviceType and revision must be set."};
    static const struct msg IdOrSerialEmpty{1036,"SerialNumber and Id must not be empty."};
    static const struct msg MissingUserID{1037,"Missing user ID."};
    static const struct msg IdMustBe0{1038,"To create a user, you must set the ID to 0"};
    static const struct msg InvalidUserRole{1039,"Invalid userRole."};
    static const struct msg InvalidEmailAddress{1040,"Invalid email address."};
    static const struct msg PasswordRejected{1041,"Password was rejected. This maybe an old password."};
    static const struct msg InvalidIPRanges{1042,"Invalid IP range specifications."};
    static const struct msg InvalidLOrderBy{1043,"Invalid orderBy specification."};
    static const struct msg NeedMobileNumber{1044,"You must provide at least one validated phone number."};
    static const struct msg BadMFAMethod{1045,"MFA only supports sms or email."};
    static const struct msg InvalidCredentials{1046,"Invalid credentials (username/password)."};
    static const struct msg InvalidPassword{1047,"Password does not conform to basic password rules."};
    static const struct msg UserPendingVerification{1048,"User access denied pending email verification."};
    static const struct msg PasswordMustBeChanged{1049,"Password must be changed."};
    static const struct msg UnrecognizedRequest{1050,"Ill-formed request. Please consult documentation."};
    static const struct msg MissingAuthenticationInformation{1051,"Missing authentication information."};
    static const struct msg InsufficientAccessRights{1052,"Insufficient access rights to complete the operation."};
    static const struct msg ExpiredToken{1053,"Token has expired, user must login."};
    static const struct msg SubscriberMustExist{1054,"Subscriber must exist."};
    static const struct msg AuthenticatorVerificationIncomplete{1055,"Authenticator validation is not complete."};
    static const struct msg SMSCouldNotBeSentRetry{1056,"SMS could not be sent to validate device, try later or change the phone number."};
    static const struct msg SMSCouldNotValidate{1057,"Code and number could not be validated"};
    static const struct msg InvalidDeviceClass{1058,"Invalid device class. Must be: any, venue, entity, or subscriber"};
    static const struct msg SerialNumberAlreadyProvisioned{1059,"This device has already been provisioned to a subscriber."};
    static const struct msg SerialNumberNotTheProperClass{1060,"Device is not available to subscribers. It ahs been assigned to another class of devices."};
    static const struct msg UserAlreadyExists{1061,"Username already exists."};
    static const struct msg NotImplemented{1062,"Function not implemented."};
    static const struct msg VariableMustExist{1063,"Specified variable does not exist."};
    static const struct msg InvalidEntityType{1064,"Invalid entity type."};
    static const struct msg CannotDeleteSubEntity{1065,"Cannot delete the default subscriber entity."};
    static const struct msg OperatorIdMustExist{1066,"Missing or bad Operator ID"};
    static const struct msg CannotDeleteDefaultOperator{1067,"Cannot delete the default operator."};
    static const struct msg CannotCreateDefaultOperator{1068,"Cannot create the default operator."};
    static const struct msg InvalidRRM{1069,"Invalid RRM value."};
    static const struct msg InvalidIPAddresses{1070,"Invalid IP addresses."};
    static const struct msg InvalidBillingCode{1071,"Empty of invalid billing code."};
    static const struct msg InvalidBillingPeriod{1072,"Invalid billing period."};
    static const struct msg InvalidSubscriberId{1073,"Invalid subscriber ID."};
    static const struct msg InvalidContactId{1074,"Invalid contact ID."};
    static const struct msg InvalidLocationId{1075,"Invalid location ID."};
    static const struct msg InvalidContactType{1076,"Invalid contact type."};
    static const struct msg InvalidLocationType{1077,"Invalid location type."};
    static const struct msg InvalidOperatorId{1078,"Invalid operator ID."};
    static const struct msg InvalidServiceClassId{1079,"Invalid service class ID."};
    static const struct msg InvalidSubscriberDeviceId{1080,"Invalid subscriber device ID."};
    static const struct msg InvalidRegistrationOperatorId{1081,"Invalid registration operator ID."};
    static const struct msg InvalidRegistrationOperatorName{1082,"Invalid registration operator name."};
    static const struct msg RegistrationNameDuplicate{1083,"Registration name must be unique."};
    static const struct msg SMSMFANotEnabled{1084,"SMS is not enabled in the security service."};
    static const struct msg EMailMFANotEnabled{1085,"email is not enabled in the security service."};

    static const struct msg TOTInvalidCode{1086,"Invalid code."};
    static const struct msg TOTInvalidIndex{1087,"Invalid index."};
    static const struct msg TOTRepeatedCode{1088,"Code is repeated. Must be new code."};
    static const struct msg TOTInvalidProtocol{1089,"Invalid protocol sequence."};
    static const struct msg TOTNoSession{1090,"No validation session present."};

    static const struct msg SignupAlreadySigned{1091,"Code is repeated. Must be new code."};
    static const struct msg SignupEmailCheck{1092,"Waiting for email check completion."};
    static const struct msg SignupWaitingForDevice{1093,"Waiting for device."};

    static const struct msg SMSMissingPhoneNumber{1094,"Missing phone number"};
    static const struct msg SMSTryLater{1095,"SMS could not be sent. Verify the number or try again later."};
    static const struct msg SMSMissingChallenge{1096,"Missing 'challengeCode'"};
	static const struct msg MustHaveConfigElement{1097,"Must have 'configuration' element."};

	static const struct msg ModelIDListCannotBeEmpty{1098,"Model ID list cannot be empty."};
	static const struct msg DefConfigNameExists{1099,"Configuration name already exists."};

    static const struct msg SubNoDeviceActivated{1100,"No devices activated yet."};
    static const struct msg SubConfigNotRefreshed{1101,"Configuration could not be refreshed."};

    static const struct msg ProvServiceNotAvailable{1102,"Provisioning service not available yet."};
    static const struct msg SSIDInvalidPassword{1103,"Invalid password length. Must be 8 characters or greater, and a maximum of 32 characters."};
    static const struct msg InvalidStartingIPAddress{1104,"Invalid starting/ending IP address."};
    static const struct msg SubnetFormatError{1105,"Subnet must be in format like 192.168.1.1/24."};
    static const struct msg DeviceModeError{1106,"Device mode subnet must be of the form 192.168.1.1/24."};

    static const struct msg BadDeviceMode{1107,"Mode must be bridge, nat, or manual."};
    static const struct msg DefaultGatewayFormat{1108,"Default gateway must be in format like 192.168.1.1."};
    static const struct msg PrimaryDNSFormat{1109,"Primary DNS must be an IP address i.e. 192.168.1.1."};

    static const struct msg SecondaryDNSFormat{1110,"Secondary DNS must be an IP address i.e. 192.168.1.1."};
    static const struct msg BadConnectionType{1111,"Internet Connection must be automatic, bridge, pppoe, or manual."};
    static const struct msg InvalidDeviceID{1112,"Invalid deviceID."};
    static const struct msg InvalidVisibilityAttribute{1113,"Invalid visibility attribute."};
    static const struct msg UnknownConfigurationSection{1114,"Unknown section."};

    static const struct msg CannotValidatePhoneNumber{1115,"Phone number could not be validated."};
    static const struct msg RootUsersNoOwners{1116,"ROOT users may not have owners."};
    static const struct msg PartnerMustHaveEntity{1118,"Partner user must belong to an entity."};
    static const struct msg RootCannotModifyUsers{1119,"ROOT may not modify user roles."};

    static const struct msg CertificateNotIssued{1120,"Certificate was not issued."};
    static const struct msg IncompleteCertificate{1121,"Incomplete certificate information. Cannot be downloaded. You must delete and recreate."};
    static const struct msg InvalidCertificateType{1122,"Invalid certificate type."};
    static const struct msg InvalidDeviceName{1123,"Invalid device name."};

    static const struct msg InvalidRedirectorName{1124,"Invalid redirector name"};
    static const struct msg CommonNameAlreadyExists{1125,"A device/server of this name already exists"};
    static const struct msg CertificateAlreadyExists{1126,"A certificate for this device already exists."};
    static const struct msg CannotCreateCertTryAgain{1127,"Device certificate could not be created. Please try later."};
    static const struct msg CouldNotRevoke{1128,"Certificate could not be revoked."};

    static const struct msg CouldNotModifyCert{1129,"Certificate could not me modified. Please verify the information you supplied."};
    static const struct msg BatchCertNoCreated{1130,"Certificates have not been created for this batch."};
    static const struct msg BatchTooBig{1131,"Illegal number of MAC Addresses: must be between 1 and 1000."};

    static const struct msg OutstandingJobs{1132,"Batch has running outstanding jobs. Please wait until job is finished."};
    static const struct msg InvalidSMSNotificationList{1133,"Invalid SMS Notification list."};
    static const struct msg InvalidEMailNotificationList{1134,"Invalid email Notification list."};
    static const struct msg CannotChangeCommanNames{1135,"You cannot provide new/modified common names after jobs have been run for a batch."};
    static const struct msg FailedToVerifyDigicert{1136,"Failed to verify the DigiCert information provided."};
    static const struct msg CouldNotPerformCommand{1137,"Could not perform command."};

	static const struct msg PoolNameInvalid{1138,"Pool name is invalid."};
	static const struct msg InvalidRadiusProxyStrategy{1139,"Strategy name must be: random, round_robin, weighted."};
	static const struct msg InvalidRadiusProxyMonitorMethod{1140,"monitorMethod must be: none, https, radius."};
	static const struct msg MustHaveAtLeastOneRadiusServer{1141,"Must have at least one RADIUS server."};
	static const struct msg InvalidRadiusServerEntry{1142,"RADIUS Server IP address invalid or port missing."};
	static const struct msg InvalidRadiusServerWeigth{1143,"RADIUS Server IP weight cannot be 0."};

	static const struct msg MaximumRTTYSessionsReached{1144,"Too many RTTY sessions currently active"};
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
	static const char * DEBUG = "debug";
	static const char * SCRIPT = "script";
	static const char * TIMEOUT = "timeout";

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
	static const char *RESULT_64 = "result_64";
	static const char *RESULT_SZ = "result_sz";
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
	static const char *DEBUG = "debug";
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

	static const char *SCRIPT = "script";
	static const char *TYPE = "type";

	static const char *RADIUS = "radius";
	static const char *RADIUSDATA = "data";
	static const char *RADIUSACCT = "acct";
	static const char *RADIUSAUTH = "auth";
	static const char *RADIUSCOA = "coa";
	static const char *RADIUSDST = "dst";
	static const char *IES = "ies";
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
	static const char *VENUE_BROADCAST = "venue_broadcast";

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
		ET_TELEMETRY,
		ET_VENUEBROADCAST
	};

	inline EVENT_MSG EventFromString(const std::string & Method) {
		if(strcmp(STATE,Method.c_str())==0)
			return ET_STATE;
		else if(strcmp(HEALTHCHECK,Method.c_str())==0)
			return ET_HEALTHCHECK;
		else if(strcmp(CONNECT,Method.c_str())==0)
			return ET_CONNECT;
		else if(strcmp(CFGPENDING,Method.c_str())==0)
			return ET_CFGPENDING;
		else if(strcmp(CRASHLOG,Method.c_str())==0)
			return ET_CRASHLOG;
		else if(strcmp(DEVICEUPDATE,Method.c_str())==0)
			return ET_DEVICEUPDATE;
		else if(strcmp(LOG,Method.c_str())==0)
			return ET_LOG;
		else if(strcmp(PING,Method.c_str())==0)
			return ET_PING;
		else if(strcmp(RECOVERY,Method.c_str())==0)
			return ET_RECOVERY;
		else if(strcmp(TELEMETRY,Method.c_str())==0)
			return ET_TELEMETRY;
		else if(strcmp(VENUE_BROADCAST,Method.c_str())==0)
			return ET_VENUEBROADCAST;
		return ET_UNKNOWN;
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


