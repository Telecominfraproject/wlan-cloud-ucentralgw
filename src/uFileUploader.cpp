//
// Created by stephane bourque on 2021-03-29.
//
#include <iostream>
#include <fstream>
#include <cstdio>

#include "uFileUploader.h"
#include "uCentral.h"
#include "uStorageService.h"

#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/DynamicAny.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/CountingStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"

#include "utils.h"

namespace uCentral::uFileUploader {
    Service *Service::instance_ = nullptr;

    int Start() {
        return uCentral::uFileUploader::Service::instance()->Start();
    }

    void Stop() {
        uCentral::uFileUploader::Service::instance()->Stop();
    }

    const std::string & FullName() {
        return uCentral::uFileUploader::Service::instance()->FullName();
    }

    bool AddUUID( const std::string & UUID) {
        return uCentral::uFileUploader::Service::instance()->AddUUID(UUID);
    }

    bool ValidRequest(const std::string & UUID) {
        return uCentral::uFileUploader::Service::instance()->ValidRequest(UUID);
    }

    void RemoveRequest(const std::string & UUID) {
        uCentral::uFileUploader::Service::instance()->RemoveRequest(UUID);
    }

    const std::string & Path() {
        return uCentral::uFileUploader::Service::instance()->Path();
    }


    Service::Service() noexcept:
            SubSystemServer("FileUploader", "FILE-UPLOAD", "ucentral.fileuploader")
    {
		std::lock_guard<std::mutex>	G(Mutex_);
    }

    static const std::string URI_BASE{"/v1/upload/"};

    int Service::Start() {
        Logger_.notice("Starting.");

        for(const auto & Svr: ConfigServersList_) {
            std::string l{"Starting: " +
                          Svr.address() + ":" + std::to_string(Svr.port()) +
                          " key:" + Svr.key_file() +
                          " cert:" + Svr.cert_file()};

            Logger_.information(l);

            Path_ = uCentral::ServiceConfig::getString("ucentral.fileuploader.path","/tmp");

            auto Sock{Svr.CreateSecureSocket(Logger_)};

			Svr.log_cert(Logger_);
			if(!Svr.root_ca().empty())
				Svr.log_cas(Logger_);

            auto Params = new Poco::Net::HTTPServerParams;
            Params->setMaxThreads(16);
            Params->setMaxQueued(100);

            if(FullName_.empty()) {
                FullName_ = "https://" + Svr.name() + ":" + std::to_string(Svr.port()) + URI_BASE;
                Logger_.information(Poco::format("Uploader URI base is '%s'", FullName_));
            }
            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new RequestHandlerFactory(Logger_), Pool_, Sock, Params);
            NewServer->start();
            Servers_.push_back(std::move(NewServer));
        }

