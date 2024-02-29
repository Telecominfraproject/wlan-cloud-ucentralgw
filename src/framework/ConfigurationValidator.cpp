//
// Created by stephane bourque on 2021-09-14.
//

#include <fstream>
#include <regex>

#include "ConfigurationValidator.h"
#include "framework/CountryCodes.h"
#include "framework/MicroServiceFuncs.h"
#include "framework/utils.h"

#include "Poco/StringTokenizer.h"
#include "Poco/URI.h"

#include "fmt/format.h"

#include <valijson/adapters/poco_json_adapter.hpp>
#include <valijson/constraints/constraint.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/utils/poco_json_utils.hpp>
#include <valijson/validator.hpp>

static const std::vector<std::string> GitJSONSchemaURLs = {
	{"https://raw.githubusercontent.com/Telecominfraproject/wlan-ucentral-schema/main/ucentral.schema.json"},
	{"https://raw.githubusercontent.com/Telecominfraproject/ols-ucentral-schema/main/ucentral.schema.json"}
};

static std::string DefaultAPSchema = R"foo(

{
    "$id": "https://openwrt.org/ucentral.schema.json",
    "$schema": "http://json-schema.org/draft-07/schema#",
    "description": "OpenWrt uCentral schema",
    "type": "object",
    "properties": {
        "strict": {
            "description": "The device will reject any configuration that causes warnings if strict mode is enabled.",
            "type": "boolean",
            "default": false
        },
        "uuid": {
            "description": "The unique ID of the configuration. This is the unix timestamp of when the config was created.",
            "type": "integer"
        },
        "unit": {
            "$ref": "#/$defs/unit"
        },
        "globals": {
            "$ref": "#/$defs/globals"
        },
        "definitions": {
            "$ref": "#/$defs/definitions"
        },
        "ethernet": {
            "type": "array",
            "items": {
                "$ref": "#/$defs/ethernet"
            }
        },
        "switch": {
            "$ref": "#/$defs/switch"
        },
        "radios": {
            "type": "array",
            "items": {
                "$ref": "#/$defs/radio"
            }
        },
        "interfaces": {
            "type": "array",
            "items": {
                "$ref": "#/$defs/interface"
            }
        },
        "services": {
            "$ref": "#/$defs/service"
        },
        "metrics": {
            "$ref": "#/$defs/metrics"
        },
        "config-raw": {
            "$ref": "#/$defs/config-raw"
        },
        "third-party": {
            "type": "object",
            "additionalProperties": true
        }
    },
    "$defs": {
        "unit": {
            "description": "A device has certain properties that describe its identity and location. These properties are described inside this object.",
            "type": "object",
            "properties": {
                "name": {
                    "description": "This is a free text field, stating the administrative name of the device. It may contain spaces and special characters.",
                    "type": "string"
                },
                "hostname": {
                    "description": "The hostname that shall be set on the device. If this field is not set, then the devices serial number is used.",
                    "type": "string",
                    "format": "hostname"
                },
                "location": {
                    "description": "This is a free text field, stating the location of the  device. It may contain spaces and special characters.",
                    "type": "string"
                },
                "timezone": {
                    "description": "This allows you to change the TZ of the device.",
                    "type": "string",
                    "examples": [
                        "UTC",
                        "EST5",
                        "CET-1CEST,M3.5.0,M10.5.0/3"
                    ]
                },
                "leds-active": {
                    "description": "This allows forcing all LEDs off.",
                    "type": "boolean",
                    "default": true
                },
                "random-password": {
                    "description": "The device shall create a random root password and tell the gateway about it.",
                    "type": "boolean",
                    "default": false
                },
                "beacon-advertisement": {
                    "description": "The TIP vendor IEs that shall be added to beacons",
                    "type": "object",
                    "properties": {
                        "device-name": {
                            "description": "Add an IE containing the device's name to beacons.",
                            "type": "boolean"
                        },
                        "device-serial": {
                            "description": "Add an IE containing the device's serial to beacons.",
                            "type": "boolean"
                        },
                        "network-id": {
                            "description": "A provider specific ID for the network/venue that the device is part of.",
                            "type": "integer"
                        }
                    }
                }
            }
        },
        "globals.wireless-multimedia.class-selector": {
            "type": "array",
            "items": {
                "type": "string",
                "enum": [
                    "CS0",
                    "CS1",
                    "CS2",
                    "CS3",
                    "CS4",
                    "CS5",
                    "CS6",
                    "CS7",
                    "AF11",
                    "AF12",
                    "AF13",
                    "AF21",
                    "AF22",
                    "AF23",
                    "AF31",
                    "AF32",
                    "AF33",
                    "AF41",
                    "AF42",
                    "AF43",
                    "DF",
                    "EF",
                    "VA",
                    "LE"
                ]
            }
        },
        "globals.wireless-multimedia.table": {
            "description": "Define the default WMM behaviour of all SSIDs on the device. Each access category can be assigned a default class selector that gets used for packet matching.",
            "type": "object",
            "additionalProperties": false,
            "properties": {
                "UP0": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP1": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP2": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP3": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP4": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP5": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP6": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP7": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                }
            }
        },
        "globals.wireless-multimedia.profile": {
            "type": "object",
            "additionalProperties": false,
            "properties": {
                "profile": {
                    "description": "Define a default profile that shall be used for the WMM behaviour of all SSIDs on the device.",
                    "type": "string",
                    "enum": [
                        "enterprise",
                        "rfc8325",
                        "3gpp"
                    ]
                }
            }
        },
        "globals": {
            "description": "A device has certain global properties that are used to derive parts of the final configuration that gets applied.",
            "type": "object",
            "properties": {
                "ipv4-network": {
                    "description": "Define the IPv4 range that is delegatable to the downstream interfaces This is described as a CIDR block. (192.168.0.0/16, 172.16.128/17)",
                    "type": "string",
                    "format": "uc-cidr4",
                    "examples": [
                        "192.168.0.0/16"
                    ]
                },
                "ipv6-network": {
                    "description": "Define the IPv6 range that is delegatable to the downstream interfaces This is described as a CIDR block. (fdca:1234:4567::/48)",
                    "type": "string",
                    "format": "uc-cidr6",
                    "examples": [
                        "fdca:1234:4567::/48"
                    ]
                },
                "wireless-multimedia": {
                    "anyOf": [
                        {
                            "$ref": "#/$defs/globals.wireless-multimedia.table"
                        },
                        {
                            "$ref": "#/$defs/globals.wireless-multimedia.profile"
                        }
                    ]
                }
            }
        },
        "interface.ssid.encryption": {
            "description": "A device has certain properties that describe its identity and location. These properties are described inside this object.",
            "type": "object",
            "properties": {
                "proto": {
                    "description": "The wireless encryption protocol that shall be used for this BSS",
                    "type": "string",
                    "enum": [
                        "none",
                        "owe",
                        "owe-transition",
                        "psk",
                        "psk2",
                        "psk-mixed",
                        "psk2-radius",
                        "wpa",
                        "wpa2",
                        "wpa-mixed",
                        "sae",
                        "sae-mixed",
                        "wpa3",
                        "wpa3-192",
                        "wpa3-mixed"
                    ],
                    "examples": [
                        "psk2"
                    ]
                },
                "key": {
                    "description": "The Pre Shared Key (PSK) that is used for encryption on the BSS when using any of the WPA-PSK modes.",
                    "type": "string",
                    "maxLength": 63,
                    "minLength": 8
                },
                "ieee80211w": {
                    "description": "Enable 802.11w Management Frame Protection (MFP) for this BSS.",
                    "type": "string",
                    "enum": [
                        "disabled",
                        "optional",
                        "required"
                    ],
                    "default": "disabled"
                },
                "key-caching": {
                    "description": "PMKSA created through EAP authentication and RSN preauthentication can be cached.",
                    "type": "boolean",
                    "default": true
                }
            }
        },
        "definitions": {
            "description": "This section is used to define templates that can be referenced by a configuration. This avoids duplication of data. A RADIUS server can be defined here for example and then referenced by several SSIDs.",
            "type": "object",
            "properties": {
                "wireless-encryption": {
                    "type": "object",
                    "description": "A dictionary of wireless encryption templates which can be referenced by the corresponding property name.",
                    "patternProperties": {
                        ".+": {
                            "$ref": "#/$defs/interface.ssid.encryption",
                            "additionalProperties": false
                        }
                    }
                }
            }
        },
        "ethernet": {
            "description": "This section defines the linkk speed and duplex mode of the physical copper/fiber ports of the device.",
            "type": "object",
            "properties": {
                "select-ports": {
                    "description": "The list of physical network devices that shall be configured. The names are logical ones and wildcardable.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "LAN1",
                            "LAN2",
                            "LAN3",
                            "LAN4",
                            "LAN*",
                            "WAN*",
                            "*"
                        ]
                    }
                },
                "speed": {
                    "description": "The link speed that shall be forced.",
                    "type": "integer",
                    "enum": [
                        10,
                        100,
                        1000,
                        2500,
                        5000,
                        10000
                    ]
                },
                "duplex": {
                    "description": "The duplex mode that shall be forced.",
                    "type": "string",
                    "enum": [
                        "half",
                        "full"
                    ]
                },
                "enabled": {
                    "description": "This allows forcing the port to down state by default.",
                    "type": "boolean",
                    "default": true
                },
                "services": {
                    "description": "The services that shall be offered on this L2 interface.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "quality-of-service"
                        ]
                    }
                }
            }
        },
        "switch": {
            "description": "This section defines the switch fabric specific features of a physical switch.",
            "type": "object",
            "properties": {
                "port-mirror": {
                    "description": "Enable mirror of traffic from multiple minotor ports to a single analysis port.",
                    "type": "object",
                    "properties": {
                        "monitor-ports": {
                            "description": "The list of ports that we want to mirror.",
                            "type": "array",
                            "items": {
                                "type": "string"
                            }
                        },
                        "analysis-port": {
                            "description": "The port that mirror'ed packets should be sent to.",
                            "type": "string"
                        }
                    }
                },
                "loop-detection": {
                    "description": "Enable loop detection on the L2 switches/bridge.",
                    "type": "object",
                    "properties": {
                        "protocol": {
                            "description": "Define which protocol shall be used for loop detection.",
                            "type": "string",
                            "enum": [
                                "rstp"
                            ],
                            "default": "rstp"
                        },
                        "roles": {
                            "description": "Define on which logical switches/bridges we want to provide loop-detection.",
                            "type": "array",
                            "items": {
                                "type": "string",
                                "enum": [
                                    "upstream",
                                    "downstream"
                                ]
                            }
                        }
                    }
                }
            }
        },
        "radio.rates": {
            "description": "The rate configuration of this BSS.",
            "type": "object",
            "properties": {
                "beacon": {
                    "description": "The beacon rate that shall be used by the BSS. Values are in Mbps.",
                    "type": "integer",
                    "default": 6000,
                    "enum": [
                        0,
                        1000,
                        2000,
                        5500,
                        6000,
                        9000,
                        11000,
                        12000,
                        18000,
                        24000,
                        36000,
                        48000,
                        54000
                    ]
                },
                "multicast": {
                    "description": "The multicast rate that shall be used by the BSS. Values are in Mbps.",
                    "type": "integer",
                    "default": 24000,
                    "enum": [
                        0,
                        1000,
                        2000,
                        5500,
                        6000,
                        9000,
                        11000,
                        12000,
                        18000,
                        24000,
                        36000,
                        48000,
                        54000
                    ]
                }
            }
        },
        "radio.he": {
            "description": "This section describes the HE specific configuration options of the BSS.",
            "type": "object",
            "properties": {
                "multiple-bssid": {
                    "description": "Enabling this option will make the PHY broadcast its BSSs using the multiple BSSID beacon IE.",
                    "type": "boolean",
                    "default": false
                },
                "ema": {
                    "description": "Enableing this option will make the PHY broadcast its multiple BSSID beacons using EMA.",
                    "type": "boolean",
                    "default": false
                },
                "bss-color": {
                    "description": "This enables BSS Coloring on the PHY. setting it to 0 disables the feature 1-63 sets the color and 64 will make hostapd pick a random color.",
                    "type": "integer",
                    "default": 64
                }
            }
        },
        "radio": {
            "description": "Describe a physical radio on the AP. A radio is be parent to several VAPs. They all share the same physical properties.",
            "type": "object",
            "properties": {
                "band": {
                    "description": "Specifies the wireless band to configure the radio for. Available radio device phys on the target system are matched by the wireless band given here. If multiple radio phys support the same band, the settings specified here will be applied to all of them.",
                    "type": "string",
                    "enum": [
                        "2G",
                        "5G",
                        "5G-lower",
                        "5G-upper",
                        "6G"
                    ]
                },
                "bandwidth": {
                    "description": "Specifies a narrow channel width in MHz, possible values are 5, 10, 20.",
                    "type": "integer",
                    "enum": [
                        5,
                        10,
                        20
                    ]
                },
                "channel": {
                    "description": "Specifies the wireless channel to use. A value of 'auto' starts the ACS algorithm.",
                    "oneOf": [
                        {
                            "type": "integer",
                            "maximum": 196,
                            "minimum": 1
                        },
                        {
                            "type": "string",
                            "const": "auto"
                        }
                    ]
                },
                "valid-channels": {
                    "description": "Pass a list of valid-channels that can be used during ACS.",
                    "type": "array",
                    "items": {
                        "type": "integer",
                        "maximum": 196,
                        "minimum": 1
                    }
                },
                "country": {
                    "description": "Specifies the country code, affects the available channels and transmission powers.",
                    "type": "string",
                    "maxLength": 2,
                    "minLength": 2,
                    "examples": [
                        "US"
                    ]
                },
                "allow-dfs": {
                    "description": "This property defines whether a radio may use DFS channels.",
                    "type": "boolean",
                    "default": true
                },
                "channel-mode": {
                    "description": "Define the ideal channel mode that the radio shall use. This can be 802.11n, 802.11ac or 802.11ax. This is just a hint for the AP. If the requested value is not supported then the AP will use the highest common denominator.",
                    "type": "string",
                    "enum": [
                        "HT",
                        "VHT",
                        "HE",
                        "EHT"
                    ],
                    "default": "HE"
                },
                "channel-width": {
                    "description": "The channel width that the radio shall use. This is just a hint for the AP. If the requested value is not supported then the AP will use the highest common denominator.",
                    "type": "integer",
                    "enum": [
                        20,
                        40,
                        80,
                        160,
                        320,
                        8080
                    ],
                    "default": 80
                },
                "require-mode": {
                    "description": "Stations that do no fulfill these HT modes will be rejected.",
                    "type": "string",
                    "enum": [
                        "HT",
                        "VHT",
                        "HE"
                    ]
                },
                "mimo": {
                    "description": "This option allows configuring the antenna pairs that shall be used. This is just a hint for the AP. If the requested value is not supported then the AP will use the highest common denominator.",
                    "type": "string",
                    "enum": [
                        "1x1",
                        "2x2",
                        "3x3",
                        "4x4",
                        "5x5",
                        "6x6",
                        "7x7",
                        "8x8"
                    ]
                },
                "tx-power": {
                    "description": "This option specifies the transmission power in dBm",
                    "type": "integer",
                    "maximum": 30,
                    "minimum": 0
                },
                "legacy-rates": {
                    "description": "Allow legacy 802.11b data rates.",
                    "type": "boolean",
                    "default": false
                },
                "beacon-interval": {
                    "description": "Beacon interval in kus (1.024 ms).",
                    "type": "integer",
                    "default": 100,
                    "maximum": 65535,
                    "minimum": 15
                },
                "maximum-clients": {
                    "description": "Set the maximum number of clients that may connect to this radio. This value is accumulative for all attached VAP interfaces.",
                    "type": "integer",
                    "example": 64
                },
                "maximum-clients-ignore-probe": {
                    "description": "Ignore probe requests if maximum-clients was reached.",
                    "type": "boolean"
                },
                "rates": {
                    "$ref": "#/$defs/radio.rates"
                },
                "he-settings": {
                    "$ref": "#/$defs/radio.he"
                },
                "hostapd-iface-raw": {
                    "description": "This array allows passing raw hostapd.conf lines.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "ap_table_expiration_time=3600",
                            "device_type=6-0050F204-1",
                            "ieee80211h=1",
                            "rssi_ignore_probe_request=-75",
                            "time_zone=EST5",
                            "uuid=12345678-9abc-def0-1234-56789abcdef0",
                            "venue_url=1:http://www.example.com/info-eng",
                            "wpa_deny_ptk0_rekey=0"
                        ]
                    }
                }
            }
        },
        "interface.vlan": {
            "description": "This section describes the vlan behaviour of a logical network interface.",
            "type": "object",
            "properties": {
                "id": {
                    "description": "This is the pvid of the vlan that shall be assigned to the interface. The individual physical network devices contained within the interface need to be told explicitly if egress traffic shall be tagged.",
                    "type": "integer",
                    "maximum": 4050
                },
                "proto": {
                    "decription": "The L2 vlan tag that shall be added (1q,1ad)",
                    "type": "string",
                    "enum": [
                        "802.1ad",
                        "802.1q"
                    ],
                    "default": "802.1q"
                }
            }
        },
        "interface.bridge": {
            "description": "This section describes the bridge behaviour of a logical network interface.",
            "type": "object",
            "properties": {
                "mtu": {
                    "description": "The MTU that shall be used by the network interface.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 256,
                    "examples": [
                        1500
                    ]
                },
                "tx-queue-len": {
                    "description": "The Transmit Queue Length is a TCP/IP stack network interface value that sets the number of packets allowed per kernel transmit queue of a network interface device.",
                    "type": "integer",
                    "examples": [
                        5000
                    ]
                },
                "isolate-ports": {
                    "description": "Isolates the bridge ports from each other.",
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "interface.ethernet": {
            "description": "This section defines the physical copper/fiber ports that are members of the interface. Network devices are referenced by their logical names.",
            "type": "object",
            "properties": {
                "select-ports": {
                    "description": "The list of physical network devices that shall be added to the interface. The names are logical ones and wildcardable. \"WAN\" will use whatever the hardwares default upstream facing port is. \"LANx\" will use the \"x'th\" downstream facing ethernet port. LAN* will use all downstream ports.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "LAN1",
                            "LAN2",
                            "LAN3",
                            "LAN4",
                            "LAN*",
                            "WAN*",
                            "*"
                        ]
                    }
                },
                "multicast": {
                    "description": "Enable multicast support.",
                    "type": "boolean",
                    "default": true
                },
                "learning": {
                    "description": "Controls whether a given port will learn MAC addresses from received traffic or not. If learning if off, the bridge will end up flooding any traffic for which it has no FDB entry. By default this flag is on.",
                    "type": "boolean",
                    "default": true
                },
                "isolate": {
                    "description": "Only allow communication with non-isolated bridge ports when enabled.",
                    "type": "boolean",
                    "default": false
                },
                "macaddr": {
                    "description": "Enforce a specific MAC to these ports.",
                    "type": "string",
                    "format": "uc-mac"
                },
                "reverse-path-filter": {
                    "description": "Reverse Path filtering is a method used by the Linux Kernel to help prevent attacks used by Spoofing IP Addresses.",
                    "type": "boolean",
                    "default": false
                },
                "vlan-tag": {
                    "description": "Shall the port have a vlan tag.",
                    "type": "string",
                    "enum": [
                        "tagged",
                        "un-tagged",
                        "auto"
                    ],
                    "default": "auto"
                }
            }
        },
        "interface.ipv4.dhcp": {
            "description": "This section describes the DHCP server configuration",
            "type": "object",
            "properties": {
                "lease-first": {
                    "description": "The last octet of the first IPv4 address in this DHCP pool.",
                    "type": "integer",
                    "examples": [
                        10
                    ]
                },
                "lease-count": {
                    "description": "The number of IPv4 addresses inside the DHCP pool.",
                    "type": "integer",
                    "examples": [
                        100
                    ]
                },
                "lease-time": {
                    "description": "How long the lease is valid before a RENEW must be issued.",
                    "type": "string",
                    "format": "uc-timeout",
                    "default": "6h"
                },
                "use-dns": {
                    "description": "The DNS server sent to clients as DHCP option 6.",
                    "type": "string",
                    "format": "uc-ip"
                }
            }
        },
        "interface.ipv4.dhcp-lease": {
            "description": "This section describes the static DHCP leases of this logical interface.",
            "type": "object",
            "properties": {
                "macaddr": {
                    "description": "The MAC address of the host that this lease shall be used for.",
                    "type": "string",
                    "format": "uc-mac",
                    "examples": [
                        "00:11:22:33:44:55"
                    ]
                },
                "static-lease-offset": {
                    "description": "The offset of the IP that shall be used in relation to the first IP in the available range.",
                    "type": "integer",
                    "examples": [
                        10
                    ]
                },
                "lease-time": {
                    "description": "How long the lease is valid before a RENEW muss ne issued.",
                    "type": "string",
                    "format": "uc-timeout",
                    "default": "6h"
                },
                "publish-hostname": {
                    "description": "Shall the hosts hostname be made available locally via DNS.",
                    "type": "boolean",
                    "default": true
                }
            }
        },
        "interface.ipv4.port-forward": {
            "description": "This section describes an IPv4 port forwarding.",
            "type": "object",
            "properties": {
                "protocol": {
                    "description": "The layer 3 protocol to match.",
                    "type": "string",
                    "enum": [
                        "tcp",
                        "udp",
                        "any"
                    ],
                    "default": "any"
                },
                "external-port": {
                    "description": "The external port(s) to forward.",
                    "type": [
                        "integer",
                        "string"
                    ],
                    "minimum": 0,
                    "maximum": 65535,
                    "format": "uc-portrange"
                },
                "internal-address": {
                    "description": "The internal IP to forward to. The address will be masked and concatenated with the effective interface subnet.",
                    "type": "string",
                    "format": "ipv4",
                    "example": "0.0.0.120"
                },
                "internal-port": {
                    "description": "The internal port to forward to. Defaults to the external port if omitted.",
                    "type": [
                        "integer",
                        "string"
                    ],
                    "minimum": 0,
                    "maximum": 65535,
                    "format": "uc-portrange"
                }
            },
            "required": [
                "external-port",
                "internal-address"
            ]
        },
        "interface.ipv4": {
            "description": "This section describes the IPv4 properties of a logical interface.",
            "type": "object",
            "properties": {
                "addressing": {
                    "description": "This option defines the method by which the IPv4 address of the interface is chosen.",
                    "type": "string",
                    "enum": [
                        "dynamic",
                        "static",
                        "none"
                    ],
                    "examples": [
                        "static"
                    ]
                },
                "subnet": {
                    "description": "This option defines the static IPv4 of the logical interface in CIDR notation. auto/24 can be used, causing the configuration layer to automatically use and address range from globals.ipv4-network.",
                    "type": "string",
                    "format": "uc-cidr4",
                    "examples": [
                        "auto/24"
                    ]
                },
                "gateway": {
                    "description": "This option defines the static IPv4 gateway of the logical interface.",
                    "type": "string",
                    "format": "ipv4",
                    "examples": [
                        "192.168.1.1"
                    ]
                },
                "send-hostname": {
                    "description": "include the devices hostname inside DHCP requests",
                    "type": "boolean",
                    "default": true,
                    "examples": [
                        true
                    ]
                },
                "use-dns": {
                    "description": "Define which DNS servers shall be used. This can either be a list of static IPv4 addresse or dhcp (use the server provided by the DHCP lease)",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "ipv4",
                        "examples": [
                            "8.8.8.8",
                            "4.4.4.4"
                        ]
                    }
                },
                "disallow-upstream-subnet": {
                    "description": "This option only applies to \"downstream\" interfaces. The downstream interface will prevent traffic going out to the listed CIDR4s. This can be used to prevent a guest / captive interface being able to communicate with RFC1918 ranges.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "uc-cidr4",
                        "examples": [
                            "192.168.0.0/16",
                            "10.0.0.0/8"
                        ]
                    }
                },
                "dhcp": {
                    "$ref": "#/$defs/interface.ipv4.dhcp"
                },
                "dhcp-leases": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ipv4.dhcp-lease"
                    }
                },
                "port-forward": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ipv4.port-forward"
                    }
                }
            }
        },
        "interface.ipv6.dhcpv6": {
            "description": "This section describes the DHCPv6 server configuration",
            "type": "object",
            "properties": {
                "mode": {
                    "description": "Specifies the DHCPv6 server operation mode. When set to \"stateless\", the system will announce router advertisements only, without offering stateful DHCPv6 service. When set to \"stateful\", emitted router advertisements will instruct clients to obtain a DHCPv6 lease. When set to \"hybrid\", clients can freely chose whether to self-assign a random address through SLAAC, whether to request an address via DHCPv6, or both. For maximum compatibility with different clients, it is recommended to use the hybrid mode. The special mode \"relay\" will instruct the unit to act as DHCPv6 relay between this interface and any of the IPv6 interfaces in \"upstream\" mode.",
                    "type": "string",
                    "enum": [
                        "hybrid",
                        "stateless",
                        "stateful",
                        "relay"
                    ]
                },
                "announce-dns": {
                    "description": "Overrides the DNS server to announce in DHCPv6 and RA messages. By default, the device will announce its own local interface address as DNS server, essentially acting as proxy for downstream clients. By specifying a non-empty list of IPv6 addresses here, this default behaviour can be overridden.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "ipv6"
                    }
                },
                "filter-prefix": {
                    "description": "Selects a specific downstream prefix or a number of downstream prefix ranges to announce in DHCPv6 and RA messages. By default, all prefixes configured on a given downstream interface are advertised. By specifying an IPv6 prefix in CIDR notation here, only prefixes covered by this CIDR are selected.",
                    "type": "string",
                    "format": "uc-cidr6",
                    "default": "::/0"
                }
            }
        },
        "interface.ipv6.port-forward": {
            "description": "This section describes an IPv6 port forwarding.",
            "type": "object",
            "properties": {
                "protocol": {
                    "description": "The layer 3 protocol to match.",
                    "type": "string",
                    "enum": [
                        "tcp",
                        "udp",
                        "any"
                    ],
                    "default": "any"
                },
                "external-port": {
                    "description": "The external port(s) to forward.",
                    "type": [
                        "integer",
                        "string"
                    ],
                    "minimum": 0,
                    "maximum": 65535,
                    "format": "uc-portrange"
                },
                "internal-address": {
                    "description": "The internal IP to forward to. The address will be masked and concatenated with the effective interface subnet.",
                    "type": "string",
                    "format": "ipv6",
                    "example": "::1234:abcd"
                },
                "internal-port": {
                    "description": "The internal port to forward to. Defaults to the external port if omitted.",
                    "type": [
                        "integer",
                        "string"
                    ],
                    "minimum": 0,
                    "maximum": 65535,
                    "format": "uc-portrange"
                }
            },
            "required": [
                "external-port",
                "internal-address"
            ]
        },
        "interface.ipv6.traffic-allow": {
            "description": "This section describes an IPv6 traffic accept rule.",
            "type": "object",
            "properties": {
                "protocol": {
                    "description": "The layer 3 protocol to match.",
                    "type": "string",
                    "default": "any"
                },
                "source-address": {
                    "description": "The source IP to allow traffic from.",
                    "type": "string",
                    "format": "uc-cidr6",
                    "example": "2001:db8:1234:abcd::/64",
                    "default": "::/0"
                },
                "source-ports": {
                    "description": "The source port(s) to accept.",
                    "type": "array",
                    "minItems": 1,
                    "items": {
                        "type": [
                            "integer",
                            "string"
                        ],
                        "minimum": 0,
                        "maximum": 65535,
                        "format": "uc-portrange"
                    }
                },
                "destination-address": {
                    "description": "The destination IP to allow traffic to. The address will be masked and concatenated with the effective interface subnet.",
                    "type": "string",
                    "format": "ipv6",
                    "example": "::1000"
                },
                "destination-ports": {
                    "description": "The destination ports to accept.",
                    "type": "array",
                    "minItems": 1,
                    "items": {
                        "type": [
                            "integer",
                            "string"
                        ],
                        "minimum": 0,
                        "maximum": 65535,
                        "format": "uc-portrange"
                    }
                }
            },
            "required": [
                "destination-address"
            ]
        },
        "interface.ipv6": {
            "description": "This section describes the IPv6 properties of a logical interface.",
            "type": "object",
            "properties": {
                "addressing": {
                    "description": "This option defines the method by which the IPv6 subnet of the interface is acquired. In static addressing mode, the specified subnet and gateway, if any, are configured on the interface in a fixed manner. Also - if a prefix size hint is specified - a prefix of the given size is allocated from each upstream received prefix delegation pool and assigned to the interface. In dynamic addressing mode, a DHCPv6 client will be launched to obtain IPv6 prefixes for the interface itself and for downstream delegation. Note that dynamic addressing usually only ever makes sense on upstream interfaces.",
                    "type": "string",
                    "enum": [
                        "dynamic",
                        "static"
                    ]
                },
                "subnet": {
                    "description": "This option defines a static IPv6 prefix in CIDR notation to set on the logical interface. A special notation \"auto/64\" can be used, causing the configuration agent to automatically allocate a suitable prefix from the IPv6 address pool specified in globals.ipv6-network. This property only applies to static addressing mode. Note that this is usually not needed due to DHCPv6-PD assisted prefix assignment.",
                    "type": "string",
                    "format": "uc-cidr6",
                    "examples": [
                        "auto/64"
                    ]
                },
                "gateway": {
                    "description": "This option defines the static IPv6 gateway of the logical interface. It only applies to static addressing mode. Note that this is usually not needed due to DHCPv6-PD assisted prefix assignment.",
                    "type": "string",
                    "format": "ipv6",
                    "examples": [
                        "2001:db8:123:456::1"
                    ]
                },
                "prefix-size": {
                    "description": "For dynamic addressing interfaces, this property specifies the prefix size to request from an upstream DHCPv6 server through prefix delegation. For static addressing interfaces, it specifies the size of the sub-prefix to allocate from the upstream-received delegation prefixes for assignment to the logical interface.",
                    "type": "integer",
                    "maximum": 64,
                    "minimum": 0
                },
                "dhcpv6": {
                    "$ref": "#/$defs/interface.ipv6.dhcpv6"
                },
                "port-forward": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ipv6.port-forward"
                    }
                },
                "traffic-allow": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ipv6.traffic-allow"
                    }
                }
            }
        },
        "interface.broad-band.wwan": {
            "description": "This Object defines the properties of a broad-band uplink.",
            "type": "object",
            "properties": {
                "protocol": {
                    "description": "This uplink uses WWAN/LTE",
                    "type": "string",
                    "const": "wwan"
                },
                "modem-type": {
                    "description": "The local protocol that the modem supports.",
                    "type": "string",
                    "enum": [
                        "qmi",
                        "mbim",
                        "wwan"
                    ]
                },
                "access-point-name": {
                    "description": "Commonly known as APN. The name of a gateway between a mobile network and the internet.",
                    "type": "string"
                },
                "authentication-type": {
                    "description": "The authentication mode that shall be used.",
                    "type": "string",
                    "enum": [
                        "none",
                        "pap",
                        "chap",
                        "pap-chap"
                    ],
                    "default": "none"
                },
                "pin-code": {
                    "description": "The PIN that shall be used to unlock the SIM card.",
                    "type": "string"
                },
                "user-name": {
                    "description": "This option is only required if an authentication-type is defined.",
                    "type": "string"
                },
                "password": {
                    "description": "This option is only required if an authentication-type is defined.",
                    "type": "string"
                },
                "packet-data-protocol": {
                    "description": "Define what kind of IP stack shall be used.",
                    "type": "string",
                    "enum": [
                        "ipv4",
                        "ipv6",
                        "dual-stack"
                    ],
                    "default": "dual-stack"
                }
            }
        },
        "interface.broad-band.pppoe": {
            "description": "This Object defines the properties of a PPPoE uplink.",
            "type": "object",
            "properties": {
                "protocol": {
                    "description": "This uplink uses PPPoE",
                    "type": "string",
                    "const": "pppoe"
                },
                "user-name": {
                    "description": "The username used to authenticate.",
                    "type": "string"
                },
                "password": {
                    "description": "The password used to authenticate.",
                    "type": "string"
                }
            }
        },
        "interface.broad-band": {
            "oneOf": [
                {
                    "$ref": "#/$defs/interface.broad-band.wwan"
                },
                {
                    "$ref": "#/$defs/interface.broad-band.pppoe"
                }
            ]
        },
        "interface.ssid.multi-psk": {
            "type": "object",
            "description": "A SSID can have multiple PSK/VID mappings. Each one of them can be bound to a specific MAC or be a wildcard.",
            "properties": {
                "mac": {
                    "type": "string",
                    "format": "uc-mac"
                },
                "key": {
                    "description": "The Pre Shared Key (PSK) that is used for encryption on the BSS when using any of the WPA-PSK modes.",
                    "type": "string",
                    "maxLength": 63,
                    "minLength": 8
                },
                "vlan-id": {
                    "type": "integer",
                    "maximum": 4096,
                    "examples": [
                        3,
                        100,
                        200,
                        4094
                    ]
                }
            }
        },
        "interface.ssid.rrm": {
            "description": "Enable 802.11k Radio Resource Management (RRM) for this BSS.",
            "type": "object",
            "properties": {
                "neighbor-reporting": {
                    "description": "Enable neighbor report via radio measurements (802.11k).",
                    "type": "boolean",
                    "default": false
                },
                "reduced-neighbor-reporting": {
                    "description": "Enable reduced neighbor reports.",
                    "type": "boolean",
                    "default": false
                },
                "lci": {
                    "description": "The content of a LCI measurement subelement",
                    "type": "string"
                },
                "civic-location": {
                    "description": "The content of a location civic measurement subelement",
                    "type": "string"
                },
                "ftm-responder": {
                    "description": "Publish fine timing measurement (FTM) responder functionality on this BSS.",
                    "type": "boolean",
                    "default": false
                },
                "stationary-ap": {
                    "description": "Stationary AP config indicates that the AP doesn't move.",
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "interface.ssid.rate-limit": {
            "description": "The UE rate-limiting configuration of this BSS.",
            "type": "object",
            "properties": {
                "ingress-rate": {
                    "description": "The ingress rate to which hosts will be shaped. Values are in Mbps",
                    "type": "integer",
                    "default": 0
                },
                "egress-rate": {
                    "description": "The egress rate to which hosts will be shaped. Values are in Mbps",
                    "type": "integer",
                    "default": 0
                }
            }
        },
        "interface.ssid.roaming": {
            "description": "Enable 802.11r Fast Roaming for this BSS.",
            "type": "object",
            "properties": {
                "message-exchange": {
                    "description": "Shall the pre authenticated message exchange happen over the air or distribution system.",
                    "type": "string",
                    "enum": [
                        "air",
                        "ds"
                    ],
                    "default": "ds"
                },
                "generate-psk": {
                    "description": "Whether to generate FT response locally for PSK networks. This avoids use of PMK-R1 push/pull from other APs with FT-PSK networks.",
                    "type": "boolean",
                    "default": false
                },
                "domain-identifier": {
                    "description": "Mobility Domain identifier (dot11FTMobilityDomainID, MDID).",
                    "type": "string",
                    "maxLength": 4,
                    "minLength": 4,
                    "examples": [
                        "abcd"
                    ]
                },
                "pmk-r0-key-holder": {
                    "description": "The pairwise master key R0. This is unique to the mobility domain and is required for fast roaming over the air. If the field is left empty a deterministic key is generated.",
                    "type": "string",
                    "example": "14:DD:20:47:14:E4,14DD204714E4,00112233445566778899aabbccddeeff"
                },
                "pmk-r1-key-holder": {
                    "description": "The pairwise master key R1. This is unique to the mobility domain and is required for fast roaming over the air. If the field is left empty a deterministic key is generated.",
                    "type": "string",
                    "example": "14:DD:20:47:14:E4,14DD204714E4,00112233445566778899aabbccddeeff"
                }
            }
        },
        "interface.ssid.radius.local-user": {
            "type": "object",
            "description": "Describes a local EAP user/psk/vid triplet.",
            "properties": {
                "mac": {
                    "type": "string",
                    "format": "uc-mac"
                },
                "user-name": {
                    "type": "string",
                    "minLength": 1
                },
                "password": {
                    "type": "string",
                    "maxLength": 63,
                    "minLength": 8
                },
                "vlan-id": {
                    "type": "integer",
                    "maximum": 4096,
                    "examples": [
                        3,
                        100,
                        200,
                        4094
                    ]
                }
            }
        },
        "interface.ssid.radius.local": {
            "description": "Describe the properties of the local Radius server inside hostapd.",
            "type": "object",
            "properties": {
                "server-identity": {
                    "description": "EAP methods that provide mechanism for authenticated server identity delivery use this value.",
                    "type": "string",
                    "default": "uCentral"
                },
                "users": {
                    "description": "Specifies a collection of local EAP user/psk/vid triplets.",
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ssid.radius.local-user"
                    }
                }
            }
        },
        "interface.ssid.radius.server": {
            "description": "Describe the properties of a Radius server.",
            "type": "object",
            "properties": {
                "host": {
                    "description": "The URI of our Radius server.",
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "port": {
                    "description": "The network port of our Radius server.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "examples": [
                        1812
                    ]
                },
                "secret": {
                    "description": "The shared Radius authentication secret.",
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "secondary": {
                    "description": "Definition of the secondary/failsafe radius server.",
                    "type": "object",
                    "properties": {
                        "host": {
                            "description": "The URI of our Radius server.",
                            "type": "string",
                            "format": "uc-host",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "port": {
                            "description": "The network port of our Radius server.",
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1812
                            ]
                        },
                        "secret": {
                            "description": "The shared Radius authentication secret.",
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        }
                    }
                },
                "request-attribute": {
                    "description": "The additional Access-Request attributes that gets sent to the server.",
                    "type": "array",
                    "items": {
                        "anyOf": [
                            {
                                "type": "object",
                                "properties": {
                                    "vendor-id": {
                                        "type": "integer",
                                        "description": "The ID of the vendor specific RADIUS attribute",
                                        "maximum": 65535,
                                        "minimum": 1
                                    },
                                    "vendor-attributes": {
                                        "type": "array",
                                        "items": {
                                            "type": "object",
                                            "description": "The numeric RADIUS attribute value",
                                            "properties": {
                                                "id": {
                                                    "type": "integer",
                                                    "description": "The ID of the vendor specific RADIUS attribute",
                                                    "maximum": 255,
                                                    "minimum": 1
                                                },
                                                "value": {
                                                    "type": "string",
                                                    "description": "The vendor specific RADIUS attribute value. This needs to be a hexadecimal string."
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer",
                                        "description": "The ID of the RADIUS attribute",
                                        "maximum": 255,
                                        "minimum": 1
                                    },
                                    "value": {
                                        "type": "integer",
                                        "description": "The numeric RADIUS attribute value",
                                        "maximum": 4294967295,
                                        "minimum": 0
                                    }
                                },
                                "examples": [
                                    {
                                        "id": 27,
                                        "value": 900
                                    },
                                    {
                                        "id": 56,
                                        "value": 1004
                                    }
                                ]
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer",
                                        "description": "The ID of the RADIUS attribute",
                                        "maximum": 255,
                                        "minimum": 1
                                    },
                                    "value": {
                                        "type": "string",
                                        "description": "The RADIUS attribute value string"
                                    }
                                },
                                "examples": [
                                    {
                                        "id": 32,
                                        "value": "My NAS ID"
                                    },
                                    {
                                        "id": 126,
                                        "value": "Example Operator"
                                    }
                                ]
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer",
                                        "description": "The ID of the RADIUS attribute",
                                        "maximum": 255,
                                        "minimum": 1
                                    },
                                    "hex-value": {
                                        "type": "string",
                                        "description": "The RADIUS attribute value string"
                                    }
                                },
                                "examples": [
                                    {
                                        "id": 32,
                                        "value": "0a0b0c0d"
                                    }
                                ]
                            }
                        ]
                    }
                }
            }
        },
        "interface.ssid.radius.health": {
            "description": "The credentials used when health check probes this radius server.",
            "type": "object",
            "properties": {
                "username": {
                    "description": "The username that gets used when doing a healthcheck on this radius server.",
                    "type": "string"
                },
                "password": {
                    "description": "The password that gets used when doing a healthcheck on this radius server.",
                    "type": "string"
                }
            }
        },
        "interface.ssid.radius": {
            "description": "When using EAP encryption we need to provide the required information allowing us to connect to the AAA servers.",
            "type": "object",
            "properties": {
                "nas-identifier": {
                    "description": "NAS-Identifier string for RADIUS messages. When used, this should be unique to the NAS within the scope of the RADIUS server.",
                    "type": "string"
                },
                "chargeable-user-id": {
                    "description": "This will enable support for Chargeable-User-Identity (RFC 4372).",
                    "type": "boolean",
                    "default": false
                },
                "local": {
                    "$ref": "#/$defs/interface.ssid.radius.local"
                },
                "dynamic-authorization": {
                    "description": "Dynamic Authorization Extensions (DAE) is an extension to Radius.",
                    "type": "object",
                    "properties": {
                        "host": {
                            "description": "The IP of the DAE client.",
                            "type": "string",
                            "format": "uc-ip",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "port": {
                            "description": "The network port that the DAE client can connet on.",
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1812
                            ]
                        },
                        "secret": {
                            "description": "The shared DAE authentication secret.",
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        }
                    }
                },
                "authentication": {
                    "allOf": [
                        {
                            "$ref": "#/$defs/interface.ssid.radius.server"
                        },
                        {
                            "type": "object",
                            "properties": {
                                "mac-filter": {
                                    "description": "Should the radius server be used for MAC address ACL.",
                                    "type": "boolean",
                                    "default": false
                                }
                            }
                        }
                    ]
                },
                "accounting": {
                    "allOf": [
                        {
                            "$ref": "#/$defs/interface.ssid.radius.server"
                        },
                        {
                            "type": "object",
                            "properties": {
                                "interval": {
                                    "description": "The interim accounting update interval. This value is defined in seconds.",
                                    "type": "integer",
                                    "maximum": 600,
                                    "minimum": 60,
                                    "default": 60
                                }
                            }
                        }
                    ]
                },
                "health": {
                    "$ref": "#/$defs/interface.ssid.radius.health"
                }
            }
        },
        "interface.ssid.certificates": {
            "description": "When running a local EAP server or using STA/MESH to connect to another BSS a set of certificates is required.",
            "type": "object",
            "properties": {
                "use-local-certificates": {
                    "description": "The device will use its local certificate bundle for the TLS setup and ignores all other certificate options in this section.",
                    "type": "boolean",
                    "default": false
                },
                "ca-certificate": {
                    "description": "The local servers CA bundle.",
                    "type": "string"
                },
                "certificate": {
                    "description": "The local servers certificate.",
                    "type": "string"
                },
                "private-key": {
                    "description": "The local servers private key/",
                    "type": "string"
                },
                "private-key-password": {
                    "description": "The password required to read the private key.",
                    "type": "string"
                }
            }
        },
        "interface.ssid.pass-point": {
            "description": "Enable Hotspot 2.0 support.",
            "type": "object",
            "properties": {
                "venue-name": {
                    "description": "This parameter can be used to configure one or more Venue Name Duples for Venue Name ANQP information.",
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "venue-group": {
                    "description": "The available values are defined in 802.11u.",
                    "type": "integer",
                    "maximum": 32
                },
                "venue-type": {
                    "description": "The available values are defined in IEEE Std 802.11u-2011, 7.3.1.34",
                    "type": "integer",
                    "maximum": 32
                },
                "venue-url": {
                    "description": "This parameter can be used to configure one or more Venue URL Duples to provide additional information corresponding to Venue Name information.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "uri"
                    }
                },
                "auth-type": {
                    "description": "This parameter indicates what type of network authentication is used in the network.",
                    "type": "object",
                    "properties": {
                        "type": {
                            "description": "Specifies the specific network authentication type in use.",
                            "type": "string",
                            "enum": [
                                "terms-and-conditions",
                                "online-enrollment",
                                "http-redirection",
                                "dns-redirection"
                            ]
                        },
                        "uri": {
                            "description": "Specifies the redirect URL applicable to the indicated authentication type.",
                            "type": "string",
                            "format": "uri",
                            "examples": [
                                "https://operator.example.org/wireless-access/terms-and-conditions.html",
                                "http://www.example.com/redirect/me/here/"
                            ]
                        }
                    },
                    "minLength": 2,
                    "maxLength": 2
                },
                "domain-name": {
                    "description": "The IEEE 802.11u Domain Name.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "hostname"
                    }
                },
                "nai-realm": {
                    "description": "NAI Realm information",
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "osen": {
                    "description": "OSU Server-Only Authenticated L2 Encryption Network;",
                    "type": "boolean"
                },
                "anqp-domain": {
                    "description": "ANQP Domain ID, An identifier for a set of APs in an ESS that share the same common ANQP information.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 0
                },
                "anqp-3gpp-cell-net": {
                    "description": "The ANQP 3GPP Cellular Network information.",
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "friendly-name": {
                    "description": "This parameter can be used to configure one or more Operator Friendly Name Duples.",
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "access-network-type": {
                    "description": "Indicate the type of network. This is part of the interworking IE.",
                    "type": "integer",
                    "maximum": 15,
                    "default": 0
                },
                "internet": {
                    "description": "Whether the network provides connectivity to the Internet",
                    "type": "boolean",
                    "default": true
                },
                "asra": {
                    "description": "Additional Step Required for Access.",
                    "type": "boolean",
                    "default": false
                },
                "esr": {
                    "description": "Emergency services reachable.",
                    "type": "boolean",
                    "default": false
                },
                "uesa": {
                    "description": "Unauthenticated emergency service accessible.",
                    "type": "boolean",
                    "default": false
                },
                "hessid": {
                    "description": "Homogeneous ESS identifier",
                    "type": "string",
                    "example": "00:11:22:33:44:55"
                },
                "roaming-consortium": {
                    "description": "Roaming Consortium OIs can be configured here. Each OI is between 3 and 15 octets and is configured as a hexstring.",
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "disable-dgaf": {
                    "description": "Disable Downstream Group-Addressed Forwarding. This can be used to configure a network where no group-addressed frames are allowed.",
                    "type": "boolean",
                    "default": false
                },
                "ipaddr-type-available": {
                    "description": "IP Address Type Availability.",
                    "type": "integer",
                    "maximum": 255
                },
                "connection-capability": {
                    "description": "This can be used to advertise what type of IP traffic can be sent through the hotspot.",
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "icons": {
                    "description": "The operator icons.",
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "width": {
                                "type": "integer",
                                "description": "The width of the operator icon in pixel",
                                "examples": [
                                    64
                                ]
                            },
                            "height": {
                                "type": "integer",
                                "description": "The height of the operator icon in pixel",
                                "examples": [
                                    64
                                ]
                            },
                            "type": {
                                "type": "string",
                                "description": "The mimetype of the operator icon",
                                "examples": [
                                    "image/png"
                                ]
                            },
                            "icon": {
                                "type": "string",
                                "format": "uc-base64",
                                "description": "The base64 encoded image"
                            },
                            "language": {
                                "type": "string",
                                "description": "ISO 639-2 language code of the icon",
                                "pattern": "^[a-z][a-z][a-z]$",
                                "examples": [
                                    "eng",
                                    "fre",
                                    "ger",
                                    "ita"
                                ]
                            }
                        },
                        "examples": [
                            {
                                "width": 32,
                                "height": 32,
                                "type": "image/png",
                                "language": "eng",
                                "icon": "R0lGODlhEAAQAMQAAORHHOVSKudfOulrSOp3WOyDZu6QdvCchPGolfO0o/XBs/fNwfjZ0frl3/zy7////wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAkAABAALAAAAAAQABAAAAVVICSOZGlCQAosJ6mu7fiyZeKqNKToQGDsM8hBADgUXoGAiqhSvp5QAnQKGIgUhwFUYLCVDFCrKUE1lBavAViFIDlTImbKC5Gm2hB0SlBCBMQiB0UjIQA7"
                            }
                        ]
                    }
                },
                "wan-metrics": {
                    "description": "A description of the wan metric offered by this device.",
                    "type": "object",
                    "properties": {
                        "info": {
                            "description": "The state of the devices uplink",
                            "type": "string",
                            "enum": [
                                "up",
                                "down",
                                "testing"
                            ]
                        },
                        "downlink": {
                            "description": "Estimate of WAN backhaul link current downlink speed in kbps.",
                            "type": "integer"
                        },
                        "uplink": {
                            "description": "Estimate of WAN backhaul link current uplink speed in kbps.",
                            "type": "integer"
                        }
                    }
                }
            }
        },
        "interface.ssid.quality-thresholds": {
            "description": "The thresholds that need to be meet for a clien association to be allowed.",
            "type": "object",
            "properties": {
                "probe-request-rssi": {
                    "description": "Probe requests will be ignored if the rssi is below this threshold.",
                    "type": "integer"
                },
                "association-request-rssi": {
                    "description": "Association requests will be denied if the rssi is below this threshold.",
                    "type": "integer"
                },
                "client-kick-rssi": {
                    "description": "Clients will get kicked if their SNR drops below this value.",
                    "type": "integer"
                },
                "client-kick-ban-time": {
                    "description": "The duration that a client is banned from re-joining after it was kicked.",
                    "type": "integer",
                    "default": 0
                }
            }
        },
        "interface.ssid.acl": {
            "description": "The MAC ACL that defines which clients are allowed or denied to associations.",
            "type": "object",
            "properties": {
                "mode": {
                    "description": "Defines if this is an allow or deny list.",
                    "type": "string",
                    "enum": [
                        "allow",
                        "deny"
                    ]
                },
                "mac-address": {
                    "description": "Association requests will be denied if the rssi is below this threshold.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "uc-mac"
                    }
                }
            }
        },
        "service.captive.click": {
            "description": "This section can be used to setup a captive portal on the AP with a click-to-continue splash page.",
            "type": "object",
            "properties": {
                "auth-mode": {
                    "description": "This field must be set to 'click-to-continue'",
                    "type": "string",
                    "const": "click-to-continue"
                }
            }
        },
        "service.captive.radius": {
            "description": "This section can be used to setup a captive portal on the AP with a click-to-continue splash page.",
            "type": "object",
            "properties": {
                "auth-mode": {
                    "description": "This field must be set to 'radius'",
                    "type": "string",
                    "const": "radius"
                },
                "auth-server": {
                    "description": "The URI of our Radius Authentication server.",
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "auth-port": {
                    "description": "The network port of our Radius Authentication server.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 1812
                },
                "auth-secret": {
                    "description": "The shared Radius authentication Authentication secret.",
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "acct-server": {
                    "description": "The URI of our Radius Authentication server.",
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "acct-port": {
                    "description": "The network port of our Radius Authentication server.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 1812
                },
                "acct-secret": {
                    "description": "The shared Radius authentication Authentication secret.",
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "acct-interval": {
                    "description": "The timeout used for interim messages.",
                    "type": "integer",
                    "default": 600
                }
            }
        },
        "service.captive.credentials": {
            "description": "This section can be used to setup a captive portal on the AP with a credentials splash page.",
            "type": "object",
            "properties": {
                "auth-mode": {
                    "description": "This field must be set to 'credentials'",
                    "type": "string",
                    "const": "credentials"
                },
                "credentials": {
                    "description": "The list of local username/password pairs that can be used to login.",
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "username": {
                                "type": "string"
                            },
                            "password": {
                                "type": "string"
                            }
                        }
                    }
                }
            }
        },
        "service.captive.uam": {
            "description": "This section can be used to setup a captive portal on the AP with a remote UAM server.",
            "type": "object",
            "properties": {
                "auth-mode": {
                    "description": "This field must be set to 'uam'",
                    "type": "string",
                    "const": "uam"
                },
                "uam-port": {
                    "description": "The local UAM port.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 3990
                },
                "uam-secret": {
                    "description": "The pre-shared UAM secret.",
                    "type": "string"
                },
                "uam-server": {
                    "description": "The fqdn of the UAM server.",
                    "type": "string"
                },
                "nasid": {
                    "description": "The NASID that gets sent to the UAM server.",
                    "type": "string"
                },
                "nasmac": {
                    "description": "The NAS MAC that gets send to the UAM server. The devices serial is used if this value is not provided.",
                    "type": "string"
                },
                "auth-server": {
                    "description": "The URI of our Radius Authentication server.",
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "auth-port": {
                    "description": "The network port of our Radius Authentication server.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 1812
                },
                "auth-secret": {
                    "description": "The shared Radius authentication Authentication secret.",
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "acct-server": {
                    "description": "The URI of our Radius Authentication server.",
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "acct-port": {
                    "description": "The network port of our Radius Authentication server.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 1812
                },
                "acct-secret": {
                    "description": "The shared Radius authentication Authentication secret.",
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "acct-interval": {
                    "description": "The timeout used for interim messages.",
                    "type": "integer",
                    "default": 600
                },
                "ssid": {
                    "description": "The name of the SSID that shall be sent as part of the UAM redirect.",
                    "type": "string"
                },
                "mac-format": {
                    "description": "Defines the format used to send the MAC address inside AAA frames.",
                    "type": "string",
                    "enum": [
                        "aabbccddeeff",
                        "aa-bb-cc-dd-ee-ff",
                        "aa:bb:cc:dd:ee:ff",
                        "AABBCCDDEEFF",
                        "AA:BB:CC:DD:EE:FF",
                        "AA-BB-CC-DD-EE-FF"
                    ]
                },
                "final-redirect-url": {
                    "description": "Define the behaviour off the final redirect. Default will honour \"userurl\" and fallback to \"local\". Alternatively it is possible to force a redirect to the \"UAM\" or \"local\" URL.",
                    "type": "string",
                    "enum": [
                        "default",
                        "uam"
                    ]
                },
                "mac-auth": {
                    "description": "Try to authenticate new clients using macauth.",
                    "type": "boolean",
                    "default": "default"
                },
                "radius-gw-proxy": {
                    "description": "Tunnel all radius traffic via the radius-gw-proxy.",
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "service.captive": {
            "allOf": [
                {
                    "oneOf": [
                        {
                            "$ref": "#/$defs/service.captive.click"
                        },
                        {
                            "$ref": "#/$defs/service.captive.radius"
                        },
                        {
                            "$ref": "#/$defs/service.captive.credentials"
                        },
                        {
                            "$ref": "#/$defs/service.captive.uam"
                        }
                    ]
                },
                {
                    "type": "object",
                    "properties": {
                        "walled-garden-fqdn": {
                            "description": "The list of FQDNs that a non-authenticated client is allowed to connect to.",
                            "type": "array",
                            "items": {
                                "type": "string"
                            }
                        },
                        "walled-garden-ipaddr": {
                            "description": "The list of IP addresses that a non-authenticated client is allowed to connect to.",
                            "type": "array",
                            "items": {
                                "type": "string",
                                "format": "uc-ip"
                            }
                        },
                        "web-root": {
                            "description": "A base64 encoded TAR file with the custom web-root.",
                            "type": "string",
                            "format": "uc-base64"
                        },
                        "idle-timeout": {
                            "description": "How long may a client be idle before getting removed.",
                            "type": "integer",
                            "default": 600
                        },
                        "session-timeout": {
                            "description": "How long may a client be idle before getting removed.",
                            "type": "integer"
                        }
                    }
                }
            ]
        },
        "interface.ssid": {
            "description": "A device has certain properties that describe its identity and location. These properties are described inside this object.",
            "type": "object",
            "properties": {
                "purpose": {
                    "description": "An SSID can have a special purpose such as the hidden on-boarding BSS. All purposes other than \"user-defined\" are static pre-defined configurations.",
                    "type": "string",
                    "enum": [
                        "user-defined",
                        "onboarding-ap",
                        "onboarding-sta"
                    ],
                    "default": "user-defined"
                },
                "name": {
                    "description": "The broadcasted SSID of the wireless network and for for managed mode the SSID of the network you\u2019re connecting to",
                    "type": "string",
                    "maxLength": 32,
                    "minLength": 1
                },
                "wifi-bands": {
                    "description": "The band that the SSID should be broadcasted on. The configuration layer will use the first matching band.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "2G",
                            "5G",
                            "5G-lower",
                            "5G-upper",
                            "6G"
                        ]
                    }
                },
                "bss-mode": {
                    "description": "Selects the operation mode of the wireless network interface controller.",
                    "type": "string",
                    "enum": [
                        "ap",
                        "sta",
                        "mesh",
                        "wds-ap",
                        "wds-sta",
                        "wds-repeater"
                    ],
                    "default": "ap"
                },
                "bssid": {
                    "description": "Override the BSSID of the network, only applicable in adhoc or sta mode.",
                    "type": "string",
                    "format": "uc-mac"
                },
                "hidden-ssid": {
                    "description": "Disables the broadcasting of beacon frames if set to 1 and,in doing so, hides the ESSID.",
                    "type": "boolean"
                },
                "isolate-clients": {
                    "description": "Isolates wireless clients from each other on this BSS.",
                    "type": "boolean"
                },
                "strict-forwarding": {
                    "description": "Isolate the BSS from all other members on the bridge apart from the first wired port.",
                    "type": "boolean",
                    "default": false
                },
                "power-save": {
                    "description": "Unscheduled Automatic Power Save Delivery.",
                    "type": "boolean"
                },
                "rts-threshold": {
                    "description": "Set the RTS/CTS threshold of the BSS.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1
                },
                "broadcast-time": {
                    "description": "This option will make the unit braodcast the time inside its beacons.",
                    "type": "boolean"
                },
                "unicast-conversion": {
                    "description": "Convert multicast traffic to unicast on this BSS.",
                    "type": "boolean",
                    "default": true
                },
                "services": {
                    "description": "The services that shall be offered on this logical interface. These are just strings such as \"wifi-steering\"",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "wifi-steering"
                        ]
                    }
                },
                "dtim-period": {
                    "description": "Set the DTIM (delivery traffic information message) period. There will be one DTIM per this many beacon frames. This may be set between 1 and 255. This option only has an effect on ap wifi-ifaces.",
                    "type": "integer",
                    "default": 2,
                    "maximum": 255,
                    "minimum": 1
                },
                "maximum-clients": {
                    "description": "Set the maximum number of clients that may connect to this VAP.",
                    "type": "integer",
                    "example": 64
                },
                "proxy-arp": {
                    "description": "Proxy ARP is the technique in which the host router, answers ARP requests intended for another machine.",
                    "type": "boolean",
                    "default": true
                },
                "disassoc-low-ack": {
                    "decription": "Disassociate stations based on excessive transmission failures or other indications of connection loss.",
                    "type": "boolean",
                    "default": false
                },
                "vendor-elements": {
                    "decription": "This option allows embedding custom vendor specific IEs inside the beacons of a BSS in AP mode.",
                    "type": "string"
                },
                "tip-information-element": {
                    "decription": "The device will broadcast the TIP vendor IE inside its beacons if this option is enabled.",
                    "type": "boolean",
                    "default": true
                },
                "fils-discovery-interval": {
                    "description": "The maximum interval for FILS discovery announcement frames. This is a condensed beacon used in 6GHz channels for passive BSS discovery.",
                    "type": "integer",
                    "default": 20,
                    "maximum": 20
                },
                "encryption": {
                    "$ref": "#/$defs/interface.ssid.encryption"
                },
                "multi-psk": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ssid.multi-psk"
                    }
                },
                "rrm": {
                    "$ref": "#/$defs/interface.ssid.rrm"
                },
                "rate-limit": {
                    "$ref": "#/$defs/interface.ssid.rate-limit"
                },
                "roaming": {
                    "anyOf": [
                        {
                            "$ref": "#/$defs/interface.ssid.roaming"
                        },
                        {
                            "description": "Enable 802.11r Fast Roaming for this BSS. This will enable \"auto\" mode which will work for most scenarios.",
                            "type": "boolean"
                        }
                    ]
                },
                "radius": {
                    "$ref": "#/$defs/interface.ssid.radius"
                },
                "certificates": {
                    "$ref": "#/$defs/interface.ssid.certificates"
                },
                "pass-point": {
                    "$ref": "#/$defs/interface.ssid.pass-point"
                },
                "quality-thresholds": {
                    "$ref": "#/$defs/interface.ssid.quality-thresholds"
                },
                "access-control-list": {
                    "$ref": "#/$defs/interface.ssid.acl"
                },
                "captive": {
                    "$ref": "#/$defs/service.captive"
                },
                "vlan-awareness": {
                    "description": "Setup additional VLANs inside the bridge",
                    "type": "object",
                    "properties": {
                        "first": {
                            "type": "integer"
                        },
                        "last": {
                            "type": "integer"
                        }
                    }
                },
                "hostapd-bss-raw": {
                    "description": "This array allows passing raw hostapd.conf lines.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "ap_table_expiration_time=3600",
                            "device_type=6-0050F204-1",
                            "ieee80211h=1",
                            "rssi_ignore_probe_request=-75",
                            "time_zone=EST5",
                            "uuid=12345678-9abc-def0-1234-56789abcdef0",
                            "venue_url=1:http://www.example.com/info-eng",
                            "wpa_deny_ptk0_rekey=0"
                        ]
                    }
                }
            }
        },
        "interface.tunnel.mesh": {
            "description": "This Object defines the properties of a mesh interface overlay.",
            "type": "object",
            "properties": {
                "proto": {
                    "description": "This field must be set to mesh.",
                    "type": "string",
                    "const": "mesh"
                }
            }
        },
        "interface.tunnel.vxlan": {
            "description": "This Object defines the properties of a vxlan tunnel.",
            "type": "object",
            "properties": {
                "proto": {
                    "description": "This field must be set to vxlan.",
                    "type": "string",
                    "const": "vxlan"
                },
                "peer-address": {
                    "description": "This is the IP address of the remote host, that the VXLAN tunnel shall be established with.",
                    "type": "string",
                    "format": "ipv4",
                    "example": "192.168.100.1"
                },
                "peer-port": {
                    "description": "The network port that shall be used to establish the VXLAN tunnel.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1,
                    "examples": [
                        4789
                    ]
                }
            }
        },
        "interface.tunnel.l2tp": {
            "description": "This Object defines the properties of a l2tp tunnel.",
            "type": "object",
            "properties": {
                "proto": {
                    "description": "This field must be set to vxlan.",
                    "type": "string",
                    "const": "l2tp"
                },
                "server": {
                    "description": "This is the IP address of the remote host, that the L2TP tunnel shall be established with.",
                    "type": "string",
                    "format": "ipv4",
                    "example": "192.168.100.1"
                },
                "user-name": {
                    "description": "The username used to authenticate.",
                    "type": "string"
                },
                "password": {
                    "description": "The password used to authenticate.",
                    "type": "string"
                }
            }
        },
        "interface.tunnel.gre": {
            "description": "This Object defines the properties of a GRE tunnel.",
            "type": "object",
            "properties": {
                "proto": {
                    "description": "This field must be set to gre.",
                    "type": "string",
                    "const": "gre"
                },
                "peer-address": {
                    "description": "This is the IP address of the remote host, that the GRE tunnel shall be established with.",
                    "type": "string",
                    "format": "ipv4",
                    "example": "192.168.100.1"
                },
                "dhcp-healthcheck": {
                    "description": "Healthcheck will probe if the remote peer replies to DHCP discovery without sending an ACK.",
                    "type": "boolean",
                    "default": false
                },
                "dont-fragment": {
                    "description": "Set \u201cDon't Fragment\u201d flag on encapsulated packets.",
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "interface.tunnel.gre6": {
            "description": "This Object defines the properties of a GREv6 tunnel.",
            "type": "object",
            "properties": {
                "proto": {
                    "description": "This field must be set to gre6.",
                    "type": "string",
                    "const": "gre6"
                },
                "peer-address": {
                    "description": "This is the IPv6 address of the remote host, that the GRE tunnel shall be established with.",
                    "type": "string",
                    "format": "ipv6",
                    "example": "2405:200:802:600:61::1"
                },
                "dhcp-healthcheck": {
                    "description": "Healthcheck will probe if the remote peer replies to DHCP discovery without sending an ACK.",
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "interface.tunnel": {
            "oneOf": [
                {
                    "$ref": "#/$defs/interface.tunnel.mesh"
                },
                {
                    "$ref": "#/$defs/interface.tunnel.vxlan"
                },
                {
                    "$ref": "#/$defs/interface.tunnel.l2tp"
                },
                {
                    "$ref": "#/$defs/interface.tunnel.gre"
                },
                {
                    "$ref": "#/$defs/interface.tunnel.gre6"
                }
            ]
        },
        "interface": {
            "description": "This section describes the logical network interfaces of the device. Interfaces as their primary have a role that is upstream, downstream, guest, ....",
            "type": "object",
            "properties": {
                "name": {
                    "description": "This is a free text field, stating the administrative name of the interface. It may contain spaces and special characters.",
                    "type": "string",
                    "examples": [
                        "LAN"
                    ]
                },
                "role": {
                    "description": "The role defines if the interface is upstream or downstream facing.",
                    "type": "string",
                    "enum": [
                        "upstream",
                        "downstream"
                    ]
                },
                "isolate-hosts": {
                    "description": "This option makes sure that any traffic leaving this interface is isolated and all local IP ranges are blocked. It essentially enforces \"guest network\" firewall settings.",
                    "type": "boolean"
                },
                "metric": {
                    "description": "The routing metric of this logical interface. Lower values have higher priority.",
                    "type": "integer",
                    "maximum": 4294967295,
                    "minimum": 0
                },
                "mtu": {
                    "description": "The MTU of this logical interface.",
                    "type": "integer",
                    "maximum": 1500,
                    "minimum": 1280
                },
                "services": {
                    "description": "The services that shall be offered on this logical interface. These are just strings such as \"ssh\", \"lldp\", \"mdns\"",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "ssh",
                            "lldp"
                        ]
                    }
                },
                "vlan-awareness": {
                    "description": "Setup additional VLANs inside the bridge",
                    "type": "object",
                    "properties": {
                        "first": {
                            "type": "integer"
                        },
                        "last": {
                            "type": "integer"
                        }
                    }
                },
                "ieee8021x-ports": {
                    "description": "The list of physical network devices that shall serve .1x for this interface.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "LAN1",
                            "LAN2",
                            "LAN3",
                            "LAN4",
                            "LAN*",
                            "WAN*",
                            "*"
                        ]
                    }
                },
                "vlan": {
                    "$ref": "#/$defs/interface.vlan"
                },
                "bridge": {
                    "$ref": "#/$defs/interface.bridge"
                },
                "ethernet": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ethernet"
                    }
                },
                "ipv4": {
                    "$ref": "#/$defs/interface.ipv4"
                },
                "ipv6": {
                    "$ref": "#/$defs/interface.ipv6"
                },
                "broad-band": {
                    "$ref": "#/$defs/interface.broad-band"
                },
                "ssids": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ssid"
                    }
                },
                "tunnel": {
                    "$ref": "#/$defs/interface.tunnel"
                }
            }
        },
        "service.lldp": {
            "type": "object",
            "properties": {
                "describe": {
                    "description": "The LLDP description field. If set to \"auto\" it will be derived from unit.name.",
                    "type": "string",
                    "default": "uCentral Access Point"
                },
                "location": {
                    "description": "The LLDP location field. If set to \"auto\" it will be derived from unit.location.",
                    "type": "string",
                    "default": "uCentral Network"
                }
            }
        },
        "service.ssh": {
            "description": "This section can be used to setup a SSH server on the AP.",
            "type": "object",
            "properties": {
                "port": {
                    "description": "This option defines which port the SSH server shall be available on.",
                    "type": "integer",
                    "maximum": 65535,
                    "default": 22
                },
                "authorized-keys": {
                    "description": "This allows the upload of public ssh keys. Keys need to be seperated by a newline.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAAAgQC0ghdSd2D2y08TFowZLMZn3x1/Djw3BkNsIeHt/Z+RaXwvfV1NQAnNdaOngMT/3uf5jZtYxhpl+dbZtRhoUPRvKflKBeFHYBqjZVzD3r4ns2Ofm2UpHlbdOpMuy9oeTSCeF0IKZZ6szpkvSirQogeP2fe9KRkzQpiza6YxxaJlWw== user@example",
                            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIJ4FDjyCsg+1Mh2C5G7ibR3z0Kw1dU57kfXebLRwS6CL bob@work",
                            "ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBP/JpJ/KHtKKImzISBDwLO0/EwytIr4pGZQXcP6GCSHchLMyfjf147KNlF9gC+3FibzqKH02EiQspVhRgfuK6y0= alice@home"
                        ]
                    }
                },
                "password-authentication": {
                    "description": "This option defines if password authentication shall be enabled. If set to false, only ssh key based authentication is possible.",
                    "type": "boolean",
                    "default": true
                }
            }
        },
        "service.ntp": {
            "type": "object",
            "description": "This section can be used to setup the upstream NTP servers.",
            "properties": {
                "servers": {
                    "description": "This is an array of URL/IP of the upstream NTP servers that the unit shall use to acquire its current time.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "uc-host"
                    },
                    "examples": [
                        "0.openwrt.pool.ntp.org"
                    ]
                }
            }
        },
        "service.mdns": {
            "description": "This section can be used to configure the MDNS server.",
            "type": "object",
            "properties": {
                "enable": {
                    "description": "Enable this option if you would like to enable the MDNS server on the unit.",
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "service.rtty": {
            "description": "This section can be used to setup a persistent connection to a rTTY server.",
            "type": "object",
            "properties": {
                "host": {
                    "description": "The server that the device shall connect to.",
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "port": {
                    "description": "This option defines the port that device shall connect to.",
                    "type": "integer",
                    "maximum": 65535,
                    "default": 5912
                },
                "token": {
                    "description": "The security token that shall be used to authenticate with the server.",
                    "type": "string",
                    "maxLength": 32,
                    "minLength": 32,
                    "examples": [
                        "01234567890123456789012345678901"
                    ]
                },
                "mutual-tls": {
                    "description": "Shall the connection enforce mTLS",
                    "type": "boolean",
                    "default": true
                }
            }
        },
        "service.log": {
            "description": "This section can be used to configure remote syslog support.",
            "type": "object",
            "properties": {
                "host": {
                    "description": "IP address of a syslog server to which the log messages should be sent in addition to the local destination.",
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "port": {
                    "description": "Port number of the remote syslog server specified with log_ip.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 100,
                    "examples": [
                        2000
                    ]
                },
                "proto": {
                    "description": "Sets the protocol to use for the connection, either tcp or udp.",
                    "type": "string",
                    "enum": [
                        "tcp",
                        "udp"
                    ],
                    "default": "udp"
                },
                "size": {
                    "description": "Size of the file based log buffer in KiB. This value is used as the fallback value for log_buffer_size if the latter is not specified.",
                    "type": "integer",
                    "minimum": 32,
                    "default": 1000
                },
                "priority": {
                    "description": "Filter messages by their log priority. the value maps directly to the 0-7 range used by syslog.",
                    "type": "integer",
                    "minimum": 0,
                    "default": 7
                }
            }
        },
        "service.http": {
            "description": "Enable the webserver with the on-boarding webui",
            "type": "object",
            "properties": {
                "http-port": {
                    "description": "The port that the HTTP server should run on.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1,
                    "default": 80
                }
            }
        },
        "service.igmp": {
            "description": "This section allows enabling the IGMP/Multicast proxy",
            "type": "object",
            "properties": {
                "enable": {
                    "description": "This option defines if the IGMP/Multicast proxy shall be enabled on the device.",
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "service.ieee8021x": {
            "description": "This section allows enabling wired ieee802.1X",
            "type": "object",
            "properties": {
                "mode": {
                    "description": "This field must be set to 'radius or user'",
                    "type": "string",
                    "enum": [
                        "radius",
                        "user"
                    ]
                },
                "select-ports": {
                    "description": "Specifies a list of ports that we want to filter.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            {
                                "LAN1": null
                            }
                        ]
                    }
                },
                "users": {
                    "description": "Specifies a collection of local EAP user/psk/vid triplets.",
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ssid.radius.local-user"
                    }
                },
                "radius": {
                    "description": "Specifies the information about radius account authentication and accounting",
                    "type": "object",
                    "properties": {
                        "nas-identifier": {
                            "description": "NAS-Identifier string for RADIUS messages. When used, this should be unique to the NAS within the scope of the RADIUS server.",
                            "type": "string"
                        },
                        "auth-server-addr": {
                            "description": "The URI of our Radius server.",
                            "type": "string",
                            "format": "uc-host",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "auth-server-port": {
                            "description": "The network port of our Radius server.",
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1812
                            ]
                        },
                        "auth-server-secret": {
                            "description": "The shared Radius authentication secret.",
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        },
                        "acct-server-addr": {
                            "description": "The URI of our Radius server.",
                            "type": "string",
                            "format": "uc-host",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "acct-server-port": {
                            "description": "The network port of our Radius server.",
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1813
                            ]
                        },
                        "acct-server-secret": {
                            "description": "The shared Radius accounting secret.",
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        },
                        "coa-server-addr": {
                            "description": "The URI of our Radius server.",
                            "type": "string",
                            "format": "uc-host",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "coa-server-port": {
                            "description": "The network port of our Radius server.",
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1814
                            ]
                        },
                        "coa-server-secret": {
                            "description": "The shared Radius accounting secret.",
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        },
                        "mac-address-bypass": {
                            "description": "Trigger mac-auth when a new ARP is learned.",
                            "type": "boolean"
                        }
                    }
                }
            }
        },
        "service.radius-proxy": {
            "description": "This section can be used to setup a radius security proxy instance (radsecproxy).",
            "type": "object",
            "properties": {
                "proxy-secret": {
                    "description": "The radius secret used to communicate with the proxy.",
                    "type": "string",
                    "default": "secret"
                },
                "realms": {
                    "description": "The various realms that we can proxy to.",
                    "type": "array",
                    "items": {
                        "anyOf": [
                            {
                                "type": "object",
                                "properties": {
                                    "protocol": {
                                        "description": "Defines whether the real should use radsec or normal radius.",
                                        "type": "string",
                                        "enum": [
                                            "radsec"
                                        ],
                                        "default": "radsec"
                                    },
                                    "realm": {
                                        "description": "The realm that that this server shall be used for.",
                                        "type": "array",
                                        "items": {
                                            "type": "string",
                                            "default": "*"
                                        }
                                    },
                                    "auto-discover": {
                                        "description": "Auto discover radsec server address via realm DNS NAPTR record.",
                                        "type": "boolean",
                                        "default": false
                                    },
                                    "host": {
                                        "description": "The remote proxy server that the device shall connect to.",
                                        "type": "string",
                                        "format": "uc-host",
                                        "examples": [
                                            "192.168.1.10"
                                        ]
                                    },
                                    "port": {
                                        "description": "The remote proxy port that the device shall connect to.",
                                        "type": "integer",
                                        "maximum": 65535,
                                        "default": 2083
                                    },
                                    "secret": {
                                        "description": "The radius secret that will be used for the connection.",
                                        "type": "string"
                                    },
                                    "use-local-certificates": {
                                        "description": "The device will use its local certificate bundle for the TLS setup and ignores all other certificate options in this section.",
                                        "type": "boolean",
                                        "default": false
                                    },
                                    "ca-certificate": {
                                        "description": "The local servers CA bundle.",
                                        "type": "string"
                                    },
                                    "certificate": {
                                        "description": "The local servers certificate.",
                                        "type": "string"
                                    },
                                    "private-key": {
                                        "description": "The local servers private key/",
                                        "type": "string"
                                    },
                                    "private-key-password": {
                                        "description": "The password required to read the private key.",
                                        "type": "string"
                                    }
                                }
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "protocol": {
                                        "description": "Defines whether the real should use radsec or normal radius.",
                                        "type": "string",
                                        "enum": [
                                            "radius"
                                        ]
                                    },
                                    "realm": {
                                        "description": "The realm that that this server shall be used for.",
                                        "type": "array",
                                        "items": {
                                            "type": "string",
                                            "default": "*"
                                        }
                                    },
                                    "auth-server": {
                                        "description": "The URI of our Radius server.",
                                        "type": "string",
                                        "format": "uc-host",
                                        "examples": [
                                            "192.168.1.10"
                                        ]
                                    },
                                    "auth-port": {
                                        "description": "The network port of our Radius server.",
                                        "type": "integer",
                                        "maximum": 65535,
                                        "minimum": 1024,
                                        "examples": [
                                            1812
                                        ]
                                    },
                                    "auth-secret": {
                                        "description": "The shared Radius authentication secret.",
                                        "type": "string",
                                        "examples": [
                                            "secret"
                                        ]
                                    },
                                    "acct-server": {
                                        "description": "The URI of our Radius server.",
                                        "type": "string",
                                        "format": "uc-host",
                                        "examples": [
                                            "192.168.1.10"
                                        ]
                                    },
                                    "acct-port": {
                                        "description": "The network port of our Radius server.",
                                        "type": "integer",
                                        "maximum": 65535,
                                        "minimum": 1024,
                                        "examples": [
                                            1812
                                        ]
                                    },
                                    "acct-secret": {
                                        "description": "The shared Radius authentication secret.",
                                        "type": "string",
                                        "examples": [
                                            "secret"
                                        ]
                                    }
                                }
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "protocol": {
                                        "description": "Defines whether the real should use radsec or normal radius.",
                                        "type": "string",
                                        "enum": [
                                            "block"
                                        ]
                                    },
                                    "realm": {
                                        "description": "The realm that that this server shall be used for.",
                                        "type": "array",
                                        "items": {
                                            "type": "string",
                                            "default": "*"
                                        }
                                    },
                                    "message": {
                                        "description": "The message that is sent when a realm is blocked.",
                                        "type": "string",
                                        "items": {
                                            "type": "string",
                                            "default": "blocked"
                                        }
                                    }
                                }
                            }
                        ]
                    }
                }
            }
        },
        "service.online-check": {
            "description": "This section can be used to configure the online check service.",
            "type": "object",
            "properties": {
                "ping-hosts": {
                    "description": "Hosts that shall be pinged to find out if we are online.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "uc-host",
                        "examples": [
                            "192.168.1.10"
                        ]
                    }
                },
                "download-hosts": {
                    "description": "URLs to which a http/s connection shall be established to find out if we are online. The service will try to download http://$string/online.txt and expects the content of that file to be \"Ok\". HTTP 30x is support allowing https redirects.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "www.example.org"
                        ]
                    }
                },
                "check-interval": {
                    "description": "The interval in seconds in between each online-check.",
                    "type": "number",
                    "default": 60
                },
                "check-threshold": {
                    "description": "How often does the online check need to fail until the system assumes that it has lost online connectivity.",
                    "type": "number",
                    "default": 1
                },
                "action": {
                    "description": "The action that the device shall execute when it has detected that it is not online.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "wifi",
                            "leds"
                        ]
                    }
                }
            }
        },
        "service.data-plane": {
            "description": "This section can be used to define eBPF and cBPF blobs that shall be loaded for virtual data-planes and SDN.",
            "type": "object",
            "properties": {
                "ingress-filters": {
                    "description": "A list of programs that can be loaded as ingress filters on interfaces.",
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "name": {
                                "description": "The name of the ingress filter.",
                                "type": "string"
                            },
                            "program": {
                                "description": "The base64 encoded xBPF.",
                                "type": "string",
                                "format": "uc-base64"
                            }
                        }
                    }
                }
            }
        },
        "service.wifi-steering": {
            "description": "This section describes the band steering behaviour of the unit.",
            "type": "object",
            "properties": {
                "mode": {
                    "description": "Wifi sterring can happen either locally or via the backend gateway.",
                    "type": "string",
                    "enum": [
                        "local",
                        "none"
                    ],
                    "examples": [
                        "local"
                    ]
                },
                "assoc-steering": {
                    "description": "Allow rejecting assoc requests for steering purposes.",
                    "type": "boolean",
                    "default": false
                },
                "required-snr": {
                    "description": "Minimum required signal level (dBm) for connected clients. If the client will be kicked if the SNR drops below this value.",
                    "type": "integer",
                    "default": 0
                },
                "required-probe-snr": {
                    "description": "Minimum required signal level (dBm) to allow connections. If the SNR is below this value, probe requests will not be replied to.",
                    "type": "integer",
                    "default": 0
                },
                "required-roam-snr": {
                    "description": "Minimum required signal level (dBm) before an attempt is made to roam the client to a better AP.",
                    "type": "integer",
                    "default": 0
                },
                "load-kick-threshold": {
                    "description": "Minimum channel load (%) before kicking clients",
                    "type": "integer",
                    "default": 0
                },
                "auto-channel": {
                    "description": "Allow multiple instances of the steering daemon to coordinate the best channel usage amongst eachother.",
                    "type": "boolean",
                    "default": false
                },
                "ipv6": {
                    "description": "Use IPv6 multicast to communicate with remote usteerd instances, rather than IPv4 broadcast.",
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "service.quality-of-service.class-selector": {
            "type": "string",
            "enum": [
                "CS0",
                "CS1",
                "CS2",
                "CS3",
                "CS4",
                "CS5",
                "CS6",
                "CS7",
                "AF11",
                "AF12",
                "AF13",
                "AF21",
                "AF22",
                "AF23",
                "AF31",
                "AF32",
                "AF33",
                "AF41",
                "AF42",
                "AF43",
                "DF",
                "EF",
                "VA",
                "LE"
            ]
        },
        "service.quality-of-service": {
            "description": "This section describes the QoS behaviour of the unit.",
            "type": "object",
            "properties": {
                "select-ports": {
                    "description": "The physical network devices that shall be considered the primary uplink interface. All classification and shaping will happen on this device.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "default": "WAN"
                    }
                },
                "bandwidth-up": {
                    "description": "Defines the upload bandwidth of this device. If it is not known or the device is attached to a shared medium, this value needs to be 0.",
                    "type": "integer",
                    "default": 0
                },
                "bandwidth-down": {
                    "description": "Defines the download bandwidth of this device. If it is not known or the device is attached to a shared medium, this value needs to be 0.",
                    "type": "integer",
                    "default": 0
                },
                "bulk-detection": {
                    "description": "The QoS feature can automatically detect and classify bulk flows. This is based on average packet size and PPS.",
                    "type": "object",
                    "properties": {
                        "dscp": {
                            "description": "The differentiated services code point that shall be assigned to packets that belong to a bulk flow.",
                            "$ref": "#/$defs/service.quality-of-service.class-selector",
                            "default": "CS0"
                        },
                        "packets-per-second": {
                            "description": "The required PPS rate that will cause a flow to be classified as bulk.",
                            "type": "number",
                            "default": 0
                        }
                    }
                },
                "services": {
                    "description": "A list of predefined named services that shall be classified according to the communities DB.",
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "classifier": {
                    "description": "A list of classifiers. Each classifier will map certain traffic to specific ToS/DSCP values based upon the defined constraints.",
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "dscp": {
                                "description": "The differentiated services code point that shall be assigned to packet that match the rules of this entry.",
                                "$ref": "#/$defs/service.quality-of-service.class-selector",
                                "default": "CS1"
                            },
                            "ports": {
                                "description": "Each entry defines a layer3 protocol and a port(range) that will be used to match packets.",
                                "type": "array",
                                "items": {
                                    "type": "object",
                                    "properties": {
                                        "protocol": {
                                            "description": "The port match can apply for TCP, UDP or any IP protocol.",
                                            "type": "string",
                                            "enum": [
                                                "any",
                                                "tcp",
                                                "udp"
                                            ],
                                            "default": "any"
                                        },
                                        "port": {
                                            "description": "The port of this match rule.",
                                            "type": "integer"
                                        },
                                        "range-end": {
                                            "description": "The last port of this match rule if it is a port range.",
                                            "type": "integer"
                                        },
                                        "reclassify": {
                                            "description": "Ignore the ToS/DSCP of packets and reclassify them.",
                                            "type": "boolean",
                                            "default": true
                                        }
                                    }
                                }
                            },
                            "dns": {
                                "description": "Each entry defines a wildcard FQDN. The IP that this resolves to will be used to match packets.",
                                "type": "array",
                                "items": {
                                    "type": "object",
                                    "properties": {
                                        "fqdn": {
                                            "type": "string",
                                            "format": "uc-fqdn"
                                        },
                                        "suffix-matching": {
                                            "description": "Match for all suffixes of the FQDN.",
                                            "type": "boolean",
                                            "default": true
                                        },
                                        "reclassify": {
                                            "description": "Ignore the ToS/DSCP of packets and reclassify them.",
                                            "type": "boolean",
                                            "default": true
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        },
        "service.facebook-wifi": {
            "description": "This section describes the FaceBook Wifi behaviour of the unit.",
            "type": "object",
            "properties": {
                "vendor-id": {
                    "description": "The Vendors ID.",
                    "type": "string"
                },
                "gateway-id": {
                    "description": "The Gateways ID.",
                    "type": "string"
                },
                "secret": {
                    "description": "The Device specific secret",
                    "type": "string"
                }
            }
        },
        "service.airtime-fairness": {
            "description": "This section describes the vlan behaviour of a logical network interface.",
            "type": "object",
            "properties": {
                "voice-weight": {
                    "description": "Voice traffic does not get aggregated. As voice and video are both considered priotity voice is considered to have a heavier weight when calculation priority average.",
                    "type": "number",
                    "default": 4
                },
                "packet-threshold": {
                    "description": "The amount of packets that need to be received for a specific type of traffic before new averageg is calculated.",
                    "type": "number",
                    "default": 100
                },
                "bulk-threshold": {
                    "description": "This option is a percentual value. If more the X% of the traffic is bulk, we assign the bulk weight.",
                    "type": "number",
                    "default": 50
                },
                "priority-threshold": {
                    "description": "This option is a percentual value. If more the X% of the traffic is priority, we assign the priority weight. Priority classification will take precedence over bulk.",
                    "type": "number",
                    "default": 30
                },
                "weight-normal": {
                    "description": "The default ATF weight that UEs get assigned.",
                    "type": "number",
                    "default": 256
                },
                "weight-priority": {
                    "description": "The default ATF weight that UEs get assigned when priority traffic above the configured percentage is detected.",
                    "type": "number",
                    "default": 394
                },
                "weight-bulk": {
                    "description": "The default ATF weight that UEs get assigned when bulk traffic above the configured percentage is detected.",
                    "type": "number",
                    "default": 128
                }
            }
        },
        "service.wireguard-overlay": {
            "description": "This Object defines the properties of a wireguard-overlay.",
            "type": "object",
            "properties": {
                "proto": {
                    "description": "This field must be set to wireguard-overlay.",
                    "type": "string",
                    "const": "wireguard-overlay"
                },
                "private-key": {
                    "description": "The private key of the device. This key is used to lookup the host entry inside the config.",
                    "type": "string"
                },
                "peer-port": {
                    "description": "The network port that shall be used to establish the wireguard tunnel.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1,
                    "default": 3456
                },
                "peer-exchange-port": {
                    "description": "The network port that shall be used to exchange peer data inside the tunnel.",
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1,
                    "default": 3458
                },
                "root-node": {
                    "description": "The descritption of the root node of the overlay.",
                    "type": "object",
                    "properties": {
                        "key": {
                            "description": "The public key of the host.",
                            "type": "string"
                        },
                        "endpoint": {
                            "description": "The public IP of the host (optional).",
                            "type": "string",
                            "format": "uc-ip"
                        },
                        "ipaddr": {
                            "description": "The list of private IPs that a host is reachable on inside the overlay.",
                            "type": "array",
                            "items": {
                                "type": "string",
                                "format": "uc-ip"
                            }
                        }
                    }
                },
                "hosts": {
                    "description": "The list of all known hosts inside the overlay.",
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "name": {
                                "description": "The unique name of the host.",
                                "type": "string"
                            },
                            "key": {
                                "description": "The public key of the host.",
                                "type": "string"
                            },
                            "endpoint": {
                                "description": "The public IP of the host (optional).",
                                "type": "string",
                                "format": "uc-ip"
                            },
                            "subnet": {
                                "description": "The list of subnets that shall be routed to this host.",
                                "type": "array",
                                "items": {
                                    "type": "string",
                                    "format": "uc-cidr"
                                }
                            },
                            "ipaddr": {
                                "description": "The list of private IPs that a host is reachable on inside the overlay.",
                                "type": "array",
                                "items": {
                                    "type": "string",
                                    "format": "uc-ip"
                                }
                            }
                        }
                    }
                },
                "vxlan": {
                    "description": "The descritption of the root node of the overlay.",
                    "type": "object",
                    "properties": {
                        "port": {
                            "description": "The network port that shall be used to establish the vxlan overlay.",
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1,
                            "default": 4789
                        },
                        "mtu": {
                            "description": "The MTU that shall be used by the vxlan tunnel.",
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 256,
                            "default": 1420
                        },
                        "isolate": {
                            "description": "If set to true hosts will only be able to talk with the root node and not forward L@ traffic between each other.",
                            "type": "boolean",
                            "default": true
                        }
                    }
                }
            }
        },
        "service.gps": {
            "description": "This section can be used to configure a GPS dongle",
            "type": "object",
            "properties": {
                "adjust-time": {
                    "description": "Adjust the systems clock upon a successful GPS lock.",
                    "type": "boolean",
                    "default": false
                },
                "baud-rate": {
                    "description": "The baudrate used by the attached GPS dongle",
                    "type": "integer",
                    "enum": [
                        2400,
                        4800,
                        9600,
                        19200
                    ]
                }
            }
        },
        "service.dhcp-relay": {
            "description": "Define the vlans on which the dhcp shall be relayed.",
            "type": "object",
            "properties": {
                "select-ports": {
                    "description": "The list of physical network devices that shall be used to fwd the DHCP frames.",
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "vlans": {
                    "description": "The list of all vlans",
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "vlan": {
                                "description": "The vlan id.",
                                "type": "number"
                            },
                            "relay-server": {
                                "description": "The unicast target DHCP pool server where frames get relayed to.",
                                "type": "string",
                                "format": "uc-ip"
                            },
                            "circuit-id-format": {
                                "description": "This option selects what info shall be contained within a relayed frame's circuit ID.",
                                "type": "string",
                                "enum": [
                                    "vlan-id",
                                    "ap-mac",
                                    "ssid"
                                ],
                                "default": "vlan-id"
                            },
                            "remote-id-format": {
                                "description": "This option selects what info shall be contained within a relayed frame's remote ID.",
                                "type": "string",
                                "enum": [
                                    "vlan-id",
                                    "ap-mac",
                                    "ssid"
                                ],
                                "default": "ap-mac"
                            }
                        }
                    }
                }
            }
        },
        "service.admin-ui": {
            "type": "object",
            "properties": {
                "wifi-ssid": {
                    "description": "The name of the admin ssid.",
                    "type": "string",
                    "default": "Maverick",
                    "maxLength": 32,
                    "minLength": 1
                },
                "wifi-key": {
                    "description": "The Pre Shared Key (PSK) that is used for encryption on the BSS.",
                    "type": "string",
                    "maxLength": 63,
                    "minLength": 8
                },
                "wifi-bands": {
                    "description": "The band that the SSID should be broadcasted on. The configuration layer will use the first matching band.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "2G",
                            "5G",
                            "5G-lower",
                            "5G-upper",
                            "6G"
                        ]
                    }
                },
                "offline-trigger": {
                    "description": "The admin-ui will be spawned when this offline threshold was exceeded.",
                    "type": "number"
                }
            }
        },
        "service.rrm": {
            "description": "This section describes the band steering behaviour of the unit.",
            "type": "object",
            "properties": {
                "beacon-request-assoc": {
                    "description": "Tell stations to send a beacon request scan when they associate.",
                    "type": "boolean",
                    "default": true
                },
                "station-stats-interval": {
                    "description": "Periodically send station statistics every N seconds.",
                    "type": "number"
                }
            }
        },
        "service": {
            "description": "This section describes all of the services that may be present on the AP. Each service is then referenced via its name inside an interface, ssid, ...",
            "type": "object",
            "properties": {
                "lldp": {
                    "$ref": "#/$defs/service.lldp"
                },
                "ssh": {
                    "$ref": "#/$defs/service.ssh"
                },
                "ntp": {
                    "$ref": "#/$defs/service.ntp"
                },
                "mdns": {
                    "$ref": "#/$defs/service.mdns"
                },
                "rtty": {
                    "$ref": "#/$defs/service.rtty"
                },
                "log": {
                    "$ref": "#/$defs/service.log"
                },
                "http": {
                    "$ref": "#/$defs/service.http"
                },
                "igmp": {
                    "$ref": "#/$defs/service.igmp"
                },
                "ieee8021x": {
                    "$ref": "#/$defs/service.ieee8021x"
                },
                "radius-proxy": {
                    "$ref": "#/$defs/service.radius-proxy"
                },
                "online-check": {
                    "$ref": "#/$defs/service.online-check"
                },
                "data-plane": {
                    "$ref": "#/$defs/service.data-plane"
                },
                "wifi-steering": {
                    "$ref": "#/$defs/service.wifi-steering"
                },
                "quality-of-service": {
                    "$ref": "#/$defs/service.quality-of-service"
                },
                "facebook-wifi": {
                    "$ref": "#/$defs/service.facebook-wifi"
                },
                "airtime-fairness": {
                    "$ref": "#/$defs/service.airtime-fairness"
                },
                "wireguard-overlay": {
                    "$ref": "#/$defs/service.wireguard-overlay"
                },
                "captive": {
                    "$ref": "#/$defs/service.captive"
                },
                "gps": {
                    "$ref": "#/$defs/service.gps"
                },
                "dhcp-relay": {
                    "$ref": "#/$defs/service.dhcp-relay"
                },
                "admin-ui": {
                    "$ref": "#/$defs/service.admin-ui"
                },
                "rrm": {
                    "$ref": "#/$defs/service.rrm"
                }
            }
        },
        "metrics.statistics": {
            "description": "Statistics are traffic counters, neighbor tables, ...",
            "type": "object",
            "properties": {
                "interval": {
                    "description": "The reporting interval defined in seconds.",
                    "type": "integer",
                    "minimum": 60
                },
                "types": {
                    "description": "A list of names of subsystems that shall be reported periodically.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "ssids",
                            "lldp",
                            "clients",
                            "tid-stats"
                        ]
                    }
                }
            }
        },
        "metrics.health": {
            "description": "Health check gets executed periodically and will report a health value between 0-100 indicating how healthy the device thinks it is",
            "type": "object",
            "properties": {
                "interval": {
                    "description": "The reporting interval defined in seconds.",
                    "type": "integer",
                    "minimum": 60
                },
                "dhcp-local": {
                    "description": "This is makes the AP probe local downstream DHCP servers.",
                    "type": "boolean",
                    "default": true
                },
                "dhcp-remote": {
                    "description": "This is makes the AP probe remote upstream DHCP servers.",
                    "type": "boolean",
                    "default": false
                },
                "dns-local": {
                    "description": "This is makes the AP probe DNS servers.",
                    "type": "boolean",
                    "default": true
                },
                "dns-remote": {
                    "description": "This is makes the AP probe DNS servers.",
                    "type": "boolean",
                    "default": true
                }
            }
        },
        "metrics.wifi-frames": {
            "description": "Define which types of ieee802.11 management frames shall be sent up to the controller.",
            "type": "object",
            "properties": {
                "filters": {
                    "description": "A list of the management frames types that shall be sent to the backend.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "probe",
                            "auth",
                            "assoc",
                            "disassoc",
                            "deauth",
                            "local-deauth",
                            "inactive-deauth",
                            "key-mismatch",
                            "beacon-report",
                            "radar-detected"
                        ]
                    }
                }
            }
        },
        "metrics.dhcp-snooping": {
            "description": "DHCP snooping allows us to intercept DHCP packages on interface that are bridged, where DHCP is not offered as a service by the AP.",
            "type": "object",
            "properties": {
                "filters": {
                    "description": "A list of the message types that shall be sent to the backend.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "ack",
                            "discover",
                            "offer",
                            "request",
                            "solicit",
                            "reply",
                            "renew"
                        ]
                    }
                }
            }
        },
        "metrics.wifi-scan": {
            "description": "Define the behaviour of the periodic wifi scanning interface.",
            "type": "object",
            "properties": {
                "interval": {
                    "description": "The periodicity at which the scan shall be performed.",
                    "type": "integer"
                },
                "verbose": {
                    "description": "Add capabilities, v/ht_oper, ... to the resulting scan info.",
                    "type": "boolean"
                },
                "information-elements": {
                    "description": "Add all IEs to the resulting scan info.",
                    "type": "boolean"
                }
            }
        },
        "metrics.telemetry": {
            "description": "Configure the unsolicited telemetry stream.",
            "type": "object",
            "properties": {
                "interval": {
                    "description": "The reporting interval defined in seconds.",
                    "type": "integer"
                },
                "types": {
                    "description": "The event types that get added to telemetry.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "ssh",
                            "health",
                            "health.dns",
                            "health.dhcp",
                            "health.radius",
                            "health.memory",
                            "client",
                            "client.join",
                            "client.leave",
                            "client.key-mismatch",
                            "wifi",
                            "wifi.start",
                            "wifi.stop",
                            "wired",
                            "wired.carrier-up",
                            "wired.carrier-down",
                            "unit",
                            "unit.boot-up"
                        ]
                    }
                }
            }
        },
        "metrics.realtime": {
            "description": "Configure the realtime events that get sent to the cloud.",
            "type": "object",
            "properties": {
                "types": {
                    "description": "The event types that get added to telemetry.",
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "ssh",
                            "health",
                            "health.dns",
                            "health.dhcp",
                            "health.radius",
                            "health.memory",
                            "client",
                            "client.join",
                            "client.leave",
                            "client.key-mismatch",
                            "wifi",
                            "wifi.start",
                            "wifi.stop",
                            "wired",
                            "wired.carrier-up",
                            "wired.carrier-down",
                            "unit",
                            "unit.boot-up"
                        ]
                    }
                }
            }
        },
        "metrics": {
            "description": "There are several types of mertics that shall be reported in certain intervals. This section provides a granual configuration.",
            "type": "object",
            "properties": {
                "statistics": {
                    "$ref": "#/$defs/metrics.statistics"
                },
                "health": {
                    "$ref": "#/$defs/metrics.health"
                },
                "wifi-frames": {
                    "$ref": "#/$defs/metrics.wifi-frames"
                },
                "dhcp-snooping": {
                    "$ref": "#/$defs/metrics.dhcp-snooping"
                },
                "wifi-scan": {
                    "$ref": "#/$defs/metrics.wifi-scan"
                },
                "telemetry": {
                    "$ref": "#/$defs/metrics.telemetry"
                },
                "realtime": {
                    "$ref": "#/$defs/metrics.realtime"
                }
            }
        },
        "config-raw": {
            "description": "This object allows passing raw uci commands, that get applied after all the other configuration was ben generated.",
            "type": "array",
            "items": {
                "type": "array",
                "minItems": 2,
                "items": {
                    "type": "string"
                },
                "examples": [
                    [
                        "set",
                        "system.@system[0].timezone",
                        "GMT0"
                    ],
                    [
                        "delete",
                        "firewall.@zone[0]"
                    ],
                    [
                        "delete",
                        "dhcp.wan"
                    ],
                    [
                        "add",
                        "dhcp",
                        "dhcp"
                    ],
                    [
                        "add-list",
                        "system.ntp.server",
                        "0.pool.example.org"
                    ],
                    [
                        "del-list",
                        "system.ntp.server",
                        "1.openwrt.pool.ntp.org"
                    ]
                ]
            }
        }
    }
}

)foo";

