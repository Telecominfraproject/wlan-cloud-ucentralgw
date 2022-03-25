//
// Created by stephane bourque on 2022-01-10.
//

#pragma once

#include "RESTAPI_ProvObjects.h"
#include <vector>

namespace OpenWifi {

    namespace AnalyticsObjects {

        struct Report {
            uint64_t snapShot = 0;

            void reset();

            void to_json(Poco::JSON::Object &Obj) const;
        };

        struct VenueInfo {
            OpenWifi::Types::UUID_t id;
            std::string name;
            std::string description;
            uint64_t retention = 0;
            uint64_t interval = 0;
            bool monitorSubVenues = false;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct BoardInfo {
            ProvObjects::ObjectInfo info;
            std::vector<VenueInfo> venueList;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);

            inline bool operator<(const BoardInfo &bb) const {
                return info.id < bb.info.id;
            }

            inline bool operator==(const BoardInfo &bb) const {
                return info.id == bb.info.id;
            }
        };

        struct DeviceInfo {
            std::string boardId;
            std::string type;
            std::string serialNumber;
            std::string deviceType;
            uint64_t lastContact = 0 ;
            uint64_t lastPing = 0;
            uint64_t lastState = 0;
            std::string lastFirmware;
            uint64_t lastFirmwareUpdate = 0;
            uint64_t lastConnection = 0;
            uint64_t lastDisconnection = 0;
            uint64_t pings = 0;
            uint64_t states = 0;
            bool connected = false;
            std::string connectionIp;
            uint64_t associations_2g = 0;
            uint64_t associations_5g = 0;
            uint64_t associations_6g = 0;
            uint64_t health = 0;
            uint64_t lastHealth = 0;
            std::string locale;
            uint64_t uptime = 0;
            double memory = 0.0;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct DeviceInfoList {
            std::vector<DeviceInfo> devices;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        enum wifi_band {
            band_2g = 0, band_5g = 1, band_6g = 2
        };

        struct TIDstat_entry {
            uint64_t rx_msdu = 0,
                    tx_msdu = 0,
                    tx_msdu_failed = 0,
                    tx_msdu_retries = 0;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct UE_rate {
            uint64_t    bitrate=0;
            uint64_t    mcs=0;
            uint64_t    nss=0;
            bool        ht=false;
            bool        sgi=false;
            uint64_t    chwidth=0;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct UETimePoint {
            std::string station;
            int64_t rssi = 0;
            uint64_t tx_bytes = 0,
                    rx_bytes = 0,
                    tx_duration = 0,
                    rx_packets = 0,
                    tx_packets = 0,
                    tx_retries = 0,
                    tx_failed = 0,
                    connected = 0,
                    inactive = 0;
            UE_rate tx_rate,
                    rx_rate;
            std::vector<TIDstat_entry> tidstat;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        enum SSID_MODES {
            unknown = 0,
            ap,
            mesh,
            sta,
            wds_ap,
            wds_sta,
            wds_repeater
        };

        inline SSID_MODES SSID_Mode(const std::string &m) {
            if (m == "ap")
                return ap;
            if (m == "sta")
                return sta;
            if (m == "mesh")
                return mesh;
            if (m == "wds-ap")
                return wds_ap;
            if (m == "wds-sta")
                return wds_sta;
            if (m == "wds-repeater")
                return wds_repeater;
            return unknown;
        }

        struct SSIDTimePoint {
            std::string bssid,
                        mode,
                        ssid;
            uint64_t    band=0;
            std::vector<UETimePoint> associations;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };


        struct APTimePoint {
            uint64_t    collisions = 0,
                        multicast = 0,
                        rx_bytes = 0,
                        rx_dropped = 0,
                        rx_errors = 0,
                        rx_packets = 0,
                        tx_bytes = 0,
                        tx_dropped = 0,
                        tx_errors = 0,
                        tx_packets = 0;

            double      tx_bytes_bw = 0.0 ,
                        rx_bytes_bw = 0.0 ,
                        rx_dropped_pct = 0.0,
                        tx_dropped_pct = 0.0,
                        rx_packets_bw = 0.0,
                        tx_packets_bw = 0.0;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct RadioTimePoint {
            uint64_t    band = 0,
                        radio_channel = 0;
            uint64_t active_ms = 0,
                    busy_ms = 0,
                    receive_ms = 0,
                    transmit_ms = 0,
                    tx_power = 0,
                    channel = 0;
            int64_t temperature = 0,
                    noise = 0;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };


        struct DeviceTimePoint {
            std::string                 id;
            std::string                 boardId;
            uint64_t                    timestamp = 0;
            APTimePoint                 ap_data;
            std::vector<SSIDTimePoint>  ssid_data;
            std::vector<RadioTimePoint> radio_data;
            AnalyticsObjects::DeviceInfo device_info;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);

            inline bool operator<(const DeviceTimePoint &rhs) const {
                if(timestamp < rhs.timestamp)
                    return true;
                if(timestamp > rhs.timestamp)
                    return false;
                if(device_info.serialNumber < rhs.device_info.serialNumber)
                    return true;
                return false;
            }

            inline bool operator==(const DeviceTimePoint &rhs) const {
                return timestamp==rhs.timestamp && device_info.serialNumber==rhs.device_info.serialNumber;
            }

            inline bool operator>(const DeviceTimePoint &rhs) const {
                if(timestamp > rhs.timestamp)
                    return true;
                if(timestamp < rhs.timestamp)
                    return false;
                if(device_info.serialNumber > rhs.device_info.serialNumber)
                    return true;
                return false;
            }

        };

        struct DeviceTimePointList {
            std::vector<DeviceTimePoint>    points;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct BandwidthAnalysisEntry {
            uint64_t    timestamp = 0;

        };

        struct BandwidthAnalysis {

        };

        struct AverageValueSigned {
            int64_t     peak=0, avg=0, low=0;
        };

        struct AverageValueUnsigned {
            uint64_t     peak=0, avg=0, low=0;
        };

        struct RadioAnalysis {
            uint64_t                timestamp=0;
            AverageValueSigned      noise, temperature;
            AverageValueUnsigned    active_ms,
                                    busy_ms,
                                    transmit_ms,
                                    receive_ms;
        };

        struct DeviceTimePointStats {
            uint64_t                firstPoint=0;
            uint64_t                lastPoint=0;
            uint64_t                count=0;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };
    }
}