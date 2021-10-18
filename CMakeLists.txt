cmake_minimum_required(VERSION 3.13)
project(owgw VERSION 2.2.0)

set(CMAKE_CXX_STANDARD 17)

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
    file(READ build BUILD_NUM)
    if(BUILD_INCREMENT)
        MATH(EXPR BUILD_NUM "${BUILD_NUM}+1")
        file(WRITE build ${BUILD_NUM})
    endif()
else()
    set(BUILD_NUM 1)
    file(WRITE build ${BUILD_NUM})
endif()
add_definitions(-DAPP_VERSION="${CMAKE_PROJECT_VERSION}" -DBUILD_NUMBER="${BUILD_NUM}")
add_definitions(-DTIP_GATEWAY_SERVICE="1")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED system)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(nlohmann_json  REQUIRED)
find_package(nlohmann_json_schema_validator REQUIRED)

if(SMALL_BUILD)
    find_package(Poco REQUIRED COMPONENTS Crypto JWT Net Util NetSSL Data DataSQLite)
else()
    find_package(CppKafka REQUIRED)
    find_package(PostgreSQL REQUIRED)
    find_package(MySQL REQUIRED)
    find_package(Poco REQUIRED COMPONENTS JSON Crypto JWT Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL)
endif()

include_directories(/usr/local/include  /usr/local/opt/openssl/include src include/kafka /usr/local/opt/mysql-client/include)

add_executable( owgw
                build
                src/Daemon.cpp src/Daemon.h
                src/RESTAPI_server.cpp src/RESTAPI_server.h
                src/WebSocketServer.cpp src/WebSocketServer.h
                src/SubSystemServer.cpp src/SubSystemServer.h
                src/StorageService.cpp src/StorageService.h
                src/RESTAPI_SecurityObjects.cpp src/RESTAPI_SecurityObjects.h
                src/DeviceRegistry.cpp src/DeviceRegistry.h
                src/RESTAPI_devices_handler.cpp src/RESTAPI_devices_handler.h
                src/RESTAPI_device_handler.cpp src/RESTAPI_device_handler.h
                src/RESTAPI_handler.cpp src/RESTAPI_handler.h
                src/RESTAPI_device_commandHandler.cpp src/RESTAPI_device_commandHandler.h
                src/RESTAPI_GWobjects.h src/RESTAPI_GWobjects.cpp
                src/CentralConfig.cpp src/CentralConfig.h
                src/RESTAPI_default_configuration.cpp
                src/RESTAPI_InternalServer.cpp src/RESTAPI_InternalServer.h
                src/RESTAPI_default_configuration.h src/RESTAPI_default_configurations.cpp src/RESTAPI_default_configurations.h
                src/RESTAPI_commands.cpp src/RESTAPI_commands.h
                src/CommandManager.cpp src/CommandManager.h
                src/RESTAPI_command.cpp src/RESTAPI_command.h
                src/FileUploader.cpp src/FileUploader.h
                src/RESTAPI_file.cpp src/RESTAPI_file.h
                src/RESTAPI_system_command.cpp src/RESTAPI_system_command.h
        src/RESTAPI_blacklist.cpp src/RESTAPI_blacklist.h
                src/Utils.h src/Utils.cpp src/storage_blacklist.cpp
                src/storage_command.cpp src/storage_healthcheck.cpp src/storage_statistics.cpp src/storage_logs.cpp
                src/storage_device.cpp src/storage_capabilities.cpp src/storage_defconfig.cpp
                src/storage_tables.cpp
                src/storage_setup.cpp
                src/StateProcessor.cpp src/StateProcessor.h
                src/storage_lifetime_stats.cpp src/uCentralProtocol.h src/RESTAPI_protocol.h
                src/ALBHealthCheckServer.h src/Kafka_topics.h
                src/OUIServer.cpp src/OUIServer.h
                src/RESTAPI_ouis.cpp src/RESTAPI_ouis.h
                src/MicroService.cpp src/MicroService.h
                src/RESTAPI_RPC.cpp src/RESTAPI_RPC.h
                src/AuthClient.cpp src/AuthClient.h
                src/OpenAPIRequest.cpp src/OpenAPIRequest.h
                src/RESTAPI_utils.h src/RESTAPI_utils.cpp
                src/StorageArchiver.cpp src/StorageArchiver.h
                src/Dashboard.cpp src/Dashboard.h
                src/RESTAPI_deviceDashboardHandler.cpp src/RESTAPI_deviceDashboardHandler.h
                src/SerialNumberCache.cpp src/SerialNumberCache.h
                src/RESTAPI_webSocketServer.cpp src/RESTAPI_webSocketServer.h
                src/OpenWifiTypes.h src/TelemetryStream.cpp src/TelemetryStream.h
                src/RESTAPI_GenericServer.cpp src/RESTAPI_GenericServer.h
                src/RESTAPI_errors.h src/RESTAPI_TelemetryWebSocket.cpp src/RESTAPI_TelemetryWebSocket.h
                src/Storage.h
                src/ConfigurationValidator.cpp src/ConfigurationValidator.h src/ConfigurationCache.cpp src/ConfigurationCache.h src/RESTAPI_blacklist_list.cpp src/RESTAPI_blacklist_list.h src/RESTAPI_capabilities_handler.cpp src/RESTAPI_capabilities_handler.h)

if(NOT SMALL_BUILD)
    target_sources(owgw PUBLIC src/KafkaManager.cpp src/KafkaManager.h)
endif()

INSTALL(TARGETS owgw
        RUNTIME DESTINATION /usr/bin
)

target_link_libraries(owgw PUBLIC
                        ${Poco_LIBRARIES} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
if(NOT SMALL_BUILD)
    target_link_libraries(owgw PUBLIC
            ${MySQL_LIBRARIES} ${ZLIB_LIBRARIES}
                CppKafka::cppkafka
                nlohmann_json_schema_validator
             )
    if(UNIX AND NOT APPLE)
        target_link_libraries(owgw PUBLIC PocoJSON)
    endif()
endif()