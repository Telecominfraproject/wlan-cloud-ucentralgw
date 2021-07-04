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
		uint64_t expires_in_;
		uint64_t idle_timeout_;
		AclTemplate acl_template_;
		uint64_t created_;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct UserInfo {
		uint64_t Id = 0;
		std::string name;
		std::string description;
		std::string avatar;
		std::string email;
		bool validated = false;
		std::string validationEmail;
		uint64_t validationDate = 0;
		uint64_t creationDate = 0;
		std::string validationURI;
		bool changePassword = true;
		uint64_t lastLogin = 0;
		std::string currentLoginURI;
		uint64_t lastPasswordChange = 0;
		uint64_t lastEmailCheck = 0;
		bool waitingForEmailCheck = false;
		std::string locale;
		std::string notes;
		std::string location;
		std::string owner;
		bool suspended = false;
		bool blackListed = false;
		std::string userRole;
		std::string userTypeProprietaryInfo;
		std::string securityPolicy;
		uint64_t securityPolicyChange;

		std::string currentPassword;
		Types::StringVec	lastPasswords;
		std::string oauthType;
		std::string oauthUserInfo;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct InternalServiceInfo {
		std::string privateURI;
		std::string publicURI;
		std::string token;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct InternalSystemServices {
		std::string key;
		std::string version;
		std::vector<InternalServiceInfo> services;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct SystemEndpoint {
		std::string type;
		uint64_t 	id;
		std::string vendor;
		std::string uri;
		std::string authenticationType;
		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct SystemEndpointList {
		std::vector<SystemEndpoint>	endpoints;
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

}

#endif //UCENTRAL_RESTAPI_SECURITYOBJECTS_H
