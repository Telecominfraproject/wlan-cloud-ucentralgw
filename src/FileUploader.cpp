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
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/NetException.h"
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

    class FileUploaderPartHandler: public Poco::Net::PartHandler
    {
    public:
		FileUploaderPartHandler(std::string UUID, Poco::Logger & Logger):
            UUID_(std::move(UUID)),
            Logger_(Logger)
        {
        }

        void handlePart(const Poco::Net::MessageHeader& Header, std::istream& Stream)
        {
			try {
				std::cout << __LINE__ << std::endl;
				Name_ = "(unnamed)";
				std::cout << __LINE__ << std::endl;
				if (Header.has("Content-Disposition")) {
					std::cout << __LINE__ << std::endl;
					std::string Disposition;
					Poco::Net::NameValueCollection Parameters;
					Poco::Net::MessageHeader::splitParameters(Header["Content-Disposition"],
															  Disposition, Parameters);
					FileName_ = Parameters.get("filename", "(unnamed)");
					Name_ = Parameters.get("name", "(unnamed)");
					std::cout << __LINE__ << std::endl;

					std::string FinalFileName = FileUploader()->Path() + "/" + UUID_;
					std::cout << __LINE__ << std::endl;

					Logger().information(
						fmt::format("FILE-UPLOADER: uploading trace for {}", FinalFileName));
					Poco::CountingInputStream InputStream(Stream);
					std::ofstream OutputStream(FinalFileName, std::ofstream::out |
																  std::ofstream::trunc |
																  std::ofstream::binary);
					Poco::StreamCopier::copyStream(InputStream, OutputStream);
					std::cout << __LINE__ << std::endl;

					Length_ = InputStream.chars();
					std::cout << __LINE__ << std::endl;
					if (Length_ < FileUploader()->MaxSize()) {
						Good_ = true;
						std::cout << __LINE__ << std::endl;
					} else {
						std::cout << __LINE__ << std::endl;
						Poco::File TmpFile(FinalFileName);
						TmpFile.remove();
						Error_ = "File is too large.";
					}
					std::cout << __LINE__ << std::endl;
					return;
				}
			}
			catch (const Poco::Exception &E) {
				std::cout << __LINE__ << std::endl;
				Logger().log(E);
				Error_ = std::string("Upload caused an internal error: ") + E.what();
			}
			std::cout << __LINE__ << std::endl;
		}

        [[nodiscard]] uint64_t Length() const { return Length_; }
        [[nodiscard]] const std::string& Name() const { return Name_; }
		[[nodiscard]] bool Good() const { return Good_; }
		std::string & Error() { return Error_; }
		inline Poco::Logger & Logger() { return Logger_; }

    private:
        uint64_t        Length_=0;
		bool 			Good_=false;
        std::string     Name_, FileName_;
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

        void handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) final
        {
            try {
				std::cout << __LINE__ << std::endl;
				std::stringstream	FileContent;
				FileUploaderPartHandler2 partHandler(UUID_, Logger(), FileContent);
				std::cout << __LINE__ << std::endl;

                Poco::Net::HTMLForm form;

				std::cout << __LINE__ << std::endl;
				form.load(Request, Request.stream(), partHandler);
				std::cout << __LINE__ << std::endl;

				Response.setChunkedTransferEncoding(true);
				std::cout << __LINE__ << std::endl;
                Response.setContentType("application/json");
				std::cout << __LINE__ << std::endl;

				Poco::JSON::Object	Answer;
                if (partHandler.Length()<FileUploader()->MaxSize()) {
					std::cout << __LINE__ << std::endl;
					Answer.set("filename", UUID_);
					Answer.set("error", 0);
					std::cout << __LINE__ << std::endl;
					// StorageService()->AttachFileToCommand(UUID_);
					StorageService()->AttachFileDataToCommand(UUID_, FileContent);
					std::cout << __LINE__ << std::endl;
				} else {
					std::cout << __LINE__ << std::endl;
					Answer.set("filename", UUID_);
					Answer.set("error", 13);
					Answer.set("errorText", "Attached file is too large" );
					std::string	Error{"Attached file is too large"};
					StorageService()->CancelWaitFile(UUID_, Error );
					std::cout << __LINE__ << std::endl;
				}
				std::cout << __LINE__ << std::endl;
				std::ostream &ResponseStream = Response.send();
				std::cout << __LINE__ << std::endl;
				Poco::JSON::Stringifier::stringify(Answer, ResponseStream);
				std::cout << __LINE__ << std::endl;
				return;
            }
			catch (const Poco::IOException &E) {
				std::cout << __LINE__ << E.displayText() << "   "  << E.what() << " " << E.name() << " " << E.code() << std::endl;
			}
			catch (const Poco::Net::MultipartException &E ) {
				std::cout << __LINE__ << E.displayText() << "   "  << E.what() << std::endl;
				Logger().warning(fmt::format("Form Error occurred while performing upload. Error='{}' What='{}'",E.displayText(),E.what()));
			}
			catch ( const Poco::Net::HTMLFormException & E) {
				std::cout << __LINE__ << E.displayText() << "   "  << E.what() << std::endl;
				Logger().warning(fmt::format("Form Error occurred while performing upload. Error='{}' What='{}'",E.displayText(),E.what()));
			}
            catch( const Poco::Exception & E )
            {
				std::cout << __LINE__ << E.displayText() << "   "  << E.what() << std::endl;
                Logger().warning(fmt::format("Error occurred while performing upload. Error='{}'",E.displayText()));
            }
            catch( ... )
            {
				std::cout << __LINE__ << std::endl;
            }
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