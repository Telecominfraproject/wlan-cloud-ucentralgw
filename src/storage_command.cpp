//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <fstream>

#include "Poco/File.h"
#include "Poco/Data/LOBStream.h"
#include "Poco/Data/RecordSet.h"

#include "Daemon.h"
#include "DeviceRegistry.h"
#include "StorageService.h"
#include "FileUploader.h"

namespace uCentral {

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

	bool Storage::AddCommand(std::string &SerialNumber, GWObjects::CommandDetails &Command, CommandExecutionType Type) {
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

			if(Type == COMMAND_PENDING) {
				Command.Status = "pending";
			} else if(Type == COMMAND_COMPLETED) {
				Command.Status = "completed";
			} else {
				Command.Executed = Now;
				Command.Status = "executing";
			}

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Insert(Sess);

			std::string St{"INSERT INTO CommandList (UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, "
						   "Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate, AttachSize, AttachType) "
						   "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"};

			Insert << ConvertParams(St), Poco::Data::Keywords::use(Command.UUID),
				Poco::Data::Keywords::use(Command.SerialNumber),
				Poco::Data::Keywords::use(Command.Command),
				Poco::Data::Keywords::use(Command.Status),
				Poco::Data::Keywords::use(Command.SubmittedBy),
				Poco::Data::Keywords::use(Command.Results),
				Poco::Data::Keywords::use(Command.Details),
				Poco::Data::Keywords::use(Command.Submitted),
				Poco::Data::Keywords::use(Command.Executed),
				Poco::Data::Keywords::use(Command.Completed),
				Poco::Data::Keywords::use(Command.RunAt),
				Poco::Data::Keywords::use(Command.ErrorCode),
				Poco::Data::Keywords::use(Command.Custom),
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

	bool Storage::GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate,
							  uint64_t Offset, uint64_t HowMany,
							  std::vector<GWObjects::CommandDetails> &Commands) {

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
				"AttachSize, AttachType  FROM CommandList ORDER BY UUID "};
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

			Select << IntroStatement + DateSelector +
						  ComputeRange(Offset, HowMany), Poco::Data::Keywords::into(Records);

			Select.execute();

