//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>
#include <array>

#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
#include "Poco/Net/HTTPRequest.h"

#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {
	class RESTAPI_GenericServerAccounting {
	  public:

		enum {
			LOG_GET=0,
			LOG_DELETE,
			LOG_PUT,
			LOG_POST
		};

		void inline SetFlags(bool External, const std::string &Methods) {
			Poco::StringTokenizer   Tokens(Methods,",");
			auto Offset = (External ? 0 : 4);
			for(const auto &i:Tokens) {
				if(Poco::icompare(i,Poco::Net::HTTPRequest::HTTP_DELETE)==0)
					LogFlags_[Offset+LOG_DELETE]=true;
				else if(Poco::icompare(i,Poco::Net::HTTPRequest::HTTP_PUT)==0)
					LogFlags_[Offset+LOG_PUT]=true;
				else if(Poco::icompare(i,Poco::Net::HTTPRequest::HTTP_POST)==0)
					LogFlags_[Offset+LOG_POST]=true;
				else if(Poco::icompare(i,Poco::Net::HTTPRequest::HTTP_GET)==0)
					LogFlags_[Offset+LOG_GET]=true;
			}
		}

		inline void InitLogging() {
			std::string Public = MicroServiceConfigGetString("apilogging.public.methods","PUT,POST,DELETE");
			SetFlags(true, Public);
			std::string Private = MicroServiceConfigGetString("apilogging.private.methods","PUT,POST,DELETE");
			SetFlags(false, Private);

			std::string PublicBadTokens = MicroServiceConfigGetString("apilogging.public.badtokens.methods","");
			LogBadTokens_[0] = (Poco::icompare(PublicBadTokens,"true")==0);
			std::string PrivateBadTokens = MicroServiceConfigGetString("apilogging.private.badtokens.methods","");
			LogBadTokens_[1] = (Poco::icompare(PrivateBadTokens,"true")==0);
		}

		[[nodiscard]] inline bool LogIt(const std::string &Method, bool External) const {
			auto Offset = (External ? 0 : 4);
			if(Method == Poco::Net::HTTPRequest::HTTP_GET)
				return LogFlags_[Offset+LOG_GET];
			if(Method == Poco::Net::HTTPRequest::HTTP_POST)
				return LogFlags_[Offset+LOG_POST];
			if(Method == Poco::Net::HTTPRequest::HTTP_PUT)
				return LogFlags_[Offset+LOG_PUT];
			if(Method == Poco::Net::HTTPRequest::HTTP_DELETE)
				return LogFlags_[Offset+LOG_DELETE];
			return false;
		};

		[[nodiscard]] inline bool LogBadTokens(bool External) const {
			return LogBadTokens_[ (External ? 0 : 1) ];
		};

	  private:
		std::array<bool,8>       LogFlags_{false};
		std::array<bool,2>       LogBadTokens_{false};
	};
}