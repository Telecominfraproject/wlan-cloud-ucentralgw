//
// Created by stephane bourque on 2021-03-04.
//

#include "uAuthService.h"
#include "Poco/Net/OAuth20Credentials.h"
#include "uCentral.h"
#include <random>

namespace uCentral::Auth {
    Service *Service::instance_ = nullptr;

    Poco::JSON::Object AclTemplate::to_JSON() const {
        Poco::JSON::Object Obj;

        Obj.set("Read",Read_);
        Obj.set("ReadWrite",ReadWrite_);
        Obj.set("ReadWriteCreate",ReadWriteCreate_);
        Obj.set("Delete",Delete_);
        Obj.set("PortalLogin",PortalLogin_);

        return Obj;
    }

    Poco::JSON::Object WebToken::to_JSON() const {

        Poco::JSON::Object  AclTemplateObj = acl_template_.to_JSON();
        Poco::JSON::Object  WebTokenObj;

        WebTokenObj.set("access_token",access_token_);
        WebTokenObj.set("refresh_token",refresh_token_);
        WebTokenObj.set("token_type",token_type_);
        WebTokenObj.set("expires_in",expires_in_);
        WebTokenObj.set("idle_timeout",idle_timeout_);
        WebTokenObj.set("aclTemplate",AclTemplateObj);

        return WebTokenObj;
    }

    Service::Service() noexcept:
            SubSystemServer("Authentication", "AUTH-SVR", "authentication"),
            Secure_(true)
    {
    }

    int Start() {
        return uCentral::Auth::Service::instance()->Start();
    }

    void Stop() {
        uCentral::Auth::Service::instance()->Stop();
    }

    bool IsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken) {
        return uCentral::Auth::Service::instance()->IsAuthorized(Request,SessionToken);
    }

    void CreateToken(const std::string & UserName, WebToken & ResultToken) {
        uCentral::Auth::Service::instance()->CreateToken(UserName,ResultToken);
    }

    bool Authorize( const std::string & UserName, const std::string & Password, WebToken & ResultToken ) {
        return uCentral::Auth::Service::instance()->Authorize(UserName,Password,ResultToken);
    }

    void Logout(const std::string &Token) {
        uCentral::Auth::Service::instance()->Logout(Token);
    }

    int Service::Start() {
        Secure_ = uCentral::Daemon::instance().config().getBool(SubSystemConfigPrefix_+".enabled",true);
        DefaultPassword_ = uCentral::Daemon::instance().config().getString(SubSystemConfigPrefix_+".default.password","");
        DefaultUserName_ = uCentral::Daemon::instance().config().getString(SubSystemConfigPrefix_+".default.username","");
        Mechanism_ = uCentral::Daemon::instance().config().getString(SubSystemConfigPrefix_+".service.type","internal");
        return 0;
    }

    void Service::Stop() {
    }

    bool Service::IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string & SessionToken)
    {
        if(!Secure_)
            return true;

        auto Authorization = Request.get("Authorization","");

        if(Authorization.substr(0,6) == "Bearer")
        {
            auto RequestToken = Authorization.substr(7);
            std::lock_guard<std::mutex> guard(mutex_);

            auto Token = Tokens_.find(RequestToken);

            if( Token == Tokens_.end() )
                return false;

            SessionToken = RequestToken;

            return true;
        }

        return false;
    }

    void Service::Logout(const std::string &token) {
        std::lock_guard<std::mutex> guard(mutex_);

        Tokens_.erase(token);
    }

    std::string Service::GenerateToken() {
        static char buf[]={"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRESTUVWXYZ"};

        std::string Res;
        std::default_random_engine generator(time(nullptr));
        std::uniform_int_distribution<int> distribution(0,time(nullptr));

        for(auto i=0;i<sizeof(buf);i++)
            Res += buf[distribution(generator) % (sizeof(buf)-1)];

        return Res;
    }

    void Service::CreateToken(const std::string & UserName, WebToken & ResultToken)
    {
        std::lock_guard<std::mutex> guard(mutex_);

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