static std::string DefaultSWITCHSchema = R"foo(

{
    "$id": "https://openwrt.org/ucentral.schema.json",
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "properties": {
        "strict": {
            "type": "boolean",
            "default": false
        },
        "uuid": {
            "type": "integer"
        },
        "unit": {
            "$ref": "#/$defs/unit"
        },
        "globals": {
            "$ref": "#/$defs/globals"
        },
        "definitions": {
            "$ref": "#/$defs/definitions"
        },
        "ethernet": {
            "type": "array",
            "items": {
                "$ref": "#/$defs/ethernet"
            }
        },
        "switch": {
            "$ref": "#/$defs/switch"
        },
        "radios": {
            "type": "array",
            "items": {
                "$ref": "#/$defs/radio"
            }
        },
        "interfaces": {
            "type": "array",
            "items": {
                "$ref": "#/$defs/interface"
            }
        },
        "services": {
            "$ref": "#/$defs/service"
        },
        "metrics": {
            "$ref": "#/$defs/metrics"
        },
        "config-raw": {
            "$ref": "#/$defs/config-raw"
        },
        "third-party": {
            "type": "object",
            "additionalProperties": true
        }
    },
    "$defs": {
        "unit": {
            "type": "object",
            "properties": {
                "name": {
                    "type": "string"
                },
                "hostname": {
                    "type": "string",
                    "format": "hostname"
                },
                "location": {
                    "type": "string"
                },
                "timezone": {
                    "type": "string",
                    "examples": [
                        "UTC",
                        "EST5",
                        "CET-1CEST,M3.5.0,M10.5.0/3"
                    ]
                },
                "leds-active": {
                    "type": "boolean",
                    "default": true
                },
                "random-password": {
                    "type": "boolean",
                    "default": false
                },
                "system-password": {
                    "type": "string"
                },
                "beacon-advertisement": {
                    "type": "object",
                    "properties": {
                        "device-name": {
                            "type": "boolean"
                        },
                        "device-serial": {
                            "type": "boolean"
                        },
                        "network-id": {
                            "type": "integer"
                        }
                    }
                },
                "poe": {
                    "type": "object",
                    "properties": {
                        "power-management": {
                            "type": "string",
                            "examples": [
                                "class",
                                "dynamic",
                                "dynamic-priority",
                                "static",
                                "static-priority"
                            ]
                        },
                        "usage-threshold": {
                            "type": "number",
                            "default": 90
                        }
                    }
                },
                "multicast": {
                    "type": "object",
                    "properties": {
                        "igmp-snooping-enable": {
                            "type": "boolean",
                            "default": true
                        },
                        "mld-snooping-enable": {
                            "type": "boolean",
                            "default": true
                        }
                    }
                }
            }
        },
        "globals.wireless-multimedia.class-selector": {
            "type": "array",
            "items": {
                "type": "string",
                "enum": [
                    "CS0",
                    "CS1",
                    "CS2",
                    "CS3",
                    "CS4",
                    "CS5",
                    "CS6",
                    "CS7",
                    "AF11",
                    "AF12",
                    "AF13",
                    "AF21",
                    "AF22",
                    "AF23",
                    "AF31",
                    "AF32",
                    "AF33",
                    "AF41",
                    "AF42",
                    "AF43",
                    "DF",
                    "EF",
                    "VA",
                    "LE"
                ]
            }
        },
        "globals.wireless-multimedia.table": {
            "type": "object",
            "additionalProperties": false,
            "properties": {
                "UP0": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP1": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP2": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP3": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP4": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP5": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP6": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                },
                "UP7": {
                    "$ref": "#/$defs/globals.wireless-multimedia.class-selector"
                }
            }
        },
        "globals.wireless-multimedia.profile": {
            "type": "object",
            "additionalProperties": false,
            "properties": {
                "profile": {
                    "type": "string",
                    "enum": [
                        "enterprise",
                        "rfc8325",
                        "3gpp"
                    ]
                }
            }
        },
        "globals": {
            "type": "object",
            "properties": {
                "ipv4-network": {
                    "type": "string",
                    "format": "uc-cidr4",
                    "examples": [
                        "192.168.0.0/16"
                    ]
                },
                "ipv6-network": {
                    "type": "string",
                    "format": "uc-cidr6",
                    "examples": [
                        "fdca:1234:4567::/48"
                    ]
                },
                "wireless-multimedia": {
                    "anyOf": [
                        {
                            "$ref": "#/$defs/globals.wireless-multimedia.table"
                        },
                        {
                            "$ref": "#/$defs/globals.wireless-multimedia.profile"
                        }
                    ]
                },
                "ipv4-blackhole": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "prefix": {
                                "type": "string",
                                "format": "uc-cidr4",
                                "examples": [
                                    "192.168.1.0/24"
                                ]
                            },
                            "vrf": {
                                "type": "number"
                            }
                        }
                    }
                },
                "ipv4-unreachable": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "prefix": {
                                "type": "string",
                                "format": "uc-cidr4",
                                "examples": [
                                    "192.168.1.0/24"
                                ]
                            },
                            "vrf": {
                                "type": "number"
                            }
                        }
                    }
                }
            }
        },
        "interface.ssid.encryption": {
            "type": "object",
            "properties": {
                "proto": {
                    "type": "string",
                    "enum": [
                        "none",
                        "owe",
                        "owe-transition",
                        "psk",
                        "psk2",
                        "psk-mixed",
                        "psk2-radius",
                        "wpa",
                        "wpa2",
                        "wpa-mixed",
                        "sae",
                        "sae-mixed",
                        "wpa3",
                        "wpa3-192",
                        "wpa3-mixed"
                    ],
                    "examples": [
                        "psk2"
                    ]
                },
                "key": {
                    "type": "string",
                    "maxLength": 63,
                    "minLength": 8
                },
                "ieee80211w": {
                    "type": "string",
                    "enum": [
                        "disabled",
                        "optional",
                        "required"
                    ],
                    "default": "disabled"
                },
                "key-caching": {
                    "type": "boolean",
                    "default": true
                }
            }
        },
        "definitions": {
            "type": "object",
            "properties": {
                "wireless-encryption": {
                    "type": "object",
                    "patternProperties": {
                        ".+": {
                            "$ref": "#/$defs/interface.ssid.encryption",
                            "additionalProperties": false
                        }
                    }
                }
            }
        },
        "ethernet": {
            "type": "object",
            "properties": {
                "select-ports": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "LAN1",
                            "LAN2",
                            "LAN3",
                            "LAN4",
                            "LAN*",
                            "WAN*",
                            "*"
                        ]
                    }
                },
                "speed": {
                    "type": "integer",
                    "enum": [
                        10,
                        100,
                        1000,
                        2500,
                        5000,
                        10000,
                        25000,
                        100000
                    ],
                    "default": 1000
                },
                "duplex": {
                    "type": "string",
                    "enum": [
                        "half",
                        "full"
                    ],
                    "default": "full"
                },
                "enabled": {
                    "type": "boolean",
                    "default": true
                },
                "services": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "quality-of-service"
                        ]
                    }
                },
                "poe": {
                    "type": "object",
                    "properties": {
                        "admin-mode": {
                            "type": "boolean",
                            "default": false
                        },
                        "do-reset": {
                            "type": "boolean"
                        },
                        "detection": {
                            "type": "string",
                            "examples": [
                                "2pt-dot3af",
                                "2pt-dot3af+legacy",
                                "4pt-dot3af",
                                "4pt-dot3af+legacy",
                                "dot3bt",
                                "dot3bt+legacy",
                                "legacy"
                            ],
                            "default": "dot3bt"
                        },
                        "power-limit": {
                            "type": "integer",
                            "default": 99900
                        },
                        "priority": {
                            "type": "string",
                            "default": "low",
                            "examples": [
                                "critical",
                                "high",
                                "medium",
                                "low"
                            ]
                        }
                    }
                },
                "ieee8021x": {
                    "type": "object",
                    "properties": {
                        "is-authenticator": {
                            "type": "boolean",
                            "default": false
                        },
                        "authentication-mode": {
                            "type": "string",
                            "enum": [
                                "force-authorized",
                                "force-unauthorized",
                                "auto"
                            ],
                            "default": "force-authorized"
                        },
                        "host-mode": {
                            "type": "string",
                            "enum": [
                                "multi-auth",
                                "multi-domain",
                                "multi-host",
                                "single-host"
                            ],
                            "default": "multi-auth"
                        },
                        "guest-vlan": {
                            "type": "integer",
                            "minimum": 1,
                            "maximum": 4094
                        },
                        "unauthenticated-vlan": {
                            "type": "integer",
                            "minimum": 1,
                            "maximum": 4094
                        }
                    }
                },
                "port-isolation": {
                    "type": "object",
                    "properties": {
                        "sessions": {
                            "type": "array",
                            "items": {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer"
                                    },
                                    "uplink": {
                                        "type": "object",
                                        "properties": {
                                            "interface-list": {
                                                "type": "array",
                                                "items": {
                                                    "type": "string"
                                                }
                                            }
                                        }
                                    },
                                    "downlink": {
                                        "type": "object",
                                        "properties": {
                                            "interface-list": {
                                                "type": "array",
                                                "items": {
                                                    "type": "string"
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        },
        "switch": {
            "type": "object",
            "properties": {
                "port-mirror": {
                    "type": "object",
                    "properties": {
                        "monitor-ports": {
                            "type": "array",
                            "items": {
                                "type": "string"
                            }
                        },
                        "analysis-port": {
                            "type": "string"
                        }
                    }
                },
                "loop-detection": {
                    "type": "object",
                    "properties": {
                        "protocol": {
                            "type": "string",
                            "enum": [
                                "none",
                                "stp",
                                "rstp",
                                "mstp",
                                "pvstp",
                                "rpvstp"
                            ],
                            "default": "rstp"
                        },
                        "roles": {
                            "type": "array",
                            "items": {
                                "type": "string",
                                "enum": [
                                    "upstream",
                                    "downstream"
                                ]
                            }
                        }
                    },
                    "instances": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "id": {
                                    "type": "integer"
                                },
                                "enabled": {
                                    "type": "boolean",
                                    "default": true
                                },
                                "priority": {
                                    "type": "integer",
                                    "default": 32768
                                },
                                "forward_delay": {
                                    "type": "integer",
                                    "default": 15
                                },
                                "hello_time": {
                                    "type": "integer",
                                    "default": 2
                                },
                                "max_age": {
                                    "type": "integer",
                                    "default": 20
                                }
                            }
                        }
                    }
                },
                "ieee8021x": {
                    "type": "object",
                    "properties": {
                        "auth-control-enable": {
                            "type": "boolean",
                            "default": false
                        },
                        "radius": {
                            "type": "array",
                            "items": {
                                "type": "object",
                                "properties": {
                                    "server-host": {
                                        "type": "string",
                                        "examples": [
                                            "192.168.1.1",
                                            "somehost.com"
                                        ]
                                    },
                                    "server-authentication-port": {
                                        "type": "integer",
                                        "maximum": 65535,
                                        "minimum": 1
                                    },
                                    "server-key": {
                                        "type": "string",
                                        "examples": [
                                            "somepassword"
                                        ]
                                    },
                                    "server-priority": {
                                        "type": "integer",
                                        "maximum": 64,
                                        "minimum": 1
                                    }
                                }
                            }
                        },
                        "dynamic-authorization": {
                            "type": "object",
                            "properties": {
                                "auth-type": {
                                    "type": "string",
                                    "enum": [
                                        "all",
                                        "any",
                                        "session-key"
                                    ]
                                },
                                "bounce-port-ignore": {
                                    "type": "boolean",
                                    "default": false
                                },
                                "disable-port-ignore": {
                                    "type": "boolean",
                                    "default": false
                                },
                                "ignore-server-key": {
                                    "type": "boolean",
                                    "default": false
                                },
                                "ignore-session-key": {
                                    "type": "boolean",
                                    "default": false
                                },
                                "server-key": {
                                    "type": "string"
                                },
                                "client": {
                                    "type": "array",
                                    "items": {
                                        "type": "object",
                                        "properties": {
                                            "address": {
                                                "type": "string"
                                            },
                                            "server-key": {
                                                "type": "string"
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        },
        "radio.rates": {
            "type": "object",
            "properties": {
                "beacon": {
                    "type": "integer",
                    "default": 6000,
                    "enum": [
                        0,
                        1000,
                        2000,
                        5500,
                        6000,
                        9000,
                        11000,
                        12000,
                        18000,
                        24000,
                        36000,
                        48000,
                        54000
                    ]
                },
                "multicast": {
                    "type": "integer",
                    "default": 24000,
                    "enum": [
                        0,
                        1000,
                        2000,
                        5500,
                        6000,
                        9000,
                        11000,
                        12000,
                        18000,
                        24000,
                        36000,
                        48000,
                        54000
                    ]
                }
            }
        },
        "radio.he": {
            "type": "object",
            "properties": {
                "multiple-bssid": {
                    "type": "boolean",
                    "default": false
                },
                "ema": {
                    "type": "boolean",
                    "default": false
                },
                "bss-color": {
                    "type": "integer",
                    "default": 64
                }
            }
        },
        "radio": {
            "type": "object",
            "properties": {
                "band": {
                    "type": "string",
                    "enum": [
                        "2G",
                        "5G",
                        "5G-lower",
                        "5G-upper",
                        "6G"
                    ]
                },
                "bandwidth": {
                    "type": "integer",
                    "enum": [
                        5,
                        10,
                        20
                    ]
                },
                "channel": {
                    "oneOf": [
                        {
                            "type": "integer",
                            "maximum": 196,
                            "minimum": 1
                        },
                        {
                            "type": "string",
                            "const": "auto"
                        }
                    ]
                },
                "valid-channels": {
                    "type": "array",
                    "items": {
                        "type": "integer",
                        "maximum": 196,
                        "minimum": 1
                    }
                },
                "country": {
                    "type": "string",
                    "maxLength": 2,
                    "minLength": 2,
                    "examples": [
                        "US"
                    ]
                },
                "allow-dfs": {
                    "type": "boolean",
                    "default": true
                },
                "channel-mode": {
                    "type": "string",
                    "enum": [
                        "HT",
                        "VHT",
                        "HE",
                        "EHT"
                    ],
                    "default": "HE"
                },
                "channel-width": {
                    "type": "integer",
                    "enum": [
                        20,
                        40,
                        80,
                        160,
                        320,
                        8080
                    ],
                    "default": 80
                },
                "require-mode": {
                    "type": "string",
                    "enum": [
                        "HT",
                        "VHT",
                        "HE"
                    ]
                },
                "mimo": {
                    "type": "string",
                    "enum": [
                        "1x1",
                        "2x2",
                        "3x3",
                        "4x4",
                        "5x5",
                        "6x6",
                        "7x7",
                        "8x8"
                    ]
                },
                "tx-power": {
                    "type": "integer",
                    "maximum": 30,
                    "minimum": 0
                },
                "legacy-rates": {
                    "type": "boolean",
                    "default": false
                },
                "beacon-interval": {
                    "type": "integer",
                    "default": 100,
                    "maximum": 65535,
                    "minimum": 15
                },
                "maximum-clients": {
                    "type": "integer",
                    "example": 64
                },
                "maximum-clients-ignore-probe": {
                    "type": "boolean"
                },
                "rates": {
                    "$ref": "#/$defs/radio.rates"
                },
                "he-settings": {
                    "$ref": "#/$defs/radio.he"
                },
                "hostapd-iface-raw": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "ap_table_expiration_time=3600",
                            "device_type=6-0050F204-1",
                            "ieee80211h=1",
                            "rssi_ignore_probe_request=-75",
                            "time_zone=EST5",
                            "uuid=12345678-9abc-def0-1234-56789abcdef0",
                            "venue_url=1:http://www.example.com/info-eng",
                            "wpa_deny_ptk0_rekey=0"
                        ]
                    }
                }
            }
        },
        "interface.vlan": {
            "type": "object",
            "properties": {
                "id": {
                    "type": "integer",
                    "maximum": 4050
                },
                "proto": {
                    "decription": "The L2 vlan tag that shall be added (1q,1ad)",
                    "type": "string",
                    "enum": [
                        "802.1ad",
                        "802.1q"
                    ],
                    "default": "802.1q"
                },
                "stp-instance": {
                    "decription": "MSTP instance identifier of the vlan. This field does nothing if MSTP is not enabled.",
                    "type": "integer"
                }
            }
        },
        "interface.bridge": {
            "type": "object",
            "properties": {
                "mtu": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 256,
                    "examples": [
                        1500
                    ]
                },
                "tx-queue-len": {
                    "type": "integer",
                    "examples": [
                        5000
                    ]
                },
                "isolate-ports": {
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "interface.ethernet": {
            "type": "object",
            "properties": {
                "select-ports": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "LAN1",
                            "LAN2",
                            "LAN3",
                            "LAN4",
                            "LAN*",
                            "WAN*",
                            "*"
                        ]
                    }
                },
                "multicast": {
                    "type": "boolean",
                    "default": true
                },
                "learning": {
                    "type": "boolean",
                    "default": true
                },
                "isolate": {
                    "type": "boolean",
                    "default": false
                },
                "macaddr": {
                    "type": "string",
                    "format": "uc-mac"
                },
                "reverse-path-filter": {
                    "type": "boolean",
                    "default": false
                },
                "vlan-tag": {
                    "type": "string",
                    "enum": [
                        "tagged",
                        "un-tagged",
                        "auto"
                    ],
                    "default": "auto"
                }
            }
        },
        "interface.ipv4.dhcp": {
            "type": "object",
            "properties": {
                "lease-first": {
                    "type": "integer",
                    "examples": [
                        10
                    ]
                },
                "lease-count": {
                    "type": "integer",
                    "examples": [
                        100
                    ]
                },
                "lease-time": {
                    "type": "string",
                    "format": "uc-timeout",
                    "default": "6h"
                },
                "relay-server": {
                    "type": "string",
                    "format": "ipv4",
                    "example": "192.168.2.1"
                },
                "circuit-id-format": {
                    "type": "string",
                    "example": [
                        "\\{Interface\\}:\\{VLAN-ID\\}}"
                    ]
                }
            }
        },
        "interface.ipv4.dhcp-lease": {
            "type": "object",
            "properties": {
                "macaddr": {
                    "type": "string",
                    "format": "uc-mac",
                    "examples": [
                        "00:11:22:33:44:55"
                    ]
                },
                "static-lease-offset": {
                    "type": "integer",
                    "examples": [
                        10
                    ]
                },
                "lease-time": {
                    "type": "string",
                    "format": "uc-timeout",
                    "default": "6h"
                },
                "publish-hostname": {
                    "type": "boolean",
                    "default": true
                }
            }
        },
        "interface.ipv4.port-forward": {
            "type": "object",
            "properties": {
                "protocol": {
                    "type": "string",
                    "enum": [
                        "tcp",
                        "udp",
                        "any"
                    ],
                    "default": "any"
                },
                "external-port": {
                    "type": [
                        "integer",
                        "string"
                    ],
                    "minimum": 0,
                    "maximum": 65535,
                    "format": "uc-portrange"
                },
                "internal-address": {
                    "type": "string",
                    "format": "ipv4",
                    "example": "0.0.0.120"
                },
                "internal-port": {
                    "type": [
                        "integer",
                        "string"
                    ],
                    "minimum": 0,
                    "maximum": 65535,
                    "format": "uc-portrange"
                }
            },
            "required": [
                "external-port",
                "internal-address"
            ]
        },
        "interface.ipv4": {
            "type": "object",
            "properties": {
                "addressing": {
                    "type": "string",
                    "enum": [
                        "dynamic",
                        "static",
                        "none"
                    ],
                    "examples": [
                        "static"
                    ]
                },
                "subnet": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "prefix": {
                                "type": "string",
                                "format": "uc-cidr4",
                                "examples": [
                                    "192.168.1.0/24"
                                ]
                            },
                            "vrf": {
                                "type": "number"
                            }
                        }
                    }
                },
                "gateway": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "prefix": {
                                "type": "string",
                                "format": "uc-cidr4",
                                "examples": [
                                    "192.168.1.0/24"
                                ]
                            },
                            "nexthop": {
                                "type": "string",
                                "format": "ipv4",
                                "examples": [
                                    "192.168.1.1"
                                ]
                            },
                            "vrf": {
                                "type": "number"
                            },
                            "metric": {
                                "type": "number"
                            }
                        }
                    }
                },
                "broadcast": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "prefix": {
                                "type": "string",
                                "format": "uc-cidr4",
                                "examples": [
                                    "192.168.1.0/24"
                                ]
                            },
                            "vrf": {
                                "type": "number"
                            }
                        }
                    }
                },
                "multicast": {
                    "type": "object",
                    "properties": {
                        "unknown-multicast-flood-control": {
                            "type": "boolean",
                            "default": true
                        },
                        "igmp": {
                            "type": "object",
                            "properties": {
                                "snooping-enable": {
                                    "type": "boolean",
                                    "default": true
                                },
                                "version": {
                                    "type": "integer",
                                    "enum": [
                                        1,
                                        2,
                                        3
                                    ],
                                    "examples": [
                                        3
                                    ],
                                    "default": 3
                                },
                                "querier-enable": {
                                    "type": "boolean",
                                    "default": false
                                },
                                "fast-leave-enable": {
                                    "type": "boolean",
                                    "default": false
                                },
                                "query-interval": {
                                    "type": "integer",
                                    "default": 1000
                                },
                                "last-member-query-interval": {
                                    "type": "integer",
                                    "default": 1000
                                },
                                "max-response-time": {
                                    "type": "integer",
                                    "default": 10
                                },
                                "static-mcast-groups": {
                                    "type": "array",
                                    "items": {
                                        "type": "object",
                                        "properties": {
                                            "egress-ports": {
                                                "type": "array",
                                                "items": {
                                                    "type": "string"
                                                }
                                            },
                                            "address": {
                                                "type": "string",
                                                "format": "ipv4",
                                                "examples": [
                                                    "225.0.0.1"
                                                ]
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                },
                "send-hostname": {
                    "type": "boolean",
                    "default": true,
                    "examples": [
                        true
                    ]
                },
                "use-dns": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "ipv4",
                        "examples": [
                            "8.8.8.8",
                            "4.4.4.4"
                        ]
                    }
                },
                "dhcp": {
                    "$ref": "#/$defs/interface.ipv4.dhcp"
                },
                "dhcp-leases": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ipv4.dhcp-lease"
                    }
                },
                "port-forward": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ipv4.port-forward"
                    }
                }
            }
        },
        "interface.ipv6.dhcpv6": {
            "type": "object",
            "properties": {
                "mode": {
                    "type": "string",
                    "enum": [
                        "hybrid",
                        "stateless",
                        "stateful",
                        "relay"
                    ]
                },
                "announce-dns": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "ipv6"
                    }
                },
                "filter-prefix": {
                    "type": "string",
                    "format": "uc-cidr6",
                    "default": "::/0"
                }
            }
        },
        "interface.ipv6.port-forward": {
            "type": "object",
            "properties": {
                "protocol": {
                    "type": "string",
                    "enum": [
                        "tcp",
                        "udp",
                        "any"
                    ],
                    "default": "any"
                },
                "external-port": {
                    "type": [
                        "integer",
                        "string"
                    ],
                    "minimum": 0,
                    "maximum": 65535,
                    "format": "uc-portrange"
                },
                "internal-address": {
                    "type": "string",
                    "format": "ipv6",
                    "example": "::1234:abcd"
                },
                "internal-port": {
                    "type": [
                        "integer",
                        "string"
                    ],
                    "minimum": 0,
                    "maximum": 65535,
                    "format": "uc-portrange"
                }
            },
            "required": [
                "external-port",
                "internal-address"
            ]
        },
        "interface.ipv6.traffic-allow": {
            "type": "object",
            "properties": {
                "protocol": {
                    "type": "string",
                    "default": "any"
                },
                "source-address": {
                    "type": "string",
                    "format": "uc-cidr6",
                    "example": "2001:db8:1234:abcd::/64",
                    "default": "::/0"
                },
                "source-ports": {
                    "type": "array",
                    "minItems": 1,
                    "items": {
                        "type": [
                            "integer",
                            "string"
                        ],
                        "minimum": 0,
                        "maximum": 65535,
                        "format": "uc-portrange"
                    }
                },
                "destination-address": {
                    "type": "string",
                    "format": "ipv6",
                    "example": "::1000"
                },
                "destination-ports": {
                    "type": "array",
                    "minItems": 1,
                    "items": {
                        "type": [
                            "integer",
                            "string"
                        ],
                        "minimum": 0,
                        "maximum": 65535,
                        "format": "uc-portrange"
                    }
                }
            },
            "required": [
                "destination-address"
            ]
        },
        "interface.ipv6": {
            "type": "object",
            "properties": {
                "addressing": {
                    "type": "string",
                    "enum": [
                        "dynamic",
                        "static"
                    ]
                },
                "subnet": {
                    "type": "string",
                    "format": "uc-cidr6",
                    "examples": [
                        "auto/64"
                    ]
                },
                "gateway": {
                    "type": "string",
                    "format": "ipv6",
                    "examples": [
                        "2001:db8:123:456::1"
                    ]
                },
                "prefix-size": {
                    "type": "integer",
                    "maximum": 64,
                    "minimum": 0
                },
                "dhcpv6": {
                    "$ref": "#/$defs/interface.ipv6.dhcpv6"
                },
                "port-forward": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ipv6.port-forward"
                    }
                },
                "traffic-allow": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ipv6.traffic-allow"
                    }
                }
            }
        },
        "interface.broad-band.wwan": {
            "type": "object",
            "properties": {
                "protocol": {
                    "type": "string",
                    "const": "wwan"
                },
                "modem-type": {
                    "type": "string",
                    "enum": [
                        "qmi",
                        "mbim",
                        "wwan"
                    ]
                },
                "access-point-name": {
                    "type": "string"
                },
                "authentication-type": {
                    "type": "string",
                    "enum": [
                        "none",
                        "pap",
                        "chap",
                        "pap-chap"
                    ],
                    "default": "none"
                },
                "pin-code": {
                    "type": "string"
                },
                "user-name": {
                    "type": "string"
                },
                "password": {
                    "type": "string"
                },
                "packet-data-protocol": {
                    "type": "string",
                    "enum": [
                        "ipv4",
                        "ipv6",
                        "dual-stack"
                    ],
                    "default": "dual-stack"
                }
            }
        },
        "interface.broad-band.pppoe": {
            "type": "object",
            "properties": {
                "protocol": {
                    "type": "string",
                    "const": "pppoe"
                },
                "user-name": {
                    "type": "string"
                },
                "password": {
                    "type": "string"
                }
            }
        },
        "interface.broad-band": {
            "oneOf": [
                {
                    "$ref": "#/$defs/interface.broad-band.wwan"
                },
                {
                    "$ref": "#/$defs/interface.broad-band.pppoe"
                }
            ]
        },
        "interface.ssid.multi-psk": {
            "type": "object",
            "properties": {
                "mac": {
                    "type": "string",
                    "format": "uc-mac"
                },
                "key": {
                    "type": "string",
                    "maxLength": 63,
                    "minLength": 8
                },
                "vlan-id": {
                    "type": "integer",
                    "maximum": 4096,
                    "examples": [
                        3,
                        100,
                        200,
                        4094
                    ]
                }
            }
        },
        "interface.ssid.rrm": {
            "type": "object",
            "properties": {
                "neighbor-reporting": {
                    "type": "boolean",
                    "default": false
                },
                "reduced-neighbor-reporting": {
                    "type": "boolean",
                    "default": false
                },
                "lci": {
                    "type": "string"
                },
                "civic-location": {
                    "type": "string"
                },
                "ftm-responder": {
                    "type": "boolean",
                    "default": false
                },
                "stationary-ap": {
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "interface.ssid.rate-limit": {
            "type": "object",
            "properties": {
                "ingress-rate": {
                    "type": "integer",
                    "default": 0
                },
                "egress-rate": {
                    "type": "integer",
                    "default": 0
                }
            }
        },
        "interface.ssid.roaming": {
            "type": "object",
            "properties": {
                "message-exchange": {
                    "type": "string",
                    "enum": [
                        "air",
                        "ds"
                    ],
                    "default": "ds"
                },
                "generate-psk": {
                    "type": "boolean",
                    "default": false
                },
                "domain-identifier": {
                    "type": "string",
                    "maxLength": 4,
                    "minLength": 4,
                    "examples": [
                        "abcd"
                    ]
                },
                "pmk-r0-key-holder": {
                    "type": "string",
                    "example": "14:DD:20:47:14:E4,14DD204714E4,00112233445566778899aabbccddeeff"
                },
                "pmk-r1-key-holder": {
                    "type": "string",
                    "example": "14:DD:20:47:14:E4,14DD204714E4,00112233445566778899aabbccddeeff"
                }
            }
        },
        "interface.ssid.radius.local-user": {
            "type": "object",
            "properties": {
                "mac": {
                    "type": "string",
                    "format": "uc-mac"
                },
                "user-name": {
                    "type": "string",
                    "minLength": 1
                },
                "password": {
                    "type": "string",
                    "maxLength": 63,
                    "minLength": 8
                },
                "vlan-id": {
                    "type": "integer",
                    "maximum": 4096,
                    "examples": [
                        3,
                        100,
                        200,
                        4094
                    ]
                }
            }
        },
        "interface.ssid.radius.local": {
            "type": "object",
            "properties": {
                "server-identity": {
                    "type": "string",
                    "default": "uCentral"
                },
                "users": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ssid.radius.local-user"
                    }
                }
            }
        },
        "interface.ssid.radius.server": {
            "type": "object",
            "properties": {
                "host": {
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "examples": [
                        1812
                    ]
                },
                "secret": {
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "secondary": {
                    "type": "object",
                    "properties": {
                        "host": {
                            "type": "string",
                            "format": "uc-host",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "port": {
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1812
                            ]
                        },
                        "secret": {
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        }
                    }
                },
                "request-attribute": {
                    "type": "array",
                    "items": {
                        "anyOf": [
                            {
                                "type": "object",
                                "properties": {
                                    "vendor-id": {
                                        "type": "integer",
                                        "maximum": 65535,
                                        "minimum": 1
                                    },
                                    "vendor-attributes": {
                                        "type": "array",
                                        "items": {
                                            "type": "object",
                                            "properties": {
                                                "id": {
                                                    "type": "integer",
                                                    "maximum": 255,
                                                    "minimum": 1
                                                },
                                                "value": {
                                                    "type": "string"
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer",
                                        "maximum": 255,
                                        "minimum": 1
                                    },
                                    "value": {
                                        "type": "integer",
                                        "maximum": 4294967295,
                                        "minimum": 0
                                    }
                                },
                                "examples": [
                                    {
                                        "id": 27,
                                        "value": 900
                                    },
                                    {
                                        "id": 56,
                                        "value": 1004
                                    }
                                ]
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer",
                                        "maximum": 255,
                                        "minimum": 1
                                    },
                                    "value": {
                                        "type": "string"
                                    }
                                },
                                "examples": [
                                    {
                                        "id": 32,
                                        "value": "My NAS ID"
                                    },
                                    {
                                        "id": 126,
                                        "value": "Example Operator"
                                    }
                                ]
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer",
                                        "maximum": 255,
                                        "minimum": 1
                                    },
                                    "hex-value": {
                                        "type": "string"
                                    }
                                },
                                "examples": [
                                    {
                                        "id": 32,
                                        "value": "0a0b0c0d"
                                    }
                                ]
                            }
                        ]
                    }
                }
            }
        },
        "interface.ssid.radius.health": {
            "type": "object",
            "properties": {
                "username": {
                    "type": "string"
                },
                "password": {
                    "type": "string"
                }
            }
        },
        "interface.ssid.radius": {
            "type": "object",
            "properties": {
                "nas-identifier": {
                    "type": "string"
                },
                "chargeable-user-id": {
                    "type": "boolean",
                    "default": false
                },
                "local": {
                    "$ref": "#/$defs/interface.ssid.radius.local"
                },
                "dynamic-authorization": {
                    "type": "object",
                    "properties": {
                        "host": {
                            "type": "string",
                            "format": "uc-ip",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "port": {
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1812
                            ]
                        },
                        "secret": {
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        }
                    }
                },
                "authentication": {
                    "allOf": [
                        {
                            "$ref": "#/$defs/interface.ssid.radius.server"
                        },
                        {
                            "type": "object",
                            "properties": {
                                "mac-filter": {
                                    "type": "boolean",
                                    "default": false
                                }
                            }
                        }
                    ]
                },
                "accounting": {
                    "allOf": [
                        {
                            "$ref": "#/$defs/interface.ssid.radius.server"
                        },
                        {
                            "type": "object",
                            "properties": {
                                "interval": {
                                    "type": "integer",
                                    "maximum": 600,
                                    "minimum": 60,
                                    "default": 60
                                }
                            }
                        }
                    ]
                },
                "health": {
                    "$ref": "#/$defs/interface.ssid.radius.health"
                }
            }
        },
        "interface.ssid.certificates": {
            "type": "object",
            "properties": {
                "use-local-certificates": {
                    "type": "boolean",
                    "default": false
                },
                "ca-certificate": {
                    "type": "string"
                },
                "certificate": {
                    "type": "string"
                },
                "private-key": {
                    "type": "string"
                },
                "private-key-password": {
                    "type": "string"
                }
            }
        },
        "interface.ssid.pass-point": {
            "type": "object",
            "properties": {
                "venue-name": {
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "venue-group": {
                    "type": "integer",
                    "maximum": 32
                },
                "venue-type": {
                    "type": "integer",
                    "maximum": 32
                },
                "venue-url": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "uri"
                    }
                },
                "auth-type": {
                    "type": "object",
                    "properties": {
                        "type": {
                            "type": "string",
                            "enum": [
                                "terms-and-conditions",
                                "online-enrollment",
                                "http-redirection",
                                "dns-redirection"
                            ]
                        },
                        "uri": {
                            "type": "string",
                            "format": "uri",
                            "examples": [
                                "https://operator.example.org/wireless-access/terms-and-conditions.html",
                                "http://www.example.com/redirect/me/here/"
                            ]
                        }
                    },
                    "minLength": 2,
                    "maxLength": 2
                },
                "domain-name": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "hostname"
                    }
                },
                "nai-realm": {
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "osen": {
                    "type": "boolean"
                },
                "anqp-domain": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 0
                },
                "anqp-3gpp-cell-net": {
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "friendly-name": {
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "access-network-type": {
                    "type": "integer",
                    "maximum": 15,
                    "default": 0
                },
                "internet": {
                    "type": "boolean",
                    "default": true
                },
                "asra": {
                    "type": "boolean",
                    "default": false
                },
                "esr": {
                    "type": "boolean",
                    "default": false
                },
                "uesa": {
                    "type": "boolean",
                    "default": false
                },
                "hessid": {
                    "type": "string",
                    "example": "00:11:22:33:44:55"
                },
                "roaming-consortium": {
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "disable-dgaf": {
                    "type": "boolean",
                    "default": false
                },
                "ipaddr-type-available": {
                    "type": "integer",
                    "maximum": 255
                },
                "connection-capability": {
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "icons": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "width": {
                                "type": "integer",
                                "examples": [
                                    64
                                ]
                            },
                            "height": {
                                "type": "integer",
                                "examples": [
                                    64
                                ]
                            },
                            "type": {
                                "type": "string",
                                "examples": [
                                    "image/png"
                                ]
                            },
                            "icon": {
                                "type": "string",
                                "format": "uc-base64"
                            },
                            "language": {
                                "type": "string",
                                "pattern": "^[a-z][a-z][a-z]$",
                                "examples": [
                                    "eng",
                                    "fre",
                                    "ger",
                                    "ita"
                                ]
                            }
                        },
                        "examples": [
                            {
                                "width": 32,
                                "height": 32,
                                "type": "image/png",
                                "language": "eng",
                                "icon": "R0lGODlhEAAQAMQAAORHHOVSKudfOulrSOp3WOyDZu6QdvCchPGolfO0o/XBs/fNwfjZ0frl3/zy7////wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAkAABAALAAAAAAQABAAAAVVICSOZGlCQAosJ6mu7fiyZeKqNKToQGDsM8hBADgUXoGAiqhSvp5QAnQKGIgUhwFUYLCVDFCrKUE1lBavAViFIDlTImbKC5Gm2hB0SlBCBMQiB0UjIQA7"
                            }
                        ]
                    }
                },
                "wan-metrics": {
                    "type": "object",
                    "properties": {
                        "info": {
                            "type": "string",
                            "enum": [
                                "up",
                                "down",
                                "testing"
                            ]
                        },
                        "downlink": {
                            "type": "integer"
                        },
                        "uplink": {
                            "type": "integer"
                        }
                    }
                }
            }
        },
        "interface.ssid.quality-thresholds": {
            "type": "object",
            "properties": {
                "probe-request-rssi": {
                    "type": "integer"
                },
                "association-request-rssi": {
                    "type": "integer"
                },
                "client-kick-rssi": {
                    "type": "integer"
                },
                "client-kick-ban-time": {
                    "type": "integer",
                    "default": 0
                }
            }
        },
        "interface.ssid.acl": {
            "type": "object",
            "properties": {
                "mode": {
                    "type": "string",
                    "enum": [
                        "allow",
                        "deny"
                    ]
                },
                "mac-address": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "uc-mac"
                    }
                }
            }
        },
        "service.captive.click": {
            "type": "object",
            "properties": {
                "auth-mode": {
                    "type": "string",
                    "const": "click-to-continue"
                }
            }
        },
        "service.captive.radius": {
            "type": "object",
            "properties": {
                "auth-mode": {
                    "type": "string",
                    "const": "radius"
                },
                "auth-server": {
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "auth-port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 1812
                },
                "auth-secret": {
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "acct-server": {
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "acct-port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 1812
                },
                "acct-secret": {
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "acct-interval": {
                    "type": "integer",
                    "default": 600
                }
            }
        },
        "service.captive.credentials": {
            "type": "object",
            "properties": {
                "auth-mode": {
                    "type": "string",
                    "const": "credentials"
                },
                "credentials": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "username": {
                                "type": "string"
                            },
                            "password": {
                                "type": "string"
                            }
                        }
                    }
                }
            }
        },
        "service.captive.uam": {
            "type": "object",
            "properties": {
                "auth-mode": {
                    "type": "string",
                    "const": "uam"
                },
                "uam-port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 3990
                },
                "uam-secret": {
                    "type": "string"
                },
                "uam-server": {
                    "type": "string"
                },
                "nasid": {
                    "type": "string"
                },
                "nasmac": {
                    "type": "string"
                },
                "auth-server": {
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "auth-port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 1812
                },
                "auth-secret": {
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "acct-server": {
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "acct-port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1024,
                    "default": 1812
                },
                "acct-secret": {
                    "type": "string",
                    "examples": [
                        "secret"
                    ]
                },
                "acct-interval": {
                    "type": "integer",
                    "default": 600
                },
                "ssid": {
                    "type": "string"
                },
                "mac-format": {
                    "type": "string",
                    "enum": [
                        "aabbccddeeff",
                        "aa-bb-cc-dd-ee-ff",
                        "aa:bb:cc:dd:ee:ff",
                        "AABBCCDDEEFF",
                        "AA:BB:CC:DD:EE:FF",
                        "AA-BB-CC-DD-EE-FF"
                    ]
                },
                "final-redirect-url": {
                    "type": "string",
                    "enum": [
                        "default",
                        "uam"
                    ]
                },
                "mac-auth": {
                    "type": "boolean",
                    "default": "default"
                },
                "radius-gw-proxy": {
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "service.captive": {
            "allOf": [
                {
                    "oneOf": [
                        {
                            "$ref": "#/$defs/service.captive.click"
                        },
                        {
                            "$ref": "#/$defs/service.captive.radius"
                        },
                        {
                            "$ref": "#/$defs/service.captive.credentials"
                        },
                        {
                            "$ref": "#/$defs/service.captive.uam"
                        }
                    ]
                },
                {
                    "type": "object",
                    "properties": {
                        "walled-garden-fqdn": {
                            "type": "array",
                            "items": {
                                "type": "string"
                            }
                        },
                        "walled-garden-ipaddr": {
                            "type": "array",
                            "items": {
                                "type": "string",
                                "format": "uc-ip"
                            }
                        },
                        "web-root": {
                            "type": "string",
                            "format": "uc-base64"
                        },
                        "idle-timeout": {
                            "type": "integer",
                            "default": 600
                        },
                        "session-timeout": {
                            "type": "integer"
                        }
                    }
                }
            ]
        },
        "interface.ssid": {
            "type": "object",
            "properties": {
                "purpose": {
                    "type": "string",
                    "enum": [
                        "user-defined",
                        "onboarding-ap",
                        "onboarding-sta"
                    ],
                    "default": "user-defined"
                },
                "name": {
                    "type": "string",
                    "maxLength": 32,
                    "minLength": 1
                },
                "wifi-bands": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "2G",
                            "5G",
                            "5G-lower",
                            "5G-upper",
                            "6G"
                        ]
                    }
                },
                "bss-mode": {
                    "type": "string",
                    "enum": [
                        "ap",
                        "sta",
                        "mesh",
                        "wds-ap",
                        "wds-sta",
                        "wds-repeater"
                    ],
                    "default": "ap"
                },
                "bssid": {
                    "type": "string",
                    "format": "uc-mac"
                },
                "hidden-ssid": {
                    "type": "boolean"
                },
                "isolate-clients": {
                    "type": "boolean"
                },
                "strict-forwarding": {
                    "type": "boolean",
                    "default": false
                },
                "power-save": {
                    "type": "boolean"
                },
                "rts-threshold": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1
                },
                "broadcast-time": {
                    "type": "boolean"
                },
                "unicast-conversion": {
                    "type": "boolean",
                    "default": true
                },
                "services": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "wifi-steering"
                        ]
                    }
                },
                "dtim-period": {
                    "type": "integer",
                    "default": 2,
                    "maximum": 255,
                    "minimum": 1
                },
                "maximum-clients": {
                    "type": "integer",
                    "example": 64
                },
                "proxy-arp": {
                    "type": "boolean",
                    "default": true
                },
                "disassoc-low-ack": {
                    "decription": "Disassociate stations based on excessive transmission failures or other indications of connection loss.",
                    "type": "boolean",
                    "default": false
                },
                "vendor-elements": {
                    "decription": "This option allows embedding custom vendor specific IEs inside the beacons of a BSS in AP mode.",
                    "type": "string"
                },
                "tip-information-element": {
                    "decription": "The device will broadcast the TIP vendor IE inside its beacons if this option is enabled.",
                    "type": "boolean",
                    "default": true
                },
                "fils-discovery-interval": {
                    "type": "integer",
                    "default": 20,
                    "maximum": 20
                },
                "encryption": {
                    "$ref": "#/$defs/interface.ssid.encryption"
                },
                "multi-psk": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ssid.multi-psk"
                    }
                },
                "rrm": {
                    "$ref": "#/$defs/interface.ssid.rrm"
                },
                "rate-limit": {
                    "$ref": "#/$defs/interface.ssid.rate-limit"
                },
                "roaming": {
                    "anyOf": [
                        {
                            "$ref": "#/$defs/interface.ssid.roaming"
                        },
                        {
                            "type": "boolean"
                        }
                    ]
                },
                "radius": {
                    "$ref": "#/$defs/interface.ssid.radius"
                },
                "certificates": {
                    "$ref": "#/$defs/interface.ssid.certificates"
                },
                "pass-point": {
                    "$ref": "#/$defs/interface.ssid.pass-point"
                },
                "quality-thresholds": {
                    "$ref": "#/$defs/interface.ssid.quality-thresholds"
                },
                "access-control-list": {
                    "$ref": "#/$defs/interface.ssid.acl"
                },
                "captive": {
                    "$ref": "#/$defs/service.captive"
                },
                "hostapd-bss-raw": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "ap_table_expiration_time=3600",
                            "device_type=6-0050F204-1",
                            "ieee80211h=1",
                            "rssi_ignore_probe_request=-75",
                            "time_zone=EST5",
                            "uuid=12345678-9abc-def0-1234-56789abcdef0",
                            "venue_url=1:http://www.example.com/info-eng",
                            "wpa_deny_ptk0_rekey=0"
                        ]
                    }
                }
            }
        },
        "interface.tunnel.mesh": {
            "type": "object",
            "properties": {
                "proto": {
                    "type": "string",
                    "const": "mesh"
                }
            }
        },
        "interface.tunnel.vxlan": {
            "type": "object",
            "properties": {
                "proto": {
                    "type": "string",
                    "const": "vxlan"
                },
                "peer-address": {
                    "type": "string",
                    "format": "ipv4",
                    "example": "192.168.100.1"
                },
                "peer-port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1,
                    "examples": [
                        4789
                    ]
                }
            }
        },
        "interface.tunnel.l2tp": {
            "type": "object",
            "properties": {
                "proto": {
                    "type": "string",
                    "const": "l2tp"
                },
                "server": {
                    "type": "string",
                    "format": "ipv4",
                    "example": "192.168.100.1"
                },
                "user-name": {
                    "type": "string"
                },
                "password": {
                    "type": "string"
                }
            }
        },
        "interface.tunnel.gre": {
            "type": "object",
            "properties": {
                "proto": {
                    "type": "string",
                    "const": "gre"
                },
                "peer-address": {
                    "type": "string",
                    "format": "ipv4",
                    "example": "192.168.100.1"
                },
                "dhcp-healthcheck": {
                    "type": "boolean",
                    "default": false
                },
                "dont-fragment": {
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "interface.tunnel.gre6": {
            "type": "object",
            "properties": {
                "proto": {
                    "type": "string",
                    "const": "gre6"
                },
                "peer-address": {
                    "type": "string",
                    "format": "ipv6",
                    "example": "2405:200:802:600:61::1"
                },
                "dhcp-healthcheck": {
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "interface.tunnel": {
            "oneOf": [
                {
                    "$ref": "#/$defs/interface.tunnel.mesh"
                },
                {
                    "$ref": "#/$defs/interface.tunnel.vxlan"
                },
                {
                    "$ref": "#/$defs/interface.tunnel.l2tp"
                },
                {
                    "$ref": "#/$defs/interface.tunnel.gre"
                },
                {
                    "$ref": "#/$defs/interface.tunnel.gre6"
                }
            ]
        },
        "interface": {
            "type": "object",
            "properties": {
                "name": {
                    "type": "string",
                    "examples": [
                        "LAN"
                    ]
                },
                "role": {
                    "type": "string",
                    "enum": [
                        "upstream",
                        "downstream"
                    ]
                },
                "isolate-hosts": {
                    "type": "boolean"
                },
                "metric": {
                    "type": "integer",
                    "maximum": 4294967295,
                    "minimum": 0
                },
                "mtu": {
                    "type": "integer",
                    "maximum": 1500,
                    "minimum": 1280
                },
                "services": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "ssh",
                            "lldp"
                        ]
                    }
                },
                "vlan-awareness": {
                    "type": "object",
                    "properties": {
                        "first": {
                            "type": "integer"
                        },
                        "last": {
                            "type": "integer"
                        }
                    }
                },
                "vlan": {
                    "$ref": "#/$defs/interface.vlan"
                },
                "bridge": {
                    "$ref": "#/$defs/interface.bridge"
                },
                "ethernet": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ethernet"
                    }
                },
                "ipv4": {
                    "$ref": "#/$defs/interface.ipv4"
                },
                "ipv6": {
                    "$ref": "#/$defs/interface.ipv6"
                },
                "broad-band": {
                    "$ref": "#/$defs/interface.broad-band"
                },
                "ssids": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ssid"
                    }
                },
                "tunnel": {
                    "$ref": "#/$defs/interface.tunnel"
                }
            }
        },
        "service.lldp": {
            "type": "object",
            "properties": {
                "describe": {
                    "type": "string",
                    "default": "uCentral Access Point"
                },
                "location": {
                    "type": "string",
                    "default": "uCentral Network"
                }
            }
        },
        "service.ssh": {
            "type": "object",
            "properties": {
                "port": {
                    "type": "integer",
                    "maximum": 65535,
                    "default": 22
                },
                "authorized-keys": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAAAgQC0ghdSd2D2y08TFowZLMZn3x1/Djw3BkNsIeHt/Z+RaXwvfV1NQAnNdaOngMT/3uf5jZtYxhpl+dbZtRhoUPRvKflKBeFHYBqjZVzD3r4ns2Ofm2UpHlbdOpMuy9oeTSCeF0IKZZ6szpkvSirQogeP2fe9KRkzQpiza6YxxaJlWw== user@example",
                            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIJ4FDjyCsg+1Mh2C5G7ibR3z0Kw1dU57kfXebLRwS6CL bob@work",
                            "ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBP/JpJ/KHtKKImzISBDwLO0/EwytIr4pGZQXcP6GCSHchLMyfjf147KNlF9gC+3FibzqKH02EiQspVhRgfuK6y0= alice@home"
                        ]
                    }
                },
                "password-authentication": {
                    "type": "boolean",
                    "default": true
                },
                "enable": {
                    "type": "boolean"
                }
            }
        },
        "service.ntp": {
            "type": "object",
            "properties": {
                "servers": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "uc-host"
                    },
                    "examples": [
                        "0.openwrt.pool.ntp.org"
                    ]
                },
                "local-server": {
                    "type": "boolean",
                    "examples": [
                        true
                    ]
                }
            }
        },
        "service.mdns": {
            "type": "object",
            "properties": {
                "enable": {
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "service.rtty": {
            "type": "object",
            "properties": {
                "host": {
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "port": {
                    "type": "integer",
                    "maximum": 65535,
                    "default": 5912
                },
                "token": {
                    "type": "string",
                    "maxLength": 32,
                    "minLength": 32,
                    "examples": [
                        "01234567890123456789012345678901"
                    ]
                },
                "mutual-tls": {
                    "type": "boolean",
                    "default": true
                }
            }
        },
        "service.log": {
            "type": "object",
            "properties": {
                "host": {
                    "type": "string",
                    "format": "uc-host",
                    "examples": [
                        "192.168.1.10"
                    ]
                },
                "port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 100,
                    "examples": [
                        2000
                    ]
                },
                "proto": {
                    "type": "string",
                    "enum": [
                        "tcp",
                        "udp"
                    ],
                    "default": "udp"
                },
                "size": {
                    "type": "integer",
                    "minimum": 32,
                    "default": 1000
                },
                "priority": {
                    "type": "integer",
                    "minimum": 0,
                    "default": 7
                }
            }
        },
        "service.http": {
            "type": "object",
            "properties": {
                "http-port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1,
                    "default": 80
                },
                "enable": {
                    "type": "boolean"
                }
            }
        },
        "service.igmp": {
            "type": "object",
            "properties": {
                "enable": {
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "service.ieee8021x": {
            "type": "object",
            "properties": {
                "mode": {
                    "type": "string",
                    "enum": [
                        "radius",
                        "user"
                    ]
                },
                "port-filter": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            {
                                "LAN1": null
                            }
                        ]
                    }
                },
                "users": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ssid.radius.local-user"
                    }
                },
                "radius": {
                    "type": "object",
                    "properties": {
                        "nas-identifier": {
                            "type": "string"
                        },
                        "auth-server-addr": {
                            "type": "string",
                            "format": "uc-host",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "auth-server-port": {
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1812
                            ]
                        },
                        "auth-server-secret": {
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        },
                        "acct-server-addr": {
                            "type": "string",
                            "format": "uc-host",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "acct-server-port": {
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1813
                            ]
                        },
                        "acct-server-secret": {
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        },
                        "coa-server-addr": {
                            "type": "string",
                            "format": "uc-host",
                            "examples": [
                                "192.168.1.10"
                            ]
                        },
                        "coa-server-port": {
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1024,
                            "examples": [
                                1814
                            ]
                        },
                        "coa-server-secret": {
                            "type": "string",
                            "examples": [
                                "secret"
                            ]
                        }
                    }
                }
            }
        },
        "service.radius-proxy": {
            "type": "object",
            "properties": {
                "proxy-secret": {
                    "type": "string",
                    "default": "secret"
                },
                "realms": {
                    "type": "array",
                    "items": {
                        "anyOf": [
                            {
                                "type": "object",
                                "properties": {
                                    "protocol": {
                                        "type": "string",
                                        "enum": [
                                            "radsec"
                                        ],
                                        "default": "radsec"
                                    },
                                    "realm": {
                                        "type": "array",
                                        "items": {
                                            "type": "string",
                                            "default": "*"
                                        }
                                    },
                                    "auto-discover": {
                                        "type": "boolean",
                                        "default": false
                                    },
                                    "host": {
                                        "type": "string",
                                        "format": "uc-host",
                                        "examples": [
                                            "192.168.1.10"
                                        ]
                                    },
                                    "port": {
                                        "type": "integer",
                                        "maximum": 65535,
                                        "default": 2083
                                    },
                                    "secret": {
                                        "type": "string"
                                    },
                                    "use-local-certificates": {
                                        "type": "boolean",
                                        "default": false
                                    },
                                    "ca-certificate": {
                                        "type": "string"
                                    },
                                    "certificate": {
                                        "type": "string"
                                    },
                                    "private-key": {
                                        "type": "string"
                                    },
                                    "private-key-password": {
                                        "type": "string"
                                    }
                                }
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "protocol": {
                                        "type": "string",
                                        "enum": [
                                            "radius"
                                        ]
                                    },
                                    "realm": {
                                        "type": "array",
                                        "items": {
                                            "type": "string",
                                            "default": "*"
                                        }
                                    },
                                    "auth-server": {
                                        "type": "string",
                                        "format": "uc-host",
                                        "examples": [
                                            "192.168.1.10"
                                        ]
                                    },
                                    "auth-port": {
                                        "type": "integer",
                                        "maximum": 65535,
                                        "minimum": 1024,
                                        "examples": [
                                            1812
                                        ]
                                    },
                                    "auth-secret": {
                                        "type": "string",
                                        "examples": [
                                            "secret"
                                        ]
                                    },
                                    "acct-server": {
                                        "type": "string",
                                        "format": "uc-host",
                                        "examples": [
                                            "192.168.1.10"
                                        ]
                                    },
                                    "acct-port": {
                                        "type": "integer",
                                        "maximum": 65535,
                                        "minimum": 1024,
                                        "examples": [
                                            1812
                                        ]
                                    },
                                    "acct-secret": {
                                        "type": "string",
                                        "examples": [
                                            "secret"
                                        ]
                                    }
                                }
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "protocol": {
                                        "type": "string",
                                        "enum": [
                                            "block"
                                        ]
                                    },
                                    "realm": {
                                        "type": "array",
                                        "items": {
                                            "type": "string",
                                            "default": "*"
                                        }
                                    },
                                    "message": {
                                        "type": "string",
                                        "items": {
                                            "type": "string",
                                            "default": "blocked"
                                        }
                                    }
                                }
                            }
                        ]
                    }
                }
            }
        },
        "service.online-check": {
            "type": "object",
            "properties": {
                "ping-hosts": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "format": "uc-host",
                        "examples": [
                            "192.168.1.10"
                        ]
                    }
                },
                "download-hosts": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "examples": [
                            "www.example.org"
                        ]
                    }
                },
                "check-interval": {
                    "type": "number",
                    "default": 60
                },
                "check-threshold": {
                    "type": "number",
                    "default": 1
                },
                "action": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "wifi",
                            "leds"
                        ]
                    }
                }
            }
        },
        "service.data-plane": {
            "type": "object",
            "properties": {
                "ingress-filters": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "name": {
                                "type": "string"
                            },
                            "program": {
                                "type": "string",
                                "format": "uc-base64"
                            }
                        }
                    }
                }
            }
        },
        "service.wifi-steering": {
            "type": "object",
            "properties": {
                "mode": {
                    "type": "string",
                    "enum": [
                        "local",
                        "none"
                    ],
                    "examples": [
                        "local"
                    ]
                },
                "assoc-steering": {
                    "type": "boolean",
                    "default": false
                },
                "required-snr": {
                    "type": "integer",
                    "default": 0
                },
                "required-probe-snr": {
                    "type": "integer",
                    "default": 0
                },
                "required-roam-snr": {
                    "type": "integer",
                    "default": 0
                },
                "load-kick-threshold": {
                    "type": "integer",
                    "default": 0
                },
                "auto-channel": {
                    "type": "boolean",
                    "default": false
                },
                "ipv6": {
                    "type": "boolean",
                    "default": false
                }
            }
        },
        "service.quality-of-service.class-selector": {
            "type": "string",
            "enum": [
                "CS0",
                "CS1",
                "CS2",
                "CS3",
                "CS4",
                "CS5",
                "CS6",
                "CS7",
                "AF11",
                "AF12",
                "AF13",
                "AF21",
                "AF22",
                "AF23",
                "AF31",
                "AF32",
                "AF33",
                "AF41",
                "AF42",
                "AF43",
                "DF",
                "EF",
                "VA",
                "LE"
            ]
        },
        "service.quality-of-service": {
            "type": "object",
            "properties": {
                "select-ports": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "default": "WAN"
                    }
                },
                "bandwidth-up": {
                    "type": "integer",
                    "default": 0
                },
                "bandwidth-down": {
                    "type": "integer",
                    "default": 0
                },
                "bulk-detection": {
                    "type": "object",
                    "properties": {
                        "dscp": {
                            "$ref": "#/$defs/service.quality-of-service.class-selector",
                            "default": "CS0"
                        },
                        "packets-per-second": {
                            "type": "number",
                            "default": 0
                        }
                    }
                },
                "services": {
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "classifier": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "dscp": {
                                "$ref": "#/$defs/service.quality-of-service.class-selector",
                                "default": "CS1"
                            },
                            "ports": {
                                "type": "array",
                                "items": {
                                    "type": "object",
                                    "properties": {
                                        "protocol": {
                                            "type": "string",
                                            "enum": [
                                                "any",
                                                "tcp",
                                                "udp"
                                            ],
                                            "default": "any"
                                        },
                                        "port": {
                                            "type": "integer"
                                        },
                                        "range-end": {
                                            "type": "integer"
                                        },
                                        "reclassify": {
                                            "type": "boolean",
                                            "default": true
                                        }
                                    }
                                }
                            },
                            "dns": {
                                "type": "array",
                                "items": {
                                    "type": "object",
                                    "properties": {
                                        "fqdn": {
                                            "type": "string",
                                            "format": "uc-fqdn"
                                        },
                                        "suffix-matching": {
                                            "type": "boolean",
                                            "default": true
                                        },
                                        "reclassify": {
                                            "type": "boolean",
                                            "default": true
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        },
        "service.facebook-wifi": {
            "type": "object",
            "properties": {
                "vendor-id": {
                    "type": "string"
                },
                "gateway-id": {
                    "type": "string"
                },
                "secret": {
                    "type": "string"
                }
            }
        },
        "service.airtime-fairness": {
            "type": "object",
            "properties": {
                "voice-weight": {
                    "type": "number",
                    "default": 4
                },
                "packet-threshold": {
                    "type": "number",
                    "default": 100
                },
                "bulk-threshold": {
                    "type": "number",
                    "default": 50
                },
                "priority-threshold": {
                    "type": "number",
                    "default": 30
                },
                "weight-normal": {
                    "type": "number",
                    "default": 256
                },
                "weight-priority": {
                    "type": "number",
                    "default": 394
                },
                "weight-bulk": {
                    "type": "number",
                    "default": 128
                }
            }
        },
        "service.wireguard-overlay": {
            "type": "object",
            "properties": {
                "proto": {
                    "type": "string",
                    "const": "wireguard-overlay"
                },
                "private-key": {
                    "type": "string"
                },
                "peer-port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1,
                    "default": 3456
                },
                "peer-exchange-port": {
                    "type": "integer",
                    "maximum": 65535,
                    "minimum": 1,
                    "default": 3458
                },
                "root-node": {
                    "type": "object",
                    "properties": {
                        "key": {
                            "type": "string"
                        },
                        "endpoint": {
                            "type": "string",
                            "format": "uc-ip"
                        },
                        "ipaddr": {
                            "type": "array",
                            "items": {
                                "type": "string",
                                "format": "uc-ip"
                            }
                        }
                    }
                },
                "hosts": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "name": {
                                "type": "string"
                            },
                            "key": {
                                "type": "string"
                            },
                            "endpoint": {
                                "type": "string",
                                "format": "uc-ip"
                            },
                            "subnet": {
                                "type": "array",
                                "items": {
                                    "type": "string",
                                    "format": "uc-cidr"
                                }
                            },
                            "ipaddr": {
                                "type": "array",
                                "items": {
                                    "type": "string",
                                    "format": "uc-ip"
                                }
                            }
                        }
                    }
                },
                "vxlan": {
                    "type": "object",
                    "properties": {
                        "port": {
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 1,
                            "default": 4789
                        },
                        "mtu": {
                            "type": "integer",
                            "maximum": 65535,
                            "minimum": 256,
                            "default": 1420
                        },
                        "isolate": {
                            "type": "boolean",
                            "default": true
                        }
                    }
                }
            }
        },
        "service.gps": {
            "type": "object",
            "properties": {
                "adjust-time": {
                    "type": "boolean",
                    "default": false
                },
                "baud-rate": {
                    "type": "integer",
                    "enum": [
                        2400,
                        4800,
                        9600,
                        19200
                    ]
                }
            }
        },
        "service.dhcp-relay": {
            "type": "object",
            "properties": {
                "select-ports": {
                    "type": "array",
                    "items": {
                        "type": "string"
                    }
                },
                "vlans": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "vlan": {
                                "type": "number"
                            },
                            "relay-server": {
                                "type": "string",
                                "format": "uc-ip"
                            },
                            "circuit-id-format": {
                                "type": "string",
                                "enum": [
                                    "vlan-id",
                                    "ap-mac",
                                    "ssid"
                                ],
                                "default": "vlan-id"
                            },
                            "remote-id-format": {
                                "type": "string",
                                "enum": [
                                    "vlan-id",
                                    "ap-mac",
                                    "ssid"
                                ],
                                "default": "ap-mac"
                            }
                        }
                    }
                }
            }
        },
        "service.admin-ui": {
            "type": "object",
            "properties": {
                "wifi-ssid": {
                    "type": "string",
                    "default": "Maverick",
                    "maxLength": 32,
                    "minLength": 1
                },
                "wifi-key": {
                    "type": "string",
                    "maxLength": 63,
                    "minLength": 8
                },
                "wifi-bands": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "2G",
                            "5G",
                            "5G-lower",
                            "5G-upper",
                            "6G"
                        ]
                    }
                },
                "offline-trigger": {
                    "type": "number"
                }
            }
        },
        "service.rrm": {
            "type": "object",
            "properties": {
                "beacon-request-assoc": {
                    "type": "boolean",
                    "default": true
                },
                "station-stats-interval": {
                    "type": "number"
                }
            }
        },
        "service.telnet": {
            "type": "object",
            "properties": {
                "enable": {
                    "type": "boolean"
                }
            }
        },
        "service": {
            "type": "object",
            "properties": {
                "lldp": {
                    "$ref": "#/$defs/service.lldp"
                },
                "ssh": {
                    "$ref": "#/$defs/service.ssh"
                },
                "ntp": {
                    "$ref": "#/$defs/service.ntp"
                },
                "mdns": {
                    "$ref": "#/$defs/service.mdns"
                },
                "rtty": {
                    "$ref": "#/$defs/service.rtty"
                },
                "log": {
                    "$ref": "#/$defs/service.log"
                },
                "http": {
                    "$ref": "#/$defs/service.http"
                },
                "igmp": {
                    "$ref": "#/$defs/service.igmp"
                },
                "ieee8021x": {
                    "$ref": "#/$defs/service.ieee8021x"
                },
                "radius-proxy": {
                    "$ref": "#/$defs/service.radius-proxy"
                },
                "online-check": {
                    "$ref": "#/$defs/service.online-check"
                },
                "data-plane": {
                    "$ref": "#/$defs/service.data-plane"
                },
                "wifi-steering": {
                    "$ref": "#/$defs/service.wifi-steering"
                },
                "quality-of-service": {
                    "$ref": "#/$defs/service.quality-of-service"
                },
                "facebook-wifi": {
                    "$ref": "#/$defs/service.facebook-wifi"
                },
                "airtime-fairness": {
                    "$ref": "#/$defs/service.airtime-fairness"
                },
                "wireguard-overlay": {
                    "$ref": "#/$defs/service.wireguard-overlay"
                },
                "captive": {
                    "$ref": "#/$defs/service.captive"
                },
                "gps": {
                    "$ref": "#/$defs/service.gps"
                },
                "dhcp-relay": {
                    "$ref": "#/$defs/service.dhcp-relay"
                },
                "admin-ui": {
                    "$ref": "#/$defs/service.admin-ui"
                },
                "rrm": {
                    "$ref": "#/$defs/service.rrm"
                },
                "telnet": {
                    "$ref": "#/$defs/service.telnet"
                }
            }
        },
        "metrics.statistics": {
            "type": "object",
            "properties": {
                "interval": {
                    "type": "integer",
                    "minimum": 60
                },
                "types": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "ssids",
                            "lldp",
                            "clients",
                            "tid-stats"
                        ]
                    }
                },
                "wired-clients-max-num": {
                    "type": "integer"
                }
            }
        },
        "metrics.health": {
            "type": "object",
            "properties": {
                "interval": {
                    "type": "integer",
                    "minimum": 60
                },
                "dhcp-local": {
                    "type": "boolean",
                    "default": true
                },
                "dhcp-remote": {
                    "type": "boolean",
                    "default": false
                },
                "dns-local": {
                    "type": "boolean",
                    "default": true
                },
                "dns-remote": {
                    "type": "boolean",
                    "default": true
                }
            }
        },
        "metrics.wifi-frames": {
            "type": "object",
            "properties": {
                "filters": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "probe",
                            "auth",
                            "assoc",
                            "disassoc",
                            "deauth",
                            "local-deauth",
                            "inactive-deauth",
                            "key-mismatch",
                            "beacon-report",
                            "radar-detected"
                        ]
                    }
                }
            }
        },
        "metrics.dhcp-snooping": {
            "type": "object",
            "properties": {
                "filters": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "ack",
                            "discover",
                            "offer",
                            "request",
                            "solicit",
                            "reply",
                            "renew"
                        ]
                    }
                }
            }
        },
        "metrics.wifi-scan": {
            "type": "object",
            "properties": {
                "interval": {
                    "type": "integer"
                },
                "verbose": {
                    "type": "boolean"
                },
                "information-elements": {
                    "type": "boolean"
                }
            }
        },
        "metrics.telemetry": {
            "type": "object",
            "properties": {
                "interval": {
                    "type": "integer"
                },
                "types": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "ssh",
                            "health",
                            "health.dns",
                            "health.dhcp",
                            "health.radius",
                            "health.memory",
                            "client",
                            "client.join",
                            "client.leave",
                            "client.key-mismatch",
                            "wifi",
                            "wifi.start",
                            "wifi.stop",
                            "wired",
                            "wired.carrier-up",
                            "wired.carrier-down",
                            "unit",
                            "unit.boot-up"
                        ]
                    }
                }
            }
        },
        "metrics.realtime": {
            "type": "object",
            "properties": {
                "types": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": [
                            "ssh",
                            "health",
                            "health.dns",
                            "health.dhcp",
                            "health.radius",
                            "health.memory",
                            "client",
                            "client.join",
                            "client.leave",
                            "client.key-mismatch",
                            "wifi",
                            "wifi.start",
                            "wifi.stop",
                            "wired",
                            "wired.carrier-up",
                            "wired.carrier-down",
                            "unit",
                            "unit.boot-up"
                        ]
                    }
                }
            }
        },
        "metrics": {
            "type": "object",
            "properties": {
                "statistics": {
                    "$ref": "#/$defs/metrics.statistics"
                },
                "health": {
                    "$ref": "#/$defs/metrics.health"
                },
                "wifi-frames": {
                    "$ref": "#/$defs/metrics.wifi-frames"
                },
                "dhcp-snooping": {
                    "$ref": "#/$defs/metrics.dhcp-snooping"
                },
                "wifi-scan": {
                    "$ref": "#/$defs/metrics.wifi-scan"
                },
                "telemetry": {
                    "$ref": "#/$defs/metrics.telemetry"
                },
                "realtime": {
                    "$ref": "#/$defs/metrics.realtime"
                }
            }
        },
        "config-raw": {
            "type": "array",
            "items": {
                "type": "array",
                "minItems": 2,
                "items": {
                    "type": "string"
                },
                "examples": [
                    [
                        "set",
                        "system.@system[0].timezone",
                        "GMT0"
                    ],
                    [
                        "delete",
                        "firewall.@zone[0]"
                    ],
                    [
                        "delete",
                        "dhcp.wan"
                    ],
                    [
                        "add",
                        "dhcp",
                        "dhcp"
                    ],
                    [
                        "add-list",
                        "system.ntp.server",
                        "0.pool.example.org"
                    ],
                    [
                        "del-list",
                        "system.ntp.server",
                        "1.openwrt.pool.ntp.org"
                    ]
                ]
            }
        }
    }
}

)foo";

