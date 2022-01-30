//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#include <fstream>

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/File.h"

#include "CentralConfig.h"
#include "framework/MicroService.h"
#include "Daemon.h"

namespace OpenWifi::Config {

	const static std::string BasicConfig {
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

			std::cout << "Config: " << Config_ << std::endl;

            auto Object = Parser.parse(Config_).extract<Poco::JSON::Object::Ptr>();

			std::cout << "Config: " << Config_ << std::endl;

			Object->set("uuid", (uint32_t) UUID);

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
