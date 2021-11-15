//
// Created by stephane bourque on 2021-07-12.
//

#ifndef UCENTRALGW_STORAGEARCHIVER_H
#define UCENTRALGW_STORAGEARCHIVER_H

#include <functional>

#include "framework/MicroService.h"
#include "Poco/Timer.h"

namespace OpenWifi {

    static const std::list<std::string>		AllInternalDBNames{"healthchecks", "statistics", "devicelogs" , "commandlist" };

    class Archiver {
      public:
    	struct ArchiverDBEntry {
    		std::string 					DBName;
    		uint64_t 						HowManyDays=7;
    	};
    	typedef std::vector<ArchiverDBEntry>	ArchiverDBEntryVec;

		explicit Archiver(Poco::Logger &Logger):
			Logger_(Logger) {
		}

    	void onTimer(Poco::Timer & timer);
    	inline void AddDb(const ArchiverDBEntry &E ) {
			DBs_.push_back(E);
		}
      private:
		Poco::Logger		&Logger_;
    	ArchiverDBEntryVec	DBs_;
    };

    class StorageArchiver : public SubSystemServer {

        public:
            static StorageArchiver *instance() {
                static StorageArchiver * instance_ = new StorageArchiver;
                return instance_;
            }

            int 	Start() override;
            void 	Stop() override;
            inline bool Enabled() const { return Enabled_; }

        private:
            std::atomic_bool 				Enabled_ = false;
            Poco::Timer                     Timer_;
            Archiver                        Archiver_{Logger_};
            std::unique_ptr<Poco::TimerCallback<Archiver>>   ArchiverCallback_;

            StorageArchiver() noexcept:
                SubSystemServer("StorageArchiver", "STORAGE-ARCHIVE", "archiver")
            {
            }
    };

    inline StorageArchiver * StorageArchiver() { return StorageArchiver::instance(); }

}  // namespace


#endif // UCENTRALGW_STORAGEARCHIVER_H
