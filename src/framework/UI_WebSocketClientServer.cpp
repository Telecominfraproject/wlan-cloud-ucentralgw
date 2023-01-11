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

#if defined(TIP_SECURITY_SERVICE)
#include "AuthService.h"
#endif

#define DBG { std::cout << __LINE__ << std::endl; }

namespace OpenWifi {

	void UI_WebSocketClientServer::NewClient(Poco::Net::WebSocket & WS, const std::string &Id, const std::string &UserName, std::uint64_t TID ) {

        std::lock_guard G(LocalMutex_);
        auto Client = std::make_unique<UI_WebSocketClientInfo>(WS,Id, UserName);
        auto ClientSocket = Client->WS_->impl()->sockfd();
        TID_ = TID;
        Client->WS_->setNoDelay(true);
        Client->WS_->setKeepAlive(true);
        Client->WS_->setBlocking(false);
        Reactor_.addEventHandler(*Client->WS_,
                                 Poco::NObserver<UI_WebSocketClientServer, Poco::Net::ReadableNotification>(
                                         *this, &UI_WebSocketClientServer::OnSocketReadable));
        Reactor_.addEventHandler(*Client->WS_,
                                 Poco::NObserver<UI_WebSocketClientServer, Poco::Net::ShutdownNotification>(
                                         *this, &UI_WebSocketClientServer::OnSocketShutdown));
        Reactor_.addEventHandler(*Client->WS_,
                                 Poco::NObserver<UI_WebSocketClientServer, Poco::Net::ErrorNotification>(
                                         *this, &UI_WebSocketClientServer::OnSocketError));
        Client->SocketRegistered_ = true;
        Clients_[ClientSocket] = std::move(Client);
		UsersConnected_ = Clients_.size();
    }

	void UI_WebSocketClientServer::SetProcessor( UI_WebSocketClientProcessor * F) {
		Processor_ = F;
	}

	UI_WebSocketClientServer::UI_WebSocketClientServer() noexcept:
		 SubSystemServer("WebSocketClientServer", "UI-WSCLNT-SVR", "websocketclients")
	{
	}

	void UI_WebSocketClientServer::run() {
		Running_ = true;
		while(Running_) {
			Poco::Thread::trySleep(2000);

			if(!Running_)
				break;

			std::lock_guard		G(LocalMutex_);
			for(const auto i:ToBeRemoved_) {
				// std::cout << "Erasing old WS UI connection..." << std::endl;
				Clients_.erase(i);
			}
            ToBeRemoved_.clear();
			UsersConnected_ = Clients_.size();
		}

	}

    void UI_WebSocketClientServer::EndConnection(ClientList::iterator Client) {
        if(Client->second->SocketRegistered_) {
            Client->second->SocketRegistered_ = false;
            Reactor_.removeEventHandler(*Client->second->WS_,
                                        Poco::NObserver<UI_WebSocketClientServer,
                                                Poco::Net::ReadableNotification>(*this,&UI_WebSocketClientServer::OnSocketReadable));
            Reactor_.removeEventHandler(*Client->second->WS_,
                                        Poco::NObserver<UI_WebSocketClientServer,
                                                Poco::Net::ShutdownNotification>(*this,&UI_WebSocketClientServer::OnSocketShutdown));
            Reactor_.removeEventHandler(*Client->second->WS_,
                                        Poco::NObserver<UI_WebSocketClientServer,
                                                Poco::Net::ErrorNotification>(*this,&UI_WebSocketClientServer::OnSocketError));
        }
		ToBeRemoved_.push_back(Client);
    }

	int UI_WebSocketClientServer::Start() {
        poco_information(Logger(),"Starting...");
		GoogleApiKey_ = MicroServiceConfigGetString("google.apikey","");
		GeoCodeEnabled_ = !GoogleApiKey_.empty();
		ReactorThread_.start(Reactor_);
		ReactorThread_.setName("ws:ui-reactor");
		CleanerThread_.start(*this);
		CleanerThread_.setName("ws:ui-cleaner");
		return 0;
	};

