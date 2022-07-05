//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <array>

#include "Poco/Tuple.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Logger.h"
#include "Poco/StringTokenizer.h"
#include "StorageClass.h"

namespace ORM {

    enum FieldType {
        FT_INT,
        FT_BIGINT,
        FT_TEXT,
        FT_VARCHAR,
        FT_BLOB,
        FT_BOOLEAN,
        FT_REAL
    };

    enum Indextype {
        ASC,
        DESC
    };

    struct Field {
        std::string Name;
        FieldType   Type;
        int         Size=0;
        bool        Index=false;


        Field(std::string N, FieldType T, int S=0, bool Index=false) :
            Name(std::move(N)),
            Type(T),
            Size(S),
            Index(Index) {}

        explicit Field(std::string N) :
            Name(std::move(N))
        {
            Type = FT_TEXT;
        }

        Field(std::string N, int S) :
            Name(std::move(N)), Size(S)
        {
            if(Size>0 && Size<255)
                Type = FT_VARCHAR;
            else
                Type = FT_TEXT;
        }

        Field(std::string N, int S, bool I):
            Name(std::move(N)), Size(S), Index(I)
        {
            if(Size>0 && Size<255)
                Type = FT_VARCHAR;
            else
                Type = FT_TEXT;
        }
    };
    typedef std::vector<Field>  FieldVec;

    struct IndexEntry {
        std::string         FieldName;
        Indextype           Type;
    };
    typedef std::vector<IndexEntry>     IndexEntryVec;

    struct Index {
        std::string         Name;
        IndexEntryVec       Entries;
    };
    typedef std::vector<Index>      IndexVec;

    inline std::string FieldTypeToChar(OpenWifi::DBType Type, FieldType T, int Size=0) {
        switch(T) {
            case FT_INT:    return "INT";
            case FT_BIGINT: return "BIGINT";
            case FT_TEXT:   return "TEXT";
            case FT_BOOLEAN:   return "BOOLEAN";
            case FT_VARCHAR:
                if(Size)
                    return std::string("VARCHAR(") + std::to_string(Size) + std::string(")");
                else
                    return "TEXT";
            case FT_BLOB:
                if(Type==OpenWifi::DBType::mysql)
                    return "LONGBLOB";
                else if(Type==OpenWifi::DBType::pgsql)
                    return "BYTEA";
                else
                    return "BLOB";
            case FT_REAL:
                return "REAL";
            default:
                assert(false);

        }
        assert(false);
        return "";
    }

    inline std::string Escape(const std::string &S) {
        std::string R;

        for(const auto &i:S) {
            if (i == '\'')
                R += "''";
            else
                R += i;
        }
        return R;
    }

    inline std::string WHERE_AND_(std::string Result) {
        return Result;
    }

    template <typename T, typename... Args> std::string WHERE_AND_(std::string Result, const char *fieldName, const T &Value, Args... args) {
        if constexpr(std::is_same_v<T,std::string>)
        {
            if(!Value.empty()) {
                if(!Result.empty())
                    Result += " and ";
                Result += fieldName;
                Result += '=';
                Result += "'";
                Result += Escape(Value);
                Result += "'";
            }
        } else {
            if(!Result.empty())
                Result += " and ";
            Result += fieldName ;
            Result += '=';
            Result += std::to_string(Value);
        }
        return WHERE_AND_(Result,args...);
    }

    template <typename... Args> std::string WHERE_AND(Args... args) {
        std::string Result;
        return WHERE_AND_(Result, args...);
    }

    enum SqlComparison { EQ = 0, NEQ, LT, LTE, GT, GTE };
    enum SqlBinaryOp { AND = 0 , OR };

    static const std::vector<std::string> BOPS{" and ", " or "};
    static const std::vector<std::string> SQLCOMPS{"=","!=","<","<=",">",">="};

    inline std::string to_string(uint64_t V) {
        return std::to_string(V);
    }

    inline std::string to_string(int V) {
        return std::to_string(V);
    }

    inline std::string to_string(bool V) {
        return std::to_string(V);
    }

    inline std::string to_string(const std::string &S) {
        return S;
    }

