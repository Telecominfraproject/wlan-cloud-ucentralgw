//
// Created by stephane bourque on 2021-07-01.
//
#include <iostream>

#include "OpenAPIRequest.h"

#include "Poco/Net/HTTPSClientSession.h"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include "Utils.h"
#include "Daemon.h"

namespace OpenWifi {

	OpenAPIRequestGet::OpenAPIRequestGet( 	const std::string & ServiceType,
											const std::string & EndPoint,
									 		Types::StringPairVec & QueryData,
											uint64_t msTimeout):
 		Type_(ServiceType),
 		EndPoint_(EndPoint),
		QueryData_(QueryData),
		msTimeout_(msTimeout) {

	}

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
}
