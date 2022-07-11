//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_WebServer.h"
#include "Poco/Net/MediaType.h"
#include "RTTYS_ClientConnection.h"
#include "RTTYS_server.h"

namespace OpenWifi {

	RTTY_Client_WebSocketRequestHandler::RTTY_Client_WebSocketRequestHandler(Poco::Logger & L)
		:Logger_(L) {
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

		if(!RTTYS_server()->ValidId(T[2])) {
			response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
			response.send();
			return;
		}

		try {
			RTTYS_server()->CreateNewClient(request,response,T[2]);
			// RTTYS_server()->RegisterClient(T[2],NewRTTYClient);
			// NewRTTYClient->CompleteLogin();
		} catch (...) {
			Logger_.warning("Exception during WS creation");
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

	inline void ProcessOptions(Poco::Net::HTTPServerRequest &Request,
							   Poco::Net::HTTPServerResponse &Response) {
		Response.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
		Response.setChunkedTransferEncoding(true);
		auto Origin = Request.find("Origin");
		if (Origin != Request.end()) {
			Response.set("Access-Control-Allow-Origin", Origin->second);
		} else {
			Response.set("Access-Control-Allow-Origin", "*");
		}
		Response.set("Access-Control-Allow-Methods", "GET, OPTIONS");
		auto RequestHeaders = Request.find("Access-Control-Request-Headers");
		if(RequestHeaders!=Request.end())
			Response.set("Access-Control-Allow-Headers", RequestHeaders->second);
		Response.set("Vary", "Origin, Accept-Encoding");
		Response.set("Access-Control-Allow-Credentials", "true");
		Response.set("Access-Control-Max-Age", "86400");
		Response.set("Connection", "Keep-Alive");
		Response.set("Keep-Alive", "timeout=30, max=1000");

		Response.setContentLength(0);
		Response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
		Response.send();
	}

	inline void SetCommonHeaders(Poco::Net::HTTPServerRequest &Request,
								 Poco::Net::HTTPServerResponse &Response, bool CloseConnection) {
		Response.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
		Response.setChunkedTransferEncoding(true);
		Response.setContentType("application/json");
		auto Origin = Request.find("Origin");
		if (Origin != Request.end()) {
			Response.set("Access-Control-Allow-Origin", Origin->second);
		} else {
			Response.set("Access-Control-Allow-Origin", "*");
		}
		Response.set("Vary", "Origin, Accept-Encoding");
		if(CloseConnection) {
			Response.set("Connection", "close");
			Response.setKeepAlive(false);
		} else {
			Response.setKeepAlive(true);
			Response.set("Connection", "Keep-Alive");
			Response.set("Keep-Alive", "timeout=30, max=1000");
		}
	}

	static inline std::atomic_uint64_t rtty_ws_id = 1;

	void PageRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
					   Poco::Net::HTTPServerResponse &response) {

		Utils::SetThreadName("rt:webserver");
		[[maybe_unused]] uint64_t id = rtty_ws_id++;

		poco_debug(Logger(),fmt::format("{}: Starting request.",id));
		Poco::URI uri(request.getURI());
		auto Path = uri.getPath();

		if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
			return ProcessOptions(request, response);
		}

		if(request.getMethod() != Poco::Net::HTTPRequest::HTTP_GET) {
			SetCommonHeaders(request,response,false);
			response.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
			response.send();
			return;
		}

		if (Path == "/") {
			Path = RTTYS_server()->UIAssets() + "/index.html";
		} else {
			auto ParsedPath = Poco::StringTokenizer(Path, "/");
			if (ParsedPath.count() > 1) {
				if (ParsedPath[1] == "connect") {
					response.redirect(Poco::replace(Path,"/connect/","/rtty/"));
					poco_debug(Logger(),fmt::format("{}: Redirect: {}",id,Path));
					return;
				} else if (ParsedPath[1] == "authorized") {
					SetCommonHeaders(request,response, false);
					nlohmann::json doc;
					doc["authorized"] = true;
					response.setContentType("application/json");
					std::ostream &answer = response.send();
					answer << to_string(doc);
					poco_debug(Logger(),fmt::format("{}: Finishing authorization request.",id));
					return;
				} else if (ParsedPath[1] == "fontsize") {
					SetCommonHeaders(request,response, false);
					nlohmann::json doc;
					doc["size"] = 16;
					response.setContentType("application/json");
					std::ostream &answer = response.send();
					answer << to_string(doc);
					poco_debug(Logger(),fmt::format("{}: Finishing font size request.",id));
					return;
				}
			}
			Path = RTTYS_server()->UIAssets() + Path;
		}

		if(Path.find("../")!=std::string::npos) {
			poco_debug(Logger(),fmt::format("{}: Finishing request.",id));
			return;
		}

		if(Path.find("~/")!=std::string::npos) {
			poco_debug(Logger(),fmt::format("{}: Finishing request.",id));
			return;
		}

		Poco::File	F(Path);
		SetCommonHeaders(request,response, false);
		if(!F.exists()) {
			// std::cout << id << ": Path " << Path << " does not exist" << std::endl;
			Path = RTTYS_server()->UIAssets() + "/index.html";
			response.sendFile(Path,"text/html");
			poco_debug(Logger(),fmt::format("{}: Finishing request.",id));
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
				poco_debug(Logger(),fmt::format("{}: Downloading UI Assets.",id));
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
		poco_debug(Logger(),fmt::format("{}: Finishing request.",id));
	}

	RTTY_Client_RequestHandlerFactory::RTTY_Client_RequestHandlerFactory(Poco::Logger & L)
		: Logger_(L) {}

	Poco::Net::HTTPRequestHandler *
	RTTY_Client_RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &request) {
		try {
			if (request.find("Upgrade") != request.end() &&
				Poco::icompare(request["Upgrade"], "websocket") == 0) {
				Poco::Thread::current()->setName("WebRTTYRequest_WSHandler");
				return new RTTY_Client_WebSocketRequestHandler(Logger_);
			} else {
				Poco::Thread::current()->setName("WebRTTYRequest_PageHandler");
				return new PageRequestHandler(Logger_);
			}
		} catch (...) {

		}
		return nullptr;
	}
}