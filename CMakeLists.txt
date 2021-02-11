cmake_minimum_required(VERSION 3.17)
project(ucentral)

set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(cpprestsdk REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread system)
find_package(OpenSSL REQUIRED)

include_directories(/usr/local/include  /usr/local/opt/openssl/include)

# link_directories(/usr/local/lib /usr/local/opt/openssl/lib)

add_executable(ucentral main.cpp)

target_link_libraries(ucentral PRIVATE cpprestsdk::cpprest  ${Boost_LIBRARIES})