        return 0;
    }

    const std::string & Service::FullName() {
        return FullName_;
    }

    //  if you pass in an empty UUID, it will just clean the list and not add it.
    bool Service::AddUUID( const std::string & UUID) {
        std::lock_guard<std::mutex> guard(Mutex_);

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

    bool Service::ValidRequest(const std::string &UUID) {
        std::lock_guard<std::mutex> guard(Mutex_);

        return OutStandingUploads_.find(UUID)!=OutStandingUploads_.end();
    }

    void Service::RemoveRequest(const std::string &UUID) {
        std::lock_guard<std::mutex> guard(Mutex_);
        OutStandingUploads_.erase(UUID);
    }

    class MyPartHandler: public Poco::Net::PartHandler
    {
    public:
        MyPartHandler(std::string UUID, Poco::Logger & Logger):
            UUID_(std::move(UUID)),
            Logger_(Logger)
        {
        }

        void handlePart(const Poco::Net::MessageHeader& Header, std::istream& Stream) override
        {
            FileType_ = Header.get("Content-Type", "(unspecified)");
            if (Header.has("Content-Disposition"))
            {
                std::string Disposition;
                Poco::Net::NameValueCollection Parameters;
                Poco::Net::MessageHeader::splitParameters(Header["Content-Disposition"], Disposition, Parameters);
                Name_ = Parameters.get("name", "(unnamed)");
            }

            Poco::CountingInputStream InputStream(Stream);
            std::string TmpFileName = uCentral::uFileUploader::Path() + "/" + UUID_ + ".upload.start" ;
            std::string FinalFileName = uCentral::uFileUploader::Path() + "/" + UUID_ ;

            Logger_.information(Poco::format("FILE-UPLOADER: uploading %s",TmpFileName));

            std::ofstream OutputStream(TmpFileName, std::ofstream::out);
            Poco::StreamCopier::copyStream(InputStream, OutputStream);
            Length_ = InputStream.chars();
            rename(TmpFileName.c_str(),FinalFileName.c_str());
        }

        [[nodiscard]] uint64_t Length() const { return Length_; }
        [[nodiscard]] const std::string& Name() const { return Name_; }
        [[nodiscard]] const std::string& ContentType() const { return FileType_; }

    private:
        uint64_t        Length_=0;
        std::string     FileType_;
        std::string     Name_;
        std::string     UUID_;
        Poco::Logger    & Logger_;
    };


    class FormRequestHandler: public Poco::Net::HTTPRequestHandler
        /// Return a HTML document with the current date and time.
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
                MyPartHandler partHandler(UUID_,Logger_);

                Poco::Net::HTMLForm form(Request, Request.stream(), partHandler);
                Response.setChunkedTransferEncoding(true);
                Response.setContentType("text/html");
                std::ostream &ResponseStream = Response.send();

                ResponseStream <<
                     "<html>\n"
                     "<head>\n"
                     "<title>POCO Form Server Sample</title>\n"
                     "</head>\n"
                     "<body>\n"
                     "<h1>POCO Form Server Sample</h1>\n"
                     "<h2>GET Form</h2>\n"
                     "<form method=\"GET\" action=\"/form\">\n"
                     "<input type=\"text\" name=\"text\" size=\"31\">\n"
                     "<input type=\"submit\" value=\"GET\">\n"
                     "</form>\n"
                     "<h2>POST Form</h2>\n"
                     "<form method=\"POST\" action=\"/form\">\n"
                     "<input type=\"text\" name=\"text\" size=\"31\">\n"
                     "<input type=\"submit\" value=\"POST\">\n"
                     "</form>\n"
                     "<h2>File Upload</h2>\n"
                     "<form method=\"POST\" action=\"/form\" enctype=\"multipart/form-data\">\n"
                     "<input type=\"file\" name=\"file\" size=\"31\"> \n"
                     "<input type=\"submit\" value=\"Upload\">\n"
                     "</form>\n";

                ResponseStream << "<h2>Request</h2><p>\n";
                ResponseStream << "Method: " << Request.getMethod() << "<br>\n";
                ResponseStream << "URI: " << Request.getURI() << "<br>\n";
                for (auto & i:Request) {
                    ResponseStream << i.first << ": " << i.second << "<br>\n";
                }

                ResponseStream << "</p>";

                if (!form.empty()) {
                    ResponseStream << "<h2>Form</h2><p>\n";
                    for (const auto & i:form)
                        ResponseStream << i.first << ": " << i.second << "<br>\n";
                    ResponseStream << "</p>";
                }

                if (!partHandler.Name().empty()) {
                    ResponseStream << "<h2>Upload</h2><p>\n";
                    ResponseStream << "Name: " << partHandler.Name() << "<br>\n";
                    ResponseStream << "Type: " << partHandler.ContentType() << "<br>\n";
                    ResponseStream << "Size: " << partHandler.Length() << "<br>\n";
                    ResponseStream << "</p>";
                }
                ResponseStream << "</body>\n";

                uCentral::Storage::AttachFileToCommand(UUID_);
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

    Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {

		Logger_.information(Poco::format("REQUEST(%s): %s %s", uCentral::Utils::FormatIPv6(Request.clientAddress().toString()), Request.getMethod(), Request.getURI()));

        //  The UUID should be after the /v1/upload/ part...
        auto UUIDLocation = Request.getURI().find_first_of(URI_BASE);

        if( UUIDLocation != std::string::npos )
        {
            auto UUID = Request.getURI().substr(UUIDLocation+URI_BASE.size());
            if(uCentral::uFileUploader::ValidRequest(UUID))
            {
                //  make sure we do not allow anyone else to overwrite our file
                uCentral::uFileUploader::RemoveRequest(UUID);
                return new FormRequestHandler(UUID,Logger_);
            }
            else
            {
                Logger_.warning(Poco::format("Unknown UUID=%s",UUID));
            }
        }
        return nullptr;
    }

    void Service::Stop() {
        Logger_.notice("Stopping ");
        for( const auto & svr : Servers_ )
            svr->stop();
    }

}  //  Namespace