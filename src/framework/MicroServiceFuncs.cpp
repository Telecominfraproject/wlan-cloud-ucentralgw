//
// Created by stephane bourque on 2022-10-25.
//

#include "framework/MicroService.h"
#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {
    const std::string &MicroServiceDataDirectory() { return MicroService::instance().DataDir(); }

    Types::MicroServiceMetaVec MicroServiceGetServices(const std::string &Type) {
        return MicroService::instance().GetServices(Type);
    }

    Types::MicroServiceMetaVec MicroServiceGetServices() {
        return MicroService::instance().GetServices();
    }

    std::string MicroServicePublicEndPoint() { return MicroService::instance().PublicEndPoint(); }

    std::string MicroServiceConfigGetString(const std::string &Key, const std::string &DefaultValue) {
        return MicroService::instance().ConfigGetString(Key, DefaultValue);
    }

    bool MicroServiceConfigGetBool(const std::string &Key, bool DefaultValue) {
        return MicroService::instance().ConfigGetBool(Key, DefaultValue);
    }

    std::uint64_t MicroServiceConfigGetInt(const std::string &Key, std::uint64_t DefaultValue) {
        return MicroService::instance().ConfigGetInt(Key, DefaultValue);
    }

    std::string MicroServicePrivateEndPoint() { return MicroService::instance().PrivateEndPoint(); }

    std::uint64_t MicroServiceID() { return MicroService::instance().ID(); }

    bool MicroServiceIsValidAPIKEY(const Poco::Net::HTTPServerRequest &Request) {
        return MicroService::instance().IsValidAPIKEY(Request);
    }

    bool MicroServiceNoAPISecurity() { return MicroService::instance().NoAPISecurity(); }

    void MicroServiceLoadConfigurationFile() { MicroService::instance().LoadConfigurationFile(); }

    void MicroServiceReload() { MicroService::instance().Reload(); }

    void MicroServiceReload(const std::string &Type) { MicroService::instance().Reload(Type); }

    const Types::StringVec MicroServiceGetLogLevelNames() {
        return MicroService::instance().GetLogLevelNames();
    }

    const Types::StringVec MicroServiceGetSubSystems() {
        return MicroService::instance().GetSubSystems();
    }

    Types::StringPairVec MicroServiceGetLogLevels() { return MicroService::instance().GetLogLevels(); }

    bool MicroServiceSetSubsystemLogLevel(const std::string &SubSystem, const std::string &Level) {
        return MicroService::instance().SetSubsystemLogLevel(SubSystem, Level);
    }

    void MicroServiceGetExtraConfiguration(Poco::JSON::Object &Answer) {
        MicroService::instance().GetExtraConfiguration(Answer);
    }

    std::string MicroServiceVersion() { return MicroService::instance().Version(); }

    std::uint64_t MicroServiceUptimeTotalSeconds() {
        return MicroService::instance().uptime().totalSeconds();
    }

    std::uint64_t MicroServiceStartTimeEpochTime() {
        return MicroService::instance().startTime().epochTime();
    }

    std::string MicroServiceGetUIURI() { return MicroService::instance().GetUIURI(); }

    const SubSystemVec MicroServiceGetFullSubSystems() {
        return MicroService::instance().GetFullSubSystems();
    }

    std::string MicroServiceCreateUUID() { return MicroService::CreateUUID(); }

    std::uint64_t MicroServiceDaemonBusTimer() { return MicroService::instance().DaemonBusTimer(); }

    std::string MicroServiceMakeSystemEventMessage(const std::string &Type) {
        return MicroService::instance().MakeSystemEventMessage(Type);
    }

    Poco::ThreadPool &MicroServiceTimerPool() { return MicroService::instance().TimerPool(); }

    std::string MicroServiceConfigPath(const std::string &Key,
                                       const std::string &DefaultValue) {
        return MicroService::instance().ConfigPath(Key, DefaultValue);
    }

    std::string MicroServiceWWWAssetsDir() {
        return MicroService::instance().WWWAssetsDir();
    }

    std::uint64_t MicroServiceRandom(std::uint64_t Start,std::uint64_t End) {
        return MicroService::instance().Random(Start, End);
    }

    std::uint64_t MicroServiceRandom(std::uint64_t Range) {
        return MicroService::instance().Random(Range);
    }

    std::string MicroServiceSign(Poco::JWT::Token &T, const std::string &Algo) {
        return MicroService::instance().Sign(T, Algo);
    }

    std::string MicroServiceGetPublicAPIEndPoint() {
        return MicroService::instance().GetPublicAPIEndPoint();
    }

	void MicroServiceDeleteOverrideConfiguration() {
		return MicroService::instance().DeleteOverrideConfiguration();
	}

    bool AllowExternalMicroServices() {
        return MicroService::instance().AllowExternalMicroServices();
    }

}
