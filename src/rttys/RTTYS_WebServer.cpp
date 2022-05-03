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
		const auto & P = uri.getPath();
		auto T = Poco::StringTokenizer(P, "/");
		if (T.count() != 3)
			return;
		if (T[1] != "connect")
			return;
		try {
			Poco::Thread::current()->setName(fmt::format("WebRTTYRequest_WSHandler_{}", T[2]));
			auto ws_ptr = std::make_unique<Poco::Net::WebSocket>(request, response);
			new RTTYS_ClientConnection(std::move(ws_ptr), T[2], R_);
		} catch (...) {
			RTTYS_server()->Logger().warning("Exception during WS creation");
		}
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
						Poco::Net::HTTPServerResponse & Response, Poco::Logger & Logger_, uint64_t id) {

		Logger_.information(fmt::format("{}: Adding CORS", id));
		Response.setChunkedTransferEncoding(true);
		auto Origin = Request.find("Origin");
		if (Origin != Request.end()) {
			Response.set("Access-Control-Allow-Origin", Origin->second);
			Response.set("Vary", "Origin");
		} else {
			Response.set("Access-Control-Allow-Origin", "*");
		}
		auto Referer = Request.find("Referer");
		if(Referer!=Request.end()) {
			Response.set("Access-Control-Allow-Origin", Referer->second);
		} else {
			Response.set("Access-Control-Allow-Origin", "*");
		}
		Response.set("Access-Control-Allow-Headers", "*");
		Response.set("Access-Control-Max-Age", "86400");
		Response.set("Access-Control-Allow-Methods", "GET, OPTIONS, HEAD");
		Response.set("Connection", "Keep-Alive");
		Response.set("Keep-Alive", "timeout=120");
		Response.set("Accept-Ranges","bytes");
	}

	static inline std::atomic_uint64_t rtty_ws_id = 1;

	void PageRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
					   Poco::Net::HTTPServerResponse &response) {

		Poco::Logger & Logger_ = RTTYS_server()->Logger();
		uint64_t id = rtty_ws_id++;

		Logger_.information(fmt::format("{}: Starting request.",id));
		Poco::URI uri(request.getURI());
		auto Path = uri.getPath();

		if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
			AddCORS(request,response, Logger_, id);
			response.send();
			Logger_.information(fmt::format("{}: Finishing OPTIONS request.",id));
			return;
		} else if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_HEAD){
			AddCORS(request,response, Logger_, id);
			response.send();
			Logger_.information(fmt::format("{}: Finishing HEAD request.",id));
			return;
		}

		if (Path == "/") {
			Path = RTTYS_server()->UIAssets() + "/index.html";
		} else {
			auto ParsedPath = Poco::StringTokenizer(Path, "/");
			if (ParsedPath.count() > 1) {
				if (ParsedPath[1] == "connect") {
					response.redirect(Poco::replace(Path,"/connect/","/rtty/"));
					RTTYS_server()->Logger().information(fmt::format("redirect: {}",Path));
					return;
				} else if (ParsedPath[1] == "authorized") {
					AddCORS(request,response, Logger_, id);
					nlohmann::json doc;
					doc["authorized"] = true;
					response.setContentType("application/json");
					std::ostream &answer = response.send();
					answer << to_string(doc);
					Logger_.information(fmt::format("{}: Finishing authorization request.",id));
					return;
				} else if (ParsedPath[1] == "fontsize") {
					AddCORS(request,response, Logger_, id);
					nlohmann::json doc;
					doc["size"] = 16;
					response.setContentType("application/json");
					std::ostream &answer = response.send();
					answer << to_string(doc);
					Logger_.information(fmt::format("{}: Finishing font size request.",id));
					return;
				}
			}
			Path = RTTYS_server()->UIAssets() + Path;
		}

		// std::cout << id << ": Serving path '" << Path << "'" << std::endl;

		//	simple test to block .. or ~ in path names.
		if(Path.find("../")!=std::string::npos) {
			Logger_.information(fmt::format("{}: Finishing request.",id));
			return;
		}

		if(Path.find("~/")!=std::string::npos) {
			Logger_.information(fmt::format("{}: Finishing request.",id));
			return;
		}

		Poco::File	F(Path);
		AddCORS(request,response, Logger_, id);
		if(!F.exists()) {
			// std::cout << id << ": Path " << Path << " does not exist" << std::endl;
			Path = RTTYS_server()->UIAssets() + "/index.html";
			response.sendFile(Path,"text/html");
			Logger_.information(fmt::format("{}: Finishing request.",id));
			return;
		}
		Poco::Path P(Path);
		auto Ext = P.getExtension();

		std::string Type;
		if (Ext == "html")
			Type = "text/html; charset=utf-8";
		else if (Ext == "js") {
			Type = "text/javascript; charset=utf-8";
			if(IsFileGZipped(Path)) {
				response.set("Content-Encoding", "gzip");
			}
		}  else if (Ext == "css") {
			Type = "text/css; charset=utf-8";
			if(IsFileGZipped(Path)) {
				Logger_.information(fmt::format("{}: Downloading UI Assets.",id));
				response.set("Content-Encoding", "gzip");
			}
		}  else if (Ext == "ico")
			Type = "image/x-icon";
		else if (Ext == "woff")
			Type = "font/woff";
		else if (Ext == "woff2")
			Type = "font/woff2";
		else if (Ext == "ttf")
			Type = "font/ttf";

		response.setContentLength(F.getSize());
		response.sendFile(Path, Type);
		Logger_.information(fmt::format("{}: Finishing request.",id));
	}

	RTTY_Client_RequestHandlerFactory::RTTY_Client_RequestHandlerFactory(Poco::Net::SocketReactor &R)
		: Reactor_(R) {}

	Poco::Net::HTTPRequestHandler *
	RTTY_Client_RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &request) {
		try {
			if (request.find("Upgrade") != request.end() &&
				Poco::icompare(request["Upgrade"], "websocket") == 0) {
				Poco::Thread::current()->setName("WebRTTYRequest_WSHandler");
				return new RTTY_Client_WebSocketRequestHandler(Reactor_);
			} else {
				Poco::Thread::current()->setName("WebRTTYRequest_PageHandler");
				return new PageRequestHandler;
			}
		} catch (...) {

		}
		return nullptr;
	}
}