	void UI_WebSocketClientServer::Stop() {
		if(Running_) {
            poco_information(Logger(),"Stopping...");
            Clients_.clear();
			Reactor_.stop();
			ReactorThread_.join();
			Running_ = false;
			CleanerThread_.wakeUp();
			CleanerThread_.join();
            poco_information(Logger(),"Stopped...");
		}
	};

	bool UI_WebSocketClientServer::IsFiltered(std::uint64_t id, const OpenWifi::UI_WebSocketClientInfo &Client) {
		return std::find(Client.Filter_.begin(), Client.Filter_.end(),id)!=end(Client.Filter_);
	}

	bool UI_WebSocketClientServer::SendToUser(const std::string &UserName, std::uint64_t id, const std::string &Payload) {
		std::lock_guard G(LocalMutex_);

		for(const auto &Client:Clients_) {
			if(Client.second->UserName_ == UserName) {
				try {
					if(!IsFiltered(id,*Client.second) && Client.second->Authenticated_) {
						return Client.second->WS_->sendFrame(
								   Payload.c_str(), (int)Payload.size()) == (int)Payload.size();
					} else {
						return false;
					}
				} catch (...) {
					return false;
				}
			}
		}
		return false;
	}

	void UI_WebSocketClientServer::SendToAll(std::uint64_t id, const std::string &Payload) {
		std::lock_guard G(LocalMutex_);

		for(const auto &Client:Clients_) {
			try {
				if(!IsFiltered(id,*Client.second) && Client.second->Authenticated_)
					Client.second->WS_->sendFrame(Payload.c_str(),(int)Payload.size());
			} catch (...) {

			}
		}
	}

    UI_WebSocketClientServer::ClientList::iterator UI_WebSocketClientServer::FindWSClient( [[maybe_unused]]  std::lock_guard<std::recursive_mutex> &G, int ClientSocket) {
        return Clients_.find(ClientSocket);
    }

	void UI_WebSocketClientServer::SortNotifications() {
		struct {
			bool operator()(const NotificationEntry &A, const NotificationEntry & B) const {
				return A.id < B.id; };
		} CompareNotifications;
		std::sort(NotificationTypes_.begin(), NotificationTypes_.end(), CompareNotifications);

		NotificationTypesJSON_.clear();
		Poco::JSON::Array		AllNotifications;
		for(const auto &notification:NotificationTypes_) {
			Poco::JSON::Object	Notification;
			Notification.set("id", notification.id);
			Notification.set("helper", notification.helper);
			AllNotifications.add(Notification);
		}
		NotificationTypesJSON_.set("notificationTypes", AllNotifications);
	}

	void UI_WebSocketClientServer::RegisterNotifications(const OpenWifi::UI_WebSocketClientServer::NotificationTypeIdVec &Notifications) {
		std::copy(Notifications.begin(), Notifications.end(), std::back_inserter(NotificationTypes_));
		SortNotifications();
	}

