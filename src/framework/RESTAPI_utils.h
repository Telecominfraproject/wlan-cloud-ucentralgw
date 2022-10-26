//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Data/LOB.h"
#include "Poco/Net/HTTPServerRequest.h"

#include "framework/OpenWifiTypes.h"
#include "framework/utils.h"

namespace OpenWifi::RESTAPI_utils {

	inline void EmbedDocument(const std::string & ObjName, Poco::JSON::Object & Obj, const std::string &ObjStr) {
		std::string D = ObjStr.empty() ? "{}" : ObjStr;
		Poco::JSON::Parser P;
		Poco::Dynamic::Var result = P.parse(D);
		const auto &DetailsObj = result.extract<Poco::JSON::Object::Ptr>();
		Obj.set(ObjName, DetailsObj);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, bool V) {
		Obj.set(Field,V);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, double V) {
		Obj.set(Field,V);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, float V) {
		Obj.set(Field,V);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const std::string & S) {
		Obj.set(Field,S);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const char * S) {
		Obj.set(Field,S);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, int16_t Value) {
		Obj.set(Field, Value);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, int32_t Value) {
		Obj.set(Field, Value);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, int64_t Value) {
		Obj.set(Field, Value);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, uint16_t Value) {
		Obj.set(Field, Value);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, uint32_t Value) {
		Obj.set(Field, Value);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, uint64_t Value) {
		Obj.set(Field,Value);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Poco::Data::BLOB &Value) {
		auto Result = Utils::base64encode((const unsigned char *)Value.rawContent(),Value.size());
		Obj.set(Field,Result);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::StringPairVec & S) {
		Poco::JSON::Array   Array;
		for(const auto &i:S) {
			Poco::JSON::Object  O;
			O.set("tag",i.first);
			O.set("value", i.second);
			Array.add(O);
		}
		Obj.set(Field,Array);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::StringVec &V) {
		Poco::JSON::Array	A;
		for(const auto &i:V)
			A.add(i);
		Obj.set(Field,A);
	}

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::TagList &V) {
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

	inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::Counted3DMapSII &M) {
		Poco::JSON::Array	A;
		for(const auto &[OrgName,MonthlyNumberMap]:M) {
			Poco::JSON::Object  OrgObject;
			OrgObject.set("tag",OrgName);
			Poco::JSON::Array   MonthlyArray;
			for(const auto &[Month,Counter]:MonthlyNumberMap) {
				Poco::JSON::Object  Inner;
				Inner.set("value", Month);
				Inner.set("counter", Counter);
				MonthlyArray.add(Inner);
			}
			OrgObject.set("index",MonthlyArray);
			A.add(OrgObject);
		}
		Obj.set(Field, A);
	}

	template<typename T> void field_to_json(Poco::JSON::Object &Obj,
					   const char *Field,
					   const T &V,
					   std::function<std::string(const T &)> F) {
		Obj.set(Field, F(V));
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

	template<class T> void field_to_json(Poco::JSON::Object &Obj, const char *Field, const T &Value) {
		Poco::JSON::Object  Answer;
		Value.to_json(Answer);
		Obj.set(Field, Answer);
	}

	///////////////////////////
	///////////////////////////
	///////////////////////////
	///////////////////////////

	template<typename T> bool field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, T & V,
						 std::function<T(const std::string &)> F) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			V = F(Obj->get(Field).toString());
		return true;
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, std::string &S) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			S = Obj->get(Field).toString();
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, double & Value) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			Value = (double)Obj->get(Field);
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, float & Value) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			Value = (float)Obj->get(Field);
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, bool &Value) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			Value = (Obj->get(Field).toString() == "true");
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, int16_t &Value) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			Value = (int16_t)Obj->get(Field);
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, int32_t &Value) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			Value = (int32_t) Obj->get(Field);
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, int64_t &Value) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			Value = (int64_t)Obj->get(Field);
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, uint16_t &Value) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			Value = (uint16_t)Obj->get(Field);
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, uint32_t &Value) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			Value = (uint32_t)Obj->get(Field);
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, uint64_t &Value) {
		if(Obj->has(Field) && !Obj->isNull(Field))
			Value = (uint64_t)Obj->get(Field);
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, Poco::Data::BLOB &Value) {
		if(Obj->has(Field) && !Obj->isNull(Field)) {
			auto Result = Utils::base64decode(Obj->get(Field).toString());
			Value.assignRaw((const unsigned char *)&Result[0],Result.size());
		}
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, Types::StringPairVec &Vec) {
		if(Obj->isArray(Field) && !Obj->isNull(Field)) {
			auto O = Obj->getArray(Field);
			for(const auto &i:*O) {
				std::string S1,S2;
				auto Inner = i.extract<Poco::JSON::Object::Ptr>();
				if(Inner->has("tag"))
					S1 = Inner->get("tag").toString();
				if(Inner->has("value"))
					S2 = Inner->get("value").toString();
				auto P = std::make_pair(S1,S2);
				Vec.push_back(P);
			}
		}
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, Types::StringVec &Value) {
		if(Obj->isArray(Field) && !Obj->isNull(Field)) {
			Value.clear();
			Poco::JSON::Array::Ptr A = Obj->getArray(Field);
			for(const auto &i:*A) {
				Value.push_back(i.toString());
			}
		}
	}

	inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, Types::TagList &Value) {
		if(Obj->isArray(Field) && !Obj->isNull(Field)) {
			Value.clear();
			Poco::JSON::Array::Ptr A = Obj->getArray(Field);
			for(const auto &i:*A) {
				Value.push_back(i);
			}
		}
	}

	template<class T> void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, std::vector<T> &Value) {
		if(Obj->isArray(Field) && !Obj->isNull(Field)) {
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
		if(Obj->isObject(Field) && !Obj->isNull(Field)) {
			Poco::JSON::Object::Ptr	A = Obj->getObject(Field);
			Value.from_json(A);
		}
	}

	inline std::string to_string(const Types::TagList & ObjectArray) {
		Poco::JSON::Array OutputArr;
		if(ObjectArray.empty())
			return "[]";
		for(auto const &i:ObjectArray) {
			OutputArr.add(i);
		}
		std::ostringstream OS;
		Poco::JSON::Stringifier::stringify(OutputArr,OS, 0,0, Poco::JSON_PRESERVE_KEY_ORDER );
		return OS.str();
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

	inline std::string to_string(const Types::StringPairVec & ObjectArray) {
		Poco::JSON::Array OutputArr;
		if(ObjectArray.empty())
			return "[]";
		for(auto const &i:ObjectArray) {
			Poco::JSON::Array InnerArray;
			InnerArray.add(i.first);
			InnerArray.add(i.second);
			OutputArr.add(InnerArray);
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

	template<class T> std::string to_string(const std::vector<std::vector<T>> & ObjectArray) {
		Poco::JSON::Array OutputArr;
		if(ObjectArray.empty())
			return "[]";
		for(auto const &i:ObjectArray) {
			Poco::JSON::Array InnerArr;
			for(auto const &j:i) {
				if constexpr(std::is_integral<T>::value) {
					InnerArr.add(j);
				} if constexpr(std::is_same_v<T,std::string>) {
					InnerArr.add(j);
				} else {
					InnerArr.add(j);
					Poco::JSON::Object O;
					j.to_json(O);
					InnerArr.add(O);
				}
			}
			OutputArr.add(InnerArr);
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
			for (auto const &i : *Object) {
				Result.push_back(i.toString());
			}
		} catch (...) {

		}
		return Result;
	}

	inline OpenWifi::Types::TagList to_taglist(const std::string & ObjectString) {
		Types::TagList 	Result;
		if(ObjectString.empty())
			return Result;

		try {
			Poco::JSON::Parser P;
			auto Object = P.parse(ObjectString).template extract<Poco::JSON::Array::Ptr>();
			for (auto const &i : *Object) {
				Result.push_back(i);
			}
		} catch (...) {

		}
		return Result;
	}

	inline Types::StringPairVec to_stringpair_array(const std::string &S) {
		Types::StringPairVec   R;
		if(S.empty())
			return R;
		try {
			Poco::JSON::Parser P;
			auto Object = P.parse(S).template extract<Poco::JSON::Array::Ptr>();
			for (const auto &i : *Object) {
				auto InnerObject = i.template extract<Poco::JSON::Array::Ptr>();
				if(InnerObject->size()==2) {
					auto S1 = InnerObject->getElement<std::string>(0);
					auto S2 = InnerObject->getElement<std::string>(1);
					R.push_back(std::make_pair(S1,S2));
				}
			}
		} catch (...) {

		}

		return R;
	}

	template<class T> std::vector<T> to_object_array(const std::string & ObjectString) {
		std::vector<T>	Result;
		if(ObjectString.empty())
			return Result;

		try {
			Poco::JSON::Parser P;
			auto Object = P.parse(ObjectString).template extract<Poco::JSON::Array::Ptr>();
			for (auto const &i : *Object) {
				auto InnerObject = i.template extract<Poco::JSON::Object::Ptr>();
				T Obj;
				Obj.from_json(InnerObject);
				Result.push_back(Obj);
			}
		} catch (...) {

		}
		return Result;
	}

	template<class T> std::vector<std::vector<T>> to_array_of_array_of_object(const std::string & ObjectString) {
		std::vector<std::vector<T>>	Result;
		if(ObjectString.empty())
			return Result;
		try {
			Poco::JSON::Parser P1;
			auto OutterArray = P1.parse(ObjectString).template extract<Poco::JSON::Array::Ptr>();
			for (auto const &i : *OutterArray) {
				Poco::JSON::Parser P2;
				auto InnerArray = P2.parse(i).template extract<Poco::JSON::Array::Ptr>();
				std::vector<T>  InnerVector;
				for(auto const &j: *InnerArray) {
					auto Object = j.template extract<Poco::JSON::Object::Ptr>();
					T Obj;
					Obj.from_json(Object);
					InnerVector.push_back(Obj);
				}
				Result.push_back(InnerVector);
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
