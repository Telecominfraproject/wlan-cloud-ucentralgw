//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"

#include "framework/RESTAPI_utils.h"
#include "RESTAPI_SecurityObjects.h"

using OpenWifi::RESTAPI_utils::field_to_json;
using OpenWifi::RESTAPI_utils::field_from_json;

namespace OpenWifi::SecurityObjects {

	void AclTemplate::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"Read",Read_);
		field_to_json(Obj,"ReadWrite",ReadWrite_);
		field_to_json(Obj,"ReadWriteCreate",ReadWriteCreate_);
		field_to_json(Obj,"Delete",Delete_);
		field_to_json(Obj,"PortalLogin",PortalLogin_);
	}

	ResourceAccessType ResourceAccessTypeFromString(const std::string &s) {
		if(!Poco::icompare(s,"READ")) return READ;
		if(!Poco::icompare(s,"MODIFY")) return MODIFY;
		if(!Poco::icompare(s,"DELETE")) return DELETE;
		if(!Poco::icompare(s,"CREATE")) return CREATE;
		if(!Poco::icompare(s,"TEST")) return TEST;
		if(!Poco::icompare(s,"MOVE")) return MOVE;
		return NONE;
	}

	std::string ResourceAccessTypeToString(const ResourceAccessType & T) {
		switch(T) {
		case READ: return "READ";
		case MODIFY: return "MODIFY";
		case DELETE: return "DELETE";
		case CREATE: return "CREATE";
		case TEST: return "TEST";
		case MOVE: return "MOVE";
		default: return "NONE";
		}
	}

    USER_ROLE UserTypeFromString(const std::string &U) {
        if (!Poco::icompare(U,"root"))
            return ROOT;
        else if (!Poco::icompare(U,"admin"))
            return ADMIN;
        else if (!Poco::icompare(U,"subscriber"))
            return SUBSCRIBER;
        else if (!Poco::icompare(U,"partner"))
            return PARTNER;
        else if (!Poco::icompare(U,"csr"))
            return CSR;
        else if (!Poco::icompare(U, "system"))
            return SYSTEM;
        else if (!Poco::icompare(U, "installer"))
            return INSTALLER;
        else if (!Poco::icompare(U, "noc"))
            return NOC;
        else if (!Poco::icompare(U, "accounting"))
            return ACCOUNTING;
        return UNKNOWN;
    }

    std::string UserTypeToString(USER_ROLE U) {
        switch(U) {
            case ROOT: return "root";
            case ADMIN: return "admin";
            case SUBSCRIBER: return "subscriber";
            case PARTNER: return "partner";
            case CSR: return "csr";
            case SYSTEM: return "system";
            case INSTALLER: return "installer";
            case NOC: return "noc";
            case ACCOUNTING: return "accounting";
            case UNKNOWN:
            default:
                return "unknown";
        }
    }

	bool AclTemplate::from_json(const Poco::JSON::Object::Ptr &Obj)  {
		try {
			field_from_json(Obj, "Read", Read_);
			field_from_json(Obj, "ReadWrite", ReadWrite_);
			field_from_json(Obj, "ReadWriteCreate", ReadWriteCreate_);
			field_from_json(Obj, "Delete", Delete_);
			field_from_json(Obj, "PortalLogin", PortalLogin_);
			return true;
		} catch(...) {
            std::cout << "Cannot parse: AclTemplate" << std::endl;
		}
		return false;
	}

	void WebToken::to_json(Poco::JSON::Object & Obj) const {
		Poco::JSON::Object  AclTemplateObj;
		acl_template_.to_json(AclTemplateObj);
		field_to_json(Obj,"access_token",access_token_);
		field_to_json(Obj,"refresh_token",refresh_token_);
		field_to_json(Obj,"token_type",token_type_);
		field_to_json(Obj,"expires_in",expires_in_);
		field_to_json(Obj,"idle_timeout",idle_timeout_);
		field_to_json(Obj,"created",created_);
		field_to_json(Obj,"username",username_);
        field_to_json(Obj,"userMustChangePassword",userMustChangePassword);
        field_to_json(Obj,"errorCode", errorCode);
		Obj.set("aclTemplate",AclTemplateObj);
        field_to_json(Obj,"errorCode", errorCode);
        field_to_json(Obj,"lastRefresh", lastRefresh_);
	}

	bool WebToken::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			if (Obj->isObject("aclTemplate")) {
				Poco::JSON::Object::Ptr AclTemplate = Obj->getObject("aclTemplate");
				acl_template_.from_json(AclTemplate);
			}
			field_from_json(Obj, "access_token", access_token_);
			field_from_json(Obj, "refresh_token", refresh_token_);
			field_from_json(Obj, "token_type", token_type_);
			field_from_json(Obj, "expires_in", expires_in_);
			field_from_json(Obj, "idle_timeout", idle_timeout_);
			field_from_json(Obj, "created", created_);
			field_from_json(Obj, "username", username_);
            field_from_json(Obj, "userMustChangePassword",userMustChangePassword);
            field_from_json(Obj,"lastRefresh", lastRefresh_);
			return true;
		} catch (...) {
            std::cout << "Cannot parse: WebToken" << std::endl;
		}
		return false;
	}

	void MobilePhoneNumber::to_json(Poco::JSON::Object &Obj) const {
	    field_to_json(Obj,"number", number);
	    field_to_json(Obj,"verified", verified);
	    field_to_json(Obj,"primary", primary);
	}

	bool MobilePhoneNumber::from_json(const Poco::JSON::Object::Ptr &Obj) {
	    try {
	        field_from_json(Obj,"number",number);
	        field_from_json(Obj,"verified",verified);
	        field_from_json(Obj,"primary",primary);
	        return true;
	    } catch (...) {
            std::cout << "Cannot parse: MobilePhoneNumber" << std::endl;
	    }
	    return false;
	};

	void MfaAuthInfo::to_json(Poco::JSON::Object &Obj) const {
	    field_to_json(Obj,"enabled", enabled);
	    field_to_json(Obj,"method", method);
	}

	bool MfaAuthInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
	    try {
	        field_from_json(Obj,"enabled",enabled);
	        field_from_json(Obj,"method",method);
	        return true;
	    } catch (...) {
            std::cout << "Cannot parse: MfaAuthInfo" << std::endl;
	    }
	    return false;
	}

	void UserLoginLoginExtensions::to_json(Poco::JSON::Object &Obj) const {
	    field_to_json(Obj, "mobiles", mobiles);
	    field_to_json(Obj, "mfa", mfa);
        field_to_json(Obj, "authenticatorSecret", authenticatorSecret);
	}

	bool UserLoginLoginExtensions::from_json(const Poco::JSON::Object::Ptr &Obj) {
	    try {
	        field_from_json(Obj, "mobiles",mobiles);
	        field_from_json(Obj, "mfa",mfa);
            field_from_json(Obj, "authenticatorSecret", authenticatorSecret);
	        return true;
	    } catch (...) {
            std::cout << "Cannot parse: UserLoginLoginExtensions" << std::endl;
	    }
	    return false;
	}

    void MFAChallengeRequest::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "uuid", uuid);
        field_to_json(Obj, "question", question);
        field_to_json(Obj, "created", created);
        field_to_json(Obj, "method", method);
    }

    bool MFAChallengeRequest::from_json(const Poco::JSON::Object::Ptr &Obj) {
	    try {
	        field_from_json(Obj,"uuid",uuid);
	        field_from_json(Obj,"question",question);
	        field_from_json(Obj,"created",created);
	        field_from_json(Obj,"method",method);
	        return true;
	    } catch (...) {
            std::cout << "Cannot parse: MFAChallengeRequest" << std::endl;
	    }
	    return false;
	};

    void MFAChallengeResponse::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "uuid", uuid);
        field_to_json(Obj, "answer", answer);
    }

    bool MFAChallengeResponse::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"uuid",uuid);
            field_from_json(Obj,"answer",answer);
            return true;
        } catch (...) {
            std::cout << "Cannot parse: MFAChallengeResponse" << std::endl;
        }
        return false;

    }

    void UserInfo::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"id",id);
		field_to_json(Obj,"name",name);
		field_to_json(Obj,"description", description);
		field_to_json(Obj,"avatar", avatar);
		field_to_json(Obj,"email", email);
		field_to_json(Obj,"validated", validated);
		field_to_json(Obj,"validationEmail", validationEmail);
		field_to_json(Obj,"validationDate", validationDate);
		field_to_json(Obj,"creationDate", creationDate);
		field_to_json(Obj,"validationURI", validationURI);
		field_to_json(Obj,"changePassword", changePassword);
		field_to_json(Obj,"lastLogin", lastLogin);
		field_to_json(Obj,"currentLoginURI", currentLoginURI);
		field_to_json(Obj,"lastPasswordChange", lastPasswordChange);
		field_to_json(Obj,"lastEmailCheck", lastEmailCheck);
		field_to_json(Obj,"waitingForEmailCheck", waitingForEmailCheck);
		field_to_json(Obj,"locale", locale);
		field_to_json(Obj,"notes", notes);
		field_to_json(Obj,"location", location);
		field_to_json(Obj,"owner", owner);
		field_to_json(Obj,"suspended", suspended);
		field_to_json(Obj,"blackListed", blackListed);
		field_to_json<USER_ROLE>(Obj,"userRole", userRole, UserTypeToString);
		field_to_json(Obj,"userTypeProprietaryInfo", userTypeProprietaryInfo);
		field_to_json(Obj,"securityPolicy", securityPolicy);
		field_to_json(Obj,"securityPolicyChange", securityPolicyChange);
		field_to_json(Obj,"currentPassword",currentPassword);
		field_to_json(Obj,"lastPasswords",lastPasswords);
		field_to_json(Obj,"oauthType",oauthType);
		field_to_json(Obj,"oauthUserInfo",oauthUserInfo);
        field_to_json(Obj,"modified",modified);
        field_to_json(Obj,"signingUp",signingUp);
    };

    bool UserInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
			field_from_json(Obj,"id",id);
			field_from_json(Obj,"name",name);
			field_from_json(Obj,"description",description);
			field_from_json(Obj,"avatar",avatar);
			field_from_json(Obj,"email",email);
			field_from_json(Obj,"validationEmail",validationEmail);
			field_from_json(Obj,"validationURI",validationURI);
			field_from_json(Obj,"currentLoginURI",currentLoginURI);
			field_from_json(Obj,"locale",locale);
			field_from_json(Obj,"notes",notes);
            field_from_json(Obj,"location", location);
            field_from_json(Obj,"owner", owner);
			field_from_json<USER_ROLE>(Obj,"userRole",userRole, UserTypeFromString);
			field_from_json(Obj,"securityPolicy",securityPolicy);
			field_from_json(Obj,"userTypeProprietaryInfo",userTypeProprietaryInfo);
			field_from_json(Obj,"validationDate",validationDate);
			field_from_json(Obj,"creationDate",creationDate);
			field_from_json(Obj,"lastLogin",lastLogin);
			field_from_json(Obj,"lastPasswordChange",lastPasswordChange);
			field_from_json(Obj,"lastEmailCheck",lastEmailCheck);
			field_from_json(Obj,"securityPolicyChange",securityPolicyChange);
			field_from_json(Obj,"validated",validated);
			field_from_json(Obj,"changePassword",changePassword);
			field_from_json(Obj,"waitingForEmailCheck",waitingForEmailCheck);
			field_from_json(Obj,"suspended",suspended);
			field_from_json(Obj,"blackListed",blackListed);
			field_from_json(Obj,"currentPassword",currentPassword);
			field_from_json(Obj,"lastPasswords",lastPasswords);
			field_from_json(Obj,"oauthType",oauthType);
			field_from_json(Obj,"oauthUserInfo",oauthUserInfo);
            field_from_json(Obj,"modified",modified);
            field_from_json(Obj,"signingUp",signingUp);
            return true;
        } catch (const Poco::Exception &E) {
            std::cout << "Cannot parse: UserInfo" << std::endl;
        }
        return false;
    };

    void UserInfoList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"users",users);
    }

    bool UserInfoList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"users",users);
            return true;
        } catch (...) {
            std::cout << "Cannot parse: InternalServiceInfo" << std::endl;
        }
        return false;
    }

	void InternalServiceInfo::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"privateURI",privateURI);
		field_to_json(Obj,"publicURI",publicURI);
		field_to_json(Obj,"token",token);
	};

	bool InternalServiceInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"privateURI",privateURI);
			field_from_json(Obj,"publicURI",publicURI);
			field_from_json(Obj,"token",token);
			return true;
		} catch (...) {
            std::cout << "Cannot parse: InternalServiceInfo" << std::endl;
		}
		return false;
	};

	void InternalSystemServices::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"key",key);
		field_to_json(Obj,"version",version);
		field_to_json(Obj,"services",services);
	};

	bool InternalSystemServices::from_json(const Poco::JSON::Object::Ptr &Obj)  {
		try {
			field_from_json(Obj, "key", key);
			field_from_json(Obj, "version", version);
			field_from_json(Obj, "services", services);
			return true;
		} catch(...) {
            std::cout << "Cannot parse: InternalSystemServices" << std::endl;
		}
		return false;
	};

	void SystemEndpoint::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"type",type);
		field_to_json(Obj,"id",id);
		field_to_json(Obj,"vendor",vendor);
		field_to_json(Obj,"uri",uri);
		field_to_json(Obj,"authenticationType",authenticationType);
	};

	bool SystemEndpoint::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "type", type);
			field_from_json(Obj, "id", id);
			field_from_json(Obj, "vendor", vendor);
			field_from_json(Obj, "uri", uri);
			field_from_json(Obj, "authenticationType", authenticationType);
			return true;
		} catch (...) {
            std::cout << "Cannot parse: SystemEndpoint" << std::endl;
		}
		return false;
	};

	void SystemEndpointList::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"endpoints",endpoints);
	}

	bool SystemEndpointList::from_json(const Poco::JSON::Object::Ptr &Obj)  {
		try {
			field_from_json(Obj, "endpoints", endpoints);
			return true;
		} catch (...) {
            std::cout << "Cannot parse: SystemEndpointList" << std::endl;
		}
		return false;
	}

	void UserInfoAndPolicy::to_json(Poco::JSON::Object &Obj) const {
		Poco::JSON::Object	UI, TI;
		userinfo.to_json(UI);
		webtoken.to_json(TI);
		Obj.set("tokenInfo",TI);
		Obj.set("userInfo",UI);
	}

	bool UserInfoAndPolicy::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj, "tokenInfo", webtoken);
			field_from_json(Obj, "userInfo", userinfo);
			return true;
		} catch(...) {
            std::cout << "Cannot parse: UserInfoAndPolicy" << std::endl;
		}
		return false;
	}

	void NoteInfo::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"created", created);
		field_to_json(Obj,"createdBy", createdBy);
		field_to_json(Obj,"note", note);
	}

	bool NoteInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
            field_from_json(Obj,"created",created);
			field_from_json(Obj,"createdBy",createdBy);
			field_from_json(Obj,"note", note);
			return true;
		} catch(...) {
            std::cout << "Cannot parse: NoteInfo" << std::endl;
		}
		return false;
	}

    bool MergeNotes(Poco::JSON::Object::Ptr Obj, const UserInfo &UInfo, NoteInfoVec & Notes) {
	    try {
	        if(Obj->has("notes") && Obj->isArray("notes")) {
	            SecurityObjects::NoteInfoVec NIV;
	            NIV = RESTAPI_utils::to_object_array<SecurityObjects::NoteInfo>(Obj->get("notes").toString());
	            for(auto const &i:NIV) {
	                SecurityObjects::NoteInfo   ii{.created=(uint64_t)Utils::Now(), .createdBy=UInfo.email, .note=i.note};
	                Notes.push_back(ii);
	            }
	        }
	        return true;
	    } catch(...) {
            std::cout << "Cannot parse: MergeNotes" << std::endl;
	    }
	    return false;
	}

	bool MergeNotes(const NoteInfoVec & NewNotes, const UserInfo &UInfo, NoteInfoVec & ExistingNotes) {
	    for(auto const &i:NewNotes) {
	        SecurityObjects::NoteInfo   ii{.created=(uint64_t)Utils::Now(), .createdBy=UInfo.email, .note=i.note};
	        ExistingNotes.push_back(ii);
	    }
        return true;
	}

	void ProfileAction::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"resource", resource);
		field_to_json<ResourceAccessType>(Obj,"access", access, ResourceAccessTypeToString);
	}

	bool ProfileAction::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"resource",resource);
			field_from_json<ResourceAccessType>(Obj,"access",access,ResourceAccessTypeFromString );
			return true;
		} catch(...) {
            std::cout << "Cannot parse: ProfileAction" << std::endl;
		}
		return false;
	}

	void SecurityProfile::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj,"id", id);
		field_to_json(Obj,"name", name);
		field_to_json(Obj,"description", description);
		field_to_json(Obj,"policy", policy);
		field_to_json(Obj,"role", role);
		field_to_json(Obj,"notes", notes);
	}

	bool SecurityProfile::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"id",id);
			field_from_json(Obj,"name",name);
			field_from_json(Obj,"description",description);
			field_from_json(Obj,"policy",policy);
			field_from_json(Obj,"role",role);
			field_from_json(Obj,"notes",notes);
			return true;
		} catch(...) {
            std::cout << "Cannot parse: SecurityProfile" << std::endl;
		}
		return false;
	}

	void SecurityProfileList::to_json(Poco::JSON::Object &Obj) const {
		field_to_json(Obj, "profiles", profiles);
	}

	bool SecurityProfileList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			field_from_json(Obj,"profiles",profiles);
			return true;
		} catch(...) {
            std::cout << "Cannot parse: SecurityProfileList" << std::endl;
		}
		return false;
	}

    void ActionLink::to_json(Poco::JSON::Object &Obj) const {
	    field_to_json(Obj,"id",id);
	    field_to_json(Obj,"action",action);
	    field_to_json(Obj,"userId",userId);
	    field_to_json(Obj,"actionTemplate",actionTemplate);
	    field_to_json(Obj,"variables",variables);
	    field_to_json(Obj,"locale",locale);
	    field_to_json(Obj,"message",message);
	    field_to_json(Obj,"sent",sent);
	    field_to_json(Obj,"created",created);
	    field_to_json(Obj,"expires",expires);
	    field_to_json(Obj,"completed",completed);
	    field_to_json(Obj,"canceled",canceled);
        field_to_json(Obj,"userAction",userAction);
	}

    bool ActionLink::from_json(const Poco::JSON::Object::Ptr &Obj) {
	    try {
	        field_from_json(Obj,"id",id);
	        field_from_json(Obj,"action",action);
	        field_from_json(Obj,"userId",userId);
	        field_from_json(Obj,"actionTemplate",actionTemplate);
	        field_from_json(Obj,"variables",variables);
	        field_from_json(Obj,"locale",locale);
	        field_from_json(Obj,"message",message);
	        field_from_json(Obj,"sent",sent);
	        field_from_json(Obj,"created",created);
	        field_from_json(Obj,"expires",expires);
	        field_from_json(Obj,"completed",completed);
	        field_from_json(Obj,"canceled",canceled);
            field_from_json(Obj,"userAction",userAction);
	        return true;
	    } catch(...) {
            std::cout << "Cannot parse: ActionLink" << std::endl;
	    }
	    return false;
	}

    void Preferences::to_json(Poco::JSON::Object &Obj) const {
	    field_to_json(Obj,"id",id);
	    field_to_json(Obj,"modified",modified);
	    field_to_json(Obj,"data",data);
	}

    bool Preferences::from_json(const Poco::JSON::Object::Ptr &Obj) {
	    try {
	        field_from_json(Obj,"id",id);
	        field_from_json(Obj,"modified",modified);
	        field_from_json(Obj,"data",data);
	        return true;
	    } catch(...) {
            std::cout << "Cannot parse: Preferences" << std::endl;
	    }
	    return false;
	}

    void SubMfaConfig::to_json(Poco::JSON::Object &Obj) const {
	    field_to_json(Obj,"id",id);
	    field_to_json(Obj,"type",type);
	    field_to_json(Obj,"sms",sms);
	    field_to_json(Obj,"email",email);
	}

    bool SubMfaConfig::from_json(const Poco::JSON::Object::Ptr &Obj) {
	    try {
	        field_from_json(Obj,"id",id);
	        field_from_json(Obj,"type",type);
	        field_from_json(Obj,"sms",sms);
	        field_from_json(Obj,"email",email);
	        return true;
	    } catch(...) {
            std::cout << "Cannot parse: SubMfaConfig" << std::endl;
	    }
	    return false;
	}

    void Token::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"token",token);
        field_to_json(Obj,"refreshToken",refreshToken);
        field_to_json(Obj,"tokenType",tokenType);
        field_to_json(Obj,"userName",userName);
        field_to_json(Obj,"created",created);
        field_to_json(Obj,"expires",expires);
        field_to_json(Obj,"idleTimeout",idleTimeout);
        field_to_json(Obj,"revocationDate",revocationDate);
        field_to_json(Obj,"lastRefresh", lastRefresh);
    }

    bool Token::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"token",token);
            field_from_json(Obj,"refreshToken",refreshToken);
            field_from_json(Obj,"tokenType",tokenType);
            field_from_json(Obj,"userName",userName);
            field_from_json(Obj,"created",created);
            field_from_json(Obj,"expires",expires);
            field_from_json(Obj,"idleTimeout",idleTimeout);
            field_from_json(Obj,"revocationDate",revocationDate);
            field_from_json(Obj,"lastRefresh", lastRefresh);
            return true;
        } catch(...) {
            std::cout << "Cannot parse: Token" << std::endl;
        }
        return false;
    }

    void LoginRecordInfo::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"sessionId",sessionId);
        field_to_json(Obj,"userId",userId);
        field_to_json(Obj,"email",email);
        field_to_json(Obj,"login",login);
        field_to_json(Obj,"logout",logout);
    }

    void ApiKeyAccessRight::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "service", service);
        field_to_json(Obj, "access", access);
    }

    bool ApiKeyAccessRight::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "service", service);
            field_from_json(Obj, "access", access);
            return true;
        } catch(...) {
            std::cout << "Cannot parse: Token" << std::endl;
        }
        return false;
    }

    void ApiKeyAccessRightList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "acls", acls);
    }

    bool ApiKeyAccessRightList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "acls", acls);
            return true;
        } catch(...) {
            std::cout << "Cannot parse: Token" << std::endl;
        }
        return false;
    }

    void ApiKeyEntry::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "id", id);
        field_to_json(Obj, "userUuid", userUuid);
        field_to_json(Obj, "name", name);
        field_to_json(Obj, "apiKey", apiKey);
        field_to_json(Obj, "salt", salt);
        field_to_json(Obj, "description", description);
        field_to_json(Obj, "expiresOn", expiresOn);
        field_to_json(Obj, "rights", rights);
        field_to_json(Obj, "lastUse", lastUse);
    }

    bool ApiKeyEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "id", id);
            field_from_json(Obj, "userUuid", userUuid);
            field_from_json(Obj, "name", name);
            field_from_json(Obj, "apiKey", apiKey);
            field_from_json(Obj, "salt", salt);
            field_from_json(Obj, "description", description);
            field_from_json(Obj, "expiresOn", expiresOn);
            field_from_json(Obj, "rights", rights);
            field_from_json(Obj, "lastUse", lastUse);
            return true;
        } catch(...) {
            std::cout << "Cannot parse: Token" << std::endl;
        }
        return false;
    }

    void ApiKeyEntryList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "apiKeys", apiKeys);
    }

    bool ApiKeyEntryList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "apiKeys", apiKeys);
            return true;
        } catch(...) {
            std::cout << "Cannot parse: Token" << std::endl;
        }
        return false;
    }

}