static inline bool IsIPv4(const std::string &value) {
	Poco::Net::IPAddress A;
	return ((Poco::Net::IPAddress::tryParse(value, A) && A.family() == Poco::Net::IPAddress::IPv4));
}

static inline bool IsIPv6(const std::string &value) {
	Poco::Net::IPAddress A;
	return ((Poco::Net::IPAddress::tryParse(value, A) && A.family() == Poco::Net::IPAddress::IPv6));
}

static inline bool IsIP(const std::string &value) { return IsIPv4(value) || IsIPv6(value); }

static inline bool IsCIDRv6(const std::string &value) {
	auto Tokens = Poco::StringTokenizer(value, "/");
	if (Tokens.count() == 2 && IsIPv6(Tokens[0])) {
		auto Mask = std::atoi(Tokens[1].c_str());
		if (Mask >= 48 && Mask <= 128)
			return true;
	}
	return false;
}

static inline bool IsCIDRv4(const std::string &value) {
	auto Tokens = Poco::StringTokenizer(value, "/");
	if (Tokens.count() == 2 && IsIPv4(Tokens[0])) {
		auto Mask = std::atoi(Tokens[1].c_str());
		if (Mask >= 0 && Mask <= 32)
			return true;
	}
	return false;
}

static inline bool IsCIDR(const std::string &value) { return IsCIDRv4(value) || IsCIDRv6(value); }

