//
// Created by stephane bourque on 2021-09-07.
//
#include <thread>

#include "TelemetryStream.h"
#include "Poco/Net/IPAddress.h"
#include "Poco/Net/SSLException.h"
#include "Poco/Net/HTTPServerSession.h"
#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/Net/HTTPServerRequestImpl.h"
#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/JSON/Array.h"
#include "Poco/zlib.h"
#include "Poco/URI.h"
#include "Utils.h"
#include "Daemon.h"
#include "DeviceRegistry.h"

namespace OpenWifi {

	class TelemetryStream *TelemetryStream::instance_ = nullptr;

	int TelemetryStream::Start() {
		for(const auto & Svr : ConfigServersList_ ) {
			Logger_.notice(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.Address(), std::to_string(Svr.Port()),
										Svr.KeyFile(),Svr.CertFile()));

			Svr.LogCert(Logger_);
			if(!Svr.RootCA().empty())
				Svr.LogCas(Logger_);

			auto Sock{Svr.CreateSecureSocket(Logger_)};
			auto NewSocketAcceptor = std::make_unique<Poco::Net::ParallelSocketAcceptor<TelemetryClient, Poco::Net::SocketReactor>>( Sock, Reactor_);
			Acceptors_.push_back(std::move(NewSocketAcceptor));
		}
		ReactorThread_.start(Reactor_);
		return 0;
	}

	void TelemetryStream::Stop() {
		Logger_.notice("Stopping reactors...");
		Reactor_.stop();
		ReactorThread_.join();
	}

	bool TelemetryStream::CreateEndpoint(const std::string &SerialNumber, std::string &EndPoint, std::string &UUID) {
		std::lock_guard	G(Mutex_);

		Poco::URI	U;
		UUID = Daemon()->CreateUUID();
		U.setScheme("wss");
		U.setHost(ConfigServersList_[0].Name());
		U.setPort(ConfigServersList_[0].Port());
		U.setPath(UUID);
		U.addQueryParameter("serialNumber", SerialNumber);
		EndPoint = U.toString();
		SerialNumbers_[SerialNumber] = UUID;
		Clients_[UUID] = nullptr;
		return true;
	}

	void TelemetryStream::DeleteEndPoint(const std::string &SerialNumber) {
		std::lock_guard	G(Mutex_);

		auto H1 = SerialNumbers_.find(SerialNumber);
		if(H1!=SerialNumbers_.end()) {
			auto H2 = Clients_.find(H1->second);
			if(H2!=Clients_.end()) {
				try {
					delete H2->second;
				} catch (const Poco::Exception &E ) {

				}
			}
		}
	}

	void TelemetryStream::UpdateEndPoint(const std::string &SerialNumber, const std::string &PayLoad) {
		std::lock_guard	G(Mutex_);

		auto H1 = SerialNumbers_.find(SerialNumber);
		if(H1!=SerialNumbers_.end()) {
			auto H2 = Clients_.find(H1->second);
			if(H2!=Clients_.end() && H2->second!=nullptr) {
				try {
					H2->second->Send(PayLoad);
				} catch (const Poco::Exception &E ) {

				}
			}
		}
	}

	bool TelemetryStream::RegisterClient(const std::string &UUID, TelemetryClient *Client) {
		std::lock_guard	G(Mutex_);

		auto Hint = Clients_.find(UUID);
		if(Hint!=Clients_.end()) {
			Hint->second = Client;
			return true;
		}
		return false;
	}

	void TelemetryStream::DeRegisterClient(const std::string &UUID) {
		std::lock_guard		G(Mutex_);

		auto Hint = Clients_.find(UUID);
		if(Hint!=Clients_.end()) {
			Clients_.erase(Hint);
			for(const auto &i:SerialNumbers_) {
				if(i.second==UUID) {
					SerialNumbers_.erase(i.first);
					break;
				}
			}
		}
	}

	void TelemetryClient::CompleteStartup() {
		std::cout << __LINE__ << std::endl;
		std::lock_guard Guard(Mutex_);
		std::cout << __LINE__ << std::endl;
		try {
			if (!Socket_.secure()) {
				std::cout << "Not secure socket..." << std::endl;
			} else {
/*				std::cout << __LINE__ << std::endl;
				SocketImpl_ = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(Socket_.impl());

				std::cout << __LINE__ << std::endl;
				if (SocketImpl_->getLazyHandshake()) {
					std::cout << __LINE__ << std::endl;
					SocketImpl_->completeHandshake();
				}

				std::cout << __LINE__ << std::endl;
*/
				CId_ = Utils::FormatIPv6(Socket_.peerAddress().toString());
				std::cout << __LINE__ << std::endl;

				if (!Socket_.secure()) {
					Logger_.error(Poco::format("%s: Connection is NOT secure.", CId_));
				} else {
					Logger_.debug(Poco::format("%s: Connection is secure.", CId_));
				}
				std::cout << __LINE__ << std::endl;

				auto Params = Poco::AutoPtr<Poco::Net::HTTPServerParams>(new Poco::Net::HTTPServerParams);
				std::cout << __LINE__ << std::endl;
				Poco::Net::HTTPServerSession Session(Socket_, Params);
				std::cout << __LINE__ << std::endl;
				Poco::Net::HTTPServerResponseImpl Response(Session);
				std::cout << __LINE__ << std::endl;
				Poco::Net::HTTPServerRequestImpl Request(Response, Session, Params);
				std::cout << __LINE__ << std::endl;
				Poco::URI U(Request.getURI());
				std::cout << __LINE__ << std::endl;

				UUID_ = U.getPath().substr(1);
				std::cout << __LINE__ << std::endl;
				if (TelemetryStream()->RegisterClient(UUID_, this)) {
					std::cout << __LINE__ << std::endl;
					auto Parameters = U.getQueryParameters();
					std::cout << __LINE__ << std::endl;
					for (const auto &i : Parameters) {
						std::cout << __LINE__ << std::endl;
						if (i.first == "serialNumber")
							SerialNumber_ = i.second;
					}
					std::cout << __LINE__ << std::endl;
					auto Now = time(nullptr);
					Response.setDate(Now);
					std::cout << __LINE__ << std::endl;
					Response.setVersion(Request.getVersion());
					Response.setKeepAlive(Params->getKeepAlive() && Request.getKeepAlive() &&
					Session.canKeepAlive());
					WS_ = std::make_unique<Poco::Net::WebSocket>(Request, Response);
					WS_->setMaxPayloadSize(BufSize);

					std::cout << __LINE__ << std::endl;
					auto TS = Poco::Timespan(240, 0);
					std::cout << __LINE__ << std::endl;

					WS_->setReceiveTimeout(TS);
					WS_->setNoDelay(true);
					WS_->setKeepAlive(true);
					Reactor_.addEventHandler(
						*WS_, Poco::NObserver<TelemetryClient, Poco::Net::ReadableNotification>(
							*this, &TelemetryClient::OnSocketReadable));
					Reactor_.addEventHandler(
						*WS_, Poco::NObserver<TelemetryClient, Poco::Net::ShutdownNotification>(
							*this, &TelemetryClient::OnSocketShutdown));
					Reactor_.addEventHandler(
						*WS_, Poco::NObserver<TelemetryClient, Poco::Net::ErrorNotification>(
							*this, &TelemetryClient::OnSocketError));
					std::cout << __LINE__ << std::endl;
					Registered_ = true;
					std::cout << __LINE__ << std::endl;
					Logger_.information(Poco::format("CONNECTION(%s): completed.", CId_));
					return;
				}
			}
		} catch (const Poco::Net::SSLException &E) {
			std::cout << __LINE__ << std::endl;
			Logger_.log(E);
		}
		catch (const Poco::Exception &E) {
			std::cout << __LINE__ << std::endl;
			Logger_.log(E);
		}
		delete this;
	}

	TelemetryClient::TelemetryClient(Poco::Net::StreamSocket &Socket, Poco::Net::SocketReactor &Reactor) :
		Socket_(Socket),
		Reactor_(Reactor),
		Logger_(TelemetryStream()->Logger()) {
		std::thread T([this]() { this->CompleteStartup(); });
		T.detach();
	}

	TelemetryClient::~TelemetryClient() {
		Logger_.information("Closing telemetry session.");
		if(Registered_ && WS_)
		{
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<TelemetryClient,
										Poco::Net::ReadableNotification>(*this,&TelemetryClient::OnSocketReadable));
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<TelemetryClient,
										Poco::Net::ShutdownNotification>(*this,&TelemetryClient::OnSocketShutdown));
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<TelemetryClient,
										Poco::Net::ErrorNotification>(*this,&TelemetryClient::OnSocketError));
			(*WS_).close();
			Socket_.shutdown();
		} else {
			if(WS_)
				(*WS_).close();
			Socket_.shutdown();
		}
	}

	bool TelemetryClient::Send(const std::string &Payload) {
		std::lock_guard Guard(Mutex_);
		auto BytesSent = WS_->sendFrame(Payload.c_str(),(int)Payload.size());
		return  BytesSent == Payload.size();
	}

	void TelemetryClient::SendTelemetryShutdown() {
		Logger_.information(Poco::format("TELEMETRY-SHUTDOWN(%s): Closing.",CId_));
		TelemetryStream()->DeRegisterClient(UUID_);
		Poco::JSON::Object	StopMessage;
		StopMessage.set("jsonrpc","2.0");
		StopMessage.set("method","telemetry");
		Poco::JSON::Object	Params;
		Params.set("serial", SerialNumber_);
		Params.set("interval",0);
		StopMessage.set("id",1);
		StopMessage.set("params",Params);
		Poco::JSON::Stringifier		Stringify;
		std::ostringstream OS;
		Stringify.condense(StopMessage,OS);
		DeviceRegistry()->SendFrame(SerialNumber_, OS.str());
		TelemetryStream()->DeRegisterClient(UUID_);
		delete this;
	}

	void TelemetryClient::OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		std::lock_guard Guard(Mutex_);
		Logger_.information(Poco::format("SOCKET-SHUTDOWN(%s): Orderly shutdown.", CId_));
		SendTelemetryShutdown();
	}

	void TelemetryClient::OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
		std::lock_guard Guard(Mutex_);
		Logger_.information(Poco::format("SOCKET-ERROR(%s): Closing.",CId_));
		SendTelemetryShutdown();
	}

	void TelemetryClient::OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		std::lock_guard Guard(Mutex_);
		try
		{
			ProcessIncomingFrame();
		}
		catch (const Poco::Exception & E)
		{
			Logger_.log(E);
			SendTelemetryShutdown();
		}
		catch (const std::exception & E) {
			std::string W = E.what();
			Logger_.information(Poco::format("std::exception caught: %s. Connection terminated with %s",W,CId_));
			SendTelemetryShutdown();
		}
		catch ( ... ) {
			Logger_.information(Poco::format("Unknown exception for %s. Connection terminated.",CId_));
			SendTelemetryShutdown();
		}
	}

	void TelemetryClient::ProcessIncomingFrame() {

		bool MustDisconnect=false;
		Poco::Buffer<char>			IncomingFrame(0);

		try {
			int Op,flags;
			int IncomingSize;
			IncomingSize = WS_->receiveFrame(IncomingFrame,flags);
			Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

			if (IncomingSize == 0 && flags == 0 && Op == 0) {
				Logger_.information(Poco::format("DISCONNECT(%s): device has disconnected.", CId_));
				MustDisconnect = true;
			} else {
				if (Op == Poco::Net::WebSocket::FRAME_OP_PING) {
					Logger_.debug(Poco::format("WS-PING(%s): received. PONG sent back.", CId_));
					WS_->sendFrame("", 0,
								   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
									   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
				} else if (Op == Poco::Net::WebSocket::FRAME_OP_CLOSE) {
					Logger_.information(Poco::format("DISCONNECT(%s): device wants to disconnect.", CId_));
					MustDisconnect = true ;
				}
			}
		} catch (...) {
			MustDisconnect = true ;
		}

		if(!MustDisconnect)
			return;

		SendTelemetryShutdown();
	}


}