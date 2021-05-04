//
// Created by stephane bourque on 2021-03-04.
//
#include <random>
#include <ctime>

#include "uAuthService.h"
#include "uCentral.h"
#include "RESTAPI_handler.h"

namespace uCentral::Auth {
    Service *Service::instance_ = nullptr;

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
		DE_ = std::make_unique<Poco::Crypto::DigestEngine>(E);
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

    bool Service::IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string & SessionToken, struct WebToken & UserInfo  )
    {
        if(!Secure_)
            return true;

        auto Authorization = Request.get("Authorization","");

        if(!Authorization.empty() && Authorization.substr(0,6) == "Bearer")
        {
            auto RequestToken = Authorization.substr(7);
            std::lock_guard<std::mutex> guard(Mutex_);

            auto Token = Tokens_.find(RequestToken);

            if( Token == Tokens_.end() )
                return false;

            if((Token->second.created_ + Token->second.expires_in_) > time(nullptr)) {
                SessionToken = RequestToken;
				UserInfo = Token->second ;
                return true;
            }
            Tokens_.erase(Token);
            return false;
        } else {
			auto ApiToken = Request.get("X-API-KEY ", "");
			if (!ApiToken.empty()) {
				std::vector<unsigned char> M;
				for (const auto &i : ApiToken)
					M.push_back(i);
				auto Hash = DE_->digestToHex(M);
				return false;
			} else {
				return false;
			}
		}
    }

    void Service::Logout(const std::string &token) {
        std::lock_guard<std::mutex> guard(Mutex_);

        Tokens_.erase(token);
    }

    std::string Service::GenerateToken() {
        static char buf[]={"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRESTUVWXYZ"};

        std::string Res;
        std::default_random_engine generator(time(nullptr) * (int64_t )(uCentral::Auth::Service::instance_) );
        std::uniform_int_distribution<int> distribution(0,time(nullptr));

        for(auto i=0;i<sizeof(buf);i++)
            Res += buf[distribution(generator) % (sizeof(buf)-1)];

        return Res;
    }

    void Service::CreateToken(const std::string & UserName, WebToken & ResultToken)
    {
        std::lock_guard<std::mutex> guard(Mutex_);

        ResultToken.acl_template_.PortalLogin_ = true ;
        ResultToken.acl_template_.Delete_ = true ;
        ResultToken.acl_template_.ReadWriteCreate_ = true ;
        ResultToken.acl_template_.ReadWrite_ = true ;
        ResultToken.acl_template_.Read_ = true ;

        ResultToken.expires_in_ = 30 * 24 * 60 ;
        ResultToken.idle_timeout_ = 5 * 60;
        ResultToken.token_type_ = "Bearer";
        ResultToken.access_token_ = GenerateToken();
        ResultToken.id_token_ = GenerateToken();
        ResultToken.refresh_token_ = GenerateToken();
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