    void UI_WebSocketClientServer::OnSocketError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
        std::lock_guard     G(LocalMutex_);
        auto Client = Clients_.find(pNf->socket().impl()->sockfd());
        if(Client==end(Clients_))
            return;
        EndConnection(Client);
	}

	void UI_WebSocketClientServer::OnSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {

        UI_WebSocketClientServer::ClientList::iterator Client;
        std::lock_guard     G(LocalMutex_);

		try {

            Client = Clients_.find(pNf->socket().impl()->sockfd());
            if( Client == end(Clients_))
                return;

			Poco::Buffer<char> IncomingFrame(0);
            int flags;
            int n;
			n = Client->second->WS_->receiveFrame(IncomingFrame, flags);
			auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

			if (n == 0) {
				poco_debug(Logger(),fmt::format("CLOSE({}): {} UI Client is closing WS connection.", Client->second->Id_, Client->second->UserName_));
				return EndConnection(Client);
			}

			switch (Op) {
			case Poco::Net::WebSocket::FRAME_OP_PING: {
                Client->second->WS_->sendFrame("", 0,
							   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
								   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
			} break;
			case Poco::Net::WebSocket::FRAME_OP_PONG: {
			} break;
			case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
				poco_debug(Logger(),fmt::format("CLOSE({}): {} UI Client is closing WS connection.", Client->second->Id_, Client->second->UserName_));
                return EndConnection(Client);
			} break;
			case Poco::Net::WebSocket::FRAME_OP_TEXT: {
				constexpr const char *DropMessagesCommand = "drop-notifications";
				IncomingFrame.append(0);
				if (!Client->second->Authenticated_) {
					std::string Frame{IncomingFrame.begin()};
					auto Tokens = Utils::Split(Frame, ':');
					bool Expired = false;
#if not defined(TIP_SECURITY_SERVICE)
                    bool    Contacted = false;
#endif
					if (Tokens.size() == 2 &&
#if defined(TIP_SECURITY_SERVICE)
        			    AuthService()->IsAuthorized(Tokens[1], Client->second->UserInfo_, TID_, Expired)) {
#else
                        AuthClient()->IsAuthorized(Tokens[1], Client->second->UserInfo_, TID_, Expired, Contacted)) {
#endif
                        Client->second->Authenticated_ = true;
                        Client->second->UserName_ = Client->second->UserInfo_.userinfo.email;
						poco_debug(Logger(),fmt::format("START({}): {} UI Client is starting WS connection.", Client->second->Id_, Client->second->UserName_));
						auto WelcomeMessage = NotificationTypesJSON_;
						WelcomeMessage.set("success", "Welcome! Bienvenue! Bienvenidos!");
						std::ostringstream OS;
						WelcomeMessage.stringify(OS);
						Client->second->WS_->sendFrame(OS.str().c_str(), (int) OS.str().size());
                        Client->second->UserName_ = Client->second->UserInfo_.userinfo.email;
					} else {
						Poco::JSON::Object	WelcomeMessage;
						WelcomeMessage.set("error", "Invalid token. Closing connection.");
						std::ostringstream OS;
						WelcomeMessage.stringify(OS);
						Client->second->WS_->sendFrame(OS.str().c_str(), (int) OS.str().size());
                        return EndConnection(Client);
					}
				} else {
                    Poco::JSON::Parser P;
                    auto Obj =
                        P.parse(IncomingFrame.begin()).extract<Poco::JSON::Object::Ptr>();

					if(Obj->has(DropMessagesCommand) && Obj->isArray(DropMessagesCommand)) {
						auto Filters = Obj->getArray(DropMessagesCommand);
						Client->second->Filter_.clear();
						for(const auto &Filter:*Filters) {
							Client->second->Filter_.emplace_back( (std::uint64_t) Filter);
						}
						std::sort(begin(Client->second->Filter_),end(Client->second->Filter_));
						return;
					}

                    std::string Answer;
                    bool CloseConnection=false;
                    if (Processor_ != nullptr) {
                        Processor_->Processor(Obj, Answer, CloseConnection,Client->second->UserInfo_.userinfo);
                    }
                    if (!Answer.empty())
                        Client->second->WS_->sendFrame(Answer.c_str(), (int)Answer.size());
                    else {
                        Client->second->WS_->sendFrame("{}", 2);
                    }

                    if(CloseConnection) {
                        return EndConnection(Client);
                    }
				}
			} break;
			default: {
			}
			}
		} catch (...) {
            return EndConnection(Client);
		}
	}

	void UI_WebSocketClientServer::OnSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
        try {
			std::lock_guard     G(LocalMutex_);
			auto Client = Clients_.find(pNf->socket().impl()->sockfd());
            if (Client == end(Clients_))
                return;
            EndConnection(Client);
        } catch (...) {

        }
	}

} // namespace OpenWifi