//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <string>
#include <type_traits>
#include "framework/OpenWifiTypes.h"
#include "Poco/JSON/Object.h"
#include "Poco/Data/LOB.h"
#include "Poco/Data/LOBStream.h"
#include "framework/utils.h"

namespace OpenWifi {
    uint64_t Now();
    namespace SecurityObjects {
        
        typedef std::string USER_ID_TYPE;

        struct AclTemplate {
            bool Read_ = true;
            bool ReadWrite_ = true;
            bool ReadWriteCreate_ = true;
            bool Delete_ = true;
            bool PortalLogin_ = true;

            AclTemplate()  noexcept = default;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        static_assert( std::is_nothrow_move_constructible_v<AclTemplate> );

        struct WebToken {
            std::string access_token_;
            std::string refresh_token_;
            std::string id_token_;
            std::string token_type_;
            std::string username_;
            bool userMustChangePassword=false;
            uint64_t errorCode=0;
            uint64_t expires_in_=0;
            uint64_t idle_timeout_=0;
            AclTemplate acl_template_;
            uint64_t created_=0;
            uint64_t lastRefresh_=0;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        enum USER_ROLE {
            UNKNOWN, ROOT, ADMIN, SUBSCRIBER, CSR, SYSTEM, INSTALLER, NOC, ACCOUNTING, PARTNER
        };

        USER_ROLE UserTypeFromString(const std::string &U);
        std::string UserTypeToString(USER_ROLE U);

        struct NoteInfo {
            uint64_t    created=0; // = Utils::Now();
            std::string createdBy;
            std::string note;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };
        typedef std::vector<NoteInfo>	NoteInfoVec;

