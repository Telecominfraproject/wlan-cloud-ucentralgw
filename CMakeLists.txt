cmake_minimum_required(VERSION 3.13)
project(owgw VERSION 3.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(UNIX AND APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    set(MYSQL_ROOT_DIR /usr/local/opt/mysql-client)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

if(UNIX AND NOT APPLE)
    set(PostgreSQL_TYPE_INCLUDE_DIR /usr/include/postgresql)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

if(SMALL_BUILD)
    add_definitions(-DSMALL_BUILD)
endif()

# Auto build increment. You must define BUILD_INCREMENT with cmake -DBUILD_INCREMENT=1
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/build)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/build BUILD_NUM)
    if(BUILD_INCREMENT)
        MATH(EXPR BUILD_NUM "${BUILD_NUM}+1")
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/build ${BUILD_NUM})
    endif()
else()
    set(BUILD_NUM 1)
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/build ${BUILD_NUM})
endif()

if(ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_RESULT
            OUTPUT_VARIABLE GIT_HASH)
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "git rev-parse --short HEAD failed with ${GIT_RESULT}")
    endif()
    string(REGEX REPLACE "\n$" "" GIT_HASH "${GIT_HASH}")
endif()

add_definitions(-DTIP_GATEWAY_SERVICE="1" -DPOCO_LOG_DEBUG="1" -DBOOST_NO_CXX98_FUNCTION_BASE=1)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(fmt  REQUIRED)
find_package(nlohmann_json  REQUIRED)
# find_package(valijson       REQUIRED)

if(SMALL_BUILD)
    find_package(Poco REQUIRED COMPONENTS Crypto JWT Net Util NetSSL Data DataSQLite)
else()
    find_package(CppKafka REQUIRED)
    find_package(PostgreSQL REQUIRED)
    find_package(MySQL REQUIRED)
    find_package(Poco REQUIRED COMPONENTS JSON Crypto JWT Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL)
endif()

include_directories(/usr/local/include  /usr/local/opt/openssl/include src include/kafka /usr/local/opt/mysql-client/include)

configure_file(src/ow_version.h.in ${PROJECT_SOURCE_DIR}/src/ow_version.h @ONLY)

add_compile_options(-Wall -Wextra)

