//
// Created by stephane bourque on 2021-06-04.
//

#include "AwsNLBHealthCheck.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "uCentral.h"
#include "uUtils.h"

namespace uCentral::NLBHealthCheck {
    static std::string LastModified{"Last-Modified: " +Poco::DateTimeFormatter::format(Poco::DateTime(),Poco::DateTimeFormat::HTTP_FORMAT)};
    static std::string NLBMessage{"uCentralGW alive and kicking!\r\n"};

    void NLBConnection::run() {
        try {
            Poco::DateTime  Now;
            std::string Date = Poco::DateTimeFormatter::format(Now,Poco::DateTimeFormat::HTTP_FORMAT);

            std::string Response;
            Response += "HTTP/1.1 200 OK\r\nServer: NLB Healthcheck\r\nContent-Type: text/html\r\nAccept-Ranges: bytes\r\nConnection: close\r\n";
            Response += "Date: " + Date + "\r\n";
            Response += LastModified + "\r\n";
            Response += "Content-Length : " + std::to_string(NLBMessage.length()) + "\r\n\r\n";
            Response += NLBMessage;

            Poco::Net::StreamSocket &ss = socket();
            ss.sendBytes(Response.c_str(), (int) Response.size());
        } catch (...) {

        }
    }

    int Service::Start() {
        if(uCentral::ServiceConfig::GetBool("nlb.enable",false)) {
            Port_ = (int)uCentral::ServiceConfig::GetInt("nlb.port",15015);
            Srv_ = std::make_unique<Poco::Net::TCPServer>  ( new TCPFactory(), Port_ );
            Srv_->start();
        }
        return 0;
    }

    void Service::Stop() {
        if(Srv_) {
            Srv_->stop();
        }
    }

}

