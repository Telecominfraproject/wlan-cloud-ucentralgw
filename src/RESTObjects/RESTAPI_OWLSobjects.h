//
// Created by stephane bourque on 2021-08-31.
//

#ifndef UCENTRALSIM_RESTAPI_OWLSOBJECTS_H
#define UCENTRALSIM_RESTAPI_OWLSOBJECTS_H

#include <vector>
#include "Poco/JSON/Object.h"

namespace OpenWifi::OWLSObjects {

    struct SimulationDetails {
        std::string     id;
        std::string     name;
        std::string     gateway;
        std::string     certificate;
        std::string     key;
        std::string     macPrefix;
        std::string     deviceType;
        uint64_t        devices = 5;
        uint64_t        healthCheckInterval = 60;
        uint64_t        stateInterval = 60 ;
        uint64_t        minAssociations = 1;
        uint64_t        maxAssociations = 3;
        uint64_t        minClients = 1 ;
        uint64_t        maxClients = 3;
        uint64_t        simulationLength = 60 * 60;
        uint64_t        threads = 16;
        uint64_t        clientInterval = 1;
        uint64_t        keepAlive = 300;
        uint64_t        reconnectInterval = 30 ;
        uint64_t        concurrentDevices = 5;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct SimulationDetailsList {
        std::vector<SimulationDetails>  list;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct SimulationStatus {
        std::string     id;
        std::string     simulationId;
        std::string     state;
        uint64_t        tx;
        uint64_t        rx;
        uint64_t        msgsTx;
        uint64_t        msgsRx;
        uint64_t        liveDevices;
        uint64_t        timeToFullDevices;
        uint64_t        startTime;
        uint64_t        endTime;
        uint64_t        errorDevices;
        std::string     owner;

        void to_json(Poco::JSON::Object &Obj) const;
    };


    struct Dashboard {
        int O;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
        void reset();

    };

}


#endif //UCENTRALSIM_RESTAPI_OWLSOBJECTS_H
