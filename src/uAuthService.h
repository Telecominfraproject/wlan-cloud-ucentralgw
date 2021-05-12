//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UAUTHSERVICE_H
#define UCENTRAL_UAUTHSERVICE_H

#include "uSubSystemServer.h"

#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JWT/Signer.h"
#include "Poco/SHA2Engine.h"

#include "RESTAPI_objects.h"

namespace uCentral::Auth {

	enum ACCESS_TYPE {
		USERNAME,
		SERVER,
		CUSTOM
	};

	ACCESS_TYPE IntToAccessType(int C);
	int AccessTypeToInt(ACCESS_TYPE T);

    int Start();
    void Stop();
    bool IsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, struct uCentral::Objects::WebToken & UserInfo );
    bool Authorize( const std::string & UserName, const std::string & Password, uCentral::Objects::WebToken & ResultToken );
    void Logout(const std::string &token);

    class Service : public uSubSystemServer {
    public:

        Service() noexcept;

        friend int Start();
        friend void Stop();

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

        friend bool IsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, struct uCentral::Objects::WebToken & UserInfo );
        friend bool Authorize( const std::string & UserName, const std::string & Password, uCentral::Objects::WebToken & ResultToken );
        [[nodiscard]] std::string GenerateToken(const std::string & UserName, ACCESS_TYPE Type, int NumberOfDays);
		[[nodiscard]] bool ValidateToken(const std::string & Token, std::string & SessionToken, struct uCentral::Objects::WebToken & UserInfo  );
        friend void Logout(const std::string &token);

    private:
		static Service *instance_;
		std::map<std::string,uCentral::Objects::WebToken>   Tokens_;
		bool    			Secure_ = false ;
		std::string     	DefaultUserName_;
		std::string			DefaultPassword_;
		std::string     	Mechanism_;
		bool            	AutoProvisioning_ = false ;
		Poco::JWT::Signer	Signer_;
		Poco::SHA2Engine	SHA2_;

        int Start() override;
        void Stop() override;
        bool IsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, struct uCentral::Objects::WebToken & UserInfo );
        void CreateToken(const std::string & UserName, uCentral::Objects::WebToken & ResultToken, uCentral::Objects::AclTemplate & ACL);
        bool Authorize( const std::string & UserName, const std::string & Password, uCentral::Objects::WebToken & ResultToken );
        void Logout(const std::string &token);
    };

} // end of namespace

#endif //UCENTRAL_UAUTHSERVICE_H
