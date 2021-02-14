//
// Created by stephane bourque on 2021-02-14.
//

#ifndef UCENTRAL_SVR_H
#define UCENTRAL_SVR_H

#include <iostream>
#include <vector>

#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>

#include <Poco/Net/HTTPRequestHandlerFactory.h>


#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
using namespace std;

// Page renderer for / uri
class MyPageHandler: public Poco::Net::HTTPRequestHandler
{
public:

    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response){
        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");

        std::ostream& responseStream = response.send();

        responseStream << "<html><head><head><title>My  HTTP Server in C++ </title></head>";
        responseStream << "<body><h1>Hello World</h1><p>";
        responseStream << "";
        responseStream << "</p></body></html>";
    };
};

// Page renderer for /Test uri

class TestPageHandler: public Poco::Net::HTTPRequestHandler
{
public:

    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response){
        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");

        std::ostream& responseStream = response.send();

        responseStream << "<html><head><head><title>My  HTTP Server in C++ </title></head>";
        responseStream << "<body><h1>Hello World 2</h1><p>";
        responseStream << "";
        responseStream << "</p></body></html>";
    };
};

class ErroPageHandler: public Poco::Net::HTTPRequestHandler
{
public:

    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response){
        response.setChunkedTransferEncoding(true);
        //Sets mime type text/html application/json etc.
        response.setContentType("text/html");
        //Sets the response status 404, 200 etc.
        response.setStatus("404");

        //opens the file stream
        std::ostream& responseStream = response.send();

        responseStream << "<html><head><head><title>My  HTTP Server in C++ </title></head>";
        responseStream << "<body><h1>PAGE NOT FOUND, SORRY!</h1><p>";
        responseStream << "";
        responseStream << "</p></body></html>";
    };
};

//  Url Router
class HandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:

    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest &request){
        // choose response based uri
        if(request.getURI()=="/"){

            return new MyPageHandler();
        }

        if(request.getURI()=="/test"){

            return new TestPageHandler();
        }
        //uri not recognized
        return new ErroPageHandler();
    }
};


// Server  Application
class MyWebHTTPServerApplication : public Poco::Util::ServerApplication
{
protected:
    int main(const std::vector<std::string> &args){

        //Port to listen
        Poco::UInt16 port = 9999;

        Poco::Net::ServerSocket socket(port);

        Poco::Net::HTTPServerParams *pParams = new Poco::Net::HTTPServerParams();
        //Sets the maximum number of queued connections.
        pParams->setMaxQueued(100);
        //Sets the maximum number of simultaneous threads available for this Server
        pParams->setMaxThreads(16);
        // Instanciate HandlerFactory
        Poco::Net::HTTPServer server(new HandlerFactory(), socket, pParams);

        server.start();

        waitForTerminationRequest();

        server.stop();

        return EXIT_OK;

    }
};


#endif //UCENTRAL_SVR_H
