//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_file.h"

#include "uFileUploader.h"
#include "uStorageService.h"
#include "Poco/File.h"

#include <fstream>

void RESTAPI_file::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    try {
        ParseParameters(Request);

        if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            auto UUID = GetBinding("uuid", "");

            //does the file exist
            Poco::File  DownloadFile(uCentral::uFileUploader::Path() + "/" + UUID);

			std::cout << "File: " << DownloadFile.path() << std::endl;

            if(!uCentral::Storage::GetAttachedFile(UUID,DownloadFile.path()))
            {
                NotFound(Response);
                return;
            }

			std::cout << "Preparing to send file" << std::endl;

            Response.set("Content-Type","application/octet-stream");
            Response.set("Content-Disposition", "attachment; filename=" + UUID );
            Response.set("Content-Transfer-Encoding","binary");
            Response.set("Accept-Ranges", "bytes");
            Response.set("Cache-Control", "private");
            Response.set("Pragma", "private");
            Response.set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
            Response.set("Content-Length", std::to_string(DownloadFile.getSize()));
            Response.sendFile(DownloadFile.path(),"application/octet-stream");
			std::cout << "File sent..." << std::endl;
			DownloadFile.remove();
			std::cout << "File removed..." << std::endl;

            return;

        } else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
            auto UUID = GetBinding("uuid", "");

			if(UUID.empty())
			{
				BadRequest(Response);
				return;
			}

			if(uCentral::Storage::RemoveAttachedFile(UUID))
				OK(Response);
			else
				NotFound(Response);
        }
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}