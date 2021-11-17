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
        FT_BLOB
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
        std::string     FieldName;
        Indextype   Type;
    };
    typedef std::vector<IndexEntry>     IndexEntryVec;

    struct Index {
        std::string         Name;
        IndexEntryVec   Entries;
    };
    typedef std::vector<Index>      IndexVec;

    inline std::string FieldTypeToChar(OpenWifi::DBType Type, FieldType T, int Size=0) {
        switch(T) {
            case FT_INT:    return "INT";
            case FT_BIGINT: return "BIGINT";
            case FT_TEXT:   return "TEXT";
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
                    else if(Type==OpenWifi::DBType::sqlite)
                        return "BLOB";
                    default:
                        assert(false);
                        return "";
        }
        assert(false);
        return "";
    }

    inline std::string Escape(const std::string &S) {
        std::string R;

        for(const auto &i:S)
            if(i=='\'')
                R += "''";
            else
                R += i;
            return R;
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

    inline std::string to_string(const char * S) {
        return S;
    }

    template <typename RecordTuple, typename RecordType> class DB {
    public:
        DB( OpenWifi::DBType dbtype,
            const char *TableName,
            const FieldVec & Fields,
            const IndexVec & Indexes,
            Poco::Data::SessionPool & Pool,
                Poco::Logger &L,
                const char *Prefix):
                Type(dbtype),
                DBName(TableName),
                Pool_(Pool),
                Logger_(L),
                Prefix_(Prefix)
        {
            bool first = true;
            int  Place=0;

            assert( RecordTuple::length == Fields.size());

            for(const auto &i:Fields) {

                FieldNames_[i.Name] = Place;
                if(!first) {
                    CreateFields_ += ", ";
                    SelectFields_ += ", ";
                    UpdateFields_ += ", ";
                    SelectList_ += ", ";
                } else {
                    SelectList_ += "(";
                }

                CreateFields_ += i.Name + " " + FieldTypeToChar(Type, i.Type,i.Size) + (i.Index ? " unique primary key" : "");
                SelectFields_ += i.Name ;
                UpdateFields_ += i.Name + "=?";
                SelectList_ += "?";
                first = false;
                Place++;
            }
            SelectList_ += ")";

            if(!Indexes.empty()) {
                if(Type==OpenWifi::DBType::sqlite || Type==OpenWifi::DBType::pgsql) {
                    for(const auto &j:Indexes) {
                        std::string IndexLine;

                        IndexLine = std::string("CREATE INDEX IF NOT EXISTS ") + j.Name + std::string(" ON ") + DBName + " (";
                        bool first_entry=true;
                        for(const auto &k:j.Entries) {
                            assert(FieldNames_.find(k.FieldName) != FieldNames_.end());
                            if(!first_entry) {
                                IndexLine += " , ";
                            }
                            first_entry = false;
                            IndexLine += k.FieldName + std::string(" ") + std::string(k.Type == Indextype::ASC ? "ASC" : "DESC") ;
                        }
                        IndexLine += " );";
                        IndexCreation += IndexLine;
                    }
                } else if(Type==OpenWifi::DBType::mysql) {
                    bool firstIndex = true;
                    std::string IndexLine;
                    for(const auto &j:Indexes) {
                        if(!firstIndex)
                            IndexLine += ", ";
                        firstIndex = false;
                        IndexLine += " INDEX " + j.Name + " ( " ;
                        bool first_entry=true;
                        for(const auto &k:j.Entries) {
                            assert(FieldNames_.find(k.FieldName) != FieldNames_.end());
                            if(!first_entry) {
                                IndexLine += " ,";
                            }
                            first_entry = false;
                            IndexLine += k.FieldName + std::string(k.Type == Indextype::ASC ? " ASC" : " DESC");
                        }
                        IndexLine += " ) ";
                    }
                    IndexCreation = IndexLine;
                }
            }
        }

        [[nodiscard]] const std::string & CreateFields() const { return CreateFields_; };
        [[nodiscard]] const std::string & SelectFields() const { return SelectFields_; };
        [[nodiscard]] const std::string & SelectList() const { return SelectList_; };
        [[nodiscard]] const std::string & UpdateFields() const { return UpdateFields_; };

        inline std::string OP(const char *F, SqlComparison O , int V) {
            assert( FieldNames_.find(F) != FieldNames_.end() );
            return std::string{"("} + F + SQLCOMPS[O] + std::to_string(V) + ")" ;
        }

        inline std::string OP(const char *F, SqlComparison O , uint64_t V) {
            assert( FieldNames_.find(F) != FieldNames_.end() );
            return std::string{"("} + F + SQLCOMPS[O] + std::to_string(V) + ")" ;
        }

        std::string OP(const char *F, SqlComparison O , const std::string & V) {
            assert( FieldNames_.find(F) != FieldNames_.end() );
            return std::string{"("} + F + SQLCOMPS[O] + "'" + Escape(V) + "')" ;
        }

        std::string OP(const char *F, SqlComparison O , const char * V) {
            assert( FieldNames_.find(F) != FieldNames_.end() );
            return std::string{"("} + F + SQLCOMPS[O] + "'" + Escape(V) + "')" ;
        }

        static std::string OP( const std::string &P1, SqlBinaryOp BOP , const std::string &P2) {
            return std::string("(")+P1 + BOPS[BOP] + P2 +")";
        }

        std::string OP( bool Paran, const std::string &P1, SqlBinaryOp BOP , const std::string &P2) {
            return P1 + BOPS[BOP] + P2 +")";
        }

        template <typename... Others> std::string OP( bool ParanOpen, const std::string &P1, SqlBinaryOp BOP , const std::string &P2, Others... More) {
            return P1 + BOPS[BOP] + OP(ParanOpen, P2, More...) + ")";
        }

        template <typename... Others> std::string OP( const std::string &P1, SqlBinaryOp BOP , const std::string &P2, Others... More) {
            return std::string{"("} + P1 + BOPS[BOP] + OP(true, P2, More...);
        }

        inline bool  Create() {
            std::string S;

            if(Type==OpenWifi::DBType::mysql) {
                if(IndexCreation.empty())
                    S = "create table if not exists " + DBName +" ( " + CreateFields_ + " )" ;
                else
                    S = "create table if not exists " + DBName +" ( " + CreateFields_ + " ), " + IndexCreation + " )";
            } else if (Type==OpenWifi::DBType::pgsql || Type==OpenWifi::DBType::sqlite) {
                S = "create table if not exists " + DBName + " ( " + CreateFields_ + " ); " + IndexCreation ;
            }

            // std::cout << "CREATE-DB: " << S << std::endl;

            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   CreateStatement(Session);

                CreateStatement << S;
                CreateStatement.execute();
                return true;
            } catch (const Poco::Exception &E) {
                std::cout << "Exception while creating DB: " << E.name() << std::endl;
            }
            return false;
        }

        [[nodiscard]] std::string ConvertParams(const std::string & S) const {
            if(Type!=OpenWifi::DBType::pgsql)
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

        void Convert( RecordTuple &in , RecordType &out);
        void Convert( RecordType &in , RecordTuple &out);

        inline const std::string & Prefix() { return Prefix_; };

        bool CreateRecord( RecordType & R) {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Insert(Session);

                RecordTuple RT;
                Convert(R, RT);
                std::string St = "insert into  " + DBName + " ( " + SelectFields_ + " ) values " + SelectList_;
                Insert  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(RT);
                Insert.execute();
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template<typename T> bool GetRecord( const char * FieldName, T Value,  RecordType & R) {
            try {

                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple             RT;

                std::string St = "select " + SelectFields_ + " from " + DBName + " where " + FieldName + "=?" ;

                Select  << ConvertParams(St) ,
                    Poco::Data::Keywords::into(RT),
                    Poco::Data::Keywords::use(Value);
                if(Select.execute()==1) {
                    Convert(RT,R);
                    return true;
                }
                return false;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        typedef std::vector<std::string> StringVec;

        template <  typename T,
                typename T0, typename T1> bool GR(const char *FieldName, T & R,T0 &V0, T1 &V1) {
            try {

                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple RT;

                std::string St = "select " + SelectFields_ + " from " + DBName
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

        typedef std::vector<RecordTuple> RecordList;

        bool GetRecords( uint64_t Offset, uint64_t HowMany, std::vector<RecordType> & Records, const std::string & Where = "", const std::string & OrderBy = "") {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordList RL;
                std::string St = "select " + SelectFields_ + " from " + DBName +
                        (Where.empty() ? "" : " where " + Where) + OrderBy +
                        ComputeRange(Offset, HowMany) ;

                Select  << St ,
                    Poco::Data::Keywords::into(RL);

                if(Select.execute()>0) {
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

        template <typename T> bool UpdateRecord( const char *FieldName, T & Value,  RecordType & R) {
            try {
                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Update(Session);

                RecordTuple RT;

                Convert(R, RT);

                std::string St = "update " + DBName + " set " + UpdateFields_ + " where " + FieldName + "=?" ;
                Update  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(RT),
                    Poco::Data::Keywords::use(Value);
                Update.execute();
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool GetNameAndDescription(const char *FieldName, T & Value, std::string & Name, std::string & Description ) {
            try {
                assert( FieldNames_.find(FieldName) != FieldNames_.end() );
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple             RT;

                std::string St = "select " + SelectFields_ + " from " + DBName + " where " + FieldName + "=?" ;
                RecordType R;
                Select  << ConvertParams(St) ,
                Poco::Data::Keywords::into(RT),
                Poco::Data::Keywords::use(Value);
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

        template <typename T> bool DeleteRecord( const char *FieldName, T Value) {
            try {
                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Delete(Session);

                std::string St = "delete from " + DBName + " where " + FieldName + "=?" ;
                Delete  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(Value);
                Delete.execute();
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

                std::string St = "delete from " + DBName + " where " + WhereClause;
                Delete  << St;
                Delete.execute();
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool Exists(const char *FieldName, std::string & Value) {
            try {
                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                RecordType  R;
                if(GetRecord(FieldName,Value,R))
                    return true;
                return false;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool Iterate( std::function<bool(const RecordType &R)> F) {
            try {

                uint64_t    Offset=1;
                uint64_t    Batch=50;
                bool Done=false;
                while(!Done) {
                    std::vector<RecordType> Records;
                    if(GetRecords(Offset,Batch,Records)) {
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
                auto hint = FieldNames_.find(T[0]);
                if(hint==FieldNames_.end()) {
                    return false;
                }
                ItemList += T[0] + (T[1]=="a" ? " ASC" : " DESC");
            }

            if(!ItemList.empty()) {
                OrderByString = " ORDER BY " + ItemList;
            }

            std::cout << OrderByString << std::endl;

            return true;
        }

        uint64_t Count( const std::string & Where="" ) {
            try {
                uint64_t Cnt=0;

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);

                std::string st{"SELECT COUNT(*) FROM " + DBName + " " + (Where.empty() ? "" : (" where " + Where)) };

                Select << st ,
                    Poco::Data::Keywords::into(Cnt);
                Select.execute();

                return Cnt;

            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return 0;
        }

        template <typename X> bool ManipulateVectorMember( X T, const char *FieldName, std::string & ParentUUID, std::string & ChildUUID, bool Add) {
            try {
                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

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

        inline bool AddChild( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::children, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteChild( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::children, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddLocation( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::locations, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteLocation( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::locations, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddContact( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::contacts, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteContact( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::contacts, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddVenue( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::venues, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteVenue( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::venues, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddDevice( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::devices, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteDevice( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::devices, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddEntity( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::entities, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteEntity( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::entities, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddUser( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::users, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DelUser( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::users, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddInUse(const char *FieldName, std::string & ParentUUID, const std::string & Prefix, const std::string & ChildUUID) {
            std::string FakeUUID{ Prefix + ":" + ChildUUID};
            return ManipulateVectorMember(&RecordType::inUse,FieldName, ParentUUID, FakeUUID, true);
        }

        inline bool DeleteInUse(const char *FieldName, std::string & ParentUUID, const std::string & Prefix, const std::string & ChildUUID) {
            std::string FakeUUID{ Prefix + ":" + ChildUUID};
            return ManipulateVectorMember(&RecordType::inUse,FieldName, ParentUUID, FakeUUID, false);
        }

        inline bool DeleteContact(const char *FieldName, std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::contacts,FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddContact(const char *FieldName, std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::contacts,FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteLocation(const char *FieldName, std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::locations,FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddLocation(const char *FieldName, std::string & ParentUUID, const std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::locations,FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool GetInUse(const char *FieldName, std::string & UUID, std::vector<std::string> & UUIDs ) {
            RecordType  R;
            if(GetRecord(FieldName,UUID,R)) {
                UUIDs = R.inUse;
                return true;
            }
            return false;
        }

        [[nodiscard]] inline std::string ComputeRange(uint64_t From, uint64_t HowMany) {
            if(From<1) From=1;
            switch(Type) {
                case OpenWifi::DBType::sqlite:
                    return " LIMIT " + std::to_string(From-1) + ", " + std::to_string(HowMany) +  " ";
                case OpenWifi::DBType::pgsql:
                    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
                case OpenWifi::DBType::mysql:
                    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
                default:
                    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
            }
        }

        Poco::Logger & Logger() { return Logger_; }

    private:
        OpenWifi::DBType            Type;
        std::string                 DBName;
        std::string                 CreateFields_;
        std::string                 SelectFields_;
        std::string                 SelectList_;
        std::string                 UpdateFields_;
        std::string                 IndexCreation;
        std::map<std::string,int>   FieldNames_;
        Poco::Data::SessionPool     &Pool_;
        Poco::Logger                &Logger_;
        std::string                 Prefix_;
    };
}

