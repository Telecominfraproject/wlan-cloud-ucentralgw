//
// Created by stephane bourque on 2021-11-30.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/JSON/Parser.h"

namespace OpenWifi {
    inline void API_Proxy( Poco::Logger &Logger,
                    Poco::Net::HTTPServerRequest *Request,
                    Poco::Net::HTTPServerResponse *Response,
                    const char * ServiceType,
                    const char * PathRewrite,
                    uint64_t msTimeout_ = 10000 ) {
        try {
            auto Services = MicroService::instance().GetServices(ServiceType);
            for(auto const &Svc:Services) {
                Poco::URI   SourceURI(Request->getURI());
                Poco::URI	DestinationURI(Svc.PrivateEndPoint);
                DestinationURI.setPath(PathRewrite);
                DestinationURI.setQuery(SourceURI.getQuery());

                // std::cout << "     Source: " << SourceURI.toString() << std::endl;
                // std::cout << "Destination: " << DestinationURI.toString() << std::endl;

                Poco::Net::HTTPSClientSession Session(DestinationURI.getHost(), DestinationURI.getPort());
                Session.setKeepAlive(true);
                Session.setTimeout(Poco::Timespan(msTimeout_/1000, msTimeout_ % 1000));
                Poco::Net::HTTPRequest ProxyRequest(Request->getMethod(),
                                                    DestinationURI.getPathAndQuery(),
                                                    Poco::Net::HTTPMessage::HTTP_1_1);
                if(Request->has("Authorization")) {
                    ProxyRequest.add("Authorization", Request->get("Authorization"));
                } else {
                    ProxyRequest.add("X-API-KEY", Svc.AccessKey);
                    ProxyRequest.add("X-INTERNAL-NAME", MicroService::instance().PublicEndPoint());
                }

                if(Request->getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
                    Session.sendRequest(ProxyRequest);
                    Poco::Net::HTTPResponse ProxyResponse;
                    Session.receiveResponse(ProxyResponse);
                    Response->setStatus(ProxyResponse.getStatus());
                    Response->send();
                    return;
                } else {
                    Poco::JSON::Parser P;
                    std::stringstream SS;
                    try {
                        auto Body = P.parse(Request->stream()).extract<Poco::JSON::Object::Ptr>();
                        Poco::JSON::Stringifier::condense(Body,SS);
                        SS << "\r\n\r\n";
                    } catch(const Poco::Exception &E) {
                        Logger.log(E);
                    }

                    if(SS.str().empty()) {
                        Session.sendRequest(ProxyRequest);
                    } else {
                        ProxyRequest.setContentType("application/json");
                        ProxyRequest.setContentLength(SS.str().size());
                        std::ostream & os = Session.sendRequest(ProxyRequest);
                        os << SS.str() ;
                    }

                    Poco::Net::HTTPResponse ProxyResponse;
                    std::stringstream SSR;
                    try {
                        std::istream &ProxyResponseStream = Session.receiveResponse(ProxyResponse);
                        Poco::JSON::Parser  P2;
                        auto ProxyResponseBody = P2.parse(ProxyResponseStream).extract<Poco::JSON::Object::Ptr>();
                        Poco::JSON::Stringifier::condense(ProxyResponseBody,SSR);
                        Response->setContentType("application/json");
                        Response->setContentLength(SSR.str().size());
                        Response->setStatus(ProxyResponse.getStatus());
                        Response->sendBuffer(SSR.str().c_str(),SSR.str().size());
                        return;
                    } catch( const Poco::Exception & E) {

                    }
                    Response->setStatus(ProxyResponse.getStatus());
                    Response->send();
                    return;
                }
            }

        } catch (const Poco::Exception &E) {
            Logger.log(E);
        }
    }
}