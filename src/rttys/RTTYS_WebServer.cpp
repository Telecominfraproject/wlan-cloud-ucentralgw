//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_WebServer.h"
#include "Poco/Net/MediaType.h"
#include "RTTYS_ClientConnection.h"
#include "RTTYS_server.h"

namespace OpenWifi {

	RTTY_Client_WebSocketRequestHandler::RTTY_Client_WebSocketRequestHandler(Poco::Net::SocketReactor &R)
		:R_(R) {
	}

	void RTTY_Client_WebSocketRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
					   Poco::Net::HTTPServerResponse &response)  {
		Poco::URI uri(request.getURI());
		const auto P = uri.getPath();
		auto T = Poco::StringTokenizer(P, "/");
		if (T.count() != 3)
			return;
		if (T[1] != "connect")
			return;
		Poco::Net::WebSocket ws(request, response);
		new RTTYS_ClientConnection(ws, T[2], R_);
	};

	static bool IsFileGZipped(const std::string &FileName) {
		try {
			std::ifstream 	F(FileName, std::ifstream::binary);
			if(F) {
				unsigned buf[4]{0};
				F.seekg(0, F.beg);
				F.read((char*)&buf[0],1);
				F.read((char*)&buf[1],1);
				return buf[0]==0x1f && buf[1]==0x8b;
			}
		} catch (...) {
		}
		return false;
	}

	static void AddCORS(Poco::Net::HTTPServerRequest &Request,
						Poco::Net::HTTPServerResponse & Response) {
		auto Origin = Request.find("Origin");
		if (Origin != Request.end()) {
			Response.set("Access-Control-Allow-Origin", Origin->second);
			Response.set("Vary", "Origin");
		} else {
			Response.set("Access-Control-Allow-Origin", "*");
		}
		Response.set("Access-Control-Allow-Headers", "*");
		Response.set("Access-Control-Max-Age", "86400");
	}

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
					RTTYS_server()->Logger().information(Poco::format("... rtty connect redirect: %s",Path));
					response.redirect(Poco::replace(Path,"/connect/","/rtty/"));
					response.send();
					return;
				} else if (ParsedPath[1] == "authorized") {
					nlohmann::json doc;
					doc["authorized"] = true;
					AddCORS(request,response);
					response.setContentType("application/json");
					std::ostream &answer = response.send();
					answer << to_string(doc);
					return;
				} else if (ParsedPath[1] == "fontsize") {
					nlohmann::json doc;
					doc["size"] = 16;
					AddCORS(request,response);
					response.setContentType("application/json");
					std::ostream &answer = response.send();
					answer << to_string(doc);
					return;
				}
			}
			Path = RTTYS_server()->UIAssets() + Path;
		}

		Poco::File	F(Path);

		AddCORS(request,response);
		if(!F.exists()) {
			response.setChunkedTransferEncoding(true);
			Path = RTTYS_server()->UIAssets() + "/index.html";
			response.sendFile(Path,"text/html");
			return;
		}
		Poco::Path P(Path);
		auto Ext = P.getExtension();

		std::string Type;
		if (Ext == "html")
			Type = "text/html; charset=utf-8";
		else if (Ext == "js") {
			Type = "text/javascript; charset=utf-8";
			if(IsFileGZipped(Path))
				response.set("Content-Encoding", "gzip");
		}  else if (Ext == "css") {
			Type = "text/css; charset=utf-8";
			if(IsFileGZipped(Path))
				response.set("Content-Encoding", "gzip");
		}  else if (Ext == "ico")
			Type = "image/x-icon";
		else if (Ext == "woff")
			Type = "font/woff";
		else if (Ext == "woff2")
			Type = "font/woff2";
		else if (Ext == "ttf")
			Type = "font/ttf";

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