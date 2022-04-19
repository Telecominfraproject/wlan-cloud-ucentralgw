//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include <array>

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketNotifier.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/NObserver.h"
#include "Poco/FIFOBuffer.h"

namespace OpenWifi {

inline static const std::size_t RTTY_DEVICE_BUFSIZE=64000;

template <typename T,std::size_t S> class Ringer {
  public:
	inline bool get(T &v1) {
		if(empty()) return false;
		v1 = buf_[head_];
		advance(1);
		return true;
	}

	inline void advance(int s) {
		head_+=s;
		if(head_==S) head_=0;
	}

	inline bool get(T &v1, T &v2) {
		return get(v1) && get(v2);
	}

	inline bool enough_room(int size) {
		if(empty()) return true;
		if(tail_<head_) {
			return head_-tail_>size;
		}
		return S-tail_+head_>size;
	}

	inline bool enough_data(int size) {
		if(empty()) return false;
		if(tail_>head_) {
			return size < (tail_-head_);
		}
		return (S-head_+tail_);
	}

	inline bool get(int size, T * p) {
		if(empty()) return false;
		if(!enough_data(size)) return false;
		for (int i = 0; i < size; i++)
			p[i] = buf_[(head_ + i) % S];
		head_ = (head_ + size) % S;
		return true;
	}

	inline bool add(int size, const T *p) {
		if(enough_room(size)) {
			for(int i=0;i<size;i++)
				buf_[(tail_+i) % S] = p[i];
			tail_ = (tail_ + size) % S;
			return true;
		}
		return false;
	}

	inline bool full() {
		if( head_<tail_ && (tail_-head_)>1) return false;
		if( head_>tail_ && (head_-tail_)>1) return false;
		return true;
	}

	[[nodiscard]] inline bool empty() const {
		return (tail_==head_);
	}

	[[nodiscard]] inline int available() const {
		if(empty()) return 0;
		if(tail_>head_)
			return tail_-head_;
		return S-head_+tail_;
	}

	[[nodiscard]] inline int free() const {
		if(empty()) return S;
		if(tail_>head_)
			return S-tail_+head_;
		return head_-tail_;
	}

	// template <typename U,std::size_t SS> friend std::ostream & operator<<(std::ostream& os, const Ringer<U,SS> & R);

  private:
	std::array<T,S>	buf_{0};
	int head_=0, tail_=0;
};

class RTTY_Device_ConnectionHandler {

  public:
	enum RTTY_MSG_TYPE {
		msgTypeRegister = 0,
		msgTypeLogin,
		msgTypeLogout,
		msgTypeTermData,
		msgTypeWinsize,
		msgTypeCmd,
		msgTypeHeartbeat,
		msgTypeFile,
		msgTypeHttp,
		msgTypeAck,
		msgTypeMax };

	RTTY_Device_ConnectionHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor & reactor);
	~RTTY_Device_ConnectionHandler();

	void onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
	void onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);

	std::string SafeCopy( const u_char * buf, int MaxSize, int & NewPos);
	void PrintBuf(const u_char * buf, int size);
	int SendMessage( RTTY_MSG_TYPE Type, const u_char * Buf, int len);
	int SendMessage( RTTY_MSG_TYPE Type, std::string &S );
	int SendMessage( RTTY_MSG_TYPE Type);

	bool Login();
	bool Logout();

	void SendToClient(const u_char *buf, int len);
	void SendToClient(const std::string &S);
	void WindowSize(int cols, int rows);
	void KeyStrokes(const u_char *buf, size_t len);
	std::string ReadString();

  private:
	Poco::Logger & Logger() { return Logger_; }
	Poco::Net::StreamSocket       socket_;
	Poco::Net::SocketReactor&     reactor_;
	std::string                   id_;
	std::string                   token_;
	std::string                   desc_;
	std::string 				  serial_;
	char 				          sid_=0;
	Poco::Logger				  &Logger_;
	Poco::FIFOBuffer  			  inBuf_{64000};
	std::array<char,32000>		  scratch_{0};
	std::size_t      			  waiting_for_bytes_{0};
	u_char 						  last_command_=0;


	// Ringer<unsigned char,RTTY_DEVICE_BUFSIZE>	inBuf;
};


}
