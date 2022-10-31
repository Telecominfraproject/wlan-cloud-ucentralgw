//
// Created by stephane bourque on 2021-08-31.
//

#include "framework/RESTAPI_utils.h"

using OpenWifi::RESTAPI_utils::field_to_json;
using OpenWifi::RESTAPI_utils::field_from_json;
using OpenWifi::RESTAPI_utils::EmbedDocument;

#include "RESTAPI_OWLSobjects.h"

// SIM -> 0x53/0x073, 0x49/0x69, 0x4d/0x6d

namespace OpenWifi::OWLSObjects {

    void SimulationDetails::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"id", id);
        field_to_json(Obj,"name", name);
        field_to_json(Obj,"gateway", gateway);
        field_to_json(Obj,"certificate", certificate);
        field_to_json(Obj,"key", key);
        field_to_json(Obj,"macPrefix", macPrefix);
        field_to_json(Obj,"deviceType", deviceType);
        field_to_json(Obj,"devices", devices);
        field_to_json(Obj,"healthCheckInterval", healthCheckInterval);
        field_to_json(Obj,"stateInterval", stateInterval);
        field_to_json(Obj,"minAssociations", minAssociations);
        field_to_json(Obj,"maxAssociations", maxAssociations);
        field_to_json(Obj,"minClients", minClients);
        field_to_json(Obj,"maxClients", maxClients);
        field_to_json(Obj,"simulationLength", simulationLength);
        field_to_json(Obj,"threads", threads);
        field_to_json(Obj,"clientInterval", clientInterval);
        field_to_json(Obj,"keepAlive", keepAlive);
        field_to_json(Obj,"reconnectInterval", reconnectInterval);
        field_to_json(Obj,"concurrentDevices", concurrentDevices);
    }

    bool SimulationDetails::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"id", id);
            field_from_json(Obj,"name", name);
            field_from_json(Obj,"gateway", gateway);
            field_from_json(Obj,"certificate", certificate);
            field_from_json(Obj,"key", key);
            field_from_json(Obj,"macPrefix", macPrefix);
            field_from_json(Obj,"deviceType", deviceType);
            field_from_json(Obj,"devices", devices);
            field_from_json(Obj,"healthCheckInterval", healthCheckInterval);
            field_from_json(Obj,"stateInterval", stateInterval);
            field_from_json(Obj,"minAssociations", minAssociations);
            field_from_json(Obj,"maxAssociations", maxAssociations);
            field_from_json(Obj,"minClients", minClients);
            field_from_json(Obj,"maxClients", maxClients);
            field_from_json(Obj,"simulationLength", simulationLength);
            field_from_json(Obj,"threads", threads);
            field_from_json(Obj,"clientInterval", clientInterval);
            field_from_json(Obj,"keepAlive", keepAlive);
            field_from_json(Obj,"reconnectInterval", reconnectInterval);
            field_from_json(Obj,"concurrentDevices", concurrentDevices);
            return true;
        } catch(...) {

        }
        return false;
    }

    void SimulationDetailsList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"list", list);
    }

    bool SimulationDetailsList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"list", list);
            return true;
        } catch(...) {

        }
        return false;
    }

    void SimulationStatus::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"id", id);
        field_to_json(Obj,"simulationId", simulationId);
        field_to_json(Obj,"state", state);
        field_to_json(Obj,"tx", tx);
        field_to_json(Obj,"rx", rx);
        field_to_json(Obj,"msgsTx", msgsTx);
        field_to_json(Obj,"msgsRx", msgsRx);
        field_to_json(Obj,"liveDevices", liveDevices);
        field_to_json(Obj,"timeToFullDevices", timeToFullDevices);
        field_to_json(Obj,"startTime", startTime);
        field_to_json(Obj,"endTime", endTime);
        field_to_json(Obj,"errorDevices", errorDevices);
        field_to_json(Obj,"owner", owner);
    }

    void Dashboard::to_json([[maybe_unused]] Poco::JSON::Object &Obj) const {

    }

    bool Dashboard::from_json([[maybe_unused]] const Poco::JSON::Object::Ptr &Obj) {
        return true;
    }

    void Dashboard::reset() {

    }
}
