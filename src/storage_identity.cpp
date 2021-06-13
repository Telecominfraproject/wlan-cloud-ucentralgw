//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"

namespace uCentral {

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

	bool Storage::IdentityExists(std::string & Identity, AuthService::ACCESS_TYPE Type) {
		try {
			int TT = AuthService::AccessTypeToInt(Type);
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

	bool Storage::AddIdentity(std::string & Identity, std::string & Password, AuthService::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL) {
		try {

		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::GetIdentity(std::string & Identity, std::string & Password,AuthService::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL) {
		try {

		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::UpdateIdentity(std::string & Identity, std::string & Password, AuthService::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL) {
		try {

		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::DeleteIdentity(std::string & Identity, AuthService::ACCESS_TYPE Type) {
		try {
			int TT = AuthService::AccessTypeToInt(Type);
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

	bool Storage::ListIdentities(uint64_t Offset, uint64_t HowMany, std::vector<std::string> & Identities, AuthService::ACCESS_TYPE Type) {
		try {

		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}

	bool Storage::GetIdentityRights(std::string &Identity, uCentral::Objects::AclTemplate &ACL) {
		try {

		} catch(const Poco::Exception &E )
		{
			Logger_.log(E);
		}
		return false;
	}


}

