//
// Created by stephane bourque on 2022-02-03.
//

#include "framework/MicroService.h"
#include "AP_WS_Connection.h"
#include "DeviceRegistry.h"
#include "TelemetryClient.h"
#include "TelemetryStream.h"

#include "Poco/Net/SSLException.h"

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
		try {
			std::thread T([this]() { this->CompleteStartup(); });
			T.detach();
			return;
		} catch (...) {
			delete this;
		}
	}

	void TelemetryClient::CompleteStartup() {
		try {
			std::lock_guard Guard(Mutex_);
			Socket_ = *WS_;
			CId_ = Utils::FormatIPv6(Socket_.peerAddress().toString());

			if (TelemetryStream()->RegisterClient(UUID_, this)) {
				auto TS = Poco::Timespan(240, 0);

				WS_->setReceiveTimeout(TS);
				WS_->setNoDelay(true);
				WS_->setKeepAlive(true);
				WS_->setMaxPayloadSize(2048);
				WS_->setBlocking(false);
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
				poco_information(Logger(),fmt::format("CONNECTION({}): Connection completed.", CId_));
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
		poco_information(Logger(),fmt::format("CONNECTION({}): Closing connection.", CId_));
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
		poco_information(Logger(),fmt::format("TELEMETRY-SHUTDOWN({}): Closing.",CId_));
		DeviceRegistry()->StopWebSocketTelemetry(SerialNumber_);
		TelemetryStream()->DeRegisterClient(UUID_);
		delete this;
	}

	void TelemetryClient::OnSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		poco_information(Logger(),fmt::format("SOCKET-SHUTDOWN({}): Orderly shutdown.", CId_));
		SendTelemetryShutdown();
	}

	void TelemetryClient::OnSocketError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
		poco_information(Logger(),fmt::format("SOCKET-ERROR({}): Closing.",CId_));
		SendTelemetryShutdown();
	}

	void TelemetryClient::OnSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		try
		{
			std::lock_guard Guard(Mutex_);
			ProcessIncomingFrame();
		}
		catch (const Poco::Exception & E)
		{
			Logger().log(E);
			SendTelemetryShutdown();
		}
		catch (const std::exception & E) {
			std::string W = E.what();
			poco_information(Logger(),fmt::format("std::exception caught: {}. Connection terminated with {}",W,CId_));
			SendTelemetryShutdown();
		}
		catch ( ... ) {
			poco_information(Logger(),fmt::format("Unknown exception for {}. Connection terminated.",CId_));
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
				poco_information(Logger(),fmt::format("DISCONNECT({}): device has disconnected.", CId_));
				MustDisconnect = true;
			} else {
				if (Op == Poco::Net::WebSocket::FRAME_OP_PING) {
					Logger().debug(fmt::format("WS-PING({}): received. PONG sent back.", CId_));
					WS_->sendFrame("", 0,
								   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
									   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
				} else if (Op == Poco::Net::WebSocket::FRAME_OP_CLOSE) {
					poco_information(Logger(),fmt::format("DISCONNECT({}): device wants to disconnect.", CId_));
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