cmake_minimum_required(VERSION 3.10)
project(ucentral VERSION 0.3.0)

if(UNIX AND APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    set(MYSQL_ROOT_DIR /usr/local/opt/mysql-client)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

if(UNIX AND NOT APPLE)
    set(PostgreSQL_TYPE_INCLUDE_DIR /usr/include/postgresql)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

set(CMAKE_CXX_STANDARD 17)

if(SMALL_BUILD)
    add_definitions(-DSMALL_BUILD)
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED system)
find_package(OpenSSL REQUIRED)
find_package(yaml-cpp REQUIRED)
if(NOT SMALL_BUILD)
find_package(PostgreSQL REQUIRED)
find_package(MySQL REQUIRED)
find_package(ODBC REQUIRED)
endif()

if(SMALL_BUILD)
    find_package(Poco REQUIRED COMPONENTS Crypto Net Util NetSSL Data DataSQLite)
else()
    find_package(Poco REQUIRED COMPONENTS Crypto Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL DataODBC)
endif()

include_directories(/usr/local/include  /usr/local/opt/openssl/include src)

add_executable( ucentral
                src/main.cpp src/TIPGWServer.cpp src/TIPGWServer.h
                src/TIP/Api.cpp src/TIP/Api.h
                src/uCentral.cpp src/uCentral.h src/TIP/Routing.cpp src/TIP/Routing.h
                src/TIP/WebTokenResult.cpp src/TIP/WebTokenResult.h src/uCentralRESTAPIServer.cpp src/uCentralRESTAPIServer.h
                src/uCentralWebSocketServer.cpp src/uCentralWebSocketServer.h src/SubSystemServer.cpp src/SubSystemServer.h
                src/uStorageService.cpp src/uStorageService.h src/uDeviceRegistry.cpp src/uDeviceRegistry.h
                src/RESTAPI_oauth2Handler.cpp src/RESTAPI_oauth2Handler.h src/RESTAPI_devicesHandler.cpp src/RESTAPI_devicesHandler.h
                src/RESTAPI_deviceHandler.cpp src/RESTAPI_deviceHandler.h src/RESTAPI_UnknownRequestHandler.cpp src/RESTAPI_UnknownRequestHandler.h
                src/RESTAPI_Handler.cpp src/RESTAPI_Handler.h src/RESTAPI_deviceCommandHandler.cpp src/RESTAPI_deviceCommandHandler.h
                src/RESTAPI_Objects.h src/RESTAPI_Objects.cpp src/uAuthService.cpp src/uAuthService.h src/uCentralConfig.cpp src/uCentralConfig.h
                src/RESTAPI_default_configuration.cpp
                src/RESTAPI_default_configuration.h src/RESTAPI_default_configurations.cpp src/RESTAPI_default_configurations.h)

if(SMALL_BUILD)
target_link_libraries(ucentral PRIVATE yaml-cpp
                        ${Poco_LIBRARIES} ${Boost_LIBRARIES})
else()
    target_link_libraries(ucentral PRIVATE yaml-cpp
            ${Poco_LIBRARIES} ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES}
            ${MySQL_LIBRARIES} ${ODBC_LIBRARIES})
endif()