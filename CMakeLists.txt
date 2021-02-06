cmake_minimum_required(VERSION 3.17)
project(ucentral)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif()

set(CMAKE_CXX_STANDARD 20)

add_executable(ucentral main.cpp)

target_link_libraries(ucentral ${CONAN_LIBS})
### target_link_libraries(ucentral PRIVATE spdlog::spdlog_header_only)
