//
// Created by stephane bourque on 2021-09-12.
//

#pragma once

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
    static const std::string SerialNumberMismatch{"Serial Number mismatch."};
    static const std::string InvalidCommand{"Invalid command."};
    static const std::string NoRecordsDeleted{"No records deleted."};
    static const std::string DeviceNotConnected{"Device is not currently connected."};
    static const std::string CannotCreateWS{"Telemetry system could not create WS endpoint. Please try again."};
    static const std::string BothDeviceTypeRevision{"Both deviceType and revision must be set."};
    static const std::string IdOrSerialEmpty{"SerialNumber and Id must not be empty."};
    static const std::string MissingUserID{"Missing user ID."};
    static const std::string IdMustBe0{"To create a user, you must set the ID to 0"};
    static const std::string InvalidUserRole{"Invalid userRole."};
    static const std::string InvalidEmailAddress{"Invalid email address."};
    static const std::string PasswordRejected{"Password was rejected. This maybe an old password."};
    static const std::string InvalidIPRanges{"Invalid IP range specifications."};
    static const std::string InvalidLOrderBy{"Invalid orderBy specification."};
    static const std::string NeedMobileNumber{"You must provide at least one validated phone number."};
    static const std::string BadMFAMethod{"MFA only supports sms or email."};
    static const std::string InvalidCredentials{"Invalid credentials (username/password)."};
    static const std::string InvalidPassword{"Password does not conform to basic password rules."};
    static const std::string UserPendingVerification{"User access denied pending email verification."};
    static const std::string PasswordMustBeChanged{"Password must be changed."};
    static const std::string UnrecognizedRequest{"Ill-formed request. Please consult documentation."};
    static const std::string MissingAuthenticationInformation{"Missing authentication information."};
    static const std::string InsufficientAccessRights{"Insufficient access rights to complete the operation."};
    static const std::string ExpiredToken{"Token has expired, user must login."};
}

