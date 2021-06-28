//
// Created by stephane bourque on 2021-06-07.
//

#ifndef UCENTRALGW_KAFKA_TOPICS_H
#define UCENTRALGW_KAFKA_TOPICS_H

namespace uCentral::KafkaTopics {
	static const std::string HEALTHCHECK{"healthcheck"};
	static const std::string STATE{"state"};
	static const std::string CONNECTION{"connection"};
	static const std::string WIFISCAN{"wifiscan"};
	static const std::string ALERTS{"alerts"};
	static const std::string COMMAND{"command"};
	static const std::string SERVICE_EVENTS{"service_events"};
}
#endif // UCENTRALGW_KAFKA_TOPICS_H