static inline bool IsPortRangeIsValid(const std::string &r) {
	const auto ports = Poco::StringTokenizer("-", r, Poco::StringTokenizer::TOK_TRIM);

	for (const auto &port : ports) {
		uint32_t port_num = std::stoul(port);
		if (port_num == 0 || port_num > 65535)
			return false;
	}
	return true;
}

bool ExternalValijsonFormatChecker(const std::string &format, const std::string &value,
								   [[maybe_unused]] std::vector<std::string> &context,
								   [[maybe_unused]] valijson::ValidationResults *const results) {
	static const std::regex host_regex{
		"^(?=.{1,254}$)((?=[a-z0-9-]{1,63}\\.)(xn--+)?[a-z0-9]+(-[a-z0-9]+)*\\.)+[a-z]{2,63}$"};
	static const std::regex mac_regex{"^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$"};
	static const std::regex uc_timeout_regex{"^[0-9]+[dmshw]$"};
	static const std::regex b64_regex("^[a-zA-Z0-9\\+/]*={0,3}$");

	if (format == "uc-cidr4") {
		if (IsCIDRv4(value))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid CIDR IPv4 block", value));
        return false;
	} else if (format == "uc-cidr6") {
		if (IsCIDRv6(value))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid CIDR IPv6 block", value));
        return false;
	} else if (format == "uc-cidr") {
		if (IsCIDR(value))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid CIDR block", value));
        return false;
	} else if (format == "uc-mac") {
		if (std::regex_match(value, mac_regex))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid MAC address", value));
        return false;
	} else if (format == "uc-timeout") {
		if (std::regex_match(value, uc_timeout_regex))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid timeout value", value));
        return false;
	} else if (format == "uc-host") {
		if (IsIP(value))
			return true;
		if (std::regex_match(value, host_regex))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid hostname", value));
        return false;
	} else if (format == "fqdn" || format == "uc-fqdn") {
		if (std::regex_match(value, host_regex))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid FQDN", value));
        return false;
	} else if (format == "uc-base64") {
        bool valid =  value.size() % 4 == 0 && std::all_of(value.begin(), value.end(), [](char c) {
            return std::isalnum(c) || c == '+' || c == '/' || c == '=';
        });
        if (valid)
            return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid base 64 value", "..."));
        return false;
	} else if (format == "uri") {
		try {
			Poco::URI uri(value);
			return true;
		} catch (...) {
		}
		if (results)
			results->pushError(context, fmt::format("{} is not a valid URL", value));
        return false;
	} else if (format == "uc-portrange") {
		try {
			if (IsPortRangeIsValid(value))
				return true;
		} catch (...) {
		}
		if (results)
			results->pushError(context, fmt::format("{} is not a valid post range", value));
        return false;
	} else if (format == "ip") {
		if (IsIP(value))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid IP address", value));
        return false;
	}
	return true;
}

