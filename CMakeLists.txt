cmake_minimum_required(VERSION 3.10)
project(ucentral)

set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
find_package(Boost REQUIRED system)
find_package(OpenSSL REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Poco REQUIRED Crypto Net Util NetSSL Data DataSQLite)

include_directories(/usr/local/include  /usr/local/opt/openssl/include src)

add_executable(ucentral
                src/main.cpp src/TIPGWServer.cpp src/TIPGWServer.h
                src/portal-services-openapi.cpp src/portal-services-openapi.h
                src/TIP/Api.cpp src/TIP/Api.h src/PropertiesFileServerList.cpp src/PropertiesFileServerList.h src/svr.cpp src/svr.h
                src/uCentral.cpp src/uCentral.h src/common.h src/TIP/Routing.cpp src/TIP/Routing.h
                src/TIP/WebTokenResult.cpp src/TIP/WebTokenResult.h src/uCentralRESTAPIServer.cpp src/uCentralRESTAPIServer.h
                src/uCentralWebSocketServer.cpp src/uCentralWebSocketServer.h src/SubSystemServer.cpp src/SubSystemServer.h
                src/uStorageService.cpp src/uStorageService.h src/DeviceStatusServer.cpp src/DeviceStatusServer.h src/RESTAPI_oauth2Handler.cpp src/RESTAPI_oauth2Handler.h src/RESTAPI_devicesHandler.cpp src/RESTAPI_devicesHandler.h src/RESTAPI_deviceHandler.cpp src/RESTAPI_deviceHandler.h src/RESTAPI_UnknownRequestHandler.cpp src/RESTAPI_UnknownRequestHandler.h)

target_link_libraries(ucentral PRIVATE yaml-cpp ${Poco_LIBRARIES} ${Boost_LIBRARIES})