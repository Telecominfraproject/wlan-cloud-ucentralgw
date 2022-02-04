//
// Created by stephane bourque on 2022-02-03.
//

#include "framework/MicroService.h"

#include "Poco/Net/SSLException.h"

#include "TelemetryClient.h"
#include "TelemetryStream.h"
#include "DeviceRegistry.h"
#include "WS_Connection.h"

namespace OpenWifi {

	TelemetryClient::TelemetryClient(
		std::string UUID,
		uint64_t SerialNumber,
		Poco::SharedPtr<Poco::Net::WebSocket> WSock,
		Poco::Net::SocketReactor& Reactor,
		Poco::Logger &Logger):
				UUID_(std::move(UUID)), SerialNumber_(SerialNumber), WS_(std::move(WSock)),Reactor_(Reactor), Logger_(Logger) {
		std::cout << "Telemetry client creation" << std::endl;
		try {
			std::thread T([this]() { this->CompleteStartup(); });
			T.detach();
			return;
		} catch (...) {
			delete this;
		}
	}

	void TelemetryClient::CompleteStartup() {
		std::lock_guard Guard(Mutex_);
		try {
			Socket_ = *WS_;
			CId_ = Utils::FormatIPv6(Socket_.peerAddress().toString());

			// auto SS = static_cast<Poco::Net::SecureStreamSocketImpl*>((WS_->impl()));
			// SS->havePeerCertificate();

			if (TelemetryStream()->RegisterClient(UUID_, this)) {
				auto TS = Poco::Timespan(240, 0);

				WS_->setReceiveTimeout(TS);
				WS_->setNoDelay(true);
				WS_->setKeepAlive(true);
				WS_->setMaxPayloadSize(2048);
				Reactor_.addEventHandler(
					*WS_, Poco::NObserver<TelemetryClient, Poco::Net::ReadableNotification>(
							  *this, &TelemetryClient::OnSocketReadable));
				Reactor_.addEventHandler(
					*WS_, Poco::NObserver<TelemetryClient, Poco::Net::ShutdownNotification>(
							  *this, &TelemetryClient::OnSocketShutdown));
				Reactor_.addEventHandler(
					*WS_, Poco::NObserver<TelemetryClient, Poco::Net::ErrorNotification>(
							  *this, &TelemetryClient::OnSocketError));
				Registered_ = true;
				Logger().information(Poco::format("CONNECTION(%s): completed.", CId_));
				return;
			}
		} catch (const Poco::Net::SSLException &E) {
			Logger().log(E);
		}
		catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		delete this;
	}

	TelemetryClient::~TelemetryClient() {
		Logger().information("Closing telemetry session.");
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
		Logger().information(Poco::format("TELEMETRY-SHUTDOWN(%s): Closing.",CId_));
		auto Device = DeviceRegistry()->GetDeviceConnection(SerialNumber_);
		if(Device) {
			if(Device->WSConn_)
				Device->WSConn_->StopWebSocketTelemetry();
		}
		TelemetryStream()->DeRegisterClient(UUID_);
		delete this;
	}

	void TelemetryClient::OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		std::lock_guard Guard(Mutex_);
		Logger().information(Poco::format("SOCKET-SHUTDOWN(%s): Orderly shutdown.", CId_));
		SendTelemetryShutdown();
	}

	void TelemetryClient::OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
		std::lock_guard Guard(Mutex_);
		Logger().information(Poco::format("SOCKET-ERROR(%s): Closing.",CId_));
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
			Logger().log(E);
			SendTelemetryShutdown();
		}
		catch (const std::exception & E) {
			std::string W = E.what();
			Logger().information(Poco::format("std::exception caught: %s. Connection terminated with %s",W,CId_));
			SendTelemetryShutdown();
		}
		catch ( ... ) {
			Logger().information(Poco::format("Unknown exception for %s. Connection terminated.",CId_));
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
				Logger().information(Poco::format("DISCONNECT(%s): device has disconnected.", CId_));
				MustDisconnect = true;
			} else {
				if (Op == Poco::Net::WebSocket::FRAME_OP_PING) {
					Logger().debug(Poco::format("WS-PING(%s): received. PONG sent back.", CId_));
					WS_->sendFrame("", 0,
								   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
									   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
				} else if (Op == Poco::Net::WebSocket::FRAME_OP_CLOSE) {
					Logger().information(Poco::format("DISCONNECT(%s): device wants to disconnect.", CId_));
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