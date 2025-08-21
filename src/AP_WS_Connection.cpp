//
// Created by stephane bourque on 2022-02-03.
//


#include <Poco/Base64Decoder.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/HTTPServerRequestImpl.h>
#include <Poco/Net/HTTPServerResponseImpl.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SSLException.h>
#include <Poco/Net/SecureStreamSocketImpl.h>
#include <Poco/Net/WebSocketImpl.h>

#include <framework/KafkaManager.h>
#include <framework/MicroServiceFuncs.h>
#include <framework/utils.h>
#include <framework/ow_constants.h>

#include <fmt/format.h>

#include <AP_WS_Connection.h>
#include <AP_WS_Server.h>
#include <CentralConfig.h>
#include <CommandManager.h>
#include <StorageService.h>
#include <RADIUSSessionTracker.h>
#include <RADIUS_proxy_server.h>
#include <GWKafkaEvents.h>
#include <UI_GW_WebSocketNotifications.h>

namespace OpenWifi {

	void AP_WS_Connection::LogException(const Poco::Exception &E) {
		poco_information(Logger_, fmt::format("EXCEPTION({}): {}", CId_, E.displayText()));
	}

	AP_WS_Connection::AP_WS_Connection(Poco::Net::HTTPServerRequest &request,
									   Poco::Net::HTTPServerResponse &response,
									   uint64_t session_id, Poco::Logger &L,
									   std::pair<std::shared_ptr<Poco::Net::SocketReactor>, std::shared_ptr<LockedDbSession>> R)
		: Logger_(L) {

		Reactor_ = R.first;
		DbSession_ = R.second;
		State_.sessionId = session_id;

		WS_ = std::make_unique<Poco::Net::WebSocket>(request, response);

		auto TS = Poco::Timespan(360, 0);

		WS_->setMaxPayloadSize(BufSize);
		WS_->setReceiveTimeout(TS);
		WS_->setNoDelay(false);
		WS_->setKeepAlive(true);
		WS_->setBlocking(false);
		uuid_ = MicroServiceRandom(std::numeric_limits<std::uint64_t>::max()-1);

		AP_WS_Server()->IncrementConnectionCount();
	}

	void AP_WS_Connection::Start() {
		Registered_ = true;
		LastContact_ = Utils::Now();

		Reactor_->addEventHandler(*WS_,
								  Poco::NObserver<AP_WS_Connection, Poco::Net::ReadableNotification>(
									  *this, &AP_WS_Connection::OnSocketReadable));
		Reactor_->addEventHandler(*WS_,
								  Poco::NObserver<AP_WS_Connection, Poco::Net::ShutdownNotification>(
									  *this, &AP_WS_Connection::OnSocketShutdown));
		Reactor_->addEventHandler(*WS_,
								  Poco::NObserver<AP_WS_Connection, Poco::Net::ErrorNotification>(
									  *this, &AP_WS_Connection::OnSocketError));

	}

	AP_WS_Connection::~AP_WS_Connection() {
		std::lock_guard G(ConnectionMutex_);
		AP_WS_Server()->DecrementConnectionCount();
		EndConnection();
		poco_debug(Logger_, fmt::format("TERMINATION({}): Session={}, Connection removed.", SerialNumber_,
										State_.sessionId));
	}

