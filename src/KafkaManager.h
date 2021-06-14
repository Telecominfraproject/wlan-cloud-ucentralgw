//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_KAFKAMANAGER_H
#define UCENTRALGW_KAFKAMANAGER_H

#include <queue>
#include <thread>

#include "SubSystemServer.h"
#include "uCentralTypes.h"

#include "cppkafka/cppkafka.h"

namespace uCentral {

	class KafkaManager : public SubSystemServer {
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

		static void Producer(KafkaManager *);
		static void Consumer(KafkaManager *);

		int Start() override;
		void Stop() override;

		void PostMessage(std::string topic, std::string key, std::string payload);
		[[nodiscard]] std::string WrapSystemId(const std::string & PayLoad);
		[[nodiscard]] bool Enabled() { return Running_ && KafkaEnabled_; }
		int RegisterTopicWatcher(const std::string &Topic, Types::TopicNotifyFunction & F);
		void UnregisterTopicWatcher(const std::string &Topic, int FunctionId);

	  private:
		static KafkaManager *instance_;
		SubMutex 				ProducerMutex_;
		SubMutex 				ConsumerMutex_;
		bool 					KafkaEnabled_ = false;
		std::atomic_bool 		Running_ = false;
		std::queue<KMessage>	Queue_;
		std::string 			SystemInfoWrapper_;
		std::unique_ptr<std::thread>	ConsumerThr_;
		std::unique_ptr<std::thread>	ProducerThr_;
		int                       FunctionId_=1;
		Types::NotifyTable        Notifiers_;
		std::unique_ptr<cppkafka::Configuration>    Config_;

		KafkaManager() noexcept;
	};

	inline KafkaManager * KafkaManager() { return KafkaManager::instance(); }
}	// NameSpace

#endif // UCENTRALGW_KAFKAMANAGER_H
