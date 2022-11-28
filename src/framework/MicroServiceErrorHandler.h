//
// Created by stephane bourque on 2022-09-29.
//

#pragma once

#include "fmt/format.h"
#include "Poco/Util/Application.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/SSLException.h"
#include "Poco/JSON/Template.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Thread.h"

namespace OpenWifi {

	class MicroServiceErrorHandler : public Poco::ErrorHandler {
	  public:
		explicit MicroServiceErrorHandler(Poco::Util::Application &App) : App_(App) {
		}

		inline void exception(const Poco::Exception & Base) override {
			try {
				if(Poco::Thread::current()!= nullptr) {
					t_name = Poco::Thread::current()->getName();
					t_id = Poco::Thread::current()->id();
				} else {
					t_name = "startup_code";
					t_id = 0;
				}

				App_.logger().log(Base);
				Base.rethrow();

			} catch (const Poco::Net::InvalidCertificateException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::InvalidCertificateException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::Net::InvalidSocketException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::InvalidSocketException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::Net::WebSocketException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::WebSocketException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::Net::ConnectionResetException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::ConnectionResetException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::Net::CertificateValidationException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::CertificateValidationException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::Net::SSLConnectionUnexpectedlyClosedException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::SSLConnectionUnexpectedlyClosedException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::Net::SSLContextException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::SSLContextException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::Net::SSLException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::SSLException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));

			} catch (const Poco::Net::InvalidAddressException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::InvalidAddressException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));

			} catch (const Poco::Net::NetException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::NetException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));

			} catch (const Poco::IOException &E) {
				poco_error(App_.logger(), fmt::format("Poco::IOException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::TimeoutException &E) {
				poco_error(App_.logger(), fmt::format("Poco::TimeoutException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::NoThreadAvailableException &E) {
				poco_error(App_.logger(), fmt::format("Poco::NoThreadAvailableException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::OutOfMemoryException &E) {
				poco_error(App_.logger(), fmt::format("Poco::OutOfMemoryException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::BadCastException &E) {
				poco_error(App_.logger(), fmt::format("Poco::BadCastException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::DataException &E) {
				poco_error(App_.logger(), fmt::format("Poco::DataException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::PoolOverflowException &E) {
				poco_error(App_.logger(), fmt::format("Poco::PoolOverflowException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::SystemException &E) {
				poco_error(App_.logger(), fmt::format("Poco::SystemException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::RuntimeException &E) {
				poco_error(App_.logger(), fmt::format("Poco::RuntimeException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::JSON::JSONTemplateException &E) {
				poco_error(App_.logger(), fmt::format("Poco::JSON::JSONTemplateException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::JSON::JSONException &E) {
				poco_error(App_.logger(), fmt::format("Poco::JSON::JSONException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::ApplicationException &E) {
				poco_error(App_.logger(), fmt::format("Poco::ApplicationException thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (const Poco::Exception &E) {
				poco_error(App_.logger(), fmt::format("Poco::Exception thr_name={} thr_id={} code={} text={} msg={} what={}",
													  t_name, t_id, E.code(),
													  E.displayText(),
													  E.message(),
													  E.what()));
			} catch (...) {
				poco_error(App_.logger(), fmt::format("Poco:Generic thr_name={}",t_name, t_id));
			}
		}

		inline void exception(const std::exception & E) override {
			if(Poco::Thread::current()!= nullptr) {
				t_name = Poco::Thread::current()->getName();
				t_id = Poco::Thread::current()->id();
			} else {
				t_name = "startup_code";
				t_id = 0;
			}
			poco_warning(App_.logger(), fmt::format("std::exception in {}: {} thr_id={}",
													t_name,E.what(),
													t_id));
		}

		inline void exception() override {
			if(Poco::Thread::current()!= nullptr) {
				t_name = Poco::Thread::current()->getName();
				t_id = Poco::Thread::current()->id();
			} else {
				t_name = "startup_code";
				t_id = 0;
			}
			poco_warning(App_.logger(), fmt::format("generic exception in {} thr_id={}",
													t_name, t_id));
		}
	  private:
		Poco::Util::Application	&App_;
		std::string		t_name;
		int 			t_id=0;
	};

}