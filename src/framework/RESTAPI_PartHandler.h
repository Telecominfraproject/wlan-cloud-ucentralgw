//
// Created by stephane bourque on 2022-10-26.
//

#pragma once

#include <string>
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/CountingStream.h"
#include "Poco/NullStream.h"
#include "Poco/StreamCopier.h"

namespace OpenWifi {
	class RESTAPI_PartHandler: public Poco::Net::PartHandler {
	  public:
		RESTAPI_PartHandler():
								_length(0)
		{
		}

		inline void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream) override
		{
			_type = header.get("Content-Type", "(unspecified)");
			if (header.has("Content-Disposition"))
			{
				std::string disp;
				Poco::Net::NameValueCollection params;
				Poco::Net::MessageHeader::splitParameters(header["Content-Disposition"], disp, params);
				_name = params.get("name", "(unnamed)");
				_fileName = params.get("filename", "(unnamed)");
			}

			Poco::CountingInputStream istr(stream);
			Poco::NullOutputStream ostr;
			Poco::StreamCopier::copyStream(istr, ostr);
			_length = (int)istr.chars();
		}

		[[nodiscard]] inline int length() const
		{
			return _length;
		}

		[[nodiscard]] inline const std::string& name() const
		{
			return _name;
		}

		[[nodiscard]] inline const std::string& fileName() const
		{
			return _fileName;
		}

		[[nodiscard]] inline const std::string& contentType() const
		{
			return _type;
		}

	  private:
		int _length;
		std::string _type;
		std::string _name;
		std::string _fileName;
	};
}
