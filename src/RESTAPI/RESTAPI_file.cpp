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

#include "framework/ow_constants.h"
#include <fstream>

namespace OpenWifi {
	void RESTAPI_file::DoGet() {
		auto UUID = GetBinding(RESTAPI::Protocol::FILEUUID, "");
		auto SerialNumber = GetParameter(RESTAPI::Protocol::SERIALNUMBER, "");

		std::string FileType;
		std::string FileContent;
		if (!StorageService()->GetAttachedFileContent(UUID, SerialNumber, FileContent, FileType)) {
			return NotFound();
		}
		if(FileType=="pcap")
			SendFileContent(FileContent,"application/vnd.tcpdump.pcap",UUID+".pcap");
		else if(FileType=="gzip")
			SendFileContent(FileContent,"application/gzip",UUID+".tar.gz");
		else
			SendFileContent(FileContent,"application/txt",UUID+".txt");
	}

	void RESTAPI_file::DoDelete() {
		auto UUID = GetBinding(RESTAPI::Protocol::FILEUUID, "");

		if (UUID.empty()) {
			return BadRequest(RESTAPI::Errors::MissingUUID);
		}

		if (StorageService()->RemoveAttachedFile(UUID)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::CouldNotBeDeleted);
	}
}