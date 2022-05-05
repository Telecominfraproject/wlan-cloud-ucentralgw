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

#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/DynamicAny.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/MultipartReader.h"
#include "Poco/CountingStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"

#include "FileUploader.h"
#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    static const std::string URI_BASE{"/v1/upload/"};

    int FileUploader::Start() {
        Logger().notice("Starting.");

        Poco::File UploadsDir(MicroService::instance().ConfigPath("openwifi.fileuploader.path","/tmp"));
        Path_ = UploadsDir.path();
        if(!UploadsDir.exists()) {
        	try {
        		UploadsDir.createDirectory();
        	} catch (const Poco::Exception &E) {
        		Logger().log(E);
        		Path_ = "/tmp";
        	}
        }

        for(const auto & Svr: ConfigServersList_) {
			if(MicroService::instance().NoAPISecurity()) {
				Logger().information(fmt::format("Starting: {}:{}",Svr.Address(),Svr.Port()));

				auto Sock{Svr.CreateSocket(Logger())};

				auto Params = new Poco::Net::HTTPServerParams;
				Params->setMaxThreads(16);
				Params->setMaxQueued(100);

				if (FullName_.empty()) {
					std::string TmpName =
						MicroService::instance().ConfigGetString("openwifi.fileuploader.uri", "");
					if (TmpName.empty()) {
						FullName_ =
							"https://" + Svr.Name() + ":" + std::to_string(Svr.Port()) + URI_BASE;
					} else {
						FullName_ = TmpName + URI_BASE;
					}
					Logger().information(fmt::format("Uploader URI base is '{}'", FullName_));
				}

				auto NewServer = std::make_unique<Poco::Net::HTTPServer>(
					new FileUpLoaderRequestHandlerFactory(Logger()), Pool_, Sock, Params);
				NewServer->start();
				Servers_.push_back(std::move(NewServer));
			} else {
				std::string l{"Starting: " + Svr.Address() + ":" + std::to_string(Svr.Port()) +
							  " key:" + Svr.KeyFile() + " cert:" + Svr.CertFile()};
				Logger().information(l);

				auto Sock{Svr.CreateSecureSocket(Logger())};

				Svr.LogCert(Logger());
				if (!Svr.RootCA().empty())
					Svr.LogCas(Logger());

				auto Params = new Poco::Net::HTTPServerParams;
				Params->setMaxThreads(16);
				Params->setMaxQueued(100);

				if (FullName_.empty()) {
					std::string TmpName =
						MicroService::instance().ConfigGetString("openwifi.fileuploader.uri", "");
					if (TmpName.empty()) {
						FullName_ =
							"https://" + Svr.Name() + ":" + std::to_string(Svr.Port()) + URI_BASE;
					} else {
						FullName_ = TmpName + URI_BASE;
					}
					Logger().information(fmt::format("Uploader URI base is '{}'", FullName_));
				}

				auto NewServer = std::make_unique<Poco::Net::HTTPServer>(
					new FileUpLoaderRequestHandlerFactory(Logger()), Pool_, Sock, Params);
				NewServer->start();
				Servers_.push_back(std::move(NewServer));
			}
        }

        MaxSize_ = 1000 * MicroService::instance().ConfigGetInt("openwifi.fileuploader.maxsize", 10000);

        return 0;
    }

	void FileUploader::reinitialize([[maybe_unused]] Poco::Util::Application &self) {
		MicroService::instance().LoadConfigurationFile();
    	Logger().information("Reinitializing.");
		Stop();
		Start();
	}

    const std::string & FileUploader::FullName() {
        return FullName_;
    }

    //  if you pass in an empty UUID, it will just clean the list and not add it.
    bool FileUploader::AddUUID( const std::string & UUID) {
		std::lock_guard		Guard(Mutex_);

        uint64_t now = OpenWifi::Now();

        // remove old stuff...
        for(auto i=OutStandingUploads_.begin();i!=OutStandingUploads_.end();) {
            if ((now-i->second) > (60 * 30))
                i = OutStandingUploads_.erase(i);
            else
                ++i;
        }

        if(!UUID.empty())
            OutStandingUploads_[UUID] = now;

        return true;
    }

    bool FileUploader::ValidRequest(const std::string &UUID) {
		std::lock_guard		Guard(Mutex_);

        return OutStandingUploads_.find(UUID)!=OutStandingUploads_.end();
    }

    void FileUploader::RemoveRequest(const std::string &UUID) {
		std::lock_guard		Guard(Mutex_);
        OutStandingUploads_.erase(UUID);
    }

	class FileUploaderPartHandler2 : public Poco::Net::PartHandler {
	  public:
		FileUploaderPartHandler2(std::string Id, Poco::Logger &Logger, std::stringstream & ofs) :
																						  Id_(std::move(Id)),
																						  Logger_(Logger),
																						  OutputStream_(ofs){
		}
		void handlePart(const Poco::Net::MessageHeader &Header, std::istream &Stream) {
			FileType_ = Header.get(RESTAPI::Protocol::CONTENTTYPE, RESTAPI::Protocol::UNSPECIFIED);
			if (Header.has(RESTAPI::Protocol::CONTENTDISPOSITION)) {
				std::string Disposition;
				Poco::Net::NameValueCollection Parameters;
				Poco::Net::MessageHeader::splitParameters(Header[RESTAPI::Protocol::CONTENTDISPOSITION], Disposition, Parameters);
				Name_ = Parameters.get(RESTAPI::Protocol::NAME, RESTAPI::Protocol::UNNAMED);
			}
			Poco::CountingInputStream InputStream(Stream);
			Poco::StreamCopier::copyStream(InputStream, OutputStream_);
			Length_ = OutputStream_.str().size();
		}
		[[nodiscard]] uint64_t Length() const { return Length_; }
		[[nodiscard]] std::string &Name() { return Name_; }
		[[nodiscard]] std::string &ContentType() { return FileType_; }

	  private:
		uint64_t        Length_ = 0;
		std::string     FileType_;
		std::string     Name_;
		std::string     Id_;
		Poco::Logger    &Logger_;
		std::stringstream &OutputStream_;

		inline Poco::Logger & Logger() { return Logger_; };
	};

    class FormRequestHandler: public Poco::Net::HTTPRequestHandler
    {
    public:
        explicit FormRequestHandler(std::string UUID, Poco::Logger & L):
            UUID_(std::move(UUID)),
            Logger_(L)
        {
        }

        void handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) final {

			const auto & ContentType = Request.getContentType();
			const auto & Tokens = Poco::StringTokenizer(ContentType,";",Poco::StringTokenizer::TOK_TRIM);

			Logger().debug(fmt::format("{}: Preparing to upload trace file.",UUID_));
			Poco::JSON::Object Answer;

			try {
				if (Poco::icompare(Tokens[0], "multipart/form-data") == 0 ||
					Poco::icompare(Tokens[0], "multipart/mixed") == 0) {

					const auto &BoundaryTokens =
						Poco::StringTokenizer(Tokens[1], "=", Poco::StringTokenizer::TOK_TRIM);

					if (BoundaryTokens[0] == "boundary") {
						const std::string &Boundary = BoundaryTokens[1];
						Poco::Net::MultipartReader Reader(Request.stream(), Boundary);
						bool Done = false;

						while (!Done) {
							Poco::Net::MessageHeader Hdr;
							Reader.nextPart(Hdr);

							const auto &PartContentType = Hdr.get("Content-Type", "");
							if (PartContentType == "application/octet-stream") {
								std::stringstream FileContent;
								Poco::StreamCopier::copyStream(Reader.stream(), FileContent);
								Answer.set("filename", UUID_);
								Answer.set("error", 0);
								Logger().debug(fmt::format("{}: Trace file uploaded.", UUID_));
								StorageService()->AttachFileDataToCommand(UUID_, FileContent);
								std::ostream &ResponseStream = Response.send();
								Poco::JSON::Stringifier::stringify(Answer, ResponseStream);
								return;
							} else {
								std::stringstream OO;
								Poco::StreamCopier::copyStream(Reader.stream(), OO);
							}

							if (!Reader.hasNextPart())
								Done = true;
						}
					}
				}
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			} catch (...) {
				Logger().debug("Exception while receiving trace file.");
			}

			Logger().debug(fmt::format("{}: Failed to upload trace file.",UUID_));
			std::string Error{"Trace file rejected"};
			StorageService()->CancelWaitFile(UUID_, Error);
			Answer.set("filename", UUID_);
			Answer.set("error", 13);
			Answer.set("errorText", "Attached file is too large");
			StorageService()->CancelWaitFile(UUID_, Error);
			std::ostream &ResponseStream = Response.send();
			Poco::JSON::Stringifier::stringify(Answer, ResponseStream);
		}

		inline Poco::Logger & Logger() { return Logger_; }

    private:
        std::string     UUID_;
        Poco::Logger    & Logger_;
    };

    Poco::Net::HTTPRequestHandler *FileUpLoaderRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {

		Logger().debug(fmt::format("REQUEST({}): {} {}", Utils::FormatIPv6(Request.clientAddress().toString()), Request.getMethod(), Request.getURI()));

        //  The UUID should be after the /v1/upload/ part...
        auto UUIDLocation = Request.getURI().find_first_of(URI_BASE);

        if( UUIDLocation != std::string::npos )
        {
            auto UUID = Request.getURI().substr(UUIDLocation+URI_BASE.size());
            if(FileUploader()->ValidRequest(UUID))
            {
                //  make sure we do not allow anyone else to overwrite our file
				FileUploader()->RemoveRequest(UUID);
                return new FormRequestHandler(UUID,Logger());
            }
            else
            {
                Logger().warning(fmt::format("Unknown UUID={}",UUID));
            }
        }
        return nullptr;
    }

    void FileUploader::Stop() {
        Logger().notice("Stopping ");
        for( const auto & svr : Servers_ )
            svr->stop();
		Servers_.clear();
    }

}  //  Namespace