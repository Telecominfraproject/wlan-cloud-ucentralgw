//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_SECURITYOBJECTS_H
#define UCENTRAL_RESTAPI_SECURITYOBJECTS_H

#include "framework/OpenWifiTypes.h"
#include "Poco/JSON/Object.h"

namespace OpenWifi::SecurityObjects {

	struct AclTemplate {
		bool Read_ = true;
		bool ReadWrite_ = true;
		bool ReadWriteCreate_ = true;
		bool Delete_ = true;
		bool PortalLogin_ = true;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);	};

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

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

    enum USER_ROLE {
        UNKNOWN, ROOT, ADMIN, SUBSCRIBER, CSR, SYSTEM, INSTALLER, NOC, ACCOUNTING
    };

    USER_ROLE UserTypeFromString(const std::string &U);
    std::string UserTypeToString(USER_ROLE U);

    struct NoteInfo {
		uint64_t created = std::time(nullptr);
		std::string createdBy;
		std::string note;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(Poco::JSON::Object::Ptr &Obj);
	};
	typedef std::vector<NoteInfo>	NoteInfoVec;

	struct MobilePhoneNumber {
	    std::string number;
	    bool verified = false;
	    bool primary = false;

	    void to_json(Poco::JSON::Object &Obj) const;
	    bool from_json(Poco::JSON::Object::Ptr &Obj);
	};

	struct MfaAuthInfo {
	    bool enabled = false;
	    std::string method;

	    void to_json(Poco::JSON::Object &Obj) const;
	    bool from_json(Poco::JSON::Object::Ptr &Obj);
	};

	struct UserLoginLoginExtensions {
	    std::vector<MobilePhoneNumber>  mobiles;
	    struct MfaAuthInfo mfa;

	    void to_json(Poco::JSON::Object &Obj) const;
	    bool from_json(Poco::JSON::Object::Ptr &Obj);
	};

	struct MFAChallengeRequest {
	    std::string uuid;
	    std::string question;
	    std::string method;
	    uint64_t    created = std::time(nullptr);

	    void to_json(Poco::JSON::Object &Obj) const;
	    bool from_json(Poco::JSON::Object::Ptr &Obj);
	};

    struct MFAChallengeResponse {
        std::string uuid;
        std::string answer;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(Poco::JSON::Object::Ptr &Obj);
    };

	struct UserInfo {
        std::string Id;
		std::string name;
		std::string description;
		std::string avatar;
		std::string email;
		bool validated = false;
		std::string validationEmail;
		uint64_t validationDate = 0;
		uint64_t creationDate = 0;
		std::string validationURI;
		bool changePassword = false;
		uint64_t lastLogin = 0;
		std::string currentLoginURI;
		uint64_t lastPasswordChange = 0;
		uint64_t lastEmailCheck = 0;
		bool waitingForEmailCheck = false;
		std::string locale;
		NoteInfoVec notes;
		std::string location;
		std::string owner;
		bool suspended = false;
		bool blackListed = false;
        USER_ROLE userRole;
        UserLoginLoginExtensions userTypeProprietaryInfo;
		std::string securityPolicy;
		uint64_t securityPolicyChange = 0 ;
		std::string currentPassword;
		Types::StringVec lastPasswords;
		std::string oauthType;
		std::string oauthUserInfo;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};
	typedef std::vector<UserInfo>   UserInfoVec;

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
		bool from_json(Poco::JSON::Object::Ptr &Obj);
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
		bool from_json(Poco::JSON::Object::Ptr &Obj);
	};
	typedef std::vector<SecurityProfile> SecurityProfileVec;

	struct SecurityProfileList {
		SecurityProfileVec profiles;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(Poco::JSON::Object::Ptr &Obj);
	};

	enum LinkActions {
	    FORGOT_PASSWORD=1,
	    VERIFY_EMAIL
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
	    uint64_t            created=std::time(nullptr);
	    uint64_t            expires=0;
	    uint64_t            completed=0;
	    uint64_t            canceled=0;

        void to_json(Poco::JSON::Object &Obj) const;
	    bool from_json(Poco::JSON::Object::Ptr &Obj);
	};

	struct Preferences {
	    std::string                         id;
	    uint64_t                            modified;
	    Types::StringPairVec                data;
	    void to_json(Poco::JSON::Object &Obj) const;
	    bool from_json(Poco::JSON::Object::Ptr &Obj);
	};
}

#endif //UCENTRAL_RESTAPI_SECURITYOBJECTS_H