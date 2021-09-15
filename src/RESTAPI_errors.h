//
// Created by stephane bourque on 2021-09-12.
//

#ifndef OWPROV_RESTAPI_ERRORS_H
#define OWPROV_RESTAPI_ERRORS_H

namespace OpenWifi::RESTAPI::Errors {
    static const std::string MissingUUID{"Missing UUID."};
    static const std::string MissingSerialNumber{"Missing Serial Number."};
    static const std::string InternalError{"Internal error. Please try later."};
    static const std::string InvalidJSONDocument{"Invalid JSON document."};
    static const std::string UnsupportedHTTPMethod{"Unsupported HTTP Method"};
    static const std::string StillInUse{"Element still in use."};
    static const std::string CouldNotBeDeleted{"Element could not be deleted."};
    static const std::string NameMustBeSet{"The name property must be set."};
    static const std::string ConfigBlockInvalid{"Configuration block type invalid."};
    static const std::string UnknownId{"Unknown management policy."};
    static const std::string InvalidDeviceTypes{"Unknown or invalid device type(s)."};
    static const std::string RecordNotCreated{"Record could not be created."};
    static const std::string RecordNotUpdated{"Record could not be updated."};
    static const std::string UnknownManagementPolicyUUID{"Unknown management policy UUID."};
    static const std::string CannotDeleteRoot{"Root Entity cannot be removed, only modified."};
    static const std::string MustCreateRootFirst{"Root entity must be created first."};
    static const std::string ParentUUIDMustExist{"Parent UUID must exist."};
    static const std::string ConfigurationMustExist{"Configuration must exist."};
    static const std::string MissingOrInvalidParameters{"Invalid or missing parameters."};
    static const std::string UnknownSerialNumber{"Unknown Serial Number."};
    static const std::string InvalidSerialNumber{"Invalid Serial Number."};
    static const std::string SerialNumberExists{"Serial Number already exists."};
    static const std::string ValidNonRootUUID{"Must be a non-root, and valid UUID."};
    static const std::string VenueMustExist{"Venue does not exist."};
    static const std::string NotBoth{"You cannot specify both Entity and Venue"};
    static const std::string EntityMustExist{"Entity must exist."};
    static const std::string ParentOrEntityMustBeSet{"Parent or Entity must be set."};
    static const std::string ContactMustExist{"Contact must exist."};
    static const std::string LocationMustExist{"Location must exist."};
    static const std::string OnlyWSSupported{"This endpoint only supports WebSocket."};

}

#endif //OWPROV_RESTAPI_ERRORS_H
