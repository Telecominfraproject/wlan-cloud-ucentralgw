cmake_minimum_required(VERSION 3.13)
project(owgw VERSION 2.6.0)

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
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/build BUILD_NUM)
    if(BUILD_INCREMENT)
        MATH(EXPR BUILD_NUM "${BUILD_NUM}+1")
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/build ${BUILD_NUM})
    endif()
else()
    set(BUILD_NUM 1)
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/build ${BUILD_NUM})
endif()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --tags
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_RESULT
            OUTPUT_VARIABLE GIT_HASH)
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "git describe --always --tags failed with ${GIT_RESULT}")
    endif()
    string(REGEX REPLACE "\n$" "" GIT_HASH "${GIT_HASH}")
endif()

add_definitions(-DTIP_GATEWAY_SERVICE="1")

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(fmt  REQUIRED)
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

configure_file(src/ow_version.h.in ${PROJECT_SOURCE_DIR}/src/ow_version.h @ONLY)

add_compile_options(-Wall -Wextra)
if(ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

add_executable( owgw
        build
        src/ow_version.h.in
        src/framework/CountryCodes.h
        src/framework/KafkaTopics.h
        src/framework/MicroService.h
        src/framework/OpenWifiTypes.h
        src/framework/orm.h
        src/framework/StorageClass.h
        src/RESTObjects/RESTAPI_SecurityObjects.h src/RESTObjects/RESTAPI_SecurityObjects.cpp
        src/RESTObjects/RESTAPI_ProvObjects.cpp src/RESTObjects/RESTAPI_ProvObjects.h
        src/RESTObjects/RESTAPI_GWobjects.h src/RESTObjects/RESTAPI_GWobjects.cpp
        src/RESTObjects/RESTAPI_FMSObjects.h src/RESTObjects/RESTAPI_FMSObjects.cpp
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
        src/storage/storage_blacklist.cpp src/storage/storage_tables.cpp src/storage/storage_logs.cpp
        src/storage/storage_command.cpp src/storage/storage_healthcheck.cpp src/storage/storage_statistics.cpp
        src/storage/storage_device.cpp src/storage/storage_capabilities.cpp src/storage/storage_defconfig.cpp
        src/storage/storage_tables.cpp
        src/RESTAPI/RESTAPI_routers.cpp
        src/Daemon.cpp src/Daemon.h
        src/WS_Server.cpp src/WS_Server.h
        src/StorageService.cpp src/StorageService.h
        src/DeviceRegistry.cpp src/DeviceRegistry.h
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
        src/CapabilitiesCache.h src/FindCountry.h src/rttys/RTTYS_server.cpp src/rttys/RTTYS_server.h src/rttys/RTTYS_device.cpp src/rttys/RTTYS_device.h src/rttys/RTTYS_ClientConnection.cpp src/rttys/RTTYS_ClientConnection.h src/rttys/RTTYS_WebServer.cpp src/rttys/RTTYS_WebServer.h src/RESTAPI/RESTAPI_device_helper.h src/SDKcalls.cpp src/SDKcalls.h src/StateUtils.cpp src/StateUtils.h src/WS_ReactorPool.h src/WS_Connection.h src/WS_Connection.cpp src/TelemetryClient.h src/TelemetryClient.cpp src/RESTAPI/RESTAPI_iptocountry_handler.cpp src/RESTAPI/RESTAPI_iptocountry_handler.h src/framework/ow_constants.h src/GwWebSocketClient.cpp src/GwWebSocketClient.h src/framework/WebSocketClientNotifications.h)

if(NOT SMALL_BUILD)

endif()

INSTALL(TARGETS owgw
        RUNTIME DESTINATION /usr/bin
)

target_link_libraries(owgw PUBLIC
        ${Poco_LIBRARIES}
        ${ZLIB_LIBRARIES})

if(NOT SMALL_BUILD)
    target_link_libraries(owgw PUBLIC
            ${MySQL_LIBRARIES} ${ZLIB_LIBRARIES}
                CppKafka::cppkafka
                nlohmann_json_schema_validator
                fmt::fmt
             )
    if(UNIX AND NOT APPLE)
        target_link_libraries(owgw PUBLIC PocoJSON)
    endif()
endif()