add_executable( owgw
        build
        src/ow_version.h.in
        src/framework/CountryCodes.h
        src/framework/KafkaTopics.h
        src/framework/MicroService.h
        src/framework/OpenWifiTypes.h
        src/framework/orm.h
        src/framework/StorageClass.h
        src/framework/MicroServiceErrorHandler.h
        src/framework/UI_WebSocketClientServer.cpp
        src/framework/UI_WebSocketClientServer.h
        src/framework/UI_WebSocketClientNotifications.cpp
        src/framework/UI_WebSocketClientNotifications.h
        src/framework/utils.h
        src/framework/utils.cpp
        src/framework/AppServiceRegistry.h
        src/framework/SubSystemServer.cpp
        src/framework/SubSystemServer.h
        src/framework/RESTAPI_utils.h
        src/framework/AuthClient.cpp
        src/framework/AuthClient.h
        src/framework/MicroServiceNames.h
        src/framework/MicroServiceFuncs.h
        src/framework/OpenAPIRequests.cpp
        src/framework/OpenAPIRequests.h
        src/framework/MicroServiceFuncs.cpp
        src/framework/ALBserver.cpp
        src/framework/ALBserver.h
        src/framework/KafkaManager.cpp
        src/framework/KafkaManager.h
        src/framework/RESTAPI_RateLimiter.h
        src/framework/WebSocketLogger.h
        src/framework/RESTAPI_GenericServerAccounting.h
        src/framework/CIDR.h
        src/framework/RESTAPI_Handler.cpp
        src/framework/RESTAPI_Handler.h
        src/framework/RESTAPI_ExtServer.h
        src/framework/RESTAPI_ExtServer.cpp
        src/framework/RESTAPI_IntServer.cpp
        src/framework/RESTAPI_IntServer.h
        src/framework/RESTAPI_SystemCommand.h
        src/framework/RESTAPI_WebSocketServer.h
        src/framework/RESTAPI_SystemConfiguration.h
        src/framework/EventBusManager.cpp
        src/framework/EventBusManager.h
        src/framework/RESTAPI_PartHandler.h
        src/framework/MicroService.cpp
        src/framework/MicroServiceExtra.h
        src/RESTObjects/RESTAPI_SecurityObjects.h src/RESTObjects/RESTAPI_SecurityObjects.cpp
        src/RESTObjects/RESTAPI_GWobjects.h src/RESTObjects/RESTAPI_GWobjects.cpp
        src/RESTObjects/RESTAPI_FMSObjects.h src/RESTObjects/RESTAPI_FMSObjects.cpp
        src/RESTObjects/RESTAPI_CertObjects.cpp src/RESTObjects/RESTAPI_CertObjects.h
        src/RESTObjects/RESTAPI_OWLSobjects.cpp src/RESTObjects/RESTAPI_OWLSobjects.h
        src/RESTObjects/RESTAPI_ProvObjects.cpp src/RESTObjects/RESTAPI_ProvObjects.h
        src/RESTObjects/RESTAPI_AnalyticsObjects.cpp src/RESTObjects/RESTAPI_AnalyticsObjects.h
        src/RESTObjects/RESTAPI_SubObjects.cpp src/RESTObjects/RESTAPI_SubObjects.h
        src/RESTAPI/RESTAPI_devices_handler.cpp src/RESTAPI/RESTAPI_devices_handler.h
        src/RESTAPI/RESTAPI_device_handler.cpp src/RESTAPI/RESTAPI_device_handler.h
        src/RESTAPI/RESTAPI_device_commandHandler.cpp src/RESTAPI/RESTAPI_device_commandHandler.h
        src/RESTAPI/RESTAPI_default_configuration.cpp
        src/RESTAPI/RESTAPI_default_configuration.h src/RESTAPI/RESTAPI_default_configurations.cpp src/RESTAPI/RESTAPI_default_configurations.h
        src/RESTAPI/RESTAPI_commands.cpp src/RESTAPI/RESTAPI_commands.h
        src/RESTAPI/RESTAPI_command.cpp src/RESTAPI/RESTAPI_command.h
        src/RESTAPI/RESTAPI_file.cpp src/RESTAPI/RESTAPI_file.h
        src/RESTAPI/RESTAPI_blacklist.cpp src/RESTAPI/RESTAPI_blacklist.h
        src/RESTAPI/RESTAPI_ouis.cpp src/RESTAPI/RESTAPI_ouis.h
        src/RESTAPI/RESTAPI_blacklist_list.cpp src/RESTAPI/RESTAPI_blacklist_list.h
        src/RESTAPI/RESTAPI_capabilities_handler.cpp src/RESTAPI/RESTAPI_capabilities_handler.h
        src/RESTAPI/RESTAPI_RPC.cpp src/RESTAPI/RESTAPI_RPC.h
        src/RESTAPI/RESTAPI_deviceDashboardHandler.cpp src/RESTAPI/RESTAPI_deviceDashboardHandler.h
        src/RESTAPI/RESTAPI_telemetryWebSocket.cpp src/RESTAPI/RESTAPI_telemetryWebSocket.h
        src/RESTAPI/RESTAPI_scripts_handler.cpp src/RESTAPI/RESTAPI_scripts_handler.h
        src/RESTAPI/RESTAPI_script_handler.cpp src/RESTAPI/RESTAPI_script_handler.h
        src/RESTAPI/RESTAPI_regulatory.cpp src/RESTAPI/RESTAPI_regulatory.h
        src/RESTAPI/RESTAPI_radiussessions_handler.cpp src/RESTAPI/RESTAPI_radiussessions_handler.h
        src/storage/storage_blacklist.cpp src/storage/storage_tables.cpp src/storage/storage_logs.cpp
        src/storage/storage_command.cpp src/storage/storage_healthcheck.cpp src/storage/storage_statistics.cpp
        src/storage/storage_device.cpp src/storage/storage_capabilities.cpp src/storage/storage_defconfig.cpp
        src/storage/storage_scripts.cpp src/storage/storage_scripts.h
        src/storage/storage_tables.cpp
        src/RESTAPI/RESTAPI_routers.cpp
        src/Daemon.cpp src/Daemon.h
        src/AP_WS_Server.cpp src/AP_WS_Server.h
        src/StorageService.cpp src/StorageService.h
        src/CommandManager.cpp src/CommandManager.h
        src/CentralConfig.cpp src/CentralConfig.h
        src/FileUploader.cpp src/FileUploader.h
        src/OUIServer.cpp src/OUIServer.h
        src/StorageArchiver.cpp src/StorageArchiver.h
        src/Dashboard.cpp src/Dashboard.h
        src/SerialNumberCache.cpp src/SerialNumberCache.h
        src/TelemetryStream.cpp src/TelemetryStream.h
        src/framework/ConfigurationValidator.cpp src/framework/ConfigurationValidator.h
        src/ConfigurationCache.h
        src/CapabilitiesCache.h src/FindCountry.h
        src/rttys/RTTYS_server.cpp
        src/rttys/RTTYS_server.h
        src/rttys/RTTYS_WebServer.cpp
        src/rttys/RTTYS_WebServer.h src/RESTAPI/RESTAPI_device_helper.h
        src/SDKcalls.cpp
        src/SDKcalls.h
        src/StateUtils.cpp src/StateUtils.h
        src/AP_WS_Reactor_Pool.h
        src/AP_WS_Connection.h
        src/AP_WS_Connection.cpp
        src/TelemetryClient.h src/TelemetryClient.cpp
        src/RESTAPI/RESTAPI_iptocountry_handler.cpp src/RESTAPI/RESTAPI_iptocountry_handler.h
        src/framework/ow_constants.h
        src/GwWebSocketClient.cpp src/GwWebSocketClient.h
        src/RADIUS_proxy_server.cpp src/RADIUS_proxy_server.h
        src/RESTAPI/RESTAPI_radiusProxyConfig_handler.cpp src/RESTAPI/RESTAPI_radiusProxyConfig_handler.h
        src/ParseWifiScan.h
        src/RADIUS_helpers.h
        src/VenueBroadcaster.h
        src/sdks/sdk_prov.h
        src/AP_WS_Process_connect.cpp
        src/AP_WS_Process_state.cpp
        src/AP_WS_Process_healthcheck.cpp
        src/AP_WS_Process_log.cpp
        src/AP_WS_Process_crashlog.cpp
        src/AP_WS_Process_ping.cpp
        src/AP_WS_Process_cfgpending.cpp
        src/AP_WS_Process_recovery.cpp
        src/AP_WS_Process_deviceupdate.cpp
        src/AP_WS_Process_telemetry.cpp
        src/AP_WS_Process_venuebroadcast.cpp
        src/RADIUS_Destination.h
        src/UI_GW_WebSocketNotifications.cpp src/UI_GW_WebSocketNotifications.h
        src/framework/RESTAPI_SystemConfiguration.h
        src/ScriptManager.cpp src/ScriptManager.h
        src/SignatureMgr.h
        src/AP_WS_Process_event.cpp
        src/AP_WS_Process_wifiscan.cpp
        src/AP_WS_Process_alarm.cpp
        src/GWKafkaEvents.cpp src/GWKafkaEvents.h
        src/RegulatoryInfo.cpp src/RegulatoryInfo.h
        src/RADIUSSessionTracker.cpp src/RADIUSSessionTracker.h
        src/libs/Scheduler.h src/libs/InterruptableSleep.h src/libs/ctpl_stl.h src/libs/Cron.h
        src/GenericScheduler.cpp src/GenericScheduler.h src/framework/default_device_types.h src/AP_WS_Process_rebootLog.cpp src/AP_WS_ConfigAutoUpgrader.cpp src/AP_WS_ConfigAutoUpgrader.h src/RESTAPI/RESTAPI_default_firmwares.cpp src/RESTAPI/RESTAPI_default_firmwares.h src/RESTAPI/RESTAPI_default_firmware.cpp src/RESTAPI/RESTAPI_default_firmware.h src/storage/storage_def_firmware.cpp src/firmware_revision_cache.h src/sdks/sdk_fms.h
        src/AP_WS_LookForUpgrade.cpp)

if(NOT SMALL_BUILD)

endif()

INSTALL(TARGETS owgw
        RUNTIME DESTINATION /usr/bin
)

target_link_libraries(owgw PUBLIC
        ${Poco_LIBRARIES}
        ${ZLIB_LIBRARIES}
)

if(NOT SMALL_BUILD)
    target_link_libraries(owgw PUBLIC
            ${MySQL_LIBRARIES}
            ${ZLIB_LIBRARIES}
            CppKafka::cppkafka
            fmt::fmt
            resolv
    )
    if(UNIX AND NOT APPLE)
        target_link_libraries(owgw PUBLIC PocoJSON)
    endif()
endif()
