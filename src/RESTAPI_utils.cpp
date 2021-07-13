//
// Created by stephane bourque on 2021-07-05.
//

#include "RESTAPI_utils.h"

namespace uCentral::RESTAPI_utils {

	void EmbedDocument(const std::string & ObjName, Poco::JSON::Object & Obj, const std::string &ObjStr) {
		std::string D = ObjStr.empty() ? "{}" : ObjStr;
		Poco::JSON::Parser P;
		Poco::Dynamic::Var result = P.parse(D);
		const auto &DetailsObj = result.extract<Poco::JSON::Object::Ptr>();
		Obj.set(ObjName, DetailsObj);
	}
}

