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
find_package(yaml-cpp REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Lua REQUIRED)

if(SMALL_BUILD)
    find_package(Poco REQUIRED COMPONENTS Crypto Net Util NetSSL Data DataSQLite)
else()
    find_package(CppKafka REQUIRED)
    find_package(PostgreSQL REQUIRED)
    find_package(MySQL REQUIRED)
    find_package(ODBC REQUIRED)
    find_package(Poco REQUIRED COMPONENTS Crypto Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL DataODBC)
endif()

include_directories(/usr/local/include  /usr/local/opt/openssl/include src include/kafka)

add_executable( ucentralgw
                build
                src/main.cpp
                src/uCentral.cpp src/uCentral.h src/uCentralRESTAPIServer.cpp src/uCentralRESTAPIServer.h
                src/uCentralWebSocketServer.cpp src/uCentralWebSocketServer.h src/SubSystemServer.cpp src/SubSystemServer.h
                src/uStorageService.cpp src/uStorageService.h src/uDeviceRegistry.cpp src/uDeviceRegistry.h
                src/RESTAPI_oauth2Handler.cpp src/RESTAPI_oauth2Handler.h src/RESTAPI_devicesHandler.cpp src/RESTAPI_devicesHandler.h
                src/RESTAPI_deviceHandler.cpp src/RESTAPI_deviceHandler.h src/RESTAPI_unknownRequestHandler.cpp src/RESTAPI_unknownRequestHandler.h
                src/RESTAPI_handler.cpp src/RESTAPI_handler.h src/RESTAPI_deviceCommandHandler.cpp src/RESTAPI_deviceCommandHandler.h
                src/RESTAPI_objects.h src/RESTAPI_objects.cpp src/uAuthService.cpp src/uAuthService.h src/uCentralConfig.cpp src/uCentralConfig.h
                src/RESTAPI_default_configuration.cpp
                src/RESTAPI_default_configuration.h src/RESTAPI_default_configurations.cpp src/RESTAPI_default_configurations.h src/RESTAPI_commands.cpp
                src/RESTAPI_commands.h src/uCommandManager.cpp src/uCommandManager.h src/RESTAPI_command.cpp src/RESTAPI_command.h src/base64util.cpp
                src/base64util.h src/uFileUploader.cpp src/uFileUploader.h src/RESTAPI_file.cpp src/RESTAPI_file.h src/CommandChannel.cpp src/CommandChannel.h
                src/RESTAPI_SystemCommand.cpp src/RESTAPI_SystemCommand.h src/RESTAPI_BlackList.cpp src/RESTAPI_BlackList.h)

if(NOT SMALL_BUILD)
    target_sources(ucentralgw PUBLIC src/kafka_service.cpp src/kafka_service.h)
endif()

INSTALL(TARGETS ucentralgw
        RUNTIME DESTINATION /usr/bin
)

target_link_libraries(ucentralgw PUBLIC yaml-cpp
                        ${Poco_LIBRARIES} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
if(NOT SMALL_BUILD)
    target_link_libraries(ucentralgw PUBLIC
            ${MySQL_LIBRARIES} ${ODBC_LIBRARIES} ${ZLIB_LIBRARIES} ${LUA_LIBRARIES}
            CppKafka::cppkafka
             )
endif()