//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_file.h"

#include "FileUploader.h"
#include "Poco/File.h"
#include "StorageService.h"

#include <fstream>
#include "RESTAPI_protocol.h"

namespace OpenWifi {
	void RESTAPI_file::handleRequest(Poco::Net::HTTPServerRequest &Request,
									 Poco::Net::HTTPServerResponse &Response) {
		if (!ContinueProcessing(Request, Response))
			return;

		if (!IsAuthorized(Request, Response))
			return;

		try {
			ParseParameters(Request);

			if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
				auto UUID = GetBinding(RESTAPI::Protocol::FILEUUID, "");
				auto SerialNumber = GetParameter(RESTAPI::Protocol::SERIALNUMBER, "");

				// does the file exist
				Poco::File DownloadFile(FileUploader()->Path() + "/" + UUID);

				std::string FileType;
				if (!Storage()->GetAttachedFile(UUID, SerialNumber, DownloadFile.path(),
														FileType)) {
					NotFound(Request, Response);
					return;
				}
				SendFile(DownloadFile, UUID, Request, Response);
				DownloadFile.remove();
				return;

			} else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
				auto UUID = GetBinding(RESTAPI::Protocol::FILEUUID, "");

				if (UUID.empty()) {
					BadRequest(Request, Response);
					return;
				}

				if (Storage()->RemoveAttachedFile(UUID))
					OK(Request, Response);
				else
					NotFound(Request, Response);
			}
			return;
		} catch (const Poco::Exception &E) {
			Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
		}
		BadRequest(Request, Response);
	}
}