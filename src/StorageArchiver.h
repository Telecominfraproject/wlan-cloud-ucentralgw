//
// Created by stephane bourque on 2021-07-12.
//

#ifndef UCENTRALGW_STORAGEARCHIVER_H
#define UCENTRALGW_STORAGEARCHIVER_H

#include <functional>

#include "framework/MicroService.h"

namespace OpenWifi {

    struct ArchiverDBEntry {
        std::string 					DBName;
        uint64_t 						HowManyDays=7;
    };
    typedef std::vector<ArchiverDBEntry>	ArchiverDBEntryVec;

    static const std::list<std::string>		AllInternalDBNames{"healthchecks", "statistics", "devicelogs" , "commandlist" };

    class StorageArchiver : public SubSystemServer, Poco::Runnable {

        public:
            static StorageArchiver *instance() {
                static StorageArchiver * instance_ = new StorageArchiver;
                return instance_;
            }

            void 	run() override;
            int 	Start() override;
            void 	Stop() override;
            void 	GetLastRun();
            void 	SetLastRun();

            inline bool Enabled() const { return Enabled_; }

        private:
            std::atomic_bool 			Running_ = false;
            std::atomic_bool 			Enabled_ = false;
            Poco::Thread				Janitor_;
            ArchiverDBEntryVec			DBs_;
            uint64_t 					RunAtHour_=0;
            uint64_t 					RunAtMin_=0;
            uint64_t 					LastRun_=0;
            std::string 				LastRunFileName_;

            StorageArchiver() noexcept:
                SubSystemServer("StorageArchiver", "STORAGE-ARCHIVE", "archiver")
            {
            }
    };

    inline StorageArchiver * StorageArchiver() { return StorageArchiver::instance(); }

}  // namespace


#endif // UCENTRALGW_STORAGEARCHIVER_H
