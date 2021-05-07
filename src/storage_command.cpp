//
// Created by stephane bourque on 2021-05-06.
//

#include <fstream>

#include "Poco/File.h"
#include "Poco/Data/LOBStream.h"

#include "uCentral.h"
#include "uStorageService.h"
#include "uDeviceRegistry.h"


namespace uCentral::Storage {

	typedef Poco::Tuple<
		std::string,
		std::string,
		std::string,
		std::string,
		std::string,
		std::string,
		std::string,
		std::string,
		uint64_t,
		uint64_t,
		uint64_t,
		uint64_t,
		uint64_t,
		uint64_t,
		uint64_t,
		uint64_t,
		uint64_t,
		std::string> CommandDetailsRecordTuple;

	bool AddCommand(std::string &SerialNumber, uCentral::Objects::CommandDetails &Command, bool AlreadyExecuted) {
		return uCentral::Storage::Service::instance()->AddCommand(SerialNumber, Command, AlreadyExecuted);
	}

	bool GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
					 std::vector<uCentral::Objects::CommandDetails> &Commands) {
		return uCentral::Storage::Service::instance()->GetCommands(SerialNumber, FromDate, ToDate, Offset, HowMany,
																   Commands);
	}

	bool DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
		return uCentral::Storage::Service::instance()->DeleteCommands(SerialNumber, FromDate, ToDate);
	}

	bool GetNonExecutedCommands(uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> &Commands) {
		return uCentral::Storage::Service::instance()->GetNonExecutedCommands(Offset, HowMany, Commands);
	}

	bool UpdateCommand(std::string &UUID, uCentral::Objects::CommandDetails &Command) {
		return uCentral::Storage::Service::instance()->UpdateCommand(UUID, Command);
	}

	bool GetCommand(std::string &UUID, uCentral::Objects::CommandDetails &Command) {
		return uCentral::Storage::Service::instance()->GetCommand(UUID, Command);
	}

	bool DeleteCommand(std::string &UUID) {
		return uCentral::Storage::Service::instance()->DeleteCommand(UUID);
	}

	bool GetReadyToExecuteCommands(uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> &Commands) {
		return uCentral::Storage::Service::instance()->GetReadyToExecuteCommands(Offset, HowMany, Commands);
	}

	bool CommandExecuted(std::string &UUID) {
		return uCentral::Storage::Service::instance()->CommandExecuted(UUID);
	}

	bool CommandCompleted(std::string &UUID, Poco::DynamicStruct & ReturnVars, bool FullCommand) {
		return uCentral::Storage::Service::instance()->CommandCompleted(UUID, ReturnVars, FullCommand);
	}

	bool AttachFileToCommand(std::string &UUID) {
		return uCentral::Storage::Service::instance()->AttachFileToCommand(UUID);
	}

	bool GetAttachedFile(std::string & UUID, const std::string & FileName) {
		return uCentral::Storage::Service::instance()->GetAttachedFile(UUID,FileName);
	}

	bool RemoveAttachedFile(std::string & UUID) {
		return uCentral::Storage::Service::instance()->RemoveAttachedFile(UUID);
	}

	bool SetCommandResult(std::string & UUID, std::string & Result) {
		return uCentral::Storage::Service::instance()->SetCommandResult(UUID, Result);
	}

	bool Service::AddCommand(std::string &SerialNumber, uCentral::Objects::CommandDetails &Command,
							 bool AlreadyExecuted) {
		try {
			/*
					"UUID           VARCHAR(30) PRIMARY KEY, "
					"SerialNumber   VARCHAR(30), "
					"Command        VARCHAR(32), "
					"Status         VARCHAR(64), "
					"SubmittedBy    VARCHAR(64), "
					"Results        TEXT, "
					"Details        TEXT, "
					"Submitted      BIGINT, "
					"Executed       BIGINT, "
					"Completed      BIGINT, "
					"RunAt          BIGINT, "
					"ErrorCode      BIGINT, "
					"Custom         BIGINT, "
					"WaitingForFile BIGINT, "
					"AttachDate     BIGINT,"
					"AttachSize     BIGINT,"
					"AttachType     VARCHAR(64)"
			 */

			uint64_t Now = time(nullptr);

			Command.Submitted = Now;
			Command.Completed = 0;
			if (AlreadyExecuted) {
				Command.Executed = Now;
				Command.Status = "executing";
			} else {
				Command.Executed = 0;
				Command.Status = "pending";
			}

			Command.ErrorCode = 0;
			Command.AttachDate = 0;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Insert(Sess);

			std::string St{"INSERT INTO CommandList (UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, "
						   "Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate, AttachSize, AttachType) "
						   "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"};

			Insert << ConvertParams(St), Poco::Data::Keywords::use(Command.UUID),
				Poco::Data::Keywords::use(Command.SerialNumber),
				Poco::Data::Keywords::use(Command.Command), Poco::Data::Keywords::use(Command.Status),
				Poco::Data::Keywords::use(Command.SubmittedBy),
				Poco::Data::Keywords::use(Command.Results), Poco::Data::Keywords::use(Command.Details),
				Poco::Data::Keywords::use(Command.Submitted),
				Poco::Data::Keywords::use(Command.Executed),
				Poco::Data::Keywords::use(Command.Completed), Poco::Data::Keywords::use(Command.RunAt),
				Poco::Data::Keywords::use(Command.ErrorCode), Poco::Data::Keywords::use(Command.Custom),
				Poco::Data::Keywords::use(Command.WaitingForFile),
				Poco::Data::Keywords::use(Command.AttachDate),
				Poco::Data::Keywords::use(Command.AttachSize),
				Poco::Data::Keywords::use(Command.AttachType);

			Insert.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber,
										 E.displayText()));
		}
		return false;
	}

	bool Service::GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate,
							  uint64_t Offset, uint64_t HowMany,
							  std::vector<uCentral::Objects::CommandDetails> &Commands) {
		typedef std::vector<CommandDetailsRecordTuple> RecordList;

		/*
			"UUID           VARCHAR(30) PRIMARY KEY, "
			"SerialNumber   VARCHAR(30), "
			"Command        VARCHAR(32), "
			"Status         VARCHAR(64), "
			"SubmittedBy    VARCHAR(64), "
			"Results        TEXT, "
			"Details        TEXT, "
			"ErrorText      TEXT, "
			"Submitted      BIGINT, "
			"Executed       BIGINT, "
			"Completed      BIGINT, "
			"RunAt          BIGINT, "
			"ErrorCode      BIGINT, "
			"Custom         BIGINT, "
			"WaitingForFile BIGINT, "
			"AttachDate     BIGINT,"
			"AttachSize     BIGINT,"
			"AttachType     VARCHAR(64)"
		 */

		try {
			RecordList Records;
			Poco::Data::Session Sess = Pool_->get();

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);

			std::string Fields{
				"SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText, "
				"Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate, "
				"AttachSize, AttachType  FROM CommandList "};
			std::string IntroStatement = SerialNumber.empty()
											 ? Fields + std::string(DatesIncluded ? "WHERE " : "")
											 : Fields + "WHERE SerialNumber='" + SerialNumber + "'" +
												   std::string(DatesIncluded ? " AND " : "");

			std::string DateSelector;
			if (FromDate && ToDate) {
				DateSelector = " Submitted>=" + std::to_string(FromDate) +
							   " AND Submitted<=" + std::to_string(ToDate);
			} else if (FromDate) {
				DateSelector = " Submitted>=" + std::to_string(FromDate);
			} else if (ToDate) {
				DateSelector = " Submitted<=" + std::to_string(ToDate);
			}

			Poco::Data::Statement Select(Sess);

			Select << IntroStatement + DateSelector, Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::range(Offset, Offset + HowMany);

			Select.execute();

			for (auto i : Records) {
				uCentral::Objects::CommandDetails R{.UUID = i.get<0>(),
										 .SerialNumber = i.get<1>(),
										 .Command = i.get<2>(),
										 .Status = i.get<3>(),
										 .SubmittedBy = i.get<4>(),
										 .Results = i.get<5>(),
										 .Details = i.get<6>(),
										 .ErrorText = i.get<7>(),
										 .Submitted = i.get<8>(),
										 .Executed = i.get<9>(),
										 .Completed = i.get<10>(),
										 .RunAt = i.get<11>(),
										 .ErrorCode = i.get<12>(),
										 .Custom = i.get<13>(),
										 .WaitingForFile = i.get<14>(),
										 .AttachDate = i.get<15>(),
										 .AttachSize = i.get<16>(),
										 .AttachType = i.get<17>()};

				Commands.push_back(R);
			}

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber,
										 E.displayText()));
		}
		return false;
	}

	bool Service::DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);

			std::string IntroStatement =
				SerialNumber.empty()
					? "DELETE FROM CommandList " + std::string(DatesIncluded ? "WHERE " : "")
					: "DELETE FROM CommandList WHERE SerialNumber='" + SerialNumber + "'" +
						  std::string(DatesIncluded ? " AND " : "");

			std::string DateSelector;
			if (FromDate && ToDate) {
				DateSelector = " Submitted>=" + std::to_string(FromDate) +
							   " AND Submitted<=" + std::to_string(ToDate);
			} else if (FromDate) {
				DateSelector = " Submitted>=" + std::to_string(FromDate);
			} else if (ToDate) {
				DateSelector = " Submitted<=" + std::to_string(ToDate);
			}

			Delete << IntroStatement + DateSelector;

			Delete.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber,
										 E.displayText()));
		}
		return false;
	}

	bool Service::GetNonExecutedCommands(uint64_t Offset, uint64_t HowMany,
										 std::vector<uCentral::Objects::CommandDetails> &Commands) {
		typedef std::vector<CommandDetailsRecordTuple> RecordList;
		/*
			"UUID           VARCHAR(30) PRIMARY KEY, "
			"SerialNumber   VARCHAR(30), "
			"Command        VARCHAR(32), "
			"Status         VARCHAR(64), "
			"SubmittedBy    VARCHAR(64), "
			"Results        TEXT, "
			"Details        TEXT, "
			"ErrorText      TEXT, "
			"Submitted      BIGINT, "
			"Executed       BIGINT, "
			"Completed      BIGINT, "
			"RunAt          BIGINT, "
			"ErrorCode      BIGINT, "
			"Custom         BIGINT, "
			"WaitingForFile BIGINT, "
			"AttachDate     BIGINT,"
			"AttachSize     BIGINT,"
			"AttachType     VARCHAR(64)"
		 */

		try {
			RecordList Records;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			bool Done = false;

			while (Commands.size() < HowMany && !Done) {
				// range(Offset, Offset + HowMany - 1)
				Select << "SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText,"
						  "Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate,"
						  "AttachSize, AttachType FROM CommandList "
						  "WHERE Executed=0",
					Poco::Data::Keywords::into(Records),
					Poco::Data::Keywords::range(Offset, Offset + HowMany);
				Select.execute();

				for (auto i : Records) {
					Offset++;
					uCentral::Objects::CommandDetails R{.UUID = i.get<0>(),
											 .SerialNumber = i.get<1>(),
											 .Command = i.get<2>(),
											 .Status = i.get<3>(),
											 .SubmittedBy = i.get<4>(),
											 .Results = i.get<5>(),
											 .Details = i.get<6>(),
											 .ErrorText = i.get<7>(),
											 .Submitted = i.get<8>(),
											 .Executed = i.get<9>(),
											 .Completed = i.get<10>(),
											 .RunAt = i.get<11>(),
											 .ErrorCode = i.get<12>(),
											 .Custom = i.get<13>(),
											 .WaitingForFile = i.get<14>(),
											 .AttachDate = i.get<15>(),
											 .AttachSize = i.get<16>(),
											 .AttachType = i.get<17>()};

					//	Only return rhe commands for our own devices.
					if (uCentral::DeviceRegistry::Connected(R.SerialNumber))
						Commands.push_back(R);
				}

				//	If we could not return enough commands, we are done.
				if (Records.size() < HowMany)
					Done = true;
			}

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Service::UpdateCommand(std::string &UUID, uCentral::Objects::CommandDetails &Command) {

		try {
			Poco::Data::Session Sess = Pool_->get();
			/*
				"UUID           VARCHAR(30) PRIMARY KEY, "
				"SerialNumber   VARCHAR(30), "
				"Command        VARCHAR(32), "
				"Status         VARCHAR(64), "
				"SubmittedBy    VARCHAR(64), "
				"Results        TEXT, "
				"Details        TEXT, "
				"ErrorText      TEXT, "
				"Submitted      BIGINT, "
				"Executed       BIGINT, "
				"Completed      BIGINT, "
				"RunAt          BIGINT, "
				"ErrorCode      BIGINT, "
				"Custom         BIGINT, "
				"WaitingForFile BIGINT, "
				"AttachDate     BIGINT"
			 */

			Poco::Data::Statement Update(Sess);

			std::string St{"UPDATE CommandList SET Status=?,  Executed=?,  Completed=?,  Results=?,  ErrorText=?,  ErrorCode=?  WHERE UUID=?"};

			Update << ConvertParams(St), Poco::Data::Keywords::use(Command.Status),
				Poco::Data::Keywords::use(Command.Executed),
				Poco::Data::Keywords::use(Command.Completed),
				Poco::Data::Keywords::use(Command.Results),
				Poco::Data::Keywords::use(Command.ErrorText),
				Poco::Data::Keywords::use(Command.ErrorCode), Poco::Data::Keywords::use(UUID);

			Update.execute();

			return true;

		} catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), UUID, E.displayText()));
		}
		return false;
	}

	bool Service::GetCommand(std::string &UUID, uCentral::Objects::CommandDetails &Command) {

		try {
			Poco::Data::Session Sess = Pool_->get();
			/*
				"UUID           VARCHAR(30) PRIMARY KEY, "
				"SerialNumber   VARCHAR(30), "
				"Command        VARCHAR(32), "
				"Status         VARCHAR(64), "
				"SubmittedBy    VARCHAR(64), "
				"Results        TEXT, "
				"Details        TEXT, "
				"ErrorText      TEXT, "
				"Submitted      BIGINT, "
				"Executed       BIGINT, "
				"Completed      BIGINT, "
				"RunAt          BIGINT, "
				"ErrorCode      BIGINT, "
				"Custom         BIGINT, "
				"WaitingForFile BIGINT, "
				"AttachDate     BIGINT,"
				"AttachSize     BIGINT,"
				"AttachType     VARCHAR(64)"
			 */
			Poco::Data::Statement Select(Sess);

			std::string St{
				"SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText, "
				"Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate, AttachSize, AttachType  FROM CommandList "
				"WHERE UUID=?"};

			Select << ConvertParams(St), Poco::Data::Keywords::into(Command.UUID),
				Poco::Data::Keywords::into(Command.SerialNumber),
				Poco::Data::Keywords::into(Command.Command), Poco::Data::Keywords::into(Command.Status),
				Poco::Data::Keywords::into(Command.SubmittedBy),
				Poco::Data::Keywords::into(Command.Results),
				Poco::Data::Keywords::into(Command.Details),
				Poco::Data::Keywords::into(Command.ErrorText),
				Poco::Data::Keywords::into(Command.Submitted),
				Poco::Data::Keywords::into(Command.Executed),
				Poco::Data::Keywords::into(Command.Completed),
				Poco::Data::Keywords::into(Command.RunAt),
				Poco::Data::Keywords::into(Command.ErrorCode),
				Poco::Data::Keywords::into(Command.Custom),
				Poco::Data::Keywords::into(Command.WaitingForFile),
				Poco::Data::Keywords::into(Command.AttachDate),
				Poco::Data::Keywords::into(Command.AttachSize),
				Poco::Data::Keywords::into(Command.AttachType), Poco::Data::Keywords::use(UUID);

			Select.execute();

			return true;

		} catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), UUID, E.displayText()));
		}
		return false;
	}

	bool Service::DeleteCommand(std::string &UUID) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			std::string St{"DELETE FROM CommandList WHERE UUID=?"};

			Delete << ConvertParams(St), Poco::Data::Keywords::use(UUID);
			Delete.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), UUID, E.displayText()));
		}
		return false;
	}

	bool Service::GetReadyToExecuteCommands(uint64_t Offset, uint64_t HowMany,
											std::vector<uCentral::Objects::CommandDetails> &Commands) {
		// todo: finish the GetReadyToExecuteCommands call...
		try {
			typedef std::vector<CommandDetailsRecordTuple> RecordList;
			uint64_t Now = time(nullptr);
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string St{
				"SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText, "
				"Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate, AttachSize, AttachType FROM CommandList "
				"WHERE RunAt < ? And Executed=0"};

			RecordList Records;

			Select << ConvertParams(St), Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::use(Now), Poco::Data::Keywords::range(Offset, Offset + HowMany);
			Select.execute();

			for (auto i : Records) {
				uCentral::Objects::CommandDetails R{.UUID = i.get<0>(),
										 .SerialNumber = i.get<1>(),
										 .Command = i.get<2>(),
										 .Status = i.get<3>(),
										 .SubmittedBy = i.get<4>(),
										 .Results = i.get<5>(),
										 .Details = i.get<6>(),
										 .ErrorText = i.get<7>(),
										 .Submitted = i.get<8>(),
										 .Executed = i.get<9>(),
										 .Completed = i.get<10>(),
										 .RunAt = i.get<11>(),
										 .ErrorCode = i.get<12>(),
										 .Custom = i.get<13>(),
										 .WaitingForFile = i.get<14>(),
										 .AttachDate = i.get<15>(),
										 .AttachSize = i.get<16>(),
										 .AttachType = i.get<17>()};

				if (uCentral::DeviceRegistry::Connected(R.SerialNumber))
					Commands.push_back(R);
			}

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("GetReadyToExecuteCommands(): Failed to retrieve the list. %s",
										 E.displayText()));
		}
		return false;
	}

	bool Service::CommandExecuted(std::string &UUID) {
		try {
			uint64_t Now = time(nullptr);

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Update(Sess);

			std::string St{"UPDATE CommandList SET Executed=? WHERE UUID=?"};

			Update << ConvertParams(St), Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(UUID);

			Update.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("Could not update field on command %s", UUID));
		}

		return false;
	}

	bool Service::CommandCompleted(std::string &UUID, Poco::DynamicStruct &ReturnVars,
								   bool FullCommand) {
		try {

			uint64_t Now = FullCommand ? time(nullptr) : 0;

			// Parse the result to get the ErrorText and make sure that this is a JSON document
			uint64_t ErrorCode = 0;
			std::string ErrorText, ResultStr;

			if (ReturnVars.contains("result")) {
				auto ResultObj = ReturnVars["result"];
				Poco::DynamicStruct ResultFields = ResultObj.extract<Poco::DynamicStruct>();

				if (ResultFields.contains("status")) {
					auto StatusObj = ResultFields["status"];
					Poco::DynamicStruct StatusInnerObj = StatusObj.extract<Poco::DynamicStruct>();

					if (StatusInnerObj.contains("error"))
						ErrorCode = StatusInnerObj["error"];
					if (StatusInnerObj.contains("text"))
						ErrorText = StatusInnerObj["text"].toString();

					std::stringstream ResultText;
					Poco::JSON::Stringifier::stringify(ResultObj, ResultText);
					ResultStr = ResultText.str();
				}
			}

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Update(Sess);

			std::string StatusText{"completed"};

			std::string St{"UPDATE CommandList SET Completed=?, ErrorCode=?, ErrorText=?, Results=?, Status=? WHERE UUID=?"};

			Update << ConvertParams(St), Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(ErrorCode), Poco::Data::Keywords::use(ErrorText),
				Poco::Data::Keywords::use(ResultStr), Poco::Data::Keywords::use(StatusText),
				Poco::Data::Keywords::use(UUID);
			Update.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}

		return false;
	}

	bool Service::AttachFileToCommand(std::string &UUID) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			uint64_t Now = time(nullptr);
			uint64_t WaitForFile = 0;

			Poco::Data::Statement Update(Sess);

			Poco::File FileName = uCentral::ServiceConfig::GetString("ucentral.fileuploader.path", "/tmp") + "/" + UUID;
			uint64_t Size = FileName.getSize();

			std::string St{
				"UPDATE CommandList SET WaitingForFile=?, AttachDate=?, AttachSize=? WHERE UUID=?"};

			Update << ConvertParams(St), Poco::Data::Keywords::use(WaitForFile),
				Poco::Data::Keywords::use(Now), Poco::Data::Keywords::use(Size),
				Poco::Data::Keywords::use(UUID);
			Update.execute();

			Poco::Data::LOB<char> L;
			Poco::Data::LOBOutputStream OL(L);

			if (FileName.getSize() <
				(1000 * uCentral::ServiceConfig::GetInt("ucentral.fileuploader.maxsize", 10000))) {

				std::ifstream f(FileName.path(), std::ios::binary);
				Poco::StreamCopier::copyStream(f, OL);
				/*
							"UUID			VARCHAR(64) PRIMARY KEY, "
							"Type			VARCHAR(32), "
							"Created 		BIGINT, "
							"FileContent	BYTEA"
				*/
				Poco::Data::Statement Insert(Sess);
				std::string FileType{"trace"};

				std::string St2{
					"INSERT INTO FileUploads (UUID,Type,Created,FileContent) VALUES(?,?,?,?)"};

				Insert << ConvertParams(St2), Poco::Data::Keywords::use(UUID),
					Poco::Data::Keywords::use(FileType), Poco::Data::Keywords::use(Now),
					Poco::Data::Keywords::use(L);
				Insert.execute();

				FileName.remove();

				return true;
			} else {
				Logger_.warning(Poco::format("File %s is too large.", FileName.path()));
			}
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
			Logger_.warning(Poco::format(
				"Could not update outstanding command %s for file upload completion", UUID));
		}
		return false;
	}

	bool Service::GetAttachedFile(std::string &UUID, const std::string &FileName) {
		try {
			Poco::Data::LOB<char> L;
			/*
						"UUID			VARCHAR(64) PRIMARY KEY, "
						"Type			VARCHAR(32), "
						"Created 		BIGINT, "
						"FileContent	BYTEA"
			*/
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string St2{"SELECT FileContent FROM FileUploads WHERE UUID=?"};

			Select << ConvertParams(St2), Poco::Data::Keywords::into(L),
				Poco::Data::Keywords::use(UUID);
			Select.execute();

			Poco::Data::LOBInputStream IL(L);

			std::ofstream f(FileName, std::ios::binary);
			Poco::StreamCopier::copyStream(IL, f);

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Service::SetCommandResult(std::string &UUID, std::string &Result) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Update(Sess);

			uint64_t Now = time(nullptr);
			std::string St{"UPDATE CommandList SET Completed=?, Results=? WHERE UUID=?"};

			Update << ConvertParams(St), Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Result), Poco::Data::Keywords::use(UUID);
			Update.execute();
			return true;

		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Service::RemoveAttachedFile(std::string &UUID) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			std::string St{"DELETE FROM FileUploads WHERE UUID=?"};

			Delete << ConvertParams(St), Poco::Data::Keywords::use(UUID);
			Delete.execute();

			return true;

		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}

		return false;
	}
}