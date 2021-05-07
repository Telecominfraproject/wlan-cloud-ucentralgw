//
// Created by stephane bourque on 2021-03-04.
//
#include <random>
#include <ctime>

#include "uAuthService.h"
#include "uCentral.h"
#include "RESTAPI_handler.h"
#include "Poco/Net/OAuth20Credentials.h"
#include "Poco/JWT/Token.h"
#include "Poco/JWT/Signer.h"

namespace uCentral::Auth {
    Service *Service::instance_ = nullptr;

	ACCESS_TYPE IntToAccessType(int C) {
		switch (C) {
		case 1: return USERNAME;
		case 2: return SERVER;
		case 3: return CUSTOM;
		}
		return USERNAME;
	}

	int AccessTypeToInt(ACCESS_TYPE T) {
		switch (T) {
		case USERNAME: return 1;
		case SERVER: return 2;
		case CUSTOM: return 3;
		}
		return 1;	// some compilers complain...
	}

    void AclTemplate::to_JSON(Poco::JSON::Object &Obj) const {
        Obj.set("Read",Read_);
        Obj.set("ReadWrite",ReadWrite_);
        Obj.set("ReadWriteCreate",ReadWriteCreate_);
        Obj.set("Delete",Delete_);
        Obj.set("PortalLogin",PortalLogin_);
    }

    void WebToken::to_JSON(Poco::JSON::Object & Obj) const {
        Poco::JSON::Object  AclTemplateObj;
		acl_template_.to_JSON(AclTemplateObj);
		Obj.set("access_token",access_token_);
		Obj.set("refresh_token",refresh_token_);
		Obj.set("token_type",token_type_);
		Obj.set("expires_in",expires_in_);
		Obj.set("idle_timeout",idle_timeout_);
		Obj.set("created",RESTAPIHandler::to_RFC3339(created_));
		Obj.set("username",username_);
		Obj.set("aclTemplate",AclTemplateObj);
    }

    Service::Service() noexcept:
            SubSystemServer("Authentication", "AUTH-SVR", "authentication")
    {
		std::string E{"SHA512"};
    }

    int Start() {
        return uCentral::Auth::Service::instance()->Start();
    }

    void Stop() {
        uCentral::Auth::Service::instance()->Stop();
    }

    bool IsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, struct WebToken & UserInfo ) {
        return uCentral::Auth::Service::instance()->IsAuthorized(Request,SessionToken, UserInfo);
    }

    bool Authorize( const std::string & UserName, const std::string & Password, WebToken & ResultToken ) {
        return uCentral::Auth::Service::instance()->Authorize(UserName,Password,ResultToken);
    }

    void Logout(const std::string &Token) {
        uCentral::Auth::Service::instance()->Logout(Token);
    }

    int Service::Start() {
		Signer_.setRSAKey(uCentral::instance()->Key());
		Signer_.addAllAlgorithms();
		Logger_.notice("Starting...");
        Secure_ = uCentral::ServiceConfig::getBool(SubSystemConfigPrefix_+".enabled",true);
        DefaultPassword_ = uCentral::ServiceConfig::getString(SubSystemConfigPrefix_+".default.password","");
        DefaultUserName_ = uCentral::ServiceConfig::getString(SubSystemConfigPrefix_+".default.username","");
        Mechanism_ = uCentral::ServiceConfig::getString(SubSystemConfigPrefix_+".service.type","internal");
        return 0;
    }

    void Service::Stop() {
		Logger_.notice("Stopping...");
    }

	bool Service::TryRestoringToken(const std::string & Token) {
		return false;
	}

	bool Service::IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string & SessionToken, struct WebToken & UserInfo  )
    {
        if(!Secure_)
            return true;

		std::lock_guard<SubMutex> guard(Mutex_);

		std::string CallToken;

		try {
			Poco::Net::OAuth20Credentials Auth(Request);

			if (Auth.getScheme() == "Bearer") {
				CallToken = Auth.getBearerToken();
			}
		} catch(const Poco::Exception &E) {
		}

		if(CallToken.empty())
			CallToken = Request.get("X-API-KEY ", "");

		if(CallToken.empty())
			return false;

		auto Client = Tokens_.find(CallToken);

		if( Client == Tokens_.end() )
			return ValidateToken(CallToken);

		if((Client->second.created_ + Client->second.expires_in_) > time(nullptr)) {
			SessionToken = CallToken;
			UserInfo = Client->second ;
			return true;
		}

		Tokens_.erase(CallToken);
		return false;
    }

    void Service::Logout(const std::string &token) {
        std::lock_guard<SubMutex> 	Lock(Mutex_);

        Tokens_.erase(token);
    }

    std::string Service::GenerateToken(const std::string & Identity, ACCESS_TYPE Type, int NumberOfDays) {
		std::lock_guard<SubMutex>	Lock(Mutex_);

		Poco::JWT::Token	T;

		T.setType("JWT");
		switch(Type) {
			case USERNAME:	T.setSubject("usertoken"); break;
			case SERVER: 	T.setSubject("servertoken"); break;
			case CUSTOM:	T.setSubject("customtoken"); break;
		}

		T.payload().set("identity", Identity);
		T.setIssuedAt(Poco::Timestamp());
		T.setExpiration(Poco::Timestamp() + Poco::Timespan(NumberOfDays,0,0,0,0));
		std::string JWT = Signer_.sign(T,Poco::JWT::Signer::ALGO_RS256);

		return JWT;
    }

	bool Service::ValidateToken(const std::string & Token) {
		std::lock_guard<SubMutex>		Lock(Mutex_);
		Poco::JWT::Token				DecryptedToken;

		if(Signer_.tryVerify(Token,DecryptedToken)) {
			/*
			if(Expires>Poco::Timestamp() && DecryptedToken.getSubject()=="usertoken")
			{
				UserName = DecryptedToken.payload().get("username").toString();
				return true;
			}*/
		}
		return false;
	}

    void Service::CreateToken(const std::string & UserName, WebToken & ResultToken)
    {
        std::lock_guard<SubMutex> Lock(Mutex_);

		std::string Token = GenerateToken(UserName,USERNAME,30);

        ResultToken.acl_template_.PortalLogin_ = true ;
        ResultToken.acl_template_.Delete_ = true ;
        ResultToken.acl_template_.ReadWriteCreate_ = true ;
        ResultToken.acl_template_.ReadWrite_ = true ;
        ResultToken.acl_template_.Read_ = true ;

        ResultToken.expires_in_ = 30 * 24 * 60 ;
        ResultToken.idle_timeout_ = 5 * 60;
        ResultToken.token_type_ = "Bearer";
        ResultToken.access_token_ = Token;
        ResultToken.id_token_ = Token;
        ResultToken.refresh_token_ = Token;
        ResultToken.created_ = time(nullptr);
        ResultToken.username_ = UserName;

        Tokens_[ResultToken.access_token_] = ResultToken;
    }

    bool Service::Authorize( const std::string & UserName, const std::string & Password, WebToken & ResultToken )
    {
        if(Mechanism_=="internal")
        {
            if(((UserName == DefaultUserName_) && (Password == DefaultPassword_)) || !Secure_)
            {
                CreateToken(UserName,ResultToken);
                return true;
            }
        }
        return false;
    }

};  // end of namespace
