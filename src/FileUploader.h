//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UFILEUPLOADER_H
#define UCENTRAL_UFILEUPLOADER_H

#include "SubSystemServer.h"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "RESTAPI_handler.h"

namespace OpenWifi {

    class FileUploader : public SubSystemServer {
    public:
		int Start() override;
		void Stop() override;
		const std::string & FullName();
		bool AddUUID( const std::string & UUID);
		bool ValidRequest(const std::string & UUID);
		void RemoveRequest(const std::string &UUID);
		const std::string & Path() { return Path_; };

        static FileUploader *instance() {
            if (instance_ == nullptr) {
                instance_ = new FileUploader;
            }
            return instance_;
        }

		[[nodiscard]] inline uint64_t MaxSize() const { return MaxSize_; }

    private:
        static FileUploader *instance_;
        std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   Servers_;
		Poco::ThreadPool				Pool_;
        std::string                     FullName_;
        std::map<std::string,uint64_t>  OutStandingUploads_;
        std::string                     Path_;
		uint64_t 						MaxSize_=10000000;

		explicit FileUploader() noexcept:
			SubSystemServer("FileUploader", "FILE-UPLOAD", "openwifi.fileuploader")
		{
		}
    };

    class FileUpLoaderRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        explicit FileUpLoaderRequestHandlerFactory(Poco::Logger &L) :
                Logger_(L){}

        Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
    private:
        Poco::Logger    & Logger_;
    };

	inline FileUploader * FileUploader() { return FileUploader::instance(); }
} //   namespace

#endif //UCENTRAL_UFILEUPLOADER_H
