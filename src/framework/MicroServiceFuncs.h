//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>

#include "framework/OpenWifiTypes.h"

#include "Poco/JSON/Object.h"
#include "Poco/JWT/Token.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/ThreadPool.h"

namespace OpenWifi {
	class SubSystemServer;
	using SubSystemVec = std::vector<SubSystemServer *>;
	const std::string &MicroServiceDataDirectory();
	Types::MicroServiceMetaVec MicroServiceGetServices(const std::string &Type);
	Types::MicroServiceMetaVec MicroServiceGetServices();
	std::string MicroServicePublicEndPoint();
	std::string MicroServiceConfigGetString(const std::string &Key,
											const std::string &DefaultValue);
	std::string MicroServiceAccessKey();
	bool MicroServiceConfigGetBool(const std::string &Key, bool DefaultValue);
	std::uint64_t MicroServiceConfigGetInt(const std::string &Key, std::uint64_t DefaultValue);
	std::string MicroServicePrivateEndPoint();
	std::uint64_t MicroServiceID();
	bool MicroServiceIsValidAPIKEY(const Poco::Net::HTTPServerRequest &Request);
	bool MicroServiceNoAPISecurity();
	void MicroServiceLoadConfigurationFile();
	void MicroServiceReload();
	void MicroServiceReload(const std::string &Type);
	Types::StringVec MicroServiceGetLogLevelNames();
	Types::StringVec MicroServiceGetSubSystems();
	Types::StringPairVec MicroServiceGetLogLevels();
	bool MicroServiceSetSubsystemLogLevel(const std::string &SubSystem, const std::string &Level);
	void MicroServiceGetExtraConfiguration(Poco::JSON::Object &Answer);
	std::string MicroServiceVersion();
	std::uint64_t MicroServiceUptimeTotalSeconds();
	std::uint64_t MicroServiceStartTimeEpochTime();
	std::string MicroServiceGetUIURI();
	SubSystemVec MicroServiceGetFullSubSystems();
	std::string MicroServiceCreateUUID();
	std::uint64_t MicroServiceDaemonBusTimer();
	std::string MicroServiceMakeSystemEventMessage(const char *Type);
	Poco::ThreadPool &MicroServiceTimerPool();
	std::string MicroServiceConfigPath(const std::string &Key, const std::string &DefaultValue);
	std::string MicroServiceWWWAssetsDir();
	std::uint64_t MicroServiceRandom(std::uint64_t Start, std::uint64_t End);
	std::uint64_t MicroServiceRandom(std::uint64_t Range);
	std::string MicroServiceSign(Poco::JWT::Token &T, const std::string &Algo);
	std::string MicroServiceGetPublicAPIEndPoint();
	void MicroServiceDeleteOverrideConfiguration();
	bool AllowExternalMicroServices();
	void MicroServiceALBCallback( std::string Callback());
} // namespace OpenWifi
