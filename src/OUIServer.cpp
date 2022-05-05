//
// Created by stephane bourque on 2021-06-17.
//
#include <thread>
#include <fstream>
#include <vector>

#include "OUIServer.h"

#include "Poco/String.h"
#include "Poco/StringTokenizer.h"
#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/File.h"

#include "OUIServer.h"
#include "framework/MicroService.h"

namespace OpenWifi {

	int OUIServer::Start() {
		Running_ = true;
		UpdaterCallBack_ = std::make_unique<Poco::TimerCallback<OUIServer>>(*this, &OUIServer::onTimer);
		Timer_.setStartInterval(30 * 1000);  // first run in 5 minutes
		Timer_.setPeriodicInterval(7 * 24 * 60 * 60 * 1000);
		Timer_.start(*UpdaterCallBack_);
		return 0;
	}

	void OUIServer::Stop() {
		Running_=false;
		Timer_.stop();
	}

	void OUIServer::reinitialize([[maybe_unused]] Poco::Util::Application &self) {
		MicroService::instance().LoadConfigurationFile();
		Logger().information("Reinitializing.");
		Stop();
		Start();
	}

	bool OUIServer::GetFile(const std::string &FileName) {
		try {
			LastUpdate_ = OpenWifi::Now();
			Logger().information(fmt::format("Start: Retrieving OUI file: {}",MicroService::instance().ConfigGetString("oui.download.uri")));
			std::unique_ptr<std::istream> pStr(
				Poco::URIStreamOpener::defaultOpener().open(MicroService::instance().ConfigGetString("oui.download.uri")));
			std::ofstream OS;
			OS.open(FileName);
			Poco::StreamCopier::copyStream(*pStr, OS);
			OS.close();
			Logger().information(fmt::format("Done: Retrieving OUI file: {}",MicroService::instance().ConfigGetString("oui.download.uri")));
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool OUIServer::ProcessFile( const std::string &FileName, OUIMap &Map) {
		try {
			std::ifstream Input;
			Input.open(FileName, std::ios::binary);

			while (!Input.eof()) {
				if(!Running_)
					return false;
				char buf[1024];
				Input.getline(buf, sizeof(buf));
				std::string Line{buf};
				auto Tokens = Poco::StringTokenizer(Line, " \t",
													Poco::StringTokenizer::TOK_TRIM |
														Poco::StringTokenizer::TOK_IGNORE_EMPTY);

				if (Tokens.count() > 2) {
					if (Tokens[1] == "(hex)") {
						auto MAC = Utils::SerialNumberToOUI(Tokens[0]);
						if (MAC > 0) {
							std::string Manufacturer;
							for (size_t i = 2; i < Tokens.count(); i++)
								Manufacturer += Tokens[i] + " ";
							auto M = Poco::trim(Manufacturer);
							if (!M.empty())
								Map[MAC] = M;
						}
					}
				}
			}
			return true;
		} catch ( const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	void OUIServer::onTimer([[maybe_unused]] Poco::Timer & timer) {
		if(Updating_)
			return;
		Updating_ = true;

		//	fetch data from server, if not available, just use the file we already have.
		std::string LatestOUIFileName{ MicroService::instance().DataDir() + "/newOUIFile.txt"};
		std::string CurrentOUIFileName{ MicroService::instance().DataDir() + "/current_oui.txt"};

		OUIMap TmpOUIs;
		if(GetFile(LatestOUIFileName) && ProcessFile(LatestOUIFileName, TmpOUIs)) {
			std::lock_guard G(Mutex_);
			OUIs_ = std::move(TmpOUIs);
			LastUpdate_ = OpenWifi::Now();
			Poco::File F1(CurrentOUIFileName);
			if(F1.exists())
				F1.remove();
			Poco::File F2(LatestOUIFileName);
			F2.renameTo(CurrentOUIFileName);
			Logger().information(fmt::format("New OUI file {} downloaded.",LatestOUIFileName));
		} else if(OUIs_.empty()) {
			if(ProcessFile(CurrentOUIFileName, TmpOUIs)) {
				LastUpdate_ = OpenWifi::Now();
				std::lock_guard G(Mutex_);
				OUIs_ = std::move(TmpOUIs);
			}
		}
		Updating_ = false;
	}

	std::string OUIServer::GetManufacturer(const std::string &MAC) {
		std::lock_guard Guard(Mutex_);
		auto Manufacturer = OUIs_.find(Utils::SerialNumberToOUI(MAC));
		if(Manufacturer != OUIs_.end())
			return Manufacturer->second;
		return "";
	}
};
