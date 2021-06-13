//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_KAFKA_SERVICE_H
#define UCENTRALGW_KAFKA_SERVICE_H

#include <queue>

#include "SubSystemServer.h"

#include "cppkafka/cppkafka.h"

namespace uCentral {

	class KafkaManager : public SubSystemServer, Poco::Runnable {
	  public:

		struct KMessage {
			std::string Topic,
				Key,
				PayLoad;
		};

		void initialize(Poco::Util::Application & self) override;
		static KafkaManager *instance() {
			if(instance_== nullptr)
				instance_ = new KafkaManager;
			return instance_;
		}

		void run() final;
		int Start() override;
		void Stop() override;

		void PostMessage(std::string topic, std::string key, std::string payload);
		[[nodiscard]] std::string WrapSystemId(const std::string & PayLoad);

		[[nodiscard]] bool Enabled() { return Running_ && KafkaEnabled_; }

	  private:
		static KafkaManager *instance_;
		std::unique_ptr<cppkafka::Producer> 	Producer_;
		bool 					KafkaEnabled_ = false;
		std::atomic_bool 		Running_ = false;
		std::queue<KMessage>	Queue_;
		std::string 			SystemInfoWrapper_;
		Poco::Thread			Th_;

		KafkaManager() noexcept;
	};

	inline KafkaManager * KafkaManager() { return KafkaManager::instance(); }
}	// NameSpace

#endif // UCENTRALGW_KAFKA_SERVICE_H
