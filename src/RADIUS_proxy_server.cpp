//
// Created by stephane bourque on 2022-05-18.
//

#include "RADIUS_proxy_server.h"
#include "DeviceRegistry.h"

namespace OpenWifi {

	const int SMALLEST_RADIUS_PACKET = 20+19+4;
	const int RADIUS_BUFFER_SIZE = 2048;
	const int DEFAULT_RADIUS_AUTHENTICATION_PORT = 1812;
	const int DEFAULT_RADIUS_ACCOUNTING_PORT = 1813;

	int RADIUS_proxy_server::Start() {

		Poco::Net::SocketAddress	AuthSockAddr(Poco::Net::AddressFamily::IPv4,
									   MicroService::instance().ConfigGetInt("radius.proxy.authentication.port",DEFAULT_RADIUS_AUTHENTICATION_PORT));
		AuthenticationSocket_ = std::make_unique<Poco::Net::DatagramSocket>(AuthSockAddr,true);

		Poco::Net::SocketAddress	AcctSockAddr(Poco::Net::AddressFamily::IPv4,
									   MicroService::instance().ConfigGetInt("radius.proxy.accounting.port",DEFAULT_RADIUS_ACCOUNTING_PORT));
		AccountingSocket_ = std::make_unique<Poco::Net::DatagramSocket>(AcctSockAddr,true);

		AuthenticationReactor_.addEventHandler(*AuthenticationSocket_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
														   *this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));
		AccountingReactor_.addEventHandler(*AccountingSocket_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																		  *this, &RADIUS_proxy_server::OnAccountingSocketReadable));
		AuthenticationReactorThread_.start(AuthenticationReactor_);
		AccountingReactorThread_.start(AccountingReactor_);
		return 0;
	}

	void RADIUS_proxy_server::Stop() {
		AuthenticationReactor_.removeEventHandler(*AuthenticationSocket_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																		  *this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));
		AccountingReactor_.removeEventHandler(*AccountingSocket_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																  *this, &RADIUS_proxy_server::OnAccountingSocketReadable));
		AuthenticationReactor_.stop();
		AuthenticationReactorThread_.join();

		AccountingReactor_.stop();
		AccountingReactorThread_.join();
	}

	std::string ExtractSerialNumber(const unsigned char *b, uint32_t s) {
		std::string result;

		try {
			uint32_t pos=0;
			while(pos<s) {
				auto len = b[pos+1];
				if(b[pos] != 26) {
					pos += len;
					continue;
				}

				uint32_t vendor_id = b[pos+2] * 256 * 256 * 256  + b[pos+3] * 256 * 256 + b[pos+4] * 256+ b[pos+5];
				if(vendor_id!=58888) {
					pos += len;
					continue;
				}

				auto tpos0 = pos + 6;
				auto tend0 = tpos0 + b[tpos0+1];
				while (tpos0<tend0) {
					if (b[tpos0] == 71) {
						auto tpos1 = tpos0+2;
						auto tlen1 = b[tpos0+1];
						auto tend1 = tpos1 + tlen1;
						while (tpos1 < tend1) {
							if (b[tpos1] == 0x01) {
								uint32_t tlen = b[tpos1 + 1] - 2;
								tpos1 += 2;
								while (tlen) {
									if (b[tpos1] != '-') {
										result += (char) b[tpos1];
									}
									tlen--;
									tpos1++;
								}
								if (result.size() == 12)
									return result;
								return "";
							} else {
								tpos1 += b[tpos1+1];
							}
						}
					} else {
						tpos0 += b[tpos0 + 1];
					}
				}
				pos += len;
			}
		} catch (...) {

		}

		return result;
	}


	void RADIUS_proxy_server::OnAccountingSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		Poco::Net::SocketAddress	Sender;
		unsigned char Buffer[RADIUS_BUFFER_SIZE];

		auto ReceiveSize = AccountingSocket_->receiveBytes(Buffer,sizeof(Buffer));
		if(ReceiveSize<SMALLEST_RADIUS_PACKET)
			return;
		auto SerialNumber = ExtractSerialNumber(&Buffer[20],ReceiveSize);
		Logger().information(fmt::format("Accounting Packet received for {}",SerialNumber));
		std::cout << "Received an Accounting packet for :" << SerialNumber << std::endl;
		DeviceRegistry()->SendRadiusAccountingData(SerialNumber,Buffer,ReceiveSize);
	}

	void RADIUS_proxy_server::OnAuthenticationSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		Poco::Net::SocketAddress	Sender;
		unsigned char Buffer[RADIUS_BUFFER_SIZE];

		auto ReceiveSize = AuthenticationSocket_->receiveBytes(Buffer,sizeof(Buffer));
		if(ReceiveSize<SMALLEST_RADIUS_PACKET)
			return;
		auto SerialNumber = ExtractSerialNumber(&Buffer[20],ReceiveSize);
		Logger().information(fmt::format("Authentication Packet received for {}",SerialNumber));
		std::cout << "Received an Authentication packet for :" << SerialNumber << std::endl;
		DeviceRegistry()->SendRadiusAuthenticationData(SerialNumber,Buffer,ReceiveSize);
	}

	void RADIUS_proxy_server::SendAccountingData(const std::string &serialNumber, const std::string &Destination,const char *buffer, std::size_t size) {
		Poco::Net::SocketAddress	Dst(Destination);
		AccountingSocket_->sendTo(buffer,(int)size,Dst);
		Logger().information(fmt::format("{}: Sending Accounting Packet to {}", serialNumber, Destination));
		std::cout << "Sending Accounting data to " << Destination << std::endl;
	}

	void RADIUS_proxy_server::SendAuthenticationData(const std::string &serialNumber, const std::string &Destination,const char *buffer, std::size_t size) {
		Poco::Net::SocketAddress	Dst(Destination);
		AuthenticationSocket_->sendTo(buffer,(int)size,Dst);
		Logger().information(fmt::format("{}: Sending Authentication Packet to {}", serialNumber, Destination));
		std::cout << "Sending Authentication data to " << Destination << std::endl;
	}
}