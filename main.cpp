#include <iostream>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include "oatpp/network/Server.hpp"
#include "oatpp-websocket/AsyncWebSocket.hpp"

int main() {

    oatpp::base::Environment::init();

    std::cout << "Hello, World!" << std::endl;

    spdlog::warn("Easy padding in numbers like {:08d}", 12);

    return 0;
}
