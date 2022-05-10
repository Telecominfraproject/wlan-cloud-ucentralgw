//
// Created by stephane bourque on 2021-10-27.
//

#ifndef OWSUB_RESTAPI_SUBOBJECTS_H
#define OWSUB_RESTAPI_SUBOBJECTS_H

#include <string>

#include "Poco/JSON/Object.h"

namespace OpenWifi::SubObjects {

    struct HomeDeviceMode {
        bool            enableLEDS = true;
        std::string     type;       // bridge, manual, automatic
        std::string     subnet;
        std::string     subnetMask;
        std::string     startIP;
        std::string     endIP;
        uint64_t        created = 0 ;
        uint64_t        modified = 0 ;
        std::string     subnetV6;
        int             subnetMaskV6=0;
        std::string     startIPV6;
        std::string     endIPV6;
        std::string     leaseTime;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct IPReservation  {
        std::string     nickname;
        std::string     ipAddress;
        std::string     macAddress;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct IPReservationList {
        std::string                 id;
        std::vector<IPReservation>  reservations;
        uint64_t created = 0 ;
        uint64_t modified = 0 ;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DnsConfiguration {
        bool            ISP=false;
        bool            custom=false;
        std::string     primary;
        std::string     secondary;
        std::string     primaryV6;
        std::string     secondaryV6;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct InternetConnection {
        std::string     type; // automatic, pppoe, manual
        std::string     username;
        std::string     password;
        std::string     ipAddress;
        std::string     subnetMask;
        std::string     defaultGateway;
        bool            sendHostname = true;
        std::string     primaryDns;
        std::string     secondaryDns;
        uint64_t        created=0;
        uint64_t        modified=0;
        bool            ipV6Support=false;
        std::string     ipAddressV6;
        int             subnetMaskV6=0;
        std::string     defaultGatewayV6;
        std::string     primaryDnsV6;
        std::string     secondaryDnsV6;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct WifiNetwork {
        std::string     type;       // main, guest
        std::string     name;
        std::string     password;
        std::string     encryption;
        std::vector<std::string>    bands;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct WifiNetworkList {
        std::vector<WifiNetwork>    wifiNetworks;
        uint64_t                    created=0;
        uint64_t                    modified=0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct AccessTime {
        std::string day;
        std::vector<std::string>    rangeList;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct AccessTimes {
        std::vector<AccessTime> schedule;
        uint64_t        created=0;
        uint64_t        modified=0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct SubscriberDevice {
        std::string     name;
        std::string     description;
        std::string     macAddress;
        std::string     manufacturer;
        uint64_t        firstContact=0;
        uint64_t        lastContact=0;
        std::string     group;
        std::string     icon;
        bool            suspended=false;
        std::string     ip;
        std::vector<AccessTimes>    schedule;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct SubscriberDeviceList {
        std::vector<SubscriberDevice>   devices;
        uint64_t        created=0;
        uint64_t        modified=0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct Association {
        std::string     name;
        std::string     ssid;
        std::string     macAddress;
        int             rssi=0;
        int             power=0;
        std::string     ipv4;
        std::string     ipv6;
        uint64_t        tx=0;
        uint64_t        rx=0;
        std::string     manufacturer;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct AssociationList {
        std::vector<Association>    associations;
        uint64_t        created=0;
        uint64_t        modified=0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct Client {
        std::string     macAddress;
        std::string     speed;
        std::string     mode;
        std::string     ipv4;
        std::string     ipv6;
        uint64_t        tx=0;
        uint64_t        rx=0;
        std::string     manufacturer;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ClientList {
        std::vector<Client> clients;
        uint64_t        created=0;
        uint64_t        modified=0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct Location {
        std::string                 buildingName;
        std::vector<std::string>    addressLines;
        std::string                 city;
        std::string                 state;
        std::string                 postal;
        std::string                 country;
        std::vector<std::string>    phones;
        std::vector<std::string>    mobiles;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct RadioHE {
        bool                        multipleBSSID = false;
        bool                        ema = false;
        uint64_t                    bssColor = 64;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct RadioRates {
        uint64_t                    beacon = 6000;
        uint64_t                    multicast = 24000;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct RadioInformation {
        std::string             band;
        uint64_t                bandwidth;
        uint64_t                channel = 0 ;
        std::string             country;
        std::string             channelMode{"HE"};
        uint64_t                channelWidth = 80;
        std::string             requireMode;
        uint64_t                txpower=0;
        bool                    legacyRates = false;
        uint64_t                beaconInterval = 100;
        uint64_t                dtimPeriod = 2;
        uint64_t                maximumClients = 64;
        RadioRates              rates;
        RadioHE                 he;
        bool                    allowDFS=false;
        std::string             mimo;
        std::vector<std::string>    rawInfo;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct AccessPoint {
        std::string                 id;
        std::string                 macAddress;
        std::string                 serialNumber;
        std::string                 name;
        std::string                 deviceType;
        SubscriberDeviceList        subscriberDevices;
        IPReservationList           ipReservations;
        Location                    address;
        WifiNetworkList             wifiNetworks;
        InternetConnection          internetConnection;
        HomeDeviceMode              deviceMode;
        DnsConfiguration            dnsConfiguration;
        std::vector<RadioInformation>   radios;
        bool                        automaticUpgrade = true;
        std::string                 configurationUUID;
        std::string                 currentFirmware;
        uint64_t                    currentFirmwareDate;
        std::string                 latestFirmware;
        uint64_t                    latestFirmwareDate;
        bool                        newFirmwareAvailable;
        std::string                 latestFirmwareURI;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct AccessPointList {
        std::vector<AccessPoint>   list;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct SubscriberInfo {
        std::string                 id;
        std::string                 userId;
        std::string                 firstName;
        std::string                 initials;
        std::string                 lastName;
        std::string                 phoneNumber;
        std::string                 secondaryEmail;
        AccessPointList             accessPoints;
        Location                    serviceAddress;
        Location                    billingAddress;
        uint64_t                    created = 0;
        uint64_t                    modified = 0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct StatsEntry {
        uint64_t        timestamp=0;
        uint64_t        tx=0;
        uint64_t        rx=0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct StatsBlock {
        uint64_t                    modified=0;
        std::vector<StatsEntry>     external, internal;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
}

#endif //OWSUB_RESTAPI_SUBOBJECTS_H