			for (auto i : Records) {
				GWObjects::CommandDetails R{.UUID = i.get<0>(),
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

	bool Storage::DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
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

	typedef std::vector<CommandDetailsRecordTuple> RecordList;

	bool Storage::GetNonExecutedCommands(uint64_t Offset, uint64_t HowMany,
										 std::vector<GWObjects::CommandDetails> &Commands) {
	//	typedef std::vector<CommandDetailsRecordTuple> RecordList;
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
				std::string st{	"SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText,"
								   	"Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate,"
								   	"AttachSize, AttachType FROM CommandList ORDER BY UUID "
								   	"WHERE Executed=0" };
				Select << 	ConvertParams(st) + ComputeRange(Offset, HowMany),
							Poco::Data::Keywords::into(Records);
				Select.execute();

				for (auto i : Records) {
					Offset++;
					GWObjects::CommandDetails R{.UUID = i.get<0>(),
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
					if (DeviceRegistry()->Connected(R.SerialNumber))
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

	bool Storage::UpdateCommand(std::string &UUID, GWObjects::CommandDetails &Command) {

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

	bool Storage::SetCommandExecuted(std::string &CommandUUID) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Update(Sess);

			uint64_t Now = time(nullptr);
			std::string St{"UPDATE CommandList SET Executed=? WHERE UUID=?"};

			Update << ConvertParams(St),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use( CommandUUID);
			Update.execute();
			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), CommandUUID, E.displayText()));
		}
		return false;
	}

	bool Storage::GetCommand(std::string &UUID, GWObjects::CommandDetails &Command) {

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

	bool Storage::DeleteCommand(std::string &UUID) {
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

	bool Storage::GetNewestCommands(std::string &SerialNumber, uint64_t HowMany, std::vector<GWObjects::CommandDetails> &Commands) {
		try {
			RecordList Records;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			// select uuid, submitted from commandlist  where serialnumber='24f5a207a130' order by submitted desc

			std::string st{"SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText,"
						   "Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate,"
						   "AttachSize, AttachType FROM CommandList "
						   "WHERE SerialNumber=? ORDER BY Submitted DESC"};

			Select << 	ConvertParams(st),
						Poco::Data::Keywords::into(Records),
						Poco::Data::Keywords::use(SerialNumber),
						Poco::Data::Keywords::limit(HowMany);
			Select.execute();

			// std::cout << "Returned " << Records.size() << " records" << std::endl;

			for (auto i : Records) {
				GWObjects::CommandDetails R{
					.UUID = i.get<0>(),
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
				if (DeviceRegistry()->Connected(R.SerialNumber))
					Commands.push_back(R);
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetReadyToExecuteCommands(uint64_t Offset, uint64_t HowMany,
											std::vector<GWObjects::CommandDetails> &Commands) {
		// todo: finish the GetReadyToExecuteCommands call...
		try {
			typedef std::vector<CommandDetailsRecordTuple> RecordList;
			uint64_t Now = time(nullptr);
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string St{
				"SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText, "
				" Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate, AttachSize, AttachType FROM CommandList ORDER BY UUID "
				" WHERE RunAt < ? And Executed=0"};

			RecordList Records;

			Select << ConvertParams(St) + ComputeRange(Offset, HowMany),
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::use(Now);
			Select.execute();

			for (auto i : Records) {
				GWObjects::CommandDetails R{.UUID = i.get<0>(),
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

				if (DeviceRegistry()->Connected(R.SerialNumber))
					Commands.push_back(R);
			}

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.warning(Poco::format("GetReadyToExecuteCommands(): Failed to retrieve the list. %s",
										 E.displayText()));
		}
		return false;
	}

	bool Storage::CommandExecuted(std::string &UUID) {
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

	bool Storage::CommandCompleted(std::string &UUID, const Poco::JSON::Object::Ptr & ReturnVars,
								   bool FullCommand) {
		try {

			uint64_t Now = FullCommand ? time(nullptr) : 0;

			// Parse the result to get the ErrorText and make sure that this is a JSON document
			uint64_t ErrorCode = 0;
			std::string ErrorText, ResultStr;

			if (ReturnVars->has("result")) {
				auto ResultObj = ReturnVars->get("result");
				auto ResultFields = ResultObj.extract<Poco::JSON::Object::Ptr>();
				if (ResultFields->has("status")) {
					auto StatusObj = ResultFields->get("status");
					auto StatusInnerObj = StatusObj.extract<Poco::JSON::Object::Ptr>();
					if (StatusInnerObj->has("error"))
						ErrorCode = StatusInnerObj->get("error");
					if (StatusInnerObj->has("text"))
						ErrorText = StatusInnerObj->get("text").toString();

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

	bool Storage::CancelWaitFile( std::string & UUID, std::string & ErrorText ) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			uint64_t Now = time(nullptr);
			uint64_t Size = 0, WaitForFile = 0;

			Poco::Data::Statement Update(Sess);

			std::string St{
				"UPDATE CommandList SET WaitingForFile=?, AttachDate=?, AttachSize=?, ErrorText=?, Completed=?  WHERE UUID=?"};

			Update << ConvertParams(St),
				Poco::Data::Keywords::use(WaitForFile),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Size),
				Poco::Data::Keywords::use(ErrorText),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(UUID);
			Update.execute();
			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;

	}

	bool Storage::AttachFileToCommand(std::string &UUID) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			uint64_t Now = time(nullptr);
			uint64_t WaitForFile = 0;

			Poco::Data::Statement Update(Sess);

			Poco::File FileName = FileUploader()->Path() + "/" + UUID;
			uint64_t Size = FileName.getSize();

			std::string St{
				"UPDATE CommandList SET WaitingForFile=?, AttachDate=?, AttachSize=? WHERE UUID=?"};

			Update << ConvertParams(St),
				Poco::Data::Keywords::use(WaitForFile),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Size),
				Poco::Data::Keywords::use(UUID);
			Update.execute();

			Poco::Data::LOB<char> L;
			Poco::Data::LOBOutputStream OL(L);

			if (FileName.getSize() <
				(1000 * Daemon()->ConfigGetInt("ucentral.fileuploader.maxsize", 10000))) {

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
					Poco::Data::Keywords::use(FileType),
					Poco::Data::Keywords::use(Now),
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

	bool Storage::GetAttachedFile(std::string &UUID, std::string & SerialNumber, const std::string &FileName, std::string &Type) {
		try {
			Poco::Data::LOB<char> L;
			/*
						"UUID			VARCHAR(64) PRIMARY KEY, "
						"Type			VARCHAR(32), "
						"Created 		BIGINT, "
						"FileContent	BYTEA"
			*/
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select1(Sess);

			std::string TmpSerialNumber;
			std::string st1{"SELECT SerialNumber FROM CommandList WHERE UUID=?"};
			Select1	<< 	ConvertParams(st1),
						Poco::Data::Keywords::into(TmpSerialNumber),
						Poco::Data::Keywords::use(UUID);
			Select1.execute();

			if(TmpSerialNumber!=SerialNumber) {
				return false;
			}

			std::string St2{"SELECT FileContent, Type FROM FileUploads WHERE UUID=?"};

			Poco::Data::Statement Select2(Sess);
			Select2 << ConvertParams(St2),
				Poco::Data::Keywords::into(L),
				Poco::Data::Keywords::into(Type),
				Poco::Data::Keywords::use(UUID);
			Select2.execute();

			Poco::Data::LOBInputStream IL(L);
			std::ofstream f(FileName, std::ios::binary);
			Poco::StreamCopier::copyStream(IL, f);

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::SetCommandResult(std::string &UUID, std::string &Result) {
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

	bool Storage::RemoveAttachedFile(std::string &UUID) {
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

	bool Storage::RemoveCommandListRecordsOlderThan(uint64_t Date) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			std::string St1{"delete from CommandList where Submitted<?"};
			Delete << ConvertParams(St1), Poco::Data::Keywords::use(Date);
			Delete.execute();
			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::AnalyzeCommands(Types::CountedMap &R) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Select(Sess);

			Select << "SELECT Command from CommandList";
			Select.execute();

			Poco::Data::RecordSet   RSet(Select);

			bool More = RSet.moveFirst();
			while(More) {
				auto Command = RSet[0].convert<std::string>();
				if(!Command.empty())
					Types::UpdateCountedMap(R,Command);
				More = RSet.moveNext();
			}
			return true;
		} catch(const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

}