	static void NotifyKafkaDisconnect(const std::string &SerialNumber, std::uint64_t uuid) {
		try {
			Poco::JSON::Object Disconnect;
			Poco::JSON::Object Details;
			Details.set(uCentralProtocol::SERIALNUMBER, SerialNumber);
			Details.set(uCentralProtocol::TIMESTAMP, Utils::Now());
			Details.set(uCentralProtocol::UUID,uuid);
			Disconnect.set(uCentralProtocol::DISCONNECTION, Details);
			KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber, Disconnect);
		} catch (...) {
		}
	}

	void AP_WS_Connection::EndConnection() {
		bool expectedValue=false;
		if (Dead_.compare_exchange_strong(expectedValue,true,std::memory_order_release,std::memory_order_relaxed)) {

			if(!SerialNumber_.empty() && State_.LastContact!=0) {
				StorageService()->SetDeviceLastRecordedContact(SerialNumber_, State_.LastContact);
			}

			if (Registered_) {
				Registered_ = false;
				Reactor_->removeEventHandler(
					*WS_, Poco::NObserver<AP_WS_Connection, Poco::Net::ReadableNotification>(
							  *this, &AP_WS_Connection::OnSocketReadable));
				Reactor_->removeEventHandler(
					*WS_, Poco::NObserver<AP_WS_Connection, Poco::Net::ShutdownNotification>(
							  *this, &AP_WS_Connection::OnSocketShutdown));
				Reactor_->removeEventHandler(
					*WS_, Poco::NObserver<AP_WS_Connection, Poco::Net::ErrorNotification>(
							  *this, &AP_WS_Connection::OnSocketError));
				Registered_=false;
			}
			WS_->close();

			if(!SerialNumber_.empty()) {
				DeviceDisconnectionCleanup(SerialNumber_, uuid_);
			}
			AP_WS_Server()->AddCleanupSession(State_.sessionId, SerialNumberInt_);
		}
	}

	bool AP_WS_Connection::ValidatedDevice() {

		if(Dead_)
			return false;

		if (DeviceValidated_)
			return true;

		try {
			auto SockImpl = dynamic_cast<Poco::Net::WebSocketImpl *>(WS_->impl());
			auto SS =
				dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(SockImpl->streamSocketImpl());

			PeerAddress_ = SS->peerAddress().host();
			CId_ = Utils::FormatIPv6(SS->peerAddress().toString());

			State_.started = Utils::Now();

			if (!SS->secure()) {
				poco_warning(Logger_, fmt::format("TLS-CONNECTION({}): Session={} Connection is "
												  "NOT secure. Device is not allowed.",
												  CId_, State_.sessionId));
				return false;
			}

			poco_trace(Logger_, fmt::format("TLS-CONNECTION({}): Session={} Connection is secure.",
											CId_, State_.sessionId));

			if (!SS->havePeerCertificate()) {
				State_.VerifiedCertificate = GWObjects::NO_CERTIFICATE;
				poco_warning(
					Logger_,
					fmt::format("TLS-CONNECTION({}): Session={} No certificates available..", CId_,
								State_.sessionId));
				return false;
			}

			Poco::Crypto::X509Certificate PeerCert(SS->peerCertificate());
			if (!AP_WS_Server()->ValidateCertificate(CId_, PeerCert)) {
				State_.VerifiedCertificate = GWObjects::NO_CERTIFICATE;
				poco_warning(Logger_,
							 fmt::format("TLS-CONNECTION({}): Session={} Device certificate is not "
										 "valid. Device is not allowed.",
										 CId_, State_.sessionId));
				return false;
			}

			CN_ = Poco::trim(Poco::toLower(PeerCert.commonName()));
			if(!Utils::ValidSerialNumber(CN_)) {
				poco_trace(Logger_,
						   fmt::format("TLS-CONNECTION({}): Session={} Invalid serial number: CN={}", CId_,
									   State_.sessionId, CN_));
				return false;
			}
			SerialNumber_ = CN_;
			SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);

			State_.VerifiedCertificate = GWObjects::VALID_CERTIFICATE;
			poco_trace(Logger_,
					   fmt::format("TLS-CONNECTION({}): Session={} Valid certificate: CN={}", CId_,
								   State_.sessionId, CN_));

			if (AP_WS_Server::IsSim(CN_) && !AP_WS_Server()->IsSimEnabled()) {
				poco_warning(Logger_, fmt::format("TLS-CONNECTION({}): Session={} Sim Device {} is "
												  "not allowed. Disconnecting.",
												  CId_, State_.sessionId, CN_));
				return false;
			}

			if(AP_WS_Server::IsSim(SerialNumber_)) {
				State_.VerifiedCertificate = GWObjects::SIMULATED;
				Simulated_ = true;
			}

			std::string reason, author;
			std::uint64_t created;
			if (!CN_.empty() && StorageService()->IsBlackListed(SerialNumberInt_, reason, author, created)) {
				DeviceBlacklistedKafkaEvent KE(Utils::SerialNumberToInt(CN_), Utils::Now(), reason, author, created, CId_);
				poco_warning(
					Logger_,
					fmt::format(
						"TLS-CONNECTION({}): Session={} Device {} is black listed. Disconnecting.",
						CId_, State_.sessionId, CN_));
				return false;
			}

			State_.certificateExpiryDate = PeerCert.expiresOn().timestamp().epochTime();
			State_.certificateIssuerName = PeerCert.issuerName();

			poco_trace(Logger_,
					   fmt::format("TLS-CONNECTION({}): Session={} CN={} Completed. (t={})", CId_,
								   State_.sessionId, CN_, ConcurrentStartingDevices_));
			DeviceValidated_ = true;
			return true;

		} catch (const Poco::Net::CertificateValidationException &E) {
			poco_error(
				Logger_,
				fmt::format(
					"CONNECTION({}): Session:{} Poco::CertificateValidationException Certificate "
					"Validation failed during connection. Device will have to retry.",
					CId_, State_.sessionId));
			Logger_.log(E);
		} catch (const Poco::Net::WebSocketException &E) {
			poco_error(Logger_,
					   fmt::format("CONNECTION({}): Session:{} Poco::WebSocketException WebSocket "
								   "error during connection. Device will have to retry.",
								   CId_, State_.sessionId));
			Logger_.log(E);
		} catch (const Poco::Net::ConnectionAbortedException &E) {
			poco_error(
				Logger_,
				fmt::format("CONNECTION({}):Session:{}  Poco::ConnectionAbortedException "
							"Connection was aborted during connection. Device will have to retry.",
							CId_, State_.sessionId));
			Logger_.log(E);
		} catch (const Poco::Net::ConnectionResetException &E) {
			poco_error(
				Logger_,
				fmt::format("CONNECTION({}): Session:{} Poco::ConnectionResetException Connection "
							"was reset during connection. Device will have to retry.",
							CId_, State_.sessionId));
			Logger_.log(E);
		} catch (const Poco::Net::InvalidCertificateException &E) {
			poco_error(Logger_,
					   fmt::format("CONNECTION({}): Session:{} Poco::InvalidCertificateException "
								   "Invalid certificate. Device will have to retry.",
								   CId_, State_.sessionId));
			Logger_.log(E);
		} catch (const Poco::Net::SSLException &E) {
			poco_error(Logger_,
					   fmt::format("CONNECTION({}): Session:{} Poco::SSLException SSL Exception "
								   "during connection. Device will have to retry.",
								   CId_, State_.sessionId));
			Logger_.log(E);
		} catch (const Poco::Exception &E) {
			poco_error(Logger_, fmt::format("CONNECTION({}): Session:{} Poco::Exception caught "
											"during device connection. Device will have to retry.",
											CId_, State_.sessionId));
			Logger_.log(E);
		} catch (...) {
			poco_error(
				Logger_,
				fmt::format("CONNECTION({}): Session:{} Exception caught during device connection. "
							"Device will have to retry. Unsecure connect denied.",
							CId_, State_.sessionId));
		}
		EndConnection();
		return false;
	}

	void AP_WS_Connection::DeviceDisconnectionCleanup(const std::string &SerialNumber, std::uint64_t uuid) {
		if (KafkaManager()->Enabled()) {
			NotifyKafkaDisconnect(SerialNumber, uuid);
		}
		RADIUSSessionTracker()->DeviceDisconnect(SerialNumber);
		GWWebSocketNotifications::SingleDevice_t N;
		N.content.serialNumber = SerialNumber;
		GWWebSocketNotifications::DeviceDisconnected(N);
	}

	void AP_WS_Connection::ProcessJSONRPCResult(Poco::JSON::Object::Ptr Doc) {
		poco_trace(Logger_, fmt::format("RECEIVED-RPC({}): {}.", CId_,
										Doc->get(uCentralProtocol::ID).toString()));
		CommandManager()->PostCommandResult(SerialNumber_, Doc);
	}

	void AP_WS_Connection::ProcessJSONRPCEvent(Poco::JSON::Object::Ptr &Doc) {
		auto Method = Doc->get(uCentralProtocol::METHOD).toString();
		auto EventType = uCentralProtocol::Events::EventFromString(Method);
		if (EventType == uCentralProtocol::Events::ET_UNKNOWN) {
			poco_warning(Logger_, fmt::format("ILLEGAL-PROTOCOL({}): Unknown message type '{}'",
											  CId_, Method));
			Errors_++;
			return;
		}

		if (!Doc->isObject(uCentralProtocol::PARAMS)) {
			poco_warning(Logger_,
						 fmt::format("MISSING-PARAMS({}): params must be an object.", CId_));
			Errors_++;
			return;
		}

		//  expand params if necessary
		auto ParamsObj = Doc->get(uCentralProtocol::PARAMS).extract<Poco::JSON::Object::Ptr>();
		if (ParamsObj->has(uCentralProtocol::COMPRESS_64)) {
			std::string UncompressedData;
			try {
				auto CompressedData = ParamsObj->get(uCentralProtocol::COMPRESS_64).toString();
				uint64_t compress_sz = 0;
				if (ParamsObj->has("compress_sz")) {
					compress_sz = ParamsObj->get("compress_sz");
				}

				if (Utils::ExtractBase64CompressedData(CompressedData, UncompressedData,
													   compress_sz)) {
					poco_trace(Logger_,
							   fmt::format("EVENT({}): Found compressed payload expanded to '{}'.",
										   CId_, UncompressedData));
					Poco::JSON::Parser Parser;
					ParamsObj = Parser.parse(UncompressedData).extract<Poco::JSON::Object::Ptr>();
				} else {
					poco_warning(Logger_,
								 fmt::format("INVALID-COMPRESSED-DATA({}): Compressed cannot be "
											 "uncompressed - content must be corrupt..: size={}",
											 CId_, CompressedData.size()));
					Errors_++;
					return;
				}
			} catch (const Poco::Exception &E) {
				poco_warning(Logger_, fmt::format("INVALID-COMPRESSED-JSON-DATA({}): Compressed "
												  "cannot be parsed - JSON must be corrupt..",
												  CId_));
				Logger_.log(E);
				return;
			}
		}

		if (!ParamsObj->has(uCentralProtocol::SERIAL)) {
			poco_warning(
				Logger_,
				fmt::format("MISSING-PARAMS({}): Serial number is missing in message.", CId_));
			return;
		}

		auto Serial =
			Poco::trim(Poco::toLower(ParamsObj->get(uCentralProtocol::SERIAL).toString()));
		if (!Utils::ValidSerialNumber(Serial)) {
			Poco::Exception E(
				fmt::format(
					"ILLEGAL-DEVICE-NAME({}): device name is illegal and not allowed to connect.",
					Serial),
				EACCES);
			E.rethrow();
		}

		std::string reason, author;
		std::uint64_t created;
		if (StorageService()->IsBlackListed(SerialNumberInt_, reason, author, created)) {
			DeviceBlacklistedKafkaEvent KE(Utils::SerialNumberToInt(CN_), Utils::Now(), reason, author, created, CId_);
			Poco::Exception E(
				fmt::format("BLACKLIST({}): device is blacklisted and not allowed to connect.",
							Serial),
				EACCES);
			E.rethrow();
		}

		switch (EventType) {
		case uCentralProtocol::Events::ET_CONNECT: {
			Process_connect(ParamsObj, Serial);
		} break;

		case uCentralProtocol::Events::ET_STATE: {
			Process_state(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_HEALTHCHECK: {
			Process_healthcheck(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_LOG: {
			Process_log(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_CRASHLOG: {
			Process_crashlog(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_PING: {
			Process_ping(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_CFGPENDING: {
			Process_cfgpending(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_RECOVERY: {
			Process_recovery(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_DEVICEUPDATE: {
			Process_deviceupdate(ParamsObj, Serial);
		} break;

		case uCentralProtocol::Events::ET_TELEMETRY: {
			Process_telemetry(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_VENUEBROADCAST: {
			Process_venuebroadcast(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_EVENT: {
			Process_event(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_ALARM: {
			Process_alarm(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_WIFISCAN: {
			Process_wifiscan(ParamsObj);
		} break;

		case uCentralProtocol::Events::ET_REBOOTLOG: {
			Process_rebootLog(ParamsObj);
		} break;

		// 	this will never be called but some compilers will complain if we do not have a case for
		//	every single values of an enum
		case uCentralProtocol::Events::ET_UNKNOWN: {
			poco_warning(Logger_, fmt::format("ILLEGAL-EVENT({}): Event '{}' unknown. CN={}", CId_,
											  Method, CN_));
			Errors_++;
		}
		}
	}

	bool AP_WS_Connection::StartTelemetry(uint64_t RPCID,
										  const std::vector<std::string> &TelemetryTypes) {
		poco_information(Logger_, fmt::format("TELEMETRY({}): Starting.", CId_));
		Poco::JSON::Object StartMessage;
		StartMessage.set("jsonrpc", "2.0");
		StartMessage.set("method", "telemetry");
		Poco::JSON::Object Params;
		Params.set("serial", SerialNumber_);
		Params.set("interval", (uint64_t)TelemetryInterval_);
		Poco::JSON::Array Types;
		if (TelemetryTypes.empty()) {
			Types.add("wifi-frames");
			Types.add("dhcp-snooping");
			Types.add("state");
		} else {
			for (const auto &type : TelemetryTypes)
				Types.add(type);
		}
		Params.set(RESTAPI::Protocol::TYPES, Types);
		StartMessage.set("id", RPCID);
		StartMessage.set("params", Params);
		Poco::JSON::Stringifier Stringify;
		std::ostringstream OS;
		Stringify.condense(StartMessage, OS);
		return Send(OS.str());
	}

	bool AP_WS_Connection::StopTelemetry(uint64_t RPCID) {
		poco_information(Logger_, fmt::format("TELEMETRY({}): Stopping.", CId_));
		Poco::JSON::Object StopMessage;
		StopMessage.set("jsonrpc", "2.0");
		StopMessage.set("method", "telemetry");
		Poco::JSON::Object Params;
		Params.set("serial", SerialNumber_);
		Params.set("interval", 0);
		StopMessage.set("id", RPCID);
		StopMessage.set("params", Params);
		Poco::JSON::Stringifier Stringify;
		std::ostringstream OS;
		Stringify.condense(StopMessage, OS);
		TelemetryKafkaPackets_ = TelemetryWebSocketPackets_ = TelemetryInterval_ =
			TelemetryKafkaTimer_ = TelemetryWebSocketTimer_ = 0;
		return Send(OS.str());
	}

	void AP_WS_Connection::UpdateCounts() {
		State_.kafkaClients = TelemetryKafkaRefCount_;
		State_.webSocketClients = TelemetryWebSocketRefCount_;
	}

	bool AP_WS_Connection::SetWebSocketTelemetryReporting(
		std::uint64_t RPCID, std::uint64_t Interval, std::uint64_t LifeTime,
		const std::vector<std::string> &TelemetryTypes) {
		std::unique_lock Lock(TelemetryMutex_);
		TelemetryWebSocketRefCount_++;
		TelemetryInterval_ = TelemetryInterval_
								 ? (Interval < (std::uint64_t)TelemetryInterval_ ? Interval : (std::uint64_t )TelemetryInterval_)
								 : Interval;
		auto TelemetryWebSocketTimer = LifeTime + Utils::Now();
		TelemetryWebSocketTimer_ = TelemetryWebSocketTimer > (std::uint64_t)TelemetryWebSocketTimer_
									   ? (std::uint64_t)TelemetryWebSocketTimer
									   : (std::uint64_t)TelemetryWebSocketTimer_;
		UpdateCounts();
		if (!TelemetryReporting_) {
			TelemetryReporting_ = true;
			return StartTelemetry(RPCID, TelemetryTypes);
		}
		return true;
	}

	bool
	AP_WS_Connection::SetKafkaTelemetryReporting(uint64_t RPCID, uint64_t Interval,
												 uint64_t LifeTime,
												 const std::vector<std::string> &TelemetryTypes) {
		std::unique_lock Lock(TelemetryMutex_);
		TelemetryKafkaRefCount_++;
		TelemetryInterval_ = TelemetryInterval_
								 ? (Interval < (std::uint64_t)TelemetryInterval_ ? (std::uint64_t)Interval : (std::uint64_t)TelemetryInterval_)
								 : Interval;
		auto TelemetryKafkaTimer = LifeTime + Utils::Now();
		TelemetryKafkaTimer_ =
			TelemetryKafkaTimer > (std::uint64_t)TelemetryKafkaTimer_ ? (std::uint64_t)TelemetryKafkaTimer : (std::uint64_t)TelemetryKafkaTimer_;
		UpdateCounts();
		if (!TelemetryReporting_) {
			TelemetryReporting_ = true;
			return StartTelemetry(RPCID, TelemetryTypes);
		}
		return true;
	}

	bool AP_WS_Connection::StopWebSocketTelemetry(uint64_t RPCID) {
		std::unique_lock Lock(TelemetryMutex_);
		if (TelemetryWebSocketRefCount_)
			TelemetryWebSocketRefCount_--;
		UpdateCounts();
		if (TelemetryWebSocketRefCount_ == 0 && TelemetryKafkaRefCount_ == 0) {
			TelemetryReporting_ = false;
			StopTelemetry(RPCID);
		}
		return true;
	}

	bool AP_WS_Connection::StopKafkaTelemetry(uint64_t RPCID) {
		std::unique_lock Lock(TelemetryMutex_);
		if (TelemetryKafkaRefCount_)
			TelemetryKafkaRefCount_--;
		UpdateCounts();
		if (TelemetryWebSocketRefCount_ == 0 && TelemetryKafkaRefCount_ == 0) {
			TelemetryReporting_ = false;
			StopTelemetry(RPCID);
		}
		return true;
	}

	void AP_WS_Connection::OnSocketShutdown(
		[[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		poco_trace(Logger_, fmt::format("SOCKET-SHUTDOWN({}): Closing.", CId_));
//		std::lock_guard	G(ConnectionMutex_);
		return EndConnection();
	}

	void AP_WS_Connection::OnSocketError(
		[[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		poco_trace(Logger_, fmt::format("SOCKET-ERROR({}): Closing.", CId_));
//		std::lock_guard	G(ConnectionMutex_);
		return EndConnection();
	}

	void AP_WS_Connection::OnSocketReadable(
		[[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {

		if (Dead_) //	we are dead, so we do not process anything.
			return;

		std::lock_guard	G(ConnectionMutex_);

		State_.LastContact = LastContact_ = Utils::Now();
		if (AP_WS_Server()->Running() && (DeviceValidated_ || ValidatedDevice())) {
			try {
				return ProcessIncomingFrame();
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
			} catch (const std::exception &E) {
				std::string W = E.what();
				poco_information(
					Logger_, fmt::format("std::exception caught: {}. Connection terminated with {}",
										 W, CId_));
			} catch (...) {
				poco_information(
					Logger_, fmt::format("Unknown exception for {}. Connection terminated.", CId_));
			}
		}
		EndConnection();
	}

	void AP_WS_Connection::ProcessIncomingFrame() {
		Poco::Buffer<char> IncomingFrame(0);

		bool	KillConnection=false;
		try {
			int 	Op, flags;
			auto IncomingSize = WS_->receiveFrame(IncomingFrame, flags);

			Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

			if (IncomingSize == 0 && flags == 0 && Op == 0) {
				poco_information(Logger_,
								 fmt::format("DISCONNECT({}): device has disconnected. Session={}",
											 CId_, State_.sessionId));
				return EndConnection();
			}

			IncomingFrame.append(0);

			State_.RX += IncomingSize;
			AP_WS_Server()->AddRX(IncomingSize);
			State_.MessageCount++;
			State_.LastContact = Utils::Now();

			switch (Op) {
				case Poco::Net::WebSocket::FRAME_OP_PING: {
					poco_trace(Logger_, fmt::format("WS-PING({}): received. PONG sent back.", CId_));
					WS_->sendFrame("", 0,
								   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
									   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);

					if (KafkaManager()->Enabled()) {
						Poco::JSON::Object PingObject;
						Poco::JSON::Object PingDetails;
						PingDetails.set(uCentralProtocol::FIRMWARE, State_.Firmware);
						PingDetails.set(uCentralProtocol::SERIALNUMBER, SerialNumber_);
						PingDetails.set(uCentralProtocol::COMPATIBLE, Compatible_);
						PingDetails.set(uCentralProtocol::CONNECTIONIP, CId_);
						PingDetails.set(uCentralProtocol::TIMESTAMP, Utils::Now());
						PingDetails.set(uCentralProtocol::UUID, uuid_);
						PingDetails.set("locale", State_.locale);
						PingObject.set(uCentralProtocol::PING, PingDetails);
						poco_trace(Logger_,fmt::format("Sending PING for {}", SerialNumber_));
						KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber_,PingObject);
					}
				} break;

				case Poco::Net::WebSocket::FRAME_OP_PONG: {
					poco_trace(Logger_, fmt::format("PONG({}): received and ignored.", CId_));
				} break;

				case Poco::Net::WebSocket::FRAME_OP_TEXT: {
					poco_trace(Logger_,
							   fmt::format("FRAME({}): Frame received (length={}, flags={}). Msg={}",
										   CId_, IncomingSize, flags, IncomingFrame.begin()));

					Poco::JSON::Parser parser;
					auto ParsedMessage = parser.parse(IncomingFrame.begin());
					auto IncomingJSON = ParsedMessage.extract<Poco::JSON::Object::Ptr>();

					if (IncomingJSON->has(uCentralProtocol::JSONRPC)) {
						if (IncomingJSON->has(uCentralProtocol::METHOD) &&
							IncomingJSON->has(uCentralProtocol::PARAMS)) {
							ProcessJSONRPCEvent(IncomingJSON);
						} else if (IncomingJSON->has(uCentralProtocol::RESULT) &&
								   IncomingJSON->has(uCentralProtocol::ID)) {
							poco_trace(Logger_, fmt::format("RPC-RESULT({}): payload: {}", CId_,
															IncomingFrame.begin()));
							ProcessJSONRPCResult(IncomingJSON);
						} else {
							poco_warning(
								Logger_,
								fmt::format("INVALID-PAYLOAD({}): Payload is not JSON-RPC 2.0: {}",
											CId_, IncomingFrame.begin()));
						}
					} else if (IncomingJSON->has(uCentralProtocol::RADIUS)) {
						ProcessIncomingRadiusData(IncomingJSON);
					} else {
						std::ostringstream iS;
						IncomingJSON->stringify(iS);
						poco_warning(
							Logger_,
							fmt::format("FRAME({}): illegal transaction header, missing 'jsonrpc': {}",
										CId_, iS.str()));
						Errors_++;
					}
				} break;

				case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
					poco_information(Logger_,
									 fmt::format("CLOSE({}): Device is closing its connection.", CId_));
					KillConnection=true;
				} break;

				default: {
					poco_warning(Logger_, fmt::format("UNKNOWN({}): unknown WS Frame operation: {}",
													  CId_, std::to_string(Op)));
					Errors_++;
					return;
				}
			}
		} catch (const Poco::Net::ConnectionResetException &E) {
			poco_warning(Logger_,
						 fmt::format("ConnectionResetException({}): Text:{} Payload:{} Session:{}",
									 CId_, E.displayText(),
									 IncomingFrame.begin() == nullptr ? "" : IncomingFrame.begin(),
									 State_.sessionId));
			KillConnection=true;
		} catch (const Poco::JSON::JSONException &E) {
			poco_warning(Logger_,
						 fmt::format("JSONException({}): Text:{} Payload:{} Session:{}", CId_,
									 E.displayText(),
									 IncomingFrame.begin() == nullptr ? "" : IncomingFrame.begin(),
									 State_.sessionId));
			KillConnection=true;
		} catch (const Poco::Net::WebSocketException &E) {
			poco_warning(Logger_,
						 fmt::format("WebSocketException({}): Text:{} Payload:{} Session:{}", CId_,
									 E.displayText(),
									 IncomingFrame.begin() == nullptr ? "" : IncomingFrame.begin(),
									 State_.sessionId));
			KillConnection=true;
		} catch (const Poco::Net::SSLConnectionUnexpectedlyClosedException &E) {
			poco_warning(
				Logger_,
				fmt::format(
					"SSLConnectionUnexpectedlyClosedException({}): Text:{} Payload:{} Session:{}",
					CId_, E.displayText(),
					IncomingFrame.begin() == nullptr ? "" : IncomingFrame.begin(),
					State_.sessionId));
			KillConnection=true;
		} catch (const Poco::Net::SSLException &E) {
			poco_warning(Logger_,
						 fmt::format("SSLException({}): Text:{} Payload:{} Session:{}", CId_,
									 E.displayText(),
									 IncomingFrame.begin() == nullptr ? "" : IncomingFrame.begin(),
									 State_.sessionId));
			KillConnection=true;
		} catch (const Poco::Net::NetException &E) {
			poco_warning(Logger_,
						 fmt::format("NetException({}): Text:{} Payload:{} Session:{}", CId_,
									 E.displayText(),
									 IncomingFrame.begin() == nullptr ? "" : IncomingFrame.begin(),
									 State_.sessionId));
			KillConnection=true;
		} catch (const Poco::IOException &E) {
			poco_warning(Logger_,
						 fmt::format("IOException({}): Text:{} Payload:{} Session:{}", CId_,
									 E.displayText(),
									 IncomingFrame.begin() == nullptr ? "" : IncomingFrame.begin(),
									 State_.sessionId));
			KillConnection=true;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger_,
						 fmt::format("Exception({}): Text:{} Payload:{} Session:{}", CId_,
									 E.displayText(),
									 IncomingFrame.begin() == nullptr ? "" : IncomingFrame.begin(),
									 State_.sessionId));
			KillConnection=true;
		} catch (const std::exception &E) {
			poco_warning(Logger_,
						 fmt::format("std::exception({}): Text:{} Payload:{} Session:{}", CId_,
									 E.what(),
									 IncomingFrame.begin() == nullptr ? "" : IncomingFrame.begin(),
									 State_.sessionId));
			KillConnection=true;
		} catch (...) {
			poco_error(Logger_, fmt::format("UnknownException({}): Device must be disconnected. "
											"Unknown exception.  Session:{}",
											CId_, State_.sessionId));
			KillConnection=true;
		}

		if (!KillConnection && Errors_ < 10)
			return;

		poco_warning(Logger_, fmt::format("DISCONNECTING({}): ConnectionException: {} Errors: {}", CId_, KillConnection, Errors_ ));
		EndConnection();
	}

	bool AP_WS_Connection::Send(const std::string &Payload) {
		try {
			size_t BytesSent = WS_->sendFrame(Payload.c_str(), (int)Payload.size());

			/*
			 * 	There is a possibility to actually try and send data but the device is no longer
			 * listening. This code attempts to wait 5 seconds to see if the device is actually
			 * still listening. if the data is not acked under 5 seconds, then we consider that the
			 * data never made it or the device is disconnected somehow.
			 */
#if defined(__APPLE__)
			tcp_connection_info info;
			int timeout = 4000;
			auto expireAt = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);
			do {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				socklen_t opt_len = sizeof(info);
				getsockopt(WS_->impl()->sockfd(), SOL_SOCKET, TCP_CONNECTION_INFO, (void *)&info,
						   &opt_len);
			} while (!info.tcpi_tfo_syn_data_acked && expireAt > std::chrono::system_clock::now());
			if (!info.tcpi_tfo_syn_data_acked)
				return false;
#else
			tcp_info info;
			int timeout = 4000;
			auto expireAt = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);
			do {
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
				socklen_t opt_len = sizeof(info);
				getsockopt(WS_->impl()->sockfd(), SOL_TCP, TCP_INFO, (void *)&info, &opt_len);
			} while (info.tcpi_unacked > 0 && expireAt > std::chrono::system_clock::now());

			if (info.tcpi_unacked > 0) {
				return false;
			}
#endif
			State_.TX += BytesSent;
			AP_WS_Server()->AddTX(BytesSent);
			return BytesSent == Payload.size();
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	std::string Base64Encode(const unsigned char *buffer, std::size_t size) {
		return Utils::base64encode(buffer, size);
	}

	std::string Base64Decode(const std::string &F) {
		std::istringstream ifs(F);
		Poco::Base64Decoder b64in(ifs);
		std::ostringstream ofs;
		Poco::StreamCopier::copyStream(b64in, ofs);
		return ofs.str();
	}

	bool AP_WS_Connection::SendRadiusAuthenticationData(const unsigned char *buffer,
														std::size_t size) {
		Poco::JSON::Object Answer;
		Answer.set(uCentralProtocol::RADIUS, uCentralProtocol::RADIUSAUTH);
		Answer.set(uCentralProtocol::RADIUSDATA, Base64Encode(buffer, size));

		std::ostringstream Payload;
		Answer.stringify(Payload);
		return Send(Payload.str());
	}

	bool AP_WS_Connection::SendRadiusAccountingData(const unsigned char *buffer, std::size_t size) {
		Poco::JSON::Object Answer;
		Answer.set(uCentralProtocol::RADIUS, uCentralProtocol::RADIUSACCT);
		Answer.set(uCentralProtocol::RADIUSDATA, Base64Encode(buffer, size));

		std::ostringstream Payload;
		Answer.stringify(Payload);
		return Send(Payload.str());
	}

	bool AP_WS_Connection::SendRadiusCoAData(const unsigned char *buffer, std::size_t size) {
		Poco::JSON::Object Answer;
		Answer.set(uCentralProtocol::RADIUS, uCentralProtocol::RADIUSCOA);
		Answer.set(uCentralProtocol::RADIUSDATA, Base64Encode(buffer, size));

		std::ostringstream Payload;
		Answer.stringify(Payload);
		return Send(Payload.str());
	}

	void AP_WS_Connection::ProcessIncomingRadiusData(const Poco::JSON::Object::Ptr &Doc) {
		if (Doc->has(uCentralProtocol::RADIUSDATA)) {
			auto Type = Doc->get(uCentralProtocol::RADIUS).toString();
			if (Type == uCentralProtocol::RADIUSACCT) {
				auto Data = Doc->get(uCentralProtocol::RADIUSDATA).toString();
				auto DecodedData = Base64Decode(Data);
				RADIUS_proxy_server()->SendAccountingData(SerialNumber_, DecodedData.c_str(),
														  DecodedData.size());
			} else if (Type == uCentralProtocol::RADIUSAUTH) {
				auto Data = Doc->get(uCentralProtocol::RADIUSDATA).toString();
				auto DecodedData = Base64Decode(Data);
				RADIUS_proxy_server()->SendAuthenticationData(SerialNumber_, DecodedData.c_str(),
															  DecodedData.size());
			} else if (Type == uCentralProtocol::RADIUSCOA) {
				auto Data = Doc->get(uCentralProtocol::RADIUSDATA).toString();
				auto DecodedData = Base64Decode(Data);
				RADIUS_proxy_server()->SendCoAData(SerialNumber_, DecodedData.c_str(),
												   DecodedData.size());
			}
		}
	}

	void AP_WS_Connection::SetLastStats(const std::string &LastStats) {
		RawLastStats_ = LastStats;
		try {
			Poco::JSON::Parser P;
			auto Stats = P.parse(LastStats).extract<Poco::JSON::Object::Ptr>();
			State_.hasGPS = Stats->isObject("gps");
			auto Unit = Stats->getObject("unit");
			auto Memory = Unit->getObject("memory");
			std::uint64_t TotalMemory = Memory->get("total");
			std::uint64_t FreeMemory = Memory->get("free");
			if (TotalMemory > 0) {
				State_.memoryUsed =
					(100.0 * ((double)TotalMemory - (double)FreeMemory)) / (double)TotalMemory;
			}
			if (Unit->isArray("load")) {
				Poco::JSON::Array::Ptr Load = Unit->getArray("load");
				if (Load->size() > 1) {
					State_.load = Load->get(1);
				}
			}
			if (Unit->isArray("temperature")) {
				Poco::JSON::Array::Ptr Temperature = Unit->getArray("temperature");
				if (Temperature->size() > 1) {
					State_.temperature = Temperature->get(0);
				}
			}
		} catch (const Poco::Exception &E) {
			poco_error(Logger_, "Failed to parse last stats: " + E.displayText());
		}
	}

} // namespace OpenWifi