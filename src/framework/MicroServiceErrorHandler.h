//
// Created by stephane bourque on 2022-09-29.
//

#pragma once

#include "fmt/format.h"
#include "Poco/Util/Application.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/SSLException.h"
#include "Poco/Thread.h"

namespace OpenWifi {

	class MicroServiceErrorHandler : public Poco::ErrorHandler {
	  public:
		explicit MicroServiceErrorHandler(Poco::Util::Application &App) : App_(App) {}

		inline void exception(const Poco::Exception & Base) override {
			try {
				App_.logger().log(Base);
				Base.rethrow();
			} catch (const Poco::Net::InvalidCertificateException &E) {
				poco_error(App_.logger(), fmt::format("Poco::Net::InvalidCertificateException thr_name={} thr_id={} code={} text={} msg={} what={}",
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
			} catch (const Poco::RuntimeException &E) {
				poco_error(App_.logger(), fmt::format("Poco::RuntimeException thr_name={} thr_id={} code={} text={} msg={} what={}",
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
			}
		}

		inline void exception(const std::exception & E) override {
			Poco::Thread * CurrentThread = Poco::Thread::current();
			poco_warning(App_.logger(), fmt::format("std::exception in {}: {} thr_id={}",
													CurrentThread->getName(),E.what(),
													CurrentThread->id()));
		}

		inline void exception() override {
			Poco::Thread * CurrentThread = Poco::Thread::current();
			poco_warning(App_.logger(), fmt::format("generic exception in {} thr_id={}",
													CurrentThread->getName(), CurrentThread->id()));
		}
	  private:
		Poco::Util::Application	&App_;
		std::string		t_name{Poco::Thread::current()->getName()};
		int 			t_id{Poco::Thread::current()->id()};
	};

}