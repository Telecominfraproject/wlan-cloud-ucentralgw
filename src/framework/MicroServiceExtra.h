//
// Created by stephane bourque on 2022-10-26.
//

#pragma once

#include <string>
#include <map>

#include "Poco/BasicEvent.h"
#include "Poco/ExpireLRUCache.h"

namespace OpenWifi {
	class ConfigurationEntry {
	  public:
		template <typename T> explicit ConfigurationEntry(T def) :
											 Default_(def),
											 Current_(def){
		}

		template <typename T> explicit ConfigurationEntry(T def, T cur, const std::string  &Hint="") :
																				  Default_(def),
																				  Current_(cur),
																				  Hint_(Hint){
		}

		inline ConfigurationEntry()=default;
		inline ~ConfigurationEntry()=default;

		template <typename T> explicit operator T () const { return std::get<T>(Current_); }
		inline ConfigurationEntry & operator=(const char *v) { Current_ = std::string(v); return *this;}
		template <typename T> ConfigurationEntry &  operator=(T v) { Current_ = (T) v; return *this;}

		void reset() {
			Current_ = Default_;
		}

	  private:
		std::variant<bool,uint64_t,std::string> Default_, Current_;
		std::string Hint_;
	};
	inline std::string to_string(const ConfigurationEntry &v) { return (std::string) v; }

	typedef std::map<std::string,ConfigurationEntry>    ConfigurationMap_t;

	template <typename T> class FIFO {
	  public:
		explicit FIFO(uint32_t Size) :
									   Size_(Size) {
			Buffer_ = new T [Size_];
		}

		~FIFO() {
			delete [] Buffer_;
		}

		mutable Poco::BasicEvent<bool> Writable_;
		mutable Poco::BasicEvent<bool> Readable_;

		inline bool Read(T &t) {
			{
				std::lock_guard M(Mutex_);
				if (Write_ == Read_) {
					return false;
				}

				t = Buffer_[Read_++];
				if (Read_ == Size_) {
					Read_ = 0;
				}
				Used_--;
			}
			bool flag = true;
			Writable_.notify(this, flag);
			return true;
		}

		inline bool Write(const T &t) {
			{
				std::lock_guard M(Mutex_);

				Buffer_[Write_++] = t;
				if (Write_ == Size_) {
					Write_ = 0;
				}
				Used_++;
				MaxEverUsed_ = std::max(Used_,MaxEverUsed_);
			}
			bool flag = true;
			Readable_.notify(this, flag);
			return false;
		}

		inline bool isFull() {
			std::lock_guard M(Mutex_);
			return Used_==Buffer_->capacity();
		}

		inline auto MaxEverUser() const { return MaxEverUsed_; }

	  private:
		std::recursive_mutex    Mutex_;
		uint32_t 				Size_=0;
		uint32_t        		Read_=0;
		uint32_t        		Write_=0;
		uint32_t 				Used_=0;
		uint32_t 				MaxEverUsed_=0;
		T	  					* Buffer_ = nullptr;
	};

	template <class Record, typename KeyType = std::string, int Size=256, int Expiry=60000> class RecordCache {
	  public:
		explicit RecordCache( KeyType Record::* Q) :
												   MemberOffset(Q){
												   };
		inline auto update(const Record &R) {
			return Cache_.update(R.*MemberOffset, R);
		}
		inline auto get(const KeyType &K) {
			return Cache_.get(K);
		}
		inline auto remove(const KeyType &K) {
			return Cache_.remove(K);
		}
		inline auto remove(const Record &R) {
			return Cache_.remove(R.*MemberOffset);
		}
	  private:
		KeyType Record::* MemberOffset;
		Poco::ExpireLRUCache<KeyType,Record>  Cache_{Size,Expiry};
	};
}