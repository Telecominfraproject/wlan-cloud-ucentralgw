//
// Created by stephane bourque on 2021-07-05.
//

#ifndef UCENTRALGW_RESTAPI_UTILS_H
#define UCENTRALGW_RESTAPI_UTILS_H
#include <functional>

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "OpenWifiTypes.h"
#include "Utils.h"

namespace OpenWifi::RESTAPI_utils {

	void EmbedDocument(const std::string & ObjName, Poco::JSON::Object & Obj, const std::string &ObjStr);

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, bool V) {
		Obj.set(Field,V);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const std::string & S) {
		Obj.set(Field,S);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const char * S) {
		Obj.set(Field,S);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, uint64_t V) {
		Obj.set(Field,V);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::StringVec &V) {
		Poco::JSON::Array	A;
		for(const auto &i:V)
			A.add(i);
		Obj.set(Field,A);
	}

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::CountedMap &M) {
        Poco::JSON::Array	A;
        for(const auto &[Key,Value]:M) {
            Poco::JSON::Object  O;
            O.set("tag",Key);
            O.set("value", Value);
            A.add(O);
        }
        Obj.set(Field,A);
    }


    template<typename T> void field_to_json(Poco::JSON::Object &Obj,
					   						const char *Field,
					   						const T &V,
											std::function<std::string(const T &)> F) {
		Obj.set(Field, F(V));
	}

	template<typename T> bool field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, T & V,
											std::function<T(const std::string &)> F) {
		if(Obj->has(Field))
			V = F(Obj->get(Field).toString());
		return true;
	}

	inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, std::string &S) {
		if(Obj->has(Field))
			S = Obj->get(Field).toString();
	}

	inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, uint64_t &V) {
		if(Obj->has(Field))
			V = Obj->get(Field);
	}

	inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, bool &V) {
		if(Obj->has(Field))
			V = (Obj->get(Field).toString() == "true");
	}

	inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, Types::StringVec &V) {
		if(Obj->isArray(Field)) {
			V.clear();
			Poco::JSON::Array::Ptr A = Obj->getArray(Field);
			for(const auto &i:*A) {
				V.push_back(i.toString());
			}
		}
	}

	template<class T> void field_to_json(Poco::JSON::Object &Obj, const char *Field, const std::vector<T> &Value) {
		Poco::JSON::Array Arr;
		for(const auto &i:Value) {
			Poco::JSON::Object	AO;
			i.to_json(AO);
			Arr.add(AO);
		}
		Obj.set(Field, Arr);
	}

	template<class T> void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, std::vector<T> &Value) {
		if(Obj->isArray(Field)) {
			Poco::JSON::Array::Ptr	Arr = Obj->getArray(Field);
			for(auto &i:*Arr) {
				auto InnerObj = i.extract<Poco::JSON::Object::Ptr>();
				T	NewItem;
				NewItem.from_json(InnerObj);
				Value.push_back(NewItem);
			}
		}
	}

	template<class T> void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, T &Value) {
		if(Obj->isObject(Field)) {
			Poco::JSON::Object::Ptr	A = Obj->getObject(Field);
			Value.from_json(A);
		}
	}

	inline std::string to_string(const Types::StringVec & ObjectArray) {
		Poco::JSON::Array OutputArr;
		if(ObjectArray.empty())
			return "[]";
		for(auto const &i:ObjectArray) {
			OutputArr.add(i);
		}
		std::ostringstream OS;
		Poco::JSON::Stringifier::condense(OutputArr,OS);
		return OS.str();
	}

	template<class T> std::string to_string(const std::vector<T> & ObjectArray) {
		Poco::JSON::Array OutputArr;
		if(ObjectArray.empty())
			return "[]";
		for(auto const &i:ObjectArray) {
			Poco::JSON::Object O;
			i.to_json(O);
			OutputArr.add(O);
		}
		std::ostringstream OS;
		Poco::JSON::Stringifier::condense(OutputArr,OS);
		return OS.str();
	}

	template<class T> std::string to_string(const T & Object) {
		Poco::JSON::Object OutputObj;
		Object.to_json(OutputObj);
		std::ostringstream OS;
		Poco::JSON::Stringifier::condense(OutputObj,OS);
		return OS.str();
	}

    inline Types::StringVec to_object_array(const std::string & ObjectString) {

        Types::StringVec 	Result;
        if(ObjectString.empty())
            return Result;

        try {
            Poco::JSON::Parser P;
            auto Object = P.parse(ObjectString).template extract<Poco::JSON::Array::Ptr>();
            for (auto const i : *Object) {
                Result.push_back(i.toString());
            }
        } catch (...) {

        }
        return Result;
    }

    template<class T> std::vector<T> to_object_array(const std::string & ObjectString) {

		std::vector<T>	Result;
		if(ObjectString.empty())
			return Result;

		try {
			Poco::JSON::Parser P;
			auto Object = P.parse(ObjectString).template extract<Poco::JSON::Array::Ptr>();
			for (auto const i : *Object) {
				auto InnerObject = i.template extract<Poco::JSON::Object::Ptr>();
				T Obj;
				Obj.from_json(InnerObject);
				Result.push_back(Obj);
			}
		} catch (...) {

		}
		return Result;
	}

	template<class T> T to_object(const std::string & ObjectString) {
		T	Result;

		if(ObjectString.empty())
			return Result;

		Poco::JSON::Parser	P;
		auto Object = P.parse(ObjectString).template extract<Poco::JSON::Object::Ptr>();
		Result.from_json(Object);

		return Result;
	}

    template<class T> bool from_request(T & Obj, Poco::Net::HTTPServerRequest &Request) {
        Poco::JSON::Parser IncomingParser;
        auto RawObject = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Obj.from_json(RawObject);
        return true;
    }
}

#endif // UCENTRALGW_RESTAPI_UTILS_H
