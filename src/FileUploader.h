//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>

#include <framework/SubSystemServer.h>

namespace OpenWifi {

	class FileUploader : public SubSystemServer {
	  public:
		struct UploadId {
			std::string UUID;
			std::uint64_t Expires;
			std::string Type;
		};

		int Start() override;
		void Stop() override;
		void reinitialize(Poco::Util::Application &self) override;
		const std::string &FullName();
		bool AddUUID(const std::string &UUID, std::chrono::seconds WaitTimeInSecond,
					 const std::string &Type);
		bool ValidRequest(const std::string &UUID);
		void RemoveRequest(const std::string &UUID);
		const std::string &Path() { return Path_; };

		static auto instance() {
			static auto instance_ = new FileUploader;
			return instance_;
		}

		[[nodiscard]] inline uint64_t MaxSize() const { return MaxSize_; }

		bool Find(const std::string &UUID, UploadId &V);

	  private:
		std::vector<std::unique_ptr<Poco::Net::HTTPServer>> Servers_;
		std::string FullName_;
		std::list<UploadId> OutStandingUploads_;
		std::string Path_;
		uint64_t MaxSize_ = 10000000;

		explicit FileUploader() noexcept
			: SubSystemServer("FileUploader", "FILE-UPLOAD", "openwifi.fileuploader") {}
	};

	class FileUpLoaderRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	  public:
		explicit FileUpLoaderRequestHandlerFactory(Poco::Logger &L) : Logger_(L) {}

		Poco::Net::HTTPRequestHandler *
		createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
		inline Poco::Logger &Logger() { return Logger_; }

	  private:
		Poco::Logger &Logger_;
	};

	inline auto FileUploader() { return FileUploader::instance(); }
} // namespace OpenWifi