        struct MobilePhoneNumber {
            std::string number;
            bool verified = false;
            bool primary = false;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct MfaAuthInfo {
            bool enabled = false;
            std::string method;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct UserLoginLoginExtensions {
            std::vector<MobilePhoneNumber>  mobiles;
            struct MfaAuthInfo              mfa;
            std::string                     authenticatorSecret;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct MFAChallengeRequest {
            std::string uuid;
            std::string question;
            std::string method;
            uint64_t    created = Utils::Now();

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct MFAChallengeResponse {
            std::string uuid;
            std::string answer;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct UserInfo {
            std::string id;
            std::string name;
            std::string description;
            std::string avatar;
            std::string email;
            bool        validated = false;
            std::string validationEmail;
            uint64_t    validationDate = 0;
            uint64_t    creationDate = 0;
            std::string validationURI;
            bool        changePassword = false;
            uint64_t    lastLogin = 0;
            std::string currentLoginURI;
            uint64_t    lastPasswordChange = 0;
            uint64_t    lastEmailCheck = 0;
            bool        waitingForEmailCheck = false;
            std::string locale;
            NoteInfoVec notes;
            std::string location;
            std::string owner;
            bool        suspended = false;
            bool        blackListed = false;
            USER_ROLE   userRole;
            UserLoginLoginExtensions userTypeProprietaryInfo;
            std::string securityPolicy;
            uint64_t    securityPolicyChange = 0 ;
            std::string currentPassword;
            OpenWifi::Types::StringVec lastPasswords;
            std::string oauthType;
            std::string oauthUserInfo;
            uint64_t    modified;
            std::string signingUp;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };
        typedef std::vector<UserInfo>   UserInfoVec;

        struct UserInfoList {
            std::vector<UserInfo>   users;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        // bool append_from_json(Poco::JSON::Object::Ptr Obj, const UserInfo &UInfo, NoteInfoVec & Notes);
        bool MergeNotes(Poco::JSON::Object::Ptr Obj, const UserInfo &UInfo, NoteInfoVec & Notes);
        bool MergeNotes(const NoteInfoVec & NewNotes, const UserInfo &UInfo, NoteInfoVec & ExistingNotes);

        struct InternalServiceInfo {
            std::string privateURI;
            std::string publicURI;
            std::string token;
            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };
        typedef std::vector<InternalServiceInfo>	InternalServiceInfoVec;

        struct InternalSystemServices {
            std::string key;
            std::string version;
            InternalServiceInfoVec services;
            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct SystemEndpoint {
            std::string type;
            uint64_t 	id = 0;
            std::string vendor{"OpenWiFi"};
            std::string uri;
            std::string authenticationType{"internal_v1"};
            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };
        typedef std::vector<SystemEndpoint> SystemEndpointVec;

        struct SystemEndpointList {
            SystemEndpointVec	endpoints;
            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct UserInfoAndPolicy {
            WebToken	webtoken;
            UserInfo	userinfo;
            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };
        typedef std::map<std::string,SecurityObjects::UserInfoAndPolicy>	UserInfoCache;

        enum ResourceAccessType {
            NONE,
            READ,
            MODIFY,
            DELETE,
            CREATE,
            TEST,
            MOVE
        };

        ResourceAccessType ResourceAccessTypeFromString(const std::string &s);
        std::string ResourceAccessTypeToString(const ResourceAccessType & T);

        struct ProfileAction {
            std::string resource;
            ResourceAccessType access;
            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };
        typedef std::vector<ProfileAction>	ProfileActionVec;

        struct SecurityProfile {
            uint64_t id=0;
            std::string name;
            std::string description;
            ProfileActionVec policy;
            std::string role;
            NoteInfoVec notes;
            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };
        typedef std::vector<SecurityProfile> SecurityProfileVec;

        struct SecurityProfileList {
            SecurityProfileVec profiles;
            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        enum LinkActions {
            FORGOT_PASSWORD=1,
            VERIFY_EMAIL,
            SUB_FORGOT_PASSWORD,
            SUB_VERIFY_EMAIL,
            SUB_SIGNUP,
            EMAIL_INVITATION
        };

        struct ActionLink {
            std::string         id;
            uint64_t            action;
            std::string         userId;
            std::string         actionTemplate;
            Types::StringPairVec variables;
            std::string         locale;
            std::string         message;
            uint64_t            sent=0;
            uint64_t            created=Utils::Now();
            uint64_t            expires=0;
            uint64_t            completed=0;
            uint64_t            canceled=0;
            bool                userAction=true;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct Preferences {
            std::string                         id;
            uint64_t                            modified;
            Types::StringPairVec                data;
            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct SubMfaConfig {
            std::string                         id;
            std::string                         type;
            std::string                         sms;
            std::string                         email;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct Token {
            std::string         token;
            std::string         refreshToken;
            std::string         tokenType;
            std::string         userName;
            uint64_t            created=0;
            uint64_t            expires=0;
            uint64_t            idleTimeout=0;
            uint64_t            revocationDate=0;
            uint64_t            lastRefresh=0;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct Avatar {
            std::string             id;
            std::string             type;
            uint64_t                created=0;
            std::string             name;
            Poco::Data::BLOB        avatar;
        };

        struct LoginRecordInfo {
            std::string sessionId;
            std::string userId;
            std::string email;
            uint64_t    login=0;
            uint64_t    logout=0;

            void to_json(Poco::JSON::Object &Obj) const;
        };

        struct ApiKeyAccessRight {
            std::string     service;
            std::string     access;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct ApiKeyAccessRightList {
            std::vector<ApiKeyAccessRight>      acls;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct ApiKeyEntry {
            Types::UUID_t           id;
            Types::UUID_t           userUuid;
            std::string             name;
            std::string             description;
            std::string             apiKey;
            std::string             salt;
            std::uint64_t           created;
            std::uint64_t           expiresOn=0;
            ApiKeyAccessRightList   rights;
            std::uint64_t           lastUse=0;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

        struct ApiKeyEntryList {
            std::vector<ApiKeyEntry>    apiKeys;

            void to_json(Poco::JSON::Object &Obj) const;
            bool from_json(const Poco::JSON::Object::Ptr &Obj);
        };

    }
}
