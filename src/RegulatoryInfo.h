//
// Created by stephane bourque on 2023-02-24.
//

#pragma once

#include <RESTObjects/RESTAPI_GWobjects.h>
#include <framework/MicroServiceFuncs.h>
#include <framework/SubSystemServer.h>
#include <framework/utils.h>
#include <Poco/File.h>
#include <fmt/format.h>

namespace OpenWifi {

	class RegulatoryInfo : public SubSystemServer, Poco::Runnable {

		const static inline std::string db_uri="https://git.kernel.org/pub/scm/linux/kernel/git/sforshee/wireless-regdb.git/plain/db.txt";

	  public:
		static auto instance() {
			static auto instance_ = new RegulatoryInfo;
			return instance_;
		}

		inline int Start() override {
			poco_information(Logger(),"Starting...");
			DbWorker_.start(*this);
			TmpFileName_ = MicroServiceDataDirectory() + "/regulatory_db.txt";
			return 0;
		}

		inline void Stop() override {
			poco_information(Logger(),"Stopping...");
			poco_information(Logger(),"Stopped...");
		}

		inline void Get(const std::string & CountryList, std::vector<GWObjects::RegulatoryCountryInfo> & InfoList) {
			std::lock_guard	G(Mutex_);
			auto Countries = Poco::StringTokenizer(CountryList,",",Poco::StringTokenizer::TOK_TRIM);
			for(const auto &country:Countries) {
				auto Hint = Info_.find(country);
				if(Hint!=end(Info_)) {
					InfoList.emplace_back(Hint->second);
				}
			}
		}

		inline void run() override {
			std::lock_guard	G(Mutex_);

			Poco::File	F(TmpFileName_);

			if(!F.exists()) {
				Poco::URI		URI(db_uri);
				if(!Utils::wgetfile(URI,TmpFileName_)) {
					poco_warning(Logger(), fmt::format("Cannot download regulatory file {}", db_uri));
					return;
				}
			}
			parse_regulatory_db(TmpFileName_);
		}

		inline void Reload() {
			Poco::File	F(TmpFileName_);

			if(F.exists())
				F.remove();
			DbWorker_.start(*this);
			DbWorker_.join();
		}

	  private:
		GWObjects::RegulatoryInfoCountryMap	Info_;
		std::string TmpFileName_;

		Poco::Thread		DbWorker_;

		RegulatoryInfo() noexcept : SubSystemServer("Regulatory", "REG-INFO", "reqinfo") {}

		static inline float mw2db(double mw) {
			return (float)round(10.0 * log10(mw));
		}

		static inline float db2mw(double db) {
			return (float)round(pow(10,db/10));
		}

		inline bool parse_regulatory_db(const std::string &dbname) {
			std::ifstream       ifs(dbname.c_str());
			std::string     CurrentCountry;
			bool            inCountry=false, inWmmrule=false;
			GWObjects::RegulatoryCountryInfo CurrentRegInfo;
			Info_.clear();

			while(ifs.good()) {
				char line_buf[256];
				ifs.getline(line_buf,sizeof(line_buf));
				std::string line(line_buf);

				Poco::trimInPlace(line);

				if (line.empty()) {
					if(inCountry) {
						inCountry = false;
						CurrentRegInfo.country = CurrentCountry;
						Info_[CurrentCountry] = CurrentRegInfo;
						CurrentCountry = "";
						CurrentRegInfo.domain.clear();
						CurrentRegInfo.ranges.clear();
					}
					if(inWmmrule) {
						inWmmrule = false;
					}
				} else if(line[0]=='#') {
					//	nothing to do for comments
				} else {
					if(line.substr(0,7)=="country") {
						CurrentCountry = line.substr(8,2);
						if(line.length()>12)
							CurrentRegInfo.domain = line.substr(12);
						inCountry = true;
					} else if (line.substr(0,7)=="wmmrule") {
						inWmmrule = true;
					} else {
						if(inCountry) {
							GWObjects::FrequencyRange  fr;
							auto tokens = Poco::StringTokenizer(line,",", Poco::StringTokenizer::TOK_TRIM);
							std::sscanf(tokens[0].c_str(),"(%f - %f @ %d)", &fr.from, &fr.to, &fr.channelWidth);
							if(tokens[1].find("mW")!=std::string::npos) {
								int temp_pwr;
								std::sscanf(tokens[1].c_str(),"(%d mW)", &temp_pwr);
								fr.powerDb = mw2db(temp_pwr);
							} else {
								std::sscanf(tokens[1].c_str(),"(%d)", &fr.powerDb);
							}

							if(tokens.count()>2) {
								for(std::size_t i=2;i<tokens.count();++i) {
									if(tokens[i]=="NO-IR") {
										fr.options.NO_IR=true;
									} else if(tokens[i]=="AUTO-BW") {
										fr.options.AUTO_BW=true;
									} else if(tokens[i]=="DFS") {
										fr.options.DFS=true;
									} else if(tokens[i]=="NO-OUTDOOR") {
										fr.options.NO_OUTDOOR=true;
									} else if(tokens[i]=="wmmrule=ETSI") {
										fr.options.wmmrule_ETSI=true;
									} else if(tokens[i]=="NO-OFDM") {
										fr.options.NO_OFDM=true;
									} else {
										std::cout << tokens[i] << std::endl;
									}
								}
							}
							CurrentRegInfo.ranges.emplace_back(fr);
						} else if(inWmmrule) {

						} else {
							std::cout << line << std::endl;
						}
					}
				}
			}
			return true;
		}
	};

	inline auto RegulatoryInfo() { return RegulatoryInfo::instance(); }

} // namespace OpenWifi