    inline std::string to_string(const Poco::Data::BLOB &blob) {
        std::string result;
        result.assign(blob.begin(),blob.end());
        return result;
    }

    inline std::string to_string(const char * S) {
        return S;
    }

    template <typename RecordType> class DBCache {
    public:
        DBCache(unsigned Size, unsigned Timeout) :
            Size_(Size),
            Timeout_(Timeout)
        {

        }
        virtual void Create(const RecordType &R)=0;
        virtual bool GetFromCache(const std::string &FieldName, const std::string &Value, RecordType &R)=0;
        virtual void UpdateCache(const RecordType &R)=0;
        virtual void Delete(const std::string &FieldName, const std::string &Value)=0;
    private:
        size_t          Size_=0;
        uint64_t        Timeout_=0;
    };

    template <typename RecordTuple, typename RecordType> class DB {
    public:

        typedef const char * field_name_t;

        DB( OpenWifi::DBType dbtype,
            const char *TableName,
            const FieldVec & Fields,
            const IndexVec & Indexes,
            Poco::Data::SessionPool & Pool,
            Poco::Logger &L,
            const char *Prefix,
            DBCache<RecordType> * Cache=nullptr):
                TableName_(TableName),
                Type_(dbtype),
                Pool_(Pool),
                Logger_(L),
                Prefix_(Prefix),
                Cache_(Cache)
        {
            assert(RecordTuple::length == Fields.size());

            bool first = true;
            int  Place=0;

            for(const auto &i:Fields) {
                std::string FieldName = Poco::toLower(i.Name);
                FieldNames_[FieldName] = Place;
                if(!first) {
                    CreateFields_ += ", ";
                    SelectFields_ += ", ";
                    UpdateFields_ += ", ";
                    SelectList_ += ", ";
                } else {
                    SelectList_ += "(";
                }

                CreateFields_ += FieldName + " " + FieldTypeToChar(Type_, i.Type,i.Size) + (i.Index ? " unique primary key" : "");
                SelectFields_ += FieldName ;
                UpdateFields_ += FieldName + "=?";
                SelectList_ += "?";
                first = false;
                Place++;
            }
            SelectList_ += ")";

            if(!Indexes.empty()) {
                if(Type_==OpenWifi::DBType::sqlite || Type_==OpenWifi::DBType::pgsql) {
                    for(const auto &j:Indexes) {
                        std::string IndexLine;

                        IndexLine = std::string("CREATE INDEX IF NOT EXISTS ") + j.Name + std::string(" ON ") + TableName_+ " (";
                        bool first_entry=true;
                        for(const auto &k:j.Entries) {
                            auto IndexFieldName = Poco::toLower(k.FieldName);
                            assert(ValidFieldName(IndexFieldName));
                            if(!first_entry) {
                                IndexLine += " , ";
                            }
                            first_entry = false;
                            IndexLine += IndexFieldName + std::string(" ") + std::string(k.Type == Indextype::ASC ? "ASC" : "DESC") ;
                        }
                        IndexLine += " )";
                        IndexCreation_.template emplace_back(IndexLine);
                    }
                } else if(Type_==OpenWifi::DBType::mysql) {
                    bool firstIndex = true;
                    std::string IndexLine;
                    for(const auto &j:Indexes) {
                        if(!firstIndex)
                            IndexLine += ", ";
                        firstIndex = false;
                        IndexLine += " INDEX " + j.Name + " ( " ;
                        bool first_entry=true;
                        for(const auto &k:j.Entries) {
                            auto IndexFieldName = Poco::toLower(k.FieldName);
                            assert(FieldNames_.find(IndexFieldName) != FieldNames_.end());
                            if(!first_entry) {
                                IndexLine += " ,";
                            }
                            first_entry = false;
                            IndexLine += IndexFieldName + std::string(k.Type == Indextype::ASC ? " ASC" : " DESC");
                        }
                        IndexLine += " ) ";
                    }
                    IndexCreation_.template emplace_back(IndexLine);
                }
            }
        }

        [[nodiscard]] const std::string & CreateFields() const { return CreateFields_; };
        [[nodiscard]] const std::string & SelectFields() const { return SelectFields_; };
        [[nodiscard]] const std::string & SelectList() const { return SelectList_; };
        [[nodiscard]] const std::string & UpdateFields() const { return UpdateFields_; };

        inline std::string OP(field_name_t F, SqlComparison O , bool V) {
            assert(ValidFieldName(F));
            return std::string{"("} + F + SQLCOMPS[O] + (V ? "true" : "false") + ")" ;
        }

        inline std::string OP(field_name_t F, SqlComparison O , int V) {
            assert(ValidFieldName(F));
            return std::string{"("} + F + SQLCOMPS[O] + std::to_string(V) + ")" ;
        }

        inline std::string OP(field_name_t F, SqlComparison O , uint64_t V) {
            assert(ValidFieldName(F));
            return std::string{"("} + F + SQLCOMPS[O] + std::to_string(V) + ")" ;
        }

        std::string OP(field_name_t F, SqlComparison O , const std::string & V) {
            assert(ValidFieldName(F));
            return std::string{"("} + F + SQLCOMPS[O] + "'" + Escape(V) + "')" ;
        }

        std::string OP(field_name_t F, SqlComparison O , const char * V) {
            assert(ValidFieldName(F));
            return std::string{"("} + F + SQLCOMPS[O] + "'" + Escape(V) + "')" ;
        }

        static std::string OP( const std::string &P1, SqlBinaryOp BOP , const std::string &P2) {
            return std::string("(")+P1 + BOPS[BOP] + P2 +")";
        }

        std::string OP( [[maybe_unused]] bool  Paran, const std::string &P1, SqlBinaryOp BOP , const std::string &P2) {
            return P1 + BOPS[BOP] + P2 +")";
        }

        template <typename... Others> std::string OP( bool ParanOpen, const std::string &P1, SqlBinaryOp BOP , const std::string &P2, Others... More) {
            return P1 + BOPS[BOP] + OP(ParanOpen, P2, More...) + ")";
        }

        template <typename... Others> std::string OP( const std::string &P1, SqlBinaryOp BOP , const std::string &P2, Others... More) {
            return std::string{"("} + P1 + BOPS[BOP] + OP(true, P2, More...);
        }

        bool Upgrade() {
            uint32_t    To;
            return Upgrade(0, To);
        }

        inline bool  Create() {
            switch(Type_) {
                case OpenWifi::DBType::mysql: {
                    try {
                        Poco::Data::Session     Session = Pool_.get();
                        std::string Statement = IndexCreation_.empty() ?    "create table if not exists " + TableName_ +" ( " + CreateFields_ + " )" :
                                                                            "create table if not exists " + TableName_ +" ( " + CreateFields_ + " ), " + IndexCreation_[0] + " )";
                        Session << Statement , Poco::Data::Keywords::now;
                    } catch (const Poco::Exception &E) {
                        Logger_.error("Failure to create MySQL DB resources.");
                        Logger_.log(E);
                    }
                }
                break;

                case OpenWifi::DBType::sqlite: {
                    try {
                        Poco::Data::Session     Session = Pool_.get();
                        std::string Statement = "create table if not exists " + TableName_ + " ( " + CreateFields_ + " )";
                        Session << Statement , Poco::Data::Keywords::now;
                        for(const auto &i:IndexCreation_) {
                            Session << i , Poco::Data::Keywords::now;
                        }
                    } catch (const Poco::Exception &E) {
                        Logger_.error("Failure to create SQLITE DB resources.");
                        Logger_.log(E);
                    }
                }
                break;

                case OpenWifi::DBType::pgsql: {
                    try {
                        Poco::Data::Session     Session = Pool_.get();
                        std::string Statement = "create table if not exists " + TableName_ + " ( " + CreateFields_ + " )";
                        Session << Statement , Poco::Data::Keywords::now;
                        for(const auto &i:IndexCreation_) {
                            Session << i , Poco::Data::Keywords::now;
                        }
                    } catch (const Poco::Exception &E) {
                        Logger_.error("Failure to create POSTGRESQL DB resources.");
                        Logger_.log(E);
                    }
                }
                break;
            }
            return Upgrade();
        }

        [[nodiscard]] std::string ConvertParams(const std::string & S) const {
            if(Type_!=OpenWifi::DBType::pgsql)
                return S;

            std::string R;
            R.reserve(S.size()*2+1);
            auto Idx=1;
            for(auto const & i:S)
            {
                if(i=='?') {
                    R += '$';
                    R.append(std::to_string(Idx++));
                } else {
                    R += i;
                }
            }

            return R;
        }

        void Convert( const RecordTuple &in , RecordType &out);
        void Convert( const RecordType &in , RecordTuple &out);

        inline const std::string & Prefix() { return Prefix_; };

        bool CreateRecord( const RecordType & R) {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Insert(Session);

                RecordTuple RT;
                Convert(R, RT);
                std::string St = "insert into  " + TableName_ + " ( " + SelectFields_ + " ) values " + SelectList_;
                Insert  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(RT);
                Insert.execute();

                if(Cache_)
                    Cache_->Create(R);
                return true;

            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template<typename T> bool GetRecord(field_name_t FieldName, const T & Value,  RecordType & R) {
            try {
                assert(ValidFieldName(FieldName));

                if(Cache_) {
                    if(Cache_->GetFromCache(FieldName, Value, R))
                        return true;
                }

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple             RT;

                std::string St = "select " + SelectFields_ + " from " + TableName_ + " where " + FieldName + "=?" + " limit 1";

                auto tValue{Value};

                Select  << ConvertParams(St) ,
                    Poco::Data::Keywords::into(RT),
                    Poco::Data::Keywords::use(tValue);
                Select.execute();

                if(Select.execute()==1) {
                    Convert(RT,R);
                    if(Cache_)
                        Cache_->UpdateCache(R);
                    return true;
                }
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        typedef std::vector<std::string> StringVec;

        template <  typename T,
                typename T0, typename T1> bool GR(field_name_t FieldName, T & R,T0 &V0, T1 &V1) {
            try {

                assert( ValidFieldName(FieldName) );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple RT;

                std::string St = "select " + SelectFields_ + " from " + TableName_
                                + " where " + FieldName[0] + "=? and " + FieldName[1] + "=?"  ;
                Select  << ConvertParams(St) ,
                    Poco::Data::Keywords::into(RT),
                    Poco::Data::Keywords::use(V0),
                    Poco::Data::Keywords::use(V1);

                if(Select.execute()==1) {
                    Convert(RT,R);
                    return true;
                }
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        typedef std::vector<RecordTuple>    RecordList;
        typedef std::vector<RecordType>     RecordVec;
        typedef RecordType                  RecordName;

        bool GetRecords( uint64_t Offset, uint64_t HowMany, RecordVec & Records, const std::string & Where = "", const std::string & OrderBy = "") {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordList RL;
                std::string St = "select " + SelectFields_ + " from " + TableName_ +
                        (Where.empty() ? "" : " where " + Where) + OrderBy +
                        ComputeRange(Offset, HowMany) ;

                Select  << St ,
                    Poco::Data::Keywords::into(RL);
                Select.execute();

                if(Select.rowsExtracted()>0) {
                    for(auto &i:RL) {
                        RecordType  R;
                        Convert(i, R);
                        Records.template emplace_back(R);
                    }
                    return true;
                }
                return false;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool UpdateRecord(field_name_t FieldName, const T & Value,  const RecordType & R) {
            try {
                assert( ValidFieldName(FieldName) );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Update(Session);

                RecordTuple RT;

                Convert(R, RT);

                auto tValue(Value);

                std::string St = "update " + TableName_ + " set " + UpdateFields_ + " where " + FieldName + "=?" ;
                Update  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(RT),
                    Poco::Data::Keywords::use(tValue);
                Update.execute();
                if(Cache_)
                    Cache_->UpdateCache(R);
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool RunStatement(const std::string &St) {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Command(Session);

                Command  << St ;
                Command.execute();

                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool ReplaceRecord(field_name_t FieldName, const T & Value,  RecordType & R) {
            try {
                if(Exists(FieldName, Value)) {
                    return UpdateRecord(FieldName,Value,R);
                }
                return CreateRecord(R);
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool GetNameAndDescription(field_name_t FieldName, const T & Value, std::string & Name, std::string & Description ) {
            try {
                assert( ValidFieldName(FieldName) );
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple             RT;

                std::string St = "select " + SelectFields_ + " from " + TableName_ + " where " + FieldName + "=?" ;
                RecordType R;
                auto tValue{Value};
                Select  << ConvertParams(St) ,
                    Poco::Data::Keywords::into(RT),
                    Poco::Data::Keywords::use(tValue);

                if(Select.execute()==1) {
                    Convert(RT,R);
                    Name = R.info.name;
                    Description = R.info.description;
                    return true;
                }
                return false;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool DeleteRecord(field_name_t FieldName, const T & Value) {
            try {
                assert( ValidFieldName(FieldName) );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Delete(Session);

                std::string St = "delete from " + TableName_ + " where " + FieldName + "=?" ;
                auto tValue{Value};

                Delete  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(tValue);
                Delete.execute();
                if(Cache_)
                    Cache_->Delete(FieldName, Value);
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool DeleteRecords( const std::string & WhereClause ) {
            try {
                assert( !WhereClause.empty());
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Delete(Session);

                std::string St = "delete from " + TableName_ + " where " + WhereClause;
                Delete  << St;
                Delete.execute();
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool Exists(field_name_t FieldName, const std::string & Value) {
            try {
                assert( ValidFieldName(FieldName) );

                RecordType  R;
                if(GetRecord(FieldName,Value,R))
                    return true;
                return false;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool Iterate( std::function<bool(const RecordType &R)> F, const std::string & WhereClause = "" ) {
            try {

                uint64_t    Offset=0;
                uint64_t    Batch=50;
                bool Done=false;
                while(!Done) {
                    std::vector<RecordType> Records;
                    if(GetRecords(Offset,Batch,Records, WhereClause)) {
                        for(const auto &i:Records) {
                            if(!F(i))
                                return true;
                        }
                        if(Records.size()<Batch)
                            return true;
                        Offset += Batch;
                    } else {
                        Done=true;
                    }
                }
                return true;
            } catch(const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool PrepareOrderBy(const std::string &OrderByList, std::string &OrderByString) {
            auto items = Poco::StringTokenizer(OrderByList,",");
            std::string ItemList;

            for(const auto &i:items) {
                auto T = Poco::StringTokenizer(i,":");
                if(T.count()!=2) {
                    return false;
                }
                if(T[1]!="a" && T[1]!="d") {
                    return false;
                }
                if(!ItemList.empty())
                    ItemList += " , ";
                auto hint = FieldNames_.find(Poco::toLower(T[0]));
                if(hint==FieldNames_.end()) {
                    return false;
                }
                ItemList += T[0] + (T[1]=="a" ? " ASC" : " DESC");
            }

            if(!ItemList.empty()) {
                OrderByString = " ORDER BY " + ItemList;
            }
            return true;
        }

        uint64_t Count( const std::string & Where="" ) {
            try {
                uint64_t Cnt=0;

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);

                std::string st{"SELECT COUNT(*) FROM " + TableName_ + " " + (Where.empty() ? "" : (" where " + Where)) };

                Select << st ,
                    Poco::Data::Keywords::into(Cnt);
                Select.execute();

                return Cnt;

            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return 0;
        }

        template <typename X> bool ManipulateVectorMember( X T, field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID, bool Add) {
            try {
                assert( ValidFieldName(FieldName) );

                RecordType R;
                if(GetRecord(FieldName, ParentUUID, R)) {
                    auto it = std::find((R.*T).begin(),(R.*T).end(),ChildUUID);
                    if(Add) {
                        if(it!=(R.*T).end() && *it == ChildUUID)
                            return false;
                        (R.*T).push_back(ChildUUID);
                        std::sort((R.*T).begin(),(R.*T).end());
                    } else {
                        if(it!=(R.*T).end() && *it == ChildUUID)
                            (R.*T).erase(it);
                        else
                            return false;
                    }
                    UpdateRecord(FieldName, ParentUUID, R);
                    return true;
                }
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool RunScript(const std::vector<std::string> & Statements, bool IgnoreExceptions=true) {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Command(Session);

                for(const auto &i:Statements) {
                    try {
                        Command << i, Poco::Data::Keywords::now;
                    } catch (const Poco::Exception &E) {
                        // Logger_.log(E);
                        // Logger_.error(Poco::format("The following statement '%s' generated an exception during a table upgrade. This may or may not be a problem.", i));
                        if(!IgnoreExceptions) {
                            return false;
                        }
                    }
                    Command.reset(Session);
                }
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        virtual uint32_t Version() {
            return 0;
        }

        virtual bool Upgrade(uint32_t from, uint32_t &to) {
            to = from;
            return true;
        }
        
        inline bool AddChild(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::children, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteChild(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::children, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddLocation(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::locations, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteLocation(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::locations, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddContact(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::contacts, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteContact(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::contacts, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddVenue(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::venues, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteVenue(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::venues, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddDevice(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::devices, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteDevice(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::devices, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddEntity(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::entities, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteEntity(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::entities, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddUser(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::users, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DelUser(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::users, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddConfiguration(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::deviceConfiguration, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DelConfiguration(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::deviceConfiguration, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddVariable(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::variables, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DelVariable(field_name_t FieldName, const std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::variables, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddInUse(field_name_t FieldName, const std::string & ParentUUID, const std::string & Prefix, const std::string & ChildUUID) {
            std::string FakeUUID{ Prefix + ":" + ChildUUID};
            return ManipulateVectorMember(&RecordType::inUse,FieldName, ParentUUID, FakeUUID, true);
        }

        inline bool DeleteInUse(field_name_t FieldName, const std::string & ParentUUID, const std::string & Prefix, const std::string & ChildUUID) {
            std::string FakeUUID{ Prefix + ":" + ChildUUID};
            return ManipulateVectorMember(&RecordType::inUse,FieldName, ParentUUID, FakeUUID, false);
        }

        inline bool GetInUse(field_name_t FieldName, const std::string & UUID, std::vector<std::string> & UUIDs ) {
            RecordType  R;
            if(GetRecord(FieldName,UUID,R)) {
                UUIDs = R.inUse;
                return true;
            }
            return false;
        }

        inline bool ValidFieldName(const std::string &FieldName) {
            return FieldNames_.find(Poco::toLower(FieldName)) != FieldNames_.end();
        }

        inline bool ValidFieldName(const char *FieldName) {
            std::string Field{FieldName};
            return ValidFieldName(Field);
        }

        [[nodiscard]] inline std::string ComputeRange(uint64_t From, uint64_t HowMany) {
            if(From<1) From=0;
            switch(Type_) {
                case OpenWifi::DBType::sqlite:
                    return " LIMIT " + std::to_string(From) + ", " + std::to_string(HowMany) +  " ";
                case OpenWifi::DBType::pgsql:
                    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From) + " ";
                case OpenWifi::DBType::mysql:
                    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From) + " ";
                default:
                    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From) + " ";
            }
        }

        Poco::Logger & Logger() { return Logger_; }

        inline bool DeleteRecordsFromCache(const char *FieldName, const std::string &Value ) {
            if(Cache_)
                Cache_->Delete(FieldName, Value);
            return true;
        }

        inline void GetFieldNames( OpenWifi::Types::StringVec & F) {
            for(const auto &[field,_]:FieldNames_)
                F.push_back(field);
        }

    protected:
        std::string                 TableName_;
        OpenWifi::DBType            Type_;
        Poco::Data::SessionPool     &Pool_;
        Poco::Logger                &Logger_;
        std::string                 Prefix_;
        DBCache<RecordType>         *Cache_= nullptr;
    private:
        std::string                 CreateFields_;
        std::string                 SelectFields_;
        std::string                 SelectList_;
        std::string                 UpdateFields_;
        std::vector<std::string>    IndexCreation_;
        std::map<std::string,int>   FieldNames_;
    };
}

