//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <iostream>
#include <fstream>
#include <cstdio>

#include "Daemon.h"
#include "FileUploader.h"
#include "StorageService.h"

#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/DynamicAny.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/CountingStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"

#include "Utils.h"

namespace OpenWifi {
    class FileUploader *FileUploader::instance_ = nullptr;

    static const std::string URI_BASE{"/v1/upload/"};

    int FileUploader::Start() {
        Logger_.notice("Starting.");

        Poco::File UploadsDir(Daemon()->ConfigPath("openwifi.fileuploader.path","/tmp"));
        Path_ = UploadsDir.path();
        if(!UploadsDir.exists()) {
        	try {
        		UploadsDir.createDirectory();
        	} catch (const Poco::Exception &E) {
        		Logger_.log(E);
        		Path_ = "/tmp";
        	}
        }
        for(const auto & Svr: ConfigServersList_) {
            std::string l{"Starting: " +
                          Svr.Address() + ":" + std::to_string(Svr.Port()) +
                          " key:" + Svr.KeyFile() +
                          " cert:" + Svr.CertFile()};
            Logger_.information(l);

            auto Sock{Svr.CreateSecureSocket(Logger_)};

			Svr.LogCert(Logger_);
			if(!Svr.RootCA().empty())
				Svr.LogCas(Logger_);

            auto Params = new Poco::Net::HTTPServerParams;
            Params->setMaxThreads(16);
            Params->setMaxQueued(100);

            if(FullName_.empty()) {
            	std::string TmpName = Daemon()->ConfigGetString("openwifi.fileuploader.uri","");
            	if(TmpName.empty()) {
            		FullName_ =
            			"https://" + Svr.Name() + ":" + std::to_string(Svr.Port()) + URI_BASE;
            	} else {
            		FullName_ = TmpName + URI_BASE ;
            	}
            	Logger_.information(Poco::format("Uploader URI base is '%s'", FullName_));
            }

            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new FileUpLoaderRequestHandlerFactory(Logger_), Pool_, Sock, Params);
            NewServer->start();
            Servers_.push_back(std::move(NewServer));
        }

        MaxSize_ = 1000 * Daemon()->ConfigGetInt("openwifi.fileuploader.maxsize", 10000);

