//
// Created by stephane bourque on 2022-01-10.
//

#include "RESTAPI_AnalyticsObjects.h"
#include "RESTAPI_ProvObjects.h"
#include "framework/MicroService.h"

using OpenWifi::RESTAPI_utils::field_to_json;
using OpenWifi::RESTAPI_utils::field_from_json;

namespace OpenWifi::AnalyticsObjects {

    void Report::reset() {
    }

    void Report::to_json(Poco::JSON::Object &Obj) const {
    }

    void VenueInfo::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"id",id);
        field_to_json(Obj,"name",name);
        field_to_json(Obj,"description",description);
        field_to_json(Obj,"retention",retention);
        field_to_json(Obj,"interval",interval);
        field_to_json(Obj,"monitorSubVenues",monitorSubVenues);
    }

    bool VenueInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"id",id);
            field_from_json(Obj,"name",name);
            field_from_json(Obj,"description",description);
            field_from_json(Obj,"retention",retention);
            field_from_json(Obj,"interval",interval);
            field_from_json(Obj,"monitorSubVenues",monitorSubVenues);
            return true;
        } catch(...) {

        }
        return false;
    }

    void BoardInfo::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json(Obj,"venueList",venueList);
    }

    bool BoardInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json(Obj,"venueList",venueList);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceInfo::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"boardId",boardId);
        field_to_json(Obj,"type",type);
        field_to_json(Obj,"serialNumber",serialNumber);
        field_to_json(Obj,"deviceType",deviceType);
        field_to_json(Obj,"lastContact",lastContact);
        field_to_json(Obj,"lastPing",lastPing);
        field_to_json(Obj,"lastState",lastState);
        field_to_json(Obj,"lastFirmware",lastFirmware);
        field_to_json(Obj,"lastFirmwareUpdate",lastFirmwareUpdate);
        field_to_json(Obj,"lastConnection",lastConnection);
        field_to_json(Obj,"lastDisconnection",lastDisconnection);
        field_to_json(Obj,"pings",pings);
        field_to_json(Obj,"states",states);
        field_to_json(Obj,"connected",connected);
        field_to_json(Obj,"connectionIp",connectionIp);
        field_to_json(Obj,"associations_2g",associations_2g);
        field_to_json(Obj,"associations_5g",associations_5g);
        field_to_json(Obj,"associations_6g",associations_6g);
        field_to_json(Obj,"health",health);
        field_to_json(Obj,"lastHealth",lastHealth);
        field_to_json(Obj,"locale",locale);
        field_to_json(Obj,"uptime",uptime);
        field_to_json(Obj,"memory",memory);
    }

    bool DeviceInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"boardId",boardId);
            field_from_json(Obj,"type",type);
            field_from_json(Obj,"serialNumber",serialNumber);
            field_from_json(Obj,"deviceType",deviceType);
            field_from_json(Obj,"lastContact",lastContact);
            field_from_json(Obj,"lastPing",lastPing);
            field_from_json(Obj,"lastState",lastState);
            field_from_json(Obj,"lastFirmware",lastFirmware);
            field_from_json(Obj,"lastFirmwareUpdate",lastFirmwareUpdate);
            field_from_json(Obj,"lastConnection",lastConnection);
            field_from_json(Obj,"lastDisconnection",lastDisconnection);
            field_from_json(Obj,"pings",pings);
            field_from_json(Obj,"states",states);
            field_from_json(Obj,"connected",connected);
            field_from_json(Obj,"connectionIp",connectionIp);
            field_from_json(Obj,"associations_2g",associations_2g);
            field_from_json(Obj,"associations_5g",associations_5g);
            field_from_json(Obj,"associations_6g",associations_6g);
            field_from_json(Obj,"health",health);
            field_from_json(Obj,"lastHealth",lastHealth);
            field_from_json(Obj,"locale",locale);
            field_from_json(Obj,"uptime",uptime);
            field_from_json(Obj,"memory",memory);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceInfoList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"devices",devices);
    }

    bool DeviceInfoList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"devices",devices);
            return true;
        } catch(...) {

        }
        return false;
    }

    void UE_rate::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"bitrate",bitrate);
        field_to_json(Obj,"mcs",mcs);
        field_to_json(Obj,"nss",nss);
        field_to_json(Obj,"ht",ht);
        field_to_json(Obj,"sgi",sgi);
        field_to_json(Obj,"chwidth",chwidth);
    }

    bool UE_rate::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"bitrate",bitrate);
            field_from_json(Obj,"mcs",mcs);
            field_from_json(Obj,"nss",nss);
            field_from_json(Obj,"ht",ht);
            field_from_json(Obj,"sgi",sgi);
            field_from_json(Obj,"chwidth",chwidth);
            return true;
        } catch(...) {

        }
        return false;
    }

    void UETimePoint::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"station",station);
        field_to_json(Obj,"rssi",rssi);
        field_to_json(Obj,"tx_bytes",tx_bytes);
        field_to_json(Obj,"rx_bytes",rx_bytes);
        field_to_json(Obj,"tx_duration",tx_duration);
        field_to_json(Obj,"rx_packets",rx_packets);
        field_to_json(Obj,"tx_packets",tx_packets);
        field_to_json(Obj,"tx_retries",tx_retries);
        field_to_json(Obj,"tx_failed",tx_failed);
        field_to_json(Obj,"connected",connected);
        field_to_json(Obj,"inactive",inactive);
        field_to_json(Obj,"tx_rate",tx_rate);
        field_to_json(Obj,"rx_rate",rx_rate);
    }

    bool UETimePoint::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"station",station);
            field_from_json(Obj,"rssi",rssi);
            field_from_json(Obj,"tx_bytes",tx_bytes);
            field_from_json(Obj,"rx_bytes",rx_bytes);
            field_from_json(Obj,"tx_duration",tx_duration);
            field_from_json(Obj,"rx_packets",rx_packets);
            field_from_json(Obj,"tx_packets",tx_packets);
            field_from_json(Obj,"tx_retries",tx_retries);
            field_from_json(Obj,"tx_failed",tx_failed);
            field_from_json(Obj,"connected",connected);
            field_from_json(Obj,"inactive",inactive);
            field_from_json(Obj,"tx_rate",tx_rate);
            field_from_json(Obj,"rx_rate",rx_rate);
            return true;
        } catch(...) {

        }
        return false;
    }

    void APTimePoint::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"collisions",collisions);
        field_to_json(Obj,"multicast",multicast);
        field_to_json(Obj,"rx_bytes",rx_bytes);
        field_to_json(Obj,"rx_dropped",rx_dropped);
        field_to_json(Obj,"rx_errors",rx_errors);
        field_to_json(Obj,"rx_packets",rx_packets);
        field_to_json(Obj,"tx_bytes",tx_bytes);
        field_to_json(Obj,"tx_packets",tx_packets);
        field_to_json(Obj,"tx_dropped",tx_dropped);
        field_to_json(Obj,"tx_errors",tx_errors);
        field_to_json(Obj,"tx_packets",tx_packets);
    }

    bool APTimePoint::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"collisions",collisions);
            field_from_json(Obj,"multicast",multicast);
            field_from_json(Obj,"rx_bytes",rx_bytes);
            field_from_json(Obj,"rx_dropped",rx_dropped);
            field_from_json(Obj,"rx_errors",rx_errors);
            field_from_json(Obj,"rx_packets",rx_packets);
            field_from_json(Obj,"tx_bytes",tx_bytes);
            field_from_json(Obj,"tx_packets",tx_packets);
            field_from_json(Obj,"tx_dropped",tx_dropped);
            field_from_json(Obj,"tx_errors",tx_errors);
            field_from_json(Obj,"tx_packets",tx_packets);
            return true;
        } catch(...) {

        }
        return false;
    }

    void TIDstat_entry::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"rx_msdu",rx_msdu);
        field_to_json(Obj,"tx_msdu",tx_msdu);
        field_to_json(Obj,"tx_msdu_failed",tx_msdu_failed);
        field_to_json(Obj,"tx_msdu_retries",tx_msdu_retries);
    }

    bool TIDstat_entry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"rx_msdu",rx_msdu);
            field_from_json(Obj,"tx_msdu",tx_msdu);
            field_from_json(Obj,"tx_msdu_failed",tx_msdu_failed);
            field_from_json(Obj,"tx_msdu_retries",tx_msdu_retries);
            return true;
        } catch(...) {

        }
        return false;
    }

    void RadioTimePoint::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"band",band);
        field_to_json(Obj,"radio_channel",radio_channel);
        field_to_json(Obj,"active_ms",active_ms);
        field_to_json(Obj,"busy_ms",busy_ms);
        field_to_json(Obj,"receive_ms",receive_ms);
        field_to_json(Obj,"transmit_ms",transmit_ms);
        field_to_json(Obj,"tx_power",tx_power);
        field_to_json(Obj,"channel",channel);
        field_to_json(Obj,"temperature",temperature);
        field_to_json(Obj,"noise",noise);
    }

    bool RadioTimePoint::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"band",band);
            field_from_json(Obj,"radio_channel",radio_channel);
            field_from_json(Obj,"active_ms",active_ms);
            field_from_json(Obj,"busy_ms",busy_ms);
            field_from_json(Obj,"receive_ms",receive_ms);
            field_from_json(Obj,"transmit_ms",transmit_ms);
            field_from_json(Obj,"tx_power",tx_power);
            field_from_json(Obj,"channel",channel);
            field_from_json(Obj,"temperature",temperature);
            field_from_json(Obj,"noise",noise);
            return true;
        } catch(...) {

        }
        return false;
    }

    void SSIDTimePoint::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"bssid",bssid);
        field_to_json(Obj,"mode",mode);
        field_to_json(Obj,"ssid",ssid);
        field_to_json(Obj,"band",band);
        field_to_json(Obj,"associations",associations);
    }

    bool SSIDTimePoint::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"bssid",bssid);
            field_from_json(Obj,"mode",mode);
            field_from_json(Obj,"ssid",ssid);
            field_from_json(Obj,"band",band);
            field_from_json(Obj,"associations",associations);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceTimePoint::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"id",id);
        field_to_json(Obj,"boardId",boardId);
        field_to_json(Obj,"timestamp",timestamp);
        field_to_json(Obj,"ap_data",ap_data);
        field_to_json(Obj,"ssid_data",ssid_data);
        field_to_json(Obj,"radio_data",radio_data);
        field_to_json(Obj,"device_info",device_info);
    }

    bool DeviceTimePoint::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"id",id);
            field_from_json(Obj,"boardId",boardId);
            field_from_json(Obj,"timestamp",timestamp);
            field_from_json(Obj,"ap_data",ap_data);
            field_from_json(Obj,"ssid_data",ssid_data);
            field_from_json(Obj,"radio_data",radio_data);
            field_from_json(Obj,"device_info",device_info);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceTimePointList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"points",points);
    }

    bool DeviceTimePointList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"points",points);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceTimePointStats::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"firstPoint",firstPoint);
        field_to_json(Obj,"lastPoint",lastPoint);
        field_to_json(Obj,"count",count);
    }

    bool DeviceTimePointStats::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"firstPoint",firstPoint);
            field_from_json(Obj,"lastPoint",lastPoint);
            field_from_json(Obj,"count",count);
            return true;
        } catch(...) {

        }
        return false;
    }

}