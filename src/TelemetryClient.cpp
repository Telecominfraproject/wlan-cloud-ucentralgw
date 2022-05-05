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
		std::unique_ptr<Poco::Net::WebSocket> WSock,
		Poco::Net::SocketReactor& Reactor,
		Poco::Logger &Logger):
				UUID_(std::move(UUID)),
				SerialNumber_(SerialNumber),
				Reactor_(Reactor),
				Logger_(Logger),
				WS_(std::move(WSock)) {
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
				Logger().information(fmt::format("CONNECTION({}): completed.", CId_));
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
		}
		WS_->close();
	}

	bool TelemetryClient::Send(const std::string &Payload) {
		std::lock_guard Guard(Mutex_);
		size_t BytesSent = WS_->sendFrame(Payload.c_str(),(int)Payload.size());
		return  BytesSent == Payload.size();
	}

	void TelemetryClient::SendTelemetryShutdown() {
		Logger().information(fmt::format("TELEMETRY-SHUTDOWN({}): Closing.",CId_));
		auto Device = DeviceRegistry()->GetDeviceConnection(SerialNumber_);
		if(Device) {
			if(Device->WSConn_)
				Device->WSConn_->StopWebSocketTelemetry();
		}
		TelemetryStream()->DeRegisterClient(UUID_);
		delete this;
	}

	void TelemetryClient::OnSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		std::lock_guard Guard(Mutex_);
		Logger().information(fmt::format("SOCKET-SHUTDOWN({}): Orderly shutdown.", CId_));
		SendTelemetryShutdown();
	}

	void TelemetryClient::OnSocketError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
		std::lock_guard Guard(Mutex_);
		Logger().information(fmt::format("SOCKET-ERROR({}): Closing.",CId_));
		SendTelemetryShutdown();
	}

	void TelemetryClient::OnSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
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
			Logger().information(fmt::format("std::exception caught: {}. Connection terminated with {}",W,CId_));
			SendTelemetryShutdown();
		}
		catch ( ... ) {
			Logger().information(fmt::format("Unknown exception for {}. Connection terminated.",CId_));
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
				Logger().information(fmt::format("DISCONNECT({}): device has disconnected.", CId_));
				MustDisconnect = true;
			} else {
				if (Op == Poco::Net::WebSocket::FRAME_OP_PING) {
					Logger().debug(fmt::format("WS-PING({}): received. PONG sent back.", CId_));
					WS_->sendFrame("", 0,
								   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
									   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
				} else if (Op == Poco::Net::WebSocket::FRAME_OP_CLOSE) {
					Logger().information(fmt::format("DISCONNECT({}): device wants to disconnect.", CId_));
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