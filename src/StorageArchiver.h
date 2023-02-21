//
// Created by stephane bourque on 2021-07-12.
//

#pragma once

#include <functional>
#include <list>

#include "Poco/Timer.h"

#include "framework/SubSystemServer.h"

namespace OpenWifi {

	static const std::list<std::string> AllInternalDBNames{
		"healthchecks", "statistics", "devicelogs", "commandlist", "fileuploads"};

	class Archiver {
	  public:
		explicit Archiver(Poco::Logger &Logger) : Logger_(Logger) {
			for (const auto &db : AllInternalDBNames) {
				DBs_[db] = 7;
			}
		}

		void onTimer(Poco::Timer &timer);
		inline void AddDb(const std::string &dbname, std::uint64_t retain) {
			DBs_[dbname] = retain;
		}
		inline Poco::Logger &Logger() { return Logger_; }

	  private:
		Poco::Logger &Logger_;
		std::map<std::string, std::uint64_t> DBs_;
	};

	class StorageArchiver : public SubSystemServer {

	  public:
		static auto instance() {
			static auto instance_ = new StorageArchiver;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		inline bool Enabled() const { return Enabled_; }

	  private:
		std::atomic_bool Enabled_ = false;
		Poco::Timer Timer_;
		std::unique_ptr<Archiver> Archiver_;
		std::unique_ptr<Poco::TimerCallback<Archiver>> ArchiverCallback_;

		StorageArchiver() noexcept
			: SubSystemServer("StorageArchiver", "STORAGE-ARCHIVE", "archiver") {}
	};

	inline auto StorageArchiver() { return StorageArchiver::instance(); }

} // namespace OpenWifi
