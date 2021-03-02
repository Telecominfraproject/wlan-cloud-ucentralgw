
// Version: 1.0.0
// Title: CloudSDK Portal API
// Description: APIs that provide services for provisioning, monitoring, and history retrieval of various data elements of CloudSDK.
// Terms of service: undefined
// Contact: undefined

#include "portal-services-openapi.h"

namespace TIP::PORTAL {

	bool NeighbourScanReports::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool NeighbourScanReports::from_stream(std::istream &response) { return true; };
	bool NeighbourScanReports::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointOsuIcon::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointOsuIcon::from_stream(std::istream &response) { return true; };
	bool PasspointOsuIcon::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientEapDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientEapDetails::from_stream(std::istream &response) { return true; };
	bool ClientEapDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool GatewayRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool GatewayRemovedEvent::from_stream(std::istream &response) { return true; };
	bool GatewayRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ProfileDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ProfileDetails::from_stream(std::istream &response) { return true; };
	bool ProfileDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientSessionRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientSessionRemovedEvent::from_stream(std::istream &response) { return true; };
	bool ClientSessionRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RealTimeSipCallEventWithStats::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RealTimeSipCallEventWithStats::from_stream(std::istream &response) { return true; };
	bool RealTimeSipCallEventWithStats::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool WmmQueueStatsPerQueueTypeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool WmmQueueStatsPerQueueTypeMap::from_stream(std::istream &response) { return true; };
	bool WmmQueueStatsPerQueueTypeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientIdEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientIdEvent::from_stream(std::istream &response) { return true; };
	bool ClientIdEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentCapacityDetailsMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentCapacityDetailsMap::from_stream(std::istream &response) { return true; };
	bool EquipmentCapacityDetailsMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RoutingChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RoutingChangedEvent::from_stream(std::istream &response) { return true; };
	bool RoutingChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DhcpDiscoverEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DhcpDiscoverEvent::from_stream(std::istream &response) { return true; };
	bool DhcpDiscoverEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ProfileRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ProfileRemovedEvent::from_stream(std::istream &response) { return true; };
	bool ProfileRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LinkQualityAggregatedStats::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LinkQualityAggregatedStats::from_stream(std::istream &response) { return true; };
	bool LinkQualityAggregatedStats::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioBasedSsidConfigurationMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioBasedSsidConfigurationMap::from_stream(std::istream &response) { return true; };
	bool RadioBasedSsidConfigurationMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ActiveScanSettings::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ActiveScanSettings::from_stream(std::istream &response) { return true; };
	bool ActiveScanSettings::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SourceSelectionManagement::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SourceSelectionManagement::from_stream(std::istream &response) { return true; };
	bool SourceSelectionManagement::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AclTemplate::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AclTemplate::from_stream(std::istream &response) { return true; };
	bool AclTemplate::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LocationAddedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LocationAddedEvent::from_stream(std::istream &response) { return true; };
	bool LocationAddedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LocalTimeValue::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LocalTimeValue::from_stream(std::istream &response) { return true; };
	bool LocalTimeValue::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsClient::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsClient::from_stream(std::istream &response) { return true; };
	bool SortColumnsClient::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool IntegerValueMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool IntegerValueMap::from_stream(std::istream &response) { return true; };
	bool IntegerValueMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool WepConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool WepConfiguration::from_stream(std::istream &response) { return true; };
	bool WepConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool OperatingSystemPerformance::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool OperatingSystemPerformance::from_stream(std::istream &response) { return true; };
	bool OperatingSystemPerformance::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool UserDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool UserDetails::from_stream(std::istream &response) { return true; };
	bool UserDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool MinMaxAvgValueIntPerRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool MinMaxAvgValueIntPerRadioMap::from_stream(std::istream &response) { return true; };
	bool MinMaxAvgValueIntPerRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AlarmChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AlarmChangedEvent::from_stream(std::istream &response) { return true; };
	bool AlarmChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientInfoDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientInfoDetails::from_stream(std::istream &response) { return true; };
	bool ClientInfoDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool GatewayAddedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool GatewayAddedEvent::from_stream(std::istream &response) { return true; };
	bool GatewayAddedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioUtilizationPerRadioDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioUtilizationPerRadioDetails::from_stream(std::istream &response) { return true; };
	bool RadioUtilizationPerRadioDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentPerRadioUtilizationDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentPerRadioUtilizationDetails::from_stream(std::istream &response) { return true; };
	bool EquipmentPerRadioUtilizationDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ListOfRadioUtilizationPerRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ListOfRadioUtilizationPerRadioMap::from_stream(std::istream &response) { return true; };
	bool ListOfRadioUtilizationPerRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DnsProbeMetric::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DnsProbeMetric::from_stream(std::istream &response) { return true; };
	bool DnsProbeMetric::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentDetails::from_stream(std::istream &response) { return true; };
	bool EquipmentDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientAssocEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientAssocEvent::from_stream(std::istream &response) { return true; };
	bool ClientAssocEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ListOfMacsPerRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ListOfMacsPerRadioMap::from_stream(std::istream &response) { return true; };
	bool ListOfMacsPerRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CustomerAddedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CustomerAddedEvent::from_stream(std::istream &response) { return true; };
	bool CustomerAddedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RfConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RfConfiguration::from_stream(std::istream &response) { return true; };
	bool RfConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RoutingAddedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RoutingAddedEvent::from_stream(std::istream &response) { return true; };
	bool RoutingAddedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RealtimeChannelHopEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RealtimeChannelHopEvent::from_stream(std::istream &response) { return true; };
	bool RealtimeChannelHopEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CapacityPerRadioDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CapacityPerRadioDetails::from_stream(std::istream &response) { return true; };
	bool CapacityPerRadioDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool FirmwareVersion::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool FirmwareVersion::from_stream(std::istream &response) { return true; };
	bool FirmwareVersion::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool NeighbourReport::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool NeighbourReport::from_stream(std::istream &response) { return true; };
	bool NeighbourReport::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointMccMnc::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointMccMnc::from_stream(std::istream &response) { return true; };
	bool PasspointMccMnc::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentRrmBulkUpdateItem::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentRrmBulkUpdateItem::from_stream(std::istream &response) { return true; };
	bool EquipmentRrmBulkUpdateItem::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientMetrics::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientMetrics::from_stream(std::istream &response) { return true; };
	bool ClientMetrics::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioConfiguration::from_stream(std::istream &response) { return true; };
	bool RadioConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentGatewayRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentGatewayRecord::from_stream(std::istream &response) { return true; };
	bool EquipmentGatewayRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool VLANStatusData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool VLANStatusData::from_stream(std::istream &response) { return true; };
	bool VLANStatusData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EmptySchedule::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EmptySchedule::from_stream(std::istream &response) { return true; };
	bool EmptySchedule::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentProtocolStatusData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentProtocolStatusData::from_stream(std::istream &response) { return true; };
	bool EquipmentProtocolStatusData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ListOfMcsStatsPerRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ListOfMcsStatsPerRadioMap::from_stream(std::istream &response) { return true; };
	bool ListOfMcsStatsPerRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CountsPerAlarmCodeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CountsPerAlarmCodeMap::from_stream(std::istream &response) { return true; };
	bool CountsPerAlarmCodeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool VLANStatusDataMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool VLANStatusDataMap::from_stream(std::istream &response) { return true; };
	bool VLANStatusDataMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientSessionChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientSessionChangedEvent::from_stream(std::istream &response) { return true; };
	bool ClientSessionChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool MacAddress::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool MacAddress::from_stream(std::istream &response) { return true; };
	bool MacAddress::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RealTimeStreamingStartEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RealTimeStreamingStartEvent::from_stream(std::istream &response) { return true; };
	bool RealTimeStreamingStartEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LinkQualityAggregatedStatsPerRadioTypeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LinkQualityAggregatedStatsPerRadioTypeMap::from_stream(std::istream &response) { return true; };
	bool LinkQualityAggregatedStatsPerRadioTypeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientDhcpDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientDhcpDetails::from_stream(std::istream &response) { return true; };
	bool ClientDhcpDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DhcpOfferEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DhcpOfferEvent::from_stream(std::istream &response) { return true; };
	bool DhcpOfferEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointOperatorProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointOperatorProfile::from_stream(std::istream &response) { return true; };
	bool PasspointOperatorProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ManufacturerDetailsRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ManufacturerDetailsRecord::from_stream(std::istream &response) { return true; };
	bool ManufacturerDetailsRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DhcpInformEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DhcpInformEvent::from_stream(std::istream &response) { return true; };
	bool DhcpInformEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PeerInfo::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PeerInfo::from_stream(std::istream &response) { return true; };
	bool PeerInfo::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsEquipment::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsEquipment::from_stream(std::istream &response) { return true; };
	bool SortColumnsEquipment::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool StreamingVideoServerRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool StreamingVideoServerRecord::from_stream(std::istream &response) { return true; };
	bool StreamingVideoServerRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool TunnelMetricData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool TunnelMetricData::from_stream(std::istream &response) { return true; };
	bool TunnelMetricData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadiusMetrics::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadiusMetrics::from_stream(std::istream &response) { return true; };
	bool RadiusMetrics::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool BonjourServiceSet::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool BonjourServiceSet::from_stream(std::istream &response) { return true; };
	bool BonjourServiceSet::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool NetworkAggregateStatusData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool NetworkAggregateStatusData::from_stream(std::istream &response) { return true; };
	bool NetworkAggregateStatusData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AutoOrManualString::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AutoOrManualString::from_stream(std::istream &response) { return true; };
	bool AutoOrManualString::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentRrmBulkUpdateItemPerRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentRrmBulkUpdateItemPerRadioMap::from_stream(std::istream &response) { return true; };
	bool EquipmentRrmBulkUpdateItemPerRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientConnectionDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientConnectionDetails::from_stream(std::istream &response) { return true; };
	bool ClientConnectionDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SystemEventRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SystemEventRecord::from_stream(std::istream &response) { return true; };
	bool SystemEventRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadiusDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadiusDetails::from_stream(std::istream &response) { return true; };
	bool RadiusDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool VlanSubnet::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool VlanSubnet::from_stream(std::istream &response) { return true; };
	bool VlanSubnet::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsLocation::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsLocation::from_stream(std::istream &response) { return true; };
	bool SortColumnsLocation::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DailyTimeRangeSchedule::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DailyTimeRangeSchedule::from_stream(std::istream &response) { return true; };
	bool DailyTimeRangeSchedule::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointNaiRealmInformation::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointNaiRealmInformation::from_stream(std::istream &response) { return true; };
	bool PasspointNaiRealmInformation::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointDuple::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointDuple::from_stream(std::istream &response) { return true; };
	bool PasspointDuple::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool FirmwareTrackAssignmentRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool FirmwareTrackAssignmentRecord::from_stream(std::istream &response) { return true; };
	bool FirmwareTrackAssignmentRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponseProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponseProfile::from_stream(std::istream &response) { return true; };
	bool PaginationResponseProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ServiceMetricsCollectionConfigProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ServiceMetricsCollectionConfigProfile::from_stream(std::istream &response) { return true; };
	bool ServiceMetricsCollectionConfigProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool Location::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool Location::from_stream(std::istream &response) { return true; };
	bool Location::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextStatus::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextStatus::from_stream(std::istream &response) { return true; };
	bool PaginationContextStatus::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DhcpDeclineEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DhcpDeclineEvent::from_stream(std::istream &response) { return true; };
	bool DhcpDeclineEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AlarmDetailsAttributesMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AlarmDetailsAttributesMap::from_stream(std::istream &response) { return true; };
	bool AlarmDetailsAttributesMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientSessionMetricDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientSessionMetricDetails::from_stream(std::istream &response) { return true; };
	bool ClientSessionMetricDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CustomerFirmwareTrackRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CustomerFirmwareTrackRecord::from_stream(std::istream &response) { return true; };
	bool CustomerFirmwareTrackRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RtpFlowStats::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RtpFlowStats::from_stream(std::istream &response) { return true; };
	bool RtpFlowStats::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextServiceMetric::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextServiceMetric::from_stream(std::istream &response) { return true; };
	bool PaginationContextServiceMetric::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientAuthEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientAuthEvent::from_stream(std::istream &response) { return true; };
	bool ClientAuthEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ChannelPowerLevel::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ChannelPowerLevel::from_stream(std::istream &response) { return true; };
	bool ChannelPowerLevel::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool FirmwareTrackAssignmentDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool FirmwareTrackAssignmentDetails::from_stream(std::istream &response) { return true; };
	bool FirmwareTrackAssignmentDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioUtilization::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioUtilization::from_stream(std::istream &response) { return true; };
	bool RadioUtilization::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointVenueTypeAssignment::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointVenueTypeAssignment::from_stream(std::istream &response) { return true; };
	bool PasspointVenueTypeAssignment::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool Profile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool Profile::from_stream(std::istream &response) { return true; };
	bool Profile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponseStatus::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponseStatus::from_stream(std::istream &response) { return true; };
	bool PaginationResponseStatus::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientActivityAggregatedStatsPerRadioTypeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientActivityAggregatedStatsPerRadioTypeMap::from_stream(std::istream &response) { return true; };
	bool ClientActivityAggregatedStatsPerRadioTypeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool Customer::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool Customer::from_stream(std::istream &response) { return true; };
	bool Customer::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioProfileConfigurationMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioProfileConfigurationMap::from_stream(std::istream &response) { return true; };
	bool RadioProfileConfigurationMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentRemovedEvent::from_stream(std::istream &response) { return true; };
	bool EquipmentRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SourceSelectionSteering::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SourceSelectionSteering::from_stream(std::istream &response) { return true; };
	bool SourceSelectionSteering::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioStatistics::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioStatistics::from_stream(std::istream &response) { return true; };
	bool RadioStatistics::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientChangedEvent::from_stream(std::istream &response) { return true; };
	bool ClientChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentPeerStatusData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentPeerStatusData::from_stream(std::istream &response) { return true; };
	bool EquipmentPeerStatusData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentAutoProvisioningSettings::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentAutoProvisioningSettings::from_stream(std::istream &response) { return true; };
	bool EquipmentAutoProvisioningSettings::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponseServiceMetric::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponseServiceMetric::from_stream(std::istream &response) { return true; };
	bool PaginationResponseServiceMetric::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ApNodeMetrics::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ApNodeMetrics::from_stream(std::istream &response) { return true; };
	bool ApNodeMetrics::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PortalUser::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PortalUser::from_stream(std::istream &response) { return true; };
	bool PortalUser::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool WepKey::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool WepKey::from_stream(std::istream &response) { return true; };
	bool WepKey::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool IntegerPerStatusCodeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool IntegerPerStatusCodeMap::from_stream(std::istream &response) { return true; };
	bool IntegerPerStatusCodeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioBestApSettings::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioBestApSettings::from_stream(std::istream &response) { return true; };
	bool RadioBestApSettings::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ProfileAddedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ProfileAddedEvent::from_stream(std::istream &response) { return true; };
	bool ProfileAddedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ServiceAdoptionMetrics::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ServiceAdoptionMetrics::from_stream(std::istream &response) { return true; };
	bool ServiceAdoptionMetrics::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool Client::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool Client::from_stream(std::istream &response) { return true; };
	bool Client::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AlarmRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AlarmRemovedEvent::from_stream(std::istream &response) { return true; };
	bool AlarmRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentRrmBulkUpdateRequest::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentRrmBulkUpdateRequest::from_stream(std::istream &response) { return true; };
	bool EquipmentRrmBulkUpdateRequest::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ApNetworkConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ApNetworkConfiguration::from_stream(std::istream &response) { return true; };
	bool ApNetworkConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointVenueName::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointVenueName::from_stream(std::istream &response) { return true; };
	bool PasspointVenueName::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponseLocation::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponseLocation::from_stream(std::istream &response) { return true; };
	bool PaginationResponseLocation::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RealTimeSipCallReportEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RealTimeSipCallReportEvent::from_stream(std::istream &response) { return true; };
	bool RealTimeSipCallReportEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentRoutingRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentRoutingRecord::from_stream(std::istream &response) { return true; };
	bool EquipmentRoutingRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointProfile::from_stream(std::istream &response) { return true; };
	bool PasspointProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool IntegerPerRadioTypeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool IntegerPerRadioTypeMap::from_stream(std::istream &response) { return true; };
	bool IntegerPerRadioTypeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsPortalUser::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsPortalUser::from_stream(std::istream &response) { return true; };
	bool SortColumnsPortalUser::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RoutingRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RoutingRemovedEvent::from_stream(std::istream &response) { return true; };
	bool RoutingRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool TimedAccessUserRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool TimedAccessUserRecord::from_stream(std::istream &response) { return true; };
	bool TimedAccessUserRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CustomerChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CustomerChangedEvent::from_stream(std::istream &response) { return true; };
	bool CustomerChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool Alarm::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool Alarm::from_stream(std::istream &response) { return true; };
	bool Alarm::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AlarmCounts::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AlarmCounts::from_stream(std::istream &response) { return true; };
	bool AlarmCounts::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool MacAllowlistRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool MacAllowlistRecord::from_stream(std::istream &response) { return true; };
	bool MacAllowlistRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadiusServer::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadiusServer::from_stream(std::istream &response) { return true; };
	bool RadiusServer::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool MinMaxAvgValueInt::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool MinMaxAvgValueInt::from_stream(std::istream &response) { return true; };
	bool MinMaxAvgValueInt::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SourceSelectionValue::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SourceSelectionValue::from_stream(std::istream &response) { return true; };
	bool SourceSelectionValue::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ChannelInfoReports::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ChannelInfoReports::from_stream(std::istream &response) { return true; };
	bool ChannelInfoReports::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadiusServerDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadiusServerDetails::from_stream(std::istream &response) { return true; };
	bool RadiusServerDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioProfileConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioProfileConfiguration::from_stream(std::istream &response) { return true; };
	bool RadioProfileConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsProfile::from_stream(std::istream &response) { return true; };
	bool SortColumnsProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LongValueMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LongValueMap::from_stream(std::istream &response) { return true; };
	bool LongValueMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ServiceMetric::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ServiceMetric::from_stream(std::istream &response) { return true; };
	bool ServiceMetric::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientFirstDataEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientFirstDataEvent::from_stream(std::istream &response) { return true; };
	bool ClientFirstDataEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LocationChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LocationChangedEvent::from_stream(std::istream &response) { return true; };
	bool LocationChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ProfileChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ProfileChangedEvent::from_stream(std::istream &response) { return true; };
	bool ProfileChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CapacityPerRadioDetailsMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CapacityPerRadioDetailsMap::from_stream(std::istream &response) { return true; };
	bool CapacityPerRadioDetailsMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ManufacturerOuiDetailsPerOuiMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ManufacturerOuiDetailsPerOuiMap::from_stream(std::istream &response) { return true; };
	bool ManufacturerOuiDetailsPerOuiMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SyslogRelay::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SyslogRelay::from_stream(std::istream &response) { return true; };
	bool SyslogRelay::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AlarmAddedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AlarmAddedEvent::from_stream(std::istream &response) { return true; };
	bool AlarmAddedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool TrafficPerRadioDetailsPerRadioTypeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool TrafficPerRadioDetailsPerRadioTypeMap::from_stream(std::istream &response) { return true; };
	bool TrafficPerRadioDetailsPerRadioTypeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ApPerformance::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ApPerformance::from_stream(std::istream &response) { return true; };
	bool ApPerformance::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientFailureEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientFailureEvent::from_stream(std::istream &response) { return true; };
	bool ClientFailureEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientSession::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientSession::from_stream(std::istream &response) { return true; };
	bool ClientSession::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AutoOrManualValue::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AutoOrManualValue::from_stream(std::istream &response) { return true; };
	bool AutoOrManualValue::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentAdminStatusData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentAdminStatusData::from_stream(std::istream &response) { return true; };
	bool EquipmentAdminStatusData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool NoiseFloorDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool NoiseFloorDetails::from_stream(std::istream &response) { return true; };
	bool NoiseFloorDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioBasedSsidConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioBasedSsidConfiguration::from_stream(std::istream &response) { return true; };
	bool RadioBasedSsidConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientTimeoutEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientTimeoutEvent::from_stream(std::istream &response) { return true; };
	bool ClientTimeoutEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool TrafficDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool TrafficDetails::from_stream(std::istream &response) { return true; };
	bool TrafficDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool StatusChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool StatusChangedEvent::from_stream(std::istream &response) { return true; };
	bool StatusChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextSystemEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextSystemEvent::from_stream(std::istream &response) { return true; };
	bool PaginationContextSystemEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DhcpRequestEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DhcpRequestEvent::from_stream(std::istream &response) { return true; };
	bool DhcpRequestEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ApSsidMetrics::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ApSsidMetrics::from_stream(std::istream &response) { return true; };
	bool ApSsidMetrics::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CommonProbeDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CommonProbeDetails::from_stream(std::istream &response) { return true; };
	bool CommonProbeDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextClient::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextClient::from_stream(std::istream &response) { return true; };
	bool PaginationContextClient::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AlarmDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AlarmDetails::from_stream(std::istream &response) { return true; };
	bool AlarmDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool Status::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool Status::from_stream(std::istream &response) { return true; };
	bool Status::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ActiveBSSIDs::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ActiveBSSIDs::from_stream(std::istream &response) { return true; };
	bool ActiveBSSIDs::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool TimedAccessUserDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool TimedAccessUserDetails::from_stream(std::istream &response) { return true; };
	bool TimedAccessUserDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentNeighbouringStatusData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentNeighbouringStatusData::from_stream(std::istream &response) { return true; };
	bool EquipmentNeighbouringStatusData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool WebTokenAclTemplate::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool WebTokenAclTemplate::from_stream(std::istream &response) { return true; };
	bool WebTokenAclTemplate::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientDisconnectEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientDisconnectEvent::from_stream(std::istream &response) { return true; };
	bool ClientDisconnectEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool BannedChannel::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool BannedChannel::from_stream(std::istream &response) { return true; };
	bool BannedChannel::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PortalUserChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PortalUserChangedEvent::from_stream(std::istream &response) { return true; };
	bool PortalUserChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DhcpNakEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DhcpNakEvent::from_stream(std::istream &response) { return true; };
	bool DhcpNakEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointOsuProviderProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointOsuProviderProfile::from_stream(std::istream &response) { return true; };
	bool PasspointOsuProviderProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentPerformanceDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentPerformanceDetails::from_stream(std::istream &response) { return true; };
	bool EquipmentPerformanceDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponseEquipment::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponseEquipment::from_stream(std::istream &response) { return true; };
	bool PaginationResponseEquipment::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ElementRadioConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ElementRadioConfiguration::from_stream(std::istream &response) { return true; };
	bool ElementRadioConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RealTimeStreamingStartSessionEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RealTimeStreamingStartSessionEvent::from_stream(std::istream &response) { return true; };
	bool RealTimeStreamingStartSessionEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LocationActivityDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LocationActivityDetails::from_stream(std::istream &response) { return true; };
	bool LocationActivityDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextClientSession::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextClientSession::from_stream(std::istream &response) { return true; };
	bool PaginationContextClientSession::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ManufacturerOuiDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ManufacturerOuiDetails::from_stream(std::istream &response) { return true; };
	bool ManufacturerOuiDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool MapOfWmmQueueStatsPerRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool MapOfWmmQueueStatsPerRadioMap::from_stream(std::istream &response) { return true; };
	bool MapOfWmmQueueStatsPerRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ChannelHopSettings::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ChannelHopSettings::from_stream(std::istream &response) { return true; };
	bool ChannelHopSettings::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SourceSelectionMulticast::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SourceSelectionMulticast::from_stream(std::istream &response) { return true; };
	bool SourceSelectionMulticast::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool GatewayChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool GatewayChangedEvent::from_stream(std::istream &response) { return true; };
	bool GatewayChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LongPerRadioTypeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LongPerRadioTypeMap::from_stream(std::istream &response) { return true; };
	bool LongPerRadioTypeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool NeighbouringAPListConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool NeighbouringAPListConfiguration::from_stream(std::istream &response) { return true; };
	bool NeighbouringAPListConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentAddedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentAddedEvent::from_stream(std::istream &response) { return true; };
	bool EquipmentAddedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PingResponse::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PingResponse::from_stream(std::istream &response) { return true; };
	bool PingResponse::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponseSystemEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponseSystemEvent::from_stream(std::istream &response) { return true; };
	bool PaginationResponseSystemEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentUpgradeStatusData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentUpgradeStatusData::from_stream(std::istream &response) { return true; };
	bool EquipmentUpgradeStatusData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioStatisticsPerRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioStatisticsPerRadioMap::from_stream(std::istream &response) { return true; };
	bool RadioStatisticsPerRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsClientSession::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsClientSession::from_stream(std::istream &response) { return true; };
	bool SortColumnsClientSession::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponseAlarm::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponseAlarm::from_stream(std::istream &response) { return true; };
	bool PaginationResponseAlarm::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CustomerDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CustomerDetails::from_stream(std::istream &response) { return true; };
	bool CustomerDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientConnectSuccessEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientConnectSuccessEvent::from_stream(std::istream &response) { return true; };
	bool ClientConnectSuccessEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsAlarm::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsAlarm::from_stream(std::istream &response) { return true; };
	bool SortColumnsAlarm::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientIpAddressEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientIpAddressEvent::from_stream(std::istream &response) { return true; };
	bool ClientIpAddressEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PortalUserRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PortalUserRemovedEvent::from_stream(std::istream &response) { return true; };
	bool PortalUserRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ApElementConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ApElementConfiguration::from_stream(std::istream &response) { return true; };
	bool ApElementConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsSystemEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsSystemEvent::from_stream(std::istream &response) { return true; };
	bool SortColumnsSystemEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ServiceMetricConfigParameters::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ServiceMetricConfigParameters::from_stream(std::istream &response) { return true; };
	bool ServiceMetricConfigParameters::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool GenericResponse::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool GenericResponse::from_stream(std::istream &response) { return true; };
	bool GenericResponse::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool WebTokenRequest::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool WebTokenRequest::from_stream(std::istream &response) { return true; };
	bool WebTokenRequest::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextEquipment::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextEquipment::from_stream(std::istream &response) { return true; };
	bool PaginationContextEquipment::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CustomerPortalDashboardStatus::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CustomerPortalDashboardStatus::from_stream(std::istream &response) { return true; };
	bool CustomerPortalDashboardStatus::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool StatusRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool StatusRemovedEvent::from_stream(std::istream &response) { return true; };
	bool StatusRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientSessionDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientSessionDetails::from_stream(std::istream &response) { return true; };
	bool ClientSessionDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SsidStatistics::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SsidStatistics::from_stream(std::istream &response) { return true; };
	bool SsidStatistics::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ListOfSsidStatisticsPerRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ListOfSsidStatisticsPerRadioMap::from_stream(std::istream &response) { return true; };
	bool ListOfSsidStatisticsPerRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientActivityAggregatedStats::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientActivityAggregatedStats::from_stream(std::istream &response) { return true; };
	bool ClientActivityAggregatedStats::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool Equipment::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool Equipment::from_stream(std::istream &response) { return true; };
	bool Equipment::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ManagedFileInfo::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ManagedFileInfo::from_stream(std::istream &response) { return true; };
	bool ManagedFileInfo::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RealTimeSipCallStopEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RealTimeSipCallStopEvent::from_stream(std::istream &response) { return true; };
	bool RealTimeSipCallStopEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool TrafficPerRadioDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool TrafficPerRadioDetails::from_stream(std::istream &response) { return true; };
	bool TrafficPerRadioDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DaysOfWeekTimeRangeSchedule::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DaysOfWeekTimeRangeSchedule::from_stream(std::istream &response) { return true; };
	bool DaysOfWeekTimeRangeSchedule::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool NetworkAdminStatusData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool NetworkAdminStatusData::from_stream(std::istream &response) { return true; };
	bool NetworkAdminStatusData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool BonjourGatewayProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool BonjourGatewayProfile::from_stream(std::istream &response) { return true; };
	bool BonjourGatewayProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioMap::from_stream(std::istream &response) { return true; };
	bool RadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadiusProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadiusProfile::from_stream(std::istream &response) { return true; };
	bool RadiusProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool UnserializableSystemEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool UnserializableSystemEvent::from_stream(std::istream &response) { return true; };
	bool UnserializableSystemEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PairLongLong::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PairLongLong::from_stream(std::istream &response) { return true; };
	bool PairLongLong::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioUtilizationDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioUtilizationDetails::from_stream(std::istream &response) { return true; };
	bool RadioUtilizationDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LocationRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LocationRemovedEvent::from_stream(std::istream &response) { return true; };
	bool LocationRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PerProcessUtilization::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PerProcessUtilization::from_stream(std::istream &response) { return true; };
	bool PerProcessUtilization::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool MeshGroupMember::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool MeshGroupMember::from_stream(std::istream &response) { return true; };
	bool MeshGroupMember::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool FirmwareTrackRecord::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool FirmwareTrackRecord::from_stream(std::istream &response) { return true; };
	bool FirmwareTrackRecord::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool BlocklistDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool BlocklistDetails::from_stream(std::istream &response) { return true; };
	bool BlocklistDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool JsonSerializedException::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool JsonSerializedException::from_stream(std::istream &response) { return true; };
	bool JsonSerializedException::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadiusNasConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadiusNasConfiguration::from_stream(std::istream &response) { return true; };
	bool RadiusNasConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CaptivePortalConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CaptivePortalConfiguration::from_stream(std::istream &response) { return true; };
	bool CaptivePortalConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool DhcpAckEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool DhcpAckEvent::from_stream(std::istream &response) { return true; };
	bool DhcpAckEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextPortalUser::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextPortalUser::from_stream(std::istream &response) { return true; };
	bool PaginationContextPortalUser::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponsePortalUser::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponsePortalUser::from_stream(std::istream &response) { return true; };
	bool PaginationResponsePortalUser::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool MeshGroup::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool MeshGroup::from_stream(std::istream &response) { return true; };
	bool MeshGroup::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool MetricConfigParameterMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool MetricConfigParameterMap::from_stream(std::istream &response) { return true; };
	bool MetricConfigParameterMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RtlsSettings::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RtlsSettings::from_stream(std::istream &response) { return true; };
	bool RtlsSettings::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SsidConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SsidConfiguration::from_stream(std::istream &response) { return true; };
	bool SsidConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PortalUserAddedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PortalUserAddedEvent::from_stream(std::istream &response) { return true; };
	bool PortalUserAddedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RealTimeSipCallStartEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RealTimeSipCallStartEvent::from_stream(std::istream &response) { return true; };
	bool RealTimeSipCallStartEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextLocation::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextLocation::from_stream(std::istream &response) { return true; };
	bool PaginationContextLocation::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool NoiseFloorPerRadioDetailsMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool NoiseFloorPerRadioDetailsMap::from_stream(std::istream &response) { return true; };
	bool NoiseFloorPerRadioDetailsMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentLANStatusData::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentLANStatusData::from_stream(std::istream &response) { return true; };
	bool EquipmentLANStatusData::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool IntegerStatusCodeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool IntegerStatusCodeMap::from_stream(std::istream &response) { return true; };
	bool IntegerStatusCodeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool GreTunnelConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool GreTunnelConfiguration::from_stream(std::istream &response) { return true; };
	bool GreTunnelConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RealTimeStreamingStopEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RealTimeStreamingStopEvent::from_stream(std::istream &response) { return true; };
	bool RealTimeStreamingStopEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioUtilizationPerRadioDetailsMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioUtilizationPerRadioDetailsMap::from_stream(std::istream &response) { return true; };
	bool RadioUtilizationPerRadioDetailsMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentChangedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentChangedEvent::from_stream(std::istream &response) { return true; };
	bool EquipmentChangedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ChannelInfo::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ChannelInfo::from_stream(std::istream &response) { return true; };
	bool ChannelInfo::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ServiceMetricSurveyConfigParameters::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ServiceMetricSurveyConfigParameters::from_stream(std::istream &response) { return true; };
	bool ServiceMetricSurveyConfigParameters::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CapacityDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CapacityDetails::from_stream(std::istream &response) { return true; };
	bool CapacityDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LocationDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LocationDetails::from_stream(std::istream &response) { return true; };
	bool LocationDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentScanDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentScanDetails::from_stream(std::istream &response) { return true; };
	bool EquipmentScanDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientAddedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientAddedEvent::from_stream(std::istream &response) { return true; };
	bool ClientAddedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ActiveBSSID::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ActiveBSSID::from_stream(std::istream &response) { return true; };
	bool ActiveBSSID::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool NetworkProbeMetrics::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool NetworkProbeMetrics::from_stream(std::istream &response) { return true; };
	bool NetworkProbeMetrics::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsStatus::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsStatus::from_stream(std::istream &response) { return true; };
	bool SortColumnsStatus::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ChannelUtilizationPerRadioDetailsMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ChannelUtilizationPerRadioDetailsMap::from_stream(std::istream &response) { return true; };
	bool ChannelUtilizationPerRadioDetailsMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RrmBulkUpdateApDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RrmBulkUpdateApDetails::from_stream(std::istream &response) { return true; };
	bool RrmBulkUpdateApDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RfElementConfiguration::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RfElementConfiguration::from_stream(std::istream &response) { return true; };
	bool RfElementConfiguration::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool WmmQueueStats::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool WmmQueueStats::from_stream(std::istream &response) { return true; };
	bool WmmQueueStats::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ListOfChannelInfoReportsPerRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ListOfChannelInfoReportsPerRadioMap::from_stream(std::istream &response) { return true; };
	bool ListOfChannelInfoReportsPerRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ChannelUtilizationDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ChannelUtilizationDetails::from_stream(std::istream &response) { return true; };
	bool ChannelUtilizationDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool MeshGroupProperty::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool MeshGroupProperty::from_stream(std::istream &response) { return true; };
	bool MeshGroupProperty::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextProfile::from_stream(std::istream &response) { return true; };
	bool PaginationContextProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationContextAlarm::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationContextAlarm::from_stream(std::istream &response) { return true; };
	bool PaginationContextAlarm::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool SortColumnsServiceMetric::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool SortColumnsServiceMetric::from_stream(std::istream &response) { return true; };
	bool SortColumnsServiceMetric::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ServiceMetricRadioConfigParameters::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ServiceMetricRadioConfigParameters::from_stream(std::istream &response) { return true; };
	bool ServiceMetricRadioConfigParameters::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientRemovedEvent::from_stream(std::istream &response) { return true; };
	bool ClientRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentCapacityDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentCapacityDetails::from_stream(std::istream &response) { return true; };
	bool EquipmentCapacityDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioChannelChangeSettings::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioChannelChangeSettings::from_stream(std::istream &response) { return true; };
	bool RadioChannelChangeSettings::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool WebTokenResult::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool WebTokenResult::from_stream(std::istream &response) { return true; };
	bool WebTokenResult::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool BaseDhcpEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool BaseDhcpEvent::from_stream(std::istream &response) { return true; };
	bool BaseDhcpEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CustomerFirmwareTrackSettings::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CustomerFirmwareTrackSettings::from_stream(std::istream &response) { return true; };
	bool CustomerFirmwareTrackSettings::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool LocationActivityDetailsMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool LocationActivityDetailsMap::from_stream(std::istream &response) { return true; };
	bool LocationActivityDetailsMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ChannelUtilizationPerRadioDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ChannelUtilizationPerRadioDetails::from_stream(std::istream &response) { return true; };
	bool ChannelUtilizationPerRadioDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointConnectionCapability::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointConnectionCapability::from_stream(std::istream &response) { return true; };
	bool PasspointConnectionCapability::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RfConfigMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RfConfigMap::from_stream(std::istream &response) { return true; };
	bool RfConfigMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CustomerRemovedEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CustomerRemovedEvent::from_stream(std::istream &response) { return true; };
	bool CustomerRemovedEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RadioUtilizationReport::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RadioUtilizationReport::from_stream(std::istream &response) { return true; };
	bool RadioUtilizationReport::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PasspointVenueProfile::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PasspointVenueProfile::from_stream(std::istream &response) { return true; };
	bool PasspointVenueProfile::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool AdvancedRadioMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool AdvancedRadioMap::from_stream(std::istream &response) { return true; };
	bool AdvancedRadioMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool ClientFailureDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool ClientFailureDetails::from_stream(std::istream &response) { return true; };
	bool ClientFailureDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponseClientSession::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponseClientSession::from_stream(std::istream &response) { return true; };
	bool PaginationResponseClientSession::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool EquipmentPerRadioUtilizationDetailsMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool EquipmentPerRadioUtilizationDetailsMap::from_stream(std::istream &response) { return true; };
	bool EquipmentPerRadioUtilizationDetailsMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool NoiseFloorPerRadioDetails::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool NoiseFloorPerRadioDetails::from_stream(std::istream &response) { return true; };
	bool NoiseFloorPerRadioDetails::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool McsStats::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool McsStats::from_stream(std::istream &response) { return true; };
	bool McsStats::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool PaginationResponseClient::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool PaginationResponseClient::from_stream(std::istream &response) { return true; };
	bool PaginationResponseClient::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool CountsPerEquipmentIdPerAlarmCodeMap::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool CountsPerEquipmentIdPerAlarmCodeMap::from_stream(std::istream &response) { return true; };
	bool CountsPerEquipmentIdPerAlarmCodeMap::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

	bool RealTimeEvent::to_JSON(Poco::JSON::Object &obj) const { return true; };
	bool RealTimeEvent::from_stream(std::istream &response) { return true; };
	bool RealTimeEvent::from_object(Poco::JSON::Object::Ptr Obj) { return true; };

}; /* end namespace TIP::PORTAL */
