
// Version: 1.0.0
// Title: CloudSDK Portal API
// Description: APIs that provide services for provisioning, monitoring, and history retrieval of various data elements of CloudSDK.
// Terms of service: undefined
// Contact: undefined



#include <string> 
#include <vector> 
#include <variant> 
#include "openapi_local_defs.h" 
#include "Poco/JSON/JSON.h" 
#include "Poco/JSON/Object.h"
#include "Poco/URI.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/NetSSL.h"
#include "Poco/Net/PrivateKeyPassphraseHandler.h"
#include "Poco/Net/KeyFileHandler.h"
#include "Poco/Net/InvalidCertificateHandler.h"
#include "Poco/Logger.h"

namespace TIP::PORTAL {

// Forward declarations for all classes
class NeighbourScanReports;
class PasspointOsuIcon;
class ClientEapDetails;
class GatewayRemovedEvent;
class ProfileDetails;
class ClientSessionRemovedEvent;
class RealTimeSipCallEventWithStats;
class WmmQueueStatsPerQueueTypeMap;
class ClientIdEvent;
class EquipmentCapacityDetailsMap;
class RoutingChangedEvent;
class DhcpDiscoverEvent;
class ProfileRemovedEvent;
class LinkQualityAggregatedStats;
class RadioBasedSsidConfigurationMap;
class ActiveScanSettings;
class SourceSelectionManagement;
class AclTemplate;
class LocationAddedEvent;
class LocalTimeValue;
class SortColumnsClient;
class IntegerValueMap;
class WepConfiguration;
class OperatingSystemPerformance;
class UserDetails;
class MinMaxAvgValueIntPerRadioMap;
class AlarmChangedEvent;
class ClientInfoDetails;
class GatewayAddedEvent;
class RadioUtilizationPerRadioDetails;
class EquipmentPerRadioUtilizationDetails;
class ListOfRadioUtilizationPerRadioMap;
class DnsProbeMetric;
class EquipmentDetails;
class ClientAssocEvent;
class ListOfMacsPerRadioMap;
class CustomerAddedEvent;
class RfConfiguration;
class RoutingAddedEvent;
class RealtimeChannelHopEvent;
class CapacityPerRadioDetails;
class FirmwareVersion;
class NeighbourReport;
class PasspointMccMnc;
class EquipmentRrmBulkUpdateItem;
class ClientMetrics;
class RadioConfiguration;
class EquipmentGatewayRecord;
class VLANStatusData;
class EmptySchedule;
class EquipmentProtocolStatusData;
class ListOfMcsStatsPerRadioMap;
class CountsPerAlarmCodeMap;
class VLANStatusDataMap;
class ClientSessionChangedEvent;
class MacAddress;
class RealTimeStreamingStartEvent;
class LinkQualityAggregatedStatsPerRadioTypeMap;
class ClientDhcpDetails;
class DhcpOfferEvent;
class PasspointOperatorProfile;
class ManufacturerDetailsRecord;
class DhcpInformEvent;
class PeerInfo;
class SortColumnsEquipment;
class StreamingVideoServerRecord;
class TunnelMetricData;
class RadiusMetrics;
class BonjourServiceSet;
class NetworkAggregateStatusData;
class AutoOrManualString;
class EquipmentRrmBulkUpdateItemPerRadioMap;
class ClientConnectionDetails;
class SystemEventRecord;
class RadiusDetails;
class VlanSubnet;
class SortColumnsLocation;
class DailyTimeRangeSchedule;
class PasspointNaiRealmInformation;
class PasspointDuple;
class FirmwareTrackAssignmentRecord;
class PaginationResponseProfile;
class ServiceMetricsCollectionConfigProfile;
class Location;
class PaginationContextStatus;
class DhcpDeclineEvent;
class AlarmDetailsAttributesMap;
class ClientSessionMetricDetails;
class CustomerFirmwareTrackRecord;
class RtpFlowStats;
class PaginationContextServiceMetric;
class ClientAuthEvent;
class ChannelPowerLevel;
class FirmwareTrackAssignmentDetails;
class RadioUtilization;
class PasspointVenueTypeAssignment;
class Profile;
class PaginationResponseStatus;
class ClientActivityAggregatedStatsPerRadioTypeMap;
class Customer;
class RadioProfileConfigurationMap;
class EquipmentRemovedEvent;
class SourceSelectionSteering;
class RadioStatistics;
class ClientChangedEvent;
class EquipmentPeerStatusData;
class EquipmentAutoProvisioningSettings;
class PaginationResponseServiceMetric;
class ApNodeMetrics;
class PortalUser;
class WepKey;
class IntegerPerStatusCodeMap;
class RadioBestApSettings;
class ProfileAddedEvent;
class ServiceAdoptionMetrics;
class Client;
class AlarmRemovedEvent;
class EquipmentRrmBulkUpdateRequest;
class ApNetworkConfiguration;
class PasspointVenueName;
class PaginationResponseLocation;
class RealTimeSipCallReportEvent;
class EquipmentRoutingRecord;
class PasspointProfile;
class IntegerPerRadioTypeMap;
class SortColumnsPortalUser;
class RoutingRemovedEvent;
class TimedAccessUserRecord;
class CustomerChangedEvent;
class Alarm;
class AlarmCounts;
class MacAllowlistRecord;
class RadiusServer;
class MinMaxAvgValueInt;
class SourceSelectionValue;
class ChannelInfoReports;
class RadiusServerDetails;
class RadioProfileConfiguration;
class SortColumnsProfile;
class LongValueMap;
class ServiceMetric;
class ClientFirstDataEvent;
class LocationChangedEvent;
class ProfileChangedEvent;
class CapacityPerRadioDetailsMap;
class ManufacturerOuiDetailsPerOuiMap;
class SyslogRelay;
class AlarmAddedEvent;
class TrafficPerRadioDetailsPerRadioTypeMap;
class ApPerformance;
class ClientFailureEvent;
class ClientSession;
class AutoOrManualValue;
class EquipmentAdminStatusData;
class NoiseFloorDetails;
class RadioBasedSsidConfiguration;
class ClientTimeoutEvent;
class TrafficDetails;
class StatusChangedEvent;
class PaginationContextSystemEvent;
class DhcpRequestEvent;
class ApSsidMetrics;
class CommonProbeDetails;
class PaginationContextClient;
class AlarmDetails;
class Status;
class ActiveBSSIDs;
class TimedAccessUserDetails;
class EquipmentNeighbouringStatusData;
class WebTokenAclTemplate;
class ClientDisconnectEvent;
class BannedChannel;
class PortalUserChangedEvent;
class DhcpNakEvent;
class PasspointOsuProviderProfile;
class EquipmentPerformanceDetails;
class PaginationResponseEquipment;
class ElementRadioConfiguration;
class RealTimeStreamingStartSessionEvent;
class LocationActivityDetails;
class PaginationContextClientSession;
class ManufacturerOuiDetails;
class MapOfWmmQueueStatsPerRadioMap;
class ChannelHopSettings;
class SourceSelectionMulticast;
class GatewayChangedEvent;
class LongPerRadioTypeMap;
class NeighbouringAPListConfiguration;
class EquipmentAddedEvent;
class PingResponse;
class PaginationResponseSystemEvent;
class EquipmentUpgradeStatusData;
class RadioStatisticsPerRadioMap;
class SortColumnsClientSession;
class PaginationResponseAlarm;
class CustomerDetails;
class ClientConnectSuccessEvent;
class SortColumnsAlarm;
class ClientIpAddressEvent;
class PortalUserRemovedEvent;
class ApElementConfiguration;
class SortColumnsSystemEvent;
class ServiceMetricConfigParameters;
class GenericResponse;
class WebTokenRequest;
class PaginationContextEquipment;
class CustomerPortalDashboardStatus;
class StatusRemovedEvent;
class ClientSessionDetails;
class SsidStatistics;
class ListOfSsidStatisticsPerRadioMap;
class ClientActivityAggregatedStats;
class Equipment;
class ManagedFileInfo;
class RealTimeSipCallStopEvent;
class TrafficPerRadioDetails;
class DaysOfWeekTimeRangeSchedule;
class NetworkAdminStatusData;
class BonjourGatewayProfile;
class RadioMap;
class RadiusProfile;
class UnserializableSystemEvent;
class PairLongLong;
class RadioUtilizationDetails;
class LocationRemovedEvent;
class PerProcessUtilization;
class MeshGroupMember;
class FirmwareTrackRecord;
class BlocklistDetails;
class JsonSerializedException;
class RadiusNasConfiguration;
class CaptivePortalConfiguration;
class DhcpAckEvent;
class PaginationContextPortalUser;
class PaginationResponsePortalUser;
class MeshGroup;
class MetricConfigParameterMap;
class RtlsSettings;
class SsidConfiguration;
class PortalUserAddedEvent;
class RealTimeSipCallStartEvent;
class PaginationContextLocation;
class NoiseFloorPerRadioDetailsMap;
class EquipmentLANStatusData;
class IntegerStatusCodeMap;
class GreTunnelConfiguration;
class RealTimeStreamingStopEvent;
class RadioUtilizationPerRadioDetailsMap;
class EquipmentChangedEvent;
class ChannelInfo;
class ServiceMetricSurveyConfigParameters;
class CapacityDetails;
class LocationDetails;
class EquipmentScanDetails;
class ClientAddedEvent;
class ActiveBSSID;
class NetworkProbeMetrics;
class SortColumnsStatus;
class ChannelUtilizationPerRadioDetailsMap;
class RrmBulkUpdateApDetails;
class RfElementConfiguration;
class WmmQueueStats;
class ListOfChannelInfoReportsPerRadioMap;
class ChannelUtilizationDetails;
class MeshGroupProperty;
class PaginationContextProfile;
class PaginationContextAlarm;
class SortColumnsServiceMetric;
class ServiceMetricRadioConfigParameters;
class ClientRemovedEvent;
class EquipmentCapacityDetails;
class RadioChannelChangeSettings;
class WebTokenResult;
class BaseDhcpEvent;
class CustomerFirmwareTrackSettings;
class LocationActivityDetailsMap;
class ChannelUtilizationPerRadioDetails;
class PasspointConnectionCapability;
class RfConfigMap;
class CustomerRemovedEvent;
class RadioUtilizationReport;
class PasspointVenueProfile;
class AdvancedRadioMap;
class ClientFailureDetails;
class PaginationResponseClientSession;
class EquipmentPerRadioUtilizationDetailsMap;
class NoiseFloorPerRadioDetails;
class McsStats;
class PaginationResponseClient;
class CountsPerEquipmentIdPerAlarmCodeMap;
class RealTimeEvent;

enum class StreamingVideoType { 
	UNSUPPORTED, PLEX, YOUTUBE, NETFLIX, UNKNOWN
};

enum class ObssHopMode { 
	NON_WIFI_AND_OBSS, NON_WIFI
};

enum class NeighborScanPacketType { 
	DATA__OTHER, ACTION_NOACK, ACTION, DEAUTH, AUTH, DISASSOC, BEACON, PROBE_RESP, PROBE_REQ, REASSOC_RESP, 
	REASSOC_REQ, ASSOC_RESP, ASSOC_REQ
};

enum class TunnelIndicator { 
	secondary, primary, no
};

enum class AlarmScopeType { 
	LOCATION, CUSTOMER, VLAN, EQUIPMENT, CLIENT
};

enum class SortOrder { 
	desc, asc
};

enum class MimoMode { 
	fourByFour, threeByThree, twoByTwo, oneByOne, none
};

enum class FileCategory { 
	DoNotSteerClientList, DeviceMacBlockList, UsernamePasswordList, ExternalPolicyConfiguration, 
	CaptivePortalBackground, CaptivePortalLogo
};

enum class StatusCode { 
	disabled, error, requiresAttention, normal
};

enum class PasspointAccessNetworkType { 
	wildcard, test_or_experimental, emergency_services_only_network, person_device_network, free_public_network, 
	changeable_public_network, private_network_guest_access, private_network
};

enum class ChannelBandwidth { 
	is160MHz, is80MHz, is40MHz, is20MHz, _auto_
};

enum class ApMeshMode { 
	MESH_POINT, MESH_PORTAL, STANDALONE
};

enum class SessionExpiryType { 
	unlimited, time_limited
};

enum class PasspointNaiRealmEapAuthInnerNonEap { 
	NAI_REALM_INNER_NON_EAP_MSCHAPV2, NAI_REALM_INNER_NON_EAP_MSCHAP, NAI_REALM_INNER_NON_EAP_CHAP, 
	NAI_REALM_INNER_NON_EAP_PAP
};

enum class RadioType { 
	is5GHzL, is5GHzU, is2dot4GHz, is5GHz
};

enum class StateUpDownError { 
	error, enabled, disabled
};

enum class EthernetLinkState { 
	UP10_HALF_DUPLEX, UP10_FULL_DUPLEX, UP100_HALF_DUPLEX, UP100_FULL_DUPLEX, UP1000_HALF_DUPLEX, 
	UP1000_FULL_DUPLEX, DOWN
};

enum class OriginatorType { 
	NET, SWITCH, AP
};

enum class SipCallStopReason { 
	UNSUPPORTED, DROPPED, BYE_OK
};

enum class MulticastRate { 
	rate54mbps, rate48mbps, rate36mbps, rate24mbps, rate18mbps, rate12mbps, rate9mbps, rate6mbps, _auto_
};

enum class EquipmentUpgradeState { 
	reboot_failed, up_to_date, out_of_date, rebooting, reboot_initiated, apply_complete, apply_failed, 
	applying, apply_initiated, download_complete, download_failed, downloading, download_initiated, 
	undefined
};

enum class SyslogSeverityType { 
	DEBUG, INFO, NOTICE, WARING, ERR, CRIT, ALERT, EMERG
};

enum class PasspointNaiRealmEncoding { 
	utf8_non_ietf_rfc_4282_encoding, ietf_rfc_4282_encoding
};

enum class StatusDataType { 
	CUSTOMER_DASHBOARD, CLIENT_DETAILS, ACTIVE_BSSIDS, RADIO_UTILIZATION, NEIGHBOUR_SCAN, OS_PERFORMANCE, 
	NEIGHBOURINGINFO, LANINFO, PEERINFO, FIRMWARE, PROTOCOL, NETWORK_AGGREGATE, NETWORK_ADMIN, EQUIPMENT_ADMIN
};

enum class ManagementRate { 
	rate24mbps, rate18mbps, rate12mbps, rate11mbps, rate9mbps, rate6mbps, rate5dot5mbps, rate2mbps, rate1mbps, 
	_auto_
};

enum class WepAuthType { 
	shared, open
};

enum class AlarmCode { 
	DNSServerLatency, DNSServerUnreachable, QoEIssues5g, QoEIssues2g, ClientAuthFailure, AssocFailure, 
	RadioHung, GenericError, InconsistentBasemacs, ChannelsOutOfSync5gv, ChannelsOutOfSync2g, InTroubleshootMode, 
	NeighbourScanStuckOn5g, NeighbourScanStuckOn2g, TooManyRogueAPs, TooManyBlockedDevices, DeauthAttackDetected, 
	DisabledSSID, FailedCPAuthentications, ConfigurationOutOfSync, RadioHung5G, RadioHung2G, MultipleAPCsOnSameSubnet, 
	FirmwareUpgradeStuck, RadiusConfigurationFailed, RebootRequestFailed, TooManyClients5g, TooManyClients2g, 
	HardwareIssueDiagnostic, CountryCodeMisMatch, LowMemoryReboot, CPUTemperature, Disconnected, 
	MemoryUtilization, CPUUtilization, CloudLinkLatency, CloudLink, RadiusLatency, Radius, DHCPLatency, 
	DHCP, DNSLatency, DNS, ChannelUtilization5G, NoiseFloor5G, ChannelUtilization2G, NoiseFloor2G, 
	NoMetricsReceived, AccessPointIsUnreachable, LimitedCloudConnectivity
};

enum class DetectedAuthMode { 
	UNKNOWN, WPA, WEP, OPEN
};

enum class DeploymentType { 
	CEILING, DESK
};

enum class BackgroundRepeat { 
	cover, no_repeat, round, space, repeat, repeat_y, repeat_x
};

enum class BackgroundPosition { 
	center_bottom, center_center, center_top, right_bottom, right_center, right_top, left_bottom, left_center, 
	left_top
};

enum class PasspointEapMethods { 
	eap_aka, eap_mschap_v2, eap_aka_authentication, eap_ttls, eap_tls
};

enum class McsType { 
	MCS_AC_4x4_9, MCS_AC_4x4_8, MCS_AC_4x4_7, MCS_AC_4x4_6, MCS_AC_4x4_5, MCS_AC_4x4_4, MCS_AC_4x4_3, 
	MCS_AC_4x4_2, MCS_AC_4x4_1, MCS_AC_4x4_0, MCS_N_31, MCS_N_30, MCS_N_29, MCS_N_28, MCS_N_27, MCS_N_26, 
	MCS_N_25, MCS_N_24, MCS_N_23, MCS_N_22, MCS_N_21, MCS_N_20, MCS_N_19, MCS_N_18, MCS_N_17, MCS_N_16, MCS_AC_3x3_9, 
	MCS_AC_3x3_8, MCS_AC_3x3_7, MCS_AC_3x3_6, MCS_AC_3x3_5, MCS_AC_3x3_4, MCS_AC_3x3_3, MCS_AC_3x3_2, 
	MCS_AC_3x3_1, MCS_AC_3x3_0, MCS_AC_2x2_9, MCS_AC_2x2_8, MCS_AC_2x2_7, MCS_AC_2x2_6, MCS_AC_2x2_5, 
	MCS_AC_2x2_4, MCS_AC_2x2_3, MCS_AC_2x2_2, MCS_AC_2x2_1, MCS_AC_2x2_0, MCS_AC_1x1_9, MCS_AC_1x1_8, 
	MCS_AC_1x1_7, MCS_AC_1x1_6, MCS_AC_1x1_5, MCS_AC_1x1_4, MCS_AC_1x1_3, MCS_AC_1x1_2, MCS_AC_1x1_1, 
	MCS_AC_1x1_0, MCS_N_15, MCS_N_14, MCS_N_13, MCS_N_12, MCS_N_11, MCS_N_10, MCS_N_9, MCS_N_8, MCS_N_7, 
	MCS_N_6, MCS_N_5, MCS_N_4, MCS_N_3, MCS_N_2, MCS_N_1, MCS_N_0, MCS_54, MCS_48, MCS_36, MCS_24, MCS_18, MCS_12, 
	MCS_9, MCS_6, MCS_11__2_4GHz_only, MCS_5dot5__2_4GHz_only, MCS_2___2_4GHz_only, MCS_1___2_4GHz_only
};

enum class WlanStatusCode { 
	UNSUPPORTED, WLAN_STATUS_SAE_PK, WLAN_STATUS_SAE_HASH_TO_ELEMENT, WLAN_STATUS_UNKNOWN_PASSWORD_IDENTIFIER, 
	WLAN_STATUS_UNKNOWN_AUTHENTICATION_SERVER, WLAN_STATUS_FILS_AUTHENTICATION_FAILURE, WLAN_STATUS_AUTHORIZATION_DEENABLED, 
	WLAN_STATUS_RESTRICTION_FROM_AUTHORIZED_GDB, WLAN_STATUS_ENABLEMENT_DENIED, WLAN_STATUS_ASSOC_DENIED_NO_VHT, 
	WLAN_STATUS_DENIED_DUE_TO_SPECTRUM_MANAGEMENT, WLAN_STATUS_MCCA_TRACK_LIMIT_EXCEEDED, 
	WLAN_STATUS_MAF_LIMIT_EXCEEDED, WLAN_STATUS_MCCAOP_RESERVATION_CONFLICT, WLAN_STATUS_DENIED_WITH_SUGGESTED_BAND_AND_CHANNEL, 
	WLAN_STATUS_TS_SCHEDULE_CONFLICT, WLAN_STATUS_TCLAS_PROCESSING_TERMINATED, WLAN_STATUS_REJECT_DSE_BAND, 
	WLAN_STATUS_QUERY_RESP_OUTSTANDING, WLAN_STATUS_REJECTED_EMERGENCY_SERVICE_NOT_SUPPORTED, 
	WLAN_STATUS_REFUSED_AP_OUT_OF_MEMORY, WLAN_STATUS_REFUSED_EXTERNAL_REASON, WLAN_STATUS_REJECT_U_PID_SETTING, 
	WLAN_STATUS_PENDING_GAP_IN_BA_WINDOW, WLAN_STATUS_PERFORMING_FST_NOW, WLAN_STATUS_PENDING_ADMITTING_FST_SESSION, 
	WLAN_STATUS_SUCCESS_POWER_SAVE_MODE, WLAN_STATUS_REJECT_NO_WAKEUP_SPECIFIED, WLAN_STATUS_REJECT_WITH_SCHEDULE, 
	WLAN_STATUS_REJECTED_WITH_SUGGESTED_BSS_TRANSITION, WLAN_STATUS_TCLAS_RESOURCES_EXCHAUSTED, 
	WLAN_STATUS_REQ_TCLAS_NOT_SUPPORTED, WLAN_STATUS_TRANSMISSION_FAILURE, WLAN_STATUS_CANNOT_FIND_ALT_TBTT, 
	WLAN_STATUS_FINITE_CYCLIC_GROUP_NOT_SUPPORTED, WLAN_STATUS_ANTI_CLOGGING_TOKEN_REQ, WLAN_STATUS_BAD_INTERVAL_WITH_U_APSD_COEX, 
	WLAN_STATUS_U_APSD_COEX_MODE_NOT_SUPPORTED, WLAN_STATUS_U_APSD_COEX_NOT_SUPPORTED, WLAN_STATUS_INVALID_RSNIE, 
	WLAN_STATUS_REQ_REFUSED_UNAUTH_ACCESS, WLAN_STATUS_REQ_REFUSED_SSPN, WLAN_STATUS_ADV_SRV_UNREACHABLE, 
	WLAN_STATUS_REQ_REFUSED_HOME, WLAN_STATUS_GAS_RESP_LARGER_THAN_LIMIT, WLAN_STATUS_STA_TIMED_OUT_WAITING_FOR_GAS_RESP, 
	WLAN_STATUS_GAS_RESP_NOT_RECEIVED, WLAN_STATUS_NO_OUTSTANDING_GAS_REQ, WLAN_STATUS_GAS_ADV_PROTO_NOT_SUPPORTED, 
	WLAN_STATUS_TRY_ANOTHER_BSS, WLAN_STATUS_INSUFFICIENT_TCLAS_PROCESSING_RESOURCES, WLAN_STATUS_REQUESTED_TCLAS_NOT_SUPPORTED, 
	WLAN_STATUS_INVALID_FTIE, WLAN_STATUS_INVALID_MDIE, WLAN_STATUS_INVALID_PMKID, WLAN_STATUS_INVALID_FT_ACTION_FRAME_COUNT, 
	WLAN_STATUS_ASSOC_DENIED_LISTEN_INT_TOO_LARGE, WLAN_STATUS_DEST_STA_NOT_QOS_STA, WLAN_STATUS_DEST_STA_NOT_PRESENT, 
	WLAN_STATUS_DIRECT_LINK_NOT_ALLOWED, WLAN_STATUS_TS_NOT_CREATED, WLAN_STATUS_CIPHER_REJECTED_PER_POLICY, 
	WLAN_STATUS_INVALID_RSN_IE_CAPAB, WLAN_STATUS_UNSUPPORTED_RSN_IE_VERSION, WLAN_STATUS_AKMP_NOT_VALID, 
	WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID, WLAN_STATUS_GROUP_CIPHER_NOT_VALID, WLAN_STATUS_INVALID_IE, 
	WLAN_STATUS_REJECTED_WITH_SUGGESTED_CHANGES, WLAN_STATUS_INVALID_PARAMETERS, WLAN_STATUS_REQUEST_DECLINED, 
	WLAN_STATUS_DENIED_QOS_NOT_SUPPORTED, WLAN_STATUS_DENIED_POOR_CHANNEL_CONDITIONS, WLAN_STATUS_DENIED_INSUFFICIENT_BANDWIDTH, 
	WLAN_STATUS_UNSPECIFIED_QOS_FAILURE, WLAN_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION, 
	WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY, WLAN_STATUS_ASSOC_DENIED_NO_PCO, WLAN_STATUS_R0KH_UNREACHABLE, 
	WLAN_STATUS_ASSOC_DENIED_NO_HT, WLAN_STATUS_ASSOC_DENIED_NO_SHORT_SLOT_TIME, WLAN_STATUS_SUPPORTED_CHANNEL_NOT_VALID, 
	WLAN_STATUS_PWR_CAPABILITY_NOT_VALID, WLAN_STATUS_SPEC_MGMT_REQUIRED, WLAN_STATUS_ASSOC_DENIED_NOSHORT, 
	WLAN_STATUS_ASSOC_DENIED_RATES, WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA, WLAN_STATUS_AUTH_TIMEOUT, 
	WLAN_STATUS_CHALLENGE_FAIL, WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION, WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG, 
	WLAN_STATUS_ASSOC_DENIED_UNSPEC, WLAN_STATUS_REASSOC_NO_ASSOC, WLAN_STATUS_CAPS_UNSUPPORTED, 
	WLAN_STATUS_NOT_IN_SAME_BSS, WLAN_STATUS_UNACCEPTABLE_LIFETIME, WLAN_STATUS_SECURITY_DISABLED, 
	WLAN_STATUS_TDLS_WAKEUP_REJECT, WLAN_STATUS_TDLS_WAKEUP_ALTERNATE, WLAN_STATUS_UNSPECIFIED_FAILURE, 
	WLAN_STATUS_SUCCESS
};

enum class ChannelHopReason { 
	UNSUPPORTED, HighInterference, RadarDetected
};

enum class PasspointIPv6AddressType { 
	availability_of_the_address_type_is_unknown, address_type_available, address_type_not_available
};

enum class EquipmentType { 
	SWITCH, AP
};

enum class PortalUserRole { 
	Public, TechSupport_RO, TechSupport, ManagedServiceProvider_RO, ManagedServiceProvider, Distributor_RO, 
	Distributor, CustomerIT_RO, CustomerIT, SuperUser_RO, SuperUser
};

enum class FirmwareValidationMethod { 
	NONE, MD5_CHECKSUM
};

enum class PasspointNaiRealmEapCredType { 
	NAI_REALM_CRED_TYPE_VENDOR_SPECIFIC, NAI_REALM_CRED_TYPE_ANONYMOUS, NAI_REALM_CRED_TYPE_NONE, 
	NAI_REALM_CRED_TYPE_USERNAME_PASSWORD, NAI_REALM_CRED_TYPE_CERTIFICATE, NAI_REALM_CRED_TYPE_SOFTOKEN, 
	NAI_REALM_CRED_TYPE_HARDWARE_TOKEN, NAI_REALM_CRED_TYPE_NFC_SECURE_ELEMENT, NAI_REALM_CRED_TYPE_USIM, 
	NAI_REALM_CRED_TYPE_SIM
};

enum class SIPCallReportReason { 
	UNSUPPORTED, GOT_PUBLISH, ROAMED_TO, ROAMED_FROM
};

enum class DisconnectFrameType { 
	UNSUPPORTED, Disassoc, Deauth
};

enum class DeviceMode { 
	gatewayOnly, gatewaywithAP, managedAP, standaloneAP
};

enum class RadiusAuthenticationMethod { 
	MSCHAPv2, CHAP, PAP
};

enum class PasspointIPv4AddressType { 
	availability_of_the_address_type_is_unknown, port_restricted_IPv4_address_and_double_NATed_IPv4_address_available, 
	port_restricted_IPv4_address_and_single_NATed_IPv4_address_available, double_NATed_private_IPv4_address_available, 
	single_NATed_private_IPv4_address_available, port_restricted_IPv4_address_available, 
	public_IPv4_address_available, address_type_not_available
};

enum class AntennaType { 
	OAP30_DIRECTIONAL, OMNI
};

enum class StateSetting { 
	disabled, enabled
};

enum class CaptivePortalAuthenticationType { 
	external, radius, username, guest
};

enum class RtpFlowType { 
	UNSUPPORTED, VIDEO, VOICE
};

enum class ClientTimeoutReason { 
	UNSUPPORTED, FailedProbe, IdleTooLong
};

enum class ChannelUtilizationSurveyType { 
	FULL, OFF_CHANNEL, ON_CHANNEL
};

enum class NetworkForwardMode { 
	NAT, BRIDGE
};

enum class CountryCode { 
	UNSUPPORTED, ZW, ZM, ZA, YT, YE, WS, WF, VU, VN, VI, VG, VE, VC, VA, UZ, UY, US, UM, UG, UA, TZ, TW, TV, TT, TR, TO, TN, TM, TL, TK, TJ, TH, TG, TF, TD, 
	TC, SZ, SY, SX, SV, ST, SS, SR, SO, SN, SM, SL, SK, SJ, SI, SH, SG, SE, SD, SC, SB, SA, RW, RU, RS, RO, RE, QA, PY, PW, PT, PS, PR, PN, PM, PL, PK, PH, PG, PF, PE, 
	PA, OM, NZ, NU, NR, NP, NO, NL, NI, NG, NF, NE, NC, NA, MZ, MY, MX, MW, MV, MU, MT, MS, MR, MQ, MP, MO, MN, MM, ML, MK, MH, MG, MF, ME, MD, MC, MA, LY, LV, LU, LT, 
	LS, LR, LK, LI, LC, LB, LA, KZ, KY, KW, KR, KP, KN, KM, KI, KH, KG, KE, JP, JO, JM, JE, IT, IS, IR, IQ, IO, IN, IM, IL, IE, ID, HU, HT, HR, HN, HM, HK, GY, GW, GU, 
	GT, GS, GR, GQ, GP, GN, GM, GL, GI, GH, GG, GF, GE, GD, GB, GA, FR, FO, FM, FK, FJ, FI, ET, ES, ER, EH, EG, EE, EC, DZ, DO, DM, DK, DJ, DE, CZ, CY, CX, CW, CV, CU, 
	CR, CO, CN, CM, CL, CK, CI, CH, CG, CF, CD, CC, CA, BZ, BY, BW, BV, BT, BS, BR, BQ, BO, BN, BM, BL, BJ, BI, BH, BG, BF, BE, BD, BB, BA, AZ, AX, AW, AU, AT, AS, AR, 
	AQ, AO, AM, AL, AI, AG, AF, AE, AD
};

enum class PasspointConnectionCapabilitiesIpProtocol { 
	UDP, TCP, ICMP
};

enum class WepType { 
	wep128, wep64
};

enum class SsidSecureMode { 
	wpa3MixedEAP, wpa3OnlyEAP, wpa3MixedSAE, wpa3OnlySAE, wpa2OnlyEAP, wpa2EAP, wpaEAP, wep, wpa2OnlyRadius, 
	wpa2OnlyPSK, wpa2Radius, wpaRadius, wpa2PSK, wpaPSK, open
};

enum class RadioMode { 
	modeAB, modeG, modeB, modeA, modeAX, modeGN, modeAC, modeN
};

enum class WlanReasonCode { 
	UNSUPPORTED, WLAN_REASON_MESH_CHANNEL_SWITCH_UNSPECIFIED, WLAN_REASON_MESH_CHANNEL_SWITCH_REGULATORY_REQ, 
	WLAN_REASON_MAC_ADDRESS_ALREADY_EXISTS_IN_MBSS, WLAN_REASON_MESH_PATH_ERROR_DEST_UNREACHABLE, 
	WLAN_REASON_MESH_PATH_ERROR_NO_FORWARDING_INFO, WLAN_REASON_MESH_PATH_ERROR_NO_PROXY_INFO, 
	WLAN_REASON_MESH_INVALID_SECURITY_CAP, WLAN_REASON_MESH_INCONSISTENT_PARAMS, WLAN_REASON_MESH_INVALID_GTK, 
	WLAN_REASON_MESH_CONFIRM_TIMEOUT, WLAN_REASON_MESH_MAX_RETRIES, WLAN_REASON_MESH_CLOSE_RCVD, 
	WLAN_REASON_MESH_CONFIG_POLICY_VIOLATION, WLAN_REASON_MESH_MAX_PEERS, WLAN_REASON_MESH_PEERING_CANCELLED, 
	WLAN_REASON_INVALID_FTE, WLAN_REASON_INVALID_MDE, WLAN_REASON_INVALID_PMKID, WLAN_REASON_INVALID_FT_ACTION_FRAME_COUNT, 
	WLAN_REASON_EXTERNAL_SERVICE_REQUIREMENTS, WLAN_REASON_AUTHORIZED_ACCESS_LIMIT_REACHED, 
	WLAN_REASON_PEERKEY_MISMATCH, WLAN_REASON_TIMEOUT, WLAN_REASON_UNKNOWN_TS_BA, WLAN_REASON_END_TS_BA_DLS, 
	WLAN_REASON_STA_LEAVING, WLAN_REASON_EXCEEDED_TXOP, WLAN_REASON_DISASSOC_LOW_ACK, WLAN_REASON_NOT_ENOUGH_BANDWIDTH, 
	WLAN_REASON_UNSPECIFIED_QOS_REASON, WLAN_REASON_SERVICE_CHANGE_PRECLUDES_TS, WLAN_REASON_NOT_AUTHORIZED_THIS_LOCATION, 
	WLAN_REASON_BAD_CIPHER_OR_AKM, WLAN_REASON_NO_SSP_ROAMING_AGREEMENT, WLAN_REASON_SSP_REQUESTED_DISASSOC, 
	WLAN_REASON_TDLS_TEARDOWN_UNSPECIFIED, WLAN_REASON_TDLS_TEARDOWN_UNREACHABLE, WLAN_REASON_CIPHER_SUITE_REJECTED, 
	WLAN_REASON_IEEE_802_1X_AUTH_FAILED, WLAN_REASON_INVALID_RSN_IE_CAPAB, WLAN_REASON_UNSUPPORTED_RSN_IE_VERSION, 
	WLAN_REASON_AKMP_NOT_VALID, WLAN_REASON_PAIRWISE_CIPHER_NOT_VALID, WLAN_REASON_GROUP_CIPHER_NOT_VALID, 
	WLAN_REASON_IE_IN_4WAY_DIFFERS, WLAN_REASON_GROUP_KEY_UPDATE_TIMEOUT, WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT, 
	WLAN_REASON_MICHAEL_MIC_FAILURE, WLAN_REASON_INVALID_IE, WLAN_REASON_BSS_TRANSITION_DISASSOC, 
	WLAN_REASON_SUPPORTED_CHANNEL_NOT_VALID, WLAN_REASON_PWR_CAPABILITY_NOT_VALID, WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH, 
	WLAN_REASON_DISASSOC_STA_HAS_LEFT, WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA, WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA, 
	WLAN_REASON_DISASSOC_AP_BUSY, WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY, WLAN_REASON_DEAUTH_LEAVING, 
	WLAN_REASON_PREV_AUTH_NOT_VALID, WLAN_REASON_UNSPECIFIED
};

enum class PasspointConnectionCapabilitiesStatus { 
	unknown, open, closed
};

enum class PasspointGasAddress3Behaviour { 
	forceNonCompliantBehaviourFromRequest, ieee80211StandardCompliantOnly, p2pSpecWorkaroundFromRequest
};

enum class EquipmentProtocolState { 
	error_processing_configuration, error_when_joining, ready, configuration_received, joined, 
	init
};

enum class FileType { 
	PROTOBUF, JPG, PNG
};

enum class SteerType { 
	steer_perimeter, steer_11v, steer_deauth, steer_rsvd
};

enum class GuardInterval { 
	SGI__Short_Guard_Interval, LGI__Long_Guard_Interval
};

enum class TrackFlag { 
	DEFAULT, NEVER, ALWAYS
};

enum class ServiceMetricDataType { 
	Neighbour, Channel, Client, ApSsid, ApNode
};

enum class SecurityType { 
	SAE, PSK, RADIUS, OPEN
};

enum class ProfileType { 
	passpoint_osu_id_provider, passpoint_venue, passpoint_operator, passpoint, rf, metrics, mesh, captive_portal, 
	radius, bonjour, ssid, equipment_switch, equipment_ap
};

enum class StatsReportFormat { 
	AVERAGE, PERCENTILE, RAW
};

enum class PasspointNaiRealmEapAuthParam { 
	NAI_REALM_EAP_AUTH_VENDOR_SPECIFIC, NAI_REALM_EAP_AUTH_TUNNELED_CRED_TYPE, NAI_REALM_EAP_AUTH_CRED_TYPE, 
	NAI_REALM_EAP_AUTH_EXPANDED_INNER_EAP_METHOD, NAI_REALM_EAP_AUTH_INNER_AUTH_EAP_METHOD, 
	NAI_REALM_EAP_AUTH_NON_EAP_INNER_AUTH, NAI_REALM_EAP_AUTH_EXPANDED_EAP_METHOD
};

enum class BestAPSteerType { 
	linkQualityOnly, loadBalanceOnly, both
};

enum class DynamicVlanMode { 
	enabled_reject_if_no_radius_dynamic_vlan, enabled, disabled
};

enum class RtpFlowDirection { 
	DOWNSTREAM, UPSTREAM
};

enum class PasspointNetworkAuthenticationType { 
	dns_redirection, http_https_redirection, online_enrollment_supported, acceptance_of_terms_and_conditions
};

enum class DisconnectInitiator { 
	UNSUPPORTED, Client, AccessPoint
};

enum class WmmQueueType { 
	VO, VI, BK, BE
};

enum class GatewayType { 
	CNAGW, CEGW
};

enum class DayOfWeek { 
	SUNDAY, SATURDAY, FRIDAY, THURSDAY, WEDNESDAY, TUESDAY, MONDAY
};

enum class SourceType { 
	profile, manual, _auto_
};

enum class EquipmentUpgradeFailureReason { 
	downloadRequestFailedFlashFull, rebootTimedout, maxRetries, rebootWithWrongVersion, invalidVersion, 
	rebootRequestRejected, applyFailed, applyRequestRejected, downloadFailed, unreachableUrl, validationFailed, 
	downloadRequestRejected
};

enum class NetworkType { 
	ADHOC, AP
};

enum class SystemEventDataType { 
	UnserializableSystemEvent, RealTimeStreamingStopEvent, RealTimeStreamingStartSessionEvent, 
	RealTimeStreamingStartEvent, RealTimeSipCallStopEvent, RealTimeSipCallStartEvent, RealTimeSipCallReportEvent, 
	RealTimeSipCallEventWithStats, RealTimeChannelHopEvent, ClientIpAddressEvent, ClientFirstDataEvent, 
	ClientDisconnectEvent, ClientAuthEvent, ClientTimeoutEvent, ClientIdEvent, ClientFailureEvent, 
	ClientConnectSuccessEvent, ClientAssocEvent, DhcpRequestEvent, DhcpOfferEvent, DhcpInformEvent, 
	DhcpDiscoverEvent, DhcpDeclineEvent, DhcpNakEvent, DhcpAckEvent, StatusRemovedEvent, StatusChangedEvent, 
	EquipmentRemovedEvent, EquipmentChangedEvent, EquipmentAddedEvent, ClientSessionRemovedEvent, 
	ClientSessionChangedEvent, ClientSessionAddedEvent, AlarmRemovedEvent, AlarmChangedEvent, 
	AlarmAddedEvent, RoutingRemovedEvent, RoutingChangedEvent, RoutingAddedEvent, ProfileRemovedEvent, 
	ProfileChangedEvent, ProfileAddedEvent, PortalUserRemovedEvent, PortalUserChangedEvent, PortalUserAddedEvent, 
	LocationRemovedEvent, LocationChangedEvent, LocationAddedEvent, FirmwareRemovedEvent, FirmwareChangedEvent, 
	FirmwareAddedEvent, CustomerRemovedEvent, CustomerChangedEvent, CustomerAddedEvent, ClientRemovedEvent, 
	ClientChangedEvent, ClientAddedEvent, GatewayRemovedEvent, GatewayChangedEvent, GatewayAddedEvent
};




struct  FirmwareScheduleSetting {
	DailyTimeRangeSchedule *m_DailyTimeRangeSchedule;
	EmptySchedule *m_EmptySchedule;
	std::string  model_type{"m_discriminator"};
	std::string  m_type;
};


struct  ProfileDetailsChildren {
	RfConfiguration *m_RfConfiguration;
	ServiceMetricsCollectionConfigProfile *m_ServiceMetricsCollectionConfigProfile;
	SsidConfiguration *m_SsidConfiguration;
	PasspointOperatorProfile *m_PasspointOperatorProfile;
	CaptivePortalConfiguration *m_CaptivePortalConfiguration;
	RadiusProfile *m_RadiusProfile;
	MeshGroup *m_MeshGroup;
	BonjourGatewayProfile *m_BonjourGatewayProfile;
	ApNetworkConfiguration *m_ApNetworkConfiguration;
	PasspointOsuProviderProfile *m_PasspointOsuProviderProfile;
	PasspointProfile *m_PasspointProfile;
	PasspointVenueProfile *m_PasspointVenueProfile;
	std::string  model_type{"m_discriminator"};
	std::string  m_type;
};


struct  ScheduleSetting {
	DailyTimeRangeSchedule *m_DailyTimeRangeSchedule;
	DaysOfWeekTimeRangeSchedule *m_DaysOfWeekTimeRangeSchedule;
	EmptySchedule *m_EmptySchedule;
	std::string  model_type{"m_discriminator"};
	std::string  m_type;
};


struct  ServiceMetricDetails {
	ClientMetrics *m_ClientMetrics;
	ApNodeMetrics *m_ApNodeMetrics;
	ApSsidMetrics *m_ApSsidMetrics;
	ChannelInfoReports *m_ChannelInfoReports;
	NeighbourScanReports *m_NeighbourScanReports;
	std::string  model_type{"m_discriminator"};
	std::string  m_type;
};


struct  StatusDetails {
	EquipmentUpgradeStatusData *m_EquipmentUpgradeStatusData;
	NetworkAdminStatusData *m_NetworkAdminStatusData;
	ActiveBSSIDs *m_ActiveBSSIDs;
	RadioUtilizationReport *m_RadioUtilizationReport;
	NetworkAggregateStatusData *m_NetworkAggregateStatusData;
	EquipmentNeighbouringStatusData *m_EquipmentNeighbouringStatusData;
	EquipmentAdminStatusData *m_EquipmentAdminStatusData;
	EquipmentPeerStatusData *m_EquipmentPeerStatusData;
	CustomerPortalDashboardStatus *m_CustomerPortalDashboardStatus;
	OperatingSystemPerformance *m_OperatingSystemPerformance;
	EquipmentProtocolStatusData *m_EquipmentProtocolStatusData;
	EquipmentScanDetails *m_EquipmentScanDetails;
	EquipmentLANStatusData *m_EquipmentLANStatusData;
	ClientConnectionDetails *m_ClientConnectionDetails;
	std::string  model_type{"m_discriminator"};
	std::string  m_type;
};


struct  SystemEvent {
	RealTimeSipCallEventWithStats *m_RealTimeSipCallEventWithStats;
	ClientTimeoutEvent *m_ClientTimeoutEvent;
	DhcpAckEvent *m_DhcpAckEvent;
	StatusRemovedEvent *m_StatusRemovedEvent;
	ClientDisconnectEvent *m_ClientDisconnectEvent;
	DhcpNakEvent *m_DhcpNakEvent;
	ClientRemovedEvent *m_ClientRemovedEvent;
	ClientAddedEvent *m_ClientAddedEvent;
	CustomerRemovedEvent *m_CustomerRemovedEvent;
	CustomerAddedEvent *m_CustomerAddedEvent;
	GatewayChangedEvent *m_GatewayChangedEvent;
	ClientFailureEvent *m_ClientFailureEvent;
	AlarmChangedEvent *m_AlarmChangedEvent;
	EquipmentChangedEvent *m_EquipmentChangedEvent;
	RoutingChangedEvent *m_RoutingChangedEvent;
	ClientSessionChangedEvent *m_ClientSessionChangedEvent;
	ProfileRemovedEvent *m_ProfileRemovedEvent;
	ProfileAddedEvent *m_ProfileAddedEvent;
	PortalUserChangedEvent *m_PortalUserChangedEvent;
	LocationChangedEvent *m_LocationChangedEvent;
	ClientIpAddressEvent *m_ClientIpAddressEvent;
	DhcpDiscoverEvent *m_DhcpDiscoverEvent;
	StatusChangedEvent *m_StatusChangedEvent;
	RealTimeSipCallStopEvent *m_RealTimeSipCallStopEvent;
	RealTimeSipCallStartEvent *m_RealTimeSipCallStartEvent;
	ClientFirstDataEvent *m_ClientFirstDataEvent;
	ClientAuthEvent *m_ClientAuthEvent;
	DhcpOfferEvent *m_DhcpOfferEvent;
	ClientChangedEvent *m_ClientChangedEvent;
	ClientIdEvent *m_ClientIdEvent;
	CustomerChangedEvent *m_CustomerChangedEvent;
	GatewayRemovedEvent *m_GatewayRemovedEvent;
	GatewayAddedEvent *m_GatewayAddedEvent;
	UnserializableSystemEvent *m_UnserializableSystemEvent;
	RealTimeStreamingStartSessionEvent *m_RealTimeStreamingStartSessionEvent;
	RealTimeSipCallReportEvent *m_RealTimeSipCallReportEvent;
	AlarmRemovedEvent *m_AlarmRemovedEvent;
	AlarmAddedEvent *m_AlarmAddedEvent;
	ClientConnectSuccessEvent *m_ClientConnectSuccessEvent;
	DhcpInformEvent *m_DhcpInformEvent;
	EquipmentRemovedEvent *m_EquipmentRemovedEvent;
	EquipmentAddedEvent *m_EquipmentAddedEvent;
	RoutingRemovedEvent *m_RoutingRemovedEvent;
	RoutingAddedEvent *m_RoutingAddedEvent;
	ClientSessionRemovedEvent *m_ClientSessionRemovedEvent;
	ClientAssocEvent *m_ClientAssocEvent;
	DhcpRequestEvent *m_DhcpRequestEvent;
	RealTimeStreamingStopEvent *m_RealTimeStreamingStopEvent;
	RealTimeStreamingStartEvent *m_RealTimeStreamingStartEvent;
	RealtimeChannelHopEvent *m_RealtimeChannelHopEvent;
	DhcpDeclineEvent *m_DhcpDeclineEvent;
	ProfileChangedEvent *m_ProfileChangedEvent;
	PortalUserRemovedEvent *m_PortalUserRemovedEvent;
	PortalUserAddedEvent *m_PortalUserAddedEvent;
	LocationRemovedEvent *m_LocationRemovedEvent;
	LocationAddedEvent *m_LocationAddedEvent;
	std::string  model_type{"m_discriminator"};
	std::string  m_type;
};

class PasspointOsuIcon  {
	public:
		const std::string get_ICON_TYPE() const { return m_ICON_TYPE;}
		void set_ICON_TYPE(const std::string & value) { m_ICON_TYPE = value; }
		const std::string get_filePath() const { return m_filePath;}
		void set_filePath(const std::string & value) { m_filePath = value; }
		const int64_t get_iconHeight() const { return m_iconHeight;}
		void set_iconHeight(const int64_t & value) { m_iconHeight = value; }
		const std::string get_iconLocale() const { return m_iconLocale;}
		void set_iconLocale(const std::string & value) { m_iconLocale = value; }
		const std::string get_iconName() const { return m_iconName;}
		void set_iconName(const std::string & value) { m_iconName = value; }
		const int64_t get_iconWidth() const { return m_iconWidth;}
		void set_iconWidth(const int64_t & value) { m_iconWidth = value; }
		const std::string get_imageUrl() const { return m_imageUrl;}
		void set_imageUrl(const std::string & value) { m_imageUrl = value; }
		const std::string get_languageCode() const { return m_languageCode;}
		void set_languageCode(const std::string & value) { m_languageCode = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_ICON_TYPE;
		std::string	m_filePath;
		int64_t	m_iconHeight;
		std::string	m_iconLocale; /* The primary locale for this Icon. */
		std::string	m_iconName;
		int64_t	m_iconWidth;
		std::string	m_imageUrl;
		std::string	m_languageCode;
};

class ClientEapDetails  {
	public:
		const int64_t get_eapKey1Timestamp() const { return m_eapKey1Timestamp;}
		void set_eapKey1Timestamp(const int64_t & value) { m_eapKey1Timestamp = value; }
		const int64_t get_eapKey2Timestamp() const { return m_eapKey2Timestamp;}
		void set_eapKey2Timestamp(const int64_t & value) { m_eapKey2Timestamp = value; }
		const int64_t get_eapKey3Timestamp() const { return m_eapKey3Timestamp;}
		void set_eapKey3Timestamp(const int64_t & value) { m_eapKey3Timestamp = value; }
		const int64_t get_eapKey4Timestamp() const { return m_eapKey4Timestamp;}
		void set_eapKey4Timestamp(const int64_t & value) { m_eapKey4Timestamp = value; }
		const int64_t get_eapNegotiationStartTimestamp() const { return m_eapNegotiationStartTimestamp;}
		void set_eapNegotiationStartTimestamp(const int64_t & value) { m_eapNegotiationStartTimestamp = value; }
		const int64_t get_eapSuccessTimestamp() const { return m_eapSuccessTimestamp;}
		void set_eapSuccessTimestamp(const int64_t & value) { m_eapSuccessTimestamp = value; }
		const int64_t get_requestIdentityTimestamp() const { return m_requestIdentityTimestamp;}
		void set_requestIdentityTimestamp(const int64_t & value) { m_requestIdentityTimestamp = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_eapKey1Timestamp;
		int64_t	m_eapKey2Timestamp;
		int64_t	m_eapKey3Timestamp;
		int64_t	m_eapKey4Timestamp;
		int64_t	m_eapNegotiationStartTimestamp;
		int64_t	m_eapSuccessTimestamp;
		int64_t	m_requestIdentityTimestamp;
};

class ProfileDetails  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type; /* BonjourGatewayProfile, CaptivePortalConfiguration, ApNetworkConfiguration, MeshGroup, RadiusProfile, SsidConfiguration, RfConfiguration, PasspointOsuProviderProfile, PasspointProfile, PasspointOperatorProfile, PasspointVenueProfile, ServiceMetricsCollectionConfigProfile,  */
};

class ActiveScanSettings  {
	public:
		const bool get_enabled() const { return m_enabled;}
		void set_enabled(const bool & value) { m_enabled = value; }
		const int64_t get_scanDurationMillis() const { return m_scanDurationMillis;}
		void set_scanDurationMillis(const int64_t & value) { m_scanDurationMillis = value; }
		const int64_t get_scanFrequencySeconds() const { return m_scanFrequencySeconds;}
		void set_scanFrequencySeconds(const int64_t & value) { m_scanFrequencySeconds = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_enabled;
		int64_t	m_scanDurationMillis;
		int64_t	m_scanFrequencySeconds;
};

class SourceSelectionManagement  {
	public:
		const SourceType get_source() const { return m_source;}
		void set_source(const SourceType & value) { m_source = value; }
		const ManagementRate get_value() const { return m_value;}
		void set_value(const ManagementRate & value) { m_value = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		SourceType	m_source;
		ManagementRate	m_value;
};

class AclTemplate  {
	public:
		const bool get_Delete() const { return m_Delete;}
		void set_Delete(const bool & value) { m_Delete = value; }
		const bool get_PortalLogin() const { return m_PortalLogin;}
		void set_PortalLogin(const bool & value) { m_PortalLogin = value; }
		const bool get_Read() const { return m_Read;}
		void set_Read(const bool & value) { m_Read = value; }
		const bool get_ReadWrite() const { return m_ReadWrite;}
		void set_ReadWrite(const bool & value) { m_ReadWrite = value; }
		const bool get_ReadWriteCreate() const { return m_ReadWriteCreate;}
		void set_ReadWriteCreate(const bool & value) { m_ReadWriteCreate = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_Delete;
		bool	m_PortalLogin;
		bool	m_Read;
		bool	m_ReadWrite;
		bool	m_ReadWriteCreate;
};

class LocalTimeValue  {
	public:
		const int64_t get_hour() const { return m_hour;}
		void set_hour(const int64_t & value) { m_hour = value; }
		const int64_t get_minute() const { return m_minute;}
		void set_minute(const int64_t & value) { m_minute = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_hour;
		int64_t	m_minute;
};

class SortColumnsClient  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName;
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class IntegerValueMap  {
	public:
		const int64_t get_IntegerValueMap() const { return m_IntegerValueMap;}
		void set_IntegerValueMap(const int64_t & value) { m_IntegerValueMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_IntegerValueMap;
};

class OperatingSystemPerformance  {
	public:
		const std::vector<float> get_avgCpuPerCore() const { return m_avgCpuPerCore;}
		void set_avgCpuPerCore(const std::vector<float> & value) { m_avgCpuPerCore = value; }
		const float get_avgCpuTemperature() const { return m_avgCpuTemperature;}
		void set_avgCpuTemperature(const float & value) { m_avgCpuTemperature = value; }
		const float get_avgCpuUtilization() const { return m_avgCpuUtilization;}
		void set_avgCpuUtilization(const float & value) { m_avgCpuUtilization = value; }
		const int64_t get_avgFreeMemoryKb() const { return m_avgFreeMemoryKb;}
		void set_avgFreeMemoryKb(const int64_t & value) { m_avgFreeMemoryKb = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_numCamiCrashes() const { return m_numCamiCrashes;}
		void set_numCamiCrashes(const int64_t & value) { m_numCamiCrashes = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }
		const int64_t get_totalAvailableMemoryKb() const { return m_totalAvailableMemoryKb;}
		void set_totalAvailableMemoryKb(const int64_t & value) { m_totalAvailableMemoryKb = value; }
		const int64_t get_uptimeInSeconds() const { return m_uptimeInSeconds;}
		void set_uptimeInSeconds(const int64_t & value) { m_uptimeInSeconds = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<float>	m_avgCpuPerCore;
		float	m_avgCpuTemperature;
		float	m_avgCpuUtilization;
		int64_t	m_avgFreeMemoryKb;
		std::string	m_model_type;
		int64_t	m_numCamiCrashes;
		std::string	m_statusDataType;
		int64_t	m_totalAvailableMemoryKb;
		int64_t	m_uptimeInSeconds;
};

class RadioUtilizationPerRadioDetails  {
	public:
		const int64_t get_avgAssocClientRx() const { return m_avgAssocClientRx;}
		void set_avgAssocClientRx(const int64_t & value) { m_avgAssocClientRx = value; }
		const int64_t get_avgAssocClientTx() const { return m_avgAssocClientTx;}
		void set_avgAssocClientTx(const int64_t & value) { m_avgAssocClientTx = value; }
		const int64_t get_avgNonWifi() const { return m_avgNonWifi;}
		void set_avgNonWifi(const int64_t & value) { m_avgNonWifi = value; }
		const int64_t get_avgUnassocClientRx() const { return m_avgUnassocClientRx;}
		void set_avgUnassocClientRx(const int64_t & value) { m_avgUnassocClientRx = value; }
		const int64_t get_avgUnassocClientTx() const { return m_avgUnassocClientTx;}
		void set_avgUnassocClientTx(const int64_t & value) { m_avgUnassocClientTx = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_avgAssocClientRx;
		int64_t	m_avgAssocClientTx;
		int64_t	m_avgNonWifi;
		int64_t	m_avgUnassocClientRx;
		int64_t	m_avgUnassocClientTx;
};

class DnsProbeMetric  {
	public:
		const int64_t get_dnsLatencyMs() const { return m_dnsLatencyMs;}
		void set_dnsLatencyMs(const int64_t & value) { m_dnsLatencyMs = value; }
		const std::string get_dnsServerIp() const { return m_dnsServerIp;}
		void set_dnsServerIp(const std::string & value) { m_dnsServerIp = value; }
		const StateUpDownError get_dnsState() const { return m_dnsState;}
		void set_dnsState(const StateUpDownError & value) { m_dnsState = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_dnsLatencyMs;
		std::string	m_dnsServerIp;
		StateUpDownError	m_dnsState;
};

class FirmwareVersion  {
	public:
		const std::string get_commit() const { return m_commit;}
		void set_commit(const std::string & value) { m_commit = value; }
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const std::string get_description() const { return m_description;}
		void set_description(const std::string & value) { m_description = value; }
		const EquipmentType get_equipmentType() const { return m_equipmentType;}
		void set_equipmentType(const EquipmentType & value) { m_equipmentType = value; }
		const std::string get_filename() const { return m_filename;}
		void set_filename(const std::string & value) { m_filename = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const std::string get_modelId() const { return m_modelId;}
		void set_modelId(const std::string & value) { m_modelId = value; }
		const int64_t get_releaseDate() const { return m_releaseDate;}
		void set_releaseDate(const int64_t & value) { m_releaseDate = value; }
		const std::string get_validationCode() const { return m_validationCode;}
		void set_validationCode(const std::string & value) { m_validationCode = value; }
		const FirmwareValidationMethod get_validationMethod() const { return m_validationMethod;}
		void set_validationMethod(const FirmwareValidationMethod & value) { m_validationMethod = value; }
		const std::string get_versionName() const { return m_versionName;}
		void set_versionName(const std::string & value) { m_versionName = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_commit; /* commit number for the firmware image, from the source control system */
		int64_t	m_createdTimestamp;
		std::string	m_description;
		EquipmentType	m_equipmentType;
		std::string	m_filename;
		int64_t	m_id;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		std::string	m_modelId; /* equipment model */
		int64_t	m_releaseDate; /* release date of the firmware image, in ms epoch time */
		std::string	m_validationCode; /* firmware digest code, depending on validation method - MD5, etc. */
		FirmwareValidationMethod	m_validationMethod;
		std::string	m_versionName;
};

class PasspointMccMnc  {
	public:
		const std::string get_country() const { return m_country;}
		void set_country(const std::string & value) { m_country = value; }
		const int64_t get_countryCode() const { return m_countryCode;}
		void set_countryCode(const int64_t & value) { m_countryCode = value; }
		const std::string get_iso() const { return m_iso;}
		void set_iso(const std::string & value) { m_iso = value; }
		const int64_t get_mcc() const { return m_mcc;}
		void set_mcc(const int64_t & value) { m_mcc = value; }
		const int64_t get_mnc() const { return m_mnc;}
		void set_mnc(const int64_t & value) { m_mnc = value; }
		const std::string get_network() const { return m_network;}
		void set_network(const std::string & value) { m_network = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_country;
		int64_t	m_countryCode;
		std::string	m_iso;
		int64_t	m_mcc;
		int64_t	m_mnc;
		std::string	m_network;
};

class EquipmentGatewayRecord  {
	public:
		const int64_t get_createdTimeStamp() const { return m_createdTimeStamp;}
		void set_createdTimeStamp(const int64_t & value) { m_createdTimeStamp = value; }
		const GatewayType get_gatewayType() const { return m_gatewayType;}
		void set_gatewayType(const GatewayType & value) { m_gatewayType = value; }
		const std::string get_hostname() const { return m_hostname;}
		void set_hostname(const std::string & value) { m_hostname = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const std::string get_ipAddr() const { return m_ipAddr;}
		void set_ipAddr(const std::string & value) { m_ipAddr = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const int64_t get_port() const { return m_port;}
		void set_port(const int64_t & value) { m_port = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimeStamp;
		GatewayType	m_gatewayType;
		std::string	m_hostname;
		int64_t	m_id;
		std::string	m_ipAddr;
		int64_t	m_lastModifiedTimestamp;
		int64_t	m_port;
};

class VLANStatusData  {
	public:
		const std::string get_dhcpServer() const { return m_dhcpServer;}
		void set_dhcpServer(const std::string & value) { m_dhcpServer = value; }
		const std::string get_dnsServer1() const { return m_dnsServer1;}
		void set_dnsServer1(const std::string & value) { m_dnsServer1 = value; }
		const std::string get_dnsServer2() const { return m_dnsServer2;}
		void set_dnsServer2(const std::string & value) { m_dnsServer2 = value; }
		const std::string get_dnsServer3() const { return m_dnsServer3;}
		void set_dnsServer3(const std::string & value) { m_dnsServer3 = value; }
		const std::string get_gateway() const { return m_gateway;}
		void set_gateway(const std::string & value) { m_gateway = value; }
		const std::string get_ipBase() const { return m_ipBase;}
		void set_ipBase(const std::string & value) { m_ipBase = value; }
		const std::string get_subnetMask() const { return m_subnetMask;}
		void set_subnetMask(const std::string & value) { m_subnetMask = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_dhcpServer;
		std::string	m_dnsServer1;
		std::string	m_dnsServer2;
		std::string	m_dnsServer3;
		std::string	m_gateway;
		std::string	m_ipBase;
		std::string	m_subnetMask;
};

class EmptySchedule  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_timezone() const { return m_timezone;}
		void set_timezone(const std::string & value) { m_timezone = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		std::string	m_timezone;
};

class CountsPerAlarmCodeMap  {
	public:
		const int64_t get_CountsPerAlarmCodeMap() const { return m_CountsPerAlarmCodeMap;}
		void set_CountsPerAlarmCodeMap(const int64_t & value) { m_CountsPerAlarmCodeMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_CountsPerAlarmCodeMap;
};

class MacAddress  {
	public:
		const std::string get_addressAsString() const { return m_addressAsString;}
		void set_addressAsString(const std::string & value) { m_addressAsString = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_addressAsString;
		std::string	m_model_type;
};

class ClientDhcpDetails  {
	public:
		const std::string get_dhcpServerIp() const { return m_dhcpServerIp;}
		void set_dhcpServerIp(const std::string & value) { m_dhcpServerIp = value; }
		const int64_t get_firstDiscoverTimestamp() const { return m_firstDiscoverTimestamp;}
		void set_firstDiscoverTimestamp(const int64_t & value) { m_firstDiscoverTimestamp = value; }
		const int64_t get_firstOfferTimestamp() const { return m_firstOfferTimestamp;}
		void set_firstOfferTimestamp(const int64_t & value) { m_firstOfferTimestamp = value; }
		const int64_t get_firstRequestTimestamp() const { return m_firstRequestTimestamp;}
		void set_firstRequestTimestamp(const int64_t & value) { m_firstRequestTimestamp = value; }
		const bool get_fromInternal() const { return m_fromInternal;}
		void set_fromInternal(const bool & value) { m_fromInternal = value; }
		const std::string get_gatewayIp() const { return m_gatewayIp;}
		void set_gatewayIp(const std::string & value) { m_gatewayIp = value; }
		const int64_t get_leaseStartTimestamp() const { return m_leaseStartTimestamp;}
		void set_leaseStartTimestamp(const int64_t & value) { m_leaseStartTimestamp = value; }
		const int64_t get_leaseTimeInSeconds() const { return m_leaseTimeInSeconds;}
		void set_leaseTimeInSeconds(const int64_t & value) { m_leaseTimeInSeconds = value; }
		const int64_t get_nakTimestamp() const { return m_nakTimestamp;}
		void set_nakTimestamp(const int64_t & value) { m_nakTimestamp = value; }
		const std::string get_primaryDns() const { return m_primaryDns;}
		void set_primaryDns(const std::string & value) { m_primaryDns = value; }
		const std::string get_secondaryDns() const { return m_secondaryDns;}
		void set_secondaryDns(const std::string & value) { m_secondaryDns = value; }
		const std::string get_subnetMask() const { return m_subnetMask;}
		void set_subnetMask(const std::string & value) { m_subnetMask = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_dhcpServerIp;
		int64_t	m_firstDiscoverTimestamp;
		int64_t	m_firstOfferTimestamp;
		int64_t	m_firstRequestTimestamp;
		bool	m_fromInternal;
		std::string	m_gatewayIp;
		int64_t	m_leaseStartTimestamp;
		int64_t	m_leaseTimeInSeconds;
		int64_t	m_nakTimestamp;
		std::string	m_primaryDns;
		std::string	m_secondaryDns;
		std::string	m_subnetMask;
};

class ManufacturerDetailsRecord  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const std::string get_manufacturerAlias() const { return m_manufacturerAlias;}
		void set_manufacturerAlias(const std::string & value) { m_manufacturerAlias = value; }
		const std::string get_manufacturerName() const { return m_manufacturerName;}
		void set_manufacturerName(const std::string & value) { m_manufacturerName = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_id;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		std::string	m_manufacturerAlias;
		std::string	m_manufacturerName;
};

class PeerInfo  {
	public:
		const std::string get_peerIP() const { return m_peerIP;}
		void set_peerIP(const std::string & value) { m_peerIP = value; }
		const std::vector<int64_t> get_peerMAC() const { return m_peerMAC;}
		void set_peerMAC(const std::vector<int64_t> & value) { m_peerMAC = value; }
		const std::string get_radiusSecret() const { return m_radiusSecret;}
		void set_radiusSecret(const std::string & value) { m_radiusSecret = value; }
		const TunnelIndicator get_tunnel() const { return m_tunnel;}
		void set_tunnel(const TunnelIndicator & value) { m_tunnel = value; }
		const std::vector<int64_t> get_vlans() const { return m_vlans;}
		void set_vlans(const std::vector<int64_t> & value) { m_vlans = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_peerIP;
		std::vector<int64_t>	m_peerMAC;
		std::string	m_radiusSecret;
		TunnelIndicator	m_tunnel;
		std::vector<int64_t>	m_vlans;
};

class SortColumnsEquipment  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName; /* id, name, profileId, locationId, equipmentType, inventoryId,  */
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class StreamingVideoServerRecord  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const std::string get_ipAddr() const { return m_ipAddr;}
		void set_ipAddr(const std::string & value) { m_ipAddr = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const StreamingVideoType get_type() const { return m_type;}
		void set_type(const StreamingVideoType & value) { m_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_id;
		std::string	m_ipAddr;
		int64_t	m_lastModifiedTimestamp;
		StreamingVideoType	m_type; /* string representing InetAddress */
};

class TunnelMetricData  {
	public:
		const bool get_activeTun() const { return m_activeTun;}
		void set_activeTun(const bool & value) { m_activeTun = value; }
		const int64_t get_cfgTime() const { return m_cfgTime;}
		void set_cfgTime(const int64_t & value) { m_cfgTime = value; }
		const std::string get_ipAddr() const { return m_ipAddr;}
		void set_ipAddr(const std::string & value) { m_ipAddr = value; }
		const int64_t get_pingsRecvd() const { return m_pingsRecvd;}
		void set_pingsRecvd(const int64_t & value) { m_pingsRecvd = value; }
		const int64_t get_pingsSent() const { return m_pingsSent;}
		void set_pingsSent(const int64_t & value) { m_pingsSent = value; }
		const int64_t get_upTime() const { return m_upTime;}
		void set_upTime(const int64_t & value) { m_upTime = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_activeTun; /* Indicates if the current tunnel is the active one */
		int64_t	m_cfgTime; /* number of seconds tunnel was configured */
		std::string	m_ipAddr; /* IP address of tunnel peer */
		int64_t	m_pingsRecvd; /* number of 'ping' response received by peer in the current bin in case tunnel was DOWN */
		int64_t	m_pingsSent; /* number of 'ping' sent in the current bin in case tunnel was DOWN */
		int64_t	m_upTime; /* number of seconds tunnel was up in current bin */
};

class BonjourServiceSet  {
	public:
		const std::vector<std::string> get_serviceNames() const { return m_serviceNames;}
		void set_serviceNames(const std::vector<std::string> & value) { m_serviceNames = value; }
		const bool get_supportAllServices() const { return m_supportAllServices;}
		void set_supportAllServices(const bool & value) { m_supportAllServices = value; }
		const int64_t get_vlanId() const { return m_vlanId;}
		void set_vlanId(const int64_t & value) { m_vlanId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<std::string>	m_serviceNames;
		bool	m_supportAllServices;
		int64_t	m_vlanId;
};

class AutoOrManualString  {
	public:
		const bool get__auto_() const { return m__auto_;}
		void set__auto_(const bool & value) { m__auto_ = value; }
		const std::string get_value() const { return m_value;}
		void set_value(const std::string & value) { m_value = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m__auto_;
		std::string	m_value;
};

class VlanSubnet  {
	public:
		const std::string get_subnetBase() const { return m_subnetBase;}
		void set_subnetBase(const std::string & value) { m_subnetBase = value; }
		const std::string get_subnetDhcpServer() const { return m_subnetDhcpServer;}
		void set_subnetDhcpServer(const std::string & value) { m_subnetDhcpServer = value; }
		const std::string get_subnetDns1() const { return m_subnetDns1;}
		void set_subnetDns1(const std::string & value) { m_subnetDns1 = value; }
		const std::string get_subnetDns2() const { return m_subnetDns2;}
		void set_subnetDns2(const std::string & value) { m_subnetDns2 = value; }
		const std::string get_subnetDns3() const { return m_subnetDns3;}
		void set_subnetDns3(const std::string & value) { m_subnetDns3 = value; }
		const std::string get_subnetGateway() const { return m_subnetGateway;}
		void set_subnetGateway(const std::string & value) { m_subnetGateway = value; }
		const std::string get_subnetMask() const { return m_subnetMask;}
		void set_subnetMask(const std::string & value) { m_subnetMask = value; }
		const int64_t get_subnetVlan() const { return m_subnetVlan;}
		void set_subnetVlan(const int64_t & value) { m_subnetVlan = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_subnetBase; /* string representing InetAddress */
		std::string	m_subnetDhcpServer; /* string representing InetAddress */
		std::string	m_subnetDns1; /* string representing InetAddress */
		std::string	m_subnetDns2; /* string representing InetAddress */
		std::string	m_subnetDns3; /* string representing InetAddress */
		std::string	m_subnetGateway; /* string representing InetAddress */
		std::string	m_subnetMask; /* string representing InetAddress */
		int64_t	m_subnetVlan;
};

class SortColumnsLocation  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName; /* id, name,  */
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class PasspointNaiRealmInformation  {
	public:
		const std::string get_eapMap() const { return m_eapMap;}
		void set_eapMap(const std::string & value) { m_eapMap = value; }
		const std::vector<PasspointEapMethods> get_eapMethods() const { return m_eapMethods;}
		void set_eapMethods(const std::vector<PasspointEapMethods> & value) { m_eapMethods = value; }
		const PasspointNaiRealmEncoding get_encoding() const { return m_encoding;}
		void set_encoding(const PasspointNaiRealmEncoding & value) { m_encoding = value; }
		const std::vector<std::string> get_naiRealms() const { return m_naiRealms;}
		void set_naiRealms(const std::vector<std::string> & value) { m_naiRealms = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_eapMap;
		std::vector<PasspointEapMethods>	m_eapMethods; /* array of EAP methods */
		PasspointNaiRealmEncoding	m_encoding; /* value of PasspointNaiRealmEncoding */
		std::vector<std::string>	m_naiRealms;
};

class PasspointDuple  {
	public:
		const std::string get_defaultDupleSeparator() const { return m_defaultDupleSeparator;}
		void set_defaultDupleSeparator(const std::string & value) { m_defaultDupleSeparator = value; }
		const std::string get_dupleIso3Language() const { return m_dupleIso3Language;}
		void set_dupleIso3Language(const std::string & value) { m_dupleIso3Language = value; }
		const std::string get_dupleName() const { return m_dupleName;}
		void set_dupleName(const std::string & value) { m_dupleName = value; }
		const std::string get_locale() const { return m_locale;}
		void set_locale(const std::string & value) { m_locale = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_defaultDupleSeparator; /* default separator between the values of a duple, by default it is a ':' */
		std::string	m_dupleIso3Language; /* 3 letter iso language representation based on locale */
		std::string	m_dupleName;
		std::string	m_locale; /* The locale for this duple. */
};

class FirmwareTrackAssignmentRecord  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const bool get_defaultRevisionForTrack() const { return m_defaultRevisionForTrack;}
		void set_defaultRevisionForTrack(const bool & value) { m_defaultRevisionForTrack = value; }
		const bool get_deprecated() const { return m_deprecated;}
		void set_deprecated(const bool & value) { m_deprecated = value; }
		const int64_t get_firmwareVersionRecordId() const { return m_firmwareVersionRecordId;}
		void set_firmwareVersionRecordId(const int64_t & value) { m_firmwareVersionRecordId = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const int64_t get_trackRecordId() const { return m_trackRecordId;}
		void set_trackRecordId(const int64_t & value) { m_trackRecordId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		bool	m_defaultRevisionForTrack;
		bool	m_deprecated;
		int64_t	m_firmwareVersionRecordId;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		int64_t	m_trackRecordId;
};

class PaginationContextStatus  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class AlarmDetailsAttributesMap  {
	public:
		const std::string get_AlarmDetailsAttributesMap() const { return m_AlarmDetailsAttributesMap;}
		void set_AlarmDetailsAttributesMap(const std::string & value) { m_AlarmDetailsAttributesMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_AlarmDetailsAttributesMap; /* Additional Properties */
};

class ClientSessionMetricDetails  {
	public:
		const std::string get_classification() const { return m_classification;}
		void set_classification(const std::string & value) { m_classification = value; }
		const int64_t get_lastMetricTimestamp() const { return m_lastMetricTimestamp;}
		void set_lastMetricTimestamp(const int64_t & value) { m_lastMetricTimestamp = value; }
		const int64_t get_lastRxTimestamp() const { return m_lastRxTimestamp;}
		void set_lastRxTimestamp(const int64_t & value) { m_lastRxTimestamp = value; }
		const int64_t get_lastTxTimestamp() const { return m_lastTxTimestamp;}
		void set_lastTxTimestamp(const int64_t & value) { m_lastTxTimestamp = value; }
		const int64_t get_rssi() const { return m_rssi;}
		void set_rssi(const int64_t & value) { m_rssi = value; }
		const int64_t get_rxBytes() const { return m_rxBytes;}
		void set_rxBytes(const int64_t & value) { m_rxBytes = value; }
		const int64_t get_rxDataFrames() const { return m_rxDataFrames;}
		void set_rxDataFrames(const int64_t & value) { m_rxDataFrames = value; }
		const float get_rxMbps() const { return m_rxMbps;}
		void set_rxMbps(const float & value) { m_rxMbps = value; }
		const int64_t get_rxRateKbps() const { return m_rxRateKbps;}
		void set_rxRateKbps(const int64_t & value) { m_rxRateKbps = value; }
		const int64_t get_snr() const { return m_snr;}
		void set_snr(const int64_t & value) { m_snr = value; }
		const int64_t get_totalRxPackets() const { return m_totalRxPackets;}
		void set_totalRxPackets(const int64_t & value) { m_totalRxPackets = value; }
		const int64_t get_totalTxPackets() const { return m_totalTxPackets;}
		void set_totalTxPackets(const int64_t & value) { m_totalTxPackets = value; }
		const int64_t get_txBytes() const { return m_txBytes;}
		void set_txBytes(const int64_t & value) { m_txBytes = value; }
		const int64_t get_txDataFrames() const { return m_txDataFrames;}
		void set_txDataFrames(const int64_t & value) { m_txDataFrames = value; }
		const int64_t get_txDataFramesRetried() const { return m_txDataFramesRetried;}
		void set_txDataFramesRetried(const int64_t & value) { m_txDataFramesRetried = value; }
		const float get_txMbps() const { return m_txMbps;}
		void set_txMbps(const float & value) { m_txMbps = value; }
		const int64_t get_txRateKbps() const { return m_txRateKbps;}
		void set_txRateKbps(const int64_t & value) { m_txRateKbps = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_classification;
		int64_t	m_lastMetricTimestamp;
		int64_t	m_lastRxTimestamp;
		int64_t	m_lastTxTimestamp;
		int64_t	m_rssi;
		int64_t	m_rxBytes;
		int64_t	m_rxDataFrames; /* The number of dataframes transmitted FROM the client TO the AP. */
		float	m_rxMbps;
		int64_t	m_rxRateKbps;
		int64_t	m_snr;
		int64_t	m_totalRxPackets;
		int64_t	m_totalTxPackets;
		int64_t	m_txBytes;
		int64_t	m_txDataFrames; /* The number of dataframes transmitted TO the client from the AP. */
		int64_t	m_txDataFramesRetried; /* The number of data frames transmitted TO the client that were retried. Note this is not the same as the number of retries. */
		float	m_txMbps;
		int64_t	m_txRateKbps;
};

class RtpFlowStats  {
	public:
		const std::vector<Base64String> get_blockCodecs() const { return m_blockCodecs;}
		void set_blockCodecs(const std::vector<Base64String> & value) { m_blockCodecs = value; }
		const int64_t get_code() const { return m_code;}
		void set_code(const int64_t & value) { m_code = value; }
		const RtpFlowDirection get_direction() const { return m_direction;}
		void set_direction(const RtpFlowDirection & value) { m_direction = value; }
		const RtpFlowType get_flowType() const { return m_flowType;}
		void set_flowType(const RtpFlowType & value) { m_flowType = value; }
		const int64_t get_jitter() const { return m_jitter;}
		void set_jitter(const int64_t & value) { m_jitter = value; }
		const int64_t get_latency() const { return m_latency;}
		void set_latency(const int64_t & value) { m_latency = value; }
		const int64_t get_mosMultipliedBy100() const { return m_mosMultipliedBy100;}
		void set_mosMultipliedBy100(const int64_t & value) { m_mosMultipliedBy100 = value; }
		const int64_t get_packetLossConsecutive() const { return m_packetLossConsecutive;}
		void set_packetLossConsecutive(const int64_t & value) { m_packetLossConsecutive = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<Base64String>	m_blockCodecs;
		int64_t	m_code;
		RtpFlowDirection	m_direction;
		RtpFlowType	m_flowType;
		int64_t	m_jitter;
		int64_t	m_latency;
		int64_t	m_mosMultipliedBy100;
		int64_t	m_packetLossConsecutive;
};

class PaginationContextServiceMetric  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class ChannelPowerLevel  {
	public:
		const int64_t get_channelNumber() const { return m_channelNumber;}
		void set_channelNumber(const int64_t & value) { m_channelNumber = value; }
		const int64_t get_channelWidth() const { return m_channelWidth;}
		void set_channelWidth(const int64_t & value) { m_channelWidth = value; }
		const bool get_dfs() const { return m_dfs;}
		void set_dfs(const bool & value) { m_dfs = value; }
		const int64_t get_powerLevel() const { return m_powerLevel;}
		void set_powerLevel(const int64_t & value) { m_powerLevel = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_channelNumber;
		int64_t	m_channelWidth; /* Value is in MHz, -1 means AUTO */
		bool	m_dfs;
		int64_t	m_powerLevel;
};

class RadioUtilization  {
	public:
		const int64_t get_assocClientRx() const { return m_assocClientRx;}
		void set_assocClientRx(const int64_t & value) { m_assocClientRx = value; }
		const int64_t get_assocClientTx() const { return m_assocClientTx;}
		void set_assocClientTx(const int64_t & value) { m_assocClientTx = value; }
		const double get_ibss() const { return m_ibss;}
		void set_ibss(const double & value) { m_ibss = value; }
		const int64_t get_nonWifi() const { return m_nonWifi;}
		void set_nonWifi(const int64_t & value) { m_nonWifi = value; }
		const int64_t get_timestampSeconds() const { return m_timestampSeconds;}
		void set_timestampSeconds(const int64_t & value) { m_timestampSeconds = value; }
		const double get_unAvailableCapacity() const { return m_unAvailableCapacity;}
		void set_unAvailableCapacity(const double & value) { m_unAvailableCapacity = value; }
		const int64_t get_unassocClientRx() const { return m_unassocClientRx;}
		void set_unassocClientRx(const int64_t & value) { m_unassocClientRx = value; }
		const int64_t get_unassocClientTx() const { return m_unassocClientTx;}
		void set_unassocClientTx(const int64_t & value) { m_unassocClientTx = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_assocClientRx;
		int64_t	m_assocClientTx;
		double	m_ibss;
		int64_t	m_nonWifi;
		int64_t	m_timestampSeconds;
		double	m_unAvailableCapacity;
		int64_t	m_unassocClientRx;
		int64_t	m_unassocClientTx;
};

class PasspointVenueTypeAssignment  {
	public:
		const std::string get_venueDescription() const { return m_venueDescription;}
		void set_venueDescription(const std::string & value) { m_venueDescription = value; }
		const int64_t get_venueGroupId() const { return m_venueGroupId;}
		void set_venueGroupId(const int64_t & value) { m_venueGroupId = value; }
		const int64_t get_venueTypeId() const { return m_venueTypeId;}
		void set_venueTypeId(const int64_t & value) { m_venueTypeId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_venueDescription;
		int64_t	m_venueGroupId;
		int64_t	m_venueTypeId;
};

class Profile  {
	public:
		const std::vector<int64_t> get_childProfileIds() const { return m_childProfileIds;}
		void set_childProfileIds(const std::vector<int64_t> & value) { m_childProfileIds = value; }
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const ProfileDetailsChildren get_details() const { return m_details;}
		void set_details(const ProfileDetailsChildren & value) { m_details = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const std::string get_name() const { return m_name;}
		void set_name(const std::string & value) { m_name = value; }
		const ProfileType get_profileType() const { return m_profileType;}
		void set_profileType(const ProfileType & value) { m_profileType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<int64_t>	m_childProfileIds;
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		ProfileDetailsChildren	m_details;
		int64_t	m_id;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		std::string	m_name;
		ProfileType	m_profileType;
};

class RadioStatistics  {
	public:
		const int64_t get_numTxHT_216_6_Mbps() const { return m_numTxHT_216_6_Mbps;}
		void set_numTxHT_216_6_Mbps(const int64_t & value) { m_numTxHT_216_6_Mbps = value; }
		const int64_t get_numRxHT_325_Mbps() const { return m_numRxHT_325_Mbps;}
		void set_numRxHT_325_Mbps(const int64_t & value) { m_numRxHT_325_Mbps = value; }
		const int64_t get_numRxFcsErr() const { return m_numRxFcsErr;}
		void set_numRxFcsErr(const int64_t & value) { m_numRxFcsErr = value; }
		const int64_t get_numRxVHT_2340_Mbps() const { return m_numRxVHT_2340_Mbps;}
		void set_numRxVHT_2340_Mbps(const int64_t & value) { m_numRxVHT_2340_Mbps = value; }
		const int64_t get_numRxVHT_1733_1_Mbps() const { return m_numRxVHT_1733_1_Mbps;}
		void set_numRxVHT_1733_1_Mbps(const int64_t & value) { m_numRxVHT_1733_1_Mbps = value; }
		const int64_t get_numRxVHT_702_Mbps() const { return m_numRxVHT_702_Mbps;}
		void set_numRxVHT_702_Mbps(const int64_t & value) { m_numRxVHT_702_Mbps = value; }
		const int64_t get_numRxVHT_433_2_Mbps() const { return m_numRxVHT_433_2_Mbps;}
		void set_numRxVHT_433_2_Mbps(const int64_t & value) { m_numRxVHT_433_2_Mbps = value; }
		const int64_t get_numTxHT_6_5_Mbps() const { return m_numTxHT_6_5_Mbps;}
		void set_numTxHT_6_5_Mbps(const int64_t & value) { m_numTxHT_6_5_Mbps = value; }
		const int64_t get_numRxTimeData() const { return m_numRxTimeData;}
		void set_numRxTimeData(const int64_t & value) { m_numRxTimeData = value; }
		const int64_t get_numRxHT_351_2_Mbps() const { return m_numRxHT_351_2_Mbps;}
		void set_numRxHT_351_2_Mbps(const int64_t & value) { m_numRxHT_351_2_Mbps = value; }
		const int64_t get_numRxHT_216_Mbps() const { return m_numRxHT_216_Mbps;}
		void set_numRxHT_216_Mbps(const int64_t & value) { m_numRxHT_216_Mbps = value; }
		const int64_t get_numRxVHT_405_Mbps() const { return m_numRxVHT_405_Mbps;}
		void set_numRxVHT_405_Mbps(const int64_t & value) { m_numRxVHT_405_Mbps = value; }
		const int64_t get_numRxHT_360_Mbps() const { return m_numRxHT_360_Mbps;}
		void set_numRxHT_360_Mbps(const int64_t & value) { m_numRxHT_360_Mbps = value; }
		const int64_t get_numRxHT_120_Mbps() const { return m_numRxHT_120_Mbps;}
		void set_numRxHT_120_Mbps(const int64_t & value) { m_numRxHT_120_Mbps = value; }
		const int64_t get_numTxVHT_702_Mbps() const { return m_numTxVHT_702_Mbps;}
		void set_numTxVHT_702_Mbps(const int64_t & value) { m_numTxVHT_702_Mbps = value; }
		const int64_t get_numTxVHT_936_Mbps() const { return m_numTxVHT_936_Mbps;}
		void set_numTxVHT_936_Mbps(const int64_t & value) { m_numTxVHT_936_Mbps = value; }
		const int64_t get_numTxVHT_390_Mbps() const { return m_numTxVHT_390_Mbps;}
		void set_numTxVHT_390_Mbps(const int64_t & value) { m_numTxVHT_390_Mbps = value; }
		const int64_t get_numRxDropEthHdrRunt() const { return m_numRxDropEthHdrRunt;}
		void set_numRxDropEthHdrRunt(const int64_t & value) { m_numRxDropEthHdrRunt = value; }
		const int64_t get_numTxDataFrames_1300Plus_Mbps() const { return m_numTxDataFrames_1300Plus_Mbps;}
		void set_numTxDataFrames_1300Plus_Mbps(const int64_t & value) { m_numTxDataFrames_1300Plus_Mbps = value; }
		const int64_t get_numRxHT_150_Mbps() const { return m_numRxHT_150_Mbps;}
		void set_numRxHT_150_Mbps(const int64_t & value) { m_numRxHT_150_Mbps = value; }
		const int64_t get_numTx_18_Mbps() const { return m_numTx_18_Mbps;}
		void set_numTx_18_Mbps(const int64_t & value) { m_numTx_18_Mbps = value; }
		const int64_t get_numTxVHT_1755_Mbps() const { return m_numTxVHT_1755_Mbps;}
		void set_numTxVHT_1755_Mbps(const int64_t & value) { m_numTxVHT_1755_Mbps = value; }
		const int64_t get_numTxVHT_364_5_Mbps() const { return m_numTxVHT_364_5_Mbps;}
		void set_numTxVHT_364_5_Mbps(const int64_t & value) { m_numTxVHT_364_5_Mbps = value; }
		const int64_t get_numRxNullData() const { return m_numRxNullData;}
		void set_numRxNullData(const int64_t & value) { m_numRxNullData = value; }
		const int64_t get_numTxHT_43_2_Mbps() const { return m_numTxHT_43_2_Mbps;}
		void set_numTxHT_43_2_Mbps(const int64_t & value) { m_numTxHT_43_2_Mbps = value; }
		const int64_t get_numTxTotalAttemps() const { return m_numTxTotalAttemps;}
		void set_numTxTotalAttemps(const int64_t & value) { m_numTxTotalAttemps = value; }
		const int64_t get_rxDataBytes() const { return m_rxDataBytes;}
		void set_rxDataBytes(const int64_t & value) { m_rxDataBytes = value; }
		const int64_t get_numRxHT_39_Mbps() const { return m_numRxHT_39_Mbps;}
		void set_numRxHT_39_Mbps(const int64_t & value) { m_numRxHT_39_Mbps = value; }
		const int64_t get_numTxHT_81_Mbps() const { return m_numTxHT_81_Mbps;}
		void set_numTxHT_81_Mbps(const int64_t & value) { m_numTxHT_81_Mbps = value; }
		const int64_t get_numTxHT_39_Mbps() const { return m_numTxHT_39_Mbps;}
		void set_numTxHT_39_Mbps(const int64_t & value) { m_numTxHT_39_Mbps = value; }
		const int64_t get_numRxHT_104_Mbps() const { return m_numRxHT_104_Mbps;}
		void set_numRxHT_104_Mbps(const int64_t & value) { m_numRxHT_104_Mbps = value; }
		const int64_t get_numRxVHT_432_Mbps() const { return m_numRxVHT_432_Mbps;}
		void set_numRxVHT_432_Mbps(const int64_t & value) { m_numRxVHT_432_Mbps = value; }
		const int64_t get_numTxVHT_486_Mbps() const { return m_numTxVHT_486_Mbps;}
		void set_numTxVHT_486_Mbps(const int64_t & value) { m_numTxVHT_486_Mbps = value; }
		const int64_t get_numRxHT_28_8_Mbps() const { return m_numRxHT_28_8_Mbps;}
		void set_numRxHT_28_8_Mbps(const int64_t & value) { m_numRxHT_28_8_Mbps = value; }
		const int64_t get_numTxDropped() const { return m_numTxDropped;}
		void set_numTxDropped(const int64_t & value) { m_numTxDropped = value; }
		const int64_t get_numTxVHT_600_Mbps() const { return m_numTxVHT_600_Mbps;}
		void set_numTxVHT_600_Mbps(const int64_t & value) { m_numTxVHT_600_Mbps = value; }
		const int64_t get_numRxVHT_648_Mbps() const { return m_numRxVHT_648_Mbps;}
		void set_numRxVHT_648_Mbps(const int64_t & value) { m_numRxVHT_648_Mbps = value; }
		const int64_t get_numTxVHT_720_Mbps() const { return m_numTxVHT_720_Mbps;}
		void set_numTxVHT_720_Mbps(const int64_t & value) { m_numTxVHT_720_Mbps = value; }
		const int64_t get_numRxFramesReceived() const { return m_numRxFramesReceived;}
		void set_numRxFramesReceived(const int64_t & value) { m_numRxFramesReceived = value; }
		const int64_t get_numRxHT_121_5_Mbps() const { return m_numRxHT_121_5_Mbps;}
		void set_numRxHT_121_5_Mbps(const int64_t & value) { m_numRxHT_121_5_Mbps = value; }
		const int64_t get_curChannel() const { return m_curChannel;}
		void set_curChannel(const int64_t & value) { m_curChannel = value; }
		const int64_t get_numTxVHT_403_Mbps() const { return m_numTxVHT_403_Mbps;}
		void set_numTxVHT_403_Mbps(const int64_t & value) { m_numTxVHT_403_Mbps = value; }
		const int64_t get_numRx_48_Mbps() const { return m_numRx_48_Mbps;}
		void set_numRx_48_Mbps(const int64_t & value) { m_numRx_48_Mbps = value; }
		const int64_t get_numTxAction() const { return m_numTxAction;}
		void set_numTxAction(const int64_t & value) { m_numTxAction = value; }
		const int64_t get_numRxBeacon() const { return m_numRxBeacon;}
		void set_numRxBeacon(const int64_t & value) { m_numRxBeacon = value; }
		const int64_t get_numRxVHT_2808_Mbps() const { return m_numRxVHT_2808_Mbps;}
		void set_numRxVHT_2808_Mbps(const int64_t & value) { m_numRxVHT_2808_Mbps = value; }
		const int64_t get_numTxSuccessWithRetry() const { return m_numTxSuccessWithRetry;}
		void set_numTxSuccessWithRetry(const int64_t & value) { m_numTxSuccessWithRetry = value; }
		const int64_t get_numTxHT_30_Mbps() const { return m_numTxHT_30_Mbps;}
		void set_numTxHT_30_Mbps(const int64_t & value) { m_numTxHT_30_Mbps = value; }
		const int64_t get_numRxHT_292_5_Mbps() const { return m_numRxHT_292_5_Mbps;}
		void set_numRxHT_292_5_Mbps(const int64_t & value) { m_numRxHT_292_5_Mbps = value; }
		const int64_t get_numRxAck() const { return m_numRxAck;}
		void set_numRxAck(const int64_t & value) { m_numRxAck = value; }
		const int64_t get_numRxDropInvalidSrcMac() const { return m_numRxDropInvalidSrcMac;}
		void set_numRxDropInvalidSrcMac(const int64_t & value) { m_numRxDropInvalidSrcMac = value; }
		const int64_t get_numRxHT_19_5_Mbps() const { return m_numRxHT_19_5_Mbps;}
		void set_numRxHT_19_5_Mbps(const int64_t & value) { m_numRxHT_19_5_Mbps = value; }
		const int64_t get_numTxHT_173_3_Mbps() const { return m_numTxHT_173_3_Mbps;}
		void set_numTxHT_173_3_Mbps(const int64_t & value) { m_numTxHT_173_3_Mbps = value; }
		const int64_t get_numTxHT_240_Mbps() const { return m_numTxHT_240_Mbps;}
		void set_numTxHT_240_Mbps(const int64_t & value) { m_numTxHT_240_Mbps = value; }
		const int64_t get_numTxEapol() const { return m_numTxEapol;}
		void set_numTxEapol(const int64_t & value) { m_numTxEapol = value; }
		const int64_t get_numRxAmsduDeaggSeq() const { return m_numRxAmsduDeaggSeq;}
		void set_numRxAmsduDeaggSeq(const int64_t & value) { m_numRxAmsduDeaggSeq = value; }
		const int64_t get_numRxHT_288_7_Mbps() const { return m_numRxHT_288_7_Mbps;}
		void set_numRxHT_288_7_Mbps(const int64_t & value) { m_numRxHT_288_7_Mbps = value; }
		const int64_t get_numTxManagement() const { return m_numTxManagement;}
		void set_numTxManagement(const int64_t & value) { m_numTxManagement = value; }
		const int64_t get_numTxBeaconFail() const { return m_numTxBeaconFail;}
		void set_numTxBeaconFail(const int64_t & value) { m_numTxBeaconFail = value; }
		const int64_t get_numRxVHT_1755_Mbps() const { return m_numRxVHT_1755_Mbps;}
		void set_numRxVHT_1755_Mbps(const int64_t & value) { m_numRxVHT_1755_Mbps = value; }
		const int64_t get_numRxPspoll() const { return m_numRxPspoll;}
		void set_numRxPspoll(const int64_t & value) { m_numRxPspoll = value; }
		const int64_t get_numTxBcDropped() const { return m_numTxBcDropped;}
		void set_numTxBcDropped(const int64_t & value) { m_numTxBcDropped = value; }
		const int64_t get_numTxVHT_433_2_Mbps() const { return m_numTxVHT_433_2_Mbps;}
		void set_numTxVHT_433_2_Mbps(const int64_t & value) { m_numTxVHT_433_2_Mbps = value; }
		const int64_t get_numTxVHT_585_Mbps() const { return m_numTxVHT_585_Mbps;}
		void set_numTxVHT_585_Mbps(const int64_t & value) { m_numTxVHT_585_Mbps = value; }
		const int64_t get_numTxHT_86_8_Mbps() const { return m_numTxHT_86_8_Mbps;}
		void set_numTxHT_86_8_Mbps(const int64_t & value) { m_numTxHT_86_8_Mbps = value; }
		const int64_t get_numTxHT_52_Mbps() const { return m_numTxHT_52_Mbps;}
		void set_numTxHT_52_Mbps(const int64_t & value) { m_numTxHT_52_Mbps = value; }
		const int64_t get_numRxVHT_292_5_Mbps() const { return m_numRxVHT_292_5_Mbps;}
		void set_numRxVHT_292_5_Mbps(const int64_t & value) { m_numRxVHT_292_5_Mbps = value; }
		const int64_t get_numRxHT_60_Mbps() const { return m_numRxHT_60_Mbps;}
		void set_numRxHT_60_Mbps(const int64_t & value) { m_numRxHT_60_Mbps = value; }
		const int64_t get_numRxTimeToMe() const { return m_numRxTimeToMe;}
		void set_numRxTimeToMe(const int64_t & value) { m_numRxTimeToMe = value; }
		const int64_t get_numRxOffChan() const { return m_numRxOffChan;}
		void set_numRxOffChan(const int64_t & value) { m_numRxOffChan = value; }
		const int64_t get_numTxBeaconSuFail() const { return m_numTxBeaconSuFail;}
		void set_numTxBeaconSuFail(const int64_t & value) { m_numTxBeaconSuFail = value; }
		const int64_t get_numTxHT_28_7_Mbps() const { return m_numTxHT_28_7_Mbps;}
		void set_numTxHT_28_7_Mbps(const int64_t & value) { m_numTxHT_28_7_Mbps = value; }
		const int64_t get_numTxHT_108_Mbps() const { return m_numTxHT_108_Mbps;}
		void set_numTxHT_108_Mbps(const int64_t & value) { m_numTxHT_108_Mbps = value; }
		const int64_t get_numRxHT_156_Mbps() const { return m_numRxHT_156_Mbps;}
		void set_numRxHT_156_Mbps(const int64_t & value) { m_numRxHT_156_Mbps = value; }
		const int64_t get_numRxVHT_468_Mbps() const { return m_numRxVHT_468_Mbps;}
		void set_numRxVHT_468_Mbps(const int64_t & value) { m_numRxVHT_468_Mbps = value; }
		const int64_t get_numTxVHT_2808_Mbps() const { return m_numTxVHT_2808_Mbps;}
		void set_numTxVHT_2808_Mbps(const int64_t & value) { m_numTxVHT_2808_Mbps = value; }
		const std::vector<int64_t> get_actualCellSize() const { return m_actualCellSize;}
		void set_actualCellSize(const std::vector<int64_t> & value) { m_actualCellSize = value; }
		const int64_t get_numTxVHT_526_5_Mbps() const { return m_numTxVHT_526_5_Mbps;}
		void set_numTxVHT_526_5_Mbps(const int64_t & value) { m_numTxVHT_526_5_Mbps = value; }
		const int64_t get_numTxHT_324_Mbps() const { return m_numTxHT_324_Mbps;}
		void set_numTxHT_324_Mbps(const int64_t & value) { m_numTxHT_324_Mbps = value; }
		const int64_t get_numTxVHT_866_7_Mbps() const { return m_numTxVHT_866_7_Mbps;}
		void set_numTxVHT_866_7_Mbps(const int64_t & value) { m_numTxVHT_866_7_Mbps = value; }
		const int64_t get_numRxHT_72_1_Mbps() const { return m_numRxHT_72_1_Mbps;}
		void set_numRxHT_72_1_Mbps(const int64_t & value) { m_numRxHT_72_1_Mbps = value; }
		const int64_t get_numRxHT_65_Mbps() const { return m_numRxHT_65_Mbps;}
		void set_numRxHT_65_Mbps(const int64_t & value) { m_numRxHT_65_Mbps = value; }
		const int64_t get_numTxDataTransmitted() const { return m_numTxDataTransmitted;}
		void set_numTxDataTransmitted(const int64_t & value) { m_numTxDataTransmitted = value; }
		const int64_t get_numRx_18_Mbps() const { return m_numRx_18_Mbps;}
		void set_numRx_18_Mbps(const int64_t & value) { m_numRx_18_Mbps = value; }
		const int64_t get_numRxDataFrames_108_Mbps() const { return m_numRxDataFrames_108_Mbps;}
		void set_numRxDataFrames_108_Mbps(const int64_t & value) { m_numRxDataFrames_108_Mbps = value; }
		const int64_t get_numRxVHT_1053_Mbps() const { return m_numRxVHT_1053_Mbps;}
		void set_numRxVHT_1053_Mbps(const int64_t & value) { m_numRxVHT_1053_Mbps = value; }
		const int64_t get_numRxVHT_2106_Mbps() const { return m_numRxVHT_2106_Mbps;}
		void set_numRxVHT_2106_Mbps(const int64_t & value) { m_numRxVHT_2106_Mbps = value; }
		const int64_t get_numRxHT_288_8_Mbps() const { return m_numRxHT_288_8_Mbps;}
		void set_numRxHT_288_8_Mbps(const int64_t & value) { m_numRxHT_288_8_Mbps = value; }
		const int64_t get_numRxHT_40_5_Mbps() const { return m_numRxHT_40_5_Mbps;}
		void set_numRxHT_40_5_Mbps(const int64_t & value) { m_numRxHT_40_5_Mbps = value; }
		const int64_t get_numTxVHT_1170_Mbps() const { return m_numTxVHT_1170_Mbps;}
		void set_numTxVHT_1170_Mbps(const int64_t & value) { m_numTxVHT_1170_Mbps = value; }
		const int64_t get_numTxDataRetries() const { return m_numTxDataRetries;}
		void set_numTxDataRetries(const int64_t & value) { m_numTxDataRetries = value; }
		const int64_t get_numTxHT_117_Mbps() const { return m_numTxHT_117_Mbps;}
		void set_numTxHT_117_Mbps(const int64_t & value) { m_numTxHT_117_Mbps = value; }
		const int64_t get_numTxHT_13_Mbps() const { return m_numTxHT_13_Mbps;}
		void set_numTxHT_13_Mbps(const int64_t & value) { m_numTxHT_13_Mbps = value; }
		const int64_t get_numRxVHT_1872_Mbps() const { return m_numRxVHT_1872_Mbps;}
		void set_numRxVHT_1872_Mbps(const int64_t & value) { m_numRxVHT_1872_Mbps = value; }
		const int64_t get_numTxVHT_2340_Mbps() const { return m_numTxVHT_2340_Mbps;}
		void set_numTxVHT_2340_Mbps(const int64_t & value) { m_numTxVHT_2340_Mbps = value; }
		const int64_t get_numRxHT_216_6_Mbps() const { return m_numRxHT_216_6_Mbps;}
		void set_numRxHT_216_6_Mbps(const int64_t & value) { m_numRxHT_216_6_Mbps = value; }
		const int64_t get_numTxHT_54_Mbps() const { return m_numTxHT_54_Mbps;}
		void set_numTxHT_54_Mbps(const int64_t & value) { m_numTxHT_54_Mbps = value; }
		const int64_t get_numRx_9_Mbps() const { return m_numRx_9_Mbps;}
		void set_numRx_9_Mbps(const int64_t & value) { m_numRx_9_Mbps = value; }
		const int64_t get_numRxStbc() const { return m_numRxStbc;}
		void set_numRxStbc(const int64_t & value) { m_numRxStbc = value; }
		const int64_t get_numTxHT_270_Mbps() const { return m_numTxHT_270_Mbps;}
		void set_numTxHT_270_Mbps(const int64_t & value) { m_numTxHT_270_Mbps = value; }
		const int64_t get_numTxHT_288_8_Mbps() const { return m_numTxHT_288_8_Mbps;}
		void set_numTxHT_288_8_Mbps(const int64_t & value) { m_numTxHT_288_8_Mbps = value; }
		const int64_t get_numTxHT_351_2_Mbps() const { return m_numTxHT_351_2_Mbps;}
		void set_numTxHT_351_2_Mbps(const int64_t & value) { m_numTxHT_351_2_Mbps = value; }
		const int64_t get_numRxVHT_325_Mbps() const { return m_numRxVHT_325_Mbps;}
		void set_numRxVHT_325_Mbps(const int64_t & value) { m_numRxVHT_325_Mbps = value; }
		const int64_t get_numTxHT_135_Mbps() const { return m_numTxHT_135_Mbps;}
		void set_numTxHT_135_Mbps(const int64_t & value) { m_numTxHT_135_Mbps = value; }
		const int64_t get_numRxProbeReq() const { return m_numRxProbeReq;}
		void set_numRxProbeReq(const int64_t & value) { m_numRxProbeReq = value; }
		const int64_t get_numFreeTxBuf() const { return m_numFreeTxBuf;}
		void set_numFreeTxBuf(const int64_t & value) { m_numFreeTxBuf = value; }
		const int64_t get_numScanSucc() const { return m_numScanSucc;}
		void set_numScanSucc(const int64_t & value) { m_numScanSucc = value; }
		const int64_t get_numTxVHT_1040_Mbps() const { return m_numTxVHT_1040_Mbps;}
		void set_numTxVHT_1040_Mbps(const int64_t & value) { m_numTxVHT_1040_Mbps = value; }
		const int64_t get_numTx_12_Mbps() const { return m_numTx_12_Mbps;}
		void set_numTx_12_Mbps(const int64_t & value) { m_numTx_12_Mbps = value; }
		const int64_t get_numRxProbeResp() const { return m_numRxProbeResp;}
		void set_numRxProbeResp(const int64_t & value) { m_numRxProbeResp = value; }
		const int64_t get_numTxDtimMc() const { return m_numTxDtimMc;}
		void set_numTxDtimMc(const int64_t & value) { m_numTxDtimMc = value; }
		const int64_t get_numTxHT_14_3_Mbps() const { return m_numTxHT_14_3_Mbps;}
		void set_numTxHT_14_3_Mbps(const int64_t & value) { m_numTxHT_14_3_Mbps = value; }
		const int64_t get_numTxVHT_1300_Mbps() const { return m_numTxVHT_1300_Mbps;}
		void set_numTxVHT_1300_Mbps(const int64_t & value) { m_numTxVHT_1300_Mbps = value; }
		const int64_t get_numRxHT_13_Mbps() const { return m_numRxHT_13_Mbps;}
		void set_numRxHT_13_Mbps(const int64_t & value) { m_numRxHT_13_Mbps = value; }
		const int64_t get_numRxHT_117_Mbps() const { return m_numRxHT_117_Mbps;}
		void set_numRxHT_117_Mbps(const int64_t & value) { m_numRxHT_117_Mbps = value; }
		const int64_t get_numRxHT_28_7_Mbps() const { return m_numRxHT_28_7_Mbps;}
		void set_numRxHT_28_7_Mbps(const int64_t & value) { m_numRxHT_28_7_Mbps = value; }
		const int64_t get_numRxHT_195_Mbps() const { return m_numRxHT_195_Mbps;}
		void set_numRxHT_195_Mbps(const int64_t & value) { m_numRxHT_195_Mbps = value; }
		const int64_t get_numTxLdpc() const { return m_numTxLdpc;}
		void set_numTxLdpc(const int64_t & value) { m_numTxLdpc = value; }
		const int64_t get_numRxDropRunt() const { return m_numRxDropRunt;}
		void set_numRxDropRunt(const int64_t & value) { m_numRxDropRunt = value; }
		const int64_t get_numTxHT_263_2_Mbps() const { return m_numTxHT_263_2_Mbps;}
		void set_numTxHT_263_2_Mbps(const int64_t & value) { m_numTxHT_263_2_Mbps = value; }
		const int64_t get_numTxHT_13_5_Mbps() const { return m_numTxHT_13_5_Mbps;}
		void set_numTxHT_13_5_Mbps(const int64_t & value) { m_numTxHT_13_5_Mbps = value; }
		const int64_t get_numRxDropNoFcField() const { return m_numRxDropNoFcField;}
		void set_numRxDropNoFcField(const int64_t & value) { m_numRxDropNoFcField = value; }
		const int64_t get_numTxHT_87_8_Mbps() const { return m_numTxHT_87_8_Mbps;}
		void set_numTxHT_87_8_Mbps(const int64_t & value) { m_numTxHT_87_8_Mbps = value; }
		const int64_t get_numTxPowerChanges() const { return m_numTxPowerChanges;}
		void set_numTxPowerChanges(const int64_t & value) { m_numTxPowerChanges = value; }
		const int64_t get_numRxVHT_480_Mbps() const { return m_numRxVHT_480_Mbps;}
		void set_numRxVHT_480_Mbps(const int64_t & value) { m_numRxVHT_480_Mbps = value; }
		const int64_t get_numRxDataFramesRetried() const { return m_numRxDataFramesRetried;}
		void set_numRxDataFramesRetried(const int64_t & value) { m_numRxDataFramesRetried = value; }
		const int64_t get_numRxVHT_1040_Mbps() const { return m_numRxVHT_1040_Mbps;}
		void set_numRxVHT_1040_Mbps(const int64_t & value) { m_numRxVHT_1040_Mbps = value; }
		const int64_t get_numTxMultiRetries() const { return m_numTxMultiRetries;}
		void set_numTxMultiRetries(const int64_t & value) { m_numTxMultiRetries = value; }
		const int64_t get_numRxDup() const { return m_numRxDup;}
		void set_numRxDup(const int64_t & value) { m_numRxDup = value; }
		const int64_t get_numTxVHT_648_Mbps() const { return m_numTxVHT_648_Mbps;}
		void set_numTxVHT_648_Mbps(const int64_t & value) { m_numTxVHT_648_Mbps = value; }
		const int64_t get_numRxData() const { return m_numRxData;}
		void set_numRxData(const int64_t & value) { m_numRxData = value; }
		const int64_t get_numTxVHT_450_Mbps() const { return m_numTxVHT_450_Mbps;}
		void set_numTxVHT_450_Mbps(const int64_t & value) { m_numTxVHT_450_Mbps = value; }
		const int64_t get_numTxVHT_432_Mbps() const { return m_numTxVHT_432_Mbps;}
		void set_numTxVHT_432_Mbps(const int64_t & value) { m_numTxVHT_432_Mbps = value; }
		const int64_t get_numRxHT_26_Mbps() const { return m_numRxHT_26_Mbps;}
		void set_numRxHT_26_Mbps(const int64_t & value) { m_numRxHT_26_Mbps = value; }
		const int64_t get_numTxHT_115_5_Mbps() const { return m_numTxHT_115_5_Mbps;}
		void set_numTxHT_115_5_Mbps(const int64_t & value) { m_numTxHT_115_5_Mbps = value; }
		const int64_t get_numRxHT_200_Mbps() const { return m_numRxHT_200_Mbps;}
		void set_numRxHT_200_Mbps(const int64_t & value) { m_numRxHT_200_Mbps = value; }
		const int64_t get_numTxHT_86_6_Mbps() const { return m_numTxHT_86_6_Mbps;}
		void set_numTxHT_86_6_Mbps(const int64_t & value) { m_numTxHT_86_6_Mbps = value; }
		const int64_t get_numRx_54_Mbps() const { return m_numRx_54_Mbps;}
		void set_numRx_54_Mbps(const int64_t & value) { m_numRx_54_Mbps = value; }
		const int64_t get_numRxVHT_1950_Mbps() const { return m_numRxVHT_1950_Mbps;}
		void set_numRxVHT_1950_Mbps(const int64_t & value) { m_numRxVHT_1950_Mbps = value; }
		const int64_t get_numRxHT_130_3_Mbps() const { return m_numRxHT_130_3_Mbps;}
		void set_numRxHT_130_3_Mbps(const int64_t & value) { m_numRxHT_130_3_Mbps = value; }
		const int64_t get_numRxDataFrames() const { return m_numRxDataFrames;}
		void set_numRxDataFrames(const int64_t & value) { m_numRxDataFrames = value; }
		const int64_t get_numRxVHT_780_Mbps() const { return m_numRxVHT_780_Mbps;}
		void set_numRxVHT_780_Mbps(const int64_t & value) { m_numRxVHT_780_Mbps = value; }
		const int64_t get_numRxHT_54_Mbps() const { return m_numRxHT_54_Mbps;}
		void set_numRxHT_54_Mbps(const int64_t & value) { m_numRxHT_54_Mbps = value; }
		const int64_t get_numRxVHT_600_Mbps() const { return m_numRxVHT_600_Mbps;}
		void set_numRxVHT_600_Mbps(const int64_t & value) { m_numRxVHT_600_Mbps = value; }
		const int64_t get_numRxDataFrames_300_Mbps() const { return m_numRxDataFrames_300_Mbps;}
		void set_numRxDataFrames_300_Mbps(const int64_t & value) { m_numRxDataFrames_300_Mbps = value; }
		const int64_t get_numRxHT_243_Mbps() const { return m_numRxHT_243_Mbps;}
		void set_numRxHT_243_Mbps(const int64_t & value) { m_numRxHT_243_Mbps = value; }
		const int64_t get_numRxHT_57_7_Mbps() const { return m_numRxHT_57_7_Mbps;}
		void set_numRxHT_57_7_Mbps(const int64_t & value) { m_numRxHT_57_7_Mbps = value; }
		const int64_t get_numRxHT_234_Mbps() const { return m_numRxHT_234_Mbps;}
		void set_numRxHT_234_Mbps(const int64_t & value) { m_numRxHT_234_Mbps = value; }
		const int64_t get_numRxDataFrames_12_Mbps() const { return m_numRxDataFrames_12_Mbps;}
		void set_numRxDataFrames_12_Mbps(const int64_t & value) { m_numRxDataFrames_12_Mbps = value; }
		const int64_t get_numTxVHT_2106_Mbps() const { return m_numTxVHT_2106_Mbps;}
		void set_numTxVHT_2106_Mbps(const int64_t & value) { m_numTxVHT_2106_Mbps = value; }
		const int64_t get_numChannelBusy64s() const { return m_numChannelBusy64s;}
		void set_numChannelBusy64s(const int64_t & value) { m_numChannelBusy64s = value; }
		const int64_t get_numTxTimeFramesTransmitted() const { return m_numTxTimeFramesTransmitted;}
		void set_numTxTimeFramesTransmitted(const int64_t & value) { m_numTxTimeFramesTransmitted = value; }
		const int64_t get_numRxVHT_3466_8_Mbps() const { return m_numRxVHT_3466_8_Mbps;}
		void set_numRxVHT_3466_8_Mbps(const int64_t & value) { m_numRxVHT_3466_8_Mbps = value; }
		const int64_t get_numRxHT_52_Mbps() const { return m_numRxHT_52_Mbps;}
		void set_numRxHT_52_Mbps(const int64_t & value) { m_numRxHT_52_Mbps = value; }
		const int64_t get_numTxControl() const { return m_numTxControl;}
		void set_numTxControl(const int64_t & value) { m_numTxControl = value; }
		const int64_t get_numTx_36_Mbps() const { return m_numTx_36_Mbps;}
		void set_numTx_36_Mbps(const int64_t & value) { m_numTx_36_Mbps = value; }
		const int64_t get_numRxHT_32_5_Mbps() const { return m_numRxHT_32_5_Mbps;}
		void set_numRxHT_32_5_Mbps(const int64_t & value) { m_numRxHT_32_5_Mbps = value; }
		const int64_t get_numTxHT_15_Mbps() const { return m_numTxHT_15_Mbps;}
		void set_numTxHT_15_Mbps(const int64_t & value) { m_numTxHT_15_Mbps = value; }
		const int64_t get_numRxDataFrames_1300_Mbps() const { return m_numRxDataFrames_1300_Mbps;}
		void set_numRxDataFrames_1300_Mbps(const int64_t & value) { m_numRxDataFrames_1300_Mbps = value; }
		const int64_t get_numRxVHT_450_Mbps() const { return m_numRxVHT_450_Mbps;}
		void set_numRxVHT_450_Mbps(const int64_t & value) { m_numRxVHT_450_Mbps = value; }
		const int64_t get_numTxDataFrames_12_Mbps() const { return m_numTxDataFrames_12_Mbps;}
		void set_numTxDataFrames_12_Mbps(const int64_t & value) { m_numTxDataFrames_12_Mbps = value; }
		const int64_t get_numTxVHT_468_Mbps() const { return m_numTxVHT_468_Mbps;}
		void set_numTxVHT_468_Mbps(const int64_t & value) { m_numTxVHT_468_Mbps = value; }
		const int64_t get_numRxBcMc() const { return m_numRxBcMc;}
		void set_numRxBcMc(const int64_t & value) { m_numRxBcMc = value; }
		const int64_t get_numRxVHT_800_Mbps() const { return m_numRxVHT_800_Mbps;}
		void set_numRxVHT_800_Mbps(const int64_t & value) { m_numRxVHT_800_Mbps = value; }
		const int64_t get_numTxAggrSucc() const { return m_numTxAggrSucc;}
		void set_numTxAggrSucc(const int64_t & value) { m_numTxAggrSucc = value; }
		const int64_t get_numTxHT_243_Mbps() const { return m_numTxHT_243_Mbps;}
		void set_numTxHT_243_Mbps(const int64_t & value) { m_numTxHT_243_Mbps = value; }
		const int64_t get_numRxAmsduDeaggItmd() const { return m_numRxAmsduDeaggItmd;}
		void set_numRxAmsduDeaggItmd(const int64_t & value) { m_numRxAmsduDeaggItmd = value; }
		const int64_t get_numTxVHT_1053_Mbps() const { return m_numTxVHT_1053_Mbps;}
		void set_numTxVHT_1053_Mbps(const int64_t & value) { m_numTxVHT_1053_Mbps = value; }
		const int64_t get_numTxHT_90_Mbps() const { return m_numTxHT_90_Mbps;}
		void set_numTxHT_90_Mbps(const int64_t & value) { m_numTxHT_90_Mbps = value; }
		const int64_t get_numRxVHT_877_5_Mbps() const { return m_numRxVHT_877_5_Mbps;}
		void set_numRxVHT_877_5_Mbps(const int64_t & value) { m_numRxVHT_877_5_Mbps = value; }
		const int64_t get_numTxHT_19_5_Mbps() const { return m_numTxHT_19_5_Mbps;}
		void set_numTxHT_19_5_Mbps(const int64_t & value) { m_numTxHT_19_5_Mbps = value; }
		const int64_t get_numTxQueued() const { return m_numTxQueued;}
		void set_numTxQueued(const int64_t & value) { m_numTxQueued = value; }
		const int64_t get_rxLastRssi() const { return m_rxLastRssi;}
		void set_rxLastRssi(const int64_t & value) { m_rxLastRssi = value; }
		const int64_t get_numRxHT_208_Mbps() const { return m_numRxHT_208_Mbps;}
		void set_numRxHT_208_Mbps(const int64_t & value) { m_numRxHT_208_Mbps = value; }
		const int64_t get_numTxRetryDropped() const { return m_numTxRetryDropped;}
		void set_numTxRetryDropped(const int64_t & value) { m_numTxRetryDropped = value; }
		const int64_t get_numRxHT_351_Mbps() const { return m_numRxHT_351_Mbps;}
		void set_numRxHT_351_Mbps(const int64_t & value) { m_numRxHT_351_Mbps = value; }
		const int64_t get_numTxData() const { return m_numTxData;}
		void set_numTxData(const int64_t & value) { m_numTxData = value; }
		const int64_t get_numRxVHT_1404_Mbps() const { return m_numRxVHT_1404_Mbps;}
		void set_numRxVHT_1404_Mbps(const int64_t & value) { m_numRxVHT_1404_Mbps = value; }
		const int64_t get_numRxVHT_1560_Mbps() const { return m_numRxVHT_1560_Mbps;}
		void set_numRxVHT_1560_Mbps(const int64_t & value) { m_numRxVHT_1560_Mbps = value; }
		const int64_t get_numRxAmsduDeaggLast() const { return m_numRxAmsduDeaggLast;}
		void set_numRxAmsduDeaggLast(const int64_t & value) { m_numRxAmsduDeaggLast = value; }
		const int64_t get_numRxHT_43_2_Mbps() const { return m_numRxHT_43_2_Mbps;}
		void set_numRxHT_43_2_Mbps(const int64_t & value) { m_numRxHT_43_2_Mbps = value; }
		const int64_t get_numRxHT_29_2_Mbps() const { return m_numRxHT_29_2_Mbps;}
		void set_numRxHT_29_2_Mbps(const int64_t & value) { m_numRxHT_29_2_Mbps = value; }
		const int64_t get_numRxHT_13_5_Mbps() const { return m_numRxHT_13_5_Mbps;}
		void set_numRxHT_13_5_Mbps(const int64_t & value) { m_numRxHT_13_5_Mbps = value; }
		const int64_t get_numTxVHT_1053_1_Mbps() const { return m_numTxVHT_1053_1_Mbps;}
		void set_numTxVHT_1053_1_Mbps(const int64_t & value) { m_numTxVHT_1053_1_Mbps = value; }
		const int64_t get_numTxHT_65_Mbps() const { return m_numTxHT_65_Mbps;}
		void set_numTxHT_65_Mbps(const int64_t & value) { m_numTxHT_65_Mbps = value; }
		const int64_t get_numTxVHT_3120_Mbps() const { return m_numTxVHT_3120_Mbps;}
		void set_numTxVHT_3120_Mbps(const int64_t & value) { m_numTxVHT_3120_Mbps = value; }
		const int64_t get_numRxHT_87_8_Mbps() const { return m_numRxHT_87_8_Mbps;}
		void set_numRxHT_87_8_Mbps(const int64_t & value) { m_numRxHT_87_8_Mbps = value; }
		const int64_t get_numRxHT_312_Mbps() const { return m_numRxHT_312_Mbps;}
		void set_numRxHT_312_Mbps(const int64_t & value) { m_numRxHT_312_Mbps = value; }
		const int64_t get_numTxRtsSucc() const { return m_numTxRtsSucc;}
		void set_numTxRtsSucc(const int64_t & value) { m_numTxRtsSucc = value; }
		const int64_t get_numTxHT_325_Mbps() const { return m_numTxHT_325_Mbps;}
		void set_numTxHT_325_Mbps(const int64_t & value) { m_numTxHT_325_Mbps = value; }
		const int64_t get_numChanChanges() const { return m_numChanChanges;}
		void set_numChanChanges(const int64_t & value) { m_numChanChanges = value; }
		const int64_t get_numTxSucc() const { return m_numTxSucc;}
		void set_numTxSucc(const int64_t & value) { m_numTxSucc = value; }
		const int64_t get_numTxTimeData() const { return m_numTxTimeData;}
		void set_numTxTimeData(const int64_t & value) { m_numTxTimeData = value; }
		const int64_t get_numRxHT_97_5_Mbps() const { return m_numRxHT_97_5_Mbps;}
		void set_numRxHT_97_5_Mbps(const int64_t & value) { m_numRxHT_97_5_Mbps = value; }
		const int64_t get_numTx_48_Mbps() const { return m_numTx_48_Mbps;}
		void set_numTx_48_Mbps(const int64_t & value) { m_numTx_48_Mbps = value; }
		const int64_t get_numTxVHT_540_Mbps() const { return m_numTxVHT_540_Mbps;}
		void set_numTxVHT_540_Mbps(const int64_t & value) { m_numTxVHT_540_Mbps = value; }
		const int64_t get_numTxAggrOneMpdu() const { return m_numTxAggrOneMpdu;}
		void set_numTxAggrOneMpdu(const int64_t & value) { m_numTxAggrOneMpdu = value; }
		const int64_t get_numTxHT_360_Mbps() const { return m_numTxHT_360_Mbps;}
		void set_numTxHT_360_Mbps(const int64_t & value) { m_numTxHT_360_Mbps = value; }
		const int64_t get_numScanReq() const { return m_numScanReq;}
		void set_numScanReq(const int64_t & value) { m_numScanReq = value; }
		const int64_t get_numTxNoAck() const { return m_numTxNoAck;}
		void set_numTxNoAck(const int64_t & value) { m_numTxNoAck = value; }
		const int64_t get_numRxHT_173_1_Mbps() const { return m_numRxHT_173_1_Mbps;}
		void set_numRxHT_173_1_Mbps(const int64_t & value) { m_numRxHT_173_1_Mbps = value; }
		const int64_t get_numTxVHT_1733_1_Mbps() const { return m_numTxVHT_1733_1_Mbps;}
		void set_numTxVHT_1733_1_Mbps(const int64_t & value) { m_numTxVHT_1733_1_Mbps = value; }
		const int64_t get_numTxHT_104_Mbps() const { return m_numTxHT_104_Mbps;}
		void set_numTxHT_104_Mbps(const int64_t & value) { m_numTxHT_104_Mbps = value; }
		const int64_t get_numRxHT_27_Mbps() const { return m_numRxHT_27_Mbps;}
		void set_numRxHT_27_Mbps(const int64_t & value) { m_numRxHT_27_Mbps = value; }
		const int64_t get_numTxVHT_325_Mbps() const { return m_numTxVHT_325_Mbps;}
		void set_numTxVHT_325_Mbps(const int64_t & value) { m_numTxVHT_325_Mbps = value; }
		const int64_t get_numRcvBcForTx() const { return m_numRcvBcForTx;}
		void set_numRcvBcForTx(const int64_t & value) { m_numRcvBcForTx = value; }
		const int64_t get_numRxHT_30_Mbps() const { return m_numRxHT_30_Mbps;}
		void set_numRxHT_30_Mbps(const int64_t & value) { m_numRxHT_30_Mbps = value; }
		const int64_t get_numRxNoFcsErr() const { return m_numRxNoFcsErr;}
		void set_numRxNoFcsErr(const int64_t & value) { m_numRxNoFcsErr = value; }
		const int64_t get_numTxDataTransmittedRetried() const { return m_numTxDataTransmittedRetried;}
		void set_numTxDataTransmittedRetried(const int64_t & value) { m_numTxDataTransmittedRetried = value; }
		const int64_t get_numTxHT_28_8_Mbps() const { return m_numTxHT_28_8_Mbps;}
		void set_numTxHT_28_8_Mbps(const int64_t & value) { m_numTxHT_28_8_Mbps = value; }
		const int64_t get_numRadioResets() const { return m_numRadioResets;}
		void set_numRadioResets(const int64_t & value) { m_numRadioResets = value; }
		const int64_t get_numRxHT_135_Mbps() const { return m_numRxHT_135_Mbps;}
		void set_numRxHT_135_Mbps(const int64_t & value) { m_numRxHT_135_Mbps = value; }
		const int64_t get_numTxVHT_650_Mbps() const { return m_numTxVHT_650_Mbps;}
		void set_numTxVHT_650_Mbps(const int64_t & value) { m_numTxVHT_650_Mbps = value; }
		const int64_t get_numRxVHT_1300_Mbps() const { return m_numRxVHT_1300_Mbps;}
		void set_numRxVHT_1300_Mbps(const int64_t & value) { m_numRxVHT_1300_Mbps = value; }
		const int64_t get_numTxHT_346_7_Mbps() const { return m_numTxHT_346_7_Mbps;}
		void set_numTxHT_346_7_Mbps(const int64_t & value) { m_numTxHT_346_7_Mbps = value; }
		const int64_t get_numTxVHT_1404_Mbps() const { return m_numTxVHT_1404_Mbps;}
		void set_numTxVHT_1404_Mbps(const int64_t & value) { m_numTxVHT_1404_Mbps = value; }
		const int64_t get_numRxHT_81_Mbps() const { return m_numRxHT_81_Mbps;}
		void set_numRxHT_81_Mbps(const int64_t & value) { m_numRxHT_81_Mbps = value; }
		const int64_t get_numRxHT_78_Mbps() const { return m_numRxHT_78_Mbps;}
		void set_numRxHT_78_Mbps(const int64_t & value) { m_numRxHT_78_Mbps = value; }
		const int64_t get_numTxHT_57_5_Mbps() const { return m_numTxHT_57_5_Mbps;}
		void set_numTxHT_57_5_Mbps(const int64_t & value) { m_numTxHT_57_5_Mbps = value; }
		const int64_t get_numTxVHT_400_Mbps() const { return m_numTxVHT_400_Mbps;}
		void set_numTxVHT_400_Mbps(const int64_t & value) { m_numTxVHT_400_Mbps = value; }
		const int64_t get_numRxVHT_2600_Mbps() const { return m_numRxVHT_2600_Mbps;}
		void set_numRxVHT_2600_Mbps(const int64_t & value) { m_numRxVHT_2600_Mbps = value; }
		const int64_t get_numRxVHT_1170_Mbps() const { return m_numRxVHT_1170_Mbps;}
		void set_numRxVHT_1170_Mbps(const int64_t & value) { m_numRxVHT_1170_Mbps = value; }
		const int64_t get_numTxHT_117_1_Mbps() const { return m_numTxHT_117_1_Mbps;}
		void set_numTxHT_117_1_Mbps(const int64_t & value) { m_numTxHT_117_1_Mbps = value; }
		const int64_t get_numRx_12_Mbps() const { return m_numRx_12_Mbps;}
		void set_numRx_12_Mbps(const int64_t & value) { m_numRx_12_Mbps = value; }
		const int64_t get_numTxDataFrames_300_Mbps() const { return m_numTxDataFrames_300_Mbps;}
		void set_numTxDataFrames_300_Mbps(const int64_t & value) { m_numTxDataFrames_300_Mbps = value; }
		const int64_t get_numTxHT_180_Mbps() const { return m_numTxHT_180_Mbps;}
		void set_numTxHT_180_Mbps(const int64_t & value) { m_numTxHT_180_Mbps = value; }
		const int64_t get_numTxHT_292_5_Mbps() const { return m_numTxHT_292_5_Mbps;}
		void set_numTxHT_292_5_Mbps(const int64_t & value) { m_numTxHT_292_5_Mbps = value; }
		const int64_t get_numRxHT_162_Mbps() const { return m_numRxHT_162_Mbps;}
		void set_numRxHT_162_Mbps(const int64_t & value) { m_numRxHT_162_Mbps = value; }
		const int64_t get_numTxVHT_292_5_Mbps() const { return m_numTxVHT_292_5_Mbps;}
		void set_numTxVHT_292_5_Mbps(const int64_t & value) { m_numTxVHT_292_5_Mbps = value; }
		const int64_t get_numTxHT_27_Mbps() const { return m_numTxHT_27_Mbps;}
		void set_numTxHT_27_Mbps(const int64_t & value) { m_numTxHT_27_Mbps = value; }
		const int64_t get_numRxVHT_364_5_Mbps() const { return m_numRxVHT_364_5_Mbps;}
		void set_numRxVHT_364_5_Mbps(const int64_t & value) { m_numRxVHT_364_5_Mbps = value; }
		const int64_t get_numTxVHT_2600_Mbps() const { return m_numTxVHT_2600_Mbps;}
		void set_numTxVHT_2600_Mbps(const int64_t & value) { m_numTxVHT_2600_Mbps = value; }
		const int64_t get_numTxHT_97_5_Mbps() const { return m_numTxHT_97_5_Mbps;}
		void set_numTxHT_97_5_Mbps(const int64_t & value) { m_numTxHT_97_5_Mbps = value; }
		const int64_t get_numTxSuccRetries() const { return m_numTxSuccRetries;}
		void set_numTxSuccRetries(const int64_t & value) { m_numTxSuccRetries = value; }
		const int64_t get_numTxVHT_780_Mbps() const { return m_numTxVHT_780_Mbps;}
		void set_numTxVHT_780_Mbps(const int64_t & value) { m_numTxVHT_780_Mbps = value; }
		const int64_t get_numTxPsUnicast() const { return m_numTxPsUnicast;}
		void set_numTxPsUnicast(const int64_t & value) { m_numTxPsUnicast = value; }
		const int64_t get_numRxHT_260_Mbps() const { return m_numRxHT_260_Mbps;}
		void set_numRxHT_260_Mbps(const int64_t & value) { m_numRxHT_260_Mbps = value; }
		const int64_t get_numRxDropBadProtocol() const { return m_numRxDropBadProtocol;}
		void set_numRxDropBadProtocol(const int64_t & value) { m_numRxDropBadProtocol = value; }
		const int64_t get_numRxVHT_866_7_Mbps() const { return m_numRxVHT_866_7_Mbps;}
		void set_numRxVHT_866_7_Mbps(const int64_t & value) { m_numRxVHT_866_7_Mbps = value; }
		const int64_t get_numTxVHT_1579_5_Mbps() const { return m_numTxVHT_1579_5_Mbps;}
		void set_numTxVHT_1579_5_Mbps(const int64_t & value) { m_numTxVHT_1579_5_Mbps = value; }
		const int64_t get_numRxHT_45_Mbps() const { return m_numRxHT_45_Mbps;}
		void set_numRxHT_45_Mbps(const int64_t & value) { m_numRxHT_45_Mbps = value; }
		const int64_t get_numTxHT_175_5_Mbps() const { return m_numTxHT_175_5_Mbps;}
		void set_numTxHT_175_5_Mbps(const int64_t & value) { m_numTxHT_175_5_Mbps = value; }
		const int64_t get_numRxControl() const { return m_numRxControl;}
		void set_numRxControl(const int64_t & value) { m_numRxControl = value; }
		const int64_t get_numRxHT_263_2_Mbps() const { return m_numRxHT_263_2_Mbps;}
		void set_numRxHT_263_2_Mbps(const int64_t & value) { m_numRxHT_263_2_Mbps = value; }
		const int64_t get_numRxVHT_1733_4_Mbps() const { return m_numRxVHT_1733_4_Mbps;}
		void set_numRxVHT_1733_4_Mbps(const int64_t & value) { m_numRxVHT_1733_4_Mbps = value; }
		const int64_t get_numTxHT_60_Mbps() const { return m_numTxHT_60_Mbps;}
		void set_numTxHT_60_Mbps(const int64_t & value) { m_numTxHT_60_Mbps = value; }
		const int64_t get_numTxVHT_405_Mbps() const { return m_numTxVHT_405_Mbps;}
		void set_numTxVHT_405_Mbps(const int64_t & value) { m_numTxVHT_405_Mbps = value; }
		const int64_t get_numTxHT_200_Mbps() const { return m_numTxHT_200_Mbps;}
		void set_numTxHT_200_Mbps(const int64_t & value) { m_numTxHT_200_Mbps = value; }
		const int64_t get_numTxVHT_1560_Mbps() const { return m_numTxVHT_1560_Mbps;}
		void set_numTxVHT_1560_Mbps(const int64_t & value) { m_numTxVHT_1560_Mbps = value; }
		const int64_t get_numTxTime_BC_MC_Data() const { return m_numTxTime_BC_MC_Data;}
		void set_numTxTime_BC_MC_Data(const int64_t & value) { m_numTxTime_BC_MC_Data = value; }
		const int64_t get_numRxVHT_390_Mbps() const { return m_numRxVHT_390_Mbps;}
		void set_numRxVHT_390_Mbps(const int64_t & value) { m_numRxVHT_390_Mbps = value; }
		const int64_t get_numRadarChanChanges() const { return m_numRadarChanChanges;}
		void set_numRadarChanChanges(const int64_t & value) { m_numRadarChanChanges = value; }
		const int64_t get_numTxVHT_1950_Mbps() const { return m_numTxVHT_1950_Mbps;}
		void set_numTxVHT_1950_Mbps(const int64_t & value) { m_numTxVHT_1950_Mbps = value; }
		const int64_t get_numRxHT_86_8_Mbps() const { return m_numRxHT_86_8_Mbps;}
		void set_numRxHT_86_8_Mbps(const int64_t & value) { m_numRxHT_86_8_Mbps = value; }
		const int64_t get_numTxVHT_1872_Mbps() const { return m_numTxVHT_1872_Mbps;}
		void set_numTxVHT_1872_Mbps(const int64_t & value) { m_numTxVHT_1872_Mbps = value; }
		const int64_t get_numRxHT_57_5_Mbps() const { return m_numRxHT_57_5_Mbps;}
		void set_numRxHT_57_5_Mbps(const int64_t & value) { m_numRxHT_57_5_Mbps = value; }
		const int64_t get_numTxHT_300_Mbps() const { return m_numTxHT_300_Mbps;}
		void set_numTxHT_300_Mbps(const int64_t & value) { m_numTxHT_300_Mbps = value; }
		const int64_t get_numTxHT_231_1_Mbps() const { return m_numTxHT_231_1_Mbps;}
		void set_numTxHT_231_1_Mbps(const int64_t & value) { m_numTxHT_231_1_Mbps = value; }
		const int64_t get_numRxDataFrames_54_Mbps() const { return m_numRxDataFrames_54_Mbps;}
		void set_numRxDataFrames_54_Mbps(const int64_t & value) { m_numRxDataFrames_54_Mbps = value; }
		const int64_t get_numTxRetryAttemps() const { return m_numTxRetryAttemps;}
		void set_numTxRetryAttemps(const int64_t & value) { m_numTxRetryAttemps = value; }
		const int64_t get_numRxHT_6_5_Mbps() const { return m_numRxHT_6_5_Mbps;}
		void set_numRxHT_6_5_Mbps(const int64_t & value) { m_numRxHT_6_5_Mbps = value; }
		const int64_t get_numTx_54_Mbps() const { return m_numTx_54_Mbps;}
		void set_numTx_54_Mbps(const int64_t & value) { m_numTx_54_Mbps = value; }
		const int64_t get_numTxHT_173_1_Mbps() const { return m_numTxHT_173_1_Mbps;}
		void set_numTxHT_173_1_Mbps(const int64_t & value) { m_numTxHT_173_1_Mbps = value; }
		const int64_t get_numTxFramesTransmitted() const { return m_numTxFramesTransmitted;}
		void set_numTxFramesTransmitted(const int64_t & value) { m_numTxFramesTransmitted = value; }
		const int64_t get_numTxVHT_975_Mbps() const { return m_numTxVHT_975_Mbps;}
		void set_numTxVHT_975_Mbps(const int64_t & value) { m_numTxVHT_975_Mbps = value; }
		const int64_t get_numRxHT_58_5_Mbps() const { return m_numRxHT_58_5_Mbps;}
		void set_numRxHT_58_5_Mbps(const int64_t & value) { m_numRxHT_58_5_Mbps = value; }
		const int64_t get_numRx() const { return m_numRx;}
		void set_numRx(const int64_t & value) { m_numRx = value; }
		const int64_t get_numRxHT_173_3_Mbps() const { return m_numRxHT_173_3_Mbps;}
		void set_numRxHT_173_3_Mbps(const int64_t & value) { m_numRxHT_173_3_Mbps = value; }
		const int64_t get_numRx_36_Mbps() const { return m_numRx_36_Mbps;}
		void set_numRx_36_Mbps(const int64_t & value) { m_numRx_36_Mbps = value; }
		const int64_t get_numTxHT_7_1_Mbps() const { return m_numTxHT_7_1_Mbps;}
		void set_numTxHT_7_1_Mbps(const int64_t & value) { m_numTxHT_7_1_Mbps = value; }
		const int64_t get_numTxBeaconSucc() const { return m_numTxBeaconSucc;}
		void set_numTxBeaconSucc(const int64_t & value) { m_numTxBeaconSucc = value; }
		const int64_t get_numRxHT_346_7_Mbps() const { return m_numRxHT_346_7_Mbps;}
		void set_numRxHT_346_7_Mbps(const int64_t & value) { m_numRxHT_346_7_Mbps = value; }
		const int64_t get_numRxVHT_520_Mbps() const { return m_numRxVHT_520_Mbps;}
		void set_numRxVHT_520_Mbps(const int64_t & value) { m_numRxVHT_520_Mbps = value; }
		const int64_t get_numRxVHT_1053_1_Mbps() const { return m_numRxVHT_1053_1_Mbps;}
		void set_numRxVHT_1053_1_Mbps(const int64_t & value) { m_numRxVHT_1053_1_Mbps = value; }
		const int64_t get_numTxDataFrames_54_Mbps() const { return m_numTxDataFrames_54_Mbps;}
		void set_numTxDataFrames_54_Mbps(const int64_t & value) { m_numTxDataFrames_54_Mbps = value; }
		const int64_t get_numTxDataFrames() const { return m_numTxDataFrames;}
		void set_numTxDataFrames(const int64_t & value) { m_numTxDataFrames = value; }
		const int64_t get_numTxHT_208_Mbps() const { return m_numTxHT_208_Mbps;}
		void set_numTxHT_208_Mbps(const int64_t & value) { m_numTxHT_208_Mbps = value; }
		const int64_t get_numRxVHT_720_Mbps() const { return m_numRxVHT_720_Mbps;}
		void set_numRxVHT_720_Mbps(const int64_t & value) { m_numRxVHT_720_Mbps = value; }
		const int64_t get_numTxHT_26_Mbps() const { return m_numTxHT_26_Mbps;}
		void set_numTxHT_26_Mbps(const int64_t & value) { m_numTxHT_26_Mbps = value; }
		const int64_t get_numTxVHT_480_Mbps() const { return m_numTxVHT_480_Mbps;}
		void set_numTxVHT_480_Mbps(const int64_t & value) { m_numTxVHT_480_Mbps = value; }
		const int64_t get_numTxVHT_2080_Mbps() const { return m_numTxVHT_2080_Mbps;}
		void set_numTxVHT_2080_Mbps(const int64_t & value) { m_numTxVHT_2080_Mbps = value; }
		const int64_t get_numTxHT_351_Mbps() const { return m_numTxHT_351_Mbps;}
		void set_numTxHT_351_Mbps(const int64_t & value) { m_numTxHT_351_Mbps = value; }
		const int64_t get_numRxRts() const { return m_numRxRts;}
		void set_numRxRts(const int64_t & value) { m_numRxRts = value; }
		const int64_t get_numTxRtsFail() const { return m_numTxRtsFail;}
		void set_numTxRtsFail(const int64_t & value) { m_numTxRtsFail = value; }
		const int64_t get_numRxHT_15_Mbps() const { return m_numRxHT_15_Mbps;}
		void set_numRxHT_15_Mbps(const int64_t & value) { m_numRxHT_15_Mbps = value; }
		const int64_t get_numRxRetryFrames() const { return m_numRxRetryFrames;}
		void set_numRxRetryFrames(const int64_t & value) { m_numRxRetryFrames = value; }
		const int64_t get_numRxHT_175_5_Mbps() const { return m_numRxHT_175_5_Mbps;}
		void set_numRxHT_175_5_Mbps(const int64_t & value) { m_numRxHT_175_5_Mbps = value; }
		const int64_t get_numTxHT_288_7_Mbps() const { return m_numTxHT_288_7_Mbps;}
		void set_numTxHT_288_7_Mbps(const int64_t & value) { m_numTxHT_288_7_Mbps = value; }
		const int64_t get_numTxHT_29_2_Mbps() const { return m_numTxHT_29_2_Mbps;}
		void set_numTxHT_29_2_Mbps(const int64_t & value) { m_numTxHT_29_2_Mbps = value; }
		const int64_t get_numRxHT_144_3_Mbps() const { return m_numRxHT_144_3_Mbps;}
		void set_numRxHT_144_3_Mbps(const int64_t & value) { m_numRxHT_144_3_Mbps = value; }
		const int64_t get_numTxRateLimitDrop() const { return m_numTxRateLimitDrop;}
		void set_numTxRateLimitDrop(const int64_t & value) { m_numTxRateLimitDrop = value; }
		const int64_t get_numRxVHT_1579_5_Mbps() const { return m_numRxVHT_1579_5_Mbps;}
		void set_numRxVHT_1579_5_Mbps(const int64_t & value) { m_numRxVHT_1579_5_Mbps = value; }
		const int64_t get_numRxDataFrames_1300Plus_Mbps() const { return m_numRxDataFrames_1300Plus_Mbps;}
		void set_numRxDataFrames_1300Plus_Mbps(const int64_t & value) { m_numRxDataFrames_1300Plus_Mbps = value; }
		const int64_t get_curEirp() const { return m_curEirp;}
		void set_curEirp(const int64_t & value) { m_curEirp = value; }
		const int64_t get_numTxHT_57_7_Mbps() const { return m_numTxHT_57_7_Mbps;}
		void set_numTxHT_57_7_Mbps(const int64_t & value) { m_numTxHT_57_7_Mbps = value; }
		const int64_t get_numRxHT_86_6_Mbps() const { return m_numRxHT_86_6_Mbps;}
		void set_numRxHT_86_6_Mbps(const int64_t & value) { m_numRxHT_86_6_Mbps = value; }
		const int64_t get_numTx_6_Mbps() const { return m_numTx_6_Mbps;}
		void set_numTx_6_Mbps(const int64_t & value) { m_numTx_6_Mbps = value; }
		const int64_t get_numTxHT_45_Mbps() const { return m_numTxHT_45_Mbps;}
		void set_numTxHT_45_Mbps(const int64_t & value) { m_numTxHT_45_Mbps = value; }
		const int64_t get_numTxHT_216_Mbps() const { return m_numTxHT_216_Mbps;}
		void set_numTxHT_216_Mbps(const int64_t & value) { m_numTxHT_216_Mbps = value; }
		const int64_t get_numTxVHT_800_Mbps() const { return m_numTxVHT_800_Mbps;}
		void set_numTxVHT_800_Mbps(const int64_t & value) { m_numTxVHT_800_Mbps = value; }
		const int64_t get_numTxVHT_3466_8_Mbps() const { return m_numTxVHT_3466_8_Mbps;}
		void set_numTxVHT_3466_8_Mbps(const int64_t & value) { m_numTxVHT_3466_8_Mbps = value; }
		const int64_t get_numTxHT_260_Mbps() const { return m_numTxHT_260_Mbps;}
		void set_numTxHT_260_Mbps(const int64_t & value) { m_numTxHT_260_Mbps = value; }
		const int64_t get_numRxHT_108_Mbps() const { return m_numRxHT_108_Mbps;}
		void set_numRxHT_108_Mbps(const int64_t & value) { m_numRxHT_108_Mbps = value; }
		const int64_t get_numTxHT_40_5_Mbps() const { return m_numTxHT_40_5_Mbps;}
		void set_numTxHT_40_5_Mbps(const int64_t & value) { m_numTxHT_40_5_Mbps = value; }
		const int64_t get_numTxProbeResp() const { return m_numTxProbeResp;}
		void set_numTxProbeResp(const int64_t & value) { m_numTxProbeResp = value; }
		const int64_t get_numTxHT_234_Mbps() const { return m_numTxHT_234_Mbps;}
		void set_numTxHT_234_Mbps(const int64_t & value) { m_numTxHT_234_Mbps = value; }
		const int64_t get_numTx_1_Mbps() const { return m_numTx_1_Mbps;}
		void set_numTx_1_Mbps(const int64_t & value) { m_numTx_1_Mbps = value; }
		const int64_t get_numRxLdpc() const { return m_numRxLdpc;}
		void set_numRxLdpc(const int64_t & value) { m_numRxLdpc = value; }
		const int64_t get_numTxSuccNoRetry() const { return m_numTxSuccNoRetry;}
		void set_numTxSuccNoRetry(const int64_t & value) { m_numTxSuccNoRetry = value; }
		const int64_t get_numRxHT_21_7_Mbps() const { return m_numRxHT_21_7_Mbps;}
		void set_numRxHT_21_7_Mbps(const int64_t & value) { m_numRxHT_21_7_Mbps = value; }
		const int64_t get_numTxHT_162_Mbps() const { return m_numTxHT_162_Mbps;}
		void set_numTxHT_162_Mbps(const int64_t & value) { m_numTxHT_162_Mbps = value; }
		const int64_t get_numTxHT_195_Mbps() const { return m_numTxHT_195_Mbps;}
		void set_numTxHT_195_Mbps(const int64_t & value) { m_numTxHT_195_Mbps = value; }
		const int64_t get_numRx_6_Mbps() const { return m_numRx_6_Mbps;}
		void set_numRx_6_Mbps(const int64_t & value) { m_numRx_6_Mbps = value; }
		const int64_t get_numTxHT_21_7_Mbps() const { return m_numTxHT_21_7_Mbps;}
		void set_numTxHT_21_7_Mbps(const int64_t & value) { m_numTxHT_21_7_Mbps = value; }
		const int64_t get_numTxHT_130_3_Mbps() const { return m_numTxHT_130_3_Mbps;}
		void set_numTxHT_130_3_Mbps(const int64_t & value) { m_numTxHT_130_3_Mbps = value; }
		const int64_t get_numTxDataFrames_108_Mbps() const { return m_numTxDataFrames_108_Mbps;}
		void set_numTxDataFrames_108_Mbps(const int64_t & value) { m_numTxDataFrames_108_Mbps = value; }
		const int64_t get_elevenGProtection() const { return m_elevenGProtection;}
		void set_elevenGProtection(const int64_t & value) { m_elevenGProtection = value; }
		const int64_t get_numRxHT_90_Mbps() const { return m_numRxHT_90_Mbps;}
		void set_numRxHT_90_Mbps(const int64_t & value) { m_numRxHT_90_Mbps = value; }
		const int64_t get_numRxVHT_486_Mbps() const { return m_numRxVHT_486_Mbps;}
		void set_numRxVHT_486_Mbps(const int64_t & value) { m_numRxVHT_486_Mbps = value; }
		const int64_t get_numTxVHT_877_5_Mbps() const { return m_numTxVHT_877_5_Mbps;}
		void set_numTxVHT_877_5_Mbps(const int64_t & value) { m_numTxVHT_877_5_Mbps = value; }
		const int64_t get_numRxVHT_540_Mbps() const { return m_numRxVHT_540_Mbps;}
		void set_numRxVHT_540_Mbps(const int64_t & value) { m_numRxVHT_540_Mbps = value; }
		const int64_t get_curBackupChannel() const { return m_curBackupChannel;}
		void set_curBackupChannel(const int64_t & value) { m_curBackupChannel = value; }
		const int64_t get_numRxVHT_526_5_Mbps() const { return m_numRxVHT_526_5_Mbps;}
		void set_numRxVHT_526_5_Mbps(const int64_t & value) { m_numRxVHT_526_5_Mbps = value; }
		const int64_t get_numTxHT_121_5_Mbps() const { return m_numTxHT_121_5_Mbps;}
		void set_numTxHT_121_5_Mbps(const int64_t & value) { m_numTxHT_121_5_Mbps = value; }
		const int64_t get_numTxHT_32_5_Mbps() const { return m_numTxHT_32_5_Mbps;}
		void set_numTxHT_32_5_Mbps(const int64_t & value) { m_numTxHT_32_5_Mbps = value; }
		const int64_t get_numTxHT_130_Mbps() const { return m_numTxHT_130_Mbps;}
		void set_numTxHT_130_Mbps(const int64_t & value) { m_numTxHT_130_Mbps = value; }
		const int64_t get_numRxHT_240_Mbps() const { return m_numRxHT_240_Mbps;}
		void set_numRxHT_240_Mbps(const int64_t & value) { m_numRxHT_240_Mbps = value; }
		const int64_t get_numTxHT_150_Mbps() const { return m_numTxHT_150_Mbps;}
		void set_numTxHT_150_Mbps(const int64_t & value) { m_numTxHT_150_Mbps = value; }
		const int64_t get_numRxRetry() const { return m_numRxRetry;}
		void set_numRxRetry(const int64_t & value) { m_numRxRetry = value; }
		const int64_t get_numRxErr() const { return m_numRxErr;}
		void set_numRxErr(const int64_t & value) { m_numRxErr = value; }
		const int64_t get_numTxHT_72_1_Mbps() const { return m_numTxHT_72_1_Mbps;}
		void set_numTxHT_72_1_Mbps(const int64_t & value) { m_numTxHT_72_1_Mbps = value; }
		const int64_t get_numRxVHT_400_Mbps() const { return m_numRxVHT_400_Mbps;}
		void set_numRxVHT_400_Mbps(const int64_t & value) { m_numRxVHT_400_Mbps = value; }
		const int64_t get_numRxVHT_2080_Mbps() const { return m_numRxVHT_2080_Mbps;}
		void set_numRxVHT_2080_Mbps(const int64_t & value) { m_numRxVHT_2080_Mbps = value; }
		const int64_t get_numTxHT_144_3_Mbps() const { return m_numTxHT_144_3_Mbps;}
		void set_numTxHT_144_3_Mbps(const int64_t & value) { m_numTxHT_144_3_Mbps = value; }
		const int64_t get_numRxVHT_403_Mbps() const { return m_numRxVHT_403_Mbps;}
		void set_numRxVHT_403_Mbps(const int64_t & value) { m_numRxVHT_403_Mbps = value; }
		const int64_t get_numRxVHT_585_Mbps() const { return m_numRxVHT_585_Mbps;}
		void set_numRxVHT_585_Mbps(const int64_t & value) { m_numRxVHT_585_Mbps = value; }
		const int64_t get_numRxHT_117_1_Mbps() const { return m_numRxHT_117_1_Mbps;}
		void set_numRxHT_117_1_Mbps(const int64_t & value) { m_numRxHT_117_1_Mbps = value; }
		const int64_t get_numRxHT_7_1_Mbps() const { return m_numRxHT_7_1_Mbps;}
		void set_numRxHT_7_1_Mbps(const int64_t & value) { m_numRxHT_7_1_Mbps = value; }
		const int64_t get_numRxCts() const { return m_numRxCts;}
		void set_numRxCts(const int64_t & value) { m_numRxCts = value; }
		const int64_t get_numRxDataFrames_450_Mbps() const { return m_numRxDataFrames_450_Mbps;}
		void set_numRxDataFrames_450_Mbps(const int64_t & value) { m_numRxDataFrames_450_Mbps = value; }
		const int64_t get_numRxHT_324_Mbps() const { return m_numRxHT_324_Mbps;}
		void set_numRxHT_324_Mbps(const int64_t & value) { m_numRxHT_324_Mbps = value; }
		const int64_t get_numTxHT_120_Mbps() const { return m_numTxHT_120_Mbps;}
		void set_numTxHT_120_Mbps(const int64_t & value) { m_numTxHT_120_Mbps = value; }
		const int64_t get_numTx_9_Mbps() const { return m_numTx_9_Mbps;}
		void set_numTx_9_Mbps(const int64_t & value) { m_numTx_9_Mbps = value; }
		const int64_t get_numRxVHT_3120_Mbps() const { return m_numRxVHT_3120_Mbps;}
		void set_numRxVHT_3120_Mbps(const int64_t & value) { m_numRxVHT_3120_Mbps = value; }
		const int64_t get_numRxHT_180_Mbps() const { return m_numRxHT_180_Mbps;}
		void set_numRxHT_180_Mbps(const int64_t & value) { m_numRxHT_180_Mbps = value; }
		const int64_t get_numRx_1_Mbps() const { return m_numRx_1_Mbps;}
		void set_numRx_1_Mbps(const int64_t & value) { m_numRx_1_Mbps = value; }
		const int64_t get_numRxDropAmsduNoRcv() const { return m_numRxDropAmsduNoRcv;}
		void set_numRxDropAmsduNoRcv(const int64_t & value) { m_numRxDropAmsduNoRcv = value; }
		const int64_t get_numTxHT_78_Mbps() const { return m_numTxHT_78_Mbps;}
		void set_numTxHT_78_Mbps(const int64_t & value) { m_numTxHT_78_Mbps = value; }
		const int64_t get_numTxVHT_1733_4_Mbps() const { return m_numTxVHT_1733_4_Mbps;}
		void set_numTxVHT_1733_4_Mbps(const int64_t & value) { m_numTxVHT_1733_4_Mbps = value; }
		const int64_t get_numRxHT_300_Mbps() const { return m_numRxHT_300_Mbps;}
		void set_numRxHT_300_Mbps(const int64_t & value) { m_numRxHT_300_Mbps = value; }
		const int64_t get_numRxVHT_650_Mbps() const { return m_numRxVHT_650_Mbps;}
		void set_numRxVHT_650_Mbps(const int64_t & value) { m_numRxVHT_650_Mbps = value; }
		const int64_t get_numTxStbc() const { return m_numTxStbc;}
		void set_numTxStbc(const int64_t & value) { m_numTxStbc = value; }
		const int64_t get_numTxDataFrames_1300_Mbps() const { return m_numTxDataFrames_1300_Mbps;}
		void set_numTxDataFrames_1300_Mbps(const int64_t & value) { m_numTxDataFrames_1300_Mbps = value; }
		const int64_t get_numTxMultipleRetries() const { return m_numTxMultipleRetries;}
		void set_numTxMultipleRetries(const int64_t & value) { m_numTxMultipleRetries = value; }
		const int64_t get_numTxHT_312_Mbps() const { return m_numTxHT_312_Mbps;}
		void set_numTxHT_312_Mbps(const int64_t & value) { m_numTxHT_312_Mbps = value; }
		const int64_t get_numTxDataFrames_450_Mbps() const { return m_numTxDataFrames_450_Mbps;}
		void set_numTxDataFrames_450_Mbps(const int64_t & value) { m_numTxDataFrames_450_Mbps = value; }
		const int64_t get_numRxHT_14_3_Mbps() const { return m_numRxHT_14_3_Mbps;}
		void set_numRxHT_14_3_Mbps(const int64_t & value) { m_numRxHT_14_3_Mbps = value; }
		const int64_t get_numTxCts() const { return m_numTxCts;}
		void set_numTxCts(const int64_t & value) { m_numTxCts = value; }
		const int64_t get_numRxHT_130_Mbps() const { return m_numRxHT_130_Mbps;}
		void set_numRxHT_130_Mbps(const int64_t & value) { m_numRxHT_130_Mbps = value; }
		const int64_t get_numRxVHT_975_Mbps() const { return m_numRxVHT_975_Mbps;}
		void set_numRxVHT_975_Mbps(const int64_t & value) { m_numRxVHT_975_Mbps = value; }
		const int64_t get_numTxHT_58_5_Mbps() const { return m_numTxHT_58_5_Mbps;}
		void set_numTxHT_58_5_Mbps(const int64_t & value) { m_numTxHT_58_5_Mbps = value; }
		const int64_t get_numRcvFrameForTx() const { return m_numRcvFrameForTx;}
		void set_numRcvFrameForTx(const int64_t & value) { m_numRcvFrameForTx = value; }
		const int64_t get_numTxVHT_520_Mbps() const { return m_numTxVHT_520_Mbps;}
		void set_numTxVHT_520_Mbps(const int64_t & value) { m_numTxVHT_520_Mbps = value; }
		const int64_t get_numRxManagement() const { return m_numRxManagement;}
		void set_numRxManagement(const int64_t & value) { m_numRxManagement = value; }
		const int64_t get_numRxVHT_936_Mbps() const { return m_numRxVHT_936_Mbps;}
		void set_numRxVHT_936_Mbps(const int64_t & value) { m_numRxVHT_936_Mbps = value; }
		const int64_t get_numRx_24_Mbps() const { return m_numRx_24_Mbps;}
		void set_numRx_24_Mbps(const int64_t & value) { m_numRx_24_Mbps = value; }
		const int64_t get_numTxHT_156_Mbps() const { return m_numTxHT_156_Mbps;}
		void set_numTxHT_156_Mbps(const int64_t & value) { m_numTxHT_156_Mbps = value; }
		const int64_t get_numRxHT_270_Mbps() const { return m_numRxHT_270_Mbps;}
		void set_numRxHT_270_Mbps(const int64_t & value) { m_numRxHT_270_Mbps = value; }
		const int64_t get_numRxHT_115_5_Mbps() const { return m_numRxHT_115_5_Mbps;}
		void set_numRxHT_115_5_Mbps(const int64_t & value) { m_numRxHT_115_5_Mbps = value; }
		const int64_t get_numRxHT_231_1_Mbps() const { return m_numRxHT_231_1_Mbps;}
		void set_numRxHT_231_1_Mbps(const int64_t & value) { m_numRxHT_231_1_Mbps = value; }
		const int64_t get_numTx_24_Mbps() const { return m_numTx_24_Mbps;}
		void set_numTx_24_Mbps(const int64_t & value) { m_numTx_24_Mbps = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_numTxHT_216_6_Mbps;
		int64_t	m_numRxHT_325_Mbps;
		int64_t	m_numRxFcsErr; /* The number of received frames with FCS errors. */
		int64_t	m_numRxVHT_2340_Mbps;
		int64_t	m_numRxVHT_1733_1_Mbps;
		int64_t	m_numRxVHT_702_Mbps;
		int64_t	m_numRxVHT_433_2_Mbps;
		int64_t	m_numTxHT_6_5_Mbps;
		int64_t	m_numRxTimeData;
		int64_t	m_numRxHT_351_2_Mbps;
		int64_t	m_numRxHT_216_Mbps;
		int64_t	m_numRxVHT_405_Mbps;
		int64_t	m_numRxHT_360_Mbps;
		int64_t	m_numRxHT_120_Mbps;
		int64_t	m_numTxVHT_702_Mbps;
		int64_t	m_numTxVHT_936_Mbps;
		int64_t	m_numTxVHT_390_Mbps;
		int64_t	m_numRxDropEthHdrRunt; /* The number of dropped rx frames, Ethernet header runt. */
		int64_t	m_numTxDataFrames_1300Plus_Mbps;
		int64_t	m_numRxHT_150_Mbps;
		int64_t	m_numTx_18_Mbps;
		int64_t	m_numTxVHT_1755_Mbps;
		int64_t	m_numTxVHT_364_5_Mbps;
		int64_t	m_numRxNullData; /* The number of received null data frames. */
		int64_t	m_numTxHT_43_2_Mbps;
		int64_t	m_numTxTotalAttemps; /* The total number of tx attempts */
		int64_t	m_rxDataBytes; /* The number of received data frames. */
		int64_t	m_numRxHT_39_Mbps;
		int64_t	m_numTxHT_81_Mbps;
		int64_t	m_numTxHT_39_Mbps;
		int64_t	m_numRxHT_104_Mbps;
		int64_t	m_numRxVHT_432_Mbps;
		int64_t	m_numTxVHT_486_Mbps;
		int64_t	m_numRxHT_28_8_Mbps;
		int64_t	m_numTxDropped; /* The number of every TX frame dropped. */
		int64_t	m_numTxVHT_600_Mbps;
		int64_t	m_numRxVHT_648_Mbps;
		int64_t	m_numTxVHT_720_Mbps;
		int64_t	m_numRxFramesReceived;
		int64_t	m_numRxHT_121_5_Mbps;
		int64_t	m_curChannel; /* The current primary channel */
		int64_t	m_numTxVHT_403_Mbps;
		int64_t	m_numRx_48_Mbps;
		int64_t	m_numTxAction; /* The number of Tx action frames. */
		int64_t	m_numRxBeacon; /* The number of received beacon frames. */
		int64_t	m_numRxVHT_2808_Mbps;
		int64_t	m_numTxSuccessWithRetry;
		int64_t	m_numTxHT_30_Mbps;
		int64_t	m_numRxHT_292_5_Mbps;
		int64_t	m_numRxAck; /* The number of all received ACK frames (Acks + BlockAcks). */
		int64_t	m_numRxDropInvalidSrcMac; /* The number of dropped rx frames, invalid source MAC. */
		int64_t	m_numRxHT_19_5_Mbps;
		int64_t	m_numTxHT_173_3_Mbps;
		int64_t	m_numTxHT_240_Mbps;
		int64_t	m_numTxEapol; /* The number of EAPOL frames sent. */
		int64_t	m_numRxAmsduDeaggSeq; /* The number of dropped rx frames, AMSDU deagg sequence. */
		int64_t	m_numRxHT_288_7_Mbps;
		int64_t	m_numTxManagement; /* The number of TX management frames. */
		int64_t	m_numTxBeaconFail; /* The number of unsuccessfully transmitted beacon. */
		int64_t	m_numRxVHT_1755_Mbps;
		int64_t	m_numRxPspoll; /* The number of received ps-poll frames */
		int64_t	m_numTxBcDropped; /* The number of broadcast frames dropped. */
		int64_t	m_numTxVHT_433_2_Mbps;
		int64_t	m_numTxVHT_585_Mbps;
		int64_t	m_numTxHT_86_8_Mbps;
		int64_t	m_numTxHT_52_Mbps;
		int64_t	m_numRxVHT_292_5_Mbps;
		int64_t	m_numRxHT_60_Mbps;
		int64_t	m_numRxTimeToMe;
		int64_t	m_numRxOffChan; /* The number of frames received during off-channel scanning. */
		int64_t	m_numTxBeaconSuFail; /* The number of successive beacon tx failure. */
		int64_t	m_numTxHT_28_7_Mbps;
		int64_t	m_numTxHT_108_Mbps;
		int64_t	m_numRxHT_156_Mbps;
		int64_t	m_numRxVHT_468_Mbps;
		int64_t	m_numTxVHT_2808_Mbps;
		std::vector<int64_t>	m_actualCellSize; /* Actuall Cell Size */
		int64_t	m_numTxVHT_526_5_Mbps;
		int64_t	m_numTxHT_324_Mbps;
		int64_t	m_numTxVHT_866_7_Mbps;
		int64_t	m_numRxHT_72_1_Mbps;
		int64_t	m_numRxHT_65_Mbps;
		int64_t	m_numTxDataTransmitted;
		int64_t	m_numRx_18_Mbps;
		int64_t	m_numRxDataFrames_108_Mbps;
		int64_t	m_numRxVHT_1053_Mbps;
		int64_t	m_numRxVHT_2106_Mbps;
		int64_t	m_numRxHT_288_8_Mbps;
		int64_t	m_numRxHT_40_5_Mbps;
		int64_t	m_numTxVHT_1170_Mbps;
		int64_t	m_numTxDataRetries; /* The number of Tx data frames with retries. */
		int64_t	m_numTxHT_117_Mbps;
		int64_t	m_numTxHT_13_Mbps;
		int64_t	m_numRxVHT_1872_Mbps;
		int64_t	m_numTxVHT_2340_Mbps;
		int64_t	m_numRxHT_216_6_Mbps;
		int64_t	m_numTxHT_54_Mbps;
		int64_t	m_numRx_9_Mbps;
		int64_t	m_numRxStbc; /* The number of received STBC frames. */
		int64_t	m_numTxHT_270_Mbps;
		int64_t	m_numTxHT_288_8_Mbps;
		int64_t	m_numTxHT_351_2_Mbps;
		int64_t	m_numRxVHT_325_Mbps;
		int64_t	m_numTxHT_135_Mbps;
		int64_t	m_numRxProbeReq; /* The number of received probe request frames. */
		int64_t	m_numFreeTxBuf; /* The number of free TX buffers available. */
		int64_t	m_numScanSucc; /* The number of scanning successes. */
		int64_t	m_numTxVHT_1040_Mbps;
		int64_t	m_numTx_12_Mbps;
		int64_t	m_numRxProbeResp; /* The number of received probe response frames. */
		int64_t	m_numTxDtimMc; /* The number of transmitted DTIM multicast frames. */
		int64_t	m_numTxHT_14_3_Mbps;
		int64_t	m_numTxVHT_1300_Mbps;
		int64_t	m_numRxHT_13_Mbps;
		int64_t	m_numRxHT_117_Mbps;
		int64_t	m_numRxHT_28_7_Mbps;
		int64_t	m_numRxHT_195_Mbps;
		int64_t	m_numTxLdpc; /* The number of total LDPC frames sent. */
		int64_t	m_numRxDropRunt; /* The number of dropped rx frames, runt. */
		int64_t	m_numTxHT_263_2_Mbps;
		int64_t	m_numTxHT_13_5_Mbps;
		int64_t	m_numRxDropNoFcField; /* The number of dropped rx frames, no frame control field. */
		int64_t	m_numTxHT_87_8_Mbps;
		int64_t	m_numTxPowerChanges; /* The number of tx power changes. */
		int64_t	m_numRxVHT_480_Mbps;
		int64_t	m_numRxDataFramesRetried;
		int64_t	m_numRxVHT_1040_Mbps;
		int64_t	m_numTxMultiRetries; /* The number of Tx frames with retries. */
		int64_t	m_numRxDup; /* The number of received duplicated frames. */
		int64_t	m_numTxVHT_648_Mbps;
		int64_t	m_numRxData; /* The number of received data frames. */
		int64_t	m_numTxVHT_450_Mbps;
		int64_t	m_numTxVHT_432_Mbps;
		int64_t	m_numRxHT_26_Mbps;
		int64_t	m_numTxHT_115_5_Mbps;
		int64_t	m_numRxHT_200_Mbps;
		int64_t	m_numTxHT_86_6_Mbps;
		int64_t	m_numRx_54_Mbps;
		int64_t	m_numRxVHT_1950_Mbps;
		int64_t	m_numRxHT_130_3_Mbps;
		int64_t	m_numRxDataFrames;
		int64_t	m_numRxVHT_780_Mbps;
		int64_t	m_numRxHT_54_Mbps;
		int64_t	m_numRxVHT_600_Mbps;
		int64_t	m_numRxDataFrames_300_Mbps;
		int64_t	m_numRxHT_243_Mbps;
		int64_t	m_numRxHT_57_7_Mbps;
		int64_t	m_numRxHT_234_Mbps;
		int64_t	m_numRxDataFrames_12_Mbps;
		int64_t	m_numTxVHT_2106_Mbps;
		int64_t	m_numChannelBusy64s;
		int64_t	m_numTxTimeFramesTransmitted;
		int64_t	m_numRxVHT_3466_8_Mbps;
		int64_t	m_numRxHT_52_Mbps;
		int64_t	m_numTxControl; /* The number of Tx control frames. */
		int64_t	m_numTx_36_Mbps;
		int64_t	m_numRxHT_32_5_Mbps;
		int64_t	m_numTxHT_15_Mbps;
		int64_t	m_numRxDataFrames_1300_Mbps;
		int64_t	m_numRxVHT_450_Mbps;
		int64_t	m_numTxDataFrames_12_Mbps;
		int64_t	m_numTxVHT_468_Mbps;
		int64_t	m_numRxBcMc; /* The number of received Broadcast/Multicast frames. */
		int64_t	m_numRxVHT_800_Mbps;
		int64_t	m_numTxAggrSucc; /* The number of aggregation frames sent successfully. */
		int64_t	m_numTxHT_243_Mbps;
		int64_t	m_numRxAmsduDeaggItmd; /* The number of dropped rx frames, AMSDU deagg intermediate. */
		int64_t	m_numTxVHT_1053_Mbps;
		int64_t	m_numTxHT_90_Mbps;
		int64_t	m_numRxVHT_877_5_Mbps;
		int64_t	m_numTxHT_19_5_Mbps;
		int64_t	m_numTxQueued; /* The number of TX frames queued. */
		int64_t	m_rxLastRssi; /* The RSSI of last frame received. */
		int64_t	m_numRxHT_208_Mbps;
		int64_t	m_numTxRetryDropped; /* The number of TX frame dropped due to retries. */
		int64_t	m_numRxHT_351_Mbps;
		int64_t	m_numTxData; /* The number of Tx data frames. */
		int64_t	m_numRxVHT_1404_Mbps;
		int64_t	m_numRxVHT_1560_Mbps;
		int64_t	m_numRxAmsduDeaggLast; /* The number of dropped rx frames, AMSDU deagg last. */
		int64_t	m_numRxHT_43_2_Mbps;
		int64_t	m_numRxHT_29_2_Mbps;
		int64_t	m_numRxHT_13_5_Mbps;
		int64_t	m_numTxVHT_1053_1_Mbps;
		int64_t	m_numTxHT_65_Mbps;
		int64_t	m_numTxVHT_3120_Mbps;
		int64_t	m_numRxHT_87_8_Mbps;
		int64_t	m_numRxHT_312_Mbps;
		int64_t	m_numTxRtsSucc; /* The number of RTS frames sent successfully . */
		int64_t	m_numTxHT_325_Mbps;
		int64_t	m_numChanChanges; /* The number of channel changes. */
		int64_t	m_numTxSucc; /* The number of frames successfully transmitted. */
		int64_t	m_numTxTimeData;
		int64_t	m_numRxHT_97_5_Mbps;
		int64_t	m_numTx_48_Mbps;
		int64_t	m_numTxVHT_540_Mbps;
		int64_t	m_numTxAggrOneMpdu; /* The number of aggregation frames sent using single MPDU. */
		int64_t	m_numTxHT_360_Mbps;
		int64_t	m_numScanReq; /* The number of scanning requests. */
		int64_t	m_numTxNoAck; /* The number of TX frames failed because of not Acked. */
		int64_t	m_numRxHT_173_1_Mbps;
		int64_t	m_numTxVHT_1733_1_Mbps;
		int64_t	m_numTxHT_104_Mbps;
		int64_t	m_numRxHT_27_Mbps;
		int64_t	m_numTxVHT_325_Mbps;
		int64_t	m_numRcvBcForTx; /* The number of received ethernet and local generated broadcast frames for transmit. */
		int64_t	m_numRxHT_30_Mbps;
		int64_t	m_numRxNoFcsErr; /* The number of received frames without FCS errors. */
		int64_t	m_numTxDataTransmittedRetried;
		int64_t	m_numTxHT_28_8_Mbps;
		int64_t	m_numRadioResets; /* The number of radio resets */
		int64_t	m_numRxHT_135_Mbps;
		int64_t	m_numTxVHT_650_Mbps;
		int64_t	m_numRxVHT_1300_Mbps;
		int64_t	m_numTxHT_346_7_Mbps;
		int64_t	m_numTxVHT_1404_Mbps;
		int64_t	m_numRxHT_81_Mbps;
		int64_t	m_numRxHT_78_Mbps;
		int64_t	m_numTxHT_57_5_Mbps;
		int64_t	m_numTxVHT_400_Mbps;
		int64_t	m_numRxVHT_2600_Mbps;
		int64_t	m_numRxVHT_1170_Mbps;
		int64_t	m_numTxHT_117_1_Mbps;
		int64_t	m_numRx_12_Mbps;
		int64_t	m_numTxDataFrames_300_Mbps;
		int64_t	m_numTxHT_180_Mbps;
		int64_t	m_numTxHT_292_5_Mbps;
		int64_t	m_numRxHT_162_Mbps;
		int64_t	m_numTxVHT_292_5_Mbps;
		int64_t	m_numTxHT_27_Mbps;
		int64_t	m_numRxVHT_364_5_Mbps;
		int64_t	m_numTxVHT_2600_Mbps;
		int64_t	m_numTxHT_97_5_Mbps;
		int64_t	m_numTxSuccRetries; /* The number of successfully transmitted frames with retries. */
		int64_t	m_numTxVHT_780_Mbps;
		int64_t	m_numTxPsUnicast; /* The number of transmitted PS unicast frame. */
		int64_t	m_numRxHT_260_Mbps;
		int64_t	m_numRxDropBadProtocol; /* The number of dropped rx frames, bad protocol. */
		int64_t	m_numRxVHT_866_7_Mbps;
		int64_t	m_numTxVHT_1579_5_Mbps;
		int64_t	m_numRxHT_45_Mbps;
		int64_t	m_numTxHT_175_5_Mbps;
		int64_t	m_numRxControl; /* The number of received control frames. */
		int64_t	m_numRxHT_263_2_Mbps;
		int64_t	m_numRxVHT_1733_4_Mbps;
		int64_t	m_numTxHT_60_Mbps;
		int64_t	m_numTxVHT_405_Mbps;
		int64_t	m_numTxHT_200_Mbps;
		int64_t	m_numTxVHT_1560_Mbps;
		int64_t	m_numTxTime_BC_MC_Data;
		int64_t	m_numRxVHT_390_Mbps;
		int64_t	m_numRadarChanChanges; /* The number of channel changes due to radar detections. */
		int64_t	m_numTxVHT_1950_Mbps;
		int64_t	m_numRxHT_86_8_Mbps;
		int64_t	m_numTxVHT_1872_Mbps;
		int64_t	m_numRxHT_57_5_Mbps;
		int64_t	m_numTxHT_300_Mbps;
		int64_t	m_numTxHT_231_1_Mbps;
		int64_t	m_numRxDataFrames_54_Mbps;
		int64_t	m_numTxRetryAttemps; /* The number of retry tx attempts that have been made */
		int64_t	m_numRxHT_6_5_Mbps;
		int64_t	m_numTx_54_Mbps;
		int64_t	m_numTxHT_173_1_Mbps;
		int64_t	m_numTxFramesTransmitted;
		int64_t	m_numTxVHT_975_Mbps;
		int64_t	m_numRxHT_58_5_Mbps;
		int64_t	m_numRx; /* The number of received frames. */
		int64_t	m_numRxHT_173_3_Mbps;
		int64_t	m_numRx_36_Mbps;
		int64_t	m_numTxHT_7_1_Mbps;
		int64_t	m_numTxBeaconSucc; /* The number of successfully transmitted beacon. */
		int64_t	m_numRxHT_346_7_Mbps;
		int64_t	m_numRxVHT_520_Mbps;
		int64_t	m_numRxVHT_1053_1_Mbps;
		int64_t	m_numTxDataFrames_54_Mbps;
		int64_t	m_numTxDataFrames;
		int64_t	m_numTxHT_208_Mbps;
		int64_t	m_numRxVHT_720_Mbps;
		int64_t	m_numTxHT_26_Mbps;
		int64_t	m_numTxVHT_480_Mbps;
		int64_t	m_numTxVHT_2080_Mbps;
		int64_t	m_numTxHT_351_Mbps;
		int64_t	m_numRxRts; /* The number of received RTS frames. */
		int64_t	m_numTxRtsFail; /* The number of RTS frames failed transmission. */
		int64_t	m_numRxHT_15_Mbps;
		int64_t	m_numRxRetryFrames;
		int64_t	m_numRxHT_175_5_Mbps;
		int64_t	m_numTxHT_288_7_Mbps;
		int64_t	m_numTxHT_29_2_Mbps;
		int64_t	m_numRxHT_144_3_Mbps;
		int64_t	m_numTxRateLimitDrop; /* The number of Tx frames dropped because of rate limit and burst exceeded. */
		int64_t	m_numRxVHT_1579_5_Mbps;
		int64_t	m_numRxDataFrames_1300Plus_Mbps;
		int64_t	m_curEirp; /* The Current EIRP. */
		int64_t	m_numTxHT_57_7_Mbps;
		int64_t	m_numRxHT_86_6_Mbps;
		int64_t	m_numTx_6_Mbps;
		int64_t	m_numTxHT_45_Mbps;
		int64_t	m_numTxHT_216_Mbps;
		int64_t	m_numTxVHT_800_Mbps;
		int64_t	m_numTxVHT_3466_8_Mbps;
		int64_t	m_numTxHT_260_Mbps;
		int64_t	m_numRxHT_108_Mbps;
		int64_t	m_numTxHT_40_5_Mbps;
		int64_t	m_numTxProbeResp; /* The number of TX probe response. */
		int64_t	m_numTxHT_234_Mbps;
		int64_t	m_numTx_1_Mbps;
		int64_t	m_numRxLdpc; /* The number of received LDPC frames. */
		int64_t	m_numTxSuccNoRetry; /* The number of successfully transmitted frames at firt attemp. */
		int64_t	m_numRxHT_21_7_Mbps;
		int64_t	m_numTxHT_162_Mbps;
		int64_t	m_numTxHT_195_Mbps;
		int64_t	m_numRx_6_Mbps;
		int64_t	m_numTxHT_21_7_Mbps;
		int64_t	m_numTxHT_130_3_Mbps;
		int64_t	m_numTxDataFrames_108_Mbps;
		int64_t	m_elevenGProtection; /* 11g protection, 2.4GHz only. */
		int64_t	m_numRxHT_90_Mbps;
		int64_t	m_numRxVHT_486_Mbps;
		int64_t	m_numTxVHT_877_5_Mbps;
		int64_t	m_numRxVHT_540_Mbps;
		int64_t	m_curBackupChannel; /* The current backup channel */
		int64_t	m_numRxVHT_526_5_Mbps;
		int64_t	m_numTxHT_121_5_Mbps;
		int64_t	m_numTxHT_32_5_Mbps;
		int64_t	m_numTxHT_130_Mbps;
		int64_t	m_numRxHT_240_Mbps;
		int64_t	m_numTxHT_150_Mbps;
		int64_t	m_numRxRetry; /* The number of received retry frames. */
		int64_t	m_numRxErr; /* The number of received frames with errors. */
		int64_t	m_numTxHT_72_1_Mbps;
		int64_t	m_numRxVHT_400_Mbps;
		int64_t	m_numRxVHT_2080_Mbps;
		int64_t	m_numTxHT_144_3_Mbps;
		int64_t	m_numRxVHT_403_Mbps;
		int64_t	m_numRxVHT_585_Mbps;
		int64_t	m_numRxHT_117_1_Mbps;
		int64_t	m_numRxHT_7_1_Mbps;
		int64_t	m_numRxCts; /* The number of received CTS frames. */
		int64_t	m_numRxDataFrames_450_Mbps;
		int64_t	m_numRxHT_324_Mbps;
		int64_t	m_numTxHT_120_Mbps;
		int64_t	m_numTx_9_Mbps;
		int64_t	m_numRxVHT_3120_Mbps;
		int64_t	m_numRxHT_180_Mbps;
		int64_t	m_numRx_1_Mbps;
		int64_t	m_numRxDropAmsduNoRcv; /* The number of dropped rx frames, AMSDU no receive. */
		int64_t	m_numTxHT_78_Mbps;
		int64_t	m_numTxVHT_1733_4_Mbps;
		int64_t	m_numRxHT_300_Mbps;
		int64_t	m_numRxVHT_650_Mbps;
		int64_t	m_numTxStbc; /* The number of total STBC frames sent. */
		int64_t	m_numTxDataFrames_1300_Mbps;
		int64_t	m_numTxMultipleRetries;
		int64_t	m_numTxHT_312_Mbps;
		int64_t	m_numTxDataFrames_450_Mbps;
		int64_t	m_numRxHT_14_3_Mbps;
		int64_t	m_numTxCts; /* The number of CTS frames sent. */
		int64_t	m_numRxHT_130_Mbps;
		int64_t	m_numRxVHT_975_Mbps;
		int64_t	m_numTxHT_58_5_Mbps;
		int64_t	m_numRcvFrameForTx; /* The number of received ethernet and local generated frames for transmit. */
		int64_t	m_numTxVHT_520_Mbps;
		int64_t	m_numRxManagement; /* The number of received management frames. */
		int64_t	m_numRxVHT_936_Mbps;
		int64_t	m_numRx_24_Mbps;
		int64_t	m_numTxHT_156_Mbps;
		int64_t	m_numRxHT_270_Mbps;
		int64_t	m_numRxHT_115_5_Mbps;
		int64_t	m_numRxHT_231_1_Mbps;
		int64_t	m_numTx_24_Mbps;
};

class EquipmentPeerStatusData  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		std::string	m_statusDataType;
};

class PortalUser  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const std::string get_password() const { return m_password;}
		void set_password(const std::string & value) { m_password = value; }
		const std::vector<PortalUserRole> get_roles() const { return m_roles;}
		void set_roles(const std::vector<PortalUserRole> & value) { m_roles = value; }
		const std::string get_username() const { return m_username;}
		void set_username(const std::string & value) { m_username = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		int64_t	m_id;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		std::string	m_password;
		std::vector<PortalUserRole>	m_roles;
		std::string	m_username;
};

class WepKey  {
	public:
		const std::string get_txKey() const { return m_txKey;}
		void set_txKey(const std::string & value) { m_txKey = value; }
		const std::string get_txKeyConverted() const { return m_txKeyConverted;}
		void set_txKeyConverted(const std::string & value) { m_txKeyConverted = value; }
		const WepType get_txKeyType() const { return m_txKeyType;}
		void set_txKeyType(const WepType & value) { m_txKeyType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_txKey;
		std::string	m_txKeyConverted;
		WepType	m_txKeyType;
};

class IntegerPerStatusCodeMap  {
	public:
		const int64_t get_disabled() const { return m_disabled;}
		void set_disabled(const int64_t & value) { m_disabled = value; }
		const int64_t get_error() const { return m_error;}
		void set_error(const int64_t & value) { m_error = value; }
		const int64_t get_normal() const { return m_normal;}
		void set_normal(const int64_t & value) { m_normal = value; }
		const int64_t get_requiresAttention() const { return m_requiresAttention;}
		void set_requiresAttention(const int64_t & value) { m_requiresAttention = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_disabled;
		int64_t	m_error;
		int64_t	m_normal;
		int64_t	m_requiresAttention;
};

class RadioBestApSettings  {
	public:
		const int64_t get_dropInSnrPercentage() const { return m_dropInSnrPercentage;}
		void set_dropInSnrPercentage(const int64_t & value) { m_dropInSnrPercentage = value; }
		const int64_t get_minLoadFactor() const { return m_minLoadFactor;}
		void set_minLoadFactor(const int64_t & value) { m_minLoadFactor = value; }
		const bool get_mlComputed() const { return m_mlComputed;}
		void set_mlComputed(const bool & value) { m_mlComputed = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_dropInSnrPercentage;
		int64_t	m_minLoadFactor;
		bool	m_mlComputed;
};

class ServiceAdoptionMetrics  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_dayOfYear() const { return m_dayOfYear;}
		void set_dayOfYear(const int64_t & value) { m_dayOfYear = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_locationId() const { return m_locationId;}
		void set_locationId(const int64_t & value) { m_locationId = value; }
		const int64_t get_month() const { return m_month;}
		void set_month(const int64_t & value) { m_month = value; }
		const int64_t get_numBytesDownstream() const { return m_numBytesDownstream;}
		void set_numBytesDownstream(const int64_t & value) { m_numBytesDownstream = value; }
		const int64_t get_numBytesUpstream() const { return m_numBytesUpstream;}
		void set_numBytesUpstream(const int64_t & value) { m_numBytesUpstream = value; }
		const int64_t get_numUniqueConnectedMacs() const { return m_numUniqueConnectedMacs;}
		void set_numUniqueConnectedMacs(const int64_t & value) { m_numUniqueConnectedMacs = value; }
		const int64_t get_weekOfYear() const { return m_weekOfYear;}
		void set_weekOfYear(const int64_t & value) { m_weekOfYear = value; }
		const int64_t get_year() const { return m_year;}
		void set_year(const int64_t & value) { m_year = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_dayOfYear;
		int64_t	m_equipmentId;
		int64_t	m_locationId;
		int64_t	m_month;
		int64_t	m_numBytesDownstream;
		int64_t	m_numBytesUpstream;
		int64_t	m_numUniqueConnectedMacs; /* number of unique connected MAC addresses for the data point. Note - this number is accurate only at the lowest level of granularity - per AP per day. In case of aggregations - per location/customer or per week/month - this number is just a sum of corresponding datapoints, and it does not account for non-unique MACs in those cases. */
		int64_t	m_weekOfYear;
		int64_t	m_year;
};

class PasspointVenueName  {
	public:
		const std::string get_venueUrl() const { return m_venueUrl;}
		void set_venueUrl(const std::string & value) { m_venueUrl = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_venueUrl;
};

class EquipmentRoutingRecord  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_gatewayId() const { return m_gatewayId;}
		void set_gatewayId(const int64_t & value) { m_gatewayId = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_gatewayId;
		int64_t	m_id;
		int64_t	m_lastModifiedTimestamp;
};

class IntegerPerRadioTypeMap  {
	public:
		const int64_t get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const int64_t & value) { m_is2dot4GHz = value; }
		const int64_t get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const int64_t & value) { m_is5GHz = value; }
		const int64_t get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const int64_t & value) { m_is5GHzL = value; }
		const int64_t get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const int64_t & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_is2dot4GHz;
		int64_t	m_is5GHz;
		int64_t	m_is5GHzL;
		int64_t	m_is5GHzU;
};

class SortColumnsPortalUser  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName; /* id, username,  */
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class RadiusServer  {
	public:
		const std::string get_ipAddress() const { return m_ipAddress;}
		void set_ipAddress(const std::string & value) { m_ipAddress = value; }
		const int64_t get_port() const { return m_port;}
		void set_port(const int64_t & value) { m_port = value; }
		const std::string get_secret() const { return m_secret;}
		void set_secret(const std::string & value) { m_secret = value; }
		const int64_t get_timeout() const { return m_timeout;}
		void set_timeout(const int64_t & value) { m_timeout = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_ipAddress;
		int64_t	m_port;
		std::string	m_secret;
		int64_t	m_timeout;
};

class MinMaxAvgValueInt  {
	public:
		const int64_t get_avgValue() const { return m_avgValue;}
		void set_avgValue(const int64_t & value) { m_avgValue = value; }
		const int64_t get_maxValue() const { return m_maxValue;}
		void set_maxValue(const int64_t & value) { m_maxValue = value; }
		const int64_t get_minValue() const { return m_minValue;}
		void set_minValue(const int64_t & value) { m_minValue = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_avgValue;
		int64_t	m_maxValue;
		int64_t	m_minValue;
};

class SourceSelectionValue  {
	public:
		const SourceType get_source() const { return m_source;}
		void set_source(const SourceType & value) { m_source = value; }
		const int64_t get_value() const { return m_value;}
		void set_value(const int64_t & value) { m_value = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		SourceType	m_source;
		int64_t	m_value;
};

class RadioProfileConfiguration  {
	public:
		const BestAPSteerType get_bestAPSteerType() const { return m_bestAPSteerType;}
		void set_bestAPSteerType(const BestAPSteerType & value) { m_bestAPSteerType = value; }
		const bool get_bestApEnabled() const { return m_bestApEnabled;}
		void set_bestApEnabled(const bool & value) { m_bestApEnabled = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		BestAPSteerType	m_bestAPSteerType;
		bool	m_bestApEnabled;
};

class SortColumnsProfile  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName; /* id, name,  */
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class LongValueMap  {
	public:
		const int64_t get_LongValueMap() const { return m_LongValueMap;}
		void set_LongValueMap(const int64_t & value) { m_LongValueMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_LongValueMap;
};

class SyslogRelay  {
	public:
		const bool get_enabled() const { return m_enabled;}
		void set_enabled(const bool & value) { m_enabled = value; }
		const SyslogSeverityType get_severity() const { return m_severity;}
		void set_severity(const SyslogSeverityType & value) { m_severity = value; }
		const std::string get_srvHostIp() const { return m_srvHostIp;}
		void set_srvHostIp(const std::string & value) { m_srvHostIp = value; }
		const int64_t get_srvHostPort() const { return m_srvHostPort;}
		void set_srvHostPort(const int64_t & value) { m_srvHostPort = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_enabled;
		SyslogSeverityType	m_severity;
		std::string	m_srvHostIp;
		int64_t	m_srvHostPort;
};

class AutoOrManualValue  {
	public:
		const bool get__auto_() const { return m__auto_;}
		void set__auto_(const bool & value) { m__auto_ = value; }
		const int64_t get_value() const { return m_value;}
		void set_value(const int64_t & value) { m_value = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m__auto_;
		int64_t	m_value;
};

class EquipmentAdminStatusData  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const StatusCode get_statusCode() const { return m_statusCode;}
		void set_statusCode(const StatusCode & value) { m_statusCode = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }
		const std::string get_statusMessage() const { return m_statusMessage;}
		void set_statusMessage(const std::string & value) { m_statusMessage = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		StatusCode	m_statusCode;
		std::string	m_statusDataType;
		std::string	m_statusMessage;
};

class RadioBasedSsidConfiguration  {
	public:
		const bool get_enable80211k() const { return m_enable80211k;}
		void set_enable80211k(const bool & value) { m_enable80211k = value; }
		const bool get_enable80211r() const { return m_enable80211r;}
		void set_enable80211r(const bool & value) { m_enable80211r = value; }
		const bool get_enable80211v() const { return m_enable80211v;}
		void set_enable80211v(const bool & value) { m_enable80211v = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_enable80211k;
		bool	m_enable80211r;
		bool	m_enable80211v;
};

class PaginationContextSystemEvent  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class PaginationContextClient  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class Status  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const StatusDataType get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const StatusDataType & value) { m_statusDataType = value; }
		const StatusDetails get_statusDetails() const { return m_statusDetails;}
		void set_statusDetails(const StatusDetails & value) { m_statusDetails = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_lastModifiedTimestamp; /* This class does not perform checks against concurrrent updates. Here last update always wins. */
		std::string	m_model_type;
		StatusDataType	m_statusDataType;
		StatusDetails	m_statusDetails;
};

class TimedAccessUserDetails  {
	public:
		const std::string get_firstName() const { return m_firstName;}
		void set_firstName(const std::string & value) { m_firstName = value; }
		const std::string get_lastName() const { return m_lastName;}
		void set_lastName(const std::string & value) { m_lastName = value; }
		const bool get_passwordNeedsReset() const { return m_passwordNeedsReset;}
		void set_passwordNeedsReset(const bool & value) { m_passwordNeedsReset = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_firstName;
		std::string	m_lastName;
		bool	m_passwordNeedsReset;
};

class EquipmentNeighbouringStatusData  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		std::string	m_statusDataType;
};

class BannedChannel  {
	public:
		const int64_t get_bannedOnEpoc() const { return m_bannedOnEpoc;}
		void set_bannedOnEpoc(const int64_t & value) { m_bannedOnEpoc = value; }
		const int64_t get_channelNumber() const { return m_channelNumber;}
		void set_channelNumber(const int64_t & value) { m_channelNumber = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_bannedOnEpoc;
		int64_t	m_channelNumber;
};

class EquipmentPerformanceDetails  {
	public:
		const int64_t get_avgCpuTemperature() const { return m_avgCpuTemperature;}
		void set_avgCpuTemperature(const int64_t & value) { m_avgCpuTemperature = value; }
		const int64_t get_avgCpuUtilCore1() const { return m_avgCpuUtilCore1;}
		void set_avgCpuUtilCore1(const int64_t & value) { m_avgCpuUtilCore1 = value; }
		const int64_t get_avgCpuUtilCore2() const { return m_avgCpuUtilCore2;}
		void set_avgCpuUtilCore2(const int64_t & value) { m_avgCpuUtilCore2 = value; }
		const int64_t get_avgFreeMemory() const { return m_avgFreeMemory;}
		void set_avgFreeMemory(const int64_t & value) { m_avgFreeMemory = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_avgCpuTemperature;
		int64_t	m_avgCpuUtilCore1;
		int64_t	m_avgCpuUtilCore2;
		int64_t	m_avgFreeMemory;
};

class LocationActivityDetails  {
	public:
		const std::string get_busyTime() const { return m_busyTime;}
		void set_busyTime(const std::string & value) { m_busyTime = value; }
		const int64_t get_lastBusySnapshot() const { return m_lastBusySnapshot;}
		void set_lastBusySnapshot(const int64_t & value) { m_lastBusySnapshot = value; }
		const std::string get_quietTime() const { return m_quietTime;}
		void set_quietTime(const std::string & value) { m_quietTime = value; }
		const std::string get_timezone() const { return m_timezone;}
		void set_timezone(const std::string & value) { m_timezone = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_busyTime;
		int64_t	m_lastBusySnapshot;
		std::string	m_quietTime;
		std::string	m_timezone;
};

class PaginationContextClientSession  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class ManufacturerOuiDetails  {
	public:
		const std::string get_manufacturerAlias() const { return m_manufacturerAlias;}
		void set_manufacturerAlias(const std::string & value) { m_manufacturerAlias = value; }
		const std::string get_manufacturerName() const { return m_manufacturerName;}
		void set_manufacturerName(const std::string & value) { m_manufacturerName = value; }
		const std::string get_oui() const { return m_oui;}
		void set_oui(const std::string & value) { m_oui = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_manufacturerAlias;
		std::string	m_manufacturerName;
		std::string	m_oui; /* first 3 bytes of MAC address, expressed as a string like '1a2b3c' */
};

class ChannelHopSettings  {
	public:
		const int64_t get_noiseFloorThresholdInDB() const { return m_noiseFloorThresholdInDB;}
		void set_noiseFloorThresholdInDB(const int64_t & value) { m_noiseFloorThresholdInDB = value; }
		const int64_t get_noiseFloorThresholdTimeInSeconds() const { return m_noiseFloorThresholdTimeInSeconds;}
		void set_noiseFloorThresholdTimeInSeconds(const int64_t & value) { m_noiseFloorThresholdTimeInSeconds = value; }
		const int64_t get_nonWifiThresholdInPercentage() const { return m_nonWifiThresholdInPercentage;}
		void set_nonWifiThresholdInPercentage(const int64_t & value) { m_nonWifiThresholdInPercentage = value; }
		const int64_t get_nonWifiThresholdTimeInSeconds() const { return m_nonWifiThresholdTimeInSeconds;}
		void set_nonWifiThresholdTimeInSeconds(const int64_t & value) { m_nonWifiThresholdTimeInSeconds = value; }
		const ObssHopMode get_obssHopMode() const { return m_obssHopMode;}
		void set_obssHopMode(const ObssHopMode & value) { m_obssHopMode = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_noiseFloorThresholdInDB;
		int64_t	m_noiseFloorThresholdTimeInSeconds;
		int64_t	m_nonWifiThresholdInPercentage;
		int64_t	m_nonWifiThresholdTimeInSeconds;
		ObssHopMode	m_obssHopMode;
};

class SourceSelectionMulticast  {
	public:
		const SourceType get_source() const { return m_source;}
		void set_source(const SourceType & value) { m_source = value; }
		const MulticastRate get_value() const { return m_value;}
		void set_value(const MulticastRate & value) { m_value = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		SourceType	m_source;
		MulticastRate	m_value;
};

class LongPerRadioTypeMap  {
	public:
		const int64_t get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const int64_t & value) { m_is2dot4GHz = value; }
		const int64_t get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const int64_t & value) { m_is5GHz = value; }
		const int64_t get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const int64_t & value) { m_is5GHzL = value; }
		const int64_t get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const int64_t & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_is2dot4GHz;
		int64_t	m_is5GHz;
		int64_t	m_is5GHzL;
		int64_t	m_is5GHzU;
};

class NeighbouringAPListConfiguration  {
	public:
		const int64_t get_maxAps() const { return m_maxAps;}
		void set_maxAps(const int64_t & value) { m_maxAps = value; }
		const int64_t get_minSignal() const { return m_minSignal;}
		void set_minSignal(const int64_t & value) { m_minSignal = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_maxAps;
		int64_t	m_minSignal;
};

class PingResponse  {
	public:
		const std::string get_applicationName() const { return m_applicationName;}
		void set_applicationName(const std::string & value) { m_applicationName = value; }
		const std::string get_commitDate() const { return m_commitDate;}
		void set_commitDate(const std::string & value) { m_commitDate = value; }
		const std::string get_commitID() const { return m_commitID;}
		void set_commitID(const std::string & value) { m_commitID = value; }
		const int64_t get_currentTime() const { return m_currentTime;}
		void set_currentTime(const int64_t & value) { m_currentTime = value; }
		const std::string get_hostName() const { return m_hostName;}
		void set_hostName(const std::string & value) { m_hostName = value; }
		const std::string get_projectVersion() const { return m_projectVersion;}
		void set_projectVersion(const std::string & value) { m_projectVersion = value; }
		const int64_t get_startupTime() const { return m_startupTime;}
		void set_startupTime(const int64_t & value) { m_startupTime = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_applicationName;
		std::string	m_commitDate;
		std::string	m_commitID;
		int64_t	m_currentTime;
		std::string	m_hostName;
		std::string	m_projectVersion;
		int64_t	m_startupTime;
};

class EquipmentUpgradeStatusData  {
	public:
		const std::string get_activeSwVersion() const { return m_activeSwVersion;}
		void set_activeSwVersion(const std::string & value) { m_activeSwVersion = value; }
		const std::string get_alternateSwVersion() const { return m_alternateSwVersion;}
		void set_alternateSwVersion(const std::string & value) { m_alternateSwVersion = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const EquipmentUpgradeFailureReason get_reason() const { return m_reason;}
		void set_reason(const EquipmentUpgradeFailureReason & value) { m_reason = value; }
		const int64_t get_retries() const { return m_retries;}
		void set_retries(const int64_t & value) { m_retries = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }
		const bool get_switchBank() const { return m_switchBank;}
		void set_switchBank(const bool & value) { m_switchBank = value; }
		const std::string get_targetSwVersion() const { return m_targetSwVersion;}
		void set_targetSwVersion(const std::string & value) { m_targetSwVersion = value; }
		const int64_t get_upgradeStartTime() const { return m_upgradeStartTime;}
		void set_upgradeStartTime(const int64_t & value) { m_upgradeStartTime = value; }
		const EquipmentUpgradeState get_upgradeState() const { return m_upgradeState;}
		void set_upgradeState(const EquipmentUpgradeState & value) { m_upgradeState = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_activeSwVersion;
		std::string	m_alternateSwVersion;
		std::string	m_model_type;
		EquipmentUpgradeFailureReason	m_reason;
		int64_t	m_retries;
		std::string	m_statusDataType;
		bool	m_switchBank;
		std::string	m_targetSwVersion;
		int64_t	m_upgradeStartTime;
		EquipmentUpgradeState	m_upgradeState;
};

class SortColumnsClientSession  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName; /* customerId, equipmentId, macAddress,  */
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class SortColumnsAlarm  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName; /* equipmentId, alarmCode, createdTimestamp,  */
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class SortColumnsSystemEvent  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName; /* equipmentId, dataType, createdTimestamp,  */
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class ServiceMetricConfigParameters  {
	public:
		const int64_t get_reportingIntervalSeconds() const { return m_reportingIntervalSeconds;}
		void set_reportingIntervalSeconds(const int64_t & value) { m_reportingIntervalSeconds = value; }
		const int64_t get_samplingInterval() const { return m_samplingInterval;}
		void set_samplingInterval(const int64_t & value) { m_samplingInterval = value; }
		const ServiceMetricDataType get_serviceMetricDataType() const { return m_serviceMetricDataType;}
		void set_serviceMetricDataType(const ServiceMetricDataType & value) { m_serviceMetricDataType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_reportingIntervalSeconds;
		int64_t	m_samplingInterval;
		ServiceMetricDataType	m_serviceMetricDataType;
};

class GenericResponse  {
	public:
		const std::string get_message() const { return m_message;}
		void set_message(const std::string & value) { m_message = value; }
		const bool get_success() const { return m_success;}
		void set_success(const bool & value) { m_success = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_message;
		bool	m_success;
};

class WebTokenRequest  {
	public:
		const std::string get_password() const { return m_password;}
		void set_password(const std::string & value) { m_password = value; }
		const std::string get_refreshToken() const { return m_refreshToken;}
		void set_refreshToken(const std::string & value) { m_refreshToken = value; }
		const std::string get_userId() const { return m_userId;}
		void set_userId(const std::string & value) { m_userId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_password;
		std::string	m_refreshToken;
		std::string	m_userId;
};

class PaginationContextEquipment  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class ManagedFileInfo  {
	public:
		const bool get_altSlot() const { return m_altSlot;}
		void set_altSlot(const bool & value) { m_altSlot = value; }
		const std::string get_apExportUrl() const { return m_apExportUrl;}
		void set_apExportUrl(const std::string & value) { m_apExportUrl = value; }
		const FileCategory get_fileCategory() const { return m_fileCategory;}
		void set_fileCategory(const FileCategory & value) { m_fileCategory = value; }
		const FileType get_fileType() const { return m_fileType;}
		void set_fileType(const FileType & value) { m_fileType = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const std::vector<int64_t> get_md5checksum() const { return m_md5checksum;}
		void set_md5checksum(const std::vector<int64_t> & value) { m_md5checksum = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_altSlot;
		std::string	m_apExportUrl;
		FileCategory	m_fileCategory;
		FileType	m_fileType;
		int64_t	m_lastModifiedTimestamp;
		std::vector<int64_t>	m_md5checksum;
};

class TrafficPerRadioDetails  {
	public:
		const float get_avgRxMbps() const { return m_avgRxMbps;}
		void set_avgRxMbps(const float & value) { m_avgRxMbps = value; }
		const float get_avgTxMbps() const { return m_avgTxMbps;}
		void set_avgTxMbps(const float & value) { m_avgTxMbps = value; }
		const float get_maxRxMbps() const { return m_maxRxMbps;}
		void set_maxRxMbps(const float & value) { m_maxRxMbps = value; }
		const float get_maxTxMbps() const { return m_maxTxMbps;}
		void set_maxTxMbps(const float & value) { m_maxTxMbps = value; }
		const float get_minRxMbps() const { return m_minRxMbps;}
		void set_minRxMbps(const float & value) { m_minRxMbps = value; }
		const float get_minTxMbps() const { return m_minTxMbps;}
		void set_minTxMbps(const float & value) { m_minTxMbps = value; }
		const int64_t get_numBadEquipment() const { return m_numBadEquipment;}
		void set_numBadEquipment(const int64_t & value) { m_numBadEquipment = value; }
		const int64_t get_numGoodEquipment() const { return m_numGoodEquipment;}
		void set_numGoodEquipment(const int64_t & value) { m_numGoodEquipment = value; }
		const int64_t get_numWarnEquipment() const { return m_numWarnEquipment;}
		void set_numWarnEquipment(const int64_t & value) { m_numWarnEquipment = value; }
		const int64_t get_totalApsReported() const { return m_totalApsReported;}
		void set_totalApsReported(const int64_t & value) { m_totalApsReported = value; }
		const float get_totalRxMbps() const { return m_totalRxMbps;}
		void set_totalRxMbps(const float & value) { m_totalRxMbps = value; }
		const float get_totalTxMbps() const { return m_totalTxMbps;}
		void set_totalTxMbps(const float & value) { m_totalTxMbps = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		float	m_avgRxMbps;
		float	m_avgTxMbps;
		float	m_maxRxMbps;
		float	m_maxTxMbps;
		float	m_minRxMbps;
		float	m_minTxMbps;
		int64_t	m_numBadEquipment;
		int64_t	m_numGoodEquipment;
		int64_t	m_numWarnEquipment;
		int64_t	m_totalApsReported;
		float	m_totalRxMbps;
		float	m_totalTxMbps;
};

class UnserializableSystemEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_payload() const { return m_payload;}
		void set_payload(const std::string & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		std::string	m_payload;
};

class PairLongLong  {
	public:
		const int64_t get_value1() const { return m_value1;}
		void set_value1(const int64_t & value) { m_value1 = value; }
		const int64_t get_value2() const { return m_value2;}
		void set_value2(const int64_t & value) { m_value2 = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_value1;
		int64_t	m_value2;
};

class PerProcessUtilization  {
	public:
		const std::string get_cmd() const { return m_cmd;}
		void set_cmd(const std::string & value) { m_cmd = value; }
		const int64_t get_pid() const { return m_pid;}
		void set_pid(const int64_t & value) { m_pid = value; }
		const int64_t get_util() const { return m_util;}
		void set_util(const int64_t & value) { m_util = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cmd; /* process name */
		int64_t	m_pid; /* process id */
		int64_t	m_util; /* utilization, either as a percentage (i.e. for CPU) or in kB (for memory) */
};

class MeshGroupMember  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const ApMeshMode get_mashMode() const { return m_mashMode;}
		void set_mashMode(const ApMeshMode & value) { m_mashMode = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_equipmentId;
		int64_t	m_lastModifiedTimestamp;
		ApMeshMode	m_mashMode;
};

class FirmwareTrackRecord  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const FirmwareScheduleSetting get_maintenanceWindow() const { return m_maintenanceWindow;}
		void set_maintenanceWindow(const FirmwareScheduleSetting & value) { m_maintenanceWindow = value; }
		const int64_t get_recordId() const { return m_recordId;}
		void set_recordId(const int64_t & value) { m_recordId = value; }
		const std::string get_trackName() const { return m_trackName;}
		void set_trackName(const std::string & value) { m_trackName = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		FirmwareScheduleSetting	m_maintenanceWindow;
		int64_t	m_recordId;
		std::string	m_trackName;
};

class BlocklistDetails  {
	public:
		const bool get_enabled() const { return m_enabled;}
		void set_enabled(const bool & value) { m_enabled = value; }
		const int64_t get_endTime() const { return m_endTime;}
		void set_endTime(const int64_t & value) { m_endTime = value; }
		const int64_t get_startTime() const { return m_startTime;}
		void set_startTime(const int64_t & value) { m_startTime = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_enabled; /* When enabled, blocklisting applies to the client, subject to the optional start/end times. */
		int64_t	m_endTime; /* Optional endTime when blocklisting ceases to be enabled */
		int64_t	m_startTime; /* Optional startTime when blocklisting becomes enabled. */
};

class JsonSerializedException  {
	public:
		const std::string get_error() const { return m_error;}
		void set_error(const std::string & value) { m_error = value; }
		const std::string get_exType() const { return m_exType;}
		void set_exType(const std::string & value) { m_exType = value; }
		const std::string get_path() const { return m_path;}
		void set_path(const std::string & value) { m_path = value; }
		const int64_t get_timestamp() const { return m_timestamp;}
		void set_timestamp(const int64_t & value) { m_timestamp = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_error; /* error message */
		std::string	m_exType;
		std::string	m_path; /* API path with parameters that produced the exception */
		int64_t	m_timestamp; /* time stamp of when the exception was generated */
};

class RadiusNasConfiguration  {
	public:
		const std::string get_nasClientId() const { return m_nasClientId;}
		void set_nasClientId(const std::string & value) { m_nasClientId = value; }
		const std::string get_nasClientIp() const { return m_nasClientIp;}
		void set_nasClientIp(const std::string & value) { m_nasClientIp = value; }
		const std::string get_operatorId() const { return m_operatorId;}
		void set_operatorId(const std::string & value) { m_operatorId = value; }
		const std::string get_userDefinedNasId() const { return m_userDefinedNasId;}
		void set_userDefinedNasId(const std::string & value) { m_userDefinedNasId = value; }
		const std::string get_userDefinedNasIp() const { return m_userDefinedNasIp;}
		void set_userDefinedNasIp(const std::string & value) { m_userDefinedNasIp = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_nasClientId; /* DEFAULT, USER_DEFINED,  */
		std::string	m_nasClientIp; /* USER_DEFINED, WAN_IP, PROXY_IP,  */
		std::string	m_operatorId; /* Carries the operator namespace identifier and the operator name.  The operator name is combined with the namespace identifier to uniquely identify the owner of an access network.  The value of the Operator-Name is a non-NULL terminated text. This is not to be confused with the Passpoint Operator Domain */
		std::string	m_userDefinedNasId; /* user entered string if the nasClientId is 'USER_DEFINED'. This should not be enabled and will not be passed to the AP unless the nasClientId is USER_DEFINED. */
		std::string	m_userDefinedNasIp; /* user entered IP address if the nasClientIp is 'USER_DEFINED'.  This should not be enabled and will not be passed to the AP unless the nasClientIp is USER_DEFINED. */
};

class PaginationContextPortalUser  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class RtlsSettings  {
	public:
		const bool get_enabled() const { return m_enabled;}
		void set_enabled(const bool & value) { m_enabled = value; }
		const std::string get_srvHostIp() const { return m_srvHostIp;}
		void set_srvHostIp(const std::string & value) { m_srvHostIp = value; }
		const int64_t get_srvHostPort() const { return m_srvHostPort;}
		void set_srvHostPort(const int64_t & value) { m_srvHostPort = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_enabled;
		std::string	m_srvHostIp;
		int64_t	m_srvHostPort;
};

class PaginationContextLocation  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class IntegerStatusCodeMap  {
	public:
		const int64_t get_disabled() const { return m_disabled;}
		void set_disabled(const int64_t & value) { m_disabled = value; }
		const int64_t get_error() const { return m_error;}
		void set_error(const int64_t & value) { m_error = value; }
		const int64_t get_normal() const { return m_normal;}
		void set_normal(const int64_t & value) { m_normal = value; }
		const int64_t get_requiresAttention() const { return m_requiresAttention;}
		void set_requiresAttention(const int64_t & value) { m_requiresAttention = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_disabled;
		int64_t	m_error;
		int64_t	m_normal;
		int64_t	m_requiresAttention;
};

class GreTunnelConfiguration  {
	public:
		const std::string get_greRemoteInetAddr() const { return m_greRemoteInetAddr;}
		void set_greRemoteInetAddr(const std::string & value) { m_greRemoteInetAddr = value; }
		const std::string get_greTunnelName() const { return m_greTunnelName;}
		void set_greTunnelName(const std::string & value) { m_greTunnelName = value; }
		const std::vector<int64_t> get_vlanIdsInGreTunnel() const { return m_vlanIdsInGreTunnel;}
		void set_vlanIdsInGreTunnel(const std::vector<int64_t> & value) { m_vlanIdsInGreTunnel = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_greRemoteInetAddr;
		std::string	m_greTunnelName;
		std::vector<int64_t>	m_vlanIdsInGreTunnel;
};

class ChannelInfo  {
	public:
		const ChannelBandwidth get_bandwidth() const { return m_bandwidth;}
		void set_bandwidth(const ChannelBandwidth & value) { m_bandwidth = value; }
		const int64_t get_chanNumber() const { return m_chanNumber;}
		void set_chanNumber(const int64_t & value) { m_chanNumber = value; }
		const int64_t get_noiseFloor() const { return m_noiseFloor;}
		void set_noiseFloor(const int64_t & value) { m_noiseFloor = value; }
		const int64_t get_totalUtilization() const { return m_totalUtilization;}
		void set_totalUtilization(const int64_t & value) { m_totalUtilization = value; }
		const int64_t get_wifiUtilization() const { return m_wifiUtilization;}
		void set_wifiUtilization(const int64_t & value) { m_wifiUtilization = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ChannelBandwidth	m_bandwidth;
		int64_t	m_chanNumber;
		int64_t	m_noiseFloor;
		int64_t	m_totalUtilization;
		int64_t	m_wifiUtilization;
};

class ServiceMetricSurveyConfigParameters  {
	public:
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const int64_t get_reportingIntervalSeconds() const { return m_reportingIntervalSeconds;}
		void set_reportingIntervalSeconds(const int64_t & value) { m_reportingIntervalSeconds = value; }
		const int64_t get_samplingInterval() const { return m_samplingInterval;}
		void set_samplingInterval(const int64_t & value) { m_samplingInterval = value; }
		const ServiceMetricDataType get_serviceMetricDataType() const { return m_serviceMetricDataType;}
		void set_serviceMetricDataType(const ServiceMetricDataType & value) { m_serviceMetricDataType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RadioType	m_radioType;
		int64_t	m_reportingIntervalSeconds;
		int64_t	m_samplingInterval;
		ServiceMetricDataType	m_serviceMetricDataType;
};

class EquipmentScanDetails  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		std::string	m_statusDataType;
};

class ActiveBSSID  {
	public:
		const std::string get_bssid() const { return m_bssid;}
		void set_bssid(const std::string & value) { m_bssid = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_numDevicesConnected() const { return m_numDevicesConnected;}
		void set_numDevicesConnected(const int64_t & value) { m_numDevicesConnected = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_bssid;
		std::string	m_model_type;
		int64_t	m_numDevicesConnected;
		RadioType	m_radioType;
		std::string	m_ssid;
};

class SortColumnsStatus  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName; /* customerId, equipmentId,  */
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class WmmQueueStats  {
	public:
		const int64_t get_forwardBytes() const { return m_forwardBytes;}
		void set_forwardBytes(const int64_t & value) { m_forwardBytes = value; }
		const int64_t get_forwardFrames() const { return m_forwardFrames;}
		void set_forwardFrames(const int64_t & value) { m_forwardFrames = value; }
		const WmmQueueType get_queueType() const { return m_queueType;}
		void set_queueType(const WmmQueueType & value) { m_queueType = value; }
		const int64_t get_rxBytes() const { return m_rxBytes;}
		void set_rxBytes(const int64_t & value) { m_rxBytes = value; }
		const int64_t get_rxFailedBytes() const { return m_rxFailedBytes;}
		void set_rxFailedBytes(const int64_t & value) { m_rxFailedBytes = value; }
		const int64_t get_rxFailedFrames() const { return m_rxFailedFrames;}
		void set_rxFailedFrames(const int64_t & value) { m_rxFailedFrames = value; }
		const int64_t get_rxFrames() const { return m_rxFrames;}
		void set_rxFrames(const int64_t & value) { m_rxFrames = value; }
		const int64_t get_txBytes() const { return m_txBytes;}
		void set_txBytes(const int64_t & value) { m_txBytes = value; }
		const int64_t get_txExpiredBytes() const { return m_txExpiredBytes;}
		void set_txExpiredBytes(const int64_t & value) { m_txExpiredBytes = value; }
		const int64_t get_txExpiredFrames() const { return m_txExpiredFrames;}
		void set_txExpiredFrames(const int64_t & value) { m_txExpiredFrames = value; }
		const int64_t get_txFailedBytes() const { return m_txFailedBytes;}
		void set_txFailedBytes(const int64_t & value) { m_txFailedBytes = value; }
		const int64_t get_txFailedFrames() const { return m_txFailedFrames;}
		void set_txFailedFrames(const int64_t & value) { m_txFailedFrames = value; }
		const int64_t get_txFrames() const { return m_txFrames;}
		void set_txFrames(const int64_t & value) { m_txFrames = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_forwardBytes;
		int64_t	m_forwardFrames;
		WmmQueueType	m_queueType;
		int64_t	m_rxBytes;
		int64_t	m_rxFailedBytes;
		int64_t	m_rxFailedFrames;
		int64_t	m_rxFrames;
		int64_t	m_txBytes;
		int64_t	m_txExpiredBytes;
		int64_t	m_txExpiredFrames;
		int64_t	m_txFailedBytes;
		int64_t	m_txFailedFrames;
		int64_t	m_txFrames;
};

class MeshGroupProperty  {
	public:
		const bool get_ethernetProtection() const { return m_ethernetProtection;}
		void set_ethernetProtection(const bool & value) { m_ethernetProtection = value; }
		const int64_t get_locationId() const { return m_locationId;}
		void set_locationId(const int64_t & value) { m_locationId = value; }
		const std::string get_name() const { return m_name;}
		void set_name(const std::string & value) { m_name = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_ethernetProtection;
		int64_t	m_locationId;
		std::string	m_name;
};

class PaginationContextProfile  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class PaginationContextAlarm  {
	public:
		const std::string get_cursor() const { return m_cursor;}
		void set_cursor(const std::string & value) { m_cursor = value; }
		const bool get_lastPage() const { return m_lastPage;}
		void set_lastPage(const bool & value) { m_lastPage = value; }
		const int64_t get_lastReturnedPageNumber() const { return m_lastReturnedPageNumber;}
		void set_lastReturnedPageNumber(const int64_t & value) { m_lastReturnedPageNumber = value; }
		const int64_t get_maxItemsPerPage() const { return m_maxItemsPerPage;}
		void set_maxItemsPerPage(const int64_t & value) { m_maxItemsPerPage = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_totalItemsReturned() const { return m_totalItemsReturned;}
		void set_totalItemsReturned(const int64_t & value) { m_totalItemsReturned = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_cursor;
		bool	m_lastPage;
		int64_t	m_lastReturnedPageNumber;
		int64_t	m_maxItemsPerPage;
		std::string	m_model_type;
		int64_t	m_totalItemsReturned;
};

class SortColumnsServiceMetric  {
	public:
		const std::string get_columnName() const { return m_columnName;}
		void set_columnName(const std::string & value) { m_columnName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SortOrder get_sortOrder() const { return m_sortOrder;}
		void set_sortOrder(const SortOrder & value) { m_sortOrder = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_columnName; /* equipmentId, clientMac, dataType, createdTimestamp,  */
		std::string	m_model_type;
		SortOrder	m_sortOrder;
};

class ServiceMetricRadioConfigParameters  {
	public:
		const ChannelUtilizationSurveyType get_channelSurveyType() const { return m_channelSurveyType;}
		void set_channelSurveyType(const ChannelUtilizationSurveyType & value) { m_channelSurveyType = value; }
		const int64_t get_delayMillisecondsThreshold() const { return m_delayMillisecondsThreshold;}
		void set_delayMillisecondsThreshold(const int64_t & value) { m_delayMillisecondsThreshold = value; }
		const int64_t get_percentUtilizationThreshold() const { return m_percentUtilizationThreshold;}
		void set_percentUtilizationThreshold(const int64_t & value) { m_percentUtilizationThreshold = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const int64_t get_reportingIntervalSeconds() const { return m_reportingIntervalSeconds;}
		void set_reportingIntervalSeconds(const int64_t & value) { m_reportingIntervalSeconds = value; }
		const int64_t get_samplingInterval() const { return m_samplingInterval;}
		void set_samplingInterval(const int64_t & value) { m_samplingInterval = value; }
		const int64_t get_scanIntervalMillis() const { return m_scanIntervalMillis;}
		void set_scanIntervalMillis(const int64_t & value) { m_scanIntervalMillis = value; }
		const ServiceMetricDataType get_serviceMetricDataType() const { return m_serviceMetricDataType;}
		void set_serviceMetricDataType(const ServiceMetricDataType & value) { m_serviceMetricDataType = value; }
		const StatsReportFormat get_statsReportFormat() const { return m_statsReportFormat;}
		void set_statsReportFormat(const StatsReportFormat & value) { m_statsReportFormat = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ChannelUtilizationSurveyType	m_channelSurveyType;
		int64_t	m_delayMillisecondsThreshold;
		int64_t	m_percentUtilizationThreshold;
		RadioType	m_radioType;
		int64_t	m_reportingIntervalSeconds;
		int64_t	m_samplingInterval;
		int64_t	m_scanIntervalMillis;
		ServiceMetricDataType	m_serviceMetricDataType;
		StatsReportFormat	m_statsReportFormat;
};

class EquipmentCapacityDetails  {
	public:
		const int64_t get_availableCapacity() const { return m_availableCapacity;}
		void set_availableCapacity(const int64_t & value) { m_availableCapacity = value; }
		const int64_t get_totalCapacity() const { return m_totalCapacity;}
		void set_totalCapacity(const int64_t & value) { m_totalCapacity = value; }
		const int64_t get_unavailableCapacity() const { return m_unavailableCapacity;}
		void set_unavailableCapacity(const int64_t & value) { m_unavailableCapacity = value; }
		const int64_t get_unusedCapacity() const { return m_unusedCapacity;}
		void set_unusedCapacity(const int64_t & value) { m_unusedCapacity = value; }
		const int64_t get_usedCapacity() const { return m_usedCapacity;}
		void set_usedCapacity(const int64_t & value) { m_usedCapacity = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_availableCapacity; /* The percentage of capacity that is available for clients. */
		int64_t	m_totalCapacity; /* A theoretical maximum based on channel bandwidth */
		int64_t	m_unavailableCapacity; /* The percentage of capacity that is not available for clients (e.g. beacons, noise, non-wifi) */
		int64_t	m_unusedCapacity; /* The percentage of the overall capacity that is not being used. */
		int64_t	m_usedCapacity; /* The percentage of the overall capacity that is currently being used by associated clients. */
};

class RadioChannelChangeSettings  {
	public:
		const int64_t get_backupChannel() const { return m_backupChannel;}
		void set_backupChannel(const int64_t & value) { m_backupChannel = value; }
		const int64_t get_primaryChannel() const { return m_primaryChannel;}
		void set_primaryChannel(const int64_t & value) { m_primaryChannel = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_backupChannel;
		int64_t	m_primaryChannel;
};

class CustomerFirmwareTrackSettings  {
	public:
		const TrackFlag get_autoUpgradeDeprecatedDuringMaintenance() const { return m_autoUpgradeDeprecatedDuringMaintenance;}
		void set_autoUpgradeDeprecatedDuringMaintenance(const TrackFlag & value) { m_autoUpgradeDeprecatedDuringMaintenance = value; }
		const TrackFlag get_autoUpgradeDeprecatedOnBind() const { return m_autoUpgradeDeprecatedOnBind;}
		void set_autoUpgradeDeprecatedOnBind(const TrackFlag & value) { m_autoUpgradeDeprecatedOnBind = value; }
		const TrackFlag get_autoUpgradeUnknownDuringMaintenance() const { return m_autoUpgradeUnknownDuringMaintenance;}
		void set_autoUpgradeUnknownDuringMaintenance(const TrackFlag & value) { m_autoUpgradeUnknownDuringMaintenance = value; }
		const TrackFlag get_autoUpgradeUnknownOnBind() const { return m_autoUpgradeUnknownOnBind;}
		void set_autoUpgradeUnknownOnBind(const TrackFlag & value) { m_autoUpgradeUnknownOnBind = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		TrackFlag	m_autoUpgradeDeprecatedDuringMaintenance;
		TrackFlag	m_autoUpgradeDeprecatedOnBind;
		TrackFlag	m_autoUpgradeUnknownDuringMaintenance;
		TrackFlag	m_autoUpgradeUnknownOnBind;
};

class PasspointConnectionCapability  {
	public:
		const PasspointConnectionCapabilitiesIpProtocol get_connectionCapabilitiesIpProtocol() const { return m_connectionCapabilitiesIpProtocol;}
		void set_connectionCapabilitiesIpProtocol(const PasspointConnectionCapabilitiesIpProtocol & value) { m_connectionCapabilitiesIpProtocol = value; }
		const int64_t get_connectionCapabilitiesPortNumber() const { return m_connectionCapabilitiesPortNumber;}
		void set_connectionCapabilitiesPortNumber(const int64_t & value) { m_connectionCapabilitiesPortNumber = value; }
		const PasspointConnectionCapabilitiesStatus get_connectionCapabilitiesStatus() const { return m_connectionCapabilitiesStatus;}
		void set_connectionCapabilitiesStatus(const PasspointConnectionCapabilitiesStatus & value) { m_connectionCapabilitiesStatus = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PasspointConnectionCapabilitiesIpProtocol	m_connectionCapabilitiesIpProtocol;
		int64_t	m_connectionCapabilitiesPortNumber;
		PasspointConnectionCapabilitiesStatus	m_connectionCapabilitiesStatus;
};

class ClientFailureDetails  {
	public:
		const int64_t get_failureTimestamp() const { return m_failureTimestamp;}
		void set_failureTimestamp(const int64_t & value) { m_failureTimestamp = value; }
		const std::string get_reason() const { return m_reason;}
		void set_reason(const std::string & value) { m_reason = value; }
		const int64_t get_reasonCode() const { return m_reasonCode;}
		void set_reasonCode(const int64_t & value) { m_reasonCode = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_failureTimestamp;
		std::string	m_reason;
		int64_t	m_reasonCode;
};

class McsStats  {
	public:
		const McsType get_mcsNum() const { return m_mcsNum;}
		void set_mcsNum(const McsType & value) { m_mcsNum = value; }
		const int64_t get_rate() const { return m_rate;}
		void set_rate(const int64_t & value) { m_rate = value; }
		const int64_t get_rxFrames() const { return m_rxFrames;}
		void set_rxFrames(const int64_t & value) { m_rxFrames = value; }
		const int64_t get_txFrames() const { return m_txFrames;}
		void set_txFrames(const int64_t & value) { m_txFrames = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		McsType	m_mcsNum;
		int64_t	m_rate;
		int64_t	m_rxFrames; /* The number of received frames at this rate. */
		int64_t	m_txFrames; /* The number of successfully transmitted frames at this rate. Do not count failed transmission. */
};

class RealTimeEvent  : public SystemEvent {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
};

class GatewayRemovedEvent  {
	public:
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const EquipmentGatewayRecord get_gateway() const { return m_gateway;}
		void set_gateway(const EquipmentGatewayRecord & value) { m_gateway = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_eventTimestamp;
		EquipmentGatewayRecord	m_gateway;
		std::string	m_model_type;
};

class RealTimeSipCallEventWithStats  : public RealTimeEvent {
	public:
		const int64_t get_associationId() const { return m_associationId;}
		void set_associationId(const int64_t & value) { m_associationId = value; }
		const int64_t get_channel() const { return m_channel;}
		void set_channel(const int64_t & value) { m_channel = value; }
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const std::vector<std::string> get_codecs() const { return m_codecs;}
		void set_codecs(const std::vector<std::string> & value) { m_codecs = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_providerDomain() const { return m_providerDomain;}
		void set_providerDomain(const std::string & value) { m_providerDomain = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const int64_t get_sipCallId() const { return m_sipCallId;}
		void set_sipCallId(const int64_t & value) { m_sipCallId = value; }
		const std::vector<RtpFlowStats> get_statuses() const { return m_statuses;}
		void set_statuses(const std::vector<RtpFlowStats> & value) { m_statuses = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_associationId;
		int64_t	m_channel;
		MacAddress	m_clientMacAddress;
		std::vector<std::string>	m_codecs;
		std::string	m_model_type;
		std::string	m_providerDomain;
		RadioType	m_radioType;
		int64_t	m_sipCallId;
		std::vector<RtpFlowStats>	m_statuses;
};

class WmmQueueStatsPerQueueTypeMap  {
	public:
		const WmmQueueStats get_BE() const { return m_BE;}
		void set_BE(const WmmQueueStats & value) { m_BE = value; }
		const WmmQueueStats get_BK() const { return m_BK;}
		void set_BK(const WmmQueueStats & value) { m_BK = value; }
		const WmmQueueStats get_VI() const { return m_VI;}
		void set_VI(const WmmQueueStats & value) { m_VI = value; }
		const WmmQueueStats get_VO() const { return m_VO;}
		void set_VO(const WmmQueueStats & value) { m_VO = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		WmmQueueStats	m_BE;
		WmmQueueStats	m_BK;
		WmmQueueStats	m_VI;
		WmmQueueStats	m_VO;
};

class ClientIdEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const std::vector<Base64String> get_macAddressBytes() const { return m_macAddressBytes;}
		void set_macAddressBytes(const std::vector<Base64String> & value) { m_macAddressBytes = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const std::string get_userId() const { return m_userId;}
		void set_userId(const std::string & value) { m_userId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMacAddress;
		std::vector<Base64String>	m_macAddressBytes;
		std::string	m_model_type;
		int64_t	m_sessionId;
		std::string	m_userId;
};

class EquipmentCapacityDetailsMap  {
	public:
		const EquipmentCapacityDetails get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const EquipmentCapacityDetails & value) { m_is2dot4GHz = value; }
		const EquipmentCapacityDetails get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const EquipmentCapacityDetails & value) { m_is5GHz = value; }
		const EquipmentCapacityDetails get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const EquipmentCapacityDetails & value) { m_is5GHzL = value; }
		const EquipmentCapacityDetails get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const EquipmentCapacityDetails & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		EquipmentCapacityDetails	m_is2dot4GHz;
		EquipmentCapacityDetails	m_is5GHz;
		EquipmentCapacityDetails	m_is5GHzL;
		EquipmentCapacityDetails	m_is5GHzU;
};

class RoutingChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const EquipmentRoutingRecord get_payload() const { return m_payload;}
		void set_payload(const EquipmentRoutingRecord & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		EquipmentRoutingRecord	m_payload;
};

class ProfileRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Profile get_payload() const { return m_payload;}
		void set_payload(const Profile & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Profile	m_payload;
};

class LinkQualityAggregatedStats  {
	public:
		const int64_t get_averageClientCount() const { return m_averageClientCount;}
		void set_averageClientCount(const int64_t & value) { m_averageClientCount = value; }
		const int64_t get_badClientCount() const { return m_badClientCount;}
		void set_badClientCount(const int64_t & value) { m_badClientCount = value; }
		const int64_t get_goodClientCount() const { return m_goodClientCount;}
		void set_goodClientCount(const int64_t & value) { m_goodClientCount = value; }
		const MinMaxAvgValueInt get_snr() const { return m_snr;}
		void set_snr(const MinMaxAvgValueInt & value) { m_snr = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_averageClientCount;
		int64_t	m_badClientCount;
		int64_t	m_goodClientCount;
		MinMaxAvgValueInt	m_snr;
};

class RadioBasedSsidConfigurationMap  {
	public:
		const RadioBasedSsidConfiguration get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const RadioBasedSsidConfiguration & value) { m_is2dot4GHz = value; }
		const RadioBasedSsidConfiguration get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const RadioBasedSsidConfiguration & value) { m_is5GHz = value; }
		const RadioBasedSsidConfiguration get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const RadioBasedSsidConfiguration & value) { m_is5GHzL = value; }
		const RadioBasedSsidConfiguration get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const RadioBasedSsidConfiguration & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RadioBasedSsidConfiguration	m_is2dot4GHz;
		RadioBasedSsidConfiguration	m_is5GHz;
		RadioBasedSsidConfiguration	m_is5GHzL;
		RadioBasedSsidConfiguration	m_is5GHzU;
};

class WepConfiguration  {
	public:
		const int64_t get_primaryTxKeyId() const { return m_primaryTxKeyId;}
		void set_primaryTxKeyId(const int64_t & value) { m_primaryTxKeyId = value; }
		const WepAuthType get_wepAuthType() const { return m_wepAuthType;}
		void set_wepAuthType(const WepAuthType & value) { m_wepAuthType = value; }
		const std::vector<WepKey> get_wepKeys() const { return m_wepKeys;}
		void set_wepKeys(const std::vector<WepKey> & value) { m_wepKeys = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_primaryTxKeyId;
		WepAuthType	m_wepAuthType;
		std::vector<WepKey>	m_wepKeys;
};

class MinMaxAvgValueIntPerRadioMap  {
	public:
		const MinMaxAvgValueInt get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const MinMaxAvgValueInt & value) { m_is2dot4GHz = value; }
		const MinMaxAvgValueInt get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const MinMaxAvgValueInt & value) { m_is5GHz = value; }
		const MinMaxAvgValueInt get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const MinMaxAvgValueInt & value) { m_is5GHzL = value; }
		const MinMaxAvgValueInt get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const MinMaxAvgValueInt & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MinMaxAvgValueInt	m_is2dot4GHz;
		MinMaxAvgValueInt	m_is5GHz;
		MinMaxAvgValueInt	m_is5GHzL;
		MinMaxAvgValueInt	m_is5GHzU;
};

class ClientInfoDetails  {
	public:
		const std::string get_alias() const { return m_alias;}
		void set_alias(const std::string & value) { m_alias = value; }
		const std::string get_apFingerprint() const { return m_apFingerprint;}
		void set_apFingerprint(const std::string & value) { m_apFingerprint = value; }
		const BlocklistDetails get_blocklistDetails() const { return m_blocklistDetails;}
		void set_blocklistDetails(const BlocklistDetails & value) { m_blocklistDetails = value; }
		const int64_t get_clientType() const { return m_clientType;}
		void set_clientType(const int64_t & value) { m_clientType = value; }
		const bool get_doNotSteer() const { return m_doNotSteer;}
		void set_doNotSteer(const bool & value) { m_doNotSteer = value; }
		const std::string get_hostName() const { return m_hostName;}
		void set_hostName(const std::string & value) { m_hostName = value; }
		const std::string get_lastUsedCpUsername() const { return m_lastUsedCpUsername;}
		void set_lastUsedCpUsername(const std::string & value) { m_lastUsedCpUsername = value; }
		const std::string get_lastUserAgent() const { return m_lastUserAgent;}
		void set_lastUserAgent(const std::string & value) { m_lastUserAgent = value; }
		const std::string get_userName() const { return m_userName;}
		void set_userName(const std::string & value) { m_userName = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_alias;
		std::string	m_apFingerprint;
		BlocklistDetails	m_blocklistDetails;
		int64_t	m_clientType;
		bool	m_doNotSteer;
		std::string	m_hostName;
		std::string	m_lastUsedCpUsername;
		std::string	m_lastUserAgent;
		std::string	m_userName;
};

class GatewayAddedEvent  {
	public:
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const EquipmentGatewayRecord get_gateway() const { return m_gateway;}
		void set_gateway(const EquipmentGatewayRecord & value) { m_gateway = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_eventTimestamp;
		EquipmentGatewayRecord	m_gateway;
		std::string	m_model_type;
};

class EquipmentPerRadioUtilizationDetails  {
	public:
		const MinMaxAvgValueInt get_wifiFromOtherBss() const { return m_wifiFromOtherBss;}
		void set_wifiFromOtherBss(const MinMaxAvgValueInt & value) { m_wifiFromOtherBss = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MinMaxAvgValueInt	m_wifiFromOtherBss;
};

class ListOfRadioUtilizationPerRadioMap  {
	public:
		const std::vector<RadioUtilization> get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const std::vector<RadioUtilization> & value) { m_is2dot4GHz = value; }
		const std::vector<RadioUtilization> get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const std::vector<RadioUtilization> & value) { m_is5GHz = value; }
		const std::vector<RadioUtilization> get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const std::vector<RadioUtilization> & value) { m_is5GHzL = value; }
		const std::vector<RadioUtilization> get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const std::vector<RadioUtilization> & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<RadioUtilization>	m_is2dot4GHz;
		std::vector<RadioUtilization>	m_is5GHz;
		std::vector<RadioUtilization>	m_is5GHzL;
		std::vector<RadioUtilization>	m_is5GHzU;
};

class ClientAssocEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const int64_t get_internalSC() const { return m_internalSC;}
		void set_internalSC(const int64_t & value) { m_internalSC = value; }
		const bool get_isReassociation() const { return m_isReassociation;}
		void set_isReassociation(const bool & value) { m_isReassociation = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const int64_t get_rssi() const { return m_rssi;}
		void set_rssi(const int64_t & value) { m_rssi = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }
		const WlanStatusCode get_status() const { return m_status;}
		void set_status(const WlanStatusCode & value) { m_status = value; }
		const bool get_using11k() const { return m_using11k;}
		void set_using11k(const bool & value) { m_using11k = value; }
		const bool get_using11r() const { return m_using11r;}
		void set_using11r(const bool & value) { m_using11r = value; }
		const bool get_using11v() const { return m_using11v;}
		void set_using11v(const bool & value) { m_using11v = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMacAddress;
		int64_t	m_internalSC;
		bool	m_isReassociation;
		std::string	m_model_type;
		RadioType	m_radioType;
		int64_t	m_rssi;
		int64_t	m_sessionId;
		std::string	m_ssid;
		WlanStatusCode	m_status;
		bool	m_using11k;
		bool	m_using11r;
		bool	m_using11v;
};

class ListOfMacsPerRadioMap  {
	public:
		const std::vector<MacAddress> get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const std::vector<MacAddress> & value) { m_is2dot4GHz = value; }
		const std::vector<MacAddress> get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const std::vector<MacAddress> & value) { m_is5GHz = value; }
		const std::vector<MacAddress> get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const std::vector<MacAddress> & value) { m_is5GHzL = value; }
		const std::vector<MacAddress> get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const std::vector<MacAddress> & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<MacAddress>	m_is2dot4GHz;
		std::vector<MacAddress>	m_is5GHz;
		std::vector<MacAddress>	m_is5GHzL;
		std::vector<MacAddress>	m_is5GHzU;
};

class RoutingAddedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const EquipmentRoutingRecord get_payload() const { return m_payload;}
		void set_payload(const EquipmentRoutingRecord & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		EquipmentRoutingRecord	m_payload;
};

class RealtimeChannelHopEvent  : public RealTimeEvent {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_newChannel() const { return m_newChannel;}
		void set_newChannel(const int64_t & value) { m_newChannel = value; }
		const int64_t get_oldChannel() const { return m_oldChannel;}
		void set_oldChannel(const int64_t & value) { m_oldChannel = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const ChannelHopReason get_reasonCode() const { return m_reasonCode;}
		void set_reasonCode(const ChannelHopReason & value) { m_reasonCode = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		int64_t	m_newChannel;
		int64_t	m_oldChannel;
		RadioType	m_radioType;
		ChannelHopReason	m_reasonCode;
};

class CapacityPerRadioDetails  {
	public:
		const MinMaxAvgValueInt get_availableCapacity() const { return m_availableCapacity;}
		void set_availableCapacity(const MinMaxAvgValueInt & value) { m_availableCapacity = value; }
		const int64_t get_totalCapacity() const { return m_totalCapacity;}
		void set_totalCapacity(const int64_t & value) { m_totalCapacity = value; }
		const MinMaxAvgValueInt get_unavailableCapacity() const { return m_unavailableCapacity;}
		void set_unavailableCapacity(const MinMaxAvgValueInt & value) { m_unavailableCapacity = value; }
		const MinMaxAvgValueInt get_unusedCapacity() const { return m_unusedCapacity;}
		void set_unusedCapacity(const MinMaxAvgValueInt & value) { m_unusedCapacity = value; }
		const MinMaxAvgValueInt get_usedCapacity() const { return m_usedCapacity;}
		void set_usedCapacity(const MinMaxAvgValueInt & value) { m_usedCapacity = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MinMaxAvgValueInt	m_availableCapacity;
		int64_t	m_totalCapacity;
		MinMaxAvgValueInt	m_unavailableCapacity;
		MinMaxAvgValueInt	m_unusedCapacity;
		MinMaxAvgValueInt	m_usedCapacity;
};

class NeighbourReport  {
	public:
		const RadioType get_RadioType_radioType() const { return m_RadioType_radioType;}
		void set_RadioType_radioType(const RadioType & value) { m_RadioType_radioType = value; }
		const bool get_acMode() const { return m_acMode;}
		void set_acMode(const bool & value) { m_acMode = value; }
		const bool get_bMode() const { return m_bMode;}
		void set_bMode(const bool & value) { m_bMode = value; }
		const int64_t get_beaconInterval() const { return m_beaconInterval;}
		void set_beaconInterval(const int64_t & value) { m_beaconInterval = value; }
		const int64_t get_channel() const { return m_channel;}
		void set_channel(const int64_t & value) { m_channel = value; }
		const MacAddress get_macAddress() const { return m_macAddress;}
		void set_macAddress(const MacAddress & value) { m_macAddress = value; }
		const bool get_nMode() const { return m_nMode;}
		void set_nMode(const bool & value) { m_nMode = value; }
		const NetworkType get_networkType() const { return m_networkType;}
		void set_networkType(const NetworkType & value) { m_networkType = value; }
		const NeighborScanPacketType get_packetType() const { return m_packetType;}
		void set_packetType(const NeighborScanPacketType & value) { m_packetType = value; }
		const bool get_privacy() const { return m_privacy;}
		void set_privacy(const bool & value) { m_privacy = value; }
		const int64_t get_rate() const { return m_rate;}
		void set_rate(const int64_t & value) { m_rate = value; }
		const int64_t get_rssi() const { return m_rssi;}
		void set_rssi(const int64_t & value) { m_rssi = value; }
		const int64_t get_scanTimeInSeconds() const { return m_scanTimeInSeconds;}
		void set_scanTimeInSeconds(const int64_t & value) { m_scanTimeInSeconds = value; }
		const DetectedAuthMode get_secureMode() const { return m_secureMode;}
		void set_secureMode(const DetectedAuthMode & value) { m_secureMode = value; }
		const int64_t get_signal() const { return m_signal;}
		void set_signal(const int64_t & value) { m_signal = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RadioType	m_RadioType_radioType;
		bool	m_acMode;
		bool	m_bMode;
		int64_t	m_beaconInterval;
		int64_t	m_channel;
		MacAddress	m_macAddress;
		bool	m_nMode;
		NetworkType	m_networkType;
		NeighborScanPacketType	m_packetType;
		bool	m_privacy;
		int64_t	m_rate;
		int64_t	m_rssi;
		int64_t	m_scanTimeInSeconds;
		DetectedAuthMode	m_secureMode;
		int64_t	m_signal;
		std::string	m_ssid;
};

class EquipmentProtocolStatusData  {
	public:
		const std::string get_bandPlan() const { return m_bandPlan;}
		void set_bandPlan(const std::string & value) { m_bandPlan = value; }
		const MacAddress get_baseMacAddress() const { return m_baseMacAddress;}
		void set_baseMacAddress(const MacAddress & value) { m_baseMacAddress = value; }
		const int64_t get_cloudCfgDataVersion() const { return m_cloudCfgDataVersion;}
		void set_cloudCfgDataVersion(const int64_t & value) { m_cloudCfgDataVersion = value; }
		const std::string get_cloudProtocolVersion() const { return m_cloudProtocolVersion;}
		void set_cloudProtocolVersion(const std::string & value) { m_cloudProtocolVersion = value; }
		const std::string get_countryCode() const { return m_countryCode;}
		void set_countryCode(const std::string & value) { m_countryCode = value; }
		const std::string get_ipBasedConfiguration() const { return m_ipBasedConfiguration;}
		void set_ipBasedConfiguration(const std::string & value) { m_ipBasedConfiguration = value; }
		const bool get_isApcConnected() const { return m_isApcConnected;}
		void set_isApcConnected(const bool & value) { m_isApcConnected = value; }
		const int64_t get_lastApcUpdate() const { return m_lastApcUpdate;}
		void set_lastApcUpdate(const int64_t & value) { m_lastApcUpdate = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const bool get_poweredOn() const { return m_poweredOn;}
		void set_poweredOn(const bool & value) { m_poweredOn = value; }
		const EquipmentProtocolState get_protocolState() const { return m_protocolState;}
		void set_protocolState(const EquipmentProtocolState & value) { m_protocolState = value; }
		const std::string get_radiusProxyAddress() const { return m_radiusProxyAddress;}
		void set_radiusProxyAddress(const std::string & value) { m_radiusProxyAddress = value; }
		const std::string get_reportedApcAddress() const { return m_reportedApcAddress;}
		void set_reportedApcAddress(const std::string & value) { m_reportedApcAddress = value; }
		const CountryCode get_reportedCC() const { return m_reportedCC;}
		void set_reportedCC(const CountryCode & value) { m_reportedCC = value; }
		const int64_t get_reportedCfgDataVersion() const { return m_reportedCfgDataVersion;}
		void set_reportedCfgDataVersion(const int64_t & value) { m_reportedCfgDataVersion = value; }
		const std::string get_reportedHwVersion() const { return m_reportedHwVersion;}
		void set_reportedHwVersion(const std::string & value) { m_reportedHwVersion = value; }
		const std::string get_reportedIpV4Addr() const { return m_reportedIpV4Addr;}
		void set_reportedIpV4Addr(const std::string & value) { m_reportedIpV4Addr = value; }
		const std::string get_reportedIpV6Addr() const { return m_reportedIpV6Addr;}
		void set_reportedIpV6Addr(const std::string & value) { m_reportedIpV6Addr = value; }
		const MacAddress get_reportedMacAddr() const { return m_reportedMacAddr;}
		void set_reportedMacAddr(const MacAddress & value) { m_reportedMacAddr = value; }
		const std::string get_reportedSku() const { return m_reportedSku;}
		void set_reportedSku(const std::string & value) { m_reportedSku = value; }
		const std::string get_reportedSwAltVersion() const { return m_reportedSwAltVersion;}
		void set_reportedSwAltVersion(const std::string & value) { m_reportedSwAltVersion = value; }
		const std::string get_reportedSwVersion() const { return m_reportedSwVersion;}
		void set_reportedSwVersion(const std::string & value) { m_reportedSwVersion = value; }
		const std::string get_serialNumber() const { return m_serialNumber;}
		void set_serialNumber(const std::string & value) { m_serialNumber = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }
		const std::string get_systemContact() const { return m_systemContact;}
		void set_systemContact(const std::string & value) { m_systemContact = value; }
		const std::string get_systemLocation() const { return m_systemLocation;}
		void set_systemLocation(const std::string & value) { m_systemLocation = value; }
		const std::string get_systemName() const { return m_systemName;}
		void set_systemName(const std::string & value) { m_systemName = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_bandPlan;
		MacAddress	m_baseMacAddress;
		int64_t	m_cloudCfgDataVersion;
		std::string	m_cloudProtocolVersion;
		std::string	m_countryCode;
		std::string	m_ipBasedConfiguration;
		bool	m_isApcConnected;
		int64_t	m_lastApcUpdate;
		std::string	m_model_type;
		bool	m_poweredOn;
		EquipmentProtocolState	m_protocolState;
		std::string	m_radiusProxyAddress;
		std::string	m_reportedApcAddress;
		CountryCode	m_reportedCC;
		int64_t	m_reportedCfgDataVersion;
		std::string	m_reportedHwVersion;
		std::string	m_reportedIpV4Addr;
		std::string	m_reportedIpV6Addr;
		MacAddress	m_reportedMacAddr;
		std::string	m_reportedSku;
		std::string	m_reportedSwAltVersion;
		std::string	m_reportedSwVersion;
		std::string	m_serialNumber;
		std::string	m_statusDataType;
		std::string	m_systemContact;
		std::string	m_systemLocation;
		std::string	m_systemName;
};

class ListOfMcsStatsPerRadioMap  {
	public:
		const std::vector<McsStats> get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const std::vector<McsStats> & value) { m_is2dot4GHz = value; }
		const std::vector<McsStats> get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const std::vector<McsStats> & value) { m_is5GHz = value; }
		const std::vector<McsStats> get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const std::vector<McsStats> & value) { m_is5GHzL = value; }
		const std::vector<McsStats> get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const std::vector<McsStats> & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<McsStats>	m_is2dot4GHz;
		std::vector<McsStats>	m_is5GHz;
		std::vector<McsStats>	m_is5GHzL;
		std::vector<McsStats>	m_is5GHzU;
};

class VLANStatusDataMap  {
	public:
		const VLANStatusData get_VLANStatusDataMap() const { return m_VLANStatusDataMap;}
		void set_VLANStatusDataMap(const VLANStatusData & value) { m_VLANStatusDataMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		VLANStatusData	m_VLANStatusDataMap;
};

class RealTimeStreamingStartEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMac() const { return m_clientMac;}
		void set_clientMac(const MacAddress & value) { m_clientMac = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_serverDnsName() const { return m_serverDnsName;}
		void set_serverDnsName(const std::string & value) { m_serverDnsName = value; }
		const std::string get_serverIp() const { return m_serverIp;}
		void set_serverIp(const std::string & value) { m_serverIp = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const StreamingVideoType get_type() const { return m_type;}
		void set_type(const StreamingVideoType & value) { m_type = value; }
		const int64_t get_videoSessionId() const { return m_videoSessionId;}
		void set_videoSessionId(const int64_t & value) { m_videoSessionId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMac;
		std::string	m_model_type;
		std::string	m_serverDnsName;
		std::string	m_serverIp; /* string representing InetAddress */
		int64_t	m_sessionId;
		StreamingVideoType	m_type;
		int64_t	m_videoSessionId;
};

class PasspointOperatorProfile  : public ProfileDetails {
	public:
		const std::string get_defaultOperatorFriendlyName() const { return m_defaultOperatorFriendlyName;}
		void set_defaultOperatorFriendlyName(const std::string & value) { m_defaultOperatorFriendlyName = value; }
		const std::string get_defaultOperatorFriendlyNameFr() const { return m_defaultOperatorFriendlyNameFr;}
		void set_defaultOperatorFriendlyNameFr(const std::string & value) { m_defaultOperatorFriendlyNameFr = value; }
		const std::vector<std::string> get_domainNameList() const { return m_domainNameList;}
		void set_domainNameList(const std::vector<std::string> & value) { m_domainNameList = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::vector<PasspointDuple> get_operatorFriendlyName() const { return m_operatorFriendlyName;}
		void set_operatorFriendlyName(const std::vector<PasspointDuple> & value) { m_operatorFriendlyName = value; }
		const bool get_serverOnlyAuthenticatedL2EncryptionNetwork() const { return m_serverOnlyAuthenticatedL2EncryptionNetwork;}
		void set_serverOnlyAuthenticatedL2EncryptionNetwork(const bool & value) { m_serverOnlyAuthenticatedL2EncryptionNetwork = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_defaultOperatorFriendlyName;
		std::string	m_defaultOperatorFriendlyNameFr;
		std::vector<std::string>	m_domainNameList;
		std::string	m_model_type;
		std::vector<PasspointDuple>	m_operatorFriendlyName;
		bool	m_serverOnlyAuthenticatedL2EncryptionNetwork; /* OSEN */
};

class RadiusMetrics  {
	public:
		const MinMaxAvgValueInt get_latencyMs() const { return m_latencyMs;}
		void set_latencyMs(const MinMaxAvgValueInt & value) { m_latencyMs = value; }
		const int64_t get_numberOfNoAnswer() const { return m_numberOfNoAnswer;}
		void set_numberOfNoAnswer(const int64_t & value) { m_numberOfNoAnswer = value; }
		const std::string get_serverIp() const { return m_serverIp;}
		void set_serverIp(const std::string & value) { m_serverIp = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MinMaxAvgValueInt	m_latencyMs;
		int64_t	m_numberOfNoAnswer;
		std::string	m_serverIp;
};

class ClientConnectionDetails  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const IntegerPerRadioTypeMap get_numClientsPerRadio() const { return m_numClientsPerRadio;}
		void set_numClientsPerRadio(const IntegerPerRadioTypeMap & value) { m_numClientsPerRadio = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		IntegerPerRadioTypeMap	m_numClientsPerRadio;
		std::string	m_statusDataType;
};

class SystemEventRecord  {
	public:
		const int64_t get_clientMac() const { return m_clientMac;}
		void set_clientMac(const int64_t & value) { m_clientMac = value; }
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const SystemEventDataType get_dataType() const { return m_dataType;}
		void set_dataType(const SystemEventDataType & value) { m_dataType = value; }
		const SystemEvent get_details() const { return m_details;}
		void set_details(const SystemEvent & value) { m_details = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const int64_t get_locationId() const { return m_locationId;}
		void set_locationId(const int64_t & value) { m_locationId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_clientMac; /* int64 representation of the client MAC address, used internally for storage and indexing */
		MacAddress	m_clientMacAddress;
		int64_t	m_customerId;
		SystemEventDataType	m_dataType;
		SystemEvent	m_details;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		int64_t	m_locationId;
};

class DailyTimeRangeSchedule  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const LocalTimeValue get_timeBegin() const { return m_timeBegin;}
		void set_timeBegin(const LocalTimeValue & value) { m_timeBegin = value; }
		const LocalTimeValue get_timeEnd() const { return m_timeEnd;}
		void set_timeEnd(const LocalTimeValue & value) { m_timeEnd = value; }
		const std::string get_timezone() const { return m_timezone;}
		void set_timezone(const std::string & value) { m_timezone = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		LocalTimeValue	m_timeBegin;
		LocalTimeValue	m_timeEnd;
		std::string	m_timezone;
};

class PaginationResponseProfile  {
	public:
		const PaginationContextProfile get_context() const { return m_context;}
		void set_context(const PaginationContextProfile & value) { m_context = value; }
		const std::vector<Profile> get_items() const { return m_items;}
		void set_items(const std::vector<Profile> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextProfile	m_context;
		std::vector<Profile>	m_items;
};

class CustomerFirmwareTrackRecord  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const CustomerFirmwareTrackSettings get_settings() const { return m_settings;}
		void set_settings(const CustomerFirmwareTrackSettings & value) { m_settings = value; }
		const int64_t get_trackRecordId() const { return m_trackRecordId;}
		void set_trackRecordId(const int64_t & value) { m_trackRecordId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		CustomerFirmwareTrackSettings	m_settings;
		int64_t	m_trackRecordId;
};

class ClientAuthEvent  : public RealTimeEvent {
	public:
		const WlanStatusCode get_authStatus() const { return m_authStatus;}
		void set_authStatus(const WlanStatusCode & value) { m_authStatus = value; }
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const bool get_isReassociation() const { return m_isReassociation;}
		void set_isReassociation(const bool & value) { m_isReassociation = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		WlanStatusCode	m_authStatus;
		MacAddress	m_clientMacAddress;
		bool	m_isReassociation;
		std::string	m_model_type;
		RadioType	m_radioType;
		int64_t	m_sessionId;
		std::string	m_ssid;
};

class FirmwareTrackAssignmentDetails  : public FirmwareTrackAssignmentRecord {
	public:
		const std::string get_commit() const { return m_commit;}
		void set_commit(const std::string & value) { m_commit = value; }
		const std::string get_description() const { return m_description;}
		void set_description(const std::string & value) { m_description = value; }
		const EquipmentType get_equipmentType() const { return m_equipmentType;}
		void set_equipmentType(const EquipmentType & value) { m_equipmentType = value; }
		const std::string get_modelId() const { return m_modelId;}
		void set_modelId(const std::string & value) { m_modelId = value; }
		const int64_t get_releaseDate() const { return m_releaseDate;}
		void set_releaseDate(const int64_t & value) { m_releaseDate = value; }
		const std::string get_versionName() const { return m_versionName;}
		void set_versionName(const std::string & value) { m_versionName = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_commit; /* commit number for the firmware image, from the source control system */
		std::string	m_description;
		EquipmentType	m_equipmentType;
		std::string	m_modelId; /* equipment model */
		int64_t	m_releaseDate; /* release date of the firmware image, in ms epoch time */
		std::string	m_versionName;
};

class PaginationResponseStatus  {
	public:
		const PaginationContextStatus get_context() const { return m_context;}
		void set_context(const PaginationContextStatus & value) { m_context = value; }
		const std::vector<Status> get_items() const { return m_items;}
		void set_items(const std::vector<Status> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextStatus	m_context;
		std::vector<Status>	m_items;
};

class RadioProfileConfigurationMap  {
	public:
		const RadioProfileConfiguration get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const RadioProfileConfiguration & value) { m_is2dot4GHz = value; }
		const RadioProfileConfiguration get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const RadioProfileConfiguration & value) { m_is5GHz = value; }
		const RadioProfileConfiguration get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const RadioProfileConfiguration & value) { m_is5GHzL = value; }
		const RadioProfileConfiguration get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const RadioProfileConfiguration & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RadioProfileConfiguration	m_is2dot4GHz;
		RadioProfileConfiguration	m_is5GHz;
		RadioProfileConfiguration	m_is5GHzL;
		RadioProfileConfiguration	m_is5GHzU;
};

class SourceSelectionSteering  {
	public:
		const SourceType get_source() const { return m_source;}
		void set_source(const SourceType & value) { m_source = value; }
		const RadioBestApSettings get_value() const { return m_value;}
		void set_value(const RadioBestApSettings & value) { m_value = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		SourceType	m_source;
		RadioBestApSettings	m_value;
};

class EquipmentAutoProvisioningSettings  {
	public:
		const bool get_enabled() const { return m_enabled;}
		void set_enabled(const bool & value) { m_enabled = value; }
		const LongValueMap get_equipmentProfileIdPerModel() const { return m_equipmentProfileIdPerModel;}
		void set_equipmentProfileIdPerModel(const LongValueMap & value) { m_equipmentProfileIdPerModel = value; }
		const int64_t get_locationId() const { return m_locationId;}
		void set_locationId(const int64_t & value) { m_locationId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_enabled;
		LongValueMap	m_equipmentProfileIdPerModel;
		int64_t	m_locationId; /* auto-provisioned equipment will appear under this location */
};

class ProfileAddedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Profile get_payload() const { return m_payload;}
		void set_payload(const Profile & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Profile	m_payload;
};

class PasspointProfile  : public ProfileDetails {
	public:
		const PasspointAccessNetworkType get_accessNetworkType() const { return m_accessNetworkType;}
		void set_accessNetworkType(const PasspointAccessNetworkType & value) { m_accessNetworkType = value; }
		const int64_t get_additionalStepsRequiredForAccess() const { return m_additionalStepsRequiredForAccess;}
		void set_additionalStepsRequiredForAccess(const int64_t & value) { m_additionalStepsRequiredForAccess = value; }
		const int64_t get_anqpDomainId() const { return m_anqpDomainId;}
		void set_anqpDomainId(const int64_t & value) { m_anqpDomainId = value; }
		const std::string get_apCivicLocation() const { return m_apCivicLocation;}
		void set_apCivicLocation(const std::string & value) { m_apCivicLocation = value; }
		const std::string get_apGeospatialLocation() const { return m_apGeospatialLocation;}
		void set_apGeospatialLocation(const std::string & value) { m_apGeospatialLocation = value; }
		const std::string get_apPublicLocationIdUri() const { return m_apPublicLocationIdUri;}
		void set_apPublicLocationIdUri(const std::string & value) { m_apPublicLocationIdUri = value; }
		const std::vector<int64_t> get_associatedAccessSsidProfileIds() const { return m_associatedAccessSsidProfileIds;}
		void set_associatedAccessSsidProfileIds(const std::vector<int64_t> & value) { m_associatedAccessSsidProfileIds = value; }
		const std::vector<PasspointConnectionCapability> get_connectionCapabilitySet() const { return m_connectionCapabilitySet;}
		void set_connectionCapabilitySet(const std::vector<PasspointConnectionCapability> & value) { m_connectionCapabilitySet = value; }
		const int64_t get_deauthRequestTimeout() const { return m_deauthRequestTimeout;}
		void set_deauthRequestTimeout(const int64_t & value) { m_deauthRequestTimeout = value; }
		const bool get_disableDownstreamGroupAddressedForwarding() const { return m_disableDownstreamGroupAddressedForwarding;}
		void set_disableDownstreamGroupAddressedForwarding(const bool & value) { m_disableDownstreamGroupAddressedForwarding = value; }
		const bool get_emergencyServicesReachable() const { return m_emergencyServicesReachable;}
		void set_emergencyServicesReachable(const bool & value) { m_emergencyServicesReachable = value; }
		const bool get_enable2pt4GHz() const { return m_enable2pt4GHz;}
		void set_enable2pt4GHz(const bool & value) { m_enable2pt4GHz = value; }
		const bool get_enable5GHz() const { return m_enable5GHz;}
		void set_enable5GHz(const bool & value) { m_enable5GHz = value; }
		const bool get_enableInterworkingAndHs20() const { return m_enableInterworkingAndHs20;}
		void set_enableInterworkingAndHs20(const bool & value) { m_enableInterworkingAndHs20 = value; }
		const PasspointGasAddress3Behaviour get_gasAddr3Behaviour() const { return m_gasAddr3Behaviour;}
		void set_gasAddr3Behaviour(const PasspointGasAddress3Behaviour & value) { m_gasAddr3Behaviour = value; }
		const MacAddress get_hessid() const { return m_hessid;}
		void set_hessid(const MacAddress & value) { m_hessid = value; }
		const bool get_internetConnectivity() const { return m_internetConnectivity;}
		void set_internetConnectivity(const bool & value) { m_internetConnectivity = value; }
		const bool get_ipAddressTypeAvailability() const { return m_ipAddressTypeAvailability;}
		void set_ipAddressTypeAvailability(const bool & value) { m_ipAddressTypeAvailability = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const PasspointNetworkAuthenticationType get_networkAuthenticationType() const { return m_networkAuthenticationType;}
		void set_networkAuthenticationType(const PasspointNetworkAuthenticationType & value) { m_networkAuthenticationType = value; }
		const int64_t get_operatingClass() const { return m_operatingClass;}
		void set_operatingClass(const int64_t & value) { m_operatingClass = value; }
		const int64_t get_osuSsidProfileId() const { return m_osuSsidProfileId;}
		void set_osuSsidProfileId(const int64_t & value) { m_osuSsidProfileId = value; }
		const int64_t get_passpointOperatorProfileId() const { return m_passpointOperatorProfileId;}
		void set_passpointOperatorProfileId(const int64_t & value) { m_passpointOperatorProfileId = value; }
		const std::vector<int64_t> get_passpointOsuProviderProfileIds() const { return m_passpointOsuProviderProfileIds;}
		void set_passpointOsuProviderProfileIds(const std::vector<int64_t> & value) { m_passpointOsuProviderProfileIds = value; }
		const int64_t get_passpointVenueProfileId() const { return m_passpointVenueProfileId;}
		void set_passpointVenueProfileId(const int64_t & value) { m_passpointVenueProfileId = value; }
		const std::vector<std::string> get_qosMapSetConfiguration() const { return m_qosMapSetConfiguration;}
		void set_qosMapSetConfiguration(const std::vector<std::string> & value) { m_qosMapSetConfiguration = value; }
		const ManagedFileInfo get_termsAndConditionsFile() const { return m_termsAndConditionsFile;}
		void set_termsAndConditionsFile(const ManagedFileInfo & value) { m_termsAndConditionsFile = value; }
		const bool get_unauthenticatedEmergencyServiceAccessible() const { return m_unauthenticatedEmergencyServiceAccessible;}
		void set_unauthenticatedEmergencyServiceAccessible(const bool & value) { m_unauthenticatedEmergencyServiceAccessible = value; }
		const std::string get_whitelistDomain() const { return m_whitelistDomain;}
		void set_whitelistDomain(const std::string & value) { m_whitelistDomain = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PasspointAccessNetworkType	m_accessNetworkType;
		int64_t	m_additionalStepsRequiredForAccess;
		int64_t	m_anqpDomainId;
		std::string	m_apCivicLocation;
		std::string	m_apGeospatialLocation;
		std::string	m_apPublicLocationIdUri;
		std::vector<int64_t>	m_associatedAccessSsidProfileIds;
		std::vector<PasspointConnectionCapability>	m_connectionCapabilitySet;
		int64_t	m_deauthRequestTimeout;
		bool	m_disableDownstreamGroupAddressedForwarding;
		bool	m_emergencyServicesReachable;
		bool	m_enable2pt4GHz;
		bool	m_enable5GHz;
		bool	m_enableInterworkingAndHs20;
		PasspointGasAddress3Behaviour	m_gasAddr3Behaviour;
		MacAddress	m_hessid;
		bool	m_internetConnectivity;
		bool	m_ipAddressTypeAvailability;
		std::string	m_model_type;
		PasspointNetworkAuthenticationType	m_networkAuthenticationType;
		int64_t	m_operatingClass;
		int64_t	m_osuSsidProfileId;
		int64_t	m_passpointOperatorProfileId; /* Profile Id of a PasspointOperatorProfile profile, must be also added to the children of this profile */
		std::vector<int64_t>	m_passpointOsuProviderProfileIds; /* array containing Profile Ids of PasspointOsuProviderProfiles, must be also added to the children of this profile */
		int64_t	m_passpointVenueProfileId; /* Profile Id of a PasspointVenueProfile profile, must be also added to the children of this profile */
		std::vector<std::string>	m_qosMapSetConfiguration;
		ManagedFileInfo	m_termsAndConditionsFile;
		bool	m_unauthenticatedEmergencyServiceAccessible;
		std::string	m_whitelistDomain;
};

class RoutingRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const EquipmentRoutingRecord get_payload() const { return m_payload;}
		void set_payload(const EquipmentRoutingRecord & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		EquipmentRoutingRecord	m_payload;
};

class TimedAccessUserRecord  {
	public:
		const int64_t get_activationTime() const { return m_activationTime;}
		void set_activationTime(const int64_t & value) { m_activationTime = value; }
		const int64_t get_expirationTime() const { return m_expirationTime;}
		void set_expirationTime(const int64_t & value) { m_expirationTime = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const int64_t get_numDevices() const { return m_numDevices;}
		void set_numDevices(const int64_t & value) { m_numDevices = value; }
		const std::string get_password() const { return m_password;}
		void set_password(const std::string & value) { m_password = value; }
		const TimedAccessUserDetails get_userDetails() const { return m_userDetails;}
		void set_userDetails(const TimedAccessUserDetails & value) { m_userDetails = value; }
		const std::vector<MacAddress> get_userMacAddresses() const { return m_userMacAddresses;}
		void set_userMacAddresses(const std::vector<MacAddress> & value) { m_userMacAddresses = value; }
		const std::string get_username() const { return m_username;}
		void set_username(const std::string & value) { m_username = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_activationTime;
		int64_t	m_expirationTime;
		int64_t	m_lastModifiedTimestamp;
		int64_t	m_numDevices;
		std::string	m_password;
		TimedAccessUserDetails	m_userDetails;
		std::vector<MacAddress>	m_userMacAddresses;
		std::string	m_username;
};

class MacAllowlistRecord  {
	public:
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const MacAddress get_macAddress() const { return m_macAddress;}
		void set_macAddress(const MacAddress & value) { m_macAddress = value; }
		const std::string get_notes() const { return m_notes;}
		void set_notes(const std::string & value) { m_notes = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_lastModifiedTimestamp;
		MacAddress	m_macAddress;
		std::string	m_notes;
};

class RadiusServerDetails  {
	public:
		const std::string get_address() const { return m_address;}
		void set_address(const std::string & value) { m_address = value; }
		const MinMaxAvgValueInt get_radiusLatency() const { return m_radiusLatency;}
		void set_radiusLatency(const MinMaxAvgValueInt & value) { m_radiusLatency = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_address;
		MinMaxAvgValueInt	m_radiusLatency;
};

class ServiceMetric  {
	public:
		const int64_t get_clientMac() const { return m_clientMac;}
		void set_clientMac(const int64_t & value) { m_clientMac = value; }
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const ServiceMetricDataType get_dataType() const { return m_dataType;}
		void set_dataType(const ServiceMetricDataType & value) { m_dataType = value; }
		const ServiceMetricDetails get_details() const { return m_details;}
		void set_details(const ServiceMetricDetails & value) { m_details = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_locationId() const { return m_locationId;}
		void set_locationId(const int64_t & value) { m_locationId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_clientMac; /* int64 representation of the client MAC address, used internally for storage and indexing */
		MacAddress	m_clientMacAddress;
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		ServiceMetricDataType	m_dataType;
		ServiceMetricDetails	m_details;
		int64_t	m_equipmentId;
		int64_t	m_locationId;
};

class ClientFirstDataEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const int64_t get_firstDataRcvdTs() const { return m_firstDataRcvdTs;}
		void set_firstDataRcvdTs(const int64_t & value) { m_firstDataRcvdTs = value; }
		const int64_t get_firstDataSentTs() const { return m_firstDataSentTs;}
		void set_firstDataSentTs(const int64_t & value) { m_firstDataSentTs = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMacAddress;
		int64_t	m_firstDataRcvdTs;
		int64_t	m_firstDataSentTs;
		std::string	m_model_type;
		int64_t	m_sessionId;
};

class ProfileChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Profile get_payload() const { return m_payload;}
		void set_payload(const Profile & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Profile	m_payload;
};

class ManufacturerOuiDetailsPerOuiMap  {
	public:
		const ManufacturerOuiDetails get_ManufacturerOuiDetailsPerOuiMap() const { return m_ManufacturerOuiDetailsPerOuiMap;}
		void set_ManufacturerOuiDetailsPerOuiMap(const ManufacturerOuiDetails & value) { m_ManufacturerOuiDetailsPerOuiMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ManufacturerOuiDetails	m_ManufacturerOuiDetailsPerOuiMap;
};

class TrafficPerRadioDetailsPerRadioTypeMap  {
	public:
		const TrafficPerRadioDetails get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const TrafficPerRadioDetails & value) { m_is2dot4GHz = value; }
		const TrafficPerRadioDetails get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const TrafficPerRadioDetails & value) { m_is5GHz = value; }
		const TrafficPerRadioDetails get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const TrafficPerRadioDetails & value) { m_is5GHzL = value; }
		const TrafficPerRadioDetails get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const TrafficPerRadioDetails & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		TrafficPerRadioDetails	m_is2dot4GHz;
		TrafficPerRadioDetails	m_is5GHz;
		TrafficPerRadioDetails	m_is5GHzL;
		TrafficPerRadioDetails	m_is5GHzU;
};

class ApPerformance  {
	public:
		const int64_t get_camiCrashed() const { return m_camiCrashed;}
		void set_camiCrashed(const int64_t & value) { m_camiCrashed = value; }
		const int64_t get_cloudRxBytes() const { return m_cloudRxBytes;}
		void set_cloudRxBytes(const int64_t & value) { m_cloudRxBytes = value; }
		const int64_t get_cloudTxBytes() const { return m_cloudTxBytes;}
		void set_cloudTxBytes(const int64_t & value) { m_cloudTxBytes = value; }
		const int64_t get_cpuTemperature() const { return m_cpuTemperature;}
		void set_cpuTemperature(const int64_t & value) { m_cpuTemperature = value; }
		const std::vector<int64_t> get_cpuUtilized() const { return m_cpuUtilized;}
		void set_cpuUtilized(const std::vector<int64_t> & value) { m_cpuUtilized = value; }
		const EthernetLinkState get_ethLinkState() const { return m_ethLinkState;}
		void set_ethLinkState(const EthernetLinkState & value) { m_ethLinkState = value; }
		const int64_t get_freeMemory() const { return m_freeMemory;}
		void set_freeMemory(const int64_t & value) { m_freeMemory = value; }
		const bool get_lowMemoryReboot() const { return m_lowMemoryReboot;}
		void set_lowMemoryReboot(const bool & value) { m_lowMemoryReboot = value; }
		const std::vector<PerProcessUtilization> get_psCpuUtil() const { return m_psCpuUtil;}
		void set_psCpuUtil(const std::vector<PerProcessUtilization> & value) { m_psCpuUtil = value; }
		const std::vector<PerProcessUtilization> get_psMemUtil() const { return m_psMemUtil;}
		void set_psMemUtil(const std::vector<PerProcessUtilization> & value) { m_psMemUtil = value; }
		const int64_t get_upTime() const { return m_upTime;}
		void set_upTime(const int64_t & value) { m_upTime = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_camiCrashed; /* number of time cloud-to-ap-management process crashed */
		int64_t	m_cloudRxBytes; /* Data received by AP from cloud */
		int64_t	m_cloudTxBytes; /* Data sent by AP to the cloud */
		int64_t	m_cpuTemperature; /* cpu temperature in Celsius */
		std::vector<int64_t>	m_cpuUtilized; /* CPU utilization in percentage, one per core */
		EthernetLinkState	m_ethLinkState;
		int64_t	m_freeMemory; /* free memory in kilobytes */
		bool	m_lowMemoryReboot; /* low memory reboot happened */
		std::vector<PerProcessUtilization>	m_psCpuUtil;
		std::vector<PerProcessUtilization>	m_psMemUtil;
		int64_t	m_upTime; /* AP uptime in seconds */
};

class ClientFailureEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const WlanReasonCode get_reasonCode() const { return m_reasonCode;}
		void set_reasonCode(const WlanReasonCode & value) { m_reasonCode = value; }
		const std::string get_reasonString() const { return m_reasonString;}
		void set_reasonString(const std::string & value) { m_reasonString = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMacAddress;
		std::string	m_model_type;
		WlanReasonCode	m_reasonCode;
		std::string	m_reasonString;
		int64_t	m_sessionId;
		std::string	m_ssid;
};

class ClientTimeoutEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const int64_t get_lastRecvTime() const { return m_lastRecvTime;}
		void set_lastRecvTime(const int64_t & value) { m_lastRecvTime = value; }
		const int64_t get_lastSentTime() const { return m_lastSentTime;}
		void set_lastSentTime(const int64_t & value) { m_lastSentTime = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const ClientTimeoutReason get_timeoutReason() const { return m_timeoutReason;}
		void set_timeoutReason(const ClientTimeoutReason & value) { m_timeoutReason = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMacAddress;
		int64_t	m_lastRecvTime;
		int64_t	m_lastSentTime;
		std::string	m_model_type;
		int64_t	m_sessionId;
		ClientTimeoutReason	m_timeoutReason;
};

class StatusChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Status get_payload() const { return m_payload;}
		void set_payload(const Status & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Status	m_payload;
};

class CommonProbeDetails  {
	public:
		const MinMaxAvgValueInt get_latencyMs() const { return m_latencyMs;}
		void set_latencyMs(const MinMaxAvgValueInt & value) { m_latencyMs = value; }
		const int64_t get_numFailedProbeRequests() const { return m_numFailedProbeRequests;}
		void set_numFailedProbeRequests(const int64_t & value) { m_numFailedProbeRequests = value; }
		const int64_t get_numSuccessProbeRequests() const { return m_numSuccessProbeRequests;}
		void set_numSuccessProbeRequests(const int64_t & value) { m_numSuccessProbeRequests = value; }
		const StatusCode get_status() const { return m_status;}
		void set_status(const StatusCode & value) { m_status = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MinMaxAvgValueInt	m_latencyMs;
		int64_t	m_numFailedProbeRequests;
		int64_t	m_numSuccessProbeRequests;
		StatusCode	m_status;
};

class AlarmDetails  {
	public:
		const std::vector<int64_t> get_affectedEquipmentIds() const { return m_affectedEquipmentIds;}
		void set_affectedEquipmentIds(const std::vector<int64_t> & value) { m_affectedEquipmentIds = value; }
		const AlarmDetailsAttributesMap get_contextAttrs() const { return m_contextAttrs;}
		void set_contextAttrs(const AlarmDetailsAttributesMap & value) { m_contextAttrs = value; }
		const std::string get_generatedBy() const { return m_generatedBy;}
		void set_generatedBy(const std::string & value) { m_generatedBy = value; }
		const std::string get_message() const { return m_message;}
		void set_message(const std::string & value) { m_message = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<int64_t>	m_affectedEquipmentIds;
		AlarmDetailsAttributesMap	m_contextAttrs;
		std::string	m_generatedBy;
		std::string	m_message;
};

class ActiveBSSIDs  {
	public:
		const std::vector<ActiveBSSID> get_activeBSSIDs() const { return m_activeBSSIDs;}
		void set_activeBSSIDs(const std::vector<ActiveBSSID> & value) { m_activeBSSIDs = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<ActiveBSSID>	m_activeBSSIDs;
		std::string	m_model_type;
		std::string	m_statusDataType;
};

class WebTokenAclTemplate  {
	public:
		const AclTemplate get_aclTemplate() const { return m_aclTemplate;}
		void set_aclTemplate(const AclTemplate & value) { m_aclTemplate = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		AclTemplate	m_aclTemplate;
};

class ClientDisconnectEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const DisconnectFrameType get_frameType() const { return m_frameType;}
		void set_frameType(const DisconnectFrameType & value) { m_frameType = value; }
		const DisconnectInitiator get_initiator() const { return m_initiator;}
		void set_initiator(const DisconnectInitiator & value) { m_initiator = value; }
		const int64_t get_internalReasonCode() const { return m_internalReasonCode;}
		void set_internalReasonCode(const int64_t & value) { m_internalReasonCode = value; }
		const int64_t get_lastRecvTime() const { return m_lastRecvTime;}
		void set_lastRecvTime(const int64_t & value) { m_lastRecvTime = value; }
		const int64_t get_lastSentTime() const { return m_lastSentTime;}
		void set_lastSentTime(const int64_t & value) { m_lastSentTime = value; }
		const std::vector<Base64String> get_macAddressBytes() const { return m_macAddressBytes;}
		void set_macAddressBytes(const std::vector<Base64String> & value) { m_macAddressBytes = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const WlanReasonCode get_reasonCode() const { return m_reasonCode;}
		void set_reasonCode(const WlanReasonCode & value) { m_reasonCode = value; }
		const int64_t get_rssi() const { return m_rssi;}
		void set_rssi(const int64_t & value) { m_rssi = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMacAddress;
		DisconnectFrameType	m_frameType;
		DisconnectInitiator	m_initiator;
		int64_t	m_internalReasonCode;
		int64_t	m_lastRecvTime;
		int64_t	m_lastSentTime;
		std::vector<Base64String>	m_macAddressBytes;
		std::string	m_model_type;
		RadioType	m_radioType;
		WlanReasonCode	m_reasonCode;
		int64_t	m_rssi;
		int64_t	m_sessionId;
		std::string	m_ssid;
};

class PortalUserChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const PortalUser get_payload() const { return m_payload;}
		void set_payload(const PortalUser & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		PortalUser	m_payload;
};

class PasspointOsuProviderProfile  : public ProfileDetails {
	public:
		const std::vector<PasspointMccMnc> get_PasspointMccMncList() const { return m_PasspointMccMncList;}
		void set_PasspointMccMncList(const std::vector<PasspointMccMnc> & value) { m_PasspointMccMncList = value; }
		const std::vector<PasspointOsuIcon> get_PasspointOsuIconList() const { return m_PasspointOsuIconList;}
		void set_PasspointOsuIconList(const std::vector<PasspointOsuIcon> & value) { m_PasspointOsuIconList = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::vector<PasspointNaiRealmInformation> get_naiRealmList() const { return m_naiRealmList;}
		void set_naiRealmList(const std::vector<PasspointNaiRealmInformation> & value) { m_naiRealmList = value; }
		const std::vector<PasspointDuple> get_osuFriendlyName() const { return m_osuFriendlyName;}
		void set_osuFriendlyName(const std::vector<PasspointDuple> & value) { m_osuFriendlyName = value; }
		const int64_t get_osuMethodList() const { return m_osuMethodList;}
		void set_osuMethodList(const int64_t & value) { m_osuMethodList = value; }
		const std::string get_osuNaiShared() const { return m_osuNaiShared;}
		void set_osuNaiShared(const std::string & value) { m_osuNaiShared = value; }
		const std::string get_osuNaiStandalone() const { return m_osuNaiStandalone;}
		void set_osuNaiStandalone(const std::string & value) { m_osuNaiStandalone = value; }
		const std::string get_osuServerUri() const { return m_osuServerUri;}
		void set_osuServerUri(const std::string & value) { m_osuServerUri = value; }
		const std::vector<PasspointDuple> get_osuServiceDescription() const { return m_osuServiceDescription;}
		void set_osuServiceDescription(const std::vector<PasspointDuple> & value) { m_osuServiceDescription = value; }
		const std::string get_osuSsid() const { return m_osuSsid;}
		void set_osuSsid(const std::string & value) { m_osuSsid = value; }
		const std::string get_radiusProfileAccounting() const { return m_radiusProfileAccounting;}
		void set_radiusProfileAccounting(const std::string & value) { m_radiusProfileAccounting = value; }
		const std::string get_radiusProfileAuth() const { return m_radiusProfileAuth;}
		void set_radiusProfileAuth(const std::string & value) { m_radiusProfileAuth = value; }
		const std::vector<std::string> get_roamingOi() const { return m_roamingOi;}
		void set_roamingOi(const std::vector<std::string> & value) { m_roamingOi = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<PasspointMccMnc>	m_PasspointMccMncList;
		std::vector<PasspointOsuIcon>	m_PasspointOsuIconList;
		std::string	m_model_type;
		std::vector<PasspointNaiRealmInformation>	m_naiRealmList;
		std::vector<PasspointDuple>	m_osuFriendlyName;
		int64_t	m_osuMethodList;
		std::string	m_osuNaiShared;
		std::string	m_osuNaiStandalone;
		std::string	m_osuServerUri;
		std::vector<PasspointDuple>	m_osuServiceDescription;
		std::string	m_osuSsid;
		std::string	m_radiusProfileAccounting;
		std::string	m_radiusProfileAuth;
		std::vector<std::string>	m_roamingOi;
};

class ElementRadioConfiguration  {
	public:
		const ChannelPowerLevel get_allowedChannelsPowerLevels() const { return m_allowedChannelsPowerLevels;}
		void set_allowedChannelsPowerLevels(const ChannelPowerLevel & value) { m_allowedChannelsPowerLevels = value; }
		const int64_t get_backupChannelNumber() const { return m_backupChannelNumber;}
		void set_backupChannelNumber(const int64_t & value) { m_backupChannelNumber = value; }
		const BestAPSteerType get_bestAPSteerType() const { return m_bestAPSteerType;}
		void set_bestAPSteerType(const BestAPSteerType & value) { m_bestAPSteerType = value; }
		const int64_t get_channelNumber() const { return m_channelNumber;}
		void set_channelNumber(const int64_t & value) { m_channelNumber = value; }
		const SourceSelectionValue get_clientDisconnectThresholdDb() const { return m_clientDisconnectThresholdDb;}
		void set_clientDisconnectThresholdDb(const SourceSelectionValue & value) { m_clientDisconnectThresholdDb = value; }
		const bool get_deauthAttackDetection() const { return m_deauthAttackDetection;}
		void set_deauthAttackDetection(const bool & value) { m_deauthAttackDetection = value; }
		const bool get_eirpTxPower() const { return m_eirpTxPower;}
		void set_eirpTxPower(const bool & value) { m_eirpTxPower = value; }
		const int64_t get_manualBackupChannelNumber() const { return m_manualBackupChannelNumber;}
		void set_manualBackupChannelNumber(const int64_t & value) { m_manualBackupChannelNumber = value; }
		const int64_t get_manualChannelNumber() const { return m_manualChannelNumber;}
		void set_manualChannelNumber(const int64_t & value) { m_manualChannelNumber = value; }
		const bool get_perimeterDetectionEnabled() const { return m_perimeterDetectionEnabled;}
		void set_perimeterDetectionEnabled(const bool & value) { m_perimeterDetectionEnabled = value; }
		const SourceSelectionValue get_probeResponseThresholdDb() const { return m_probeResponseThresholdDb;}
		void set_probeResponseThresholdDb(const SourceSelectionValue & value) { m_probeResponseThresholdDb = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const SourceSelectionValue get_rxCellSizeDb() const { return m_rxCellSizeDb;}
		void set_rxCellSizeDb(const SourceSelectionValue & value) { m_rxCellSizeDb = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ChannelPowerLevel	m_allowedChannelsPowerLevels;
		int64_t	m_backupChannelNumber; /* The backup channel that was picked through the cloud's assigment */
		BestAPSteerType	m_bestAPSteerType;
		int64_t	m_channelNumber; /* The channel that was picked through the cloud's assigment */
		SourceSelectionValue	m_clientDisconnectThresholdDb;
		bool	m_deauthAttackDetection;
		bool	m_eirpTxPower;
		int64_t	m_manualBackupChannelNumber; /* The backup channel that was manually entered */
		int64_t	m_manualChannelNumber; /* The channel that was manually entered */
		bool	m_perimeterDetectionEnabled;
		SourceSelectionValue	m_probeResponseThresholdDb;
		RadioType	m_radioType;
		SourceSelectionValue	m_rxCellSizeDb;
};

class RealTimeStreamingStartSessionEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMac() const { return m_clientMac;}
		void set_clientMac(const MacAddress & value) { m_clientMac = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_serverIp() const { return m_serverIp;}
		void set_serverIp(const std::string & value) { m_serverIp = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const StreamingVideoType get_type() const { return m_type;}
		void set_type(const StreamingVideoType & value) { m_type = value; }
		const int64_t get_videoSessionId() const { return m_videoSessionId;}
		void set_videoSessionId(const int64_t & value) { m_videoSessionId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMac;
		std::string	m_model_type;
		std::string	m_serverIp; /* string representing InetAddress */
		int64_t	m_sessionId;
		StreamingVideoType	m_type;
		int64_t	m_videoSessionId;
};

class GatewayChangedEvent  {
	public:
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const EquipmentGatewayRecord get_gateway() const { return m_gateway;}
		void set_gateway(const EquipmentGatewayRecord & value) { m_gateway = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_eventTimestamp;
		EquipmentGatewayRecord	m_gateway;
		std::string	m_model_type;
};

class RadioStatisticsPerRadioMap  {
	public:
		const RadioStatistics get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const RadioStatistics & value) { m_is2dot4GHz = value; }
		const RadioStatistics get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const RadioStatistics & value) { m_is5GHz = value; }
		const RadioStatistics get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const RadioStatistics & value) { m_is5GHzL = value; }
		const RadioStatistics get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const RadioStatistics & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RadioStatistics	m_is2dot4GHz;
		RadioStatistics	m_is5GHz;
		RadioStatistics	m_is5GHzL;
		RadioStatistics	m_is5GHzU;
};

class ClientConnectSuccessEvent  : public RealTimeEvent {
	public:
		const int64_t get_assocRssi() const { return m_assocRssi;}
		void set_assocRssi(const int64_t & value) { m_assocRssi = value; }
		const int64_t get_assocTs() const { return m_assocTs;}
		void set_assocTs(const int64_t & value) { m_assocTs = value; }
		const int64_t get_authTs() const { return m_authTs;}
		void set_authTs(const int64_t & value) { m_authTs = value; }
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const std::string get_cltId() const { return m_cltId;}
		void set_cltId(const std::string & value) { m_cltId = value; }
		const int64_t get_eapolTs() const { return m_eapolTs;}
		void set_eapolTs(const int64_t & value) { m_eapolTs = value; }
		const bool get_fbtUsed() const { return m_fbtUsed;}
		void set_fbtUsed(const bool & value) { m_fbtUsed = value; }
		const int64_t get_firstDataRxTs() const { return m_firstDataRxTs;}
		void set_firstDataRxTs(const int64_t & value) { m_firstDataRxTs = value; }
		const int64_t get_firstDataTxTs() const { return m_firstDataTxTs;}
		void set_firstDataTxTs(const int64_t & value) { m_firstDataTxTs = value; }
		const int64_t get_ipAcquisitionTs() const { return m_ipAcquisitionTs;}
		void set_ipAcquisitionTs(const int64_t & value) { m_ipAcquisitionTs = value; }
		const std::string get_ipAddr() const { return m_ipAddr;}
		void set_ipAddr(const std::string & value) { m_ipAddr = value; }
		const bool get_isReassociation() const { return m_isReassociation;}
		void set_isReassociation(const bool & value) { m_isReassociation = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_portEnabledTs() const { return m_portEnabledTs;}
		void set_portEnabledTs(const int64_t & value) { m_portEnabledTs = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const SecurityType get_securityType() const { return m_securityType;}
		void set_securityType(const SecurityType & value) { m_securityType = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }
		const bool get_using11k() const { return m_using11k;}
		void set_using11k(const bool & value) { m_using11k = value; }
		const bool get_using11r() const { return m_using11r;}
		void set_using11r(const bool & value) { m_using11r = value; }
		const bool get_using11v() const { return m_using11v;}
		void set_using11v(const bool & value) { m_using11v = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_assocRssi;
		int64_t	m_assocTs;
		int64_t	m_authTs;
		MacAddress	m_clientMacAddress;
		std::string	m_cltId;
		int64_t	m_eapolTs;
		bool	m_fbtUsed;
		int64_t	m_firstDataRxTs;
		int64_t	m_firstDataTxTs;
		int64_t	m_ipAcquisitionTs;
		std::string	m_ipAddr; /* string representing InetAddress */
		bool	m_isReassociation;
		std::string	m_model_type;
		int64_t	m_portEnabledTs;
		RadioType	m_radioType;
		SecurityType	m_securityType;
		int64_t	m_sessionId;
		std::string	m_ssid;
		bool	m_using11k;
		bool	m_using11r;
		bool	m_using11v;
};

class ClientIpAddressEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const std::vector<Base64String> get_ipAddr() const { return m_ipAddr;}
		void set_ipAddr(const std::vector<Base64String> & value) { m_ipAddr = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMacAddress;
		std::vector<Base64String>	m_ipAddr;
		std::string	m_model_type;
		int64_t	m_sessionId;
};

class PortalUserRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const PortalUser get_payload() const { return m_payload;}
		void set_payload(const PortalUser & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		PortalUser	m_payload;
};

class CustomerPortalDashboardStatus  {
	public:
		const IntegerPerStatusCodeMap get_alarmsCountBySeverity() const { return m_alarmsCountBySeverity;}
		void set_alarmsCountBySeverity(const IntegerPerStatusCodeMap & value) { m_alarmsCountBySeverity = value; }
		const IntegerPerRadioTypeMap get_associatedClientsCountPerRadio() const { return m_associatedClientsCountPerRadio;}
		void set_associatedClientsCountPerRadio(const IntegerPerRadioTypeMap & value) { m_associatedClientsCountPerRadio = value; }
		const IntegerValueMap get_clientCountPerOui() const { return m_clientCountPerOui;}
		void set_clientCountPerOui(const IntegerValueMap & value) { m_clientCountPerOui = value; }
		const IntegerValueMap get_equipmentCountPerOui() const { return m_equipmentCountPerOui;}
		void set_equipmentCountPerOui(const IntegerValueMap & value) { m_equipmentCountPerOui = value; }
		const int64_t get_equipmentInServiceCount() const { return m_equipmentInServiceCount;}
		void set_equipmentInServiceCount(const int64_t & value) { m_equipmentInServiceCount = value; }
		const int64_t get_equipmentWithClientsCount() const { return m_equipmentWithClientsCount;}
		void set_equipmentWithClientsCount(const int64_t & value) { m_equipmentWithClientsCount = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }
		const int64_t get_timeBucketId() const { return m_timeBucketId;}
		void set_timeBucketId(const int64_t & value) { m_timeBucketId = value; }
		const int64_t get_timeBucketMs() const { return m_timeBucketMs;}
		void set_timeBucketMs(const int64_t & value) { m_timeBucketMs = value; }
		const int64_t get_totalProvisionedEquipment() const { return m_totalProvisionedEquipment;}
		void set_totalProvisionedEquipment(const int64_t & value) { m_totalProvisionedEquipment = value; }
		const int64_t get_trafficBytesDownstream() const { return m_trafficBytesDownstream;}
		void set_trafficBytesDownstream(const int64_t & value) { m_trafficBytesDownstream = value; }
		const int64_t get_trafficBytesUpstream() const { return m_trafficBytesUpstream;}
		void set_trafficBytesUpstream(const int64_t & value) { m_trafficBytesUpstream = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		IntegerPerStatusCodeMap	m_alarmsCountBySeverity;
		IntegerPerRadioTypeMap	m_associatedClientsCountPerRadio;
		IntegerValueMap	m_clientCountPerOui;
		IntegerValueMap	m_equipmentCountPerOui;
		int64_t	m_equipmentInServiceCount;
		int64_t	m_equipmentWithClientsCount;
		std::string	m_model_type;
		std::string	m_statusDataType;
		int64_t	m_timeBucketId; /* All metrics/events that have (createdTimestamp % timeBucketMs == timeBucketId) are counted in this object. */
		int64_t	m_timeBucketMs; /* Length of the time bucket in milliseconds */
		int64_t	m_totalProvisionedEquipment;
		int64_t	m_trafficBytesDownstream;
		int64_t	m_trafficBytesUpstream;
};

class StatusRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Status get_payload() const { return m_payload;}
		void set_payload(const Status & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Status	m_payload;
};

class ClientSessionDetails  {
	public:
		const int64_t get_priorEquipmentId() const { return m_priorEquipmentId;}
		void set_priorEquipmentId(const int64_t & value) { m_priorEquipmentId = value; }
		const bool get_is11KUsed() const { return m_is11KUsed;}
		void set_is11KUsed(const bool & value) { m_is11KUsed = value; }
		const std::string get_userAgentStr() const { return m_userAgentStr;}
		void set_userAgentStr(const std::string & value) { m_userAgentStr = value; }
		const int64_t get_timeoutTimestamp() const { return m_timeoutTimestamp;}
		void set_timeoutTimestamp(const int64_t & value) { m_timeoutTimestamp = value; }
		const std::string get_classificationName() const { return m_classificationName;}
		void set_classificationName(const std::string & value) { m_classificationName = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const ClientDhcpDetails get_dhcpDetails() const { return m_dhcpDetails;}
		void set_dhcpDetails(const ClientDhcpDetails & value) { m_dhcpDetails = value; }
		const int64_t get_disconnectByClientInternalReasonCode() const { return m_disconnectByClientInternalReasonCode;}
		void set_disconnectByClientInternalReasonCode(const int64_t & value) { m_disconnectByClientInternalReasonCode = value; }
		const std::string get_associationState() const { return m_associationState;}
		void set_associationState(const std::string & value) { m_associationState = value; }
		const int64_t get_disconnectByApInternalReasonCode() const { return m_disconnectByApInternalReasonCode;}
		void set_disconnectByApInternalReasonCode(const int64_t & value) { m_disconnectByApInternalReasonCode = value; }
		const SteerType get_steerType() const { return m_steerType;}
		void set_steerType(const SteerType & value) { m_steerType = value; }
		const int64_t get_previousValidSessionId() const { return m_previousValidSessionId;}
		void set_previousValidSessionId(const int64_t & value) { m_previousValidSessionId = value; }
		const ClientEapDetails get_eapDetails() const { return m_eapDetails;}
		void set_eapDetails(const ClientEapDetails & value) { m_eapDetails = value; }
		const bool get_isReassociation() const { return m_isReassociation;}
		void set_isReassociation(const bool & value) { m_isReassociation = value; }
		const int64_t get_assocRssi() const { return m_assocRssi;}
		void set_assocRssi(const int64_t & value) { m_assocRssi = value; }
		const ClientFailureDetails get_firstFailureDetails() const { return m_firstFailureDetails;}
		void set_firstFailureDetails(const ClientFailureDetails & value) { m_firstFailureDetails = value; }
		const bool get_is11VUsed() const { return m_is11VUsed;}
		void set_is11VUsed(const bool & value) { m_is11VUsed = value; }
		const int64_t get_dynamicVlan() const { return m_dynamicVlan;}
		void set_dynamicVlan(const int64_t & value) { m_dynamicVlan = value; }
		const SecurityType get_securityType() const { return m_securityType;}
		void set_securityType(const SecurityType & value) { m_securityType = value; }
		const int64_t get_lastEventTimestamp() const { return m_lastEventTimestamp;}
		void set_lastEventTimestamp(const int64_t & value) { m_lastEventTimestamp = value; }
		const std::string get_hostname() const { return m_hostname;}
		void set_hostname(const std::string & value) { m_hostname = value; }
		const int64_t get_priorSessionId() const { return m_priorSessionId;}
		void set_priorSessionId(const int64_t & value) { m_priorSessionId = value; }
		const int64_t get_assocTimestamp() const { return m_assocTimestamp;}
		void set_assocTimestamp(const int64_t & value) { m_assocTimestamp = value; }
		const int64_t get_lastTxTimestamp() const { return m_lastTxTimestamp;}
		void set_lastTxTimestamp(const int64_t & value) { m_lastTxTimestamp = value; }
		const int64_t get_disconnectByApTimestamp() const { return m_disconnectByApTimestamp;}
		void set_disconnectByApTimestamp(const int64_t & value) { m_disconnectByApTimestamp = value; }
		const int64_t get_disconnectByApReasonCode() const { return m_disconnectByApReasonCode;}
		void set_disconnectByApReasonCode(const int64_t & value) { m_disconnectByApReasonCode = value; }
		const int64_t get_disconnectByClientReasonCode() const { return m_disconnectByClientReasonCode;}
		void set_disconnectByClientReasonCode(const int64_t & value) { m_disconnectByClientReasonCode = value; }
		const std::string get_apFingerprint() const { return m_apFingerprint;}
		void set_apFingerprint(const std::string & value) { m_apFingerprint = value; }
		const int64_t get_portEnabledTimestamp() const { return m_portEnabledTimestamp;}
		void set_portEnabledTimestamp(const int64_t & value) { m_portEnabledTimestamp = value; }
		const int64_t get_firstDataSentTimestamp() const { return m_firstDataSentTimestamp;}
		void set_firstDataSentTimestamp(const int64_t & value) { m_firstDataSentTimestamp = value; }
		const int64_t get_associationStatus() const { return m_associationStatus;}
		void set_associationStatus(const int64_t & value) { m_associationStatus = value; }
		const ClientSessionMetricDetails get_metricDetails() const { return m_metricDetails;}
		void set_metricDetails(const ClientSessionMetricDetails & value) { m_metricDetails = value; }
		const std::string get_cpUsername() const { return m_cpUsername;}
		void set_cpUsername(const std::string & value) { m_cpUsername = value; }
		const bool get_is11RUsed() const { return m_is11RUsed;}
		void set_is11RUsed(const bool & value) { m_is11RUsed = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }
		const std::string get_ipAddress() const { return m_ipAddress;}
		void set_ipAddress(const std::string & value) { m_ipAddress = value; }
		const int64_t get_disconnectByClientTimestamp() const { return m_disconnectByClientTimestamp;}
		void set_disconnectByClientTimestamp(const int64_t & value) { m_disconnectByClientTimestamp = value; }
		const int64_t get_lastRxTimestamp() const { return m_lastRxTimestamp;}
		void set_lastRxTimestamp(const int64_t & value) { m_lastRxTimestamp = value; }
		const int64_t get_assocInternalSC() const { return m_assocInternalSC;}
		void set_assocInternalSC(const int64_t & value) { m_assocInternalSC = value; }
		const int64_t get_authTimestamp() const { return m_authTimestamp;}
		void set_authTimestamp(const int64_t & value) { m_authTimestamp = value; }
		const int64_t get_firstDataRcvdTimestamp() const { return m_firstDataRcvdTimestamp;}
		void set_firstDataRcvdTimestamp(const int64_t & value) { m_firstDataRcvdTimestamp = value; }
		const ClientFailureDetails get_lastFailureDetails() const { return m_lastFailureDetails;}
		void set_lastFailureDetails(const ClientFailureDetails & value) { m_lastFailureDetails = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const std::string get_radiusUsername() const { return m_radiusUsername;}
		void set_radiusUsername(const std::string & value) { m_radiusUsername = value; }
		const int64_t get_ipTimestamp() const { return m_ipTimestamp;}
		void set_ipTimestamp(const int64_t & value) { m_ipTimestamp = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_priorEquipmentId;
		bool	m_is11KUsed;
		std::string	m_userAgentStr;
		int64_t	m_timeoutTimestamp;
		std::string	m_classificationName;
		int64_t	m_sessionId;
		ClientDhcpDetails	m_dhcpDetails;
		int64_t	m_disconnectByClientInternalReasonCode;
		std::string	m_associationState; /* _802_11_Authenticated, _802_11_Associated,, _802_1x_Authenticated, Valid_Ip, Active_Data, AP_Timeout, Cloud_Timeout, Disconnected,  */
		int64_t	m_disconnectByApInternalReasonCode;
		SteerType	m_steerType;
		int64_t	m_previousValidSessionId;
		ClientEapDetails	m_eapDetails;
		bool	m_isReassociation;
		int64_t	m_assocRssi;
		ClientFailureDetails	m_firstFailureDetails;
		bool	m_is11VUsed;
		int64_t	m_dynamicVlan;
		SecurityType	m_securityType;
		int64_t	m_lastEventTimestamp;
		std::string	m_hostname;
		int64_t	m_priorSessionId;
		int64_t	m_assocTimestamp;
		int64_t	m_lastTxTimestamp;
		int64_t	m_disconnectByApTimestamp;
		int64_t	m_disconnectByApReasonCode;
		int64_t	m_disconnectByClientReasonCode;
		std::string	m_apFingerprint;
		int64_t	m_portEnabledTimestamp;
		int64_t	m_firstDataSentTimestamp;
		int64_t	m_associationStatus;
		ClientSessionMetricDetails	m_metricDetails;
		std::string	m_cpUsername;
		bool	m_is11RUsed;
		std::string	m_ssid;
		std::string	m_ipAddress;
		int64_t	m_disconnectByClientTimestamp;
		int64_t	m_lastRxTimestamp;
		int64_t	m_assocInternalSC;
		int64_t	m_authTimestamp;
		int64_t	m_firstDataRcvdTimestamp;
		ClientFailureDetails	m_lastFailureDetails;
		RadioType	m_radioType;
		std::string	m_radiusUsername;
		int64_t	m_ipTimestamp;
};

class ClientActivityAggregatedStats  {
	public:
		const int64_t get_highClientCount() const { return m_highClientCount;}
		void set_highClientCount(const int64_t & value) { m_highClientCount = value; }
		const int64_t get_lowClientCount() const { return m_lowClientCount;}
		void set_lowClientCount(const int64_t & value) { m_lowClientCount = value; }
		const MinMaxAvgValueInt get_mbps() const { return m_mbps;}
		void set_mbps(const MinMaxAvgValueInt & value) { m_mbps = value; }
		const int64_t get_mediumClientCount() const { return m_mediumClientCount;}
		void set_mediumClientCount(const int64_t & value) { m_mediumClientCount = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_highClientCount;
		int64_t	m_lowClientCount;
		MinMaxAvgValueInt	m_mbps;
		int64_t	m_mediumClientCount;
};

class RealTimeSipCallStopEvent  : public RealTimeEvent {
	public:
		const int64_t get_callDuration() const { return m_callDuration;}
		void set_callDuration(const int64_t & value) { m_callDuration = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SipCallStopReason get_reason() const { return m_reason;}
		void set_reason(const SipCallStopReason & value) { m_reason = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_callDuration;
		std::string	m_model_type;
		SipCallStopReason	m_reason;
};

class DaysOfWeekTimeRangeSchedule  {
	public:
		const std::vector<DayOfWeek> get_daysOfWeek() const { return m_daysOfWeek;}
		void set_daysOfWeek(const std::vector<DayOfWeek> & value) { m_daysOfWeek = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const LocalTimeValue get_timeBegin() const { return m_timeBegin;}
		void set_timeBegin(const LocalTimeValue & value) { m_timeBegin = value; }
		const LocalTimeValue get_timeEnd() const { return m_timeEnd;}
		void set_timeEnd(const LocalTimeValue & value) { m_timeEnd = value; }
		const std::string get_timezone() const { return m_timezone;}
		void set_timezone(const std::string & value) { m_timezone = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<DayOfWeek>	m_daysOfWeek;
		std::string	m_model_type;
		LocalTimeValue	m_timeBegin;
		LocalTimeValue	m_timeEnd;
		std::string	m_timezone;
};

class NetworkAdminStatusData  {
	public:
		const IntegerPerRadioTypeMap get_averageCoveragePerRadio() const { return m_averageCoveragePerRadio;}
		void set_averageCoveragePerRadio(const IntegerPerRadioTypeMap & value) { m_averageCoveragePerRadio = value; }
		const StatusCode get_cloudLinkStatus() const { return m_cloudLinkStatus;}
		void set_cloudLinkStatus(const StatusCode & value) { m_cloudLinkStatus = value; }
		const StatusCode get_dhcpStatus() const { return m_dhcpStatus;}
		void set_dhcpStatus(const StatusCode & value) { m_dhcpStatus = value; }
		const StatusCode get_dnsStatus() const { return m_dnsStatus;}
		void set_dnsStatus(const StatusCode & value) { m_dnsStatus = value; }
		const IntegerStatusCodeMap get_equipmentCountsBySeverity() const { return m_equipmentCountsBySeverity;}
		void set_equipmentCountsBySeverity(const IntegerStatusCodeMap & value) { m_equipmentCountsBySeverity = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const StatusCode get_radiusStatus() const { return m_radiusStatus;}
		void set_radiusStatus(const StatusCode & value) { m_radiusStatus = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		IntegerPerRadioTypeMap	m_averageCoveragePerRadio;
		StatusCode	m_cloudLinkStatus;
		StatusCode	m_dhcpStatus;
		StatusCode	m_dnsStatus;
		IntegerStatusCodeMap	m_equipmentCountsBySeverity;
		std::string	m_model_type;
		StatusCode	m_radiusStatus;
		std::string	m_statusDataType;
};

class BonjourGatewayProfile  : public ProfileDetails {
	public:
		const std::vector<BonjourServiceSet> get_bonjourServices() const { return m_bonjourServices;}
		void set_bonjourServices(const std::vector<BonjourServiceSet> & value) { m_bonjourServices = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_profileDescription() const { return m_profileDescription;}
		void set_profileDescription(const std::string & value) { m_profileDescription = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<BonjourServiceSet>	m_bonjourServices;
		std::string	m_model_type;
		std::string	m_profileDescription;
};

class RadiusProfile  : public ProfileDetails {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const RadiusServer get_primaryRadiusAccountingServer() const { return m_primaryRadiusAccountingServer;}
		void set_primaryRadiusAccountingServer(const RadiusServer & value) { m_primaryRadiusAccountingServer = value; }
		const RadiusServer get_primaryRadiusAuthServer() const { return m_primaryRadiusAuthServer;}
		void set_primaryRadiusAuthServer(const RadiusServer & value) { m_primaryRadiusAuthServer = value; }
		const RadiusServer get_secondaryRadiusAccountingServer() const { return m_secondaryRadiusAccountingServer;}
		void set_secondaryRadiusAccountingServer(const RadiusServer & value) { m_secondaryRadiusAccountingServer = value; }
		const RadiusServer get_secondaryRadiusAuthServer() const { return m_secondaryRadiusAuthServer;}
		void set_secondaryRadiusAuthServer(const RadiusServer & value) { m_secondaryRadiusAuthServer = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		RadiusServer	m_primaryRadiusAccountingServer; /* the primary radius accounting server */
		RadiusServer	m_primaryRadiusAuthServer; /* the primary radius auth server */
		RadiusServer	m_secondaryRadiusAccountingServer; /* the secondary radius accounting server, currently the AP supports only 1 radius auth server and 1 radius accounting server, so this is not provisioned on the AP */
		RadiusServer	m_secondaryRadiusAuthServer; /* the secondary radius auth server, currently the AP supports only 1 radius auth server and 1 radius accounting server, so this is not provisioned on the AP */
};

class PaginationResponsePortalUser  {
	public:
		const PaginationContextPortalUser get_context() const { return m_context;}
		void set_context(const PaginationContextPortalUser & value) { m_context = value; }
		const std::vector<PortalUser> get_items() const { return m_items;}
		void set_items(const std::vector<PortalUser> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextPortalUser	m_context;
		std::vector<PortalUser>	m_items;
};

class MeshGroup  : public ProfileDetails {
	public:
		const std::vector<MeshGroupMember> get_members() const { return m_members;}
		void set_members(const std::vector<MeshGroupMember> & value) { m_members = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const MeshGroupProperty get_property() const { return m_property;}
		void set_property(const MeshGroupProperty & value) { m_property = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<MeshGroupMember>	m_members;
		std::string	m_model_type;
		MeshGroupProperty	m_property;
};

class MetricConfigParameterMap  {
	public:
		const ServiceMetricSurveyConfigParameters get_ApNode() const { return m_ApNode;}
		void set_ApNode(const ServiceMetricSurveyConfigParameters & value) { m_ApNode = value; }
		const ServiceMetricRadioConfigParameters get_ApSsid() const { return m_ApSsid;}
		void set_ApSsid(const ServiceMetricRadioConfigParameters & value) { m_ApSsid = value; }
		const ServiceMetricSurveyConfigParameters get_Channel() const { return m_Channel;}
		void set_Channel(const ServiceMetricSurveyConfigParameters & value) { m_Channel = value; }
		const ServiceMetricRadioConfigParameters get_Client() const { return m_Client;}
		void set_Client(const ServiceMetricRadioConfigParameters & value) { m_Client = value; }
		const ServiceMetricConfigParameters get_ClientQoE() const { return m_ClientQoE;}
		void set_ClientQoE(const ServiceMetricConfigParameters & value) { m_ClientQoE = value; }
		const ServiceMetricSurveyConfigParameters get_Neighbour() const { return m_Neighbour;}
		void set_Neighbour(const ServiceMetricSurveyConfigParameters & value) { m_Neighbour = value; }
		const ServiceMetricConfigParameters get_QoE() const { return m_QoE;}
		void set_QoE(const ServiceMetricConfigParameters & value) { m_QoE = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ServiceMetricSurveyConfigParameters	m_ApNode;
		ServiceMetricRadioConfigParameters	m_ApSsid;
		ServiceMetricSurveyConfigParameters	m_Channel;
		ServiceMetricRadioConfigParameters	m_Client;
		ServiceMetricConfigParameters	m_ClientQoE;
		ServiceMetricSurveyConfigParameters	m_Neighbour;
		ServiceMetricConfigParameters	m_QoE;
};

class PortalUserAddedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const PortalUser get_payload() const { return m_payload;}
		void set_payload(const PortalUser & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		PortalUser	m_payload;
};

class RealTimeSipCallStartEvent  : public RealTimeEvent {
	public:
		const int64_t get_associationId() const { return m_associationId;}
		void set_associationId(const int64_t & value) { m_associationId = value; }
		const int64_t get_channel() const { return m_channel;}
		void set_channel(const int64_t & value) { m_channel = value; }
		const std::vector<std::string> get_codecs() const { return m_codecs;}
		void set_codecs(const std::vector<std::string> & value) { m_codecs = value; }
		const std::string get_deviceInfo() const { return m_deviceInfo;}
		void set_deviceInfo(const std::string & value) { m_deviceInfo = value; }
		const MacAddress get_macAddress() const { return m_macAddress;}
		void set_macAddress(const MacAddress & value) { m_macAddress = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_providerDomain() const { return m_providerDomain;}
		void set_providerDomain(const std::string & value) { m_providerDomain = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const int64_t get_sipCallId() const { return m_sipCallId;}
		void set_sipCallId(const int64_t & value) { m_sipCallId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_associationId;
		int64_t	m_channel;
		std::vector<std::string>	m_codecs;
		std::string	m_deviceInfo;
		MacAddress	m_macAddress;
		std::string	m_model_type;
		std::string	m_providerDomain;
		RadioType	m_radioType;
		int64_t	m_sipCallId;
};

class RealTimeStreamingStopEvent  : public RealTimeEvent {
	public:
		const MacAddress get_clientMac() const { return m_clientMac;}
		void set_clientMac(const MacAddress & value) { m_clientMac = value; }
		const int64_t get_durationInSecs() const { return m_durationInSecs;}
		void set_durationInSecs(const int64_t & value) { m_durationInSecs = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_serverIp() const { return m_serverIp;}
		void set_serverIp(const std::string & value) { m_serverIp = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const int64_t get_totalBytes() const { return m_totalBytes;}
		void set_totalBytes(const int64_t & value) { m_totalBytes = value; }
		const StreamingVideoType get_type() const { return m_type;}
		void set_type(const StreamingVideoType & value) { m_type = value; }
		const int64_t get_videoSessionId() const { return m_videoSessionId;}
		void set_videoSessionId(const int64_t & value) { m_videoSessionId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_clientMac;
		int64_t	m_durationInSecs;
		std::string	m_model_type;
		std::string	m_serverIp; /* string representing InetAddress */
		int64_t	m_sessionId;
		int64_t	m_totalBytes;
		StreamingVideoType	m_type;
		int64_t	m_videoSessionId;
};

class RadioUtilizationPerRadioDetailsMap  {
	public:
		const RadioUtilizationPerRadioDetails get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const RadioUtilizationPerRadioDetails & value) { m_is2dot4GHz = value; }
		const RadioUtilizationPerRadioDetails get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const RadioUtilizationPerRadioDetails & value) { m_is5GHz = value; }
		const RadioUtilizationPerRadioDetails get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const RadioUtilizationPerRadioDetails & value) { m_is5GHzL = value; }
		const RadioUtilizationPerRadioDetails get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const RadioUtilizationPerRadioDetails & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RadioUtilizationPerRadioDetails	m_is2dot4GHz;
		RadioUtilizationPerRadioDetails	m_is5GHz;
		RadioUtilizationPerRadioDetails	m_is5GHzL;
		RadioUtilizationPerRadioDetails	m_is5GHzU;
};

class NetworkProbeMetrics  {
	public:
		const int64_t get_dhcpLatencyMs() const { return m_dhcpLatencyMs;}
		void set_dhcpLatencyMs(const int64_t & value) { m_dhcpLatencyMs = value; }
		const StateUpDownError get_dhcpState() const { return m_dhcpState;}
		void set_dhcpState(const StateUpDownError & value) { m_dhcpState = value; }
		const int64_t get_dnsLatencyMs() const { return m_dnsLatencyMs;}
		void set_dnsLatencyMs(const int64_t & value) { m_dnsLatencyMs = value; }
		const std::vector<DnsProbeMetric> get_dnsProbeResults() const { return m_dnsProbeResults;}
		void set_dnsProbeResults(const std::vector<DnsProbeMetric> & value) { m_dnsProbeResults = value; }
		const StateUpDownError get_dnsState() const { return m_dnsState;}
		void set_dnsState(const StateUpDownError & value) { m_dnsState = value; }
		const int64_t get_radiusLatencyMs() const { return m_radiusLatencyMs;}
		void set_radiusLatencyMs(const int64_t & value) { m_radiusLatencyMs = value; }
		const StateUpDownError get_radiusState() const { return m_radiusState;}
		void set_radiusState(const StateUpDownError & value) { m_radiusState = value; }
		const std::string get_vlanIF() const { return m_vlanIF;}
		void set_vlanIF(const std::string & value) { m_vlanIF = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_dhcpLatencyMs;
		StateUpDownError	m_dhcpState;
		int64_t	m_dnsLatencyMs;
		std::vector<DnsProbeMetric>	m_dnsProbeResults;
		StateUpDownError	m_dnsState;
		int64_t	m_radiusLatencyMs;
		StateUpDownError	m_radiusState;
		std::string	m_vlanIF;
};

class RrmBulkUpdateApDetails  {
	public:
		const int64_t get_backupChannelNumber() const { return m_backupChannelNumber;}
		void set_backupChannelNumber(const int64_t & value) { m_backupChannelNumber = value; }
		const int64_t get_channelNumber() const { return m_channelNumber;}
		void set_channelNumber(const int64_t & value) { m_channelNumber = value; }
		const SourceSelectionValue get_clientDisconnectThresholdDb() const { return m_clientDisconnectThresholdDb;}
		void set_clientDisconnectThresholdDb(const SourceSelectionValue & value) { m_clientDisconnectThresholdDb = value; }
		const int64_t get_dropInSnrPercentage() const { return m_dropInSnrPercentage;}
		void set_dropInSnrPercentage(const int64_t & value) { m_dropInSnrPercentage = value; }
		const int64_t get_minLoadFactor() const { return m_minLoadFactor;}
		void set_minLoadFactor(const int64_t & value) { m_minLoadFactor = value; }
		const SourceSelectionValue get_probeResponseThresholdDb() const { return m_probeResponseThresholdDb;}
		void set_probeResponseThresholdDb(const SourceSelectionValue & value) { m_probeResponseThresholdDb = value; }
		const SourceSelectionValue get_rxCellSizeDb() const { return m_rxCellSizeDb;}
		void set_rxCellSizeDb(const SourceSelectionValue & value) { m_rxCellSizeDb = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_backupChannelNumber;
		int64_t	m_channelNumber;
		SourceSelectionValue	m_clientDisconnectThresholdDb;
		int64_t	m_dropInSnrPercentage;
		int64_t	m_minLoadFactor;
		SourceSelectionValue	m_probeResponseThresholdDb;
		SourceSelectionValue	m_rxCellSizeDb;
};

class RfElementConfiguration  {
	public:
		const ActiveScanSettings get_activeScanSettings() const { return m_activeScanSettings;}
		void set_activeScanSettings(const ActiveScanSettings & value) { m_activeScanSettings = value; }
		const bool get_autoChannelSelection() const { return m_autoChannelSelection;}
		void set_autoChannelSelection(const bool & value) { m_autoChannelSelection = value; }
		const int64_t get_beaconInterval() const { return m_beaconInterval;}
		void set_beaconInterval(const int64_t & value) { m_beaconInterval = value; }
		const bool get_bestApEnabled() const { return m_bestApEnabled;}
		void set_bestApEnabled(const bool & value) { m_bestApEnabled = value; }
		const RadioBestApSettings get_bestApSettings() const { return m_bestApSettings;}
		void set_bestApSettings(const RadioBestApSettings & value) { m_bestApSettings = value; }
		const ChannelBandwidth get_channelBandwidth() const { return m_channelBandwidth;}
		void set_channelBandwidth(const ChannelBandwidth & value) { m_channelBandwidth = value; }
		const ChannelHopSettings get_channelHopSettings() const { return m_channelHopSettings;}
		void set_channelHopSettings(const ChannelHopSettings & value) { m_channelHopSettings = value; }
		const int64_t get_clientDisconnectThresholdDb() const { return m_clientDisconnectThresholdDb;}
		void set_clientDisconnectThresholdDb(const int64_t & value) { m_clientDisconnectThresholdDb = value; }
		const int64_t get_eirpTxPower() const { return m_eirpTxPower;}
		void set_eirpTxPower(const int64_t & value) { m_eirpTxPower = value; }
		const StateSetting get_forceScanDuringVoice() const { return m_forceScanDuringVoice;}
		void set_forceScanDuringVoice(const StateSetting & value) { m_forceScanDuringVoice = value; }
		const ManagementRate get_managementRate() const { return m_managementRate;}
		void set_managementRate(const ManagementRate & value) { m_managementRate = value; }
		const int64_t get_maxNumClients() const { return m_maxNumClients;}
		void set_maxNumClients(const int64_t & value) { m_maxNumClients = value; }
		const MimoMode get_mimoMode() const { return m_mimoMode;}
		void set_mimoMode(const MimoMode & value) { m_mimoMode = value; }
		const int64_t get_minAutoCellSize() const { return m_minAutoCellSize;}
		void set_minAutoCellSize(const int64_t & value) { m_minAutoCellSize = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const MulticastRate get_multicastRate() const { return m_multicastRate;}
		void set_multicastRate(const MulticastRate & value) { m_multicastRate = value; }
		const NeighbouringAPListConfiguration get_neighbouringListApConfig() const { return m_neighbouringListApConfig;}
		void set_neighbouringListApConfig(const NeighbouringAPListConfiguration & value) { m_neighbouringListApConfig = value; }
		const bool get_perimeterDetectionEnabled() const { return m_perimeterDetectionEnabled;}
		void set_perimeterDetectionEnabled(const bool & value) { m_perimeterDetectionEnabled = value; }
		const int64_t get_probeResponseThresholdDb() const { return m_probeResponseThresholdDb;}
		void set_probeResponseThresholdDb(const int64_t & value) { m_probeResponseThresholdDb = value; }
		const RadioMode get_radioMode() const { return m_radioMode;}
		void set_radioMode(const RadioMode & value) { m_radioMode = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const std::string get_rf() const { return m_rf;}
		void set_rf(const std::string & value) { m_rf = value; }
		const int64_t get_rtsCtsThreshold() const { return m_rtsCtsThreshold;}
		void set_rtsCtsThreshold(const int64_t & value) { m_rtsCtsThreshold = value; }
		const int64_t get_rxCellSizeDb() const { return m_rxCellSizeDb;}
		void set_rxCellSizeDb(const int64_t & value) { m_rxCellSizeDb = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ActiveScanSettings	m_activeScanSettings;
		bool	m_autoChannelSelection;
		int64_t	m_beaconInterval;
		bool	m_bestApEnabled;
		RadioBestApSettings	m_bestApSettings;
		ChannelBandwidth	m_channelBandwidth;
		ChannelHopSettings	m_channelHopSettings;
		int64_t	m_clientDisconnectThresholdDb;
		int64_t	m_eirpTxPower;
		StateSetting	m_forceScanDuringVoice;
		ManagementRate	m_managementRate;
		int64_t	m_maxNumClients;
		MimoMode	m_mimoMode;
		int64_t	m_minAutoCellSize;
		std::string	m_model_type;
		MulticastRate	m_multicastRate;
		NeighbouringAPListConfiguration	m_neighbouringListApConfig;
		bool	m_perimeterDetectionEnabled;
		int64_t	m_probeResponseThresholdDb;
		RadioMode	m_radioMode;
		RadioType	m_radioType;
		std::string	m_rf;
		int64_t	m_rtsCtsThreshold;
		int64_t	m_rxCellSizeDb;
};

class ListOfChannelInfoReportsPerRadioMap  {
	public:
		const std::vector<ChannelInfo> get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const std::vector<ChannelInfo> & value) { m_is2dot4GHz = value; }
		const std::vector<ChannelInfo> get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const std::vector<ChannelInfo> & value) { m_is5GHz = value; }
		const std::vector<ChannelInfo> get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const std::vector<ChannelInfo> & value) { m_is5GHzL = value; }
		const std::vector<ChannelInfo> get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const std::vector<ChannelInfo> & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<ChannelInfo>	m_is2dot4GHz;
		std::vector<ChannelInfo>	m_is5GHz;
		std::vector<ChannelInfo>	m_is5GHzL;
		std::vector<ChannelInfo>	m_is5GHzU;
};

class BaseDhcpEvent  : public SystemEvent {
	public:
		const std::string get_clientIp() const { return m_clientIp;}
		void set_clientIp(const std::string & value) { m_clientIp = value; }
		const MacAddress get_clientMacAddress() const { return m_clientMacAddress;}
		void set_clientMacAddress(const MacAddress & value) { m_clientMacAddress = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const std::string get_dhcpServerIp() const { return m_dhcpServerIp;}
		void set_dhcpServerIp(const std::string & value) { m_dhcpServerIp = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_relayIp() const { return m_relayIp;}
		void set_relayIp(const std::string & value) { m_relayIp = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const int64_t get_vlanId() const { return m_vlanId;}
		void set_vlanId(const int64_t & value) { m_vlanId = value; }
		const int64_t get_xId() const { return m_xId;}
		void set_xId(const int64_t & value) { m_xId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_clientIp; /* string representing InetAddress */
		MacAddress	m_clientMacAddress;
		int64_t	m_customerId;
		std::string	m_dhcpServerIp; /* string representing InetAddress */
		int64_t	m_equipmentId;
		std::string	m_model_type;
		std::string	m_relayIp; /* string representing InetAddress */
		int64_t	m_sessionId;
		int64_t	m_vlanId;
		int64_t	m_xId;
};

class LocationActivityDetailsMap  {
	public:
		const LocationActivityDetails get_FRIDAY() const { return m_FRIDAY;}
		void set_FRIDAY(const LocationActivityDetails & value) { m_FRIDAY = value; }
		const LocationActivityDetails get_MONDAY() const { return m_MONDAY;}
		void set_MONDAY(const LocationActivityDetails & value) { m_MONDAY = value; }
		const LocationActivityDetails get_SATURDAY() const { return m_SATURDAY;}
		void set_SATURDAY(const LocationActivityDetails & value) { m_SATURDAY = value; }
		const LocationActivityDetails get_SUNDAY() const { return m_SUNDAY;}
		void set_SUNDAY(const LocationActivityDetails & value) { m_SUNDAY = value; }
		const LocationActivityDetails get_THURSDAY() const { return m_THURSDAY;}
		void set_THURSDAY(const LocationActivityDetails & value) { m_THURSDAY = value; }
		const LocationActivityDetails get_TUESDAY() const { return m_TUESDAY;}
		void set_TUESDAY(const LocationActivityDetails & value) { m_TUESDAY = value; }
		const LocationActivityDetails get_WEDNESDAY() const { return m_WEDNESDAY;}
		void set_WEDNESDAY(const LocationActivityDetails & value) { m_WEDNESDAY = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		LocationActivityDetails	m_FRIDAY;
		LocationActivityDetails	m_MONDAY;
		LocationActivityDetails	m_SATURDAY;
		LocationActivityDetails	m_SUNDAY;
		LocationActivityDetails	m_THURSDAY;
		LocationActivityDetails	m_TUESDAY;
		LocationActivityDetails	m_WEDNESDAY;
};

class ChannelUtilizationPerRadioDetails  {
	public:
		const MinMaxAvgValueInt get_channelUtilization() const { return m_channelUtilization;}
		void set_channelUtilization(const MinMaxAvgValueInt & value) { m_channelUtilization = value; }
		const int64_t get_numBadEquipment() const { return m_numBadEquipment;}
		void set_numBadEquipment(const int64_t & value) { m_numBadEquipment = value; }
		const int64_t get_numGoodEquipment() const { return m_numGoodEquipment;}
		void set_numGoodEquipment(const int64_t & value) { m_numGoodEquipment = value; }
		const int64_t get_numWarnEquipment() const { return m_numWarnEquipment;}
		void set_numWarnEquipment(const int64_t & value) { m_numWarnEquipment = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MinMaxAvgValueInt	m_channelUtilization;
		int64_t	m_numBadEquipment;
		int64_t	m_numGoodEquipment;
		int64_t	m_numWarnEquipment;
};

class PasspointVenueProfile  : public ProfileDetails {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::vector<PasspointVenueName> get_passpointVenueNameSet() const { return m_passpointVenueNameSet;}
		void set_passpointVenueNameSet(const std::vector<PasspointVenueName> & value) { m_passpointVenueNameSet = value; }
		const std::vector<PasspointVenueTypeAssignment> get_passpointVenueTypeAssignment() const { return m_passpointVenueTypeAssignment;}
		void set_passpointVenueTypeAssignment(const std::vector<PasspointVenueTypeAssignment> & value) { m_passpointVenueTypeAssignment = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		std::vector<PasspointVenueName>	m_passpointVenueNameSet;
		std::vector<PasspointVenueTypeAssignment>	m_passpointVenueTypeAssignment;
};

class NoiseFloorPerRadioDetails  {
	public:
		const MinMaxAvgValueInt get_noiseFloor() const { return m_noiseFloor;}
		void set_noiseFloor(const MinMaxAvgValueInt & value) { m_noiseFloor = value; }
		const int64_t get_numBadEquipment() const { return m_numBadEquipment;}
		void set_numBadEquipment(const int64_t & value) { m_numBadEquipment = value; }
		const int64_t get_numGoodEquipment() const { return m_numGoodEquipment;}
		void set_numGoodEquipment(const int64_t & value) { m_numGoodEquipment = value; }
		const int64_t get_numWarnEquipment() const { return m_numWarnEquipment;}
		void set_numWarnEquipment(const int64_t & value) { m_numWarnEquipment = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MinMaxAvgValueInt	m_noiseFloor;
		int64_t	m_numBadEquipment;
		int64_t	m_numGoodEquipment;
		int64_t	m_numWarnEquipment;
};

class CountsPerEquipmentIdPerAlarmCodeMap  {
	public:
		const CountsPerAlarmCodeMap get_CountsPerEquipmentIdPerAlarmCodeMap() const { return m_CountsPerEquipmentIdPerAlarmCodeMap;}
		void set_CountsPerEquipmentIdPerAlarmCodeMap(const CountsPerAlarmCodeMap & value) { m_CountsPerEquipmentIdPerAlarmCodeMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		CountsPerAlarmCodeMap	m_CountsPerEquipmentIdPerAlarmCodeMap;
};

class NeighbourScanReports  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::vector<NeighbourReport> get_neighbourReports() const { return m_neighbourReports;}
		void set_neighbourReports(const std::vector<NeighbourReport> & value) { m_neighbourReports = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		std::vector<NeighbourReport>	m_neighbourReports;
};

class DhcpDiscoverEvent  : public BaseDhcpEvent {
	public:
		const std::string get_hostName() const { return m_hostName;}
		void set_hostName(const std::string & value) { m_hostName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_hostName;
		std::string	m_model_type;
};

class ClientMetrics  {
	public:
		const int64_t get_numTxHT_216_6_Mbps() const { return m_numTxHT_216_6_Mbps;}
		void set_numTxHT_216_6_Mbps(const int64_t & value) { m_numTxHT_216_6_Mbps = value; }
		const int64_t get_numRxHT_325_Mbps() const { return m_numRxHT_325_Mbps;}
		void set_numRxHT_325_Mbps(const int64_t & value) { m_numRxHT_325_Mbps = value; }
		const int64_t get_numRxVHT_2340_Mbps() const { return m_numRxVHT_2340_Mbps;}
		void set_numRxVHT_2340_Mbps(const int64_t & value) { m_numRxVHT_2340_Mbps = value; }
		const int64_t get_numRxVHT_1733_1_Mbps() const { return m_numRxVHT_1733_1_Mbps;}
		void set_numRxVHT_1733_1_Mbps(const int64_t & value) { m_numRxVHT_1733_1_Mbps = value; }
		const int64_t get_numRxVHT_702_Mbps() const { return m_numRxVHT_702_Mbps;}
		void set_numRxVHT_702_Mbps(const int64_t & value) { m_numRxVHT_702_Mbps = value; }
		const int64_t get_numRxVHT_433_2_Mbps() const { return m_numRxVHT_433_2_Mbps;}
		void set_numRxVHT_433_2_Mbps(const int64_t & value) { m_numRxVHT_433_2_Mbps = value; }
		const int64_t get_numTxHT_6_5_Mbps() const { return m_numTxHT_6_5_Mbps;}
		void set_numTxHT_6_5_Mbps(const int64_t & value) { m_numTxHT_6_5_Mbps = value; }
		const int64_t get_numRxTimeData() const { return m_numRxTimeData;}
		void set_numRxTimeData(const int64_t & value) { m_numRxTimeData = value; }
		const int64_t get_numTxPackets() const { return m_numTxPackets;}
		void set_numTxPackets(const int64_t & value) { m_numTxPackets = value; }
		const int64_t get_numRxHT_351_2_Mbps() const { return m_numRxHT_351_2_Mbps;}
		void set_numRxHT_351_2_Mbps(const int64_t & value) { m_numRxHT_351_2_Mbps = value; }
		const int64_t get_numRxHT_216_Mbps() const { return m_numRxHT_216_Mbps;}
		void set_numRxHT_216_Mbps(const int64_t & value) { m_numRxHT_216_Mbps = value; }
		const int64_t get_numRxVHT_405_Mbps() const { return m_numRxVHT_405_Mbps;}
		void set_numRxVHT_405_Mbps(const int64_t & value) { m_numRxVHT_405_Mbps = value; }
		const int64_t get_numRxHT_360_Mbps() const { return m_numRxHT_360_Mbps;}
		void set_numRxHT_360_Mbps(const int64_t & value) { m_numRxHT_360_Mbps = value; }
		const int64_t get_numRxHT_120_Mbps() const { return m_numRxHT_120_Mbps;}
		void set_numRxHT_120_Mbps(const int64_t & value) { m_numRxHT_120_Mbps = value; }
		const int64_t get_numTxVHT_702_Mbps() const { return m_numTxVHT_702_Mbps;}
		void set_numTxVHT_702_Mbps(const int64_t & value) { m_numTxVHT_702_Mbps = value; }
		const int64_t get_numTxVHT_936_Mbps() const { return m_numTxVHT_936_Mbps;}
		void set_numTxVHT_936_Mbps(const int64_t & value) { m_numTxVHT_936_Mbps = value; }
		const int64_t get_numTxVHT_390_Mbps() const { return m_numTxVHT_390_Mbps;}
		void set_numTxVHT_390_Mbps(const int64_t & value) { m_numTxVHT_390_Mbps = value; }
		const int64_t get_numTxDataFrames_1300Plus_Mbps() const { return m_numTxDataFrames_1300Plus_Mbps;}
		void set_numTxDataFrames_1300Plus_Mbps(const int64_t & value) { m_numTxDataFrames_1300Plus_Mbps = value; }
		const int64_t get_numRxHT_150_Mbps() const { return m_numRxHT_150_Mbps;}
		void set_numRxHT_150_Mbps(const int64_t & value) { m_numRxHT_150_Mbps = value; }
		const int64_t get_numRxBytes() const { return m_numRxBytes;}
		void set_numRxBytes(const int64_t & value) { m_numRxBytes = value; }
		const int64_t get_numTx_18_Mbps() const { return m_numTx_18_Mbps;}
		void set_numTx_18_Mbps(const int64_t & value) { m_numTx_18_Mbps = value; }
		const int64_t get_numTxVHT_1755_Mbps() const { return m_numTxVHT_1755_Mbps;}
		void set_numTxVHT_1755_Mbps(const int64_t & value) { m_numTxVHT_1755_Mbps = value; }
		const int64_t get_numTxVHT_364_5_Mbps() const { return m_numTxVHT_364_5_Mbps;}
		void set_numTxVHT_364_5_Mbps(const int64_t & value) { m_numTxVHT_364_5_Mbps = value; }
		const int64_t get_numRxNullData() const { return m_numRxNullData;}
		void set_numRxNullData(const int64_t & value) { m_numRxNullData = value; }
		const int64_t get_numTxHT_43_2_Mbps() const { return m_numTxHT_43_2_Mbps;}
		void set_numTxHT_43_2_Mbps(const int64_t & value) { m_numTxHT_43_2_Mbps = value; }
		const McsType get_lastTxMcsIdx() const { return m_lastTxMcsIdx;}
		void set_lastTxMcsIdx(const McsType & value) { m_lastTxMcsIdx = value; }
		const int64_t get_rxDataBytes() const { return m_rxDataBytes;}
		void set_rxDataBytes(const int64_t & value) { m_rxDataBytes = value; }
		const int64_t get_numRxHT_39_Mbps() const { return m_numRxHT_39_Mbps;}
		void set_numRxHT_39_Mbps(const int64_t & value) { m_numRxHT_39_Mbps = value; }
		const int64_t get_numTxHT_81_Mbps() const { return m_numTxHT_81_Mbps;}
		void set_numTxHT_81_Mbps(const int64_t & value) { m_numTxHT_81_Mbps = value; }
		const int64_t get_numTxHT_39_Mbps() const { return m_numTxHT_39_Mbps;}
		void set_numTxHT_39_Mbps(const int64_t & value) { m_numTxHT_39_Mbps = value; }
		const int64_t get_numRxHT_104_Mbps() const { return m_numRxHT_104_Mbps;}
		void set_numRxHT_104_Mbps(const int64_t & value) { m_numRxHT_104_Mbps = value; }
		const int64_t get_numRxVHT_432_Mbps() const { return m_numRxVHT_432_Mbps;}
		void set_numRxVHT_432_Mbps(const int64_t & value) { m_numRxVHT_432_Mbps = value; }
		const int64_t get_numTxVHT_486_Mbps() const { return m_numTxVHT_486_Mbps;}
		void set_numTxVHT_486_Mbps(const int64_t & value) { m_numTxVHT_486_Mbps = value; }
		const int64_t get_numRxHT_28_8_Mbps() const { return m_numRxHT_28_8_Mbps;}
		void set_numRxHT_28_8_Mbps(const int64_t & value) { m_numRxHT_28_8_Mbps = value; }
		const int64_t get_numTxDropped() const { return m_numTxDropped;}
		void set_numTxDropped(const int64_t & value) { m_numTxDropped = value; }
		const int64_t get_numTxVHT_600_Mbps() const { return m_numTxVHT_600_Mbps;}
		void set_numTxVHT_600_Mbps(const int64_t & value) { m_numTxVHT_600_Mbps = value; }
		const int64_t get_numRxVHT_648_Mbps() const { return m_numRxVHT_648_Mbps;}
		void set_numRxVHT_648_Mbps(const int64_t & value) { m_numRxVHT_648_Mbps = value; }
		const int64_t get_numTxVHT_720_Mbps() const { return m_numTxVHT_720_Mbps;}
		void set_numTxVHT_720_Mbps(const int64_t & value) { m_numTxVHT_720_Mbps = value; }
		const int64_t get_numRxFramesReceived() const { return m_numRxFramesReceived;}
		void set_numRxFramesReceived(const int64_t & value) { m_numRxFramesReceived = value; }
		const int64_t get_numRxHT_121_5_Mbps() const { return m_numRxHT_121_5_Mbps;}
		void set_numRxHT_121_5_Mbps(const int64_t & value) { m_numRxHT_121_5_Mbps = value; }
		const int64_t get_numTxVHT_403_Mbps() const { return m_numTxVHT_403_Mbps;}
		void set_numTxVHT_403_Mbps(const int64_t & value) { m_numTxVHT_403_Mbps = value; }
		const int64_t get_numRx_48_Mbps() const { return m_numRx_48_Mbps;}
		void set_numRx_48_Mbps(const int64_t & value) { m_numRx_48_Mbps = value; }
		const int64_t get_numTxAction() const { return m_numTxAction;}
		void set_numTxAction(const int64_t & value) { m_numTxAction = value; }
		const int64_t get_numRxVHT_2808_Mbps() const { return m_numRxVHT_2808_Mbps;}
		void set_numRxVHT_2808_Mbps(const int64_t & value) { m_numRxVHT_2808_Mbps = value; }
		const int64_t get_numTxSuccessWithRetry() const { return m_numTxSuccessWithRetry;}
		void set_numTxSuccessWithRetry(const int64_t & value) { m_numTxSuccessWithRetry = value; }
		const int64_t get_numTxHT_30_Mbps() const { return m_numTxHT_30_Mbps;}
		void set_numTxHT_30_Mbps(const int64_t & value) { m_numTxHT_30_Mbps = value; }
		const int64_t get_txRetries() const { return m_txRetries;}
		void set_txRetries(const int64_t & value) { m_txRetries = value; }
		const int64_t get_numRxHT_292_5_Mbps() const { return m_numRxHT_292_5_Mbps;}
		void set_numRxHT_292_5_Mbps(const int64_t & value) { m_numRxHT_292_5_Mbps = value; }
		const int64_t get_numRxAck() const { return m_numRxAck;}
		void set_numRxAck(const int64_t & value) { m_numRxAck = value; }
		const int64_t get_numRxHT_19_5_Mbps() const { return m_numRxHT_19_5_Mbps;}
		void set_numRxHT_19_5_Mbps(const int64_t & value) { m_numRxHT_19_5_Mbps = value; }
		const int64_t get_numTxHT_173_3_Mbps() const { return m_numTxHT_173_3_Mbps;}
		void set_numTxHT_173_3_Mbps(const int64_t & value) { m_numTxHT_173_3_Mbps = value; }
		const int64_t get_numTxHT_240_Mbps() const { return m_numTxHT_240_Mbps;}
		void set_numTxHT_240_Mbps(const int64_t & value) { m_numTxHT_240_Mbps = value; }
		const double get_averageTxRate() const { return m_averageTxRate;}
		void set_averageTxRate(const double & value) { m_averageTxRate = value; }
		const int64_t get_numTxEapol() const { return m_numTxEapol;}
		void set_numTxEapol(const int64_t & value) { m_numTxEapol = value; }
		const int64_t get_numRxHT_288_7_Mbps() const { return m_numRxHT_288_7_Mbps;}
		void set_numRxHT_288_7_Mbps(const int64_t & value) { m_numRxHT_288_7_Mbps = value; }
		const int64_t get_numTxManagement() const { return m_numTxManagement;}
		void set_numTxManagement(const int64_t & value) { m_numTxManagement = value; }
		const int64_t get_numRxVHT_1755_Mbps() const { return m_numRxVHT_1755_Mbps;}
		void set_numRxVHT_1755_Mbps(const int64_t & value) { m_numRxVHT_1755_Mbps = value; }
		const int64_t get_numRxPspoll() const { return m_numRxPspoll;}
		void set_numRxPspoll(const int64_t & value) { m_numRxPspoll = value; }
		const int64_t get_numTxVHT_433_2_Mbps() const { return m_numTxVHT_433_2_Mbps;}
		void set_numTxVHT_433_2_Mbps(const int64_t & value) { m_numTxVHT_433_2_Mbps = value; }
		const int64_t get_numTxVHT_585_Mbps() const { return m_numTxVHT_585_Mbps;}
		void set_numTxVHT_585_Mbps(const int64_t & value) { m_numTxVHT_585_Mbps = value; }
		const int64_t get_numTxHT_86_8_Mbps() const { return m_numTxHT_86_8_Mbps;}
		void set_numTxHT_86_8_Mbps(const int64_t & value) { m_numTxHT_86_8_Mbps = value; }
		const int64_t get_numTxHT_52_Mbps() const { return m_numTxHT_52_Mbps;}
		void set_numTxHT_52_Mbps(const int64_t & value) { m_numTxHT_52_Mbps = value; }
		const std::string get_classificationName() const { return m_classificationName;}
		void set_classificationName(const std::string & value) { m_classificationName = value; }
		const int64_t get_numRxVHT_292_5_Mbps() const { return m_numRxVHT_292_5_Mbps;}
		void set_numRxVHT_292_5_Mbps(const int64_t & value) { m_numRxVHT_292_5_Mbps = value; }
		const int64_t get_sessionId() const { return m_sessionId;}
		void set_sessionId(const int64_t & value) { m_sessionId = value; }
		const int64_t get_numRxHT_60_Mbps() const { return m_numRxHT_60_Mbps;}
		void set_numRxHT_60_Mbps(const int64_t & value) { m_numRxHT_60_Mbps = value; }
		const int64_t get_numRxTimeToMe() const { return m_numRxTimeToMe;}
		void set_numRxTimeToMe(const int64_t & value) { m_numRxTimeToMe = value; }
		const int64_t get_numTxHT_28_7_Mbps() const { return m_numTxHT_28_7_Mbps;}
		void set_numTxHT_28_7_Mbps(const int64_t & value) { m_numTxHT_28_7_Mbps = value; }
		const int64_t get_numTxHT_108_Mbps() const { return m_numTxHT_108_Mbps;}
		void set_numTxHT_108_Mbps(const int64_t & value) { m_numTxHT_108_Mbps = value; }
		const int64_t get_numRxHT_156_Mbps() const { return m_numRxHT_156_Mbps;}
		void set_numRxHT_156_Mbps(const int64_t & value) { m_numRxHT_156_Mbps = value; }
		const int64_t get_numRxVHT_468_Mbps() const { return m_numRxVHT_468_Mbps;}
		void set_numRxVHT_468_Mbps(const int64_t & value) { m_numRxVHT_468_Mbps = value; }
		const int64_t get_numTxVHT_2808_Mbps() const { return m_numTxVHT_2808_Mbps;}
		void set_numTxVHT_2808_Mbps(const int64_t & value) { m_numTxVHT_2808_Mbps = value; }
		const int64_t get_numTxVHT_526_5_Mbps() const { return m_numTxVHT_526_5_Mbps;}
		void set_numTxVHT_526_5_Mbps(const int64_t & value) { m_numTxVHT_526_5_Mbps = value; }
		const int64_t get_numTxHT_324_Mbps() const { return m_numTxHT_324_Mbps;}
		void set_numTxHT_324_Mbps(const int64_t & value) { m_numTxHT_324_Mbps = value; }
		const int64_t get_numTxVHT_866_7_Mbps() const { return m_numTxVHT_866_7_Mbps;}
		void set_numTxVHT_866_7_Mbps(const int64_t & value) { m_numTxVHT_866_7_Mbps = value; }
		const int64_t get_numRxHT_72_1_Mbps() const { return m_numRxHT_72_1_Mbps;}
		void set_numRxHT_72_1_Mbps(const int64_t & value) { m_numRxHT_72_1_Mbps = value; }
		const int64_t get_numRxHT_65_Mbps() const { return m_numRxHT_65_Mbps;}
		void set_numRxHT_65_Mbps(const int64_t & value) { m_numRxHT_65_Mbps = value; }
		const int64_t get_numTxDataTransmitted() const { return m_numTxDataTransmitted;}
		void set_numTxDataTransmitted(const int64_t & value) { m_numTxDataTransmitted = value; }
		const int64_t get_numRx_18_Mbps() const { return m_numRx_18_Mbps;}
		void set_numRx_18_Mbps(const int64_t & value) { m_numRx_18_Mbps = value; }
		const int64_t get_numRxDataFrames_108_Mbps() const { return m_numRxDataFrames_108_Mbps;}
		void set_numRxDataFrames_108_Mbps(const int64_t & value) { m_numRxDataFrames_108_Mbps = value; }
		const std::vector<int64_t> get_rates() const { return m_rates;}
		void set_rates(const std::vector<int64_t> & value) { m_rates = value; }
		const int64_t get_numTxByteSucc() const { return m_numTxByteSucc;}
		void set_numTxByteSucc(const int64_t & value) { m_numTxByteSucc = value; }
		const int64_t get_numRxVHT_1053_Mbps() const { return m_numRxVHT_1053_Mbps;}
		void set_numRxVHT_1053_Mbps(const int64_t & value) { m_numRxVHT_1053_Mbps = value; }
		const int64_t get_numRxVHT_2106_Mbps() const { return m_numRxVHT_2106_Mbps;}
		void set_numRxVHT_2106_Mbps(const int64_t & value) { m_numRxVHT_2106_Mbps = value; }
		const int64_t get_numRxHT_288_8_Mbps() const { return m_numRxHT_288_8_Mbps;}
		void set_numRxHT_288_8_Mbps(const int64_t & value) { m_numRxHT_288_8_Mbps = value; }
		const int64_t get_numRxHT_40_5_Mbps() const { return m_numRxHT_40_5_Mbps;}
		void set_numRxHT_40_5_Mbps(const int64_t & value) { m_numRxHT_40_5_Mbps = value; }
		const int64_t get_numTxVHT_1170_Mbps() const { return m_numTxVHT_1170_Mbps;}
		void set_numTxVHT_1170_Mbps(const int64_t & value) { m_numTxVHT_1170_Mbps = value; }
		const int64_t get_numTxDataRetries() const { return m_numTxDataRetries;}
		void set_numTxDataRetries(const int64_t & value) { m_numTxDataRetries = value; }
		const int64_t get_numTxHT_117_Mbps() const { return m_numTxHT_117_Mbps;}
		void set_numTxHT_117_Mbps(const int64_t & value) { m_numTxHT_117_Mbps = value; }
		const int64_t get_numTxHT_13_Mbps() const { return m_numTxHT_13_Mbps;}
		void set_numTxHT_13_Mbps(const int64_t & value) { m_numTxHT_13_Mbps = value; }
		const int64_t get_numRxVHT_1872_Mbps() const { return m_numRxVHT_1872_Mbps;}
		void set_numRxVHT_1872_Mbps(const int64_t & value) { m_numRxVHT_1872_Mbps = value; }
		const int64_t get_numTxVHT_2340_Mbps() const { return m_numTxVHT_2340_Mbps;}
		void set_numTxVHT_2340_Mbps(const int64_t & value) { m_numTxVHT_2340_Mbps = value; }
		const int64_t get_numRxHT_216_6_Mbps() const { return m_numRxHT_216_6_Mbps;}
		void set_numRxHT_216_6_Mbps(const int64_t & value) { m_numRxHT_216_6_Mbps = value; }
		const int64_t get_numTxHT_54_Mbps() const { return m_numTxHT_54_Mbps;}
		void set_numTxHT_54_Mbps(const int64_t & value) { m_numTxHT_54_Mbps = value; }
		const int64_t get_numRx_9_Mbps() const { return m_numRx_9_Mbps;}
		void set_numRx_9_Mbps(const int64_t & value) { m_numRx_9_Mbps = value; }
		const int64_t get_numRxStbc() const { return m_numRxStbc;}
		void set_numRxStbc(const int64_t & value) { m_numRxStbc = value; }
		const int64_t get_numTxHT_270_Mbps() const { return m_numTxHT_270_Mbps;}
		void set_numTxHT_270_Mbps(const int64_t & value) { m_numTxHT_270_Mbps = value; }
		const int64_t get_numTxHT_288_8_Mbps() const { return m_numTxHT_288_8_Mbps;}
		void set_numTxHT_288_8_Mbps(const int64_t & value) { m_numTxHT_288_8_Mbps = value; }
		const int64_t get_numTxHT_351_2_Mbps() const { return m_numTxHT_351_2_Mbps;}
		void set_numTxHT_351_2_Mbps(const int64_t & value) { m_numTxHT_351_2_Mbps = value; }
		const int64_t get_numRxVHT_325_Mbps() const { return m_numRxVHT_325_Mbps;}
		void set_numRxVHT_325_Mbps(const int64_t & value) { m_numRxVHT_325_Mbps = value; }
		const int64_t get_numTxHT_135_Mbps() const { return m_numTxHT_135_Mbps;}
		void set_numTxHT_135_Mbps(const int64_t & value) { m_numTxHT_135_Mbps = value; }
		const int64_t get_numRxProbeReq() const { return m_numRxProbeReq;}
		void set_numRxProbeReq(const int64_t & value) { m_numRxProbeReq = value; }
		const int64_t get_numTxVHT_1040_Mbps() const { return m_numTxVHT_1040_Mbps;}
		void set_numTxVHT_1040_Mbps(const int64_t & value) { m_numTxVHT_1040_Mbps = value; }
		const int64_t get_numTx_12_Mbps() const { return m_numTx_12_Mbps;}
		void set_numTx_12_Mbps(const int64_t & value) { m_numTx_12_Mbps = value; }
		const int64_t get_numTxHT_14_3_Mbps() const { return m_numTxHT_14_3_Mbps;}
		void set_numTxHT_14_3_Mbps(const int64_t & value) { m_numTxHT_14_3_Mbps = value; }
		const int64_t get_numTxVHT_1300_Mbps() const { return m_numTxVHT_1300_Mbps;}
		void set_numTxVHT_1300_Mbps(const int64_t & value) { m_numTxVHT_1300_Mbps = value; }
		const int64_t get_numRxHT_13_Mbps() const { return m_numRxHT_13_Mbps;}
		void set_numRxHT_13_Mbps(const int64_t & value) { m_numRxHT_13_Mbps = value; }
		const int64_t get_numRxHT_117_Mbps() const { return m_numRxHT_117_Mbps;}
		void set_numRxHT_117_Mbps(const int64_t & value) { m_numRxHT_117_Mbps = value; }
		const int64_t get_numRxHT_28_7_Mbps() const { return m_numRxHT_28_7_Mbps;}
		void set_numRxHT_28_7_Mbps(const int64_t & value) { m_numRxHT_28_7_Mbps = value; }
		const int64_t get_numRxHT_195_Mbps() const { return m_numRxHT_195_Mbps;}
		void set_numRxHT_195_Mbps(const int64_t & value) { m_numRxHT_195_Mbps = value; }
		const int64_t get_numTxLdpc() const { return m_numTxLdpc;}
		void set_numTxLdpc(const int64_t & value) { m_numTxLdpc = value; }
		const int64_t get_numTxHT_263_2_Mbps() const { return m_numTxHT_263_2_Mbps;}
		void set_numTxHT_263_2_Mbps(const int64_t & value) { m_numTxHT_263_2_Mbps = value; }
		const int64_t get_numTxHT_13_5_Mbps() const { return m_numTxHT_13_5_Mbps;}
		void set_numTxHT_13_5_Mbps(const int64_t & value) { m_numTxHT_13_5_Mbps = value; }
		const int64_t get_numTxHT_87_8_Mbps() const { return m_numTxHT_87_8_Mbps;}
		void set_numTxHT_87_8_Mbps(const int64_t & value) { m_numTxHT_87_8_Mbps = value; }
		const int64_t get_numRxVHT_480_Mbps() const { return m_numRxVHT_480_Mbps;}
		void set_numRxVHT_480_Mbps(const int64_t & value) { m_numRxVHT_480_Mbps = value; }
		const int64_t get_numRxDataFramesRetried() const { return m_numRxDataFramesRetried;}
		void set_numRxDataFramesRetried(const int64_t & value) { m_numRxDataFramesRetried = value; }
		const int64_t get_numRxVHT_1040_Mbps() const { return m_numRxVHT_1040_Mbps;}
		void set_numRxVHT_1040_Mbps(const int64_t & value) { m_numRxVHT_1040_Mbps = value; }
		const int64_t get_numTxMultiRetries() const { return m_numTxMultiRetries;}
		void set_numTxMultiRetries(const int64_t & value) { m_numTxMultiRetries = value; }
		const int64_t get_numRxDup() const { return m_numRxDup;}
		void set_numRxDup(const int64_t & value) { m_numRxDup = value; }
		const int64_t get_numTxVHT_648_Mbps() const { return m_numTxVHT_648_Mbps;}
		void set_numTxVHT_648_Mbps(const int64_t & value) { m_numTxVHT_648_Mbps = value; }
		const int64_t get_numRxData() const { return m_numRxData;}
		void set_numRxData(const int64_t & value) { m_numRxData = value; }
		const int64_t get_numTxVHT_450_Mbps() const { return m_numTxVHT_450_Mbps;}
		void set_numTxVHT_450_Mbps(const int64_t & value) { m_numTxVHT_450_Mbps = value; }
		const int64_t get_numTxVHT_432_Mbps() const { return m_numTxVHT_432_Mbps;}
		void set_numTxVHT_432_Mbps(const int64_t & value) { m_numTxVHT_432_Mbps = value; }
		const int64_t get_numRxHT_26_Mbps() const { return m_numRxHT_26_Mbps;}
		void set_numRxHT_26_Mbps(const int64_t & value) { m_numRxHT_26_Mbps = value; }
		const int64_t get_numTxHT_115_5_Mbps() const { return m_numTxHT_115_5_Mbps;}
		void set_numTxHT_115_5_Mbps(const int64_t & value) { m_numTxHT_115_5_Mbps = value; }
		const int64_t get_numRxHT_200_Mbps() const { return m_numRxHT_200_Mbps;}
		void set_numRxHT_200_Mbps(const int64_t & value) { m_numRxHT_200_Mbps = value; }
		const int64_t get_numTxHT_86_6_Mbps() const { return m_numTxHT_86_6_Mbps;}
		void set_numTxHT_86_6_Mbps(const int64_t & value) { m_numTxHT_86_6_Mbps = value; }
		const int64_t get_numRx_54_Mbps() const { return m_numRx_54_Mbps;}
		void set_numRx_54_Mbps(const int64_t & value) { m_numRx_54_Mbps = value; }
		const int64_t get_numTxBytes() const { return m_numTxBytes;}
		void set_numTxBytes(const int64_t & value) { m_numTxBytes = value; }
		const double get_averageRxRate() const { return m_averageRxRate;}
		void set_averageRxRate(const double & value) { m_averageRxRate = value; }
		const int64_t get_numRxVHT_1950_Mbps() const { return m_numRxVHT_1950_Mbps;}
		void set_numRxVHT_1950_Mbps(const int64_t & value) { m_numRxVHT_1950_Mbps = value; }
		const int64_t get_numRxHT_130_3_Mbps() const { return m_numRxHT_130_3_Mbps;}
		void set_numRxHT_130_3_Mbps(const int64_t & value) { m_numRxHT_130_3_Mbps = value; }
		const int64_t get_numRxDataFrames() const { return m_numRxDataFrames;}
		void set_numRxDataFrames(const int64_t & value) { m_numRxDataFrames = value; }
		const int64_t get_numRxVHT_780_Mbps() const { return m_numRxVHT_780_Mbps;}
		void set_numRxVHT_780_Mbps(const int64_t & value) { m_numRxVHT_780_Mbps = value; }
		const int64_t get_numRxHT_54_Mbps() const { return m_numRxHT_54_Mbps;}
		void set_numRxHT_54_Mbps(const int64_t & value) { m_numRxHT_54_Mbps = value; }
		const int64_t get_numRxVHT_600_Mbps() const { return m_numRxVHT_600_Mbps;}
		void set_numRxVHT_600_Mbps(const int64_t & value) { m_numRxVHT_600_Mbps = value; }
		const int64_t get_numRxDataFrames_300_Mbps() const { return m_numRxDataFrames_300_Mbps;}
		void set_numRxDataFrames_300_Mbps(const int64_t & value) { m_numRxDataFrames_300_Mbps = value; }
		const int64_t get_numRxHT_243_Mbps() const { return m_numRxHT_243_Mbps;}
		void set_numRxHT_243_Mbps(const int64_t & value) { m_numRxHT_243_Mbps = value; }
		const int64_t get_numRxHT_57_7_Mbps() const { return m_numRxHT_57_7_Mbps;}
		void set_numRxHT_57_7_Mbps(const int64_t & value) { m_numRxHT_57_7_Mbps = value; }
		const int64_t get_numRxHT_234_Mbps() const { return m_numRxHT_234_Mbps;}
		void set_numRxHT_234_Mbps(const int64_t & value) { m_numRxHT_234_Mbps = value; }
		const int64_t get_numRxDataFrames_12_Mbps() const { return m_numRxDataFrames_12_Mbps;}
		void set_numRxDataFrames_12_Mbps(const int64_t & value) { m_numRxDataFrames_12_Mbps = value; }
		const int64_t get_numTxVHT_2106_Mbps() const { return m_numTxVHT_2106_Mbps;}
		void set_numTxVHT_2106_Mbps(const int64_t & value) { m_numTxVHT_2106_Mbps = value; }
		const int64_t get_rxDuplicatePackets() const { return m_rxDuplicatePackets;}
		void set_rxDuplicatePackets(const int64_t & value) { m_rxDuplicatePackets = value; }
		const int64_t get_numTxTimeFramesTransmitted() const { return m_numTxTimeFramesTransmitted;}
		void set_numTxTimeFramesTransmitted(const int64_t & value) { m_numTxTimeFramesTransmitted = value; }
		const int64_t get_numRxVHT_3466_8_Mbps() const { return m_numRxVHT_3466_8_Mbps;}
		void set_numRxVHT_3466_8_Mbps(const int64_t & value) { m_numRxVHT_3466_8_Mbps = value; }
		const int64_t get_numRxHT_52_Mbps() const { return m_numRxHT_52_Mbps;}
		void set_numRxHT_52_Mbps(const int64_t & value) { m_numRxHT_52_Mbps = value; }
		const int64_t get_numTxControl() const { return m_numTxControl;}
		void set_numTxControl(const int64_t & value) { m_numTxControl = value; }
		const int64_t get_numTx_36_Mbps() const { return m_numTx_36_Mbps;}
		void set_numTx_36_Mbps(const int64_t & value) { m_numTx_36_Mbps = value; }
		const int64_t get_numRxHT_32_5_Mbps() const { return m_numRxHT_32_5_Mbps;}
		void set_numRxHT_32_5_Mbps(const int64_t & value) { m_numRxHT_32_5_Mbps = value; }
		const int64_t get_numTxHT_15_Mbps() const { return m_numTxHT_15_Mbps;}
		void set_numTxHT_15_Mbps(const int64_t & value) { m_numTxHT_15_Mbps = value; }
		const int64_t get_numRxDataFrames_1300_Mbps() const { return m_numRxDataFrames_1300_Mbps;}
		void set_numRxDataFrames_1300_Mbps(const int64_t & value) { m_numRxDataFrames_1300_Mbps = value; }
		const int64_t get_numRxVHT_450_Mbps() const { return m_numRxVHT_450_Mbps;}
		void set_numRxVHT_450_Mbps(const int64_t & value) { m_numRxVHT_450_Mbps = value; }
		const int64_t get_numTxDataFrames_12_Mbps() const { return m_numTxDataFrames_12_Mbps;}
		void set_numTxDataFrames_12_Mbps(const int64_t & value) { m_numTxDataFrames_12_Mbps = value; }
		const int64_t get_numTxVHT_468_Mbps() const { return m_numTxVHT_468_Mbps;}
		void set_numTxVHT_468_Mbps(const int64_t & value) { m_numTxVHT_468_Mbps = value; }
		const int64_t get_numRxVHT_800_Mbps() const { return m_numRxVHT_800_Mbps;}
		void set_numRxVHT_800_Mbps(const int64_t & value) { m_numRxVHT_800_Mbps = value; }
		const int64_t get_numTxAggrSucc() const { return m_numTxAggrSucc;}
		void set_numTxAggrSucc(const int64_t & value) { m_numTxAggrSucc = value; }
		const int64_t get_numTxHT_243_Mbps() const { return m_numTxHT_243_Mbps;}
		void set_numTxHT_243_Mbps(const int64_t & value) { m_numTxHT_243_Mbps = value; }
		const int64_t get_numTxVHT_1053_Mbps() const { return m_numTxVHT_1053_Mbps;}
		void set_numTxVHT_1053_Mbps(const int64_t & value) { m_numTxVHT_1053_Mbps = value; }
		const int64_t get_numTxHT_90_Mbps() const { return m_numTxHT_90_Mbps;}
		void set_numTxHT_90_Mbps(const int64_t & value) { m_numTxHT_90_Mbps = value; }
		const int64_t get_numRxVHT_877_5_Mbps() const { return m_numRxVHT_877_5_Mbps;}
		void set_numRxVHT_877_5_Mbps(const int64_t & value) { m_numRxVHT_877_5_Mbps = value; }
		const int64_t get_numTxHT_19_5_Mbps() const { return m_numTxHT_19_5_Mbps;}
		void set_numTxHT_19_5_Mbps(const int64_t & value) { m_numTxHT_19_5_Mbps = value; }
		const WmmQueueStatsPerQueueTypeMap get_wmmQueueStats() const { return m_wmmQueueStats;}
		void set_wmmQueueStats(const WmmQueueStatsPerQueueTypeMap & value) { m_wmmQueueStats = value; }
		const int64_t get_numTxQueued() const { return m_numTxQueued;}
		void set_numTxQueued(const int64_t & value) { m_numTxQueued = value; }
		const int64_t get_rxLastRssi() const { return m_rxLastRssi;}
		void set_rxLastRssi(const int64_t & value) { m_rxLastRssi = value; }
		const int64_t get_numRxHT_208_Mbps() const { return m_numRxHT_208_Mbps;}
		void set_numRxHT_208_Mbps(const int64_t & value) { m_numRxHT_208_Mbps = value; }
		const int64_t get_numTxRetryDropped() const { return m_numTxRetryDropped;}
		void set_numTxRetryDropped(const int64_t & value) { m_numTxRetryDropped = value; }
		const std::vector<McsStats> get_List_McsStats__mcsStats() const { return m_List_McsStats__mcsStats;}
		void set_List_McsStats__mcsStats(const std::vector<McsStats> & value) { m_List_McsStats__mcsStats = value; }
		const int64_t get_numRxHT_351_Mbps() const { return m_numRxHT_351_Mbps;}
		void set_numRxHT_351_Mbps(const int64_t & value) { m_numRxHT_351_Mbps = value; }
		const int64_t get_numTxData() const { return m_numTxData;}
		void set_numTxData(const int64_t & value) { m_numTxData = value; }
		const int64_t get_numRxVHT_1404_Mbps() const { return m_numRxVHT_1404_Mbps;}
		void set_numRxVHT_1404_Mbps(const int64_t & value) { m_numRxVHT_1404_Mbps = value; }
		const int64_t get_numRxVHT_1560_Mbps() const { return m_numRxVHT_1560_Mbps;}
		void set_numRxVHT_1560_Mbps(const int64_t & value) { m_numRxVHT_1560_Mbps = value; }
		const int64_t get_lastSentLayer3Ts() const { return m_lastSentLayer3Ts;}
		void set_lastSentLayer3Ts(const int64_t & value) { m_lastSentLayer3Ts = value; }
		const int64_t get_numRxHT_43_2_Mbps() const { return m_numRxHT_43_2_Mbps;}
		void set_numRxHT_43_2_Mbps(const int64_t & value) { m_numRxHT_43_2_Mbps = value; }
		const int64_t get_numRxHT_29_2_Mbps() const { return m_numRxHT_29_2_Mbps;}
		void set_numRxHT_29_2_Mbps(const int64_t & value) { m_numRxHT_29_2_Mbps = value; }
		const int64_t get_numRxHT_13_5_Mbps() const { return m_numRxHT_13_5_Mbps;}
		void set_numRxHT_13_5_Mbps(const int64_t & value) { m_numRxHT_13_5_Mbps = value; }
		const int64_t get_numTxVHT_1053_1_Mbps() const { return m_numTxVHT_1053_1_Mbps;}
		void set_numTxVHT_1053_1_Mbps(const int64_t & value) { m_numTxVHT_1053_1_Mbps = value; }
		const int64_t get_numTxHT_65_Mbps() const { return m_numTxHT_65_Mbps;}
		void set_numTxHT_65_Mbps(const int64_t & value) { m_numTxHT_65_Mbps = value; }
		const int64_t get_numTxVHT_3120_Mbps() const { return m_numTxVHT_3120_Mbps;}
		void set_numTxVHT_3120_Mbps(const int64_t & value) { m_numTxVHT_3120_Mbps = value; }
		const int64_t get_numRxHT_87_8_Mbps() const { return m_numRxHT_87_8_Mbps;}
		void set_numRxHT_87_8_Mbps(const int64_t & value) { m_numRxHT_87_8_Mbps = value; }
		const int64_t get_numRxHT_312_Mbps() const { return m_numRxHT_312_Mbps;}
		void set_numRxHT_312_Mbps(const int64_t & value) { m_numRxHT_312_Mbps = value; }
		const int64_t get_secondsSinceLastRecv() const { return m_secondsSinceLastRecv;}
		void set_secondsSinceLastRecv(const int64_t & value) { m_secondsSinceLastRecv = value; }
		const int64_t get_numTxRtsSucc() const { return m_numTxRtsSucc;}
		void set_numTxRtsSucc(const int64_t & value) { m_numTxRtsSucc = value; }
		const int64_t get_numTxHT_325_Mbps() const { return m_numTxHT_325_Mbps;}
		void set_numTxHT_325_Mbps(const int64_t & value) { m_numTxHT_325_Mbps = value; }
		const int64_t get_rxBytes() const { return m_rxBytes;}
		void set_rxBytes(const int64_t & value) { m_rxBytes = value; }
		const int64_t get_numTxSucc() const { return m_numTxSucc;}
		void set_numTxSucc(const int64_t & value) { m_numTxSucc = value; }
		const int64_t get_numTxTimeData() const { return m_numTxTimeData;}
		void set_numTxTimeData(const int64_t & value) { m_numTxTimeData = value; }
		const int64_t get_numRxHT_97_5_Mbps() const { return m_numRxHT_97_5_Mbps;}
		void set_numRxHT_97_5_Mbps(const int64_t & value) { m_numRxHT_97_5_Mbps = value; }
		const int64_t get_numTx_48_Mbps() const { return m_numTx_48_Mbps;}
		void set_numTx_48_Mbps(const int64_t & value) { m_numTx_48_Mbps = value; }
		const int64_t get_numTxVHT_540_Mbps() const { return m_numTxVHT_540_Mbps;}
		void set_numTxVHT_540_Mbps(const int64_t & value) { m_numTxVHT_540_Mbps = value; }
		const int64_t get_numTxAggrOneMpdu() const { return m_numTxAggrOneMpdu;}
		void set_numTxAggrOneMpdu(const int64_t & value) { m_numTxAggrOneMpdu = value; }
		const int64_t get_numTxHT_360_Mbps() const { return m_numTxHT_360_Mbps;}
		void set_numTxHT_360_Mbps(const int64_t & value) { m_numTxHT_360_Mbps = value; }
		const int64_t get_numTxNoAck() const { return m_numTxNoAck;}
		void set_numTxNoAck(const int64_t & value) { m_numTxNoAck = value; }
		const int64_t get_numRxHT_173_1_Mbps() const { return m_numRxHT_173_1_Mbps;}
		void set_numRxHT_173_1_Mbps(const int64_t & value) { m_numRxHT_173_1_Mbps = value; }
		const int64_t get_numTxVHT_1733_1_Mbps() const { return m_numTxVHT_1733_1_Mbps;}
		void set_numTxVHT_1733_1_Mbps(const int64_t & value) { m_numTxVHT_1733_1_Mbps = value; }
		const int64_t get_numTxHT_104_Mbps() const { return m_numTxHT_104_Mbps;}
		void set_numTxHT_104_Mbps(const int64_t & value) { m_numTxHT_104_Mbps = value; }
		const int64_t get_numRxHT_27_Mbps() const { return m_numRxHT_27_Mbps;}
		void set_numRxHT_27_Mbps(const int64_t & value) { m_numRxHT_27_Mbps = value; }
		const int64_t get_numTxVHT_325_Mbps() const { return m_numTxVHT_325_Mbps;}
		void set_numTxVHT_325_Mbps(const int64_t & value) { m_numTxVHT_325_Mbps = value; }
		const McsType get_lastRxMcsIdx() const { return m_lastRxMcsIdx;}
		void set_lastRxMcsIdx(const McsType & value) { m_lastRxMcsIdx = value; }
		const int64_t get_numRxHT_30_Mbps() const { return m_numRxHT_30_Mbps;}
		void set_numRxHT_30_Mbps(const int64_t & value) { m_numRxHT_30_Mbps = value; }
		const int64_t get_numRxNoFcsErr() const { return m_numRxNoFcsErr;}
		void set_numRxNoFcsErr(const int64_t & value) { m_numRxNoFcsErr = value; }
		const int64_t get_numTxDataTransmittedRetried() const { return m_numTxDataTransmittedRetried;}
		void set_numTxDataTransmittedRetried(const int64_t & value) { m_numTxDataTransmittedRetried = value; }
		const int64_t get_numTxHT_28_8_Mbps() const { return m_numTxHT_28_8_Mbps;}
		void set_numTxHT_28_8_Mbps(const int64_t & value) { m_numTxHT_28_8_Mbps = value; }
		const int64_t get_numRxHT_135_Mbps() const { return m_numRxHT_135_Mbps;}
		void set_numRxHT_135_Mbps(const int64_t & value) { m_numRxHT_135_Mbps = value; }
		const int64_t get_numTxVHT_650_Mbps() const { return m_numTxVHT_650_Mbps;}
		void set_numTxVHT_650_Mbps(const int64_t & value) { m_numTxVHT_650_Mbps = value; }
		const int64_t get_numRxVHT_1300_Mbps() const { return m_numRxVHT_1300_Mbps;}
		void set_numRxVHT_1300_Mbps(const int64_t & value) { m_numRxVHT_1300_Mbps = value; }
		const int64_t get_lastRecvLayer3Ts() const { return m_lastRecvLayer3Ts;}
		void set_lastRecvLayer3Ts(const int64_t & value) { m_lastRecvLayer3Ts = value; }
		const int64_t get_numTxHT_346_7_Mbps() const { return m_numTxHT_346_7_Mbps;}
		void set_numTxHT_346_7_Mbps(const int64_t & value) { m_numTxHT_346_7_Mbps = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const int64_t get_numTxVHT_1404_Mbps() const { return m_numTxVHT_1404_Mbps;}
		void set_numTxVHT_1404_Mbps(const int64_t & value) { m_numTxVHT_1404_Mbps = value; }
		const int64_t get_numRxHT_81_Mbps() const { return m_numRxHT_81_Mbps;}
		void set_numRxHT_81_Mbps(const int64_t & value) { m_numRxHT_81_Mbps = value; }
		const int64_t get_numRxHT_78_Mbps() const { return m_numRxHT_78_Mbps;}
		void set_numRxHT_78_Mbps(const int64_t & value) { m_numRxHT_78_Mbps = value; }
		const int64_t get_numTxHT_57_5_Mbps() const { return m_numTxHT_57_5_Mbps;}
		void set_numTxHT_57_5_Mbps(const int64_t & value) { m_numTxHT_57_5_Mbps = value; }
		const int64_t get_numTxVHT_400_Mbps() const { return m_numTxVHT_400_Mbps;}
		void set_numTxVHT_400_Mbps(const int64_t & value) { m_numTxVHT_400_Mbps = value; }
		const int64_t get_numRxVHT_2600_Mbps() const { return m_numRxVHT_2600_Mbps;}
		void set_numRxVHT_2600_Mbps(const int64_t & value) { m_numRxVHT_2600_Mbps = value; }
		const int64_t get_numRxVHT_1170_Mbps() const { return m_numRxVHT_1170_Mbps;}
		void set_numRxVHT_1170_Mbps(const int64_t & value) { m_numRxVHT_1170_Mbps = value; }
		const int64_t get_numTxHT_117_1_Mbps() const { return m_numTxHT_117_1_Mbps;}
		void set_numTxHT_117_1_Mbps(const int64_t & value) { m_numTxHT_117_1_Mbps = value; }
		const int64_t get_numRx_12_Mbps() const { return m_numRx_12_Mbps;}
		void set_numRx_12_Mbps(const int64_t & value) { m_numRx_12_Mbps = value; }
		const int64_t get_numTxPropResp() const { return m_numTxPropResp;}
		void set_numTxPropResp(const int64_t & value) { m_numTxPropResp = value; }
		const int64_t get_numTxDataFrames_300_Mbps() const { return m_numTxDataFrames_300_Mbps;}
		void set_numTxDataFrames_300_Mbps(const int64_t & value) { m_numTxDataFrames_300_Mbps = value; }
		const int64_t get_numTxHT_180_Mbps() const { return m_numTxHT_180_Mbps;}
		void set_numTxHT_180_Mbps(const int64_t & value) { m_numTxHT_180_Mbps = value; }
		const int64_t get_numTxHT_292_5_Mbps() const { return m_numTxHT_292_5_Mbps;}
		void set_numTxHT_292_5_Mbps(const int64_t & value) { m_numTxHT_292_5_Mbps = value; }
		const int64_t get_numRxHT_162_Mbps() const { return m_numRxHT_162_Mbps;}
		void set_numRxHT_162_Mbps(const int64_t & value) { m_numRxHT_162_Mbps = value; }
		const int64_t get_numTxVHT_292_5_Mbps() const { return m_numTxVHT_292_5_Mbps;}
		void set_numTxVHT_292_5_Mbps(const int64_t & value) { m_numTxVHT_292_5_Mbps = value; }
		const int64_t get_numTxHT_27_Mbps() const { return m_numTxHT_27_Mbps;}
		void set_numTxHT_27_Mbps(const int64_t & value) { m_numTxHT_27_Mbps = value; }
		const int64_t get_numRxVHT_364_5_Mbps() const { return m_numRxVHT_364_5_Mbps;}
		void set_numRxVHT_364_5_Mbps(const int64_t & value) { m_numRxVHT_364_5_Mbps = value; }
		const int64_t get_numTxVHT_2600_Mbps() const { return m_numTxVHT_2600_Mbps;}
		void set_numTxVHT_2600_Mbps(const int64_t & value) { m_numTxVHT_2600_Mbps = value; }
		const int64_t get_numTxHT_97_5_Mbps() const { return m_numTxHT_97_5_Mbps;}
		void set_numTxHT_97_5_Mbps(const int64_t & value) { m_numTxHT_97_5_Mbps = value; }
		const int64_t get_numTxSuccRetries() const { return m_numTxSuccRetries;}
		void set_numTxSuccRetries(const int64_t & value) { m_numTxSuccRetries = value; }
		const int64_t get_numTxVHT_780_Mbps() const { return m_numTxVHT_780_Mbps;}
		void set_numTxVHT_780_Mbps(const int64_t & value) { m_numTxVHT_780_Mbps = value; }
		const int64_t get_numRxHT_260_Mbps() const { return m_numRxHT_260_Mbps;}
		void set_numRxHT_260_Mbps(const int64_t & value) { m_numRxHT_260_Mbps = value; }
		const int64_t get_numRxVHT_866_7_Mbps() const { return m_numRxVHT_866_7_Mbps;}
		void set_numRxVHT_866_7_Mbps(const int64_t & value) { m_numRxVHT_866_7_Mbps = value; }
		const int64_t get_numTxVHT_1579_5_Mbps() const { return m_numTxVHT_1579_5_Mbps;}
		void set_numTxVHT_1579_5_Mbps(const int64_t & value) { m_numTxVHT_1579_5_Mbps = value; }
		const int64_t get_numRxHT_45_Mbps() const { return m_numRxHT_45_Mbps;}
		void set_numRxHT_45_Mbps(const int64_t & value) { m_numRxHT_45_Mbps = value; }
		const int64_t get_numTxHT_175_5_Mbps() const { return m_numTxHT_175_5_Mbps;}
		void set_numTxHT_175_5_Mbps(const int64_t & value) { m_numTxHT_175_5_Mbps = value; }
		const int64_t get_numRxControl() const { return m_numRxControl;}
		void set_numRxControl(const int64_t & value) { m_numRxControl = value; }
		const int64_t get_numRxHT_263_2_Mbps() const { return m_numRxHT_263_2_Mbps;}
		void set_numRxHT_263_2_Mbps(const int64_t & value) { m_numRxHT_263_2_Mbps = value; }
		const int64_t get_numRxVHT_1733_4_Mbps() const { return m_numRxVHT_1733_4_Mbps;}
		void set_numRxVHT_1733_4_Mbps(const int64_t & value) { m_numRxVHT_1733_4_Mbps = value; }
		const int64_t get_numTxHT_60_Mbps() const { return m_numTxHT_60_Mbps;}
		void set_numTxHT_60_Mbps(const int64_t & value) { m_numTxHT_60_Mbps = value; }
		const int64_t get_numTxVHT_405_Mbps() const { return m_numTxVHT_405_Mbps;}
		void set_numTxVHT_405_Mbps(const int64_t & value) { m_numTxVHT_405_Mbps = value; }
		const int64_t get_numTxHT_200_Mbps() const { return m_numTxHT_200_Mbps;}
		void set_numTxHT_200_Mbps(const int64_t & value) { m_numTxHT_200_Mbps = value; }
		const int64_t get_numTxVHT_1560_Mbps() const { return m_numTxVHT_1560_Mbps;}
		void set_numTxVHT_1560_Mbps(const int64_t & value) { m_numTxVHT_1560_Mbps = value; }
		const int64_t get_periodLengthSec() const { return m_periodLengthSec;}
		void set_periodLengthSec(const int64_t & value) { m_periodLengthSec = value; }
		const int64_t get_numRxVHT_390_Mbps() const { return m_numRxVHT_390_Mbps;}
		void set_numRxVHT_390_Mbps(const int64_t & value) { m_numRxVHT_390_Mbps = value; }
		const int64_t get_numTxVHT_1950_Mbps() const { return m_numTxVHT_1950_Mbps;}
		void set_numTxVHT_1950_Mbps(const int64_t & value) { m_numTxVHT_1950_Mbps = value; }
		const int64_t get_numRxHT_86_8_Mbps() const { return m_numRxHT_86_8_Mbps;}
		void set_numRxHT_86_8_Mbps(const int64_t & value) { m_numRxHT_86_8_Mbps = value; }
		const int64_t get_numTxVHT_1872_Mbps() const { return m_numTxVHT_1872_Mbps;}
		void set_numTxVHT_1872_Mbps(const int64_t & value) { m_numTxVHT_1872_Mbps = value; }
		const int64_t get_numRxHT_57_5_Mbps() const { return m_numRxHT_57_5_Mbps;}
		void set_numRxHT_57_5_Mbps(const int64_t & value) { m_numRxHT_57_5_Mbps = value; }
		const int64_t get_numTxHT_300_Mbps() const { return m_numTxHT_300_Mbps;}
		void set_numTxHT_300_Mbps(const int64_t & value) { m_numTxHT_300_Mbps = value; }
		const int64_t get_numTxHT_231_1_Mbps() const { return m_numTxHT_231_1_Mbps;}
		void set_numTxHT_231_1_Mbps(const int64_t & value) { m_numTxHT_231_1_Mbps = value; }
		const int64_t get_numRxDataFrames_54_Mbps() const { return m_numRxDataFrames_54_Mbps;}
		void set_numRxDataFrames_54_Mbps(const int64_t & value) { m_numRxDataFrames_54_Mbps = value; }
		const int64_t get_numRxHT_6_5_Mbps() const { return m_numRxHT_6_5_Mbps;}
		void set_numRxHT_6_5_Mbps(const int64_t & value) { m_numRxHT_6_5_Mbps = value; }
		const int64_t get_numTx_54_Mbps() const { return m_numTx_54_Mbps;}
		void set_numTx_54_Mbps(const int64_t & value) { m_numTx_54_Mbps = value; }
		const int64_t get_numTxHT_173_1_Mbps() const { return m_numTxHT_173_1_Mbps;}
		void set_numTxHT_173_1_Mbps(const int64_t & value) { m_numTxHT_173_1_Mbps = value; }
		const int64_t get_numTxFramesTransmitted() const { return m_numTxFramesTransmitted;}
		void set_numTxFramesTransmitted(const int64_t & value) { m_numTxFramesTransmitted = value; }
		const int64_t get_numTxVHT_975_Mbps() const { return m_numTxVHT_975_Mbps;}
		void set_numTxVHT_975_Mbps(const int64_t & value) { m_numTxVHT_975_Mbps = value; }
		const int64_t get_numRxHT_58_5_Mbps() const { return m_numRxHT_58_5_Mbps;}
		void set_numRxHT_58_5_Mbps(const int64_t & value) { m_numRxHT_58_5_Mbps = value; }
		const int64_t get_numRxHT_173_3_Mbps() const { return m_numRxHT_173_3_Mbps;}
		void set_numRxHT_173_3_Mbps(const int64_t & value) { m_numRxHT_173_3_Mbps = value; }
		const int64_t get_numRx_36_Mbps() const { return m_numRx_36_Mbps;}
		void set_numRx_36_Mbps(const int64_t & value) { m_numRx_36_Mbps = value; }
		const int64_t get_numTxHT_7_1_Mbps() const { return m_numTxHT_7_1_Mbps;}
		void set_numTxHT_7_1_Mbps(const int64_t & value) { m_numTxHT_7_1_Mbps = value; }
		const int64_t get_numRxHT_346_7_Mbps() const { return m_numRxHT_346_7_Mbps;}
		void set_numRxHT_346_7_Mbps(const int64_t & value) { m_numRxHT_346_7_Mbps = value; }
		const int64_t get_numRxVHT_520_Mbps() const { return m_numRxVHT_520_Mbps;}
		void set_numRxVHT_520_Mbps(const int64_t & value) { m_numRxVHT_520_Mbps = value; }
		const int64_t get_numRxVHT_1053_1_Mbps() const { return m_numRxVHT_1053_1_Mbps;}
		void set_numRxVHT_1053_1_Mbps(const int64_t & value) { m_numRxVHT_1053_1_Mbps = value; }
		const int64_t get_numTxDataFrames_54_Mbps() const { return m_numTxDataFrames_54_Mbps;}
		void set_numTxDataFrames_54_Mbps(const int64_t & value) { m_numTxDataFrames_54_Mbps = value; }
		const int64_t get_numTxHT_208_Mbps() const { return m_numTxHT_208_Mbps;}
		void set_numTxHT_208_Mbps(const int64_t & value) { m_numTxHT_208_Mbps = value; }
		const int64_t get_numRxVHT_720_Mbps() const { return m_numRxVHT_720_Mbps;}
		void set_numRxVHT_720_Mbps(const int64_t & value) { m_numRxVHT_720_Mbps = value; }
		const int64_t get_numTxHT_26_Mbps() const { return m_numTxHT_26_Mbps;}
		void set_numTxHT_26_Mbps(const int64_t & value) { m_numTxHT_26_Mbps = value; }
		const int64_t get_numTxVHT_480_Mbps() const { return m_numTxVHT_480_Mbps;}
		void set_numTxVHT_480_Mbps(const int64_t & value) { m_numTxVHT_480_Mbps = value; }
		const int64_t get_numTxVHT_2080_Mbps() const { return m_numTxVHT_2080_Mbps;}
		void set_numTxVHT_2080_Mbps(const int64_t & value) { m_numTxVHT_2080_Mbps = value; }
		const int64_t get_numTxHT_351_Mbps() const { return m_numTxHT_351_Mbps;}
		void set_numTxHT_351_Mbps(const int64_t & value) { m_numTxHT_351_Mbps = value; }
		const int64_t get_numRxRts() const { return m_numRxRts;}
		void set_numRxRts(const int64_t & value) { m_numRxRts = value; }
		const int64_t get_numTxRtsFail() const { return m_numTxRtsFail;}
		void set_numTxRtsFail(const int64_t & value) { m_numTxRtsFail = value; }
		const int64_t get_numRxHT_15_Mbps() const { return m_numRxHT_15_Mbps;}
		void set_numRxHT_15_Mbps(const int64_t & value) { m_numRxHT_15_Mbps = value; }
		const int64_t get_numRxHT_175_5_Mbps() const { return m_numRxHT_175_5_Mbps;}
		void set_numRxHT_175_5_Mbps(const int64_t & value) { m_numRxHT_175_5_Mbps = value; }
		const int64_t get_numTxHT_288_7_Mbps() const { return m_numTxHT_288_7_Mbps;}
		void set_numTxHT_288_7_Mbps(const int64_t & value) { m_numTxHT_288_7_Mbps = value; }
		const int64_t get_numTxHT_29_2_Mbps() const { return m_numTxHT_29_2_Mbps;}
		void set_numTxHT_29_2_Mbps(const int64_t & value) { m_numTxHT_29_2_Mbps = value; }
		const int64_t get_numRxHT_144_3_Mbps() const { return m_numRxHT_144_3_Mbps;}
		void set_numRxHT_144_3_Mbps(const int64_t & value) { m_numRxHT_144_3_Mbps = value; }
		const int64_t get_numRxVHT_1579_5_Mbps() const { return m_numRxVHT_1579_5_Mbps;}
		void set_numRxVHT_1579_5_Mbps(const int64_t & value) { m_numRxVHT_1579_5_Mbps = value; }
		const int64_t get_numRxDataFrames_1300Plus_Mbps() const { return m_numRxDataFrames_1300Plus_Mbps;}
		void set_numRxDataFrames_1300Plus_Mbps(const int64_t & value) { m_numRxDataFrames_1300Plus_Mbps = value; }
		const int64_t get_numTxHT_57_7_Mbps() const { return m_numTxHT_57_7_Mbps;}
		void set_numTxHT_57_7_Mbps(const int64_t & value) { m_numTxHT_57_7_Mbps = value; }
		const int64_t get_numRxHT_86_6_Mbps() const { return m_numRxHT_86_6_Mbps;}
		void set_numRxHT_86_6_Mbps(const int64_t & value) { m_numRxHT_86_6_Mbps = value; }
		const int64_t get_numTx_6_Mbps() const { return m_numTx_6_Mbps;}
		void set_numTx_6_Mbps(const int64_t & value) { m_numTx_6_Mbps = value; }
		const int64_t get_numTxHT_45_Mbps() const { return m_numTxHT_45_Mbps;}
		void set_numTxHT_45_Mbps(const int64_t & value) { m_numTxHT_45_Mbps = value; }
		const int64_t get_numTxHT_216_Mbps() const { return m_numTxHT_216_Mbps;}
		void set_numTxHT_216_Mbps(const int64_t & value) { m_numTxHT_216_Mbps = value; }
		const int64_t get_numTxVHT_800_Mbps() const { return m_numTxVHT_800_Mbps;}
		void set_numTxVHT_800_Mbps(const int64_t & value) { m_numTxVHT_800_Mbps = value; }
		const int64_t get_numTxVHT_3466_8_Mbps() const { return m_numTxVHT_3466_8_Mbps;}
		void set_numTxVHT_3466_8_Mbps(const int64_t & value) { m_numTxVHT_3466_8_Mbps = value; }
		const int64_t get_numTxHT_260_Mbps() const { return m_numTxHT_260_Mbps;}
		void set_numTxHT_260_Mbps(const int64_t & value) { m_numTxHT_260_Mbps = value; }
		const int64_t get_numRxHT_108_Mbps() const { return m_numRxHT_108_Mbps;}
		void set_numRxHT_108_Mbps(const int64_t & value) { m_numRxHT_108_Mbps = value; }
		const int64_t get_numTxHT_40_5_Mbps() const { return m_numTxHT_40_5_Mbps;}
		void set_numTxHT_40_5_Mbps(const int64_t & value) { m_numTxHT_40_5_Mbps = value; }
		const int64_t get_numTxHT_234_Mbps() const { return m_numTxHT_234_Mbps;}
		void set_numTxHT_234_Mbps(const int64_t & value) { m_numTxHT_234_Mbps = value; }
		const int64_t get_numTx_1_Mbps() const { return m_numTx_1_Mbps;}
		void set_numTx_1_Mbps(const int64_t & value) { m_numTx_1_Mbps = value; }
		const int64_t get_numRxLdpc() const { return m_numRxLdpc;}
		void set_numRxLdpc(const int64_t & value) { m_numRxLdpc = value; }
		const int64_t get_numTxSuccNoRetry() const { return m_numTxSuccNoRetry;}
		void set_numTxSuccNoRetry(const int64_t & value) { m_numTxSuccNoRetry = value; }
		const int64_t get_numRxHT_21_7_Mbps() const { return m_numRxHT_21_7_Mbps;}
		void set_numRxHT_21_7_Mbps(const int64_t & value) { m_numRxHT_21_7_Mbps = value; }
		const int64_t get_numTxHT_162_Mbps() const { return m_numTxHT_162_Mbps;}
		void set_numTxHT_162_Mbps(const int64_t & value) { m_numTxHT_162_Mbps = value; }
		const int64_t get_numTxHT_195_Mbps() const { return m_numTxHT_195_Mbps;}
		void set_numTxHT_195_Mbps(const int64_t & value) { m_numTxHT_195_Mbps = value; }
		const int64_t get_numRx_6_Mbps() const { return m_numRx_6_Mbps;}
		void set_numRx_6_Mbps(const int64_t & value) { m_numRx_6_Mbps = value; }
		const int64_t get_numTxHT_21_7_Mbps() const { return m_numTxHT_21_7_Mbps;}
		void set_numTxHT_21_7_Mbps(const int64_t & value) { m_numTxHT_21_7_Mbps = value; }
		const int64_t get_numTxHT_130_3_Mbps() const { return m_numTxHT_130_3_Mbps;}
		void set_numTxHT_130_3_Mbps(const int64_t & value) { m_numTxHT_130_3_Mbps = value; }
		const int64_t get_numTxDataFrames_108_Mbps() const { return m_numTxDataFrames_108_Mbps;}
		void set_numTxDataFrames_108_Mbps(const int64_t & value) { m_numTxDataFrames_108_Mbps = value; }
		const int64_t get_numRxHT_90_Mbps() const { return m_numRxHT_90_Mbps;}
		void set_numRxHT_90_Mbps(const int64_t & value) { m_numRxHT_90_Mbps = value; }
		const GuardInterval get_guardInterval() const { return m_guardInterval;}
		void set_guardInterval(const GuardInterval & value) { m_guardInterval = value; }
		const int64_t get_numRxVHT_486_Mbps() const { return m_numRxVHT_486_Mbps;}
		void set_numRxVHT_486_Mbps(const int64_t & value) { m_numRxVHT_486_Mbps = value; }
		const int64_t get_numTxVHT_877_5_Mbps() const { return m_numTxVHT_877_5_Mbps;}
		void set_numTxVHT_877_5_Mbps(const int64_t & value) { m_numTxVHT_877_5_Mbps = value; }
		const int64_t get_numRxVHT_540_Mbps() const { return m_numRxVHT_540_Mbps;}
		void set_numRxVHT_540_Mbps(const int64_t & value) { m_numRxVHT_540_Mbps = value; }
		const int64_t get_numRxVHT_526_5_Mbps() const { return m_numRxVHT_526_5_Mbps;}
		void set_numRxVHT_526_5_Mbps(const int64_t & value) { m_numRxVHT_526_5_Mbps = value; }
		const int64_t get_numTxHT_121_5_Mbps() const { return m_numTxHT_121_5_Mbps;}
		void set_numTxHT_121_5_Mbps(const int64_t & value) { m_numTxHT_121_5_Mbps = value; }
		const int64_t get_numTxHT_32_5_Mbps() const { return m_numTxHT_32_5_Mbps;}
		void set_numTxHT_32_5_Mbps(const int64_t & value) { m_numTxHT_32_5_Mbps = value; }
		const int64_t get_numTxHT_130_Mbps() const { return m_numTxHT_130_Mbps;}
		void set_numTxHT_130_Mbps(const int64_t & value) { m_numTxHT_130_Mbps = value; }
		const int64_t get_numRxHT_240_Mbps() const { return m_numRxHT_240_Mbps;}
		void set_numRxHT_240_Mbps(const int64_t & value) { m_numRxHT_240_Mbps = value; }
		const int64_t get_numTxHT_150_Mbps() const { return m_numTxHT_150_Mbps;}
		void set_numTxHT_150_Mbps(const int64_t & value) { m_numTxHT_150_Mbps = value; }
		const int64_t get_numRxRetry() const { return m_numRxRetry;}
		void set_numRxRetry(const int64_t & value) { m_numRxRetry = value; }
		const int64_t get_numTxHT_72_1_Mbps() const { return m_numTxHT_72_1_Mbps;}
		void set_numTxHT_72_1_Mbps(const int64_t & value) { m_numTxHT_72_1_Mbps = value; }
		const int64_t get_ciscoLastRate() const { return m_ciscoLastRate;}
		void set_ciscoLastRate(const int64_t & value) { m_ciscoLastRate = value; }
		const int64_t get_numRxVHT_400_Mbps() const { return m_numRxVHT_400_Mbps;}
		void set_numRxVHT_400_Mbps(const int64_t & value) { m_numRxVHT_400_Mbps = value; }
		const int64_t get_numRxVHT_2080_Mbps() const { return m_numRxVHT_2080_Mbps;}
		void set_numRxVHT_2080_Mbps(const int64_t & value) { m_numRxVHT_2080_Mbps = value; }
		const int64_t get_numTxHT_144_3_Mbps() const { return m_numTxHT_144_3_Mbps;}
		void set_numTxHT_144_3_Mbps(const int64_t & value) { m_numTxHT_144_3_Mbps = value; }
		const int64_t get_numRxVHT_403_Mbps() const { return m_numRxVHT_403_Mbps;}
		void set_numRxVHT_403_Mbps(const int64_t & value) { m_numRxVHT_403_Mbps = value; }
		const int64_t get_numRxVHT_585_Mbps() const { return m_numRxVHT_585_Mbps;}
		void set_numRxVHT_585_Mbps(const int64_t & value) { m_numRxVHT_585_Mbps = value; }
		const int64_t get_numRxHT_117_1_Mbps() const { return m_numRxHT_117_1_Mbps;}
		void set_numRxHT_117_1_Mbps(const int64_t & value) { m_numRxHT_117_1_Mbps = value; }
		const int64_t get_numRxHT_7_1_Mbps() const { return m_numRxHT_7_1_Mbps;}
		void set_numRxHT_7_1_Mbps(const int64_t & value) { m_numRxHT_7_1_Mbps = value; }
		const int64_t get_numRxCts() const { return m_numRxCts;}
		void set_numRxCts(const int64_t & value) { m_numRxCts = value; }
		const int64_t get_numRxDataFrames_450_Mbps() const { return m_numRxDataFrames_450_Mbps;}
		void set_numRxDataFrames_450_Mbps(const int64_t & value) { m_numRxDataFrames_450_Mbps = value; }
		const int64_t get_numRxHT_324_Mbps() const { return m_numRxHT_324_Mbps;}
		void set_numRxHT_324_Mbps(const int64_t & value) { m_numRxHT_324_Mbps = value; }
		const int64_t get_numTxHT_120_Mbps() const { return m_numTxHT_120_Mbps;}
		void set_numTxHT_120_Mbps(const int64_t & value) { m_numTxHT_120_Mbps = value; }
		const std::vector<int64_t> get_mcs() const { return m_mcs;}
		void set_mcs(const std::vector<int64_t> & value) { m_mcs = value; }
		const int64_t get_numTx_9_Mbps() const { return m_numTx_9_Mbps;}
		void set_numTx_9_Mbps(const int64_t & value) { m_numTx_9_Mbps = value; }
		const int64_t get_numRxVHT_3120_Mbps() const { return m_numRxVHT_3120_Mbps;}
		void set_numRxVHT_3120_Mbps(const int64_t & value) { m_numRxVHT_3120_Mbps = value; }
		const int64_t get_numRxHT_180_Mbps() const { return m_numRxHT_180_Mbps;}
		void set_numRxHT_180_Mbps(const int64_t & value) { m_numRxHT_180_Mbps = value; }
		const int64_t get_numRx_1_Mbps() const { return m_numRx_1_Mbps;}
		void set_numRx_1_Mbps(const int64_t & value) { m_numRx_1_Mbps = value; }
		const int64_t get_rateCount() const { return m_rateCount;}
		void set_rateCount(const int64_t & value) { m_rateCount = value; }
		const int64_t get_numTxHT_78_Mbps() const { return m_numTxHT_78_Mbps;}
		void set_numTxHT_78_Mbps(const int64_t & value) { m_numTxHT_78_Mbps = value; }
		const int64_t get_numTxVHT_1733_4_Mbps() const { return m_numTxVHT_1733_4_Mbps;}
		void set_numTxVHT_1733_4_Mbps(const int64_t & value) { m_numTxVHT_1733_4_Mbps = value; }
		const int64_t get_numRxHT_300_Mbps() const { return m_numRxHT_300_Mbps;}
		void set_numRxHT_300_Mbps(const int64_t & value) { m_numRxHT_300_Mbps = value; }
		const int64_t get_numRxVHT_650_Mbps() const { return m_numRxVHT_650_Mbps;}
		void set_numRxVHT_650_Mbps(const int64_t & value) { m_numRxVHT_650_Mbps = value; }
		const int64_t get_numTxStbc() const { return m_numTxStbc;}
		void set_numTxStbc(const int64_t & value) { m_numTxStbc = value; }
		const int64_t get_numTxDataFrames_1300_Mbps() const { return m_numTxDataFrames_1300_Mbps;}
		void set_numTxDataFrames_1300_Mbps(const int64_t & value) { m_numTxDataFrames_1300_Mbps = value; }
		const int64_t get_numTxMultipleRetries() const { return m_numTxMultipleRetries;}
		void set_numTxMultipleRetries(const int64_t & value) { m_numTxMultipleRetries = value; }
		const int64_t get_numTxHT_312_Mbps() const { return m_numTxHT_312_Mbps;}
		void set_numTxHT_312_Mbps(const int64_t & value) { m_numTxHT_312_Mbps = value; }
		const int64_t get_numTxDataFrames_450_Mbps() const { return m_numTxDataFrames_450_Mbps;}
		void set_numTxDataFrames_450_Mbps(const int64_t & value) { m_numTxDataFrames_450_Mbps = value; }
		const int64_t get_numRxHT_14_3_Mbps() const { return m_numRxHT_14_3_Mbps;}
		void set_numRxHT_14_3_Mbps(const int64_t & value) { m_numRxHT_14_3_Mbps = value; }
		const int64_t get_numRxHT_130_Mbps() const { return m_numRxHT_130_Mbps;}
		void set_numRxHT_130_Mbps(const int64_t & value) { m_numRxHT_130_Mbps = value; }
		const int64_t get_numRxVHT_975_Mbps() const { return m_numRxVHT_975_Mbps;}
		void set_numRxVHT_975_Mbps(const int64_t & value) { m_numRxVHT_975_Mbps = value; }
		const int64_t get_numTxHT_58_5_Mbps() const { return m_numTxHT_58_5_Mbps;}
		void set_numTxHT_58_5_Mbps(const int64_t & value) { m_numTxHT_58_5_Mbps = value; }
		const int64_t get_numRcvFrameForTx() const { return m_numRcvFrameForTx;}
		void set_numRcvFrameForTx(const int64_t & value) { m_numRcvFrameForTx = value; }
		const ChannelBandwidth get_channelBandWidth() const { return m_channelBandWidth;}
		void set_channelBandWidth(const ChannelBandwidth & value) { m_channelBandWidth = value; }
		const int64_t get_numTxVHT_520_Mbps() const { return m_numTxVHT_520_Mbps;}
		void set_numTxVHT_520_Mbps(const int64_t & value) { m_numTxVHT_520_Mbps = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const int64_t get_rssi() const { return m_rssi;}
		void set_rssi(const int64_t & value) { m_rssi = value; }
		const int64_t get_numRxManagement() const { return m_numRxManagement;}
		void set_numRxManagement(const int64_t & value) { m_numRxManagement = value; }
		const int64_t get_vhtMcs() const { return m_vhtMcs;}
		void set_vhtMcs(const int64_t & value) { m_vhtMcs = value; }
		const int64_t get_numRxVHT_936_Mbps() const { return m_numRxVHT_936_Mbps;}
		void set_numRxVHT_936_Mbps(const int64_t & value) { m_numRxVHT_936_Mbps = value; }
		const int64_t get_numRxPackets() const { return m_numRxPackets;}
		void set_numRxPackets(const int64_t & value) { m_numRxPackets = value; }
		const int64_t get_numRx_24_Mbps() const { return m_numRx_24_Mbps;}
		void set_numRx_24_Mbps(const int64_t & value) { m_numRx_24_Mbps = value; }
		const int64_t get_snr() const { return m_snr;}
		void set_snr(const int64_t & value) { m_snr = value; }
		const int64_t get_numTxHT_156_Mbps() const { return m_numTxHT_156_Mbps;}
		void set_numTxHT_156_Mbps(const int64_t & value) { m_numTxHT_156_Mbps = value; }
		const int64_t get_numRxHT_270_Mbps() const { return m_numRxHT_270_Mbps;}
		void set_numRxHT_270_Mbps(const int64_t & value) { m_numRxHT_270_Mbps = value; }
		const int64_t get_numRxHT_115_5_Mbps() const { return m_numRxHT_115_5_Mbps;}
		void set_numRxHT_115_5_Mbps(const int64_t & value) { m_numRxHT_115_5_Mbps = value; }
		const int64_t get_numRxHT_231_1_Mbps() const { return m_numRxHT_231_1_Mbps;}
		void set_numRxHT_231_1_Mbps(const int64_t & value) { m_numRxHT_231_1_Mbps = value; }
		const int64_t get_numTx_24_Mbps() const { return m_numTx_24_Mbps;}
		void set_numTx_24_Mbps(const int64_t & value) { m_numTx_24_Mbps = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_numTxHT_216_6_Mbps;
		int64_t	m_numRxHT_325_Mbps;
		int64_t	m_numRxVHT_2340_Mbps;
		int64_t	m_numRxVHT_1733_1_Mbps;
		int64_t	m_numRxVHT_702_Mbps;
		int64_t	m_numRxVHT_433_2_Mbps;
		int64_t	m_numTxHT_6_5_Mbps;
		int64_t	m_numRxTimeData;
		int64_t	m_numTxPackets;
		int64_t	m_numRxHT_351_2_Mbps;
		int64_t	m_numRxHT_216_Mbps;
		int64_t	m_numRxVHT_405_Mbps;
		int64_t	m_numRxHT_360_Mbps;
		int64_t	m_numRxHT_120_Mbps;
		int64_t	m_numTxVHT_702_Mbps;
		int64_t	m_numTxVHT_936_Mbps;
		int64_t	m_numTxVHT_390_Mbps;
		int64_t	m_numTxDataFrames_1300Plus_Mbps;
		int64_t	m_numRxHT_150_Mbps;
		int64_t	m_numRxBytes;
		int64_t	m_numTx_18_Mbps;
		int64_t	m_numTxVHT_1755_Mbps;
		int64_t	m_numTxVHT_364_5_Mbps;
		int64_t	m_numRxNullData; /* The number of received null data frames. */
		int64_t	m_numTxHT_43_2_Mbps;
		McsType	m_lastTxMcsIdx;
		int64_t	m_rxDataBytes; /* The number of received data bytes. */
		int64_t	m_numRxHT_39_Mbps;
		int64_t	m_numTxHT_81_Mbps;
		int64_t	m_numTxHT_39_Mbps;
		int64_t	m_numRxHT_104_Mbps;
		int64_t	m_numRxVHT_432_Mbps;
		int64_t	m_numTxVHT_486_Mbps;
		int64_t	m_numRxHT_28_8_Mbps;
		int64_t	m_numTxDropped; /* The number of every TX frame dropped. */
		int64_t	m_numTxVHT_600_Mbps;
		int64_t	m_numRxVHT_648_Mbps;
		int64_t	m_numTxVHT_720_Mbps;
		int64_t	m_numRxFramesReceived;
		int64_t	m_numRxHT_121_5_Mbps;
		int64_t	m_numTxVHT_403_Mbps;
		int64_t	m_numRx_48_Mbps;
		int64_t	m_numTxAction; /* The number of Tx action frames. */
		int64_t	m_numRxVHT_2808_Mbps;
		int64_t	m_numTxSuccessWithRetry;
		int64_t	m_numTxHT_30_Mbps;
		int64_t	m_txRetries;
		int64_t	m_numRxHT_292_5_Mbps;
		int64_t	m_numRxAck; /* The number of all received ACK frames (Acks + BlockAcks). */
		int64_t	m_numRxHT_19_5_Mbps;
		int64_t	m_numTxHT_173_3_Mbps;
		int64_t	m_numTxHT_240_Mbps;
		double	m_averageTxRate;
		int64_t	m_numTxEapol; /* The number of EAPOL frames sent. */
		int64_t	m_numRxHT_288_7_Mbps;
		int64_t	m_numTxManagement; /* The number of TX management frames. */
		int64_t	m_numRxVHT_1755_Mbps;
		int64_t	m_numRxPspoll; /* The number of received ps-poll frames. */
		int64_t	m_numTxVHT_433_2_Mbps;
		int64_t	m_numTxVHT_585_Mbps;
		int64_t	m_numTxHT_86_8_Mbps;
		int64_t	m_numTxHT_52_Mbps;
		std::string	m_classificationName;
		int64_t	m_numRxVHT_292_5_Mbps;
		int64_t	m_sessionId;
		int64_t	m_numRxHT_60_Mbps;
		int64_t	m_numRxTimeToMe;
		int64_t	m_numTxHT_28_7_Mbps;
		int64_t	m_numTxHT_108_Mbps;
		int64_t	m_numRxHT_156_Mbps;
		int64_t	m_numRxVHT_468_Mbps;
		int64_t	m_numTxVHT_2808_Mbps;
		int64_t	m_numTxVHT_526_5_Mbps;
		int64_t	m_numTxHT_324_Mbps;
		int64_t	m_numTxVHT_866_7_Mbps;
		int64_t	m_numRxHT_72_1_Mbps;
		int64_t	m_numRxHT_65_Mbps;
		int64_t	m_numTxDataTransmitted;
		int64_t	m_numRx_18_Mbps;
		int64_t	m_numRxDataFrames_108_Mbps;
		std::vector<int64_t>	m_rates;
		int64_t	m_numTxByteSucc; /* The Number of Tx bytes successfully transmitted. */
		int64_t	m_numRxVHT_1053_Mbps;
		int64_t	m_numRxVHT_2106_Mbps;
		int64_t	m_numRxHT_288_8_Mbps;
		int64_t	m_numRxHT_40_5_Mbps;
		int64_t	m_numTxVHT_1170_Mbps;
		int64_t	m_numTxDataRetries; /* The number of Tx data frames with retries,done. */
		int64_t	m_numTxHT_117_Mbps;
		int64_t	m_numTxHT_13_Mbps;
		int64_t	m_numRxVHT_1872_Mbps;
		int64_t	m_numTxVHT_2340_Mbps;
		int64_t	m_numRxHT_216_6_Mbps;
		int64_t	m_numTxHT_54_Mbps;
		int64_t	m_numRx_9_Mbps;
		int64_t	m_numRxStbc; /* The number of received STBC frames. */
		int64_t	m_numTxHT_270_Mbps;
		int64_t	m_numTxHT_288_8_Mbps;
		int64_t	m_numTxHT_351_2_Mbps;
		int64_t	m_numRxVHT_325_Mbps;
		int64_t	m_numTxHT_135_Mbps;
		int64_t	m_numRxProbeReq; /* The number of received probe request frames. */
		int64_t	m_numTxVHT_1040_Mbps;
		int64_t	m_numTx_12_Mbps;
		int64_t	m_numTxHT_14_3_Mbps;
		int64_t	m_numTxVHT_1300_Mbps;
		int64_t	m_numRxHT_13_Mbps;
		int64_t	m_numRxHT_117_Mbps;
		int64_t	m_numRxHT_28_7_Mbps;
		int64_t	m_numRxHT_195_Mbps;
		int64_t	m_numTxLdpc; /* The number of total LDPC frames sent. */
		int64_t	m_numTxHT_263_2_Mbps;
		int64_t	m_numTxHT_13_5_Mbps;
		int64_t	m_numTxHT_87_8_Mbps;
		int64_t	m_numRxVHT_480_Mbps;
		int64_t	m_numRxDataFramesRetried;
		int64_t	m_numRxVHT_1040_Mbps;
		int64_t	m_numTxMultiRetries; /* The number of Tx frames with retries. */
		int64_t	m_numRxDup; /* The number of received duplicated frames. */
		int64_t	m_numTxVHT_648_Mbps;
		int64_t	m_numRxData; /* The number of received data frames. */
		int64_t	m_numTxVHT_450_Mbps;
		int64_t	m_numTxVHT_432_Mbps;
		int64_t	m_numRxHT_26_Mbps;
		int64_t	m_numTxHT_115_5_Mbps;
		int64_t	m_numRxHT_200_Mbps;
		int64_t	m_numTxHT_86_6_Mbps;
		int64_t	m_numRx_54_Mbps;
		int64_t	m_numTxBytes;
		double	m_averageRxRate;
		int64_t	m_numRxVHT_1950_Mbps;
		int64_t	m_numRxHT_130_3_Mbps;
		int64_t	m_numRxDataFrames;
		int64_t	m_numRxVHT_780_Mbps;
		int64_t	m_numRxHT_54_Mbps;
		int64_t	m_numRxVHT_600_Mbps;
		int64_t	m_numRxDataFrames_300_Mbps;
		int64_t	m_numRxHT_243_Mbps;
		int64_t	m_numRxHT_57_7_Mbps;
		int64_t	m_numRxHT_234_Mbps;
		int64_t	m_numRxDataFrames_12_Mbps;
		int64_t	m_numTxVHT_2106_Mbps;
		int64_t	m_rxDuplicatePackets;
		int64_t	m_numTxTimeFramesTransmitted;
		int64_t	m_numRxVHT_3466_8_Mbps;
		int64_t	m_numRxHT_52_Mbps;
		int64_t	m_numTxControl; /* The number of Tx control frames. */
		int64_t	m_numTx_36_Mbps;
		int64_t	m_numRxHT_32_5_Mbps;
		int64_t	m_numTxHT_15_Mbps;
		int64_t	m_numRxDataFrames_1300_Mbps;
		int64_t	m_numRxVHT_450_Mbps;
		int64_t	m_numTxDataFrames_12_Mbps;
		int64_t	m_numTxVHT_468_Mbps;
		int64_t	m_numRxVHT_800_Mbps;
		int64_t	m_numTxAggrSucc; /* The number of aggregation frames sent successfully. */
		int64_t	m_numTxHT_243_Mbps;
		int64_t	m_numTxVHT_1053_Mbps;
		int64_t	m_numTxHT_90_Mbps;
		int64_t	m_numRxVHT_877_5_Mbps;
		int64_t	m_numTxHT_19_5_Mbps;
		WmmQueueStatsPerQueueTypeMap	m_wmmQueueStats;
		int64_t	m_numTxQueued; /* The number of TX frames queued. */
		int64_t	m_rxLastRssi; /* The RSSI of last frame received. */
		int64_t	m_numRxHT_208_Mbps;
		int64_t	m_numTxRetryDropped; /* The number of TX frame dropped due to retries. */
		std::vector<McsStats>	m_List_McsStats__mcsStats;
		int64_t	m_numRxHT_351_Mbps;
		int64_t	m_numTxData; /* The number of Tx data frames. */
		int64_t	m_numRxVHT_1404_Mbps;
		int64_t	m_numRxVHT_1560_Mbps;
		int64_t	m_lastSentLayer3Ts; /* The timestamp of last successfully sent layer three user traffic (IP data). */
		int64_t	m_numRxHT_43_2_Mbps;
		int64_t	m_numRxHT_29_2_Mbps;
		int64_t	m_numRxHT_13_5_Mbps;
		int64_t	m_numTxVHT_1053_1_Mbps;
		int64_t	m_numTxHT_65_Mbps;
		int64_t	m_numTxVHT_3120_Mbps;
		int64_t	m_numRxHT_87_8_Mbps;
		int64_t	m_numRxHT_312_Mbps;
		int64_t	m_secondsSinceLastRecv;
		int64_t	m_numTxRtsSucc; /* The number of RTS frames sent successfully, done. */
		int64_t	m_numTxHT_325_Mbps;
		int64_t	m_rxBytes; /* The number of received bytes. */
		int64_t	m_numTxSucc; /* The number of frames successfully transmitted. */
		int64_t	m_numTxTimeData;
		int64_t	m_numRxHT_97_5_Mbps;
		int64_t	m_numTx_48_Mbps;
		int64_t	m_numTxVHT_540_Mbps;
		int64_t	m_numTxAggrOneMpdu; /* The number of aggregation frames sent using single MPDU (where the A-MPDU contains only one MPDU ). */
		int64_t	m_numTxHT_360_Mbps;
		int64_t	m_numTxNoAck; /* The number of TX frames failed because of not Acked. */
		int64_t	m_numRxHT_173_1_Mbps;
		int64_t	m_numTxVHT_1733_1_Mbps;
		int64_t	m_numTxHT_104_Mbps;
		int64_t	m_numRxHT_27_Mbps;
		int64_t	m_numTxVHT_325_Mbps;
		McsType	m_lastRxMcsIdx;
		int64_t	m_numRxHT_30_Mbps;
		int64_t	m_numRxNoFcsErr; /* The number of received frames without FCS errors. */
		int64_t	m_numTxDataTransmittedRetried;
		int64_t	m_numTxHT_28_8_Mbps;
		int64_t	m_numRxHT_135_Mbps;
		int64_t	m_numTxVHT_650_Mbps;
		int64_t	m_numRxVHT_1300_Mbps;
		int64_t	m_lastRecvLayer3Ts; /* The timestamp of last received layer three user traffic (IP data) */
		int64_t	m_numTxHT_346_7_Mbps;
		std::string	m_model_type;
		int64_t	m_numTxVHT_1404_Mbps;
		int64_t	m_numRxHT_81_Mbps;
		int64_t	m_numRxHT_78_Mbps;
		int64_t	m_numTxHT_57_5_Mbps;
		int64_t	m_numTxVHT_400_Mbps;
		int64_t	m_numRxVHT_2600_Mbps;
		int64_t	m_numRxVHT_1170_Mbps;
		int64_t	m_numTxHT_117_1_Mbps;
		int64_t	m_numRx_12_Mbps;
		int64_t	m_numTxPropResp; /* The number of TX probe response. */
		int64_t	m_numTxDataFrames_300_Mbps;
		int64_t	m_numTxHT_180_Mbps;
		int64_t	m_numTxHT_292_5_Mbps;
		int64_t	m_numRxHT_162_Mbps;
		int64_t	m_numTxVHT_292_5_Mbps;
		int64_t	m_numTxHT_27_Mbps;
		int64_t	m_numRxVHT_364_5_Mbps;
		int64_t	m_numTxVHT_2600_Mbps;
		int64_t	m_numTxHT_97_5_Mbps;
		int64_t	m_numTxSuccRetries; /* The number of successfully transmitted frames with retries. */
		int64_t	m_numTxVHT_780_Mbps;
		int64_t	m_numRxHT_260_Mbps;
		int64_t	m_numRxVHT_866_7_Mbps;
		int64_t	m_numTxVHT_1579_5_Mbps;
		int64_t	m_numRxHT_45_Mbps;
		int64_t	m_numTxHT_175_5_Mbps;
		int64_t	m_numRxControl; /* The number of received control frames. */
		int64_t	m_numRxHT_263_2_Mbps;
		int64_t	m_numRxVHT_1733_4_Mbps;
		int64_t	m_numTxHT_60_Mbps;
		int64_t	m_numTxVHT_405_Mbps;
		int64_t	m_numTxHT_200_Mbps;
		int64_t	m_numTxVHT_1560_Mbps;
		int64_t	m_periodLengthSec; /* How many seconds the AP measured for the metric */
		int64_t	m_numRxVHT_390_Mbps;
		int64_t	m_numTxVHT_1950_Mbps;
		int64_t	m_numRxHT_86_8_Mbps;
		int64_t	m_numTxVHT_1872_Mbps;
		int64_t	m_numRxHT_57_5_Mbps;
		int64_t	m_numTxHT_300_Mbps;
		int64_t	m_numTxHT_231_1_Mbps;
		int64_t	m_numRxDataFrames_54_Mbps;
		int64_t	m_numRxHT_6_5_Mbps;
		int64_t	m_numTx_54_Mbps;
		int64_t	m_numTxHT_173_1_Mbps;
		int64_t	m_numTxFramesTransmitted;
		int64_t	m_numTxVHT_975_Mbps;
		int64_t	m_numRxHT_58_5_Mbps;
		int64_t	m_numRxHT_173_3_Mbps;
		int64_t	m_numRx_36_Mbps;
		int64_t	m_numTxHT_7_1_Mbps;
		int64_t	m_numRxHT_346_7_Mbps;
		int64_t	m_numRxVHT_520_Mbps;
		int64_t	m_numRxVHT_1053_1_Mbps;
		int64_t	m_numTxDataFrames_54_Mbps;
		int64_t	m_numTxHT_208_Mbps;
		int64_t	m_numRxVHT_720_Mbps;
		int64_t	m_numTxHT_26_Mbps;
		int64_t	m_numTxVHT_480_Mbps;
		int64_t	m_numTxVHT_2080_Mbps;
		int64_t	m_numTxHT_351_Mbps;
		int64_t	m_numRxRts; /* The number of received RTS frames. */
		int64_t	m_numTxRtsFail; /* The number of RTS frames failed transmission. */
		int64_t	m_numRxHT_15_Mbps;
		int64_t	m_numRxHT_175_5_Mbps;
		int64_t	m_numTxHT_288_7_Mbps;
		int64_t	m_numTxHT_29_2_Mbps;
		int64_t	m_numRxHT_144_3_Mbps;
		int64_t	m_numRxVHT_1579_5_Mbps;
		int64_t	m_numRxDataFrames_1300Plus_Mbps;
		int64_t	m_numTxHT_57_7_Mbps;
		int64_t	m_numRxHT_86_6_Mbps;
		int64_t	m_numTx_6_Mbps;
		int64_t	m_numTxHT_45_Mbps;
		int64_t	m_numTxHT_216_Mbps;
		int64_t	m_numTxVHT_800_Mbps;
		int64_t	m_numTxVHT_3466_8_Mbps;
		int64_t	m_numTxHT_260_Mbps;
		int64_t	m_numRxHT_108_Mbps;
		int64_t	m_numTxHT_40_5_Mbps;
		int64_t	m_numTxHT_234_Mbps;
		int64_t	m_numTx_1_Mbps;
		int64_t	m_numRxLdpc; /* The number of received LDPC frames. */
		int64_t	m_numTxSuccNoRetry; /* The number of successfully transmitted frames at first attempt. */
		int64_t	m_numRxHT_21_7_Mbps;
		int64_t	m_numTxHT_162_Mbps;
		int64_t	m_numTxHT_195_Mbps;
		int64_t	m_numRx_6_Mbps;
		int64_t	m_numTxHT_21_7_Mbps;
		int64_t	m_numTxHT_130_3_Mbps;
		int64_t	m_numTxDataFrames_108_Mbps;
		int64_t	m_numRxHT_90_Mbps;
		GuardInterval	m_guardInterval;
		int64_t	m_numRxVHT_486_Mbps;
		int64_t	m_numTxVHT_877_5_Mbps;
		int64_t	m_numRxVHT_540_Mbps;
		int64_t	m_numRxVHT_526_5_Mbps;
		int64_t	m_numTxHT_121_5_Mbps;
		int64_t	m_numTxHT_32_5_Mbps;
		int64_t	m_numTxHT_130_Mbps;
		int64_t	m_numRxHT_240_Mbps;
		int64_t	m_numTxHT_150_Mbps;
		int64_t	m_numRxRetry; /* The number of received retry frames. */
		int64_t	m_numTxHT_72_1_Mbps;
		int64_t	m_ciscoLastRate;
		int64_t	m_numRxVHT_400_Mbps;
		int64_t	m_numRxVHT_2080_Mbps;
		int64_t	m_numTxHT_144_3_Mbps;
		int64_t	m_numRxVHT_403_Mbps;
		int64_t	m_numRxVHT_585_Mbps;
		int64_t	m_numRxHT_117_1_Mbps;
		int64_t	m_numRxHT_7_1_Mbps;
		int64_t	m_numRxCts; /* The number of received CTS frames. */
		int64_t	m_numRxDataFrames_450_Mbps;
		int64_t	m_numRxHT_324_Mbps;
		int64_t	m_numTxHT_120_Mbps;
		std::vector<int64_t>	m_mcs;
		int64_t	m_numTx_9_Mbps;
		int64_t	m_numRxVHT_3120_Mbps;
		int64_t	m_numRxHT_180_Mbps;
		int64_t	m_numRx_1_Mbps;
		int64_t	m_rateCount;
		int64_t	m_numTxHT_78_Mbps;
		int64_t	m_numTxVHT_1733_4_Mbps;
		int64_t	m_numRxHT_300_Mbps;
		int64_t	m_numRxVHT_650_Mbps;
		int64_t	m_numTxStbc; /* The number of total STBC frames sent. */
		int64_t	m_numTxDataFrames_1300_Mbps;
		int64_t	m_numTxMultipleRetries;
		int64_t	m_numTxHT_312_Mbps;
		int64_t	m_numTxDataFrames_450_Mbps;
		int64_t	m_numRxHT_14_3_Mbps;
		int64_t	m_numRxHT_130_Mbps;
		int64_t	m_numRxVHT_975_Mbps;
		int64_t	m_numTxHT_58_5_Mbps;
		int64_t	m_numRcvFrameForTx; /* The number of received ethernet and local generated frames for transmit. */
		ChannelBandwidth	m_channelBandWidth;
		int64_t	m_numTxVHT_520_Mbps;
		RadioType	m_radioType;
		int64_t	m_rssi;
		int64_t	m_numRxManagement; /* The number of received management frames. */
		int64_t	m_vhtMcs;
		int64_t	m_numRxVHT_936_Mbps;
		int64_t	m_numRxPackets;
		int64_t	m_numRx_24_Mbps;
		int64_t	m_snr;
		int64_t	m_numTxHT_156_Mbps;
		int64_t	m_numRxHT_270_Mbps;
		int64_t	m_numRxHT_115_5_Mbps;
		int64_t	m_numRxHT_231_1_Mbps;
		int64_t	m_numTx_24_Mbps;
};

class RadioConfiguration  {
	public:
		const SourceSelectionSteering get_bestApSettings() const { return m_bestApSettings;}
		void set_bestApSettings(const SourceSelectionSteering & value) { m_bestApSettings = value; }
		const bool get_deauthAttackDetection() const { return m_deauthAttackDetection;}
		void set_deauthAttackDetection(const bool & value) { m_deauthAttackDetection = value; }
		const int64_t get_dtimPeriod() const { return m_dtimPeriod;}
		void set_dtimPeriod(const int64_t & value) { m_dtimPeriod = value; }
		const int64_t get_fragmentationThresholdBytes() const { return m_fragmentationThresholdBytes;}
		void set_fragmentationThresholdBytes(const int64_t & value) { m_fragmentationThresholdBytes = value; }
		const StateSetting get_legacyBSSRate() const { return m_legacyBSSRate;}
		void set_legacyBSSRate(const StateSetting & value) { m_legacyBSSRate = value; }
		const SourceSelectionManagement get_managementRate() const { return m_managementRate;}
		void set_managementRate(const SourceSelectionManagement & value) { m_managementRate = value; }
		const SourceSelectionMulticast get_multicastRate() const { return m_multicastRate;}
		void set_multicastRate(const SourceSelectionMulticast & value) { m_multicastRate = value; }
		const StateSetting get_radioAdminState() const { return m_radioAdminState;}
		void set_radioAdminState(const StateSetting & value) { m_radioAdminState = value; }
		const RadioType get_radioType() const { return m_radioType;}
		void set_radioType(const RadioType & value) { m_radioType = value; }
		const StateSetting get_stationIsolation() const { return m_stationIsolation;}
		void set_stationIsolation(const StateSetting & value) { m_stationIsolation = value; }
		const StateSetting get_uapsdState() const { return m_uapsdState;}
		void set_uapsdState(const StateSetting & value) { m_uapsdState = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		SourceSelectionSteering	m_bestApSettings;
		bool	m_deauthAttackDetection;
		int64_t	m_dtimPeriod;
		int64_t	m_fragmentationThresholdBytes;
		StateSetting	m_legacyBSSRate;
		SourceSelectionManagement	m_managementRate;
		SourceSelectionMulticast	m_multicastRate;
		StateSetting	m_radioAdminState;
		RadioType	m_radioType;
		StateSetting	m_stationIsolation;
		StateSetting	m_uapsdState;
};

class LinkQualityAggregatedStatsPerRadioTypeMap  {
	public:
		const LinkQualityAggregatedStats get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const LinkQualityAggregatedStats & value) { m_is2dot4GHz = value; }
		const LinkQualityAggregatedStats get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const LinkQualityAggregatedStats & value) { m_is5GHz = value; }
		const LinkQualityAggregatedStats get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const LinkQualityAggregatedStats & value) { m_is5GHzL = value; }
		const LinkQualityAggregatedStats get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const LinkQualityAggregatedStats & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		LinkQualityAggregatedStats	m_is2dot4GHz;
		LinkQualityAggregatedStats	m_is5GHz;
		LinkQualityAggregatedStats	m_is5GHzL;
		LinkQualityAggregatedStats	m_is5GHzU;
};

class DhcpOfferEvent  : public BaseDhcpEvent {
	public:
		const bool get_fromInternal() const { return m_fromInternal;}
		void set_fromInternal(const bool & value) { m_fromInternal = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_fromInternal;
		std::string	m_model_type;
};

class DhcpInformEvent  : public BaseDhcpEvent {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
};

class EquipmentRrmBulkUpdateItemPerRadioMap  {
	public:
		const RrmBulkUpdateApDetails get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const RrmBulkUpdateApDetails & value) { m_is2dot4GHz = value; }
		const RrmBulkUpdateApDetails get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const RrmBulkUpdateApDetails & value) { m_is5GHz = value; }
		const RrmBulkUpdateApDetails get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const RrmBulkUpdateApDetails & value) { m_is5GHzL = value; }
		const RrmBulkUpdateApDetails get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const RrmBulkUpdateApDetails & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RrmBulkUpdateApDetails	m_is2dot4GHz;
		RrmBulkUpdateApDetails	m_is5GHz;
		RrmBulkUpdateApDetails	m_is5GHzL;
		RrmBulkUpdateApDetails	m_is5GHzU;
};

class RadiusDetails  {
	public:
		const std::vector<RadiusServerDetails> get_radiusServerDetails() const { return m_radiusServerDetails;}
		void set_radiusServerDetails(const std::vector<RadiusServerDetails> & value) { m_radiusServerDetails = value; }
		const StatusCode get_status() const { return m_status;}
		void set_status(const StatusCode & value) { m_status = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<RadiusServerDetails>	m_radiusServerDetails;
		StatusCode	m_status;
};

class ServiceMetricsCollectionConfigProfile  : public ProfileDetails {
	public:
		const MetricConfigParameterMap get_metricConfigParameterMap() const { return m_metricConfigParameterMap;}
		void set_metricConfigParameterMap(const MetricConfigParameterMap & value) { m_metricConfigParameterMap = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::vector<RadioType> get_radioTypes() const { return m_radioTypes;}
		void set_radioTypes(const std::vector<RadioType> & value) { m_radioTypes = value; }
		const std::vector<ServiceMetricDataType> get_serviceMetricDataTypes() const { return m_serviceMetricDataTypes;}
		void set_serviceMetricDataTypes(const std::vector<ServiceMetricDataType> & value) { m_serviceMetricDataTypes = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MetricConfigParameterMap	m_metricConfigParameterMap;
		std::string	m_model_type;
		std::vector<RadioType>	m_radioTypes;
		std::vector<ServiceMetricDataType>	m_serviceMetricDataTypes;
};

class DhcpDeclineEvent  : public BaseDhcpEvent {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
};

class ClientActivityAggregatedStatsPerRadioTypeMap  {
	public:
		const ClientActivityAggregatedStats get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const ClientActivityAggregatedStats & value) { m_is2dot4GHz = value; }
		const ClientActivityAggregatedStats get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const ClientActivityAggregatedStats & value) { m_is5GHz = value; }
		const ClientActivityAggregatedStats get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const ClientActivityAggregatedStats & value) { m_is5GHzL = value; }
		const ClientActivityAggregatedStats get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const ClientActivityAggregatedStats & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ClientActivityAggregatedStats	m_is2dot4GHz;
		ClientActivityAggregatedStats	m_is5GHz;
		ClientActivityAggregatedStats	m_is5GHzL;
		ClientActivityAggregatedStats	m_is5GHzU;
};

class PaginationResponseServiceMetric  {
	public:
		const PaginationContextServiceMetric get_context() const { return m_context;}
		void set_context(const PaginationContextServiceMetric & value) { m_context = value; }
		const std::vector<ServiceMetric> get_items() const { return m_items;}
		void set_items(const std::vector<ServiceMetric> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextServiceMetric	m_context;
		std::vector<ServiceMetric>	m_items;
};

class Client  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const ClientInfoDetails get_details() const { return m_details;}
		void set_details(const ClientInfoDetails & value) { m_details = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const MacAddress get_macAddress() const { return m_macAddress;}
		void set_macAddress(const MacAddress & value) { m_macAddress = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		ClientInfoDetails	m_details;
		int64_t	m_lastModifiedTimestamp; /* This class does not perform checks against concurrrent updates. Here last update always wins. */
		MacAddress	m_macAddress;
};

class ApNetworkConfiguration  : public ProfileDetails {
	public:
		const bool get_equipmentDiscovery() const { return m_equipmentDiscovery;}
		void set_equipmentDiscovery(const bool & value) { m_equipmentDiscovery = value; }
		const std::string get_equipmentType() const { return m_equipmentType;}
		void set_equipmentType(const std::string & value) { m_equipmentType = value; }
		const std::vector<GreTunnelConfiguration> get_greTunnelConfigurations() const { return m_greTunnelConfigurations;}
		void set_greTunnelConfigurations(const std::vector<GreTunnelConfiguration> & value) { m_greTunnelConfigurations = value; }
		const bool get_ledControlEnabled() const { return m_ledControlEnabled;}
		void set_ledControlEnabled(const bool & value) { m_ledControlEnabled = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_networkConfigVersion() const { return m_networkConfigVersion;}
		void set_networkConfigVersion(const std::string & value) { m_networkConfigVersion = value; }
		const bool get_ntpServer() const { return m_ntpServer;}
		void set_ntpServer(const bool & value) { m_ntpServer = value; }
		const RadioProfileConfigurationMap get_radioMap() const { return m_radioMap;}
		void set_radioMap(const RadioProfileConfigurationMap & value) { m_radioMap = value; }
		const RtlsSettings get_rtlsSettings() const { return m_rtlsSettings;}
		void set_rtlsSettings(const RtlsSettings & value) { m_rtlsSettings = value; }
		const bool get_syntheticClientEnabled() const { return m_syntheticClientEnabled;}
		void set_syntheticClientEnabled(const bool & value) { m_syntheticClientEnabled = value; }
		const SyslogRelay get_syslogRelay() const { return m_syslogRelay;}
		void set_syslogRelay(const SyslogRelay & value) { m_syslogRelay = value; }
		const int64_t get_vlan() const { return m_vlan;}
		void set_vlan(const int64_t & value) { m_vlan = value; }
		const bool get_vlanNative() const { return m_vlanNative;}
		void set_vlanNative(const bool & value) { m_vlanNative = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_equipmentDiscovery;
		std::string	m_equipmentType;
		std::vector<GreTunnelConfiguration>	m_greTunnelConfigurations;
		bool	m_ledControlEnabled;
		std::string	m_model_type;
		std::string	m_networkConfigVersion;
		bool	m_ntpServer;
		RadioProfileConfigurationMap	m_radioMap;
		RtlsSettings	m_rtlsSettings;
		bool	m_syntheticClientEnabled;
		SyslogRelay	m_syslogRelay;
		int64_t	m_vlan;
		bool	m_vlanNative;
};

class RealTimeSipCallReportEvent  : public RealTimeSipCallEventWithStats {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const SIPCallReportReason get_reportReason() const { return m_reportReason;}
		void set_reportReason(const SIPCallReportReason & value) { m_reportReason = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		SIPCallReportReason	m_reportReason;
};

class Alarm  {
	public:
		const bool get_acknowledged() const { return m_acknowledged;}
		void set_acknowledged(const bool & value) { m_acknowledged = value; }
		const AlarmCode get_alarmCode() const { return m_alarmCode;}
		void set_alarmCode(const AlarmCode & value) { m_alarmCode = value; }
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const AlarmDetails get_details() const { return m_details;}
		void set_details(const AlarmDetails & value) { m_details = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const OriginatorType get_originatorType() const { return m_originatorType;}
		void set_originatorType(const OriginatorType & value) { m_originatorType = value; }
		const std::string get_scopeId() const { return m_scopeId;}
		void set_scopeId(const std::string & value) { m_scopeId = value; }
		const AlarmScopeType get_scopeType() const { return m_scopeType;}
		void set_scopeType(const AlarmScopeType & value) { m_scopeType = value; }
		const StatusCode get_severity() const { return m_severity;}
		void set_severity(const StatusCode & value) { m_severity = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_acknowledged;
		AlarmCode	m_alarmCode;
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		AlarmDetails	m_details;
		int64_t	m_equipmentId;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		OriginatorType	m_originatorType;
		std::string	m_scopeId;
		AlarmScopeType	m_scopeType;
		StatusCode	m_severity;
};

class AlarmCounts  {
	public:
		const CountsPerEquipmentIdPerAlarmCodeMap get_countsPerEquipmentIdMap() const { return m_countsPerEquipmentIdMap;}
		void set_countsPerEquipmentIdMap(const CountsPerEquipmentIdPerAlarmCodeMap & value) { m_countsPerEquipmentIdMap = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const CountsPerAlarmCodeMap get_totalCountsPerAlarmCodeMap() const { return m_totalCountsPerAlarmCodeMap;}
		void set_totalCountsPerAlarmCodeMap(const CountsPerAlarmCodeMap & value) { m_totalCountsPerAlarmCodeMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		CountsPerEquipmentIdPerAlarmCodeMap	m_countsPerEquipmentIdMap;
		int64_t	m_customerId;
		CountsPerAlarmCodeMap	m_totalCountsPerAlarmCodeMap;
};

class ChannelInfoReports  {
	public:
		const ListOfChannelInfoReportsPerRadioMap get_channelInformationReportsPerRadio() const { return m_channelInformationReportsPerRadio;}
		void set_channelInformationReportsPerRadio(const ListOfChannelInfoReportsPerRadioMap & value) { m_channelInformationReportsPerRadio = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ListOfChannelInfoReportsPerRadioMap	m_channelInformationReportsPerRadio;
		std::string	m_model_type;
};

class CapacityPerRadioDetailsMap  {
	public:
		const CapacityPerRadioDetails get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const CapacityPerRadioDetails & value) { m_is2dot4GHz = value; }
		const CapacityPerRadioDetails get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const CapacityPerRadioDetails & value) { m_is5GHz = value; }
		const CapacityPerRadioDetails get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const CapacityPerRadioDetails & value) { m_is5GHzL = value; }
		const CapacityPerRadioDetails get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const CapacityPerRadioDetails & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		CapacityPerRadioDetails	m_is2dot4GHz;
		CapacityPerRadioDetails	m_is5GHz;
		CapacityPerRadioDetails	m_is5GHzL;
		CapacityPerRadioDetails	m_is5GHzU;
};

class ClientSession  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const ClientSessionDetails get_details() const { return m_details;}
		void set_details(const ClientSessionDetails & value) { m_details = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const MacAddress get_macAddress() const { return m_macAddress;}
		void set_macAddress(const MacAddress & value) { m_macAddress = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		ClientSessionDetails	m_details;
		int64_t	m_equipmentId;
		int64_t	m_lastModifiedTimestamp; /* This class does not perform checks against concurrrent updates. Here last update always wins. */
		MacAddress	m_macAddress;
};

class TrafficDetails  {
	public:
		const float get_indicatorValueRxMbps() const { return m_indicatorValueRxMbps;}
		void set_indicatorValueRxMbps(const float & value) { m_indicatorValueRxMbps = value; }
		const float get_indicatorValueTxMbps() const { return m_indicatorValueTxMbps;}
		void set_indicatorValueTxMbps(const float & value) { m_indicatorValueTxMbps = value; }
		const TrafficPerRadioDetailsPerRadioTypeMap get_perRadioDetails() const { return m_perRadioDetails;}
		void set_perRadioDetails(const TrafficPerRadioDetailsPerRadioTypeMap & value) { m_perRadioDetails = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		float	m_indicatorValueRxMbps;
		float	m_indicatorValueTxMbps;
		TrafficPerRadioDetailsPerRadioTypeMap	m_perRadioDetails;
};

class DhcpRequestEvent  : public BaseDhcpEvent {
	public:
		const std::string get_hostName() const { return m_hostName;}
		void set_hostName(const std::string & value) { m_hostName = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_hostName;
		std::string	m_model_type;
};

class DhcpNakEvent  : public BaseDhcpEvent {
	public:
		const bool get_fromInternal() const { return m_fromInternal;}
		void set_fromInternal(const bool & value) { m_fromInternal = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		bool	m_fromInternal;
		std::string	m_model_type;
};

class MapOfWmmQueueStatsPerRadioMap  {
	public:
		const WmmQueueStatsPerQueueTypeMap get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const WmmQueueStatsPerQueueTypeMap & value) { m_is2dot4GHz = value; }
		const WmmQueueStatsPerQueueTypeMap get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const WmmQueueStatsPerQueueTypeMap & value) { m_is5GHz = value; }
		const WmmQueueStatsPerQueueTypeMap get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const WmmQueueStatsPerQueueTypeMap & value) { m_is5GHzL = value; }
		const WmmQueueStatsPerQueueTypeMap get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const WmmQueueStatsPerQueueTypeMap & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		WmmQueueStatsPerQueueTypeMap	m_is2dot4GHz;
		WmmQueueStatsPerQueueTypeMap	m_is5GHz;
		WmmQueueStatsPerQueueTypeMap	m_is5GHzL;
		WmmQueueStatsPerQueueTypeMap	m_is5GHzU;
};

class PaginationResponseSystemEvent  {
	public:
		const PaginationContextSystemEvent get_context() const { return m_context;}
		void set_context(const PaginationContextSystemEvent & value) { m_context = value; }
		const std::vector<SystemEventRecord> get_items() const { return m_items;}
		void set_items(const std::vector<SystemEventRecord> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextSystemEvent	m_context;
		std::vector<SystemEventRecord>	m_items;
};

class CustomerDetails  {
	public:
		const EquipmentAutoProvisioningSettings get_autoProvisioning() const { return m_autoProvisioning;}
		void set_autoProvisioning(const EquipmentAutoProvisioningSettings & value) { m_autoProvisioning = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		EquipmentAutoProvisioningSettings	m_autoProvisioning;
};

class SsidStatistics  {
	public:
		const int64_t get_numRxNullData() const { return m_numRxNullData;}
		void set_numRxNullData(const int64_t & value) { m_numRxNullData = value; }
		const int64_t get_rxDataBytes() const { return m_rxDataBytes;}
		void set_rxDataBytes(const int64_t & value) { m_rxDataBytes = value; }
		const int64_t get_numClient() const { return m_numClient;}
		void set_numClient(const int64_t & value) { m_numClient = value; }
		const int64_t get_numTxDropped() const { return m_numTxDropped;}
		void set_numTxDropped(const int64_t & value) { m_numTxDropped = value; }
		const int64_t get_numTxAction() const { return m_numTxAction;}
		void set_numTxAction(const int64_t & value) { m_numTxAction = value; }
		const int64_t get_numRxAck() const { return m_numRxAck;}
		void set_numRxAck(const int64_t & value) { m_numRxAck = value; }
		const int64_t get_numTxEapol() const { return m_numTxEapol;}
		void set_numTxEapol(const int64_t & value) { m_numTxEapol = value; }
		const int64_t get_numTxManagement() const { return m_numTxManagement;}
		void set_numTxManagement(const int64_t & value) { m_numTxManagement = value; }
		const int64_t get_numRxPspoll() const { return m_numRxPspoll;}
		void set_numRxPspoll(const int64_t & value) { m_numRxPspoll = value; }
		const int64_t get_numTxBcDropped() const { return m_numTxBcDropped;}
		void set_numTxBcDropped(const int64_t & value) { m_numTxBcDropped = value; }
		const int64_t get_numTxDataRetries() const { return m_numTxDataRetries;}
		void set_numTxDataRetries(const int64_t & value) { m_numTxDataRetries = value; }
		const MacAddress get_bssid() const { return m_bssid;}
		void set_bssid(const MacAddress & value) { m_bssid = value; }
		const int64_t get_numRxStbc() const { return m_numRxStbc;}
		void set_numRxStbc(const int64_t & value) { m_numRxStbc = value; }
		const int64_t get_numRxProbeReq() const { return m_numRxProbeReq;}
		void set_numRxProbeReq(const int64_t & value) { m_numRxProbeReq = value; }
		const int64_t get_numTxDtimMc() const { return m_numTxDtimMc;}
		void set_numTxDtimMc(const int64_t & value) { m_numTxDtimMc = value; }
		const int64_t get_numTxLdpc() const { return m_numTxLdpc;}
		void set_numTxLdpc(const int64_t & value) { m_numTxLdpc = value; }
		const int64_t get_numTxMultiRetries() const { return m_numTxMultiRetries;}
		void set_numTxMultiRetries(const int64_t & value) { m_numTxMultiRetries = value; }
		const int64_t get_numRxDup() const { return m_numRxDup;}
		void set_numRxDup(const int64_t & value) { m_numRxDup = value; }
		const int64_t get_numRxData() const { return m_numRxData;}
		void set_numRxData(const int64_t & value) { m_numRxData = value; }
		const int64_t get_numTxControl() const { return m_numTxControl;}
		void set_numTxControl(const int64_t & value) { m_numTxControl = value; }
		const int64_t get_numTxAggrSucc() const { return m_numTxAggrSucc;}
		void set_numTxAggrSucc(const int64_t & value) { m_numTxAggrSucc = value; }
		const WmmQueueStatsPerQueueTypeMap get_wmmQueueStats() const { return m_wmmQueueStats;}
		void set_wmmQueueStats(const WmmQueueStatsPerQueueTypeMap & value) { m_wmmQueueStats = value; }
		const int64_t get_numTxQueued() const { return m_numTxQueued;}
		void set_numTxQueued(const int64_t & value) { m_numTxQueued = value; }
		const int64_t get_rxLastRssi() const { return m_rxLastRssi;}
		void set_rxLastRssi(const int64_t & value) { m_rxLastRssi = value; }
		const int64_t get_numTxRetryDropped() const { return m_numTxRetryDropped;}
		void set_numTxRetryDropped(const int64_t & value) { m_numTxRetryDropped = value; }
		const int64_t get_numTxData() const { return m_numTxData;}
		void set_numTxData(const int64_t & value) { m_numTxData = value; }
		const int64_t get_numTxRtsSucc() const { return m_numTxRtsSucc;}
		void set_numTxRtsSucc(const int64_t & value) { m_numTxRtsSucc = value; }
		const int64_t get_rxBytes() const { return m_rxBytes;}
		void set_rxBytes(const int64_t & value) { m_rxBytes = value; }
		const int64_t get_numTxSucc() const { return m_numTxSucc;}
		void set_numTxSucc(const int64_t & value) { m_numTxSucc = value; }
		const int64_t get_numTxAggrOneMpdu() const { return m_numTxAggrOneMpdu;}
		void set_numTxAggrOneMpdu(const int64_t & value) { m_numTxAggrOneMpdu = value; }
		const int64_t get_numTxNoAck() const { return m_numTxNoAck;}
		void set_numTxNoAck(const int64_t & value) { m_numTxNoAck = value; }
		const int64_t get_numRcvBcForTx() const { return m_numRcvBcForTx;}
		void set_numRcvBcForTx(const int64_t & value) { m_numRcvBcForTx = value; }
		const int64_t get_numTxBytesSucc() const { return m_numTxBytesSucc;}
		void set_numTxBytesSucc(const int64_t & value) { m_numTxBytesSucc = value; }
		const int64_t get_numRxNoFcsErr() const { return m_numRxNoFcsErr;}
		void set_numRxNoFcsErr(const int64_t & value) { m_numRxNoFcsErr = value; }
		const int64_t get_numTxPropResp() const { return m_numTxPropResp;}
		void set_numTxPropResp(const int64_t & value) { m_numTxPropResp = value; }
		const int64_t get_numTxSuccRetries() const { return m_numTxSuccRetries;}
		void set_numTxSuccRetries(const int64_t & value) { m_numTxSuccRetries = value; }
		const int64_t get_numTxPsUnicast() const { return m_numTxPsUnicast;}
		void set_numTxPsUnicast(const int64_t & value) { m_numTxPsUnicast = value; }
		const int64_t get_numRxControl() const { return m_numRxControl;}
		void set_numRxControl(const int64_t & value) { m_numRxControl = value; }
		const int64_t get_numRxRts() const { return m_numRxRts;}
		void set_numRxRts(const int64_t & value) { m_numRxRts = value; }
		const int64_t get_numTxRtsFail() const { return m_numTxRtsFail;}
		void set_numTxRtsFail(const int64_t & value) { m_numTxRtsFail = value; }
		const std::vector<McsStats> get_mcsStats() const { return m_mcsStats;}
		void set_mcsStats(const std::vector<McsStats> & value) { m_mcsStats = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }
		const int64_t get_numRxLdpc() const { return m_numRxLdpc;}
		void set_numRxLdpc(const int64_t & value) { m_numRxLdpc = value; }
		const int64_t get_numTxSuccNoRetry() const { return m_numTxSuccNoRetry;}
		void set_numTxSuccNoRetry(const int64_t & value) { m_numTxSuccNoRetry = value; }
		const int64_t get_numRxRetry() const { return m_numRxRetry;}
		void set_numRxRetry(const int64_t & value) { m_numRxRetry = value; }
		const int64_t get_numRxCts() const { return m_numRxCts;}
		void set_numRxCts(const int64_t & value) { m_numRxCts = value; }
		const int64_t get_numTxStbc() const { return m_numTxStbc;}
		void set_numTxStbc(const int64_t & value) { m_numTxStbc = value; }
		const int64_t get_numRcvFrameForTx() const { return m_numRcvFrameForTx;}
		void set_numRcvFrameForTx(const int64_t & value) { m_numRcvFrameForTx = value; }
		const int64_t get_numRxManagement() const { return m_numRxManagement;}
		void set_numRxManagement(const int64_t & value) { m_numRxManagement = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_numRxNullData; /* The number of received null data frames. */
		int64_t	m_rxDataBytes; /* The number of received data bytes. */
		int64_t	m_numClient; /* Number client associated to this BSS */
		int64_t	m_numTxDropped; /* The number of every TX frame dropped. */
		int64_t	m_numTxAction; /* The number of Tx action frames. */
		int64_t	m_numRxAck; /* The number of all received ACK frames (Acks + BlockAcks). */
		int64_t	m_numTxEapol; /* The number of EAPOL frames sent. */
		int64_t	m_numTxManagement; /* The number of TX management frames. */
		int64_t	m_numRxPspoll; /* The number of received ps-poll frames. */
		int64_t	m_numTxBcDropped; /* The number of broadcast frames dropped. */
		int64_t	m_numTxDataRetries; /* The number of Tx data frames with retries. */
		MacAddress	m_bssid;
		int64_t	m_numRxStbc; /* The number of received STBC frames. */
		int64_t	m_numRxProbeReq; /* The number of received probe request frames. */
		int64_t	m_numTxDtimMc; /* The number of transmitted DTIM multicast frames. */
		int64_t	m_numTxLdpc; /* The number of total LDPC frames sent. */
		int64_t	m_numTxMultiRetries; /* The number of Tx frames with retries. */
		int64_t	m_numRxDup; /* The number of received duplicated frames. */
		int64_t	m_numRxData; /* The number of received data frames. */
		int64_t	m_numTxControl; /* The number of Tx control frames. */
		int64_t	m_numTxAggrSucc; /* The number of aggregation frames sent successfully. */
		WmmQueueStatsPerQueueTypeMap	m_wmmQueueStats;
		int64_t	m_numTxQueued; /* The number of TX frames queued. */
		int64_t	m_rxLastRssi; /* The RSSI of last frame received. */
		int64_t	m_numTxRetryDropped; /* The number of TX frame dropped due to retries. */
		int64_t	m_numTxData; /* The number of Tx data frames. */
		int64_t	m_numTxRtsSucc; /* The number of RTS frames sent successfully. */
		int64_t	m_rxBytes; /* The number of received bytes. */
		int64_t	m_numTxSucc; /* The number of frames successfully transmitted. */
		int64_t	m_numTxAggrOneMpdu; /* The number of aggregation frames sent using single MPDU (where the A-MPDU contains only one MPDU ). */
		int64_t	m_numTxNoAck; /* The number of TX frames failed because of not Acked. */
		int64_t	m_numRcvBcForTx; /* The number of received ethernet and local generated broadcast frames for transmit. */
		int64_t	m_numTxBytesSucc; /* The number of bytes successfully transmitted. */
		int64_t	m_numRxNoFcsErr; /* The number of received frames without FCS errors. */
		int64_t	m_numTxPropResp; /* The number of TX probe response. */
		int64_t	m_numTxSuccRetries; /* The number of successfully transmitted frames with retries. */
		int64_t	m_numTxPsUnicast; /* The number of transmitted PS unicast frame. */
		int64_t	m_numRxControl; /* The number of received control frames. */
		int64_t	m_numRxRts; /* The number of received RTS frames. */
		int64_t	m_numTxRtsFail; /* The number of RTS frames failed transmission. */
		std::vector<McsStats>	m_mcsStats;
		std::string	m_ssid; /* SSID */
		int64_t	m_numRxLdpc; /* The number of received LDPC frames. */
		int64_t	m_numTxSuccNoRetry; /* The number of successfully transmitted frames at firt attemp. */
		int64_t	m_numRxRetry; /* The number of received retry frames. */
		int64_t	m_numRxCts; /* The number of received CTS frames. */
		int64_t	m_numTxStbc; /* The number of total STBC frames sent. */
		int64_t	m_numRcvFrameForTx; /* The number of received ethernet and local generated frames for transmit. */
		int64_t	m_numRxManagement; /* The number of received management frames. */
};

class RadioMap  {
	public:
		const ElementRadioConfiguration get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const ElementRadioConfiguration & value) { m_is2dot4GHz = value; }
		const ElementRadioConfiguration get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const ElementRadioConfiguration & value) { m_is5GHz = value; }
		const ElementRadioConfiguration get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const ElementRadioConfiguration & value) { m_is5GHzL = value; }
		const ElementRadioConfiguration get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const ElementRadioConfiguration & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ElementRadioConfiguration	m_is2dot4GHz;
		ElementRadioConfiguration	m_is5GHz;
		ElementRadioConfiguration	m_is5GHzL;
		ElementRadioConfiguration	m_is5GHzU;
};

class RadioUtilizationDetails  {
	public:
		const RadioUtilizationPerRadioDetailsMap get_perRadioDetails() const { return m_perRadioDetails;}
		void set_perRadioDetails(const RadioUtilizationPerRadioDetailsMap & value) { m_perRadioDetails = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RadioUtilizationPerRadioDetailsMap	m_perRadioDetails;
};

class CaptivePortalConfiguration  : public ProfileDetails {
	public:
		const CaptivePortalAuthenticationType get_authenticationType() const { return m_authenticationType;}
		void set_authenticationType(const CaptivePortalAuthenticationType & value) { m_authenticationType = value; }
		const ManagedFileInfo get_backgroundFile() const { return m_backgroundFile;}
		void set_backgroundFile(const ManagedFileInfo & value) { m_backgroundFile = value; }
		const BackgroundPosition get_backgroundPosition() const { return m_backgroundPosition;}
		void set_backgroundPosition(const BackgroundPosition & value) { m_backgroundPosition = value; }
		const BackgroundRepeat get_backgroundRepeat() const { return m_backgroundRepeat;}
		void set_backgroundRepeat(const BackgroundRepeat & value) { m_backgroundRepeat = value; }
		const std::string get_browserTitle() const { return m_browserTitle;}
		void set_browserTitle(const std::string & value) { m_browserTitle = value; }
		const SessionExpiryType get_expiryType() const { return m_expiryType;}
		void set_expiryType(const SessionExpiryType & value) { m_expiryType = value; }
		const std::string get_externalCaptivePortalURL() const { return m_externalCaptivePortalURL;}
		void set_externalCaptivePortalURL(const std::string & value) { m_externalCaptivePortalURL = value; }
		const ManagedFileInfo get_externalPolicyFile() const { return m_externalPolicyFile;}
		void set_externalPolicyFile(const ManagedFileInfo & value) { m_externalPolicyFile = value; }
		const std::string get_headerContent() const { return m_headerContent;}
		void set_headerContent(const std::string & value) { m_headerContent = value; }
		const ManagedFileInfo get_logoFile() const { return m_logoFile;}
		void set_logoFile(const ManagedFileInfo & value) { m_logoFile = value; }
		const std::vector<MacAllowlistRecord> get_macAllowList() const { return m_macAllowList;}
		void set_macAllowList(const std::vector<MacAllowlistRecord> & value) { m_macAllowList = value; }
		const int64_t get_maxUsersWithSameCredentials() const { return m_maxUsersWithSameCredentials;}
		void set_maxUsersWithSameCredentials(const int64_t & value) { m_maxUsersWithSameCredentials = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const RadiusAuthenticationMethod get_radiusAuthMethod() const { return m_radiusAuthMethod;}
		void set_radiusAuthMethod(const RadiusAuthenticationMethod & value) { m_radiusAuthMethod = value; }
		const int64_t get_radiusServiceId() const { return m_radiusServiceId;}
		void set_radiusServiceId(const int64_t & value) { m_radiusServiceId = value; }
		const std::string get_redirectURL() const { return m_redirectURL;}
		void set_redirectURL(const std::string & value) { m_redirectURL = value; }
		const int64_t get_sessionTimeoutInMinutes() const { return m_sessionTimeoutInMinutes;}
		void set_sessionTimeoutInMinutes(const int64_t & value) { m_sessionTimeoutInMinutes = value; }
		const std::string get_successPageMarkdownText() const { return m_successPageMarkdownText;}
		void set_successPageMarkdownText(const std::string & value) { m_successPageMarkdownText = value; }
		const std::string get_userAcceptancePolicy() const { return m_userAcceptancePolicy;}
		void set_userAcceptancePolicy(const std::string & value) { m_userAcceptancePolicy = value; }
		const std::vector<TimedAccessUserRecord> get_userList() const { return m_userList;}
		void set_userList(const std::vector<TimedAccessUserRecord> & value) { m_userList = value; }
		const ManagedFileInfo get_usernamePasswordFile() const { return m_usernamePasswordFile;}
		void set_usernamePasswordFile(const ManagedFileInfo & value) { m_usernamePasswordFile = value; }
		const std::vector<std::string> get_walledGardenAllowlist() const { return m_walledGardenAllowlist;}
		void set_walledGardenAllowlist(const std::vector<std::string> & value) { m_walledGardenAllowlist = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		CaptivePortalAuthenticationType	m_authenticationType;
		ManagedFileInfo	m_backgroundFile;
		BackgroundPosition	m_backgroundPosition;
		BackgroundRepeat	m_backgroundRepeat;
		std::string	m_browserTitle;
		SessionExpiryType	m_expiryType;
		std::string	m_externalCaptivePortalURL;
		ManagedFileInfo	m_externalPolicyFile;
		std::string	m_headerContent;
		ManagedFileInfo	m_logoFile;
		std::vector<MacAllowlistRecord>	m_macAllowList;
		int64_t	m_maxUsersWithSameCredentials;
		std::string	m_model_type;
		RadiusAuthenticationMethod	m_radiusAuthMethod;
		int64_t	m_radiusServiceId;
		std::string	m_redirectURL;
		int64_t	m_sessionTimeoutInMinutes;
		std::string	m_successPageMarkdownText;
		std::string	m_userAcceptancePolicy;
		std::vector<TimedAccessUserRecord>	m_userList;
		ManagedFileInfo	m_usernamePasswordFile;
		std::vector<std::string>	m_walledGardenAllowlist;
};

class DhcpAckEvent  : public BaseDhcpEvent {
	public:
		const std::string get_gatewayIp() const { return m_gatewayIp;}
		void set_gatewayIp(const std::string & value) { m_gatewayIp = value; }
		const int64_t get_leaseTime() const { return m_leaseTime;}
		void set_leaseTime(const int64_t & value) { m_leaseTime = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_primaryDns() const { return m_primaryDns;}
		void set_primaryDns(const std::string & value) { m_primaryDns = value; }
		const int64_t get_rebindingTime() const { return m_rebindingTime;}
		void set_rebindingTime(const int64_t & value) { m_rebindingTime = value; }
		const int64_t get_renewalTime() const { return m_renewalTime;}
		void set_renewalTime(const int64_t & value) { m_renewalTime = value; }
		const std::string get_secondaryDns() const { return m_secondaryDns;}
		void set_secondaryDns(const std::string & value) { m_secondaryDns = value; }
		const std::string get_subnetMask() const { return m_subnetMask;}
		void set_subnetMask(const std::string & value) { m_subnetMask = value; }
		const int64_t get_timeOffset() const { return m_timeOffset;}
		void set_timeOffset(const int64_t & value) { m_timeOffset = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_gatewayIp; /* string representing InetAddress */
		int64_t	m_leaseTime;
		std::string	m_model_type;
		std::string	m_primaryDns; /* string representing InetAddress */
		int64_t	m_rebindingTime;
		int64_t	m_renewalTime;
		std::string	m_secondaryDns; /* string representing InetAddress */
		std::string	m_subnetMask; /* string representing InetAddress */
		int64_t	m_timeOffset;
};

class SsidConfiguration  : public ProfileDetails {
	public:
		const std::vector<RadioType> get_appliedRadios() const { return m_appliedRadios;}
		void set_appliedRadios(const std::vector<RadioType> & value) { m_appliedRadios = value; }
		const int64_t get_bandwidthLimitDown() const { return m_bandwidthLimitDown;}
		void set_bandwidthLimitDown(const int64_t & value) { m_bandwidthLimitDown = value; }
		const int64_t get_bandwidthLimitUp() const { return m_bandwidthLimitUp;}
		void set_bandwidthLimitUp(const int64_t & value) { m_bandwidthLimitUp = value; }
		const int64_t get_bonjourGatewayProfileId() const { return m_bonjourGatewayProfileId;}
		void set_bonjourGatewayProfileId(const int64_t & value) { m_bonjourGatewayProfileId = value; }
		const StateSetting get_broadcastSsid() const { return m_broadcastSsid;}
		void set_broadcastSsid(const StateSetting & value) { m_broadcastSsid = value; }
		const int64_t get_captivePortalId() const { return m_captivePortalId;}
		void set_captivePortalId(const int64_t & value) { m_captivePortalId = value; }
		const int64_t get_clientBandwidthLimitDown() const { return m_clientBandwidthLimitDown;}
		void set_clientBandwidthLimitDown(const int64_t & value) { m_clientBandwidthLimitDown = value; }
		const int64_t get_clientBandwidthLimitUp() const { return m_clientBandwidthLimitUp;}
		void set_clientBandwidthLimitUp(const int64_t & value) { m_clientBandwidthLimitUp = value; }
		const DynamicVlanMode get_dynamicVlan() const { return m_dynamicVlan;}
		void set_dynamicVlan(const DynamicVlanMode & value) { m_dynamicVlan = value; }
		const bool get_enable80211w() const { return m_enable80211w;}
		void set_enable80211w(const bool & value) { m_enable80211w = value; }
		const NetworkForwardMode get_forwardMode() const { return m_forwardMode;}
		void set_forwardMode(const NetworkForwardMode & value) { m_forwardMode = value; }
		const int64_t get_keyRefresh() const { return m_keyRefresh;}
		void set_keyRefresh(const int64_t & value) { m_keyRefresh = value; }
		const std::string get_keyStr() const { return m_keyStr;}
		void set_keyStr(const std::string & value) { m_keyStr = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const bool get_noLocalSubnets() const { return m_noLocalSubnets;}
		void set_noLocalSubnets(const bool & value) { m_noLocalSubnets = value; }
		const RadioBasedSsidConfigurationMap get_radioBasedConfigs() const { return m_radioBasedConfigs;}
		void set_radioBasedConfigs(const RadioBasedSsidConfigurationMap & value) { m_radioBasedConfigs = value; }
		const int64_t get_radiusAcountingServiceInterval() const { return m_radiusAcountingServiceInterval;}
		void set_radiusAcountingServiceInterval(const int64_t & value) { m_radiusAcountingServiceInterval = value; }
		const RadiusNasConfiguration get_radiusNasConfiguration() const { return m_radiusNasConfiguration;}
		void set_radiusNasConfiguration(const RadiusNasConfiguration & value) { m_radiusNasConfiguration = value; }
		const int64_t get_radiusServiceId() const { return m_radiusServiceId;}
		void set_radiusServiceId(const int64_t & value) { m_radiusServiceId = value; }
		const SsidSecureMode get_secureMode() const { return m_secureMode;}
		void set_secureMode(const SsidSecureMode & value) { m_secureMode = value; }
		const std::string get_ssid() const { return m_ssid;}
		void set_ssid(const std::string & value) { m_ssid = value; }
		const StateSetting get_ssidAdminState() const { return m_ssidAdminState;}
		void set_ssidAdminState(const StateSetting & value) { m_ssidAdminState = value; }
		const bool get_videoTrafficOnly() const { return m_videoTrafficOnly;}
		void set_videoTrafficOnly(const bool & value) { m_videoTrafficOnly = value; }
		const int64_t get_vlanId() const { return m_vlanId;}
		void set_vlanId(const int64_t & value) { m_vlanId = value; }
		const WepConfiguration get_wepConfig() const { return m_wepConfig;}
		void set_wepConfig(const WepConfiguration & value) { m_wepConfig = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<RadioType>	m_appliedRadios;
		int64_t	m_bandwidthLimitDown;
		int64_t	m_bandwidthLimitUp;
		int64_t	m_bonjourGatewayProfileId; /* id of a BonjourGateway profile, must be also added to the children of this profile */
		StateSetting	m_broadcastSsid;
		int64_t	m_captivePortalId; /* id of a CaptivePortalConfiguration profile, must be also added to the children of this profile */
		int64_t	m_clientBandwidthLimitDown;
		int64_t	m_clientBandwidthLimitUp;
		DynamicVlanMode	m_dynamicVlan;
		bool	m_enable80211w;
		NetworkForwardMode	m_forwardMode;
		int64_t	m_keyRefresh;
		std::string	m_keyStr;
		std::string	m_model_type;
		bool	m_noLocalSubnets;
		RadioBasedSsidConfigurationMap	m_radioBasedConfigs;
		int64_t	m_radiusAcountingServiceInterval; /* If this is set (i.e. non-null), RadiusAccountingService is configured, and SsidSecureMode is configured as Enterprise/Radius, ap will send interim accounting updates every N seconds */
		RadiusNasConfiguration	m_radiusNasConfiguration;
		int64_t	m_radiusServiceId;
		SsidSecureMode	m_secureMode;
		std::string	m_ssid;
		StateSetting	m_ssidAdminState;
		bool	m_videoTrafficOnly;
		int64_t	m_vlanId;
		WepConfiguration	m_wepConfig;
};

class NoiseFloorPerRadioDetailsMap  {
	public:
		const NoiseFloorPerRadioDetails get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const NoiseFloorPerRadioDetails & value) { m_is2dot4GHz = value; }
		const NoiseFloorPerRadioDetails get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const NoiseFloorPerRadioDetails & value) { m_is5GHz = value; }
		const NoiseFloorPerRadioDetails get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const NoiseFloorPerRadioDetails & value) { m_is5GHzL = value; }
		const NoiseFloorPerRadioDetails get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const NoiseFloorPerRadioDetails & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		NoiseFloorPerRadioDetails	m_is2dot4GHz;
		NoiseFloorPerRadioDetails	m_is5GHz;
		NoiseFloorPerRadioDetails	m_is5GHzL;
		NoiseFloorPerRadioDetails	m_is5GHzU;
};

class EquipmentLANStatusData  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }
		const VLANStatusDataMap get_vlanStatusDataMap() const { return m_vlanStatusDataMap;}
		void set_vlanStatusDataMap(const VLANStatusDataMap & value) { m_vlanStatusDataMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		std::string	m_statusDataType;
		VLANStatusDataMap	m_vlanStatusDataMap;
};

class LocationDetails  {
	public:
		const CountryCode get_countryCode() const { return m_countryCode;}
		void set_countryCode(const CountryCode & value) { m_countryCode = value; }
		const LocationActivityDetailsMap get_dailyActivityDetails() const { return m_dailyActivityDetails;}
		void set_dailyActivityDetails(const LocationActivityDetailsMap & value) { m_dailyActivityDetails = value; }
		const DaysOfWeekTimeRangeSchedule get_maintenanceWindow() const { return m_maintenanceWindow;}
		void set_maintenanceWindow(const DaysOfWeekTimeRangeSchedule & value) { m_maintenanceWindow = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const bool get_rrmEnabled() const { return m_rrmEnabled;}
		void set_rrmEnabled(const bool & value) { m_rrmEnabled = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		CountryCode	m_countryCode;
		LocationActivityDetailsMap	m_dailyActivityDetails;
		DaysOfWeekTimeRangeSchedule	m_maintenanceWindow;
		std::string	m_model_type;
		bool	m_rrmEnabled;
};

class ChannelUtilizationPerRadioDetailsMap  {
	public:
		const ChannelUtilizationPerRadioDetails get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const ChannelUtilizationPerRadioDetails & value) { m_is2dot4GHz = value; }
		const ChannelUtilizationPerRadioDetails get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const ChannelUtilizationPerRadioDetails & value) { m_is5GHz = value; }
		const ChannelUtilizationPerRadioDetails get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const ChannelUtilizationPerRadioDetails & value) { m_is5GHzL = value; }
		const ChannelUtilizationPerRadioDetails get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const ChannelUtilizationPerRadioDetails & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ChannelUtilizationPerRadioDetails	m_is2dot4GHz;
		ChannelUtilizationPerRadioDetails	m_is5GHz;
		ChannelUtilizationPerRadioDetails	m_is5GHzL;
		ChannelUtilizationPerRadioDetails	m_is5GHzU;
};

class WebTokenResult  {
	public:
		const std::string get_access_token() const { return m_access_token;}
		void set_access_token(const std::string & value) { m_access_token = value; }
		const WebTokenAclTemplate get_aclTemplate() const { return m_aclTemplate;}
		void set_aclTemplate(const WebTokenAclTemplate & value) { m_aclTemplate = value; }
		const int64_t get_expires_in() const { return m_expires_in;}
		void set_expires_in(const int64_t & value) { m_expires_in = value; }
		const std::string get_id_token() const { return m_id_token;}
		void set_id_token(const std::string & value) { m_id_token = value; }
		const int64_t get_idle_timeout() const { return m_idle_timeout;}
		void set_idle_timeout(const int64_t & value) { m_idle_timeout = value; }
		const std::string get_refresh_token() const { return m_refresh_token;}
		void set_refresh_token(const std::string & value) { m_refresh_token = value; }
		const std::string get_token_type() const { return m_token_type;}
		void set_token_type(const std::string & value) { m_token_type = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_access_token;
		WebTokenAclTemplate	m_aclTemplate;
		int64_t	m_expires_in;
		std::string	m_id_token;
		int64_t	m_idle_timeout;
		std::string	m_refresh_token;
		std::string	m_token_type;
};

class RfConfigMap  {
	public:
		const RfElementConfiguration get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const RfElementConfiguration & value) { m_is2dot4GHz = value; }
		const RfElementConfiguration get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const RfElementConfiguration & value) { m_is5GHz = value; }
		const RfElementConfiguration get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const RfElementConfiguration & value) { m_is5GHzL = value; }
		const RfElementConfiguration get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const RfElementConfiguration & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RfElementConfiguration	m_is2dot4GHz;
		RfElementConfiguration	m_is5GHz;
		RfElementConfiguration	m_is5GHzL;
		RfElementConfiguration	m_is5GHzU;
};

class EquipmentPerRadioUtilizationDetailsMap  {
	public:
		const EquipmentPerRadioUtilizationDetails get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const EquipmentPerRadioUtilizationDetails & value) { m_is2dot4GHz = value; }
		const EquipmentPerRadioUtilizationDetails get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const EquipmentPerRadioUtilizationDetails & value) { m_is5GHz = value; }
		const EquipmentPerRadioUtilizationDetails get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const EquipmentPerRadioUtilizationDetails & value) { m_is5GHzL = value; }
		const EquipmentPerRadioUtilizationDetails get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const EquipmentPerRadioUtilizationDetails & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		EquipmentPerRadioUtilizationDetails	m_is2dot4GHz;
		EquipmentPerRadioUtilizationDetails	m_is5GHz;
		EquipmentPerRadioUtilizationDetails	m_is5GHzL;
		EquipmentPerRadioUtilizationDetails	m_is5GHzU;
};

class ClientSessionRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const ClientSession get_payload() const { return m_payload;}
		void set_payload(const ClientSession & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		ClientSession	m_payload;
};

class UserDetails  {
	public:
		const ClientActivityAggregatedStatsPerRadioTypeMap get_clientActivityPerRadio() const { return m_clientActivityPerRadio;}
		void set_clientActivityPerRadio(const ClientActivityAggregatedStatsPerRadioTypeMap & value) { m_clientActivityPerRadio = value; }
		const int64_t get_indicatorValue() const { return m_indicatorValue;}
		void set_indicatorValue(const int64_t & value) { m_indicatorValue = value; }
		const IntegerPerRadioTypeMap get_indicatorValuePerRadio() const { return m_indicatorValuePerRadio;}
		void set_indicatorValuePerRadio(const IntegerPerRadioTypeMap & value) { m_indicatorValuePerRadio = value; }
		const LinkQualityAggregatedStatsPerRadioTypeMap get_linkQualityPerRadio() const { return m_linkQualityPerRadio;}
		void set_linkQualityPerRadio(const LinkQualityAggregatedStatsPerRadioTypeMap & value) { m_linkQualityPerRadio = value; }
		const int64_t get_numBadEquipment() const { return m_numBadEquipment;}
		void set_numBadEquipment(const int64_t & value) { m_numBadEquipment = value; }
		const int64_t get_numGoodEquipment() const { return m_numGoodEquipment;}
		void set_numGoodEquipment(const int64_t & value) { m_numGoodEquipment = value; }
		const int64_t get_numWarnEquipment() const { return m_numWarnEquipment;}
		void set_numWarnEquipment(const int64_t & value) { m_numWarnEquipment = value; }
		const int64_t get_totalApsReported() const { return m_totalApsReported;}
		void set_totalApsReported(const int64_t & value) { m_totalApsReported = value; }
		const MinMaxAvgValueInt get_totalUsers() const { return m_totalUsers;}
		void set_totalUsers(const MinMaxAvgValueInt & value) { m_totalUsers = value; }
		const IntegerValueMap get_userDevicePerManufacturerCounts() const { return m_userDevicePerManufacturerCounts;}
		void set_userDevicePerManufacturerCounts(const IntegerValueMap & value) { m_userDevicePerManufacturerCounts = value; }
		const MinMaxAvgValueIntPerRadioMap get_usersPerRadio() const { return m_usersPerRadio;}
		void set_usersPerRadio(const MinMaxAvgValueIntPerRadioMap & value) { m_usersPerRadio = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ClientActivityAggregatedStatsPerRadioTypeMap	m_clientActivityPerRadio;
		int64_t	m_indicatorValue;
		IntegerPerRadioTypeMap	m_indicatorValuePerRadio;
		LinkQualityAggregatedStatsPerRadioTypeMap	m_linkQualityPerRadio;
		int64_t	m_numBadEquipment;
		int64_t	m_numGoodEquipment;
		int64_t	m_numWarnEquipment;
		int64_t	m_totalApsReported;
		MinMaxAvgValueInt	m_totalUsers;
		IntegerValueMap	m_userDevicePerManufacturerCounts;
		MinMaxAvgValueIntPerRadioMap	m_usersPerRadio;
};

class AlarmChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Alarm get_payload() const { return m_payload;}
		void set_payload(const Alarm & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Alarm	m_payload;
};

class RfConfiguration  : public ProfileDetails {
	public:
		const RfConfigMap get_rfConfigMap() const { return m_rfConfigMap;}
		void set_rfConfigMap(const RfConfigMap & value) { m_rfConfigMap = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RfConfigMap	m_rfConfigMap;
};

class EquipmentRrmBulkUpdateItem  {
	public:
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const EquipmentRrmBulkUpdateItemPerRadioMap get_perRadioDetails() const { return m_perRadioDetails;}
		void set_perRadioDetails(const EquipmentRrmBulkUpdateItemPerRadioMap & value) { m_perRadioDetails = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_equipmentId;
		EquipmentRrmBulkUpdateItemPerRadioMap	m_perRadioDetails;
};

class ClientSessionChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const ClientSession get_payload() const { return m_payload;}
		void set_payload(const ClientSession & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		ClientSession	m_payload;
};

class Location  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const LocationDetails get_details() const { return m_details;}
		void set_details(const LocationDetails & value) { m_details = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const std::string get_locationType() const { return m_locationType;}
		void set_locationType(const std::string & value) { m_locationType = value; }
		const std::string get_name() const { return m_name;}
		void set_name(const std::string & value) { m_name = value; }
		const int64_t get_parentId() const { return m_parentId;}
		void set_parentId(const int64_t & value) { m_parentId = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		LocationDetails	m_details;
		int64_t	m_id;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		std::string	m_locationType; /* COUNTRY, SITE, BUILDING, FLOOR, UNSUPPORTED,  */
		std::string	m_name;
		int64_t	m_parentId;
};

class Customer  {
	public:
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const CustomerDetails get_details() const { return m_details;}
		void set_details(const CustomerDetails & value) { m_details = value; }
		const std::string get_email() const { return m_email;}
		void set_email(const std::string & value) { m_email = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const std::string get_name() const { return m_name;}
		void set_name(const std::string & value) { m_name = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_createdTimestamp;
		CustomerDetails	m_details;
		std::string	m_email;
		int64_t	m_id;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		std::string	m_name;
};

class ClientChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Client get_payload() const { return m_payload;}
		void set_payload(const Client & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Client	m_payload;
};

class ApNodeMetrics  {
	public:
		const ApPerformance get_apPerformance() const { return m_apPerformance;}
		void set_apPerformance(const ApPerformance & value) { m_apPerformance = value; }
		const IntegerPerRadioTypeMap get_channelUtilizationPerRadio() const { return m_channelUtilizationPerRadio;}
		void set_channelUtilizationPerRadio(const IntegerPerRadioTypeMap & value) { m_channelUtilizationPerRadio = value; }
		const ListOfMacsPerRadioMap get_clientMacAddressesPerRadio() const { return m_clientMacAddressesPerRadio;}
		void set_clientMacAddressesPerRadio(const ListOfMacsPerRadioMap & value) { m_clientMacAddressesPerRadio = value; }
		const int64_t get_cloudLinkAvailability() const { return m_cloudLinkAvailability;}
		void set_cloudLinkAvailability(const int64_t & value) { m_cloudLinkAvailability = value; }
		const int64_t get_cloudLinkLatencyInMs() const { return m_cloudLinkLatencyInMs;}
		void set_cloudLinkLatencyInMs(const int64_t & value) { m_cloudLinkLatencyInMs = value; }
		const ListOfMcsStatsPerRadioMap get_mcsStatsPerRadio() const { return m_mcsStatsPerRadio;}
		void set_mcsStatsPerRadio(const ListOfMcsStatsPerRadioMap & value) { m_mcsStatsPerRadio = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::vector<NetworkProbeMetrics> get_networkProbeMetrics() const { return m_networkProbeMetrics;}
		void set_networkProbeMetrics(const std::vector<NetworkProbeMetrics> & value) { m_networkProbeMetrics = value; }
		const IntegerPerRadioTypeMap get_noiseFloorPerRadio() const { return m_noiseFloorPerRadio;}
		void set_noiseFloorPerRadio(const IntegerPerRadioTypeMap & value) { m_noiseFloorPerRadio = value; }
		const int64_t get_periodLengthSec() const { return m_periodLengthSec;}
		void set_periodLengthSec(const int64_t & value) { m_periodLengthSec = value; }
		const RadioStatisticsPerRadioMap get_radioStatsPerRadio() const { return m_radioStatsPerRadio;}
		void set_radioStatsPerRadio(const RadioStatisticsPerRadioMap & value) { m_radioStatsPerRadio = value; }
		const ListOfRadioUtilizationPerRadioMap get_radioUtilizationPerRadio() const { return m_radioUtilizationPerRadio;}
		void set_radioUtilizationPerRadio(const ListOfRadioUtilizationPerRadioMap & value) { m_radioUtilizationPerRadio = value; }
		const std::vector<RadiusMetrics> get_radiusMetrics() const { return m_radiusMetrics;}
		void set_radiusMetrics(const std::vector<RadiusMetrics> & value) { m_radiusMetrics = value; }
		const LongPerRadioTypeMap get_rxBytesPerRadio() const { return m_rxBytesPerRadio;}
		void set_rxBytesPerRadio(const LongPerRadioTypeMap & value) { m_rxBytesPerRadio = value; }
		const std::vector<TunnelMetricData> get_tunnelMetrics() const { return m_tunnelMetrics;}
		void set_tunnelMetrics(const std::vector<TunnelMetricData> & value) { m_tunnelMetrics = value; }
		const LongPerRadioTypeMap get_txBytesPerRadio() const { return m_txBytesPerRadio;}
		void set_txBytesPerRadio(const LongPerRadioTypeMap & value) { m_txBytesPerRadio = value; }
		const std::vector<VlanSubnet> get_vlanSubnet() const { return m_vlanSubnet;}
		void set_vlanSubnet(const std::vector<VlanSubnet> & value) { m_vlanSubnet = value; }
		const MapOfWmmQueueStatsPerRadioMap get_wmmQueuesPerRadio() const { return m_wmmQueuesPerRadio;}
		void set_wmmQueuesPerRadio(const MapOfWmmQueueStatsPerRadioMap & value) { m_wmmQueuesPerRadio = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		ApPerformance	m_apPerformance;
		IntegerPerRadioTypeMap	m_channelUtilizationPerRadio;
		ListOfMacsPerRadioMap	m_clientMacAddressesPerRadio;
		int64_t	m_cloudLinkAvailability;
		int64_t	m_cloudLinkLatencyInMs;
		ListOfMcsStatsPerRadioMap	m_mcsStatsPerRadio;
		std::string	m_model_type;
		std::vector<NetworkProbeMetrics>	m_networkProbeMetrics;
		IntegerPerRadioTypeMap	m_noiseFloorPerRadio;
		int64_t	m_periodLengthSec; /* How many seconds the AP measured for the metric */
		RadioStatisticsPerRadioMap	m_radioStatsPerRadio;
		ListOfRadioUtilizationPerRadioMap	m_radioUtilizationPerRadio;
		std::vector<RadiusMetrics>	m_radiusMetrics;
		LongPerRadioTypeMap	m_rxBytesPerRadio;
		std::vector<TunnelMetricData>	m_tunnelMetrics;
		LongPerRadioTypeMap	m_txBytesPerRadio;
		std::vector<VlanSubnet>	m_vlanSubnet;
		MapOfWmmQueueStatsPerRadioMap	m_wmmQueuesPerRadio;
};

class AlarmRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Alarm get_payload() const { return m_payload;}
		void set_payload(const Alarm & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Alarm	m_payload;
};

class AlarmAddedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Alarm get_payload() const { return m_payload;}
		void set_payload(const Alarm & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Alarm	m_payload;
};

class NoiseFloorDetails  {
	public:
		const int64_t get_indicatorValue() const { return m_indicatorValue;}
		void set_indicatorValue(const int64_t & value) { m_indicatorValue = value; }
		const NoiseFloorPerRadioDetailsMap get_perRadioDetails() const { return m_perRadioDetails;}
		void set_perRadioDetails(const NoiseFloorPerRadioDetailsMap & value) { m_perRadioDetails = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_indicatorValue;
		NoiseFloorPerRadioDetailsMap	m_perRadioDetails;
};

class PaginationResponseAlarm  {
	public:
		const PaginationContextAlarm get_context() const { return m_context;}
		void set_context(const PaginationContextAlarm & value) { m_context = value; }
		const std::vector<Alarm> get_items() const { return m_items;}
		void set_items(const std::vector<Alarm> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextAlarm	m_context;
		std::vector<Alarm>	m_items;
};

class ListOfSsidStatisticsPerRadioMap  {
	public:
		const std::vector<SsidStatistics> get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const std::vector<SsidStatistics> & value) { m_is2dot4GHz = value; }
		const std::vector<SsidStatistics> get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const std::vector<SsidStatistics> & value) { m_is5GHz = value; }
		const std::vector<SsidStatistics> get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const std::vector<SsidStatistics> & value) { m_is5GHzL = value; }
		const std::vector<SsidStatistics> get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const std::vector<SsidStatistics> & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<SsidStatistics>	m_is2dot4GHz;
		std::vector<SsidStatistics>	m_is5GHz;
		std::vector<SsidStatistics>	m_is5GHzL;
		std::vector<SsidStatistics>	m_is5GHzU;
};

class CapacityDetails  {
	public:
		const CapacityPerRadioDetailsMap get_perRadioDetails() const { return m_perRadioDetails;}
		void set_perRadioDetails(const CapacityPerRadioDetailsMap & value) { m_perRadioDetails = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		CapacityPerRadioDetailsMap	m_perRadioDetails;
};

class ClientAddedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Client get_payload() const { return m_payload;}
		void set_payload(const Client & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Client	m_payload;
};

class ChannelUtilizationDetails  {
	public:
		const int64_t get_indicatorValue() const { return m_indicatorValue;}
		void set_indicatorValue(const int64_t & value) { m_indicatorValue = value; }
		const ChannelUtilizationPerRadioDetailsMap get_perRadioDetails() const { return m_perRadioDetails;}
		void set_perRadioDetails(const ChannelUtilizationPerRadioDetailsMap & value) { m_perRadioDetails = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_indicatorValue;
		ChannelUtilizationPerRadioDetailsMap	m_perRadioDetails;
};

class ClientRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Client get_payload() const { return m_payload;}
		void set_payload(const Client & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Client	m_payload;
};

class RadioUtilizationReport  {
	public:
		const IntegerPerRadioTypeMap get_avgNoiseFloor() const { return m_avgNoiseFloor;}
		void set_avgNoiseFloor(const IntegerPerRadioTypeMap & value) { m_avgNoiseFloor = value; }
		const EquipmentCapacityDetailsMap get_capacityDetails() const { return m_capacityDetails;}
		void set_capacityDetails(const EquipmentCapacityDetailsMap & value) { m_capacityDetails = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const EquipmentPerRadioUtilizationDetailsMap get_radioUtilization() const { return m_radioUtilization;}
		void set_radioUtilization(const EquipmentPerRadioUtilizationDetailsMap & value) { m_radioUtilization = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		IntegerPerRadioTypeMap	m_avgNoiseFloor;
		EquipmentCapacityDetailsMap	m_capacityDetails;
		std::string	m_model_type;
		EquipmentPerRadioUtilizationDetailsMap	m_radioUtilization;
		std::string	m_statusDataType;
};

class AdvancedRadioMap  {
	public:
		const RadioConfiguration get_is2dot4GHz() const { return m_is2dot4GHz;}
		void set_is2dot4GHz(const RadioConfiguration & value) { m_is2dot4GHz = value; }
		const RadioConfiguration get_is5GHz() const { return m_is5GHz;}
		void set_is5GHz(const RadioConfiguration & value) { m_is5GHz = value; }
		const RadioConfiguration get_is5GHzL() const { return m_is5GHzL;}
		void set_is5GHzL(const RadioConfiguration & value) { m_is5GHzL = value; }
		const RadioConfiguration get_is5GHzU() const { return m_is5GHzU;}
		void set_is5GHzU(const RadioConfiguration & value) { m_is5GHzU = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		RadioConfiguration	m_is2dot4GHz;
		RadioConfiguration	m_is5GHz;
		RadioConfiguration	m_is5GHzL;
		RadioConfiguration	m_is5GHzU;
};

class PaginationResponseClientSession  {
	public:
		const PaginationContextClientSession get_context() const { return m_context;}
		void set_context(const PaginationContextClientSession & value) { m_context = value; }
		const std::vector<ClientSession> get_items() const { return m_items;}
		void set_items(const std::vector<ClientSession> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextClientSession	m_context;
		std::vector<ClientSession>	m_items;
};

class PaginationResponseClient  {
	public:
		const PaginationContextClient get_context() const { return m_context;}
		void set_context(const PaginationContextClient & value) { m_context = value; }
		const std::vector<Client> get_items() const { return m_items;}
		void set_items(const std::vector<Client> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextClient	m_context;
		std::vector<Client>	m_items;
};

class ApElementConfiguration  {
	public:
		const AdvancedRadioMap get_advancedRadioMap() const { return m_advancedRadioMap;}
		void set_advancedRadioMap(const AdvancedRadioMap & value) { m_advancedRadioMap = value; }
		const AntennaType get_antennaType() const { return m_antennaType;}
		void set_antennaType(const AntennaType & value) { m_antennaType = value; }
		const bool get_costSavingEventsEnabled() const { return m_costSavingEventsEnabled;}
		void set_costSavingEventsEnabled(const bool & value) { m_costSavingEventsEnabled = value; }
		const DeploymentType get_deploymentType() const { return m_deploymentType;}
		void set_deploymentType(const DeploymentType & value) { m_deploymentType = value; }
		const DeviceMode get_deviceMode() const { return m_deviceMode;}
		void set_deviceMode(const DeviceMode & value) { m_deviceMode = value; }
		const std::string get_deviceName() const { return m_deviceName;}
		void set_deviceName(const std::string & value) { m_deviceName = value; }
		const std::string get_elementConfigVersion() const { return m_elementConfigVersion;}
		void set_elementConfigVersion(const std::string & value) { m_elementConfigVersion = value; }
		const EquipmentType get_equipmentType() const { return m_equipmentType;}
		void set_equipmentType(const EquipmentType & value) { m_equipmentType = value; }
		const NetworkForwardMode get_forwardMode() const { return m_forwardMode;}
		void set_forwardMode(const NetworkForwardMode & value) { m_forwardMode = value; }
		const bool get_frameReportThrottleEnabled() const { return m_frameReportThrottleEnabled;}
		void set_frameReportThrottleEnabled(const bool & value) { m_frameReportThrottleEnabled = value; }
		const std::string get_gettingDNS() const { return m_gettingDNS;}
		void set_gettingDNS(const std::string & value) { m_gettingDNS = value; }
		const std::string get_gettingIP() const { return m_gettingIP;}
		void set_gettingIP(const std::string & value) { m_gettingIP = value; }
		const bool get_locallyConfigured() const { return m_locallyConfigured;}
		void set_locallyConfigured(const bool & value) { m_locallyConfigured = value; }
		const int64_t get_locallyConfiguredMgmtVlan() const { return m_locallyConfiguredMgmtVlan;}
		void set_locallyConfiguredMgmtVlan(const int64_t & value) { m_locallyConfiguredMgmtVlan = value; }
		const std::string get_locationData() const { return m_locationData;}
		void set_locationData(const std::string & value) { m_locationData = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const std::vector<PeerInfo> get_peerInfoList() const { return m_peerInfoList;}
		void set_peerInfoList(const std::vector<PeerInfo> & value) { m_peerInfoList = value; }
		const RadioMap get_radioMap() const { return m_radioMap;}
		void set_radioMap(const RadioMap & value) { m_radioMap = value; }
		const std::string get_staticDnsIp1() const { return m_staticDnsIp1;}
		void set_staticDnsIp1(const std::string & value) { m_staticDnsIp1 = value; }
		const std::string get_staticDnsIp2() const { return m_staticDnsIp2;}
		void set_staticDnsIp2(const std::string & value) { m_staticDnsIp2 = value; }
		const std::string get_staticIP() const { return m_staticIP;}
		void set_staticIP(const std::string & value) { m_staticIP = value; }
		const std::string get_staticIpGw() const { return m_staticIpGw;}
		void set_staticIpGw(const std::string & value) { m_staticIpGw = value; }
		const int64_t get_staticIpMaskCidr() const { return m_staticIpMaskCidr;}
		void set_staticIpMaskCidr(const int64_t & value) { m_staticIpMaskCidr = value; }
		const bool get_syntheticClientEnabled() const { return m_syntheticClientEnabled;}
		void set_syntheticClientEnabled(const bool & value) { m_syntheticClientEnabled = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		AdvancedRadioMap	m_advancedRadioMap;
		AntennaType	m_antennaType;
		bool	m_costSavingEventsEnabled;
		DeploymentType	m_deploymentType;
		DeviceMode	m_deviceMode;
		std::string	m_deviceName;
		std::string	m_elementConfigVersion;
		EquipmentType	m_equipmentType;
		NetworkForwardMode	m_forwardMode;
		bool	m_frameReportThrottleEnabled;
		std::string	m_gettingDNS; /* dhcp, manual,  */
		std::string	m_gettingIP; /* dhcp, manual,  */
		bool	m_locallyConfigured;
		int64_t	m_locallyConfiguredMgmtVlan;
		std::string	m_locationData;
		std::string	m_model_type;
		std::vector<PeerInfo>	m_peerInfoList;
		RadioMap	m_radioMap;
		std::string	m_staticDnsIp1;
		std::string	m_staticDnsIp2;
		std::string	m_staticIP;
		std::string	m_staticIpGw;
		int64_t	m_staticIpMaskCidr;
		bool	m_syntheticClientEnabled;
};

class ApSsidMetrics  {
	public:
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const ListOfSsidStatisticsPerRadioMap get_ssidStats() const { return m_ssidStats;}
		void set_ssidStats(const ListOfSsidStatisticsPerRadioMap & value) { m_ssidStats = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_model_type;
		ListOfSsidStatisticsPerRadioMap	m_ssidStats;
};

class CustomerAddedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Customer get_payload() const { return m_payload;}
		void set_payload(const Customer & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Customer	m_payload;
};

class CustomerChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Customer get_payload() const { return m_payload;}
		void set_payload(const Customer & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Customer	m_payload;
};

class CustomerRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Customer get_payload() const { return m_payload;}
		void set_payload(const Customer & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Customer	m_payload;
};

class EquipmentRrmBulkUpdateRequest  {
	public:
		const std::vector<EquipmentRrmBulkUpdateItem> get_items() const { return m_items;}
		void set_items(const std::vector<EquipmentRrmBulkUpdateItem> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::vector<EquipmentRrmBulkUpdateItem>	m_items;
};

class LocationAddedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Location get_payload() const { return m_payload;}
		void set_payload(const Location & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Location	m_payload;
};

class LocationChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Location get_payload() const { return m_payload;}
		void set_payload(const Location & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Location	m_payload;
};

class LocationRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Location get_payload() const { return m_payload;}
		void set_payload(const Location & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Location	m_payload;
};

class NetworkAggregateStatusData  {
	public:
		const int64_t get_beginAggregationTsMs() const { return m_beginAggregationTsMs;}
		void set_beginAggregationTsMs(const int64_t & value) { m_beginAggregationTsMs = value; }
		const int64_t get_beginGenerationTsMs() const { return m_beginGenerationTsMs;}
		void set_beginGenerationTsMs(const int64_t & value) { m_beginGenerationTsMs = value; }
		const int64_t get_behavior() const { return m_behavior;}
		void set_behavior(const int64_t & value) { m_behavior = value; }
		const CapacityDetails get_capacityDetails() const { return m_capacityDetails;}
		void set_capacityDetails(const CapacityDetails & value) { m_capacityDetails = value; }
		const ChannelUtilizationDetails get_channelUtilizationDetails() const { return m_channelUtilizationDetails;}
		void set_channelUtilizationDetails(const ChannelUtilizationDetails & value) { m_channelUtilizationDetails = value; }
		const CommonProbeDetails get_cloudLinkDetails() const { return m_cloudLinkDetails;}
		void set_cloudLinkDetails(const CommonProbeDetails & value) { m_cloudLinkDetails = value; }
		const int64_t get_coverage() const { return m_coverage;}
		void set_coverage(const int64_t & value) { m_coverage = value; }
		const CommonProbeDetails get_dhcpDetails() const { return m_dhcpDetails;}
		void set_dhcpDetails(const CommonProbeDetails & value) { m_dhcpDetails = value; }
		const CommonProbeDetails get_dnsDetails() const { return m_dnsDetails;}
		void set_dnsDetails(const CommonProbeDetails & value) { m_dnsDetails = value; }
		const int64_t get_endAggregationTsMs() const { return m_endAggregationTsMs;}
		void set_endAggregationTsMs(const int64_t & value) { m_endAggregationTsMs = value; }
		const int64_t get_endGenerationTsMs() const { return m_endGenerationTsMs;}
		void set_endGenerationTsMs(const int64_t & value) { m_endGenerationTsMs = value; }
		const EquipmentPerformanceDetails get_equipmentPerformanceDetails() const { return m_equipmentPerformanceDetails;}
		void set_equipmentPerformanceDetails(const EquipmentPerformanceDetails & value) { m_equipmentPerformanceDetails = value; }
		const int64_t get_handoff() const { return m_handoff;}
		void set_handoff(const int64_t & value) { m_handoff = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const NoiseFloorDetails get_noiseFloorDetails() const { return m_noiseFloorDetails;}
		void set_noiseFloorDetails(const NoiseFloorDetails & value) { m_noiseFloorDetails = value; }
		const int64_t get_numMetricsAggregated() const { return m_numMetricsAggregated;}
		void set_numMetricsAggregated(const int64_t & value) { m_numMetricsAggregated = value; }
		const int64_t get_numberOfReportingEquipment() const { return m_numberOfReportingEquipment;}
		void set_numberOfReportingEquipment(const int64_t & value) { m_numberOfReportingEquipment = value; }
		const int64_t get_numberOfTotalEquipment() const { return m_numberOfTotalEquipment;}
		void set_numberOfTotalEquipment(const int64_t & value) { m_numberOfTotalEquipment = value; }
		const RadioUtilizationDetails get_radioUtilizationDetails() const { return m_radioUtilizationDetails;}
		void set_radioUtilizationDetails(const RadioUtilizationDetails & value) { m_radioUtilizationDetails = value; }
		const RadiusDetails get_radiusDetails() const { return m_radiusDetails;}
		void set_radiusDetails(const RadiusDetails & value) { m_radiusDetails = value; }
		const std::string get_statusDataType() const { return m_statusDataType;}
		void set_statusDataType(const std::string & value) { m_statusDataType = value; }
		const TrafficDetails get_trafficDetails() const { return m_trafficDetails;}
		void set_trafficDetails(const TrafficDetails & value) { m_trafficDetails = value; }
		const UserDetails get_userDetails() const { return m_userDetails;}
		void set_userDetails(const UserDetails & value) { m_userDetails = value; }
		const int64_t get_wlanLatency() const { return m_wlanLatency;}
		void set_wlanLatency(const int64_t & value) { m_wlanLatency = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_beginAggregationTsMs;
		int64_t	m_beginGenerationTsMs;
		int64_t	m_behavior;
		CapacityDetails	m_capacityDetails;
		ChannelUtilizationDetails	m_channelUtilizationDetails;
		CommonProbeDetails	m_cloudLinkDetails;
		int64_t	m_coverage;
		CommonProbeDetails	m_dhcpDetails;
		CommonProbeDetails	m_dnsDetails;
		int64_t	m_endAggregationTsMs;
		int64_t	m_endGenerationTsMs;
		EquipmentPerformanceDetails	m_equipmentPerformanceDetails;
		int64_t	m_handoff;
		std::string	m_model_type;
		NoiseFloorDetails	m_noiseFloorDetails;
		int64_t	m_numMetricsAggregated;
		int64_t	m_numberOfReportingEquipment;
		int64_t	m_numberOfTotalEquipment;
		RadioUtilizationDetails	m_radioUtilizationDetails;
		RadiusDetails	m_radiusDetails;
		std::string	m_statusDataType;
		TrafficDetails	m_trafficDetails;
		UserDetails	m_userDetails;
		int64_t	m_wlanLatency;
};

class PaginationResponseLocation  {
	public:
		const PaginationContextLocation get_context() const { return m_context;}
		void set_context(const PaginationContextLocation & value) { m_context = value; }
		const std::vector<Location> get_items() const { return m_items;}
		void set_items(const std::vector<Location> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextLocation	m_context;
		std::vector<Location>	m_items;
};

class EquipmentDetails  {
	public:
		const std::string get_equipmentModel() const { return m_equipmentModel;}
		void set_equipmentModel(const std::string & value) { m_equipmentModel = value; }
		const std::variant<std::monostate,ApElementConfiguration> get_EquipmentDetails() const { return EquipmentDetails_variant;}
		void set_EquipmentDetails(const std::variant<std::monostate,ApElementConfiguration> & value) { EquipmentDetails_variant = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		std::string	m_equipmentModel;
		std::variant<std::monostate,ApElementConfiguration>	EquipmentDetails_variant; /* Place to put extended equipment properties. */
};

class Equipment  {
	public:
		const MacAddress get_baseMacAddress() const { return m_baseMacAddress;}
		void set_baseMacAddress(const MacAddress & value) { m_baseMacAddress = value; }
		const int64_t get_createdTimestamp() const { return m_createdTimestamp;}
		void set_createdTimestamp(const int64_t & value) { m_createdTimestamp = value; }
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const EquipmentDetails get_details() const { return m_details;}
		void set_details(const EquipmentDetails & value) { m_details = value; }
		const EquipmentType get_equipmentType() const { return m_equipmentType;}
		void set_equipmentType(const EquipmentType & value) { m_equipmentType = value; }
		const int64_t get_id() const { return m_id;}
		void set_id(const int64_t & value) { m_id = value; }
		const std::string get_inventoryId() const { return m_inventoryId;}
		void set_inventoryId(const std::string & value) { m_inventoryId = value; }
		const int64_t get_lastModifiedTimestamp() const { return m_lastModifiedTimestamp;}
		void set_lastModifiedTimestamp(const int64_t & value) { m_lastModifiedTimestamp = value; }
		const std::string get_latitude() const { return m_latitude;}
		void set_latitude(const std::string & value) { m_latitude = value; }
		const int64_t get_locationId() const { return m_locationId;}
		void set_locationId(const int64_t & value) { m_locationId = value; }
		const std::string get_longitude() const { return m_longitude;}
		void set_longitude(const std::string & value) { m_longitude = value; }
		const std::string get_name() const { return m_name;}
		void set_name(const std::string & value) { m_name = value; }
		const int64_t get_profileId() const { return m_profileId;}
		void set_profileId(const int64_t & value) { m_profileId = value; }
		const std::string get_serial() const { return m_serial;}
		void set_serial(const std::string & value) { m_serial = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		MacAddress	m_baseMacAddress;
		int64_t	m_createdTimestamp;
		int64_t	m_customerId;
		EquipmentDetails	m_details;
		EquipmentType	m_equipmentType;
		int64_t	m_id;
		std::string	m_inventoryId;
		int64_t	m_lastModifiedTimestamp; /* must be provided for update operation, update will be rejected if provided value does not match the one currently stored in the database */
		std::string	m_latitude;
		int64_t	m_locationId;
		std::string	m_longitude;
		std::string	m_name;
		int64_t	m_profileId;
		std::string	m_serial;
};

class EquipmentAddedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Equipment get_payload() const { return m_payload;}
		void set_payload(const Equipment & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Equipment	m_payload;
};

class EquipmentChangedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Equipment get_payload() const { return m_payload;}
		void set_payload(const Equipment & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Equipment	m_payload;
};

class EquipmentRemovedEvent  {
	public:
		const int64_t get_customerId() const { return m_customerId;}
		void set_customerId(const int64_t & value) { m_customerId = value; }
		const int64_t get_equipmentId() const { return m_equipmentId;}
		void set_equipmentId(const int64_t & value) { m_equipmentId = value; }
		const int64_t get_eventTimestamp() const { return m_eventTimestamp;}
		void set_eventTimestamp(const int64_t & value) { m_eventTimestamp = value; }
		const std::string get_model_type() const { return m_model_type;}
		void set_model_type(const std::string & value) { m_model_type = value; }
		const Equipment get_payload() const { return m_payload;}
		void set_payload(const Equipment & value) { m_payload = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		int64_t	m_customerId;
		int64_t	m_equipmentId;
		int64_t	m_eventTimestamp;
		std::string	m_model_type;
		Equipment	m_payload;
};

class PaginationResponseEquipment  {
	public:
		const PaginationContextEquipment get_context() const { return m_context;}
		void set_context(const PaginationContextEquipment & value) { m_context = value; }
		const std::vector<Equipment> get_items() const { return m_items;}
		void set_items(const std::vector<Equipment> & value) { m_items = value; }

		// JSON utility function to convert object
		bool to_JSON(Poco::JSON::Object &obj) const;
		bool from_stream(std::istream &response);
		bool from_object(Poco::JSON::Object::Ptr Obj);


	private:
		PaginationContextEquipment	m_context;
		std::vector<Equipment>	m_items;
};

}; /* end namespace TIP::PORTAL */
