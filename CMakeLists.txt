cmake_minimum_required(VERSION 3.10)
project(ucentral)

set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
# find_package(cpprestsdk REQUIRED)
find_package(Boost REQUIRED system)
find_package(OpenSSL REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Poco REQUIRED Crypto Net)

include_directories(/usr/local/include  /usr/local/opt/openssl/include)

add_executable(ucentral src/main.cpp src/gateway.cpp src/gateway.h src/ucentralServer.cpp src/ucentralServer.h src/tipApi.cpp src/tipApi.h src/config.cpp src/config.h src/svr.cpp src/svr.h)

# target_link_libraries(ucentral PRIVATE cpprestsdk::cpprest yaml-cpp ${Boost_LIBRARIES})
target_link_libraries(ucentral PRIVATE yaml-cpp Poco::Crypto Poco::Net ${Boost_LIBRARIES})