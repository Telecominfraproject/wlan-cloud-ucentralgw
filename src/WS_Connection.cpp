//
// Created by stephane bourque on 2022-02-03.
//

#include "WS_Connection.h"

#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/HTTPServerResponseImpl.h"
#include "Poco/Net/HTTPServerSession.h"
#include "Poco/Net/HTTPServerRequestImpl.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/SSLException.h"
#include "Poco/Base64Decoder.h"
#include "Poco/zlib.h"

#include "WS_Server.h"
#include "StorageService.h"
#include "CommandManager.h"
#include "StateUtils.h"
#include "ConfigurationCache.h"
#include "SerialNumberCache.h"
#include "Daemon.h"
#include "TelemetryStream.h"
#include "CentralConfig.h"
#include "FindCountry.h"

namespace OpenWifi {
	void WSConnection::LogException(const Poco::Exception &E) {
		Logger().information(Poco::format("EXCEPTION(%s): %s", CId_, E.displayText()));
	}

	void WSConnection::CompleteStartup() {
		std::lock_guard Guard(Mutex_);
		try {
			auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(Socket_.impl());
			while (true) {
				auto V = SS->completeHandshake();
				if (V == 1)
					break;
			}
			PeerAddress_ = SS->peerAddress().host();
			CId_ = Utils::FormatIPv6(SS->peerAddress().toString());
			if (!SS->secure()) {
				Logger().error(Poco::format("%s: Connection is NOT secure.", CId_));
			} else {
				Logger().debug(Poco::format("%s: Connection is secure.", CId_));
			}

			if (SS->havePeerCertificate()) {
				// Get the cert info...
				CertValidation_ = GWObjects::VALID_CERTIFICATE;
				try {
					Poco::Crypto::X509Certificate PeerCert(SS->peerCertificate());

					if (WebSocketServer()->ValidateCertificate(CId_, PeerCert)) {
						CN_ = Poco::trim(Poco::toLower(PeerCert.commonName()));
						CertValidation_ = GWObjects::MISMATCH_SERIAL;
						Logger().debug(Poco::format("%s: Valid certificate: CN=%s", CId_, CN_));
					} else {
						Logger().debug(Poco::format("%s: Certificate is not valid", CId_));
					}
				} catch (const Poco::Exception &E) {
					LogException(E);
				}
			} else {
				Logger().error(Poco::format("%s: No certificates available..", CId_));
			}

			if (WebSocketServer::IsSim(CN_) && !WebSocketServer()->IsSimEnabled()) {
				Logger().debug(Poco::format(
					"CONNECTION(%s): Sim Device %s is not allowed. Disconnecting.", CId_, CN_));
				delete this;
				return;
			}

			SerialNumber_ = CN_;
			SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);

			if (!CN_.empty() && StorageService()->IsBlackListed(SerialNumber_)) {
				Logger().debug(Poco::format("CONNECTION(%s): Device %s is black listed. Disconnecting.",
											CId_, CN_));
				return delete this;
			}
			auto Params = Poco::AutoPtr<Poco::Net::HTTPServerParams>(new Poco::Net::HTTPServerParams);
			Poco::Net::HTTPServerSession Session(Socket_, Params);
			Poco::Net::HTTPServerResponseImpl Response(Session);
			Poco::Net::HTTPServerRequestImpl Request(Response, Session, Params);

			auto Now = std::time(nullptr);
			Response.setDate(Now);
			Response.setVersion(Request.getVersion());
			Response.setKeepAlive(Params->getKeepAlive() && Request.getKeepAlive() &&
								  Session.canKeepAlive());
			WS_ = std::make_unique<Poco::Net::WebSocket>(Request, Response);
			WS_->setMaxPayloadSize(BufSize);
			auto TS = Poco::Timespan(360, 0);

			WS_->setReceiveTimeout(TS);
			WS_->setNoDelay(true);
			WS_->setKeepAlive(true);

			Reactor_.addEventHandler(*WS_,
									 Poco::NObserver<WSConnection, Poco::Net::ReadableNotification>(
										 *this, &WSConnection::OnSocketReadable));
			Reactor_.addEventHandler(*WS_,
									 Poco::NObserver<WSConnection, Poco::Net::ShutdownNotification>(
										 *this, &WSConnection::OnSocketShutdown));
			Reactor_.addEventHandler(*WS_, Poco::NObserver<WSConnection, Poco::Net::ErrorNotification>(
											   *this, &WSConnection::OnSocketError));
			Registered_ = true;
			Logger().information(Poco::format("CONNECTION(%s): completed.", CId_));
			return;
		} catch (const Poco::Net::CertificateValidationException &E) {
			Logger().error(Poco::format("CONNECTION(%s): Poco::Exception Certificate Validation failed during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Net::WebSocketException &E) {
			Logger().error(Poco::format("CONNECTION(%s): Poco::Exception WebSocket error during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Net::ConnectionAbortedException &E) {
			Logger().error(Poco::format("CONNECTION(%s): Poco::Exception Connection was aborted during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Net::ConnectionResetException &E) {
			Logger().error(Poco::format("CONNECTION(%s): Poco::Exception Connection was reset during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Net::InvalidCertificateException &E) {
			Logger().error(Poco::format(
				"CONNECTION(%s): Poco::Exception Invalid certificate. Device will have to retry.",
				CId_));
			Logger().log(E);
		} catch (const Poco::Net::SSLException &E) {
			Logger().error(Poco::format("CONNECTION(%s): Poco::Exception SSL Exception during connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (const Poco::Exception &E) {
			Logger().error(Poco::format("CONNECTION(%s): Poco::Exception caught during device connection. Device will have to retry.",
										CId_));
			Logger().log(E);
		} catch (...) {
			Logger().error(Poco::format("CONNECTION(%s): Exception caught during device connection. Device will have to retry. Unsecure connect denied.",
										CId_));
		}
		return delete this;
	}

	WSConnection::WSConnection(Poco::Net::StreamSocket &socket, Poco::Net::SocketReactor &reactor)
		: Socket_(socket), Reactor_(WebSocketServer()->GetNextReactor()),
		  Logger_(WebSocketServer()->Logger()) {
		std::thread T([=]() { CompleteStartup(); });
		T.detach();
		//		CompleteStartup();
	}

	static void NotifyKafkaDisconnect(const std::string & SerialNumber) {
		try {
			Poco::JSON::Object Disconnect;
			Poco::JSON::Object Details;
			Details.set(uCentralProtocol::SERIALNUMBER, SerialNumber);
			Details.set(uCentralProtocol::TIMESTAMP, std::time(nullptr));
			Disconnect.set(uCentralProtocol::DISCONNECTION, Details);
			Poco::JSON::Stringifier Stringify;
			std::ostringstream OS;
			Stringify.condense(Disconnect, OS);
			KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber, OS.str());
		} catch (...) {
		}
	}

	WSConnection::~WSConnection() {

		if (ConnectionId_)
			DeviceRegistry()->UnRegister(SerialNumberInt_, ConnectionId_);

		if (Registered_ && WS_) {
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<WSConnection, Poco::Net::ReadableNotification>(
											*this, &WSConnection::OnSocketReadable));
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<WSConnection, Poco::Net::ShutdownNotification>(
											*this, &WSConnection::OnSocketShutdown));
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<WSConnection, Poco::Net::ErrorNotification>(
											*this, &WSConnection::OnSocketError));
			(*WS_).close();
			Socket_.shutdown();
		} else if (WS_) {
			(*WS_).close();
			Socket_.shutdown();
		}

		if (KafkaManager()->Enabled() && !SerialNumber_.empty()) {
			std::string s(SerialNumber_);
			std::thread t([s]() { NotifyKafkaDisconnect(s); });
			t.detach();
		}
	}

	bool WSConnection::LookForUpgrade(uint64_t UUID) {

		//	A UUID of zero means ignore updates for that connection.
		if (UUID == 0)
			return false;

		uint64_t GoodConfig = ConfigurationCache().CurrentConfig(SerialNumberInt_);
		if (GoodConfig && (GoodConfig == UUID || GoodConfig == Conn_->Conn_.PendingUUID))
			return false;

		GWObjects::Device D;
		if (StorageService()->GetDevice(SerialNumber_, D)) {

			//	This is the case where the cache is empty after a restart. So GoodConfig will 0. If the device already 	has the right UUID, we just return.
			if (D.UUID == UUID) {
				ConfigurationCache().Add(SerialNumberInt_, UUID);
				return false;
			}

			if(UUID>D.UUID) {
				//	so we have a problem, the device has a newer config than we have. So we need to make sure our config
				//	is newer.
				Config::Config	Cfg(D.Configuration);
				D.UUID = UUID+2;
				Cfg.SetUUID(D.UUID);
				D.Configuration = Cfg.get();
				StorageService()->UpdateDevice(D);
			}

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
			poco_information(Logger(),Poco::format("CFG-UPGRADE(%s): Current ID: %Lu, newer configuration %Lu.",
										   CId_, UUID, D.UUID));
			bool Sent;

			StorageService()->AddCommand(SerialNumber_, Cmd, Storage::COMMAND_EXECUTED);
			CommandManager()->PostCommand(SerialNumber_, Cmd.Command, Params, Cmd.UUID, Sent);

			return true;
		}
		return false;
	}

	bool WSConnection::ExtractBase64CompressedData(const std::string &CompressedData,
												   std::string &UnCompressedData) {
		std::istringstream ifs(CompressedData);
		Poco::Base64Decoder b64in(ifs);
		std::ostringstream ofs;
		Poco::StreamCopier::copyStream(b64in, ofs);

		unsigned long MaxSize = ofs.str().size() * 10;
		std::vector<uint8_t> UncompressedBuffer(MaxSize);
		unsigned long FinalSize = MaxSize;
		if (uncompress((uint8_t *)&UncompressedBuffer[0], &FinalSize, (uint8_t *)ofs.str().c_str(),
					   ofs.str().size()) == Z_OK) {
			UncompressedBuffer[FinalSize] = 0;
			UnCompressedData = (char *)&UncompressedBuffer[0];
			return true;
		}
		return false;
	}

	void WSConnection::ProcessJSONRPCResult(Poco::JSON::Object::Ptr &Doc) {
		CommandManager()->PostCommandResult(SerialNumber_, Doc);
	}

	void WSConnection::ProcessJSONRPCEvent(Poco::JSON::Object::Ptr &Doc) {

		auto Method = Doc->get(uCentralProtocol::METHOD).toString();
		auto EventType = uCentralProtocol::Events::EventFromString(Method);
		if (EventType == uCentralProtocol::Events::ET_UNKNOWN) {
			poco_warning(Logger(),Poco::format("ILLEGAL-PROTOCOL(%s): Unknown message type '%s'", CId_, Method));
			Errors_++;
			return;
		}

		if (!Doc->isObject(uCentralProtocol::PARAMS)) {
			poco_warning(Logger(),Poco::format("MISSING-PARAMS(%s): params must be an object.", CId_));
			Errors_++;
			return;
		}

		//  expand params if necessary
		auto ParamsObj = Doc->get(uCentralProtocol::PARAMS).extract<Poco::JSON::Object::Ptr>();
		if (ParamsObj->has(uCentralProtocol::COMPRESS_64)) {
			std::string UncompressedData;
			try {
				if (ExtractBase64CompressedData(
						ParamsObj->get(uCentralProtocol::COMPRESS_64).toString(), UncompressedData)) {
					poco_trace(Logger(),Poco::format("EVENT(%s): Found compressed payload expanded to '%s'.",
													  CId_, UncompressedData));
					Poco::JSON::Parser Parser;
					ParamsObj = Parser.parse(UncompressedData).extract<Poco::JSON::Object::Ptr>();
				} else {
					poco_warning(Logger(),Poco::format("INVALID-COMPRESSED-DATA(%s): Compressed cannot be uncompressed - content must be corrupt..",
														CId_));
					Errors_++;
					return;
				}
			} catch (const Poco::Exception &E) {
				poco_warning(Logger(),Poco::format("INVALID-COMPRESSED-JSON-DATA(%s): Compressed cannot be parsed - JSON must be corrupt..",
													CId_));
				Logger().log(E);
				return;
			}
		}

		if (!ParamsObj->has(uCentralProtocol::SERIAL)) {
			poco_warning(Logger(),Poco::format("MISSING-PARAMS(%s): Serial number is missing in message.", CId_));
			return;
		}

		auto Serial = Poco::trim(Poco::toLower(ParamsObj->get(uCentralProtocol::SERIAL).toString()));
		if (!Utils::ValidSerialNumber(Serial)) {
			Poco::Exception E(
				Poco::format(
					"ILLEGAL-DEVICE-NAME(%s): device name is illegal and not allowed to connect.",
					Serial),
				EACCES);
			E.rethrow();
		}

		if (StorageService()->IsBlackListed(Serial)) {
			Poco::Exception E(
				Poco::format("BLACKLIST(%s): device is blacklisted and not allowed to connect.",
							 Serial),
				EACCES);
			E.rethrow();
		}

		if (Conn_ != nullptr)
			Conn_->Conn_.LastContact = std::time(nullptr);

		switch (EventType) {
		case uCentralProtocol::Events::ET_CONNECT: {
			if (ParamsObj->has(uCentralProtocol::UUID) &&
				ParamsObj->has(uCentralProtocol::FIRMWARE) &&
				ParamsObj->has(uCentralProtocol::CAPABILITIES)) {
				uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
				auto Firmware = ParamsObj->get(uCentralProtocol::FIRMWARE).toString();
				auto Capabilities = ParamsObj->get(uCentralProtocol::CAPABILITIES).toString();

				SerialNumber_ = Serial;
				SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);
				Conn_ = DeviceRegistry()->Register(SerialNumberInt_, this, ConnectionId_);
				Conn_->Conn_.UUID = UUID;
				Conn_->Conn_.Firmware = Firmware;
				Conn_->Conn_.PendingUUID = 0;
				Conn_->Conn_.LastContact = std::time(nullptr);
				Conn_->Conn_.Address = Utils::FormatIPv6(WS_->peerAddress().toString());
				CId_ = SerialNumber_ + "@" + CId_;
				//	We need to verify the certificate if we have one
				if ((!CN_.empty() && Utils::SerialNumberMatch(CN_, SerialNumber_)) ||
					WebSocketServer()->IsSimSerialNumber(CN_)) {
					CertValidation_ = GWObjects::VERIFIED;
					poco_information(Logger(),Poco::format("CONNECT(%s): Fully validated and authenticated device..", CId_));
				} else {
					if (CN_.empty())
						poco_information(Logger(),Poco::format("CONNECT(%s): Not authenticated or validated.", CId_));
					else
						poco_information(Logger(),Poco::format(
													   "CONNECT(%s): Authenticated but not validated. Serial='%s' CN='%s'", CId_,
													   Serial, CN_));
				}
				Conn_->Conn_.VerifiedCertificate = CertValidation_;
				auto IP = PeerAddress_.toString();
				if(IP.substr(0,7)=="::ffff:") {
					IP = IP.substr(7);
				}
				Conn_->Conn_.locale = FindCountryFromIP()->Get(IP);
				GWObjects::Device	DeviceInfo;
				auto DeviceExists = StorageService()->GetDevice(SerialNumber_,DeviceInfo);
				if (Daemon()->AutoProvisioning() && !DeviceExists) {
					StorageService()->CreateDefaultDevice(SerialNumber_, Capabilities, Firmware,
														  Compatible_, PeerAddress_);
				} else if (DeviceExists) {
					StorageService()->UpdateDeviceCapabilities(SerialNumber_, Capabilities,
															   Compatible_);
					bool Updated = false;
					if(!Firmware.empty() && Firmware!=DeviceInfo.Firmware) {
						DeviceInfo.Firmware = Firmware;
						Updated = true;
					}
					if(DeviceInfo.locale != Conn_->Conn_.locale) {
						DeviceInfo.locale = Conn_->Conn_.locale;
						Updated = true;
					}
					if(Compatible_ != DeviceInfo.DeviceType) {
						DeviceInfo.DeviceType = Compatible_;
						Updated = true;
					}
					if(Updated) {
						StorageService()->UpdateDevice(DeviceInfo);
					}
					LookForUpgrade(UUID);
				}

				Conn_->Conn_.Compatible = Compatible_;

				if (KafkaManager()->Enabled()) {
					Poco::JSON::Stringifier Stringify;
					ParamsObj->set(uCentralProtocol::CONNECTIONIP, CId_);
					ParamsObj->set("locale", Conn_->Conn_.locale );
					std::ostringstream OS;
					Stringify.condense(ParamsObj, OS);
					KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber_, OS.str());
				}
				Connected_ = true;
			} else {
				poco_warning(Logger(),Poco::format("INVALID-PROTOCOL(%s): Missing one of uuid, firmware, or capabilities", CId_));
				Errors_++;
				return;
			}
		} break;

		case uCentralProtocol::Events::ET_STATE: {
			if (!Connected_) {
				poco_warning(Logger(),Poco::format(
										   "INVALID-PROTOCOL(%s): Device '%s' is not following protocol", CId_, CN_));
				Errors_++;
				return;
			}
			if (ParamsObj->has(uCentralProtocol::UUID) && ParamsObj->has(uCentralProtocol::STATE)) {
				uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
				auto StateStr = ParamsObj->get(uCentralProtocol::STATE).toString();
				auto StateObj = ParamsObj->getObject(uCentralProtocol::STATE);

				std::string request_uuid;
				if (ParamsObj->has(uCentralProtocol::REQUEST_UUID))
					request_uuid = ParamsObj->get(uCentralProtocol::REQUEST_UUID).toString();

				if (request_uuid.empty())
					poco_trace(Logger(),Poco::format("STATE(%s): UUID=%Lu Updating.", CId_, UUID));
				else
					poco_trace(Logger(),Poco::format("STATE(%s): UUID=%Lu Updating for CMD=%s.", CId_, UUID,
													  request_uuid));
				Conn_->Conn_.UUID = UUID;
				Conn_->LastStats = StateStr;

				LookForUpgrade(UUID);
				GWObjects::Statistics Stats{
					.SerialNumber = SerialNumber_, .UUID = UUID, .Data = StateStr};
				Stats.Recorded = std::time(nullptr);
				StorageService()->AddStatisticsData(Stats);
				if (!request_uuid.empty()) {
					StorageService()->SetCommandResult(request_uuid, StateStr);
				}

				StateUtils::ComputeAssociations(StateObj, Conn_->Conn_.Associations_2G,
												Conn_->Conn_.Associations_5G);

				if (KafkaManager()->Enabled()) {
					Poco::JSON::Stringifier Stringify;
					std::ostringstream OS;
					Stringify.condense(ParamsObj, OS);
					KafkaManager()->PostMessage(KafkaTopics::STATE, SerialNumber_, OS.str());
				}
			} else {
				poco_warning(Logger(),Poco::format("STATE(%s): Invalid request. Missing serial, uuid, or state", CId_));
			}
		} break;

		case uCentralProtocol::Events::ET_HEALTHCHECK: {
			if (!Connected_) {
				poco_warning(Logger(),Poco::format(
										   "INVALID-PROTOCOL(%s): Device '%s' is not following protocol", CId_, CN_));
				Errors_++;
				return;
			}
			if (ParamsObj->has(uCentralProtocol::UUID) && ParamsObj->has(uCentralProtocol::SANITY) &&
				ParamsObj->has(uCentralProtocol::DATA)) {
				uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
				auto Sanity = ParamsObj->get(uCentralProtocol::SANITY);
				auto CheckData = ParamsObj->get(uCentralProtocol::DATA).toString();
				if (CheckData.empty())
					CheckData = uCentralProtocol::EMPTY_JSON_DOC;

				std::string request_uuid;
				if (ParamsObj->has(uCentralProtocol::REQUEST_UUID))
					request_uuid = ParamsObj->get(uCentralProtocol::REQUEST_UUID).toString();

				if (request_uuid.empty())
					poco_trace(Logger(),Poco::format("HEALTHCHECK(%s): UUID=%Lu Updating.", CId_, UUID));
				else
					poco_trace(Logger(),Poco::format("HEALTHCHECK(%s): UUID=%Lu Updating for CMD=%s.", CId_,
												UUID, request_uuid));

				Conn_->Conn_.UUID = UUID;
				LookForUpgrade(UUID);

				GWObjects::HealthCheck Check;

				Check.SerialNumber = SerialNumber_;
				Check.Recorded = std::time(nullptr);
				Check.UUID = UUID;
				Check.Data = CheckData;
				Check.Sanity = Sanity;

				StorageService()->AddHealthCheckData(Check);

				if (!request_uuid.empty()) {
					StorageService()->SetCommandResult(request_uuid, CheckData);
				}

				DeviceRegistry()->SetHealthcheck(Serial, Check);
				if (KafkaManager()->Enabled()) {
					Poco::JSON::Stringifier Stringify;
					std::ostringstream OS;
					ParamsObj->set("timestamp", std::time(nullptr));
					Stringify.condense(ParamsObj, OS);
					KafkaManager()->PostMessage(KafkaTopics::HEALTHCHECK, SerialNumber_, OS.str());
				}
			} else {
				poco_warning(Logger(),Poco::format("HEALTHCHECK(%s): Missing parameter", CId_));
				return;
			}
		} break;

		case uCentralProtocol::Events::ET_LOG: {
			if (!Connected_) {
				poco_warning(Logger(),Poco::format(
										   "INVALID-PROTOCOL(%s): Device '%s' is not following protocol", CId_, CN_));
				Errors_++;
				return;
			}
			if (ParamsObj->has(uCentralProtocol::LOG) && ParamsObj->has(uCentralProtocol::SEVERITY)) {
				poco_trace(Logger(),Poco::format("LOG(%s): new entry.", CId_));
				auto Log = ParamsObj->get(uCentralProtocol::LOG).toString();
				auto Severity = ParamsObj->get(uCentralProtocol::SEVERITY);
				std::string DataStr = uCentralProtocol::EMPTY_JSON_DOC;
				if (ParamsObj->has(uCentralProtocol::DATA)) {
					auto DataObj = ParamsObj->get(uCentralProtocol::DATA);
					if (DataObj.isStruct())
						DataStr = DataObj.toString();
				}

				GWObjects::DeviceLog DeviceLog{.SerialNumber = SerialNumber_,
											   .Log = Log,
											   .Data = DataStr,
											   .Severity = Severity,
											   .Recorded = (uint64_t)time(nullptr),
											   .LogType = 0,
											   .UUID = Conn_->Conn_.UUID};
				StorageService()->AddLog(DeviceLog);
			} else {
				poco_warning(Logger(),Poco::format("LOG(%s): Missing parameters.", CId_));
				return;
			}
		} break;

		case uCentralProtocol::Events::ET_CRASHLOG: {
			if (ParamsObj->has(uCentralProtocol::UUID) && ParamsObj->has(uCentralProtocol::LOGLINES)) {
				poco_trace(Logger(),Poco::format("CRASH-LOG(%s): new entry.", CId_));
				auto LogLines = ParamsObj->get(uCentralProtocol::LOGLINES);
				std::string LogText;
				if (LogLines.isArray()) {
					auto LogLinesArray = LogLines.extract<Poco::JSON::Array::Ptr>();
					for (const auto &i : *LogLinesArray)
						LogText += i.toString() + "\r\n";
				}

				GWObjects::DeviceLog DeviceLog{.SerialNumber = SerialNumber_,
											   .Log = LogText,
											   .Data = "",
											   .Severity = GWObjects::DeviceLog::LOG_EMERG,
											   .Recorded = (uint64_t)time(nullptr),
											   .LogType = 1,
											   .UUID = 0};
				StorageService()->AddLog(DeviceLog);

			} else {
				poco_warning(Logger(), Poco::format("LOG(%s): Missing parameters.", CId_));
				return;
			}
		} break;

		case uCentralProtocol::Events::ET_PING: {
			if (ParamsObj->has(uCentralProtocol::UUID)) {
				uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
				poco_trace(Logger(),Poco::format("PING(%s): Current config is %Lu", CId_, UUID));
			} else {
				poco_warning(Logger(),Poco::format("PING(%s): Missing parameter.", CId_));
			}
		} break;

		case uCentralProtocol::Events::ET_CFGPENDING: {
			if (!Connected_) {
				poco_warning(Logger(),Poco::format(
										   "INVALID-PROTOCOL(%s): Device '%s' is not following protocol", CId_, CN_));
				Errors_++;
				return;
			}
			if (ParamsObj->has(uCentralProtocol::UUID) && ParamsObj->has(uCentralProtocol::ACTIVE)) {

				uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
				uint64_t Active = ParamsObj->get(uCentralProtocol::ACTIVE);
				poco_trace(Logger(),Poco::format("CFG-PENDING(%s): Active: %Lu Target: %Lu", CId_, Active, UUID));
			} else {
				poco_warning(Logger(),Poco::format("CFG-PENDING(%s): Missing some parameters", CId_));
			}
		} break;

		case uCentralProtocol::Events::ET_RECOVERY: {
			if (ParamsObj->has(uCentralProtocol::SERIAL) &&
				ParamsObj->has(uCentralProtocol::FIRMWARE) && ParamsObj->has(uCentralProtocol::UUID) &&
				ParamsObj->has(uCentralProtocol::REBOOT) &&
				ParamsObj->has(uCentralProtocol::LOGLINES)) {

				auto LogLines = ParamsObj->get(uCentralProtocol::LOGLINES);
				std::string LogText;

				LogText = "Firmware: " + ParamsObj->get(uCentralProtocol::FIRMWARE).toString() + "\r\n";
				if (LogLines.isArray()) {
					auto LogLinesArray = LogLines.extract<Poco::JSON::Array::Ptr>();
					for (const auto &i : *LogLinesArray)
						LogText += i.toString() + "\r\n";
				}

				GWObjects::DeviceLog DeviceLog{.SerialNumber = SerialNumber_,
											   .Log = LogText,
											   .Data = "",
											   .Severity = GWObjects::DeviceLog::LOG_EMERG,
											   .Recorded = (uint64_t)time(nullptr),
											   .LogType = 1,
											   .UUID = 0};

				StorageService()->AddLog(DeviceLog);

				if (ParamsObj->get(uCentralProtocol::REBOOT).toString() == "true") {
					GWObjects::CommandDetails Cmd;
					Cmd.SerialNumber = SerialNumber_;
					Cmd.UUID = MicroService::CreateUUID();
					Cmd.SubmittedBy = uCentralProtocol::SUBMITTED_BY_SYSTEM;
					Cmd.Status = uCentralProtocol::PENDING;
					Cmd.Command = uCentralProtocol::REBOOT;
					Poco::JSON::Object Params;
					Params.set(uCentralProtocol::SERIAL, SerialNumber_);
					Params.set(uCentralProtocol::WHEN, 0);
					std::ostringstream O;
					Poco::JSON::Stringifier::stringify(Params, O);
					Cmd.Details = O.str();
					bool Sent;
					CommandManager()->PostCommand(SerialNumber_, Cmd.Command, Params, Cmd.UUID, Sent);
					StorageService()->AddCommand(SerialNumber_, Cmd, Storage::COMMAND_EXECUTED);
					poco_information(Logger(),Poco::format("RECOVERY(%s): Recovery mode received, need for a reboot.", CId_));
				} else {
					poco_information(Logger(),Poco::format(
												   "RECOVERY(%s): Recovery mode received, no need for a reboot.", CId_));
				}
			} else {
				poco_warning(Logger(),Poco::format("RECOVERY(%s): Recovery missing one of serialnumber, firmware, uuid, loglines, reboot",
													CId_));
			}
		} break;

		case uCentralProtocol::Events::ET_DEVICEUPDATE: {
			if (!Connected_) {
				poco_warning(Logger(),Poco::format(
										   "INVALID-PROTOCOL(%s): Device '%s' is not following protocol", CId_, CN_));
				Errors_++;
				return;
			}
			if (ParamsObj->has("currentPassword")) {
				auto Password = ParamsObj->get("currentPassword").toString();

				StorageService()->SetDevicePassword(Serial, Password);
				poco_trace(Logger(),Poco::format("DEVICEUPDATE(%s): Device is updating its login password.", Serial));
			}
		} break;

		case uCentralProtocol::Events::ET_TELEMETRY: {
			if (!Connected_) {
				poco_warning(Logger(),Poco::format(
										   "INVALID-PROTOCOL(%s): Device '%s' is not following protocol", CId_, CN_));
				Errors_++;
				return;
			}
			if (TelemetryReporting_) {
				if (ParamsObj->has("data")) {
					auto Payload = ParamsObj->get("data").extract<Poco::JSON::Object::Ptr>();
					Payload->set("timestamp", std::time(nullptr));
					std::ostringstream SS;
					Payload->stringify(SS);
					if (TelemetryWebSocketRefCount_) {
						std::cout << SerialNumber_ << ": Updating WebSocket telemetry" << std::endl;
						TelemetryWebSocketPackets_++;
						Conn_->Conn_.websocketPackets = TelemetryWebSocketPackets_;
						TelemetryStream()->UpdateEndPoint(SerialNumberInt_, SS.str());
					}
					if (TelemetryKafkaRefCount_ && KafkaManager()->Enabled()) {
						std::cout << SerialNumber_ << ": Updating Kafka telemetry" << std::endl;
						TelemetryKafkaPackets_++;
						Conn_->Conn_.kafkaPackets = TelemetryKafkaPackets_;
						KafkaManager()->PostMessage(KafkaTopics::DEVICE_TELEMETRY, SerialNumber_,
													SS.str());
					}
				} else {
					std::cout << SerialNumber_ << ": Invalid telemetry" << std::endl;
				}
			} else {
				std::cout << SerialNumber_ << ":Ignoring telemetry" << std::endl;
			}
		} break;

		// 	this will never be called but some compilers will complain if we do not have a case for
		//	every single values of an enum
		case uCentralProtocol::Events::ET_UNKNOWN: {
			poco_warning(Logger(),Poco::format("ILLEGAL-EVENT(%s): Event '%s' unknown. CN=%s", CId_, Method, CN_));
			Errors_++;
		}
		}
	}

	bool WSConnection::StartTelemetry() {
		std::cout << "Start telemetry for " << SerialNumber_ << std::endl;
		poco_information(Logger(),Poco::format("TELEMETRY(%s): Starting.", CId_));
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

	bool WSConnection::StopTelemetry() {
		std::cout << "Stop telemetry for " << SerialNumber_ << std::endl;
		poco_information(Logger(),Poco::format("TELEMETRY(%s): Stopping.", CId_));
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

	void WSConnection::UpdateCounts() {
		if (Conn_) {
			Conn_->Conn_.kafkaClients = TelemetryKafkaRefCount_;
			Conn_->Conn_.webSocketClients = TelemetryWebSocketRefCount_;
		}
	}

	bool WSConnection::SetWebSocketTelemetryReporting(uint64_t Interval,
													  uint64_t TelemetryWebSocketTimer) {
		std::lock_guard G(Mutex_);
		TelemetryWebSocketRefCount_++;
		TelemetryInterval_ = TelemetryInterval_ ? std::min(Interval, TelemetryInterval_) : Interval;
		TelemetryWebSocketTimer_ = std::max(TelemetryWebSocketTimer, TelemetryWebSocketTimer_);
		UpdateCounts();
		if (!TelemetryReporting_) {
			TelemetryReporting_ = true;
			return StartTelemetry();
		}
		return true;
	}

	bool WSConnection::SetKafkaTelemetryReporting(uint64_t Interval, uint64_t TelemetryKafkaTimer) {
		std::lock_guard G(Mutex_);
		TelemetryKafkaRefCount_++;
		TelemetryInterval_ = TelemetryInterval_ ? std::min(Interval, TelemetryInterval_) : Interval;
		TelemetryKafkaTimer_ = std::max(TelemetryKafkaTimer, TelemetryKafkaTimer_);
		UpdateCounts();
		if (!TelemetryReporting_) {
			TelemetryReporting_ = true;
			return StartTelemetry();
		}
		return true;
	}

	bool WSConnection::StopWebSocketTelemetry() {
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

	bool WSConnection::StopKafkaTelemetry() {
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

	void WSConnection::OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		std::lock_guard Guard(Mutex_);
		poco_trace(Logger(),Poco::format("SOCKET-SHUTDOWN(%s): Closing.", CId_));
		delete this;
	}

	void WSConnection::OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		std::lock_guard Guard(Mutex_);
		poco_trace(Logger(),Poco::format("SOCKET-ERROR(%s): Closing.", CId_));
		delete this;
	}

	void WSConnection::OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		std::lock_guard Guard(Mutex_);
		try {
			ProcessIncomingFrame();
		} catch (const Poco::Exception &E) {
			Logger().log(E);
			delete this;
		} catch (const std::exception &E) {
			std::string W = E.what();
			poco_information(Logger(),Poco::format("std::exception caught: %s. Connection terminated with %s", W, CId_));
			delete this;
		} catch (...) {
			poco_information(Logger(),Poco::format("Unknown exception for %s. Connection terminated.", CId_));
			delete this;
		}
	}

	std::string asString(Poco::Buffer<char> &buf) {
		if (buf.sizeBytes() > 0) {
			buf.append(0);
			return buf.begin();
		}
		return "";
	}

	void WSConnection::ProcessIncomingFrame() {

		// bool MustDisconnect=false;
		Poco::Buffer<char> IncomingFrame(0);

		try {
			int Op, flags;
			int IncomingSize;
			IncomingSize = WS_->receiveFrame(IncomingFrame, flags);
			Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

			if (IncomingSize == 0 && flags == 0 && Op == 0) {
				poco_information(Logger(),Poco::format("DISCONNECT(%s): device has disconnected.", CId_));
				return delete this;
			} else {

				if (Conn_ != nullptr) {
					Conn_->Conn_.RX += IncomingSize;
					Conn_->Conn_.MessageCount++;
				}

				switch (Op) {
				case Poco::Net::WebSocket::FRAME_OP_PING: {
					poco_trace(Logger(),Poco::format("WS-PING(%s): received. PONG sent back.", CId_));
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
						PingObject.set(uCentralProtocol::PING, PingDetails);
						Poco::JSON::Stringifier Stringify;
						std::ostringstream OS;
						Stringify.condense(PingObject, OS);
						KafkaManager()->PostMessage(KafkaTopics::CONNECTION, SerialNumber_, OS.str());
					}
					return;
				} break;

				case Poco::Net::WebSocket::FRAME_OP_PONG: {
					poco_trace(Logger(),Poco::format("PONG(%s): received and ignored.", CId_));
					return;
				} break;

				case Poco::Net::WebSocket::FRAME_OP_TEXT: {
					std::string IncomingMessageStr = asString(IncomingFrame);
					poco_trace(Logger(),
						Poco::format("FRAME(%s): Frame received (length=%d, flags=0x%x). Msg=%s", CId_,
									 IncomingSize, unsigned(flags), IncomingMessageStr));

					Poco::JSON::Parser parser;
					auto ParsedMessage = parser.parse(IncomingMessageStr);
					auto IncomingJSON = ParsedMessage.extract<Poco::JSON::Object::Ptr>();

					if (IncomingJSON->has(uCentralProtocol::JSONRPC)) {
						if (IncomingJSON->has(uCentralProtocol::METHOD) &&
							IncomingJSON->has(uCentralProtocol::PARAMS)) {
							ProcessJSONRPCEvent(IncomingJSON);
						} else if (IncomingJSON->has(uCentralProtocol::RESULT) &&
								   IncomingJSON->has(uCentralProtocol::ID)) {
							poco_trace(Logger(),Poco::format("RPC-RESULT(%s): payload: %s", CId_, IncomingMessageStr));
							ProcessJSONRPCResult(IncomingJSON);
						} else {
							poco_warning(Logger(),
								Poco::format("INVALID-PAYLOAD(%s): Payload is not JSON-RPC 2.0: %s",
											 CId_, IncomingMessageStr));
						}
					} else {
						poco_warning(Logger(),Poco::format(
							"FRAME(%s): illegal transaction header, missing 'jsonrpc'", CId_));
						Errors_++;
					}
					return;
				} break;

				case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
					poco_information(Logger(),
						Poco::format("CLOSE(%s): Device is closing its connection.", CId_));
					return delete this;
				} break;

				default: {
					poco_warning(Logger(),Poco::format("UNKNOWN(%s): unknownWS Frame operation: %s", CId_,
												  std::to_string(Op)));
				} break;
				}
			}
		} catch (const Poco::Net::ConnectionResetException &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			poco_warning(Logger(),Poco::format("EXCEPTION(%s): Caught a ConnectionResetException: %s, Message: %s",
										  CId_, E.displayText(), IncomingMessageStr));
			return delete this;
		} catch (const Poco::JSON::JSONException &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			poco_warning(Logger(),Poco::format("EXCEPTION(%s): Caught a JSONException: %s, Message: %s",
												CId_, E.displayText(), IncomingMessageStr));
		} catch (const Poco::Net::WebSocketException &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			poco_warning(Logger(),Poco::format("EXCEPTION(%s): Caught a JSONException: %s, Message: %s",
												CId_, E.displayText(), IncomingMessageStr));
			Logger().warning(Poco::format("%s(%s): Caught a websocket exception: %s. Message: %s",
										  std::string(__func__), CId_, E.displayText(),
										  IncomingMessageStr));
			return delete this;
		} catch (const Poco::Net::SSLConnectionUnexpectedlyClosedException &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			poco_warning(Logger(),Poco::format("EXCEPTION(%s): Caught a SSLConnectionUnexpectedlyClosedException: %s, Message: %s",
												CId_, E.displayText(), IncomingMessageStr));
			return delete this;
		} catch (const Poco::Net::SSLException &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			poco_warning(Logger(),Poco::format("EXCEPTION(%s): Caught a SSLException: %s, Message: %s",
												CId_, E.displayText(), IncomingMessageStr));
			return delete this;
		} catch (const Poco::Net::NetException &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			poco_warning(Logger(),Poco::format("EXCEPTION(%s): Caught a NetException: %s, Message: %s",
												CId_, E.displayText(), IncomingMessageStr));
			return delete this;
		} catch (const Poco::IOException &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			poco_warning(Logger(),Poco::format("EXCEPTION(%s): Caught a IOException: %s, Message: %s",
												CId_, E.displayText(), IncomingMessageStr));
			return delete this;
		} catch (const Poco::Exception &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			poco_warning(Logger(),Poco::format("EXCEPTION(%s): Caught a Exception: %s, Message: %s",
												CId_, E.displayText(), IncomingMessageStr));
			return delete this;
		} catch (const std::exception &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			poco_warning(Logger(),Poco::format("EXCEPTION(%s): Caught a std::exception: %s, Message: %s",
												CId_, std::string{E.what()}, IncomingMessageStr));
			return delete this;
		} catch (...) {
			return delete this;
		}

		if (Errors_ < 10)
			return;

		poco_warning(Logger(),Poco::format("DISCONNECTING(%s): Too many errors", CId_));
		delete this;
	}

	bool WSConnection::Send(const std::string &Payload) {
		std::lock_guard Guard(Mutex_);

		auto BytesSent = WS_->sendFrame(Payload.c_str(), (int)Payload.size());
		if (Conn_)
			Conn_->Conn_.TX += BytesSent;
		return BytesSent == Payload.size();
	}

}