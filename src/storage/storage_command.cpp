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

namespace OpenWifi {

const static std::string	DB_Command_SelectFields{
				"UUID, "
				"SerialNumber, "
				"Command, "
				"Status, "
				"SubmittedBy, "
				"Results, "
				"Details, "
				"ErrorText, "
				"Submitted, "
				"Executed, "
				"Completed, "
				"RunAt, "
				"ErrorCode, "
				"Custom, "
				"WaitingForFile, "
				"AttachDate, "
				"AttachSize, "
				"AttachType,"
				"executionTime " };

const static std::string 	DB_Command_InsertValues{"?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?"};

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
			std::string,
			double
		> CommandDetailsRecordTuple;
	typedef std::vector<CommandDetailsRecordTuple> CommandDetailsRecordList;

	void ConvertCommandRecord(const CommandDetailsRecordTuple &R, GWObjects::CommandDetails & Command) {
		Command.UUID = R.get<0>();
		Command.SerialNumber = R.get<1>();
		Command.Command = R.get<2>();
		Command.Status = R.get<3>();
		Command.SubmittedBy = R.get<4>();
		Command.Results = R.get<5>();
		Command.Details = R.get<6>();
		Command.ErrorText = R.get<7>();
		Command.Submitted = R.get<8>();
		Command.Executed = R.get<9>();
		Command.Completed = R.get<10>();
		Command.RunAt = R.get<11>();
		Command.ErrorCode = R.get<12>();
		Command.Custom = R.get<13>();
		Command.WaitingForFile = R.get<14>();
		Command.AttachDate = R.get<15>();
		Command.AttachSize = R.get<16>();
		Command.AttachType = R.get<17>();
		Command.executionTime = R.get<18>();
	}

	void ConvertCommandRecord(const GWObjects::CommandDetails & Command, CommandDetailsRecordTuple &R) {
		R.set<0>(Command.UUID);
		R.set<1>(Command.SerialNumber);
		R.set<2>(Command.Command);
		R.set<3>(Command.Status);
		R.set<4>(Command.SubmittedBy);
		R.set<5>(Command.Results);
		R.set<6>(Command.Details);
		R.set<7>(Command.ErrorText);
		R.set<8>(Command.Submitted);
		R.set<9>(Command.Executed);
		R.set<10>(Command.Completed);
		R.set<11>(Command.RunAt);
		R.set<12>(Command.ErrorCode);
		R.set<13>(Command.Custom);
		R.set<14>(Command.WaitingForFile);
		R.set<15>(Command.AttachDate);
		R.set<16>(Command.AttachSize);
		R.set<17>(Command.AttachType);
		R.set<18>(Command.executionTime);
	}

