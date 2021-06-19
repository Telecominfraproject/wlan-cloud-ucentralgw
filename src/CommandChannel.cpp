//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#include "CommandChannel.h"
#include "AuthService.h"
#include "CommandManager.h"
#include "Daemon.h"
#include "FileUploader.h"
#include "RESTAPI_server.h"
#include "StorageService.h"
#include "WebSocketServer.h"
#include <boost/algorithm/string.hpp>

namespace uCentral {

	class CommandChannel * CommandChannel::instance_ = nullptr;

	std::string  CommandChannel::ProcessCommand(const std::string &Command) {
		std::vector<std::string>	Tokens{};
		std::string Result{"OK"};


		try {
			size_t pos, old_pos = 0 ;

			Logger_.notice(Poco::format("COMMAND: %s",Command));

			while((pos = Command.find(' ', old_pos)) != std::string::npos) {
				Tokens.push_back(Command.substr(old_pos,pos-old_pos));
				old_pos = pos + 1 ;
			}

			Tokens.push_back(Command.substr(old_pos));
			boost::algorithm::to_lower(Tokens[0]);
			boost::algorithm::to_lower(Tokens[1]);

			if(Tokens[0]=="set") {
				if(Tokens[1]=="loglevel") {
					if(!Daemon()->SetSubsystemLogLevel(Tokens[3],Tokens[2]))
						Result =  "ERROR: Invalid: set logLevel subsystem name:" + Tokens[3];
				}
			} else if(Tokens[0]=="get") {
				if(Tokens[1]=="loglevel") {
					std::cout << "LogLevels:" << std::endl;
					std::cout << " Auth: " << AuthService()->Logger().getLevel() << std::endl;
					std::cout << " uFileUploader: " << FileUploader()->Logger().getLevel() << std::endl;
					std::cout << " WebSocket: " << WebSocketServer()->Logger().getLevel() << std::endl;
					std::cout << " Storage: " << Storage()->Logger().getLevel() << std::endl;
					std::cout << " RESTAPI: " << RESTAPI_server()->Logger().getLevel() << std::endl;
					std::cout << " CommandManager: " << Logger_.getLevel() << std::endl;
					std::cout << " DeviceRegistry: " << DeviceRegistry()->Logger().getLevel() << std::endl;
				} else if (Tokens[1]=="stats") {

				} else {
					Result =  "ERROR: Invalid: get command:" + Tokens[1];
				}
			} else if(Tokens[0]=="restart") {
				Logger_.information("RESTART...");
			} else if(Tokens[0]=="stop") {
				Logger_.information("STOP...");
			} else if(Tokens[0]=="stats") {
				Logger_.information("STATS...");
			} else {
				Result = "ERROR: Invalid command: " + Tokens[0];
			}
			Logger_.notice(Poco::format("COMMAND-RESULT: %s",Result));
		}
		catch ( const Poco::Exception & E) {
			Logger_.warning(Poco::format("COMMAND: Poco exception %s in performing command.",E.displayText()));
		}
		catch ( const std::exception & E) {
			Logger_.warning(Poco::format("COMMAND: std::exception %s in performing command.",std::string(E.what())));
		}

		return Result;
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
			try
			{
				std::string Message;
				std::vector<char>	buffer(1024);
				int n = 1;
				while (n > 0)
				{
					n = socket().receiveBytes(&buffer[0], (int)buffer.size());
					buffer[n] = '\0';
					Message += &buffer[0];
					Logger_.information(Poco::format("COMMAND-CHANNEL: %s",Message));
					if(buffer.size() > n && !Message.empty())
					{
						CommandChannel()->ProcessCommand(Message);
						Message.clear();
					}
				}
			}
			catch (const Poco::Exception & E)
			{
				Logger_.log(E);
			}
		}

	  private:
		Poco::Logger	& Logger_;
	};

	class UnixSocketServerConnectionFactory: public Poco::Net::TCPServerConnectionFactory
	{
	  public:
		explicit UnixSocketServerConnectionFactory() :
			Logger_(CommandChannel()->Logger())
		{
		}

		Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) override
		{
			return new UnixSocketServerConnection(socket,Logger_);
		}
	  private:
		Poco::Logger & Logger_;
	};

	CommandChannel::CommandChannel() noexcept:
		SubSystemServer("CommandChannel", "COMMAND-CHANNEL", "commandchannel")
	{
	}

	void CommandChannel::Stop() {
		Logger_.notice("Stopping...");
		Srv_->stop();
	}

	int CommandChannel::Start() {
		Poco::File	F(Daemon()->ConfigPath("ucentral.system.commandchannel","/tmp/app.ucentralgw"));
		try {
			if (F.exists())
				F.remove();
		} catch (const Poco::Exception &E ) {

		}
		SocketFile_ = std::make_unique<Poco::File>(F);
		UnixSocket_ = std::make_unique<Poco::Net::SocketAddress>(Poco::Net::SocketAddress::UNIX_LOCAL, SocketFile_->path());
		Svs_ = std::make_unique<Poco::Net::ServerSocket>(*UnixSocket_);
		Srv_ = std::make_unique<Poco::Net::TCPServer>(new UnixSocketServerConnectionFactory, *Svs_);
		Srv_->start();
		Logger_.notice("Starting...");
		return 0;
	}
}