        return 0;
    }

    const std::string & FileUploader::FullName() {
        return FullName_;
    }

    //  if you pass in an empty UUID, it will just clean the list and not add it.
    bool FileUploader::AddUUID( const std::string & UUID) {
		SubMutexGuard		Guard(Mutex_);

        uint64_t Now = time(nullptr) ;

        // remove old stuff...
        for(auto i=OutStandingUploads_.cbegin();i!=OutStandingUploads_.end();) {
            if ((Now-i->second) > (60 * 30))
                OutStandingUploads_.erase(i++);
            else
                ++i;
        }

        if(!UUID.empty())
            OutStandingUploads_[UUID] = Now;

        return true;
    }

    bool FileUploader::ValidRequest(const std::string &UUID) {
		SubMutexGuard		Guard(Mutex_);

        return OutStandingUploads_.find(UUID)!=OutStandingUploads_.end();
    }

    void FileUploader::RemoveRequest(const std::string &UUID) {
		SubMutexGuard		Guard(Mutex_);
        OutStandingUploads_.erase(UUID);
    }

    class FileUploaderPartHandler: public Poco::Net::PartHandler
    {
    public:
		FileUploaderPartHandler(std::string UUID, Poco::Logger & Logger):
            UUID_(std::move(UUID)),
            Logger_(Logger)
        {
        }

        void handlePart(const Poco::Net::MessageHeader& Header, std::istream& Stream) override
        {
			try {
				Name_ = "(unnamed)";
				if (Header.has("Content-Disposition")) {
					std::string Disposition;
					Poco::Net::NameValueCollection Parameters;
					Poco::Net::MessageHeader::splitParameters(Header["Content-Disposition"],
															  Disposition, Parameters);
					Name_ = Parameters.get("filename", "(unnamed)");
				}

				std::string FinalFileName = FileUploader()->Path() + "/" + UUID_;

				Logger_.information(Poco::format("FILE-UPLOADER: uploading trace for %s", FinalFileName));
				Poco::CountingInputStream InputStream(Stream);
				std::ofstream OutputStream(FinalFileName, std::ofstream::out);
				Poco::StreamCopier::copyStream(InputStream, OutputStream);

				Poco::File TmpFile(FinalFileName);
				Length_ = TmpFile.getSize();
				if (Length_ < FileUploader()->MaxSize()) {
					Good_=true;
				} else {
					TmpFile.remove();
					Error_ = "File is too large.";
				}
				return;
			} catch (const Poco::Exception &E ) {
				Logger_.log(E);
				Error_ = std::string("Upload caused an internal error: ") + E.what() ;
			}
		}

        [[nodiscard]] uint64_t Length() const { return Length_; }
        [[nodiscard]] const std::string& Name() const { return Name_; }
		[[nodiscard]] bool Good() const { return Good_; }
		std::string & Error() { return Error_; }

    private:
        uint64_t        Length_=0;
		bool 			Good_=false;
        std::string     Name_;
        std::string     UUID_;
		std::string 	Error_;
        Poco::Logger    & Logger_;
    };


    class FormRequestHandler: public Poco::Net::HTTPRequestHandler
    {
    public:
        explicit FormRequestHandler(std::string UUID, Poco::Logger & L):
            UUID_(std::move(UUID)),
            Logger_(L)
        {
        }

        void handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) override
        {
            try {
				FileUploaderPartHandler partHandler(UUID_,Logger_);

                Poco::Net::HTMLForm form(Request, Request.stream(), partHandler);

				Response.setChunkedTransferEncoding(true);
                Response.setContentType("application/json");

				Poco::JSON::Object	Answer;
                if (partHandler.Good()) {
					Answer.set("filename", UUID_);
					Answer.set("error", 0);
					Storage()->AttachFileToCommand(UUID_);
				} else {
					Answer.set("filename", UUID_);
					Answer.set("error", 13);
					Answer.set("errorText", partHandler.Error() );
					Storage()->CancelWaitFile(UUID_, partHandler.Error() );
				}
				std::ostream &ResponseStream = Response.send();
				Poco::JSON::Stringifier::stringify(Answer, ResponseStream);
				return;
            }
            catch( const Poco::Exception & E )
            {
                Logger_.warning(Poco::format("Error occurred while performing upload. Error='%s'",E.displayText()));
            }
            catch( ... )
            {
            }
        }
    private:
        std::string     UUID_;
        Poco::Logger    & Logger_;
    };

    Poco::Net::HTTPRequestHandler *FileUpLoaderRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {

		Logger_.debug(Poco::format("REQUEST(%s): %s %s", Utils::FormatIPv6(Request.clientAddress().toString()), Request.getMethod(), Request.getURI()));

        //  The UUID should be after the /v1/upload/ part...
        auto UUIDLocation = Request.getURI().find_first_of(URI_BASE);

        if( UUIDLocation != std::string::npos )
        {
            auto UUID = Request.getURI().substr(UUIDLocation+URI_BASE.size());
            if(FileUploader()->ValidRequest(UUID))
            {
                //  make sure we do not allow anyone else to overwrite our file
				FileUploader()->RemoveRequest(UUID);
                return new FormRequestHandler(UUID,Logger_);
            }
            else
            {
                Logger_.warning(Poco::format("Unknown UUID=%s",UUID));
            }
        }
        return nullptr;
    }

    void FileUploader::Stop() {
        Logger_.notice("Stopping ");
        for( const auto & svr : Servers_ )
            svr->stop();
    }

}  //  Namespace