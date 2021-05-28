//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "uCentral.h"
#include "uCentralConfig.h"

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"

namespace uCentral::Config {

    bool Config::SetUUID(uint64_t UUID) {
        try {
            Poco::JSON::Parser Parser;
            Poco::JSON::Object::Ptr Object = Parser.parse(Config_).extract<Poco::JSON::Object::Ptr>();
			Object->set("uuid",UUID);
			std::ostringstream NewConfig;
            Poco::JSON::Stringifier Stringifier;
			Stringifier.condense(Object, NewConfig);
            Config_ = NewConfig.str();
            return true;
        }
        catch(const Poco::Exception &E)
        {
            std::cout << __func__ << ": new Configuration failed with " << E.displayText() << std::endl;
        }
        return false;
    }

    bool Config::Valid() {
        try {
            Poco::JSON::Parser Parser;
            auto object = Parser.parse(Config_).extract<Poco::JSON::Object::Ptr>();
            if(object->has("uuid"))
                return true;
            return false;
        }
        catch (const Poco::Exception &E)
        {
            return false;
        }
    }

	Poco::JSON::Object::Ptr Config::to_json() {
        Poco::JSON::Parser Parser;
        return Parser.parse(Config_).extract<Poco::JSON::Object::Ptr>();
    }

	std::string Config::Default() {
		return std::string(
			R"lit({
	"uuid": 1,
	"radios": [
		{
			"band": "5G",
			"country": "CA",
			"channel-mode": "HE",
			"channel-width": 80,
			"channel": 32
		}
	],

	"interfaces": [
		{
			"name": "WAN",
			"role": "upstream",
			"services": [ "lldp" ],
			"ethernet": [
				{
					"select-ports": [
						"WAN*"
					]
				}
			],
			"ipv4": {
				"addressing": "dynamic"
			},
			"ssids": [
				{
					"name": "OpenWifi",
					"wifi-bands": [
						"5G"
					],
					"bss-mode": "ap",
					"encryption": {
						"proto": "psk2",
						"key": "OpenWifi",
						"ieee80211w": "optional"
					}
				}
			]
		},
		{
			"name": "LAN",
			"role": "downstream",
			"services": [ "ssh", "lldp" ],
			"ethernet": [
				{
					"select-ports": [
						"LAN*"
					]
				}
			],
			"ipv4": {
				"addressing": "static",
				"subnet": "192.168.1.1/24",
				"dhcp": {
					"lease-first": 10,
					"lease-count": 100,
					"lease-time": "6h"
				}
			},
			"ssids": [
				{
					"name": "OpenWifi",
					"wifi-bands": [
						"5G"
					],
					"bss-mode": "ap",
					"encryption": {
						"proto": "psk2",
						"key": "OpenWifi",
						"ieee80211w": "optional"
					}
				}
			]

		}
	],
	"metrics": {
		"statistics": {
			"interval": 120,
			"types": [ "ssids", "lldp", "clients" ]
		},
		"health": {
			"interval": 120
		}
	},
	"services": {
		"lldp": {
			"describe": "uCentral",
			"location": "universe"
		},
		"ssh": {
			"port": 22
		}
	}
})lit");
	}

    std::string Config::Default_Old() {
        return std::string(
                R"lit({"uuid":1613927736,"steer":{"enabled":1,"network":"wan","debug_level":0},"stats":
                {"interval":60,"neighbours":1,"traffic":1,"wifiiface":1,"wifistation":1,"pids":1,
                "serviceprobe":1,"lldp":1,"system":1,"poe":1},"phy":[{"band":"2","cfg":{"disabled":0
                ,"country":"DE","channel":6,"txpower":30,"beacon_int":100,"htmode":"HE40","hwmode"
                :"11g","chanbw":20}},{"band":"5","cfg":{"mimo":"4x4","disabled":0,"country":"DE",
                "channel":0,"htmode":"HE80"}},{"band":"5u","cfg":{"disabled":0,"country":"DE",
                "channel":100,"htmode":"VHT80"}},{"band":"5l","cfg":{"disabled":0,"country":"DE","channel"
                :36,"htmode":"VHT80"}}],"ssid":[{"band":["2"],"cfg":{"ssid":"uCentral-Guest","encryption"
                :"psk2","key":"OpenWifi","mode":"ap","isolate":1,"network":"guest","ieee80211r":1,
                "ieee80211v":1,"ieee80211k":1,"ft_psk_generate_local":1,"ft_over_ds":1,"mobility_domain":"4f57"}
                },{"band":["5l","5"],"cfg":{"ssid":"uCentral-NAT.200","encryption":"psk2","key":
                "OpenWifi","mode":"ap","network":"nat200","ieee80211r":1,"ieee80211v":1,"ieee80211k":1,
                "ft_psk_generate_local":1,"ft_over_ds":1,"mobility_domain":"4f51"}},{"band":["5l","5"],"cfg"
                :{"ssid":"uCentral-EAP","encryption":"wpa2","server":"148.251.188.218","port":1812,
                "auth_secret":"uSyncRad1u5","mode":"ap","network":"lan","ieee80211r":1,"ieee80211v":1,
                "ieee80211k":1,"ft_psk_generate_local":1,"ft_over_ds":1,"mobility_domain":"4f51"}},
                {"band":["5l","5"],"cfg":{"ssid":"uCentral","encryption":"psk2","key":"OpenWifi",
                "mode":"ap","network":"wan","ieee80211r":1,"ieee80211v":1,"ieee80211k":1,
                "ft_psk_generate_local":1,"ft_over_ds":1,"mobility_domain":"4f51"}}],"network":
                [{"mode":"wan","cfg":{"proto":"dhcp"}},{"mode":"gre","cfg":{"vid":"50"
                ,"peeraddr":"50.210.104.108"}},{"mode":"nat","vlan":200,"cfg":{"proto":"static"
                ,"ipaddr":"192.168.16.1","netmask":"255.255.255.0","mtu":1500,"ip6assign":60,"dhcp":
                {"start":10,"limit":100,"leasetime":"6h"},"leases":[{"ip":"192.168.100.2","mac":
                "00:11:22:33:44:55","hostname":"test"},{"ip":"192.168.100.3","mac":"00:11:22:33:44:56",
                "hostname":"test2"}]}},{"mode":"guest","cfg":{"proto":"static","ipaddr":"192.168.12.11",
                "dhcp":{"start":10,"limit":100,"leasetime":"6h"}}}],"ntp":{"enabled":1,"enable_server":1,
                "server":["0.openwrt.pool.ntp.org","1.openwrt.pool.ntp.org"]},"ssh":{"enable":1,"Port":22},
                "system":{"timezone":"CET-1CEST,M3.5.0,M10.5.0/3"},"log":{"_log_proto":"udp","_log_ip":
                "192.168.11.23","_log_port":12345,"_log_hostname":"foo","_log_size":128},"rtty":{"host":
                "websocket.usync.org","token":"7049cb6b7949ba06c6b356d76f0f6275","interface":"wan"}})lit");
    }

    std::string Capabilities::Default() {
        return std::string(R"lit({"model":{"id":"linksys,ea8300","name":"Linksys EA8300 (Dallas)"},
                        "network":{"lan":{"ifname":"eth0","protocol":"static"},"wan":{"ifname":"eth1","protocol":"dhcp"}},
                        "switch":{"switch0":{"enable":true,"reset":true,"ports":[{"num":0,"device":"eth0","need_tag":false,
                        "want_untag":true},{"num":1,"role":"lan"},{"num":2,"role":"lan"},{"num":3,"role":"lan"},{"num":4,"role":"lan"}],
                        "roles":[{"role":"lan","ports":"1 2 3 4 0","device":"eth0"}]}},
                        "wifi":{"soc/40000000.pci/pci0000:00/0000:00:00.0/0000:01:00.0":{"band":["5u"],"ht_capa":6639,
                        "vht_capa":865696178,"htmode":["HT20","HT40","VHT20","VHT40","VHT80"],"tx_ant":3,"rx_ant":3,
                        "channels":[100,104,108,112,116,120,124,128,132,136,140,144,149,153,157,161,165]},
                        "platform/soc/a000000.wifi":{"band":["2"],"ht_capa":6639,"vht_capa":865687986,
                        "htmode":["HT20","HT40","VHT20","VHT40","VHT80"],"tx_ant":3,"rx_ant":3,"channels":[1,2,3,4,5,6,7,8,9,10,11]},
                        "platform/soc/a800000.wifi":{"band":["5l"],"ht_capa":6639,"vht_capa":865687986,"htmode":["HT20","HT40","VHT20","VHT40","VHT80"],
                        "tx_ant":3,"rx_ant":3,"channels":[36,40,44,48,52,56,60,64]}}})lit");
    }

    void Capabilities::Parse() {
        if(Capabilities_.empty())
            Capabilities_=Default();

        try {
            Poco::JSON::Parser parser;

            auto result = parser.parse(Capabilities_);
            auto object = result.extract<Poco::JSON::Object::Ptr>();
            Poco::DynamicStruct ds = *object;

            auto Model = ds["model"];
			std::string Compatible{"unknown"};
			if(ds.contains("compatible"))
            	Compatible = Model["compatible"].toString();
            auto Name = Model["name"].toString();

            Manufacturer_ = Name;
            DeviceType_ = uCentral::instance()->IdentifyDevice(Compatible);
            ModelId_ = Compatible;
            Parsed_ = true ;
        }
        catch ( const Poco::Exception & E )
        {
            uCentral::Daemon::instance()->logger().warning(Poco::format("%s: Failed with: %s", std::string(__func__) , E.displayText()));
        }
    }

    const std::string & Capabilities::DeviceType() {
        if(!Parsed_)
            Parse();
        return DeviceType_;
    }

    const std::string & Capabilities::Manufacturer() {
        if(!Parsed_)
            Parse();
        return Manufacturer_;
    }

    const std::string & Capabilities::ModelId() {
        if(!Parsed_)
            Parse();
        return ModelId_;
    }

} // namespace
