//
// Created by stephane bourque on 2021-09-16.
//

#include "RESTAPI_telemetryWebSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "TelemetryStream.h"

namespace OpenWifi {

void RESTAPI_telemetryWebSocket::DoGet() {
		//	try and upgrade this session to websocket...
		if (Request->find("Upgrade") != Request->end() &&
			Poco::icompare((*Request)["Upgrade"], "websocket") == 0) {
			try {
				Poco::URI U(Request->getURI());
				std::string UUID, SNum;
				auto Parameters = U.getQueryParameters();
				for (const auto &i : Parameters) {
					if (i.first == "serialNumber") {
						SNum = i.second;
					} else if(i.first=="uuid") {
						UUID = i.second;
					}
				}

				if(!Utils::NormalizeMac(SNum)) {
					return BadRequest(RESTAPI::Errors::InvalidSerialNumber);
				}

				auto SerialNumber = Utils::SerialNumberToInt(SNum);

				if(!TelemetryStream()->IsValidEndPoint(SerialNumber,UUID)) {
					Logger_.warning(fmt::format("Illegal telemetry request for S: {}, UUID: {}", SerialNumber, UUID));
					Response->setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
					Response->setContentLength(0);
					Response->send();
					return;
				}

				auto WS = std::make_unique<Poco::Net::WebSocket>(*Request, *Response);
				new TelemetryClient(UUID, SerialNumber, std::move(WS), TelemetryStream()->NextReactor(), Logger_);

			} catch (const Poco::Net::WebSocketException &E) {
				Logger_.log(E);
				switch (E.code()) {
				case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
					Response->set("Sec-WebSocket-Version", Poco::Net::WebSocket::WEBSOCKET_VERSION);
				// fallthrough
				case Poco::Net::WebSocket::WS_ERR_NO_HANDSHAKE:
				case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
				case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
					Response->setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
					Response->setContentLength(0);
					Response->send();
					break;
				}
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
				Response->setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
				Response->setContentLength(0);
				Response->send();
				return;
			} catch (...) {
				Response->setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
				Response->setContentLength(0);
				Response->send();
				return;
			}
		} else {
			SetCommonHeaders(true);
			Response->setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
			Response->send();
			return;
		}
	}
}