namespace OpenWifi {

	int ConfigurationValidator::Start() {
		Init();
		return 0;
	}

	void ConfigurationValidator::Stop() {}

	bool ConfigurationValidator::SetSchema(ConfigurationType Type, const std::string &SchemaStr) {
		try {
			Poco::JSON::Parser P;
			auto SchemaDocPtr = P.parse(SchemaStr).extract<Poco::JSON::Object::Ptr>();
            valijson::SchemaParser    SchemaParser;
			valijson::adapters::PocoJsonAdapter     Adaptor(SchemaDocPtr);
			SchemaParser.populateSchema(Adaptor, RootSchema_[static_cast<int>(Type)]);
			Initialized_ = Working_ = true;
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			Logger().error("Validation schema is invalid, falling back.");
		}
		return false;
	}

	std::vector<std::string>	DefaultConfigureKeys {
		"ucentral.datamodel.uri" ,
		"ucentral.datamodelols.uri"
	};

	std::vector<std::string>	DefaultJSONSchemaFileNames {
		"/ucentral.schema.json" ,
		"/ucentralols.schema.json"
	};

	void ConfigurationValidator::Init() {

        if (Initialized_)
			return;

		std::string GitSchema;
		if (MicroServiceConfigGetBool("ucentral.datamodel.internal", true)) {
			SetSchema(ConfigurationType::AP, DefaultAPSchema);
			SetSchema(ConfigurationType::SWITCH, DefaultSWITCHSchema);
			poco_information(Logger(), "Using uCentral validation from built-in default.");
			return;
		}

		try {
			for(auto Type : {ConfigurationType::AP, ConfigurationType::SWITCH}) {
				auto GitURI =
					MicroServiceConfigGetString(DefaultConfigureKeys[static_cast<int>(Type)],
												GitJSONSchemaURLs[static_cast<int>(Type)]);
				if (Utils::wgets(GitURI, GitSchema) && SetSchema(Type, GitSchema)) {
					poco_information(Logger(),
									 "Using uCentral data model validation schema from GIT.");
					return;
				} else {
					std::string FileName{MicroServiceDataDirectory() + DefaultJSONSchemaFileNames[static_cast<int>(Type)]};
					std::ifstream input(FileName);
					std::stringstream schema_file;
					schema_file << input.rdbuf();
					input.close();
					if (SetSchema(Type,schema_file.str())) {
						poco_information(
							Logger(),
							"Using uCentral data model validation schema from local file.");
						return;
					}
				}
			}
		} catch (const Poco::Exception &E) {
            Logger().log(E);
		} catch (...) {
		}
		SetSchema(ConfigurationType::AP, DefaultAPSchema);
		SetSchema(ConfigurationType::SWITCH, DefaultSWITCHSchema);
		poco_information(Logger(),
						 "Using uCentral data model validation schema from built-in default.");
	}

