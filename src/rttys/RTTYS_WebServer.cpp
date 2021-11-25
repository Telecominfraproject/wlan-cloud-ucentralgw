//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_WebServer.h"
#include "RTTY_ClientConnection.h"
#include "RTTYS_server.h"
#include "Poco/Net/MediaType.h"

namespace OpenWifi {

	RTTY_Client_WebSocketRequestHandler::RTTY_Client_WebSocketRequestHandler(Poco::Net::SocketReactor &R)
		:R_(R) {
	}

	void RTTY_Client_WebSocketRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
					   Poco::Net::HTTPServerResponse &response)  {
		Poco::URI uri(request.getURI());
		auto P = uri.getPath();
		auto T = Poco::StringTokenizer(P, "/");
		if (T.count() != 3)
			return;
		if (T[1] != "connect")
			return;
		Poco::Net::WebSocket ws(request, response);
		new RTTY_ClientConnection(ws, T[2], R_);
	};

	void PageRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
					   Poco::Net::HTTPServerResponse &response) {
		Poco::URI uri(request.getURI());

		auto Path = uri.getPath();

		if (Path == "/") {
			Path = RTTYS_server()->UIAssets() + "/index.html";
		} else {
			auto ParsedPath = Poco::StringTokenizer(Path, "/");

			if (ParsedPath.count() > 1) {
				if (ParsedPath[1] == "connect") {
					std::cout << "Redirecting..." << std::endl;
					response.redirect(Poco::replace(Path,"/connect/","/rtty/"));
					response.send();
					return;
				} else if (ParsedPath[1] == "authorized") {
					std::cout << "authorized..." << std::endl;
					nlohmann::json doc;
					doc["authorized"] = true;
					response.set("Access-Control-Allow-Origin", "*");
					response.set("Access-Control-Allow-Headers", "*");
					response.set("Access-Control-Max-Age", "86400");
					response.setContentType("application/json");
					std::ostream &answer = response.send();
					answer << to_string(doc);
					return;
				} else if (ParsedPath[1] == "fontsize") {
					std::cout << "fontsize..." << std::endl;
					nlohmann::json doc;
					doc["size"] = 16;
					response.set("Access-Control-Allow-Origin", "*");
					response.set("Access-Control-Allow-Headers", "*");
					response.set("Access-Control-Max-Age", "86400");
					response.setContentType("application/json");
					std::ostream &answer = response.send();
					answer << to_string(doc);
					return;
				}
			}
			Path = RTTYS_server()->UIAssets() + Path;
		}

		Poco::File	F(Path);

		std::cout << "Path:" << Path << std::endl;

		if(!F.exists()) {
			response.set("Access-Control-Allow-Origin", "*");
			response.set("Access-Control-Allow-Headers", "*");
			response.set("Access-Control-Max-Age", "86400");
			response.setChunkedTransferEncoding(true);
			Path = RTTYS_server()->UIAssets() + "/index.html";
			response.sendFile(Path,"text/html");
			return;
		}

		std::cout << "Path:" << Path << std::endl;

		Poco::Path P(Path);
		auto Ext = P.getExtension();

		std::string Type;
		if (Ext == "html")
			Type = "text/html; charset=utf-8";
		else if (Ext == "js")
			Type = "application/javascript";
		else if (Ext == "css")
			Type = "text/css";
		else if (Ext == "ico")
			Type = "image/x-icon";
		else if (Ext == "woff")
			Type = "font/woff";
		else if (Ext == "woff2")
			Type = "font/woffs";
		else if (Ext == "ttf")
			Type = "font/ttf";

		response.set("Access-Control-Allow-Origin", "*");
		response.set("Access-Control-Allow-Headers", "*");
		response.set("Access-Control-Max-Age", "86400");
		response.set("Accept-Ranges","bytes");
		response.setChunkedTransferEncoding(true);
		response.setContentLength(F.getSize());
		response.sendFile(Path, Type);
	}

	RTTY_Client_RequestHandlerFactory::RTTY_Client_RequestHandlerFactory(Poco::Net::SocketReactor &R)
		: Reactor_(R) {}

	Poco::Net::HTTPRequestHandler *
	RTTY_Client_RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &request) {
		if (request.find("Upgrade") != request.end() &&
		Poco::icompare(request["Upgrade"], "websocket") == 0) {
			return new RTTY_Client_WebSocketRequestHandler(Reactor_);
		} else {
			return new PageRequestHandler;
		}
	}
}