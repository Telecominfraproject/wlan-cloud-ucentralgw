//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#include <fstream>

#include "CentralConfig.h"
#include "Daemon.h"

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/File.h"

namespace OpenWifi::Config {

	static std::string DefaultConfiguration;

	static std::string BasicConfig {
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
})lit"};

	void SetBasicConfigFile() {
		Poco::File DefaultConfigFileName{Daemon()->DataDir()+"/default_config.json"};
		DefaultConfiguration = BasicConfig;
		std::ofstream F;
		F.open(DefaultConfigFileName.path(),std::ios::binary);
		F << BasicConfig ;
		F.close();
	}

	Config::Config() {
		Config_ = DefaultConfiguration;
	}

	void Config::Init() {
		if(DefaultConfiguration.empty()) {
			//	open the file
			Poco::File DefaultConfigFileName{Daemon()->DataDir()+"/default_config.json"};
			try {
				if (!DefaultConfigFileName.exists()) {
					SetBasicConfigFile();
				} else {
					std::ifstream F;
					F.open(DefaultConfigFileName.path(),std::ios::binary);
					while(!F.eof()) {
						std::string Line;
						F >> Line;
						DefaultConfiguration += Line + "\n" ;
					}
					F.close();
					// std::cout << "Default config: " << DefaultConfiguration << std::endl;
				}
			} catch (...) {
				SetBasicConfigFile();
			}
		}
	}

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

	std::string Config::Default() { return DefaultConfiguration; }

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
            Daemon::instance()->logger().warning(Poco::format("%s: Failed with: %s", std::string(__func__) , E.displayText()));
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
