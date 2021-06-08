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

#include "uSubSystemServer.h"

#include "cppkafka/cppkafka.h"

namespace uCentral::Kafka {
	struct KMessage {
		std::string Topic,
					Key,
					PayLoad;
	};

	int Start();
	void Stop();
	void PostMessage(std::string topic, std::string key, std::string payload);
	[[nodiscard]] bool Enabled();

	class Service : public uSubSystemServer, Poco::Runnable {
	  public:

		Service() noexcept;

		friend int uCentral::Kafka::Start();
		friend void uCentral::Kafka::Stop();
		friend void PostMessage(std::string topic, std::string key, std::string payload);

		void initialize(Poco::Util::Application & self) override;
		static Service *instance() {
			if(instance_== nullptr)
				instance_ = new Service;
			return instance_;
		}

		void run() override;

		[[nodiscard]] bool Enabled() { return Running_ && KafkaEnabled_; }

	  private:
		static Service *instance_;
		std::unique_ptr<cppkafka::Producer> 	Producer_;
		bool 					KafkaEnabled_ = false;
		std::atomic_bool 		Running_ = false;
		std::queue<KMessage>	Queue_;
		std::string 			SystemInfoWrapper_;
		Poco::Thread			Th_;

		int Start() override;
		void Stop() override;

		void PostMessage(std::string topic, std::string key, std::string payload);
		[[nodiscard]] std::string WrapSystemId(const std::string & PayLoad);
	};

}	// NameSpace

#endif // UCENTRALGW_KAFKA_SERVICE_H
