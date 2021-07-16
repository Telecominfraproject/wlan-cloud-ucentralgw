//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_SECURITYOBJECTS_H
#define UCENTRAL_RESTAPI_SECURITYOBJECTS_H

#include "Poco/JSON/Object.h"
#include "uCentralTypes.h"

namespace uCentral::SecurityObjects {

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
        UNKNOWN, ROOT, ADMIN, SUBSCRIBER, CSR, SYSTEM, SPECIAL
    };

    USER_ROLE UserTypeFromString(const std::string &U);
    std::string UserTypeToString(USER_ROLE U);

    struct NoteInfo {
		uint64_t created = std::time(nullptr);
		std::string createdBy;
		std::string note;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(Poco::JSON::Object::Ptr Obj);
	};
	typedef std::vector<NoteInfo>	NoteInfoVec;

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
		std::string userTypeProprietaryInfo;
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
		std::string vendor;
		std::string uri;
		std::string authenticationType;
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
		bool from_json(Poco::JSON::Object::Ptr Obj);
	};
	typedef std::vector<ProfileAction>	ProfileActionVec;

	struct SecurityProfile {
		uint64_t id;
		std::string name;
		std::string description;
		ProfileActionVec policy;
		std::string role;
		NoteInfoVec notes;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(Poco::JSON::Object::Ptr Obj);
	};
	typedef std::vector<SecurityProfile> SecurityProfileVec;

	struct SecurityProfileList {
		SecurityProfileVec profiles;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(Poco::JSON::Object::Ptr Obj);
	};
}

#endif //UCENTRAL_RESTAPI_SECURITYOBJECTS_H