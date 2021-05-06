//
// Created by stephane bourque on 2021-05-06.
//

#include "uStorageService.h"

namespace uCentral::Storage {

	/*
				"Identity			VARCHAR(128) PRIMARY KEY, "
				"Password			VARCHAR(128), "
				"AccessType			INT, "
				"Created			BIGINT, "
				"Modified			BIGINT, "
				"Expires 			BIGINT, "
				"CreatedBy			VARCHAR(128), "
				"ACLRead 			INT, "
				"ACLReadWrite 		INT, "
				"ACLReadWriteCreate INT, "
				"ACLDelete 			INT, "
				"ACLPortal 			INT "
				") ", Poco::Data::Keywords::now;
	 */

	bool IdentityExists(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type) {
		return uCentral::Storage::Service::instance()->IdentityExists(Identity, Type);
	}

	bool AddIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Auth::AclTemplate & ACL) {
		return uCentral::Storage::Service::instance()->AddIdentity(Identity, Password, Type, ACL);
	}

	bool GetIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Auth::AclTemplate & ACL) {
		return uCentral::Storage::Service::instance()->GetIdentity(Identity, Password, Type, ACL);
	}

	bool DeleteIdentity(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type) {
		return uCentral::Storage::Service::instance()->DeleteIdentity(Identity, Type);
	}

	bool ListIdentities(uint64_t Offset, uint64_t HowMany, std::vector<std::string> & Identities, uCentral::Auth::ACCESS_TYPE Type) {
		return uCentral::Storage::Service::instance()->ListIdentities(Offset, HowMany, Identities, Type);
	}



bool Service::IdentityExists(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type) {
		try {
			int TT = uCentral::Auth::AccessTypeToInt(Type);
			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement	Select(Sess);

			std::string Tmp;
			std::string st{"SELECT Identity FROM Authentication WHERE Identity=? AND AccessType=?"};

			Select << ConvertParams(st),
				Poco::Data::Keywords::into(Tmp),
				Poco::Data::Keywords::use(Identity),
				Poco::Data::Keywords::use(TT);
			Select.execute();

			return !Identity.empty();
		} catch (const Poco::Exception &E ) {
			Logger_.log(E);
		}
		return false;
	}

	bool Service::AddIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Auth::AclTemplate & ACL) {
		try {

		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}

	bool Service::GetIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Auth::AclTemplate & ACL) {
		try {

		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}

	bool Service::UpdateIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Auth::AclTemplate & ACL) {
		try {

		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}

	bool Service::DeleteIdentity(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type) {
		try {
			int TT = uCentral::Auth::AccessTypeToInt(Type);
			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement	Delete(Sess);

			std::string Tmp;
			std::string st{"DELETE FROM Authentication WHERE Identity=? AND AccessType=?"};

			Delete << ConvertParams(st),
				Poco::Data::Keywords::use(Identity),
				Poco::Data::Keywords::use(TT);
			Delete.execute();

			return true;
		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}

	bool Service::ListIdentities(uint64_t Offset, uint64_t HowMany, std::vector<std::string> & Identities, uCentral::Auth::ACCESS_TYPE Type) {
		try {

		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}


}

