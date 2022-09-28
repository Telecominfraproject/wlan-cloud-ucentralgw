//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#include <fstream>

#include "framework/MicroService.h"

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/File.h"

#include "CentralConfig.h"
#include "Daemon.h"

namespace OpenWifi::Config {

	const static std::string BasicConfig {
R"lit(
{
  "interfaces": [
    {
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
      "name": "WAN",
      "role": "upstream",
      "services": [
        "ssh",
        "lldp",
        "dhcp-snooping"
      ],
      "ssids": [
        {
          "bss-mode": "ap",
          "encryption": {
            "ieee80211w": "optional",
            "key": "OpenWifi1",
            "proto": "psk2"
          },
          "name": "OpenWifi",
          "services": [
            "wifi-frames"
          ],
          "wifi-bands": [
           "2G","5G"
          ]
        }
      ]
    },
    {
      "ethernet": [
        {
          "select-ports": [
            "LAN*"
          ]
        }
      ],
      "ipv4": {
        "addressing": "static",
        "dhcp": {
          "lease-count": 10000,
          "lease-first": 10,
          "lease-time": "6h"
        },
        "subnet": "192.168.1.1/16"
      },
      "name": "LAN",
      "role": "downstream",
      "services": [
        "ssh",
        "lldp",
        "dhcp-snooping"
      ]
    }
  ],
  "metrics": {
    "dhcp-snooping": {
      "filters": [
        "ack",
        "discover",
        "offer",
        "request",
        "solicit",
        "reply",
        "renew"
      ]
    },
    "health": {
      "interval": 120
    },
    "statistics": {
      "interval": 60,
      "types": [
        "ssids",
        "lldp",
        "clients"
      ]
    },
    "wifi-frames": {
      "filters": [
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
  },
  "radios": [
    {
      "band": "2G",
      "channel": "auto",
      "channel-mode": "HE",
      "country": "CA"
    },
    {
      "allow-dfs": true,
      "band": "5G",
      "channel": "auto",
      "channel-mode": "HE",
      "country": "CA"
    }
  ],
  "services": {
    "lldp": {
      "describe": "TIP OpenWiFi",
      "location": "QA"
    },
    "ssh": {
      "port": 22
    }
  },
  "uuid": 2
}
)lit"};

	void Config::SetBasicConfigFile() {
		try {
			Poco::File DefaultConfigFileName{MicroService::instance().DataDir() + "/default_config.json"};
			DefaultConfiguration_ = BasicConfig;
			std::ofstream OS(DefaultConfigFileName.path(), std::ios::binary | std::ios::trunc );
			std::istringstream	IS(DefaultConfiguration_);
			Poco::StreamCopier::copyStream(IS, OS);
		} catch (...) {
			DefaultConfiguration_ = BasicConfig;
		}
	}

	Config::Config() {
		if(DefaultConfiguration_.empty())
			Init();
		Config_ = DefaultConfiguration_;
	}

	void Config::Init() {
		if(DefaultConfiguration_.empty()) {
			//	open the file
			try {
				Poco::File DefaultConfigFileName{MicroService::instance().DataDir()+"/default_config.json"};
				if (!DefaultConfigFileName.exists()) {
					SetBasicConfigFile();
				} else {
					std::ifstream F(DefaultConfigFileName.path(),std::ios::binary | std::ios::in);
					std::ostringstream C;
					Poco::StreamCopier::copyStream(F,C);
					DefaultConfiguration_ = C.str();
				}
			} catch (...) {
				SetBasicConfigFile();
			}
		}
	}

    bool Config::SetUUID(uint64_t UUID) {
        try {
            Poco::JSON::Parser Parser;
            auto Object = Parser.parse(Config_).extract<Poco::JSON::Object::Ptr>();
			Object->set("uuid", UUID);
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
        catch (...)
        {
			return false;
        }
    }

	Poco::JSON::Object::Ptr Config::to_json() {
        Poco::JSON::Parser Parser;
        return Parser.parse(Config_).extract<Poco::JSON::Object::Ptr>();
    }

	std::string Config::Default() {
		if(DefaultConfiguration_.empty())
			Init();
		return DefaultConfiguration_;
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

            auto Result = parser.parse(Capabilities_);
            auto Objects = Result.extract<Poco::JSON::Object::Ptr>();

			if(Objects->has("compatible"))
				Compatible_ = Objects->get("compatible").toString();

			if(Objects->has("model"))
				Model_ = Objects->get("model").toString();

			if(Objects->has("platform"))
				Platform_ = Objects->get("platform").toString();

            Parsed_ = true ;
        }
        catch ( const Poco::Exception & E )
        {
            Daemon()->logger().log(E);
        }
    }

	const std::string & Capabilities::Compatible() {
		if(!Parsed_)
			Parse();
		return Compatible_;
	}

	const std::string & Capabilities::Model() {
		if(!Parsed_)
			Parse();
		return Model_;
	}

	const std::string & Capabilities::Platform() {
		if(!Parsed_)
			Parse();
		return Platform_;
	}

} // namespace
