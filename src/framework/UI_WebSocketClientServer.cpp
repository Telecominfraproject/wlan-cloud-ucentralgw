//
// Created by stephane bourque on 2022-10-25.
//

#include <mutex>

#include "Poco/NObserver.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Logger.h"

#include "framework/UI_WebSocketClientServer.h"
#include "framework/AuthClient.h"
#include "framework/MicroServiceFuncs.h"

#include "fmt/format.h"

namespace OpenWifi {

	void UI_WebSocketClientServer::NewClient(Poco::Net::WebSocket & WS, const std::string &Id, const std::string &UserName ) {
		std::lock_guard G(Mutex_);
		auto Client = std::make_unique<UI_WebSocketClient>(WS,Id,UserName,Logger(), Processor_);
		Clients_[Id] = std::make_pair(std::move(Client),"");
	}

	void UI_WebSocketClientServer::SetProcessor( UI_WebSocketClientProcessor * F) {
		Processor_ = F;
	}

	void UI_WebSocketClientServer::UnRegister(const std::string &Id) {
		std::lock_guard G(Mutex_);
		Clients_.erase(Id);
	}

	void UI_WebSocketClientServer::SetUser(const std::string &Id, const std::string &UserId) {
		std::lock_guard G(Mutex_);

		auto it=Clients_.find(Id);
		if(it!=Clients_.end()) {
			Clients_[Id] = std::make_pair(std::move(it->second.first),UserId);
		}
	}

	[[nodiscard]] inline bool SendToUser(const std::string &userName, const std::string &Payload);
	UI_WebSocketClientServer::UI_WebSocketClientServer() noexcept:
		 SubSystemServer("WebSocketClientServer", "UI-WSCLNT-SVR", "websocketclients")
	{
	}

	void UI_WebSocketClientServer::run() {
		Running_ = true ;
		Utils::SetThreadName("ws:uiclnt-svr");
		while(Running_) {
			Poco::Thread::trySleep(2000);

			if(!Running_)
				break;
		}
	};

	int UI_WebSocketClientServer::Start() {
		GoogleApiKey_ = MicroServiceConfigGetString("google.apikey","");
		GeoCodeEnabled_ = !GoogleApiKey_.empty();
		// ReactorPool_ = std::make_unique<MyParallelSocketReactor>();
		ReactorThread_.start(Reactor_);
		Thr_.start(*this);
		return 0;
	};

	void UI_WebSocketClientServer::Stop() {
		if(Running_) {
			Reactor_.stop();
			ReactorThread_.join();
			Running_ = false;
			Thr_.wakeUp();
			Thr_.join();
		}
	};

	bool UI_WebSocketClientServer::Send(const std::string &Id, const std::string &Payload) {
		std::lock_guard G(Mutex_);

		auto It = Clients_.find(Id);
		if(It!=Clients_.end())
			return It->second.first->Send(Payload);
		return false;
	}

	bool UI_WebSocketClientServer::SendToUser(const std::string &UserName, const std::string &Payload) {
		std::lock_guard G(Mutex_);
		uint64_t Sent=0;

		for(const auto &client:Clients_) {
			if(client.second.second == UserName) {
				try {
					if (client.second.first->Send(Payload))
						Sent++;
				} catch (...) {
					return false;
				}
			}
		}
		return Sent>0;
	}

	void UI_WebSocketClientServer::SendToAll(const std::string &Payload) {
		std::lock_guard G(Mutex_);

		for(const auto &client:Clients_) {
			try {
				client.second.first->Send(Payload);
			} catch (...) {

			}
		}
	}