	bool Storage::RemoveOldCommands(std::string & SerialNumber, std::string & Command) {
		try {
			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement 	Delete(Sess);

			std::string St{"delete from CommandList where SerialNumber=? and command=? and completed=0"};
			Delete << ConvertParams(St),
				Poco::Data::Keywords::use(SerialNumber),
				Poco::Data::Keywords::use(Command);
			Delete.execute();
			return true;
		} catch(const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::AddCommand(std::string &SerialNumber, GWObjects::CommandDetails &Command, CommandExecutionType Type) {
		try {
			uint64_t Now = time(nullptr);

			if(Type == COMMAND_PENDING) {
				Command.Status = "pending";
			} else if(Type == COMMAND_COMPLETED) {
				Command.Status = "completed";
				Command.Executed = Now;
			} else if (Type == COMMAND_TIMEDOUT) {
				Command.Executed = Now;
				Command.Status = "timedout";
			} else if (Type == COMMAND_FAILED) {
				Command.Executed = Now;
				Command.Status = "failed";
			} else {
				Command.Executed = Now;
				Command.Status = "executing";
			}

			RemoveOldCommands(SerialNumber, Command.Command);

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Insert(Sess);

			std::string St{"INSERT INTO CommandList ( " +
						   DB_Command_SelectFields + " ) VALUES( " +
						   DB_Command_InsertValues + " )"};

			CommandDetailsRecordTuple R;
			ConvertCommandRecord(Command, R);

			Insert << ConvertParams(St),
				Poco::Data::Keywords::use(R);
			Insert.execute();

			return true;

		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate,
							  uint64_t Offset, uint64_t HowMany,
							  std::vector<GWObjects::CommandDetails> &Commands) {
		try {
			CommandDetailsRecordList Records;
			Poco::Data::Session Sess = Pool_->get();

			bool DatesIncluded = (FromDate != 0 || ToDate != 0);

			std::string Fields{
				"SELECT " + DB_Command_SelectFields + " FROM CommandList " };

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

			std::string FullQuery = IntroStatement + DateSelector +
					" ORDER BY Submitted ASC " + ComputeRange(Offset, HowMany);

			// std::cout << "Offset: " << Offset << "  >>  " << FullQuery << std::endl;

			Select << 	FullQuery,
				Poco::Data::Keywords::into(Records);
			Select.execute();
			for (const auto &i : Records) {
				GWObjects::CommandDetails R;
				ConvertCommandRecord(i, R);
				Commands.push_back(R);
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
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
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetNonExecutedCommands(uint64_t Offset, uint64_t HowMany,
										 std::vector<GWObjects::CommandDetails> &Commands) {
		try {
			CommandDetailsRecordList Records;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);
			bool Done = false;

			while (Commands.size() < HowMany && !Done) {
				std::string st{	"SELECT " +
								   DB_Command_SelectFields +
								   " FROM CommandList ORDER BY UUID ASC WHERE Executed=0" };
				Select << 	ConvertParams(st) + ComputeRange(Offset, HowMany),
							Poco::Data::Keywords::into(Records);
				Select.execute();

				for (const auto &i : Records) {
					Offset++;
					GWObjects::CommandDetails R;
					ConvertCommandRecord(i,R);
					if (DeviceRegistry()->Connected(R.SerialNumber))
						Commands.push_back(R);
				}

				//	If we could not return enough commands, we are done.
				if (Records.size() < HowMany)
					Done = true;
			}

			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return false;
	}

	bool Storage::UpdateCommand(std::string &UUID, GWObjects::CommandDetails &Command) {

		try {
			Poco::Data::Session Sess = Pool_->get();
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
			Logger().log(E);
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
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetCommand(std::string &UUID, GWObjects::CommandDetails &Command) {

		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string St{
				"SELECT " +
				DB_Command_SelectFields +
				" FROM CommandList WHERE UUID=?"};

			CommandDetailsRecordTuple R;
			Select << ConvertParams(St),
				Poco::Data::Keywords::into(R),
				Poco::Data::Keywords::use(UUID);
			ConvertCommandRecord(R,Command);
			Select.execute();
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
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
			Logger().log(E);
		}
		return false;
	}

	bool Storage::GetNewestCommands(std::string &SerialNumber, uint64_t HowMany, std::vector<GWObjects::CommandDetails> &Commands) {
		try {
			CommandDetailsRecordList Records;

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string st{"SELECT " +
							   DB_Command_SelectFields +
							   " FROM CommandList WHERE SerialNumber=? ORDER BY Submitted DESC " + ComputeRange(0, HowMany)};
			Select << 	ConvertParams(st),
						Poco::Data::Keywords::into(Records),
						Poco::Data::Keywords::use(SerialNumber);
			Select.execute();

			for (auto i : Records) {
				GWObjects::CommandDetails R;
				ConvertCommandRecord(i,R);
				Commands.push_back(R);
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

	bool Storage::GetReadyToExecuteCommands(uint64_t Offset, uint64_t HowMany,
											std::vector<GWObjects::CommandDetails> &Commands) {

		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			uint64_t Now = time(nullptr);
			std::string St{
				"SELECT " +
				DB_Command_SelectFields
				+ " FROM CommandList "
				" WHERE ((RunAt<=?) And (Executed=0)) ORDER BY UUID ASC "};
			CommandDetailsRecordList Records;

			std::string SS = ConvertParams(St) + ComputeRange(Offset, HowMany);
			Select << SS,
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::use(Now);
			Select.execute();

			for(const auto &i : Records) {
				GWObjects::CommandDetails R;
				ConvertCommandRecord(i,R);
				if (DeviceRegistry()->Connected(R.SerialNumber))
					Commands.push_back(R);
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
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
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}

		return false;
	}

	bool Storage::CommandCompleted(std::string &UUID, const Poco::JSON::Object & ReturnVars,
								   const std::chrono::duration<double, std::milli> & execution_time,
								   bool FullCommand) {
		try {

			uint64_t Now = FullCommand ? time(nullptr) : 0;

			// Parse the result to get the ErrorText and make sure that this is a JSON document
			uint64_t ErrorCode = 0;
			std::string ErrorText, ResultStr;

			if (ReturnVars.has("result")) {
				auto ResultObj = ReturnVars.get("result");
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
			std::string St{"UPDATE CommandList SET Completed=?, ErrorCode=?, ErrorText=?, Results=?, Status=?, executionTime=? WHERE UUID=?"};
			double tET{execution_time.count()};
			Update << ConvertParams(St),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(ErrorCode),
				Poco::Data::Keywords::use(ErrorText),
				Poco::Data::Keywords::use(ResultStr),
				Poco::Data::Keywords::use(StatusText),
				Poco::Data::Keywords::use(tET),
				Poco::Data::Keywords::use(UUID);
			Update.execute();

			return true;
		} catch (const Poco::Exception &E) {
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
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
			Logger().log(E);
		}
		return false;

	}

	bool Storage::AttachFileToCommand(std::string &UUID) {
		try {
			Poco::Data::Session Sess = Pool_->get();
			uint64_t Now = OpenWifi::Now();
			uint64_t WaitForFile = 0;

			Poco::Data::Statement Update(Sess);
			std::cout << __LINE__ << std::endl;

			Poco::File FileName = FileUploader()->Path() + "/" + UUID;
			std::cout << __LINE__ << std::endl;
			uint64_t Size = FileName.getSize();
			std::cout << __LINE__ << std::endl;

			std::string St{
				"UPDATE CommandList SET WaitingForFile=?, AttachDate=?, AttachSize=? WHERE UUID=?"};
			std::cout << __LINE__ << std::endl;

			Update << ConvertParams(St),
				Poco::Data::Keywords::use(WaitForFile),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(Size),
				Poco::Data::Keywords::use(UUID);
			Update.execute();
			std::cout << __LINE__ << std::endl;

			if (FileName.getSize() < FileUploader()->MaxSize()) {

				Poco::Data::BLOB 		TheBlob;

				std::cout << __LINE__ << std::endl;

				std::ifstream f(FileName.path(), std::ios::binary);
				std::ostringstream SS;
				Poco::StreamCopier::copyStream(f, SS);
				TheBlob.appendRaw((const unsigned char *)SS.str().c_str(),SS.str().size());

				std::cout << "Attach file size: " <<  std::endl;

				/*
							"UUID			VARCHAR(64) PRIMARY KEY, "
							"Type			VARCHAR(32), "
							"Created 		BIGINT, "
							"FileContent	BYTEA"
				*/
				std::cout << __LINE__ << std::endl;
				Poco::Data::Statement Insert(Sess);
				std::string FileType{"trace"};
				std::cout << __LINE__ << std::endl;

				std::string St2{
					"INSERT INTO FileUploads (UUID,Type,Created,FileContent) VALUES(?,?,?,?)"};

				std::cout << __LINE__ << std::endl;
				Insert << ConvertParams(St2), Poco::Data::Keywords::use(UUID),
					Poco::Data::Keywords::use(FileType),
					Poco::Data::Keywords::use(Now),
					Poco::Data::Keywords::use(TheBlob);
				Insert.execute();
				std::cout << __LINE__ << std::endl;

				FileName.remove();
				std::cout << __LINE__ << std::endl;

				return true;
			} else {
				std::cout << __LINE__ << std::endl;
				Logger().warning(fmt::format("File {} is too large.", FileName.path()));
			}
		} catch (const Poco::Exception &E) {
			std::cout << __LINE__ << std::endl;
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
			std::cout << __LINE__ << std::endl;
		}
		return false;
	}

	bool Storage::GetAttachedFile(std::string &UUID, const std::string & SerialNumber, const std::string &FileName, std::string &Type) {
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
			std::ofstream f(FileName, std::ios::binary | std::ios::trunc );
			Poco::StreamCopier::copyStream(IL, f);

			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
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
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
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
			Logger().log(E);
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
			Logger().log(E);
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
					UpdateCountedMap(R,Command);
				More = RSet.moveNext();
			}
			return true;
		} catch(const Poco::Exception &E) {
			Logger().warning(fmt::format("{}: Failed with: {}", std::string(__func__), E.displayText()));
		}
		return false;
	}

}