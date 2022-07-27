//
// Created by stephane bourque on 2022-02-03.
//

#include "AP_WS_Connection.h"

#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/SecureServerSocketImpl.h"
#include "Poco/Net/HTTPServerResponseImpl.h"
#include "Poco/Net/HTTPServerSession.h"
#include "Poco/Net/HTTPServerRequestImpl.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/SSLException.h"
#include "Poco/Net/Context.h"
#include "Poco/Base64Decoder.h"
#include "Poco/Base64Encoder.h"

#include "Poco/zlib.h"

#include "AP_WS_Server.h"
#include "CentralConfig.h"
#include "CommandManager.h"
#include "ConfigurationCache.h"
#include "Daemon.h"
#include "FindCountry.h"
#include "StateUtils.h"
#include "StorageService.h"
#include "TelemetryStream.h"
#include "VenueBroadcaster.h"
#include "framework/WebSocketClientNotifications.h"
#include "Poco/Net/WebSocketImpl.h"

#include "RADIUS_proxy_server.h"

namespace OpenWifi {

	void AP_WS_Connection::LogException(const Poco::Exception &E) {
		Logger().information(fmt::format("EXCEPTION({}): {}", CId_, E.displayText()));
	}

	void AP_WS_Connection::CompleteStartup() {

		std::lock_guard Guard(Mutex_);
		try {
			auto SockImpl = dynamic_cast<Poco::Net::WebSocketImpl *>(WS_->impl());
			auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl*>(SockImpl->streamSocketImpl());

			while (true) {
				auto V = SS->completeHandshake();
				if (V == 1)
					break;
			}
			PeerAddress_ = SS->peerAddress().host();
			CId_ = Utils::FormatIPv6(SS->peerAddress().toString());
			if (!SS->secure()) {
				poco_error(Logger(),fmt::format("CONNECTION({}): Connection is NOT secure. Device is not allowed.", CId_));
				return delete this;
			} else {
				poco_trace(Logger(),fmt::format("CONNECTION({}): Connection is secure.", CId_));
			}

			if (SS->havePeerCertificate()) {
				CertValidation_ = GWObjects::VALID_CERTIFICATE;
				try {
					Poco::Crypto::X509Certificate PeerCert(SS->peerCertificate());

					if (AP_WS_Server()->ValidateCertificate(CId_, PeerCert)) {
						CN_ = Poco::trim(Poco::toLower(PeerCert.commonName()));
						CertValidation_ = GWObjects::MISMATCH_SERIAL;
						poco_trace(Logger(),fmt::format("CONNECTION({}): Valid certificate: CN={}", CId_, CN_));
					} else {
						poco_error(Logger(),fmt::format("CONNECTION({}): Device certificate is not valid. Device is not allowed.", CId_));
						return delete this;
					}
				} catch (const Poco::Exception &E) {
					LogException(E);
					poco_error(Logger(),fmt::format("CONNECTION({}): Device certificate is not valid. Device is not allowed.", CId_));
					return delete this;
				}
			} else {
				poco_error(Logger(),fmt::format("CONNECTION({}): No certificates available..", CId_));
				return delete this;
			}

			if (AP_WS_Server::IsSim(CN_) && !AP_WS_Server()->IsSimEnabled()) {
				poco_warning(Logger(),fmt::format(
					"CONNECTION({}): Sim Device {} is not allowed. Disconnecting.", CId_, CN_));
				return delete this;
			}

			SerialNumber_ = CN_;
			SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);

			if (!CN_.empty() && StorageService()->IsBlackListed(SerialNumber_)) {
				poco_warning(Logger(),fmt::format("CONNECTION({}): Device {} is black listed. Disconnecting.",
												   CId_, CN_));
				return delete this;
			}
			WS_->setMaxPayloadSize(BufSize);
			auto TS = Poco::Timespan(360, 0);
			WS_->setReceiveTimeout(TS);
			WS_->setNoDelay(true);
			WS_->setKeepAlive(true);

			Reactor_.addEventHandler(*WS_,
									 Poco::NObserver<AP_WS_Connection, Poco::Net::ReadableNotification>(
										 *this, &AP_WS_Connection::OnSocketReadable));
			Reactor_.addEventHandler(*WS_,
									 Poco::NObserver<AP_WS_Connection, Poco::Net::ShutdownNotification>(
										 *this, &AP_WS_Connection::OnSocketShutdown));
			Reactor_.addEventHandler(*WS_, Poco::NObserver<AP_WS_Connection, Poco::Net::ErrorNotification>(
											   *this, &AP_WS_Connection::OnSocketError));
			Registered_ = true;
			poco_trace(Logger(),fmt::format("CONNECTION({}): completed.", CId_));
			return;
		} catch (const Poco::Net::CertificateValidationException &E) {
			Logger().error(fmt::format("CONNECTION({}): Poco::CertificateValidationException Certificate Validation failed during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Net::WebSocketException &E) {
			Logger().error(fmt::format("CONNECTION({}): Poco::WebSocketException WebSocket error during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Net::ConnectionAbortedException &E) {
			Logger().error(fmt::format("CONNECTION({}): Poco::ConnectionAbortedException Connection was aborted during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Net::ConnectionResetException &E) {
			Logger().error(fmt::format("CONNECTION({}): Poco::ConnectionResetException Connection was reset during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Net::InvalidCertificateException &E) {
			Logger().error(fmt::format(
				"CONNECTION({}): Poco::InvalidCertificateException Invalid certificate. Device will have to retry.",
				CId_));
			Logger().log(E);
		} catch (const Poco::Net::SSLException &E) {
			Logger().error(fmt::format("CONNECTION({}): Poco::SSLException SSL Exception during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Exception &E) {
			Logger().error(fmt::format("CONNECTION({}): Poco::Exception caught during device connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (...) {
			Logger().error(fmt::format("CONNECTION({}): Exception caught during device connection. Device will have to retry. Unsecure connect denied.",
										CId_));
		}
		return delete this;
	}

	AP_WS_Connection::AP_WS_Connection(Poco::Net::HTTPServerRequest &request,
									   Poco::Net::HTTPServerResponse &response)
		: Logger_(AP_WS_Server()->Logger()) ,
		  Reactor_(AP_WS_Server()->NextReactor())
  	{
		WS_ = std::make_unique<Poco::Net::WebSocket>(request,response);
		CompleteStartup();
	}

	static void NotifyKafkaDisconnect(const std::string & SerialNumber) {
		try {
			Poco::JSON::Object Disconnect;
			Poco::JSON::Object Details;
			Details.set(uCentralProtocol::SERIALNUMBER, SerialNumber);
			Details.set(uCentralProtocol::TIMESTAMP, OpenWifi::Now());
			Disconnect.set(uCentralProtocol::DISCONNECTION, Details);
			Poco::JSON::Stringifier Stringify;
			std::ostringstream OS;
			Stringify.condense(Disconnect, OS);
			KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber, OS.str());
		} catch (...) {
		}
	}

	AP_WS_Connection::~AP_WS_Connection() {

		poco_debug(Logger(),fmt::format("CONNECTION({}): Removing connection for {}.", CId_, SerialNumber_));
		if (ConnectionId_)
			DeviceRegistry()->UnRegister(SerialNumberInt_, ConnectionId_);

		if (Registered_ && WS_) {
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<AP_WS_Connection, Poco::Net::ReadableNotification>(
											*this, &AP_WS_Connection::OnSocketReadable));
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<AP_WS_Connection, Poco::Net::ShutdownNotification>(
											*this, &AP_WS_Connection::OnSocketShutdown));
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<AP_WS_Connection, Poco::Net::ErrorNotification>(
											*this, &AP_WS_Connection::OnSocketError));
			(*WS_).close();
		} else if (WS_) {
			(*WS_).close();
		}

		if (KafkaManager()->Enabled() && !SerialNumber_.empty()) {
			std::string s(SerialNumber_);
			std::thread t([s]() { NotifyKafkaDisconnect(s); });
			t.detach();
		}
		WebSocketClientNotificationDeviceDisconnected(SerialNumber_);
	}

	bool AP_WS_Connection::LookForUpgrade(const uint64_t UUID, uint64_t & UpgradedUUID) {

		//	A UUID of zero means ignore updates for that connection.
		if (UUID == 0)
			return false;

		uint64_t GoodConfig = ConfigurationCache().CurrentConfig(SerialNumberInt_);
		if (GoodConfig && (GoodConfig == UUID || GoodConfig == Conn_->Conn_.PendingUUID)) {
			UpgradedUUID = UUID;
			return false;
		}

		GWObjects::Device D;
		if (StorageService()->GetDevice(SerialNumber_, D)) {

			//	This is the case where the cache is empty after a restart. So GoodConfig will 0. If the device already 	has the right UUID, we just return.
			if (D.UUID == UUID) {
				UpgradedUUID = UUID;
				ConfigurationCache().Add(SerialNumberInt_, UUID);
				return false;
			}

			if(UUID>D.UUID) {
				//	so we have a problem, the device has a newer config than we have. So we need to make sure our config
				//	is newer.
				Config::Config	Cfg(D.Configuration);
				D.UUID = UUID+2;
				UpgradedUUID = D.UUID;
				Cfg.SetUUID(D.UUID);
				D.Configuration = Cfg.get();
				StorageService()->UpdateDevice(D);
			}

			UpgradedUUID = D.UUID;
			Conn_->Conn_.PendingUUID = D.UUID;
			GWObjects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = MicroService::CreateUUID();
			Cmd.SubmittedBy = uCentralProtocol::SUBMITTED_BY_SYSTEM;
			Cmd.Status = uCentralProtocol::PENDING;
			Cmd.Command = uCentralProtocol::CONFIGURE;
			Poco::JSON::Parser P;
			auto ParsedConfig = P.parse(D.Configuration).extract<Poco::JSON::Object::Ptr>();
			Poco::JSON::Object Params;
			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::UUID, D.UUID);
			Params.set(uCentralProtocol::WHEN, 0);
			Params.set(uCentralProtocol::CONFIG, ParsedConfig);

			std::ostringstream O;
			Poco::JSON::Stringifier::stringify(Params, O);
			Cmd.Details = O.str();
			poco_information(Logger(),fmt::format("CFG-UPGRADE({}): Current ID: {}, newer configuration {}.",
										   CId_, UUID, D.UUID));
			bool Sent;

			StorageService()->AddCommand(SerialNumber_, Cmd, Storage::COMMAND_EXECUTED);
			CommandManager()->PostCommand(SerialNumber_, Cmd.Command, Params, Cmd.UUID, Sent);

			WebSocketClientNotificationDeviceConfigurationChange(D.SerialNumber, UUID, UpgradedUUID);

			return true;
		}
		return false;
	}

	void AP_WS_Connection::ProcessJSONRPCResult(Poco::JSON::Object::Ptr Doc) {
		CommandManager()->PostCommandResult(SerialNumber_, *Doc);
	}

	void AP_WS_Connection::ProcessJSONRPCEvent(Poco::JSON::Object::Ptr &Doc) {

		auto Method = Doc->get(uCentralProtocol::METHOD).toString();
		auto EventType = uCentralProtocol::Events::EventFromString(Method);
		if (EventType == uCentralProtocol::Events::ET_UNKNOWN) {
			poco_warning(Logger(),fmt::format("ILLEGAL-PROTOCOL({}): Unknown message type '{}'", CId_, Method));
			Errors_++;
			return;
		}

		if (!Doc->isObject(uCentralProtocol::PARAMS)) {
			poco_warning(Logger(),fmt::format("MISSING-PARAMS({}): params must be an object.", CId_));
			Errors_++;
			return;
		}

		//  expand params if necessary
		auto ParamsObj = Doc->get(uCentralProtocol::PARAMS).extract<Poco::JSON::Object::Ptr>();
		if (ParamsObj->has(uCentralProtocol::COMPRESS_64)) {
			std::string UncompressedData;
			try {
				auto CompressedData = ParamsObj->get(uCentralProtocol::COMPRESS_64).toString();
				uint64_t compress_sz = 0 ;
				if(ParamsObj->has("compress_sz")) {
					compress_sz = ParamsObj->get("compress_sz");
				}

				if (Utils::ExtractBase64CompressedData(CompressedData, UncompressedData, compress_sz)) {
					poco_trace(Logger(),fmt::format("EVENT({}): Found compressed payload expanded to '{}'.",
													  CId_, UncompressedData));
					Poco::JSON::Parser Parser;
					ParamsObj = Parser.parse(UncompressedData).extract<Poco::JSON::Object::Ptr>();
				} else {
					poco_warning(Logger(),fmt::format("INVALID-COMPRESSED-DATA({}): Compressed cannot be uncompressed - content must be corrupt..: size={}",
														CId_, CompressedData.size()));
					Errors_++;
					return;
				}
			} catch (const Poco::Exception &E) {
				poco_warning(Logger(),fmt::format("INVALID-COMPRESSED-JSON-DATA({}): Compressed cannot be parsed - JSON must be corrupt..",
													CId_));
				Logger().log(E);
				return;
			}
		}

		if (!ParamsObj->has(uCentralProtocol::SERIAL)) {
			poco_warning(Logger(),fmt::format("MISSING-PARAMS({}): Serial number is missing in message.", CId_));
			return;
		}

		auto Serial = Poco::trim(Poco::toLower(ParamsObj->get(uCentralProtocol::SERIAL).toString()));
		if (!Utils::ValidSerialNumber(Serial)) {
			Poco::Exception E(
				fmt::format(
					"ILLEGAL-DEVICE-NAME({}): device name is illegal and not allowed to connect.",
					Serial),
				EACCES);
			E.rethrow();
		}

		if (StorageService()->IsBlackListed(Serial)) {
			Poco::Exception E(
				fmt::format("BLACKLIST({}): device is blacklisted and not allowed to connect.",
							 Serial),
				EACCES);
			E.rethrow();
		}

		if (Conn_ != nullptr)
			Conn_->Conn_.LastContact = OpenWifi::Now();

		switch (EventType) {
			case uCentralProtocol::Events::ET_CONNECT: {
				Process_connect(ParamsObj, Serial);
			} break;

			case uCentralProtocol::Events::ET_STATE: {
				Process_state(ParamsObj);
			} break;

			case uCentralProtocol::Events::ET_HEALTHCHECK: {
				Process_healthcheck(ParamsObj, Serial);
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

			// 	this will never be called but some compilers will complain if we do not have a case for
			//	every single values of an enum
			case uCentralProtocol::Events::ET_UNKNOWN: {
				poco_warning(Logger(), fmt::format("ILLEGAL-EVENT({}): Event '{}' unknown. CN={}", CId_, Method, CN_));
				Errors_++;
			}
		}
	}

	bool AP_WS_Connection::StartTelemetry() {
		// std::cout << "Start telemetry for " << SerialNumber_ << std::endl;
		poco_information(Logger(), fmt::format("TELEMETRY({}): Starting.", CId_));
		Poco::JSON::Object StartMessage;
		StartMessage.set("jsonrpc", "2.0");
		StartMessage.set("method", "telemetry");
		Poco::JSON::Object Params;
		Params.set("serial", SerialNumber_);
		Params.set("interval", TelemetryInterval_);
		Poco::JSON::Array Types;
		Types.add("wifi-frames");
		Types.add("dhcp-snooping");
		Types.add("state");
		Params.set(RESTAPI::Protocol::TYPES, Types);
		StartMessage.set("id", 1);
		StartMessage.set("params", Params);
		Poco::JSON::Stringifier Stringify;
		std::ostringstream OS;
		Stringify.condense(StartMessage, OS);
		Send(OS.str());
		return true;
	}

	bool AP_WS_Connection::StopTelemetry() {
		// std::cout << "Stop telemetry for " << SerialNumber_ << std::endl;
		poco_information(Logger(), fmt::format("TELEMETRY({}): Stopping.", CId_));
		Poco::JSON::Object StopMessage;
		StopMessage.set("jsonrpc", "2.0");
		StopMessage.set("method", "telemetry");
		Poco::JSON::Object Params;
		Params.set("serial", SerialNumber_);
		Params.set("interval", 0);
		StopMessage.set("id", 1);
		StopMessage.set("params", Params);
		Poco::JSON::Stringifier Stringify;
		std::ostringstream OS;
		Stringify.condense(StopMessage, OS);
		Send(OS.str());
		TelemetryKafkaPackets_ = TelemetryWebSocketPackets_ = TelemetryInterval_ =
			TelemetryKafkaTimer_ = TelemetryWebSocketTimer_ = 0;
		return true;
	}

	void AP_WS_Connection::UpdateCounts() {
		if (Conn_) {
			Conn_->Conn_.kafkaClients = TelemetryKafkaRefCount_;
			Conn_->Conn_.webSocketClients = TelemetryWebSocketRefCount_;
		}
	}

	bool AP_WS_Connection::SetWebSocketTelemetryReporting(uint64_t Interval,
													  uint64_t LifeTime) {
		std::lock_guard G(Mutex_);
		TelemetryWebSocketRefCount_++;
		TelemetryInterval_ = TelemetryInterval_ ? std::min(Interval, TelemetryInterval_) : Interval;
		auto TelemetryWebSocketTimer = LifeTime + OpenWifi::Now();
		TelemetryWebSocketTimer_ = std::max(TelemetryWebSocketTimer, TelemetryWebSocketTimer_);
		UpdateCounts();
		if (!TelemetryReporting_) {
			TelemetryReporting_ = true;
			return StartTelemetry();
		}
		return true;
	}

	bool AP_WS_Connection::SetKafkaTelemetryReporting(uint64_t Interval, uint64_t LifeTime) {
		std::lock_guard G(Mutex_);
		TelemetryKafkaRefCount_++;
		TelemetryInterval_ = TelemetryInterval_ ? std::min(Interval, TelemetryInterval_) : Interval;
		auto TelemetryKafkaTimer = LifeTime + OpenWifi::Now();
		TelemetryKafkaTimer_ = std::max(TelemetryKafkaTimer, TelemetryKafkaTimer_);
		UpdateCounts();
		if (!TelemetryReporting_) {
			TelemetryReporting_ = true;
			return StartTelemetry();
		}
		return true;
	}

	bool AP_WS_Connection::StopWebSocketTelemetry() {
		std::lock_guard G(Mutex_);
		if (TelemetryWebSocketRefCount_)
			TelemetryWebSocketRefCount_--;
		UpdateCounts();
		if (TelemetryWebSocketRefCount_ == 0 && TelemetryKafkaRefCount_ == 0) {
			TelemetryReporting_ = false;
			StopTelemetry();
		}
		return true;
	}

	bool AP_WS_Connection::StopKafkaTelemetry() {
		std::lock_guard G(Mutex_);
		if (TelemetryKafkaRefCount_)
			TelemetryKafkaRefCount_--;
		UpdateCounts();
		if (TelemetryWebSocketRefCount_ == 0 && TelemetryKafkaRefCount_ == 0) {
			TelemetryReporting_ = false;
			StopTelemetry();
		}
		return true;
	}

	void AP_WS_Connection::OnSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		std::lock_guard Guard(Mutex_);
		poco_trace(Logger(), fmt::format("SOCKET-SHUTDOWN({}): Closing.", CId_));
		delete this;
	}

	void AP_WS_Connection::OnSocketError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		std::lock_guard Guard(Mutex_);
		poco_trace(Logger(), fmt::format("SOCKET-ERROR({}): Closing.", CId_));
		delete this;
	}

	void AP_WS_Connection::OnSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		std::lock_guard Guard(Mutex_);
		try {
			ProcessIncomingFrame();
		} catch (const Poco::Exception &E) {
			Logger().log(E);
			delete this;
		} catch (const std::exception &E) {
			std::string W = E.what();
			poco_information(Logger(), fmt::format("std::exception caught: {}. Connection terminated with {}", W, CId_));
			delete this;
		} catch (...) {
			poco_information(Logger(), fmt::format("Unknown exception for {}. Connection terminated.", CId_));
			delete this;
		}
	}

	void AP_WS_Connection::ProcessIncomingFrame() {
		Poco::Buffer<char> IncomingFrame(0);
		try {
			int Op, flags;
			auto IncomingSize = WS_->receiveFrame(IncomingFrame, flags);
			IncomingFrame.append(0);

			Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

			if (IncomingSize == 0 && flags == 0 && Op == 0) {
				poco_information(Logger(), fmt::format("DISCONNECT({}): device has disconnected.", CId_));
				return delete this;
			} else {
				if (Conn_ != nullptr) {
					Conn_->Conn_.RX += IncomingSize;
					Conn_->Conn_.MessageCount++;
				}

				switch (Op) {
				case Poco::Net::WebSocket::FRAME_OP_PING: {
					poco_trace(Logger(), fmt::format("WS-PING({}): received. PONG sent back.", CId_));
					WS_->sendFrame("", 0,
								   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
									   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
					if (Conn_ != nullptr) {
						Conn_->Conn_.MessageCount++;
					}

					if (KafkaManager()->Enabled() && Conn_) {
						Poco::JSON::Object PingObject;
						Poco::JSON::Object PingDetails;
						PingDetails.set(uCentralProtocol::FIRMWARE, Conn_->Conn_.Firmware);
						PingDetails.set(uCentralProtocol::SERIALNUMBER, SerialNumber_);
						PingDetails.set(uCentralProtocol::COMPATIBLE, Compatible_);
						PingDetails.set(uCentralProtocol::CONNECTIONIP, CId_);
						PingDetails.set(uCentralProtocol::TIMESTAMP, OpenWifi::Now());
						PingDetails.set("locale", Conn_->Conn_.locale );
						PingObject.set(uCentralProtocol::PING, PingDetails);
						Poco::JSON::Stringifier Stringify;
						std::ostringstream OS;
						Stringify.condense(PingObject, OS);
						KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber_, OS.str());
					}
					return;
				} break;

				case Poco::Net::WebSocket::FRAME_OP_PONG: {
					poco_trace(Logger(), fmt::format("PONG({}): received and ignored.", CId_));
					return;
				} break;

				case Poco::Net::WebSocket::FRAME_OP_TEXT: {
					poco_trace(Logger(), fmt::format("FRAME({}): Frame received (length={}, flags={}). Msg={}", CId_,
									 IncomingSize, flags, IncomingFrame.begin()));

					Poco::JSON::Parser parser;
					auto ParsedMessage = parser.parse(IncomingFrame.begin());
					auto IncomingJSON = ParsedMessage.extract<Poco::JSON::Object::Ptr>();

					if (IncomingJSON->has(uCentralProtocol::JSONRPC)) {
						if (IncomingJSON->has(uCentralProtocol::METHOD) &&
							IncomingJSON->has(uCentralProtocol::PARAMS)) {
							ProcessJSONRPCEvent(IncomingJSON);
						} else if (IncomingJSON->has(uCentralProtocol::RESULT) &&
								   IncomingJSON->has(uCentralProtocol::ID)) {
							poco_trace(Logger(), fmt::format("RPC-RESULT({}): payload: {}", CId_, IncomingFrame.begin()));
							ProcessJSONRPCResult(IncomingJSON);
						} else {
							poco_warning(Logger(),
								fmt::format("INVALID-PAYLOAD({}): Payload is not JSON-RPC 2.0: {}",
											 CId_, IncomingFrame.begin()));
						}
					} else if (IncomingJSON->has(uCentralProtocol::RADIUS)) {
						ProcessIncomingRadiusData(IncomingJSON);
					} else {
							std::ostringstream iS;
							IncomingJSON->stringify(iS);
							std::cout << iS.str() << std::endl;
							poco_warning(Logger(), fmt::format(
													   "FRAME({}): illegal transaction header, missing 'jsonrpc'", CId_));
							Errors_++;
					}
					return;
				} break;

				case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
					poco_information(Logger(),
						fmt::format("CLOSE({}): Device is closing its connection.", CId_));
					return delete this;
				} break;

				default: {
					poco_warning(Logger(), fmt::format("UNKNOWN({}): unknown WS Frame operation: {}", CId_,
												  std::to_string(Op)));
				} break;
				}
			}
		} catch (const Poco::Net::ConnectionResetException &E) {
			poco_warning(Logger(), fmt::format("ConnectionResetException({}): Text:{} Payload:{}",
				CId_,
				E.displayText(),
				IncomingFrame.begin()));
			return delete this;
		} catch (const Poco::JSON::JSONException &E) {
			poco_warning(Logger(), fmt::format("JSONException({}): Text:{} Payload:{}",
												CId_, E.displayText(), IncomingFrame.begin()));
		} catch (const Poco::Net::WebSocketException &E) {
			poco_warning(Logger(), fmt::format("WebSocketException({}): Text:{} Payload:{}",
											   CId_, E.displayText(), IncomingFrame.begin()));
			return delete this;
		} catch (const Poco::Net::SSLConnectionUnexpectedlyClosedException &E) {
			poco_warning(Logger(), fmt::format("SSLConnectionUnexpectedlyClosedException({}): Text:{} Payload:{}",
											   CId_, E.displayText(), IncomingFrame.begin()));
			return delete this;
		} catch (const Poco::Net::SSLException &E) {
			poco_warning(Logger(), fmt::format("SSLException({}): Text:{} Payload:{}",
											   CId_, E.displayText(), IncomingFrame.begin()));
			return delete this;
		} catch (const Poco::Net::NetException &E) {
			poco_warning(Logger(), fmt::format("NetException({}): Text:{} Payload:{}",
											   CId_, E.displayText(), IncomingFrame.begin()));
			return delete this;
		} catch (const Poco::IOException &E) {
			poco_warning(Logger(), fmt::format("IOException({}): Text:{} Payload:{}",
											   CId_, E.displayText(), IncomingFrame.begin()));
			return delete this;
		} catch (const Poco::Exception &E) {
			poco_warning(Logger(), fmt::format("Exception({}): Text:{} Payload:{}",
												CId_, E.displayText(), IncomingFrame.begin()));
			return delete this;
		} catch (const std::exception &E) {
			poco_warning(Logger(), fmt::format("std::exception({}): Text:{} Payload:{}",
											   CId_, E.what(), IncomingFrame.begin()));
			return delete this;
		} catch (...) {
			poco_error(Logger(),fmt::format("UnknownException({}): Device must be disconnected. Unknown exception.", CId_));
			return delete this;
		}

		if (Errors_ < 10)
			return;

		poco_warning(Logger(), fmt::format("DISCONNECTING({}): Too many errors", CId_));
		delete this;
	}

	bool AP_WS_Connection::Send(const std::string &Payload) {
		std::lock_guard Guard(Mutex_);

		size_t BytesSent = WS_->sendFrame(Payload.c_str(), (int)Payload.size());
		if (Conn_)
			Conn_->Conn_.TX += BytesSent;
		return BytesSent == Payload.size();
	}

	std::string Base64Encode(const unsigned char *buffer, std::size_t size) {
		return Utils::base64encode(buffer,size);
	}

	std::string Base64Decode(const std::string &F) {
		std::istringstream ifs(F);
		Poco::Base64Decoder b64in(ifs);
		std::ostringstream ofs;
		Poco::StreamCopier::copyStream(b64in, ofs);
		return ofs.str();
	}

	bool AP_WS_Connection::SendRadiusAuthenticationData(const unsigned char * buffer, std::size_t size) {
		Poco::JSON::Object	Answer;
		Answer.set(uCentralProtocol::RADIUS,uCentralProtocol::RADIUSAUTH);
		Answer.set(uCentralProtocol::RADIUSDATA, Base64Encode(buffer,size));

		std::ostringstream Payload;
		Answer.stringify(Payload);
		return Send(Payload.str());
	}

	bool AP_WS_Connection::SendRadiusAccountingData(const unsigned char * buffer, std::size_t size) {
		Poco::JSON::Object	Answer;
		Answer.set(uCentralProtocol::RADIUS,uCentralProtocol::RADIUSACCT);
		Answer.set(uCentralProtocol::RADIUSDATA, Base64Encode(buffer,size));

		std::ostringstream Payload;
		Answer.stringify(Payload);
		return Send(Payload.str());
	}

	bool AP_WS_Connection::SendRadiusCoAData(const unsigned char * buffer, std::size_t size) {
		Poco::JSON::Object	Answer;
		Answer.set(uCentralProtocol::RADIUS,uCentralProtocol::RADIUSCOA);
		Answer.set(uCentralProtocol::RADIUSDATA, Base64Encode(buffer,size));

		std::ostringstream Payload;
		Answer.stringify(Payload);
		return Send(Payload.str());
	}

	void AP_WS_Connection::ProcessIncomingRadiusData(const Poco::JSON::Object::Ptr &Doc) {
		if( Doc->has(uCentralProtocol::RADIUSDATA)) {
			auto Type = Doc->get(uCentralProtocol::RADIUS).toString();
			if(Type==uCentralProtocol::RADIUSACCT) {
				auto Data = Doc->get(uCentralProtocol::RADIUSDATA).toString();
				auto DecodedData = Base64Decode(Data);
				RADIUS_proxy_server()->SendAccountingData(SerialNumber_,DecodedData.c_str(),DecodedData.size());
			} else if(Type==uCentralProtocol::RADIUSAUTH) {
				auto Data = Doc->get(uCentralProtocol::RADIUSDATA).toString();
				auto DecodedData = Base64Decode(Data);
				RADIUS_proxy_server()->SendAuthenticationData(SerialNumber_,DecodedData.c_str(),DecodedData.size());
			} else if(Type==uCentralProtocol::RADIUSCOA) {
				auto Data = Doc->get(uCentralProtocol::RADIUSDATA).toString();
				auto DecodedData = Base64Decode(Data);
				RADIUS_proxy_server()->SendCoAData(SerialNumber_,DecodedData.c_str(),DecodedData.size());
			}
		}
	}
}