	void UI_WebSocketClient::OnSocketError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		EndConnection();
	}

	void UI_WebSocketClient::OnSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		int flags;
		int n;
		bool Done=false;
		try {
			Poco::Buffer<char> IncomingFrame(0);
			n = WS_->receiveFrame(IncomingFrame, flags);
			auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

			if (n == 0) {
				poco_debug(Logger(),fmt::format("CLOSE({}): {} UI Client is closing WS connection.", Id_, UserName_));
				return EndConnection();
			}

			switch (Op) {
			case Poco::Net::WebSocket::FRAME_OP_PING: {
				WS_->sendFrame("", 0,
							   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
								   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
			} break;
			case Poco::Net::WebSocket::FRAME_OP_PONG: {
			} break;
			case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
				poco_debug(Logger(),fmt::format("CLOSE({}): {} UI Client is closing WS connection.", Id_, UserName_));
				Done = true;
			} break;
			case Poco::Net::WebSocket::FRAME_OP_TEXT: {
				IncomingFrame.append(0);
				if (!Authenticated_) {
					std::string Frame{IncomingFrame.begin()};
					auto Tokens = Utils::Split(Frame, ':');
					bool Expired = false, Contacted = false;
					if (Tokens.size() == 2 &&
						AuthClient()->IsAuthorized(Tokens[1], UserInfo_, 0, Expired, Contacted)) {
						Authenticated_ = true;
						UserName_ = UserInfo_.userinfo.email;
						poco_debug(Logger(),fmt::format("START({}): {} UI Client is starting WS connection.", Id_, UserName_));
						std::string S{"Welcome! Bienvenue! Bienvenidos!"};
						WS_->sendFrame(S.c_str(), S.size());
						UI_WebSocketClientServer()->SetUser(Id_, UserInfo_.userinfo.email);
					} else {
						std::string S{"Invalid token. Closing connection."};
						WS_->sendFrame(S.c_str(), S.size());
						Done = true;
					}

				} else {
					try {
						Poco::JSON::Parser P;
						auto Obj =
							P.parse(IncomingFrame.begin()).extract<Poco::JSON::Object::Ptr>();
						std::string Answer;
						if (Processor_ != nullptr)
							Processor_->Processor(Obj, Answer, Done);
						if (!Answer.empty())
							WS_->sendFrame(Answer.c_str(), (int)Answer.size());
						else {
							WS_->sendFrame("{}", 2);
						}
					} catch (const Poco::JSON::JSONException &E) {
						Logger().log(E);
						Done=true;
					}
				}
			} break;
			default: {
			}
			}
		} catch (...) {
			Done=true;
		}

		if(Done) {
			return EndConnection();
		}
	}

	void UI_WebSocketClient::OnSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		EndConnection();
	}


	UI_WebSocketClient::UI_WebSocketClient( Poco::Net::WebSocket & WS , const std::string &Id, const std::string &UserName, Poco::Logger & L, UI_WebSocketClientProcessor * Processor) :
		Reactor_(UI_WebSocketClientServer()->Reactor()),
		Id_(Id),
		UserName_(UserName),
		Logger_(L),
		Processor_(Processor) {
		WS_ = std::make_unique<Poco::Net::WebSocket>(WS);
		WS_->setNoDelay(true);
		WS_->setKeepAlive(true);
		WS_->setBlocking(false);
		Reactor_.addEventHandler(*WS_,
								 Poco::NObserver<UI_WebSocketClient, Poco::Net::ReadableNotification>(
									 *this, &UI_WebSocketClient::OnSocketReadable));
		Reactor_.addEventHandler(*WS_,
								 Poco::NObserver<UI_WebSocketClient, Poco::Net::ShutdownNotification>(
									 *this, &UI_WebSocketClient::OnSocketShutdown));
		Reactor_.addEventHandler(*WS_,
								 Poco::NObserver<UI_WebSocketClient, Poco::Net::ErrorNotification>(
									 *this, &UI_WebSocketClient::OnSocketError));
		SocketRegistered_ = true;
	}

	void UI_WebSocketClient::EndConnection() {
		if(SocketRegistered_) {
			SocketRegistered_ = false;
			(*WS_).shutdown();
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<UI_WebSocketClient,
														Poco::Net::ReadableNotification>(*this,&UI_WebSocketClient::OnSocketReadable));
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<UI_WebSocketClient,
														Poco::Net::ShutdownNotification>(*this,&UI_WebSocketClient::OnSocketShutdown));
			Reactor_.removeEventHandler(*WS_,
										Poco::NObserver<UI_WebSocketClient,
														Poco::Net::ErrorNotification>(*this,&UI_WebSocketClient::OnSocketError));
			UI_WebSocketClientServer()->UnRegister(Id_);
		}
	}

	UI_WebSocketClient::~UI_WebSocketClient() {
		EndConnection();
	}

	[[nodiscard]] const std::string & UI_WebSocketClient::Id() {
		return Id_;
	};

	[[nodiscard]] Poco::Logger & UI_WebSocketClient::Logger() {
		return Logger_;
	}

	[[nodiscard]] bool UI_WebSocketClient::Send(const std::string &Payload) {
		try {
			WS_->sendFrame(Payload.c_str(),Payload.size());
			return true;
		} catch (...) {

		}
		return false;
	}


} // namespace OpenWifi