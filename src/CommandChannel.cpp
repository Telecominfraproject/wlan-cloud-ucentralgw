//
// Created by stephane bourque on 2021-04-10.
//

#include "CommandChannel.h"
#include "uCentral.h"
#include "uCentralWebSocketServer.h"
#include "uFileUploader.h"
#include "uStorageService.h"
#include "uCentralRESTAPIServer.h"
#include "uCommandManager.h"

namespace uCentral::CommandChannel {

	Service * Service::instance_ = nullptr;

	int Start() {
		return uCentral::CommandChannel::Service::instance()->Start();
	}

	void Stop() {
		uCentral::CommandChannel::Service::instance()->Stop();
	}

	Poco::Message::Priority	StrToPriority(const std::string &S) {
		if(S=="PRIO_FATAL")
			return Poco::Message::PRIO_FATAL;
		else if(S=="PRIO_CRITICAL")
			return Poco::Message::PRIO_CRITICAL;
		else if(S=="PRIO_ERROR")
			return Poco::Message::PRIO_ERROR;
		else if(S=="PRIO_WARNING")
			return Poco::Message::PRIO_WARNING;
		else if(S=="PRIO_NOTICE")
			return Poco::Message::PRIO_NOTICE;
		else if(S=="PRIO_DEBUG")
			return Poco::Message::PRIO_DEBUG;
		else if(S=="PRIO_TRACE")
			return Poco::Message::PRIO_TRACE;
		else
			return Poco::Message::PRIO_INFORMATION;
	}

	void ProcessCommand(const std::string &Command) {
		// tokenize
		std::vector<std::string>	Tokens;

		size_t pos = 0, old_pos = 0 ;

		while((pos = Command.find(' ', old_pos)) != std::string::npos) {
			Tokens.push_back(Command.substr(old_pos,pos-old_pos));
			old_pos = pos + 1 ;
		}

		Tokens.push_back(Command.substr(old_pos));

		for(const auto &i:Tokens)
			std::cout << "Word:" << i << std::endl;

		// PRIO_FATAL, PRIO_CRITICAL, PRIO_ERROR, PRIO_WARNING, PRIO_NOTICE, PRIO_INFORMATION, PRIO_DEBUG, PRIO_TRACE

		if(Tokens[0]=="set") {
			if(Tokens[1]=="logLevel") {
				Poco::Message::Priority	P = StrToPriority(Tokens[2]);
				if(Tokens[3]=="uFileUploader")
					uCentral::uFileUploader::Service().SetLoggingLevel(P);
				else if(Tokens[3]=="WebSocket")
					uCentral::WebSocket::Service().SetLoggingLevel(P);
				else if(Tokens[3]=="Storage")
					uCentral::Storage::Service().SetLoggingLevel(P);
				else if(Tokens[3]=="RESTAPI")
					uCentral::RESTAPI::Service().SetLoggingLevel(P);
				else if(Tokens[3]=="CommandManager")
					uCentral::CommandManager::Service().SetLoggingLevel(P);
				else
					std::cout << "Invalid command..." << std::endl;

/*


					uCentral::CommandChannel::Stop();
				uCentral::uFileUploader::Stop();
				uCentral::CommandManager::Stop();
				uCentral::WebSocket::Stop();
				uCentral::RESTAPI::Stop();
				uCentral::DeviceRegistry::Stop();
				uCentral::Auth::Stop();
				uCentral::Storage::Stop();

*/




			}
		}

		for(const auto &i:Tokens)
			std::cout << "Word:" << i << std::endl;

	}

	/// This class handles all client connections.
	class UnixSocketServerConnection: public Poco::Net::TCPServerConnection
	{
	  public:
		explicit UnixSocketServerConnection(const Poco::Net::StreamSocket & S, Poco::Logger & Logger):
			TCPServerConnection(S),
		 	Logger_(Logger)
		{
		}

		void run() override
		{
			std::cout << __LINE__ << std::endl;
			// std::string	Addr = socket().peerAddress().toString();
			std::cout << __LINE__ << std::endl;
			try
			{
				std::string Message;
				std::vector<char>	buffer(1024);
				int n = 1;
				while (n > 0)
				{
					std::cout << __LINE__ << std::endl;
					n = socket().receiveBytes(&buffer[0], (int)buffer.size());
					buffer[n] = '\0';
					Message += &buffer[0];
					if(buffer.size() > n && !Message.empty())
					{
						ProcessCommand(Message);
						Message.clear();
					}
				}
			}
			catch (Poco::Exception& exc)
			{
				std::cerr << "Error: " << exc.displayText() << std::endl;
			}
			// Logger_.information(Poco::format("Disconnection from %s",Addr));
		}

	  private:
		Poco::Logger	& Logger_;
		static inline void EchoBack(const std::string & message)
		{
			std::cout << "Message: " << message << std::endl;
			// socket().sendBytes(message.c_str(), (int)message.size());
		}
	};

	class UnixSocketServerConnectionFactory: public Poco::Net::TCPServerConnectionFactory
	{
	  public:
		explicit UnixSocketServerConnectionFactory() :
			Logger_(Service::instance()->Logger())
		{
		}

		Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) override
		{
			std::cout << __LINE__ << std::endl;
			// Logger_.information(Poco::format("New connection from %s",socket.peerAddress().toString()));
			std::cout << __LINE__ << std::endl;
			return new UnixSocketServerConnection(socket,Logger_);
		}
	  private:
		Poco::Logger & Logger_;
	};

	Service::Service() noexcept:
		SubSystemServer("Authentication", "AUTH-SVR", "authentication")
	{
	}

	void Service::Stop() {
		Srv_->stop();
	}

	int Service::Start() {
		Poco::File	F(uCentral::ServiceConfig::getString("ucentral.system.commandchannel","/tmp/app.ucentralgw"));
		try { F.remove(); } catch (...) {};
		SocketFile_ = std::make_unique<Poco::File>(F);
		UnixSocket_ = std::make_unique<Poco::Net::SocketAddress>(Poco::Net::SocketAddress::UNIX_LOCAL, SocketFile_->path());
		Svs_ = std::make_unique<Poco::Net::ServerSocket>(*UnixSocket_);
		Srv_ = std::make_unique<Poco::Net::TCPServer>(new UnixSocketServerConnectionFactory, *Svs_);
		Srv_->start();
		return 0;
	}
};