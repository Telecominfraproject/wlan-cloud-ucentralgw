//
// Created by stephane bourque on 2021-03-04.
//

#include "uAuthService.h"

namespace uCentral::Auth {
    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("Authentication", "AUTH-SVR", "authentication") {
    }

    int Service::start() {
        return 0;
    }

    void Service::stop() {

    }

};  // end of namespace
