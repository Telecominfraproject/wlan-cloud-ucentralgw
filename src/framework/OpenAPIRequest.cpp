//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
//

#include <iostream>

#include "OpenAPIRequest.h"

#include "Poco/Net/HTTPSClientSession.h"
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include "Utils.h"
#include "Daemon.h"

namespace OpenWifi {

	int OpenAPIRequestGet::Do(Poco::JSON::Object::Ptr &ResponseObject) {
		try {
		    auto Services = Daemon()->GetServices(Type_);
			for(auto const &Svc:Services) {
				Poco::URI	URI(Svc.PrivateEndPoint);
				Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());

				URI.setPath(EndPoint_);
				for (const auto &qp : QueryData_)
					URI.addQueryParameter(qp.first, qp.second);

				std::string Path(URI.getPathAndQuery());
				Session.setTimeout(Poco::Timespan(msTimeout_/1000, msTimeout_ % 1000));

				Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_GET,
											   Path,
											   Poco::Net::HTTPMessage::HTTP_1_1);
				Request.add("X-API-KEY", Svc.AccessKey);
				Request.add("X-INTERNAL-NAME", Daemon()->PublicEndPoint());
				Session.sendRequest(Request);

				Poco::Net::HTTPResponse Response;
				std::istream &is = Session.receiveResponse(Response);
				if(Response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
					Poco::JSON::Parser	P;
					ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
				}
				return Response.getStatus();
			}
		}
		catch (const Poco::Exception &E)
		{
			std::cerr << E.displayText() << std::endl;
		}
		return -1;
	}

    int OpenAPIRequestPut::Do(Poco::JSON::Object::Ptr &ResponseObject) {
	    try {
	        auto Services = Daemon()->GetServices(Type_);
	        for(auto const &Svc:Services) {
	            Poco::URI	URI(Svc.PrivateEndPoint);
	            Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());

	            URI.setPath(EndPoint_);
	            for (const auto &qp : QueryData_)
	                URI.addQueryParameter(qp.first, qp.second);

	            std::string Path(URI.getPathAndQuery());
	            Session.setTimeout(Poco::Timespan(msTimeout_/1000, msTimeout_ % 1000));

	            Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_PUT,
                                               Path,
                                               Poco::Net::HTTPMessage::HTTP_1_1);
	            std::ostringstream obody;
	            Poco::JSON::Stringifier::stringify(Body_,obody);

	            Request.setContentType("application/json");
	            Request.setContentLength(obody.str().size());

	            Request.add("X-API-KEY", Svc.AccessKey);
	            Request.add("X-INTERNAL-NAME", Daemon()->PublicEndPoint());

	            std::ostream & os = Session.sendRequest(Request);
	            os << obody.str();

	            Poco::Net::HTTPResponse Response;
	            std::istream &is = Session.receiveResponse(Response);
	            if(Response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
	                Poco::JSON::Parser	P;
	                ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
//	                std::cout << "Response OK" << std::endl;
	            } else {
	                Poco::JSON::Parser	P;
	                ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
//	                std::cout << "Response: " << Response.getStatus() << std::endl;
	            }
	            return Response.getStatus();
	        }
	    }
	    catch (const Poco::Exception &E)
	    {
	        std::cerr << E.displayText() << std::endl;
	    }
	    return -1;
	}

	int OpenAPIRequestPost::Do(Poco::JSON::Object::Ptr &ResponseObject) {
	    try {
	        auto Services = Daemon()->GetServices(Type_);
	        for(auto const &Svc:Services) {
	            Poco::URI	URI(Svc.PrivateEndPoint);
	            Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());

	            URI.setPath(EndPoint_);
	            for (const auto &qp : QueryData_)
	                URI.addQueryParameter(qp.first, qp.second);

	            std::string Path(URI.getPathAndQuery());
	            Session.setTimeout(Poco::Timespan(msTimeout_/1000, msTimeout_ % 1000));

	            Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_POST,
                                               Path,
                                               Poco::Net::HTTPMessage::HTTP_1_1);
	            std::ostringstream obody;
	            Poco::JSON::Stringifier::stringify(Body_,obody);

	            Request.setContentType("application/json");
	            Request.setContentLength(obody.str().size());

	            Request.add("X-API-KEY", Svc.AccessKey);
	            Request.add("X-INTERNAL-NAME", Daemon()->PublicEndPoint());

	            std::ostream & os = Session.sendRequest(Request);
	            os << obody.str();

	            Poco::Net::HTTPResponse Response;
	            std::istream &is = Session.receiveResponse(Response);
	            if(Response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
	                Poco::JSON::Parser	P;
	                ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
	                //	                std::cout << "Response OK" << std::endl;
	            } else {
	                Poco::JSON::Parser	P;
	                ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
	                //	                std::cout << "Response: " << Response.getStatus() << std::endl;
	            }
	            return Response.getStatus();
	        }
	    }
	    catch (const Poco::Exception &E)
	    {
	        std::cerr << E.displayText() << std::endl;
	    }
	    return -1;
	}

}
