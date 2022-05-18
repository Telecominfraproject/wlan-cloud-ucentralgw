//
// Created by stephane bourque on 2022-05-18.
//

#include "RADIUS_proxy_server.h"

namespace OpenWifi {

	int RADIUS_proxy_server::Start() {

		Poco::Net::SocketAddress	AuthSockAddr(Poco::Net::AddressFamily::IPv4,
									   MicroService::instance().ConfigGetInt("radius.proxy.authentication.port",1812));
		AuthenticationSocket_.bind(AuthSockAddr,true);
		Poco::Net::SocketAddress	AcctSockAddr(Poco::Net::AddressFamily::IPv4,
									   MicroService::instance().ConfigGetInt("radius.proxy.accounting.port",1813));
		AccountingSocket_.bind(AcctSockAddr,true);

		AuthenticationReactor_.addEventHandler(AuthenticationSocket_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
														   *this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));
		AccountingReactor_.addEventHandler(AccountingSocket_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																		  *this, &RADIUS_proxy_server::OnAccountingSocketReadable));

		AuthenticationReactorThread_.start(AuthenticationReactor_);
		AccountingReactorThread_.start(AccountingReactor_);

		return 0;
	}

	void RADIUS_proxy_server::Stop() {
		AuthenticationReactor_.removeEventHandler(AuthenticationSocket_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																		  *this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));
		AccountingReactor_.removeEventHandler(AccountingSocket_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																  *this, &RADIUS_proxy_server::OnAccountingSocketReadable));
		AuthenticationReactor_.stop();
		AuthenticationReactorThread_.join();

		AccountingReactor_.stop();
		AccountingReactorThread_.join();
	}

	void RADIUS_proxy_server::OnAccountingSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		Poco::Net::SocketAddress	Sender;
		unsigned char Buffer[4096];

		auto ReceiveSize = AccountingSocket_.receiveBytes(Buffer,sizeof(Buffer));
		std::cout << "Received an Accounting packet:" << ReceiveSize << " bytes." << std::endl;
	}

	void RADIUS_proxy_server::OnAuthenticationSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		Poco::Net::SocketAddress	Sender;
		unsigned char Buffer[4096];

		auto ReceiveSize = AuthenticationSocket_.receiveBytes(Buffer,sizeof(Buffer));
		std::cout << "Received an Authentication packet:" << ReceiveSize << " bytes." << std::endl;
	}

}