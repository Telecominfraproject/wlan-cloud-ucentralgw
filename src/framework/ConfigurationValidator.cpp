//
// Created by stephane bourque on 2021-09-14.
//

#include <fstream>
#include <iostream>
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

static const std::string GitUCentralJSONSchemaFile{
	"https://raw.githubusercontent.com/Telecominfraproject/wlan-ucentral-schema/main/"
	"ucentral.schema.json"};

static std::string DefaultUCentralSchema = R"foo(

{
    "$id": "https://openwrt.org/ucentral.schema.json",
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "properties": {
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
                        10000
                    ]
                },
                "duplex": {
                    "type": "string",
                    "enum": [
                        "half",
                        "full"
                    ]
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
                                "rstp"
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
                        "HE"
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
                        "\\{Interface\\}:\\{VLAN-Id\\}:\\{SSID\\}:\\{Model\\}:\\{Name\\}:\\{AP-MAC\\}:\\{Location\\}",
                        "\\{AP-MAC\\};\\{SSID\\};\\{Crypto\\}",
                        "\\{Name\\} \\{ESSID\\}"
                    ]
                },
                "remote-id-format": {
                    "type": "string",
                    "example": [
                        "\\{Client-MAC-hex\\} \\{SSID\\}",
                        "\\{AP-MAC-hex\\} \\{SSID\\}"
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
                        "static"
                    ],
                    "examples": [
                        "static"
                    ]
                },
                "subnet": {
                    "type": "string",
                    "format": "uc-cidr4",
                    "examples": [
                        "auto/24"
                    ]
                },
                "gateway": {
                    "type": "string",
                    "format": "ipv4",
                    "examples": [
                        "192.168.1.1"
                    ]
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
                    "$ref": "#/$defs/interface.ssid.roaming"
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
                "ca-certificate": {
                    "type": "string"
                },
                "use-local-certificates": {
                    "type": "boolean",
                    "default": false
                },
                "server-certificate": {
                    "type": "string"
                },
                "private-key": {
                    "type": "string"
                },
                "users": {
                    "type": "array",
                    "items": {
                        "$ref": "#/$defs/interface.ssid.radius.local-user"
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
                            "wifi.stop"
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
                            "wifi.stop"
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
	} else if (format == "uc-cidr6") {
		if (IsCIDRv6(value))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid CIDR IPv6 block", value));
	} else if (format == "uc-cidr") {
		if (IsCIDR(value))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid CIDR block", value));
	} else if (format == "uc-mac") {
		if (std::regex_match(value, mac_regex))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid MAC address", value));
	} else if (format == "uc-timeout") {
		if (std::regex_match(value, uc_timeout_regex))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid timeout value", value));
	} else if (format == "uc-host") {
		if (IsIP(value))
			return true;
		if (std::regex_match(value, host_regex))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid hostname", value));
	} else if (format == "fqdn" || format == "uc-fqdn") {
		if (std::regex_match(value, host_regex))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid FQDN", value));
	} else if (format == "uc-base64") {
		std::string s{value};
		Poco::trimInPlace(s);
		if ((s.size() % 4 == 0) && std::regex_match(s, b64_regex))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid base 64 value", value));
	} else if (format == "uri") {
		try {
			Poco::URI uri(value);
			return true;
		} catch (...) {
		}
		if (results)
			results->pushError(context, fmt::format("{} is not a valid URL", value));
	} else if (format == "uc-portrange") {
		try {
			if (IsPortRangeIsValid(value))
				return true;
		} catch (...) {
		}
		if (results)
			results->pushError(context, fmt::format("{} is not a valid post range", value));
	} else if (format == "ip") {
		if (IsIP(value))
			return true;
		if (results)
			results->pushError(context, fmt::format("{} is not a valid IP address", value));
	}
	return true;
}

namespace OpenWifi {

	int ConfigurationValidator::Start() {
		Init();
		return 0;
	}

	void ConfigurationValidator::Stop() {}

	bool ConfigurationValidator::SetSchema(const std::string &SchemaStr) {
		try {
			Poco::JSON::Parser P;
			SchemaDocPtr_ = P.parse(SchemaStr).extract<Poco::JSON::Object::Ptr>();
			RootSchema_ = std::make_unique<valijson::Schema>();
			SchemaParser_ = std::make_unique<valijson::SchemaParser>();
			PocoJsonAdapter_ = std::make_unique<valijson::adapters::PocoJsonAdapter>(SchemaDocPtr_);
			SchemaParser_->populateSchema(*PocoJsonAdapter_, *RootSchema_);
			Initialized_ = Working_ = true;
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			Logger().error("Validation schema is invalid, falling back.");
		}
		return false;
	}

	void ConfigurationValidator::Init() {
		if (Initialized_)
			return;

		std::string GitSchema;
		if (MicroServiceConfigGetBool("ucentral.datamodel.internal", true)) {
			SetSchema(DefaultUCentralSchema);
			poco_information(Logger(), "Using uCentral validation from built-in default.");
			return;
		}

		try {
			auto GitURI =
				MicroServiceConfigGetString("ucentral.datamodel.uri", GitUCentralJSONSchemaFile);
			if (Utils::wgets(GitURI, GitSchema) && SetSchema(GitSchema)) {
				poco_information(Logger(), "Using uCentral data model validation schema from GIT.");
				return;
			} else {
				std::string FileName{MicroServiceDataDirectory() + "/ucentral.schema.json"};
				std::ifstream input(FileName);
				std::stringstream schema_file;
				schema_file << input.rdbuf();
				input.close();
				if (SetSchema(schema_file.str())) {
					poco_information(
						Logger(), "Using uCentral data model validation schema from local file.");
					return;
				}
			}
		} catch (const Poco::Exception &E) {

		} catch (...) {
		}
		SetSchema(DefaultUCentralSchema);
		poco_information(Logger(),
						 "Using uCentral data model validation schema from built-in default.");
	}

	bool ConfigurationValidator::Validate(const std::string &C, std::vector<std::string> &Errors,
										  bool Strict) {
		if (Working_) {
			try {
				Poco::JSON::Parser P;
				auto Doc = P.parse(C).extract<Poco::JSON::Object::Ptr>();
				valijson::adapters::PocoJsonAdapter Tester(Doc);
				valijson::Validator Validator;
				valijson::ValidationResults Results;
				if (Validator.validate(*RootSchema_, Tester, &Results)) {
					return true;
				}
				for (const auto &error : Results) {
					Errors.push_back(error.description);
				}
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
