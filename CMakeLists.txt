cmake_minimum_required(VERSION 3.13)
project(ucentralgw VERSION 0.4.0)

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

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED system)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Lua REQUIRED)

if(SMALL_BUILD)
    find_package(Poco REQUIRED COMPONENTS Crypto JWT Net Util NetSSL Data DataSQLite)
else()
    find_package(CppKafka REQUIRED)
    find_package(PostgreSQL REQUIRED)
    find_package(MySQL REQUIRED)
    find_package(ODBC REQUIRED)
    find_package(Poco REQUIRED COMPONENTS Crypto JWT Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL DataODBC)
endif()

include_directories(/usr/local/include  /usr/local/opt/openssl/include src include/kafka)

add_executable( ucentralgw
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
                src/RESTAPI_objects.h src/RESTAPI_objects.cpp
                src/CentralConfig.cpp src/CentralConfig.h
                src/RESTAPI_default_configuration.cpp
                src/RESTAPI_InternalServer.cpp src/RESTAPI_InternalServer.h
                src/RESTAPI_default_configuration.h src/RESTAPI_default_configurations.cpp src/RESTAPI_default_configurations.h
                src/RESTAPI_commands.cpp src/RESTAPI_commands.h
                src/CommandManager.cpp src/CommandManager.h
                src/RESTAPI_command.cpp src/RESTAPI_command.h
                src/FileUploader.cpp src/FileUploader.h
                src/RESTAPI_file.cpp src/RESTAPI_file.h
                src/CommandChannel.cpp src/CommandChannel.h
                src/RESTAPI_system_command.cpp src/RESTAPI_system_command.h
                src/RESTAPI_BlackList.cpp src/RESTAPI_BlackList.h
                src/Utils.h src/Utils.cpp src/storage_blacklist.cpp
                src/storage_command.cpp src/storage_healthcheck.cpp src/storage_statistics.cpp src/storage_logs.cpp
                src/storage_device.cpp src/storage_capabilities.cpp src/storage_defconfig.cpp src/storage_sqlite.cpp
                src/storage_mysql.cpp src/storage_pgql.cpp src/storage_odbc.cpp src/storage_tables.cpp src/RESTAPI_callback.cpp
                src/RESTAPI_callback.h src/CallbackManager.cpp src/CallbackManager.h
                src/StateProcessor.cpp src/StateProcessor.h
                src/storage_lifetime_stats.cpp src/uCentralProtocol.h src/RESTAPI_protocol.h
                src/ALBHealthCheckServer.h src/Kafka_topics.h src/uCentralTypes.h
                src/OUIServer.cpp src/OUIServer.h
                src/RESTAPI_ouis.cpp src/RESTAPI_ouis.h
                src/MicroService.cpp src/MicroService.h
                src/RESTAPI_RPC.cpp src/RESTAPI_RPC.h src/AuthClient.cpp src/AuthClient.h src/OpenAPIRequest.cpp src/OpenAPIRequest.h src/RESTAPI_utils.h src/RESTAPI_utils.cpp)

if(NOT SMALL_BUILD)
    target_sources(ucentralgw PUBLIC src/KafkaManager.cpp src/KafkaManager.h)
endif()

INSTALL(TARGETS ucentralgw
        RUNTIME DESTINATION /usr/bin
)

target_link_libraries(ucentralgw PUBLIC
                        ${Poco_LIBRARIES} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
if(NOT SMALL_BUILD)
    target_link_libraries(ucentralgw PUBLIC
            ${MySQL_LIBRARIES} ${ODBC_LIBRARIES} ${ZLIB_LIBRARIES} ${LUA_LIBRARIES}
            CppKafka::cppkafka
             )
endif()