	bool ConfigurationValidator::Validate(ConfigurationType Type, const std::string &C, std::string &Errors,
										  bool Strict) {
		if (Working_) {
			try {
				Poco::JSON::Parser P;
				auto Doc = P.parse(C).extract<Poco::JSON::Object::Ptr>();
				valijson::adapters::PocoJsonAdapter Tester(Doc);
				valijson::Validator Validator;
				valijson::ValidationResults Results;
				if (Validator.validate(RootSchema_[static_cast<int>(Type)], Tester, &Results)) {
					return true;
				}

                Poco::JSON::Array ErrorArray;
				for (const auto &error : Results) {
                    Poco::JSON::Array   ContextArray;
                    for(const auto &context : error.context) {
                        ContextArray.add(context);
                    }
                    Poco::JSON::Object  ErrorObject;
                    ErrorObject.set("context", ContextArray);
                    ErrorObject.set("description", error.description);
                    ErrorArray.add(ErrorObject);
				}
                std::stringstream os;
                ErrorArray.stringify(os);
                Errors = os.str();
				return false;
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			} catch (const std::exception &E) {
				Logger().warning(
					fmt::format("Error wile validating a configuration (1): {}", E.what()));
			} catch (...) {
				Logger().warning("Error wile validating a configuration (2)");
			}
		}
		if (Strict)
			return false;
		return true;
	}

	void ConfigurationValidator::reinitialize([[maybe_unused]] Poco::Util::Application &self) {
		poco_information(Logger(), "Reinitializing.");
		Working_ = Initialized_ = false;
		Init();
	}

} // namespace OpenWifi
