#include "db/kvdb.h"
#include <iostream>
#include <assert.h>
#include "db/algo.h"
#include "db/serialize.h"
#include <string>

using std::string;

namespace DataType
{
    char KV		= 'k'; // key-value string
    char SET		= 's'; //  set value
}

template<> kvdb* singleton<kvdb>::_single_instance=NULL;
/////////////////////////////////////////////////////////////////////

kvdb::kvdb( )
{
    options_.create_if_missing = true;
}

kvdb::~kvdb( )
{
    close();
}

bool kvdb::open( const std::string dbpath )
{
    leveldb::Status status = leveldb::DB::Open( options_, dbpath, &db_ );
    if ( !status.ok( ) )
    {
        std::cerr << "Open db failed,error:" << status.ToString( ) <<std::endl;
        return false;
    }

    return true;
}

void kvdb::close()
{
    if(NULL!=db_)
    {
        delete db_;
        db_=NULL;
    }
}


//strings

void kvdb::set(const std::string key,const  std::string value, leveldb::Status &s)
{
    string temp=value;
    OutStream os;
    os<<DataType::KV;
    os<<temp;
    s = db_->Put( leveldb::WriteOptions( ), key, os.str() );
}

void kvdb::get(const std::string key,std::string &value,leveldb::Status &s)
{
    string content;
    s = db_->Get( leveldb::ReadOptions( ), key, &content );
    
    char type='\0';
    InStream is( content );
    is>>type;
    
    if(type == DataType::KV)
    {
        is>>value;
    }
    else
    {
        s=leveldb::Status::NotFound( "WRONGTYPE Operation against a key holding the wrong kind of value" );
    }
}

 //set

void kvdb::sadd(const std::string key,const std::string &value,leveldb::Status &s)
{
    string content;
    s = db_->Get( leveldb::ReadOptions( ), key, &content );

    std::set<std::string> dbset;
    
    if ( content.empty( ) )
    {
        dbset.insert(value);
        
        OutStream os;
        os << DataType::SET << dbset;
        s = db_->Put( leveldb::WriteOptions( ), key, os.str( ) );
    }
    else
    {
        char type='\0';
        InStream is( content );
        is>>type;

        if ( type == DataType::SET )
        {
            is>>dbset;
            dbset.insert( value );
            OutStream os;
            os << DataType::SET << dbset;
            s = db_->Put( leveldb::WriteOptions( ), key, os.str( ) );
        }
        else
        {
            s=leveldb::Status::NotFound( "WRONGTYPE Operation against a key holding the wrong kind of value" );
        }

    }
}

void kvdb::srem(const std::string key,const std::string &value,leveldb::Status &s)
{
    string content;
    s = db_->Get( leveldb::ReadOptions( ), key, &content );

    std::set<std::string> dbset;
    
    if ( content.empty( ) )
    {
        return;
    }
    else
    {
        char type='\0';
        InStream is( content );
        is>>type;

        if ( type == DataType::SET )
        {
            is>>dbset;

            std::set<std::string>::iterator it = dbset.find( value );
            if ( it == dbset.end( ) )
                return;

            dbset.erase( it );

            OutStream os;
            os << DataType::SET << dbset;
            s = db_->Put( leveldb::WriteOptions( ), key, os.str( ) );
        }
        else
        {
            s=leveldb::Status::NotFound( "WRONGTYPE Operation against a key holding the wrong kind of value" );
        }

    }  
}

void kvdb::smember(const std::string key,std::vector<std::string> &ret,leveldb::Status &s)
{
    string content;
    s = db_->Get( leveldb::ReadOptions( ), key, &content );

    std::set<std::string> dbset;
    
    if ( content.empty( ) )
    {
        return;
    }
    else
    {
        char type='\0';
        InStream is( content );
        is>>type;

        if ( type == DataType::SET )
        {
            is>>dbset;

            std::set<std::string>::iterator it;
            for ( it=dbset.begin( ); it != dbset.end( ); ++it )
            {
                ret.push_back( *it );
            }
        }
        else
        {
            s=leveldb::Status::NotFound( "WRONGTYPE Operation against a key holding the wrong kind of value" );
        }

    }
}

void kvdb::scard(const std::string key,int &ret,leveldb::Status &s)
{
    string content;
    s = db_->Get( leveldb::ReadOptions( ), key, &content );

    std::set<std::string> dbset;
    
    if ( content.empty( ) )
    {
        ret=0;
        return;
    }
    else
    {
        char type='\0';
        InStream is( content );
        is>>type;

        if ( type == DataType::SET )
        {
            is>>dbset;

            ret=(int)dbset.size();
        }
        else
        {
            s=leveldb::Status::NotFound( "WRONGTYPE Operation against a key holding the wrong kind of value" );
        }

    }   
}

//common

 void kvdb::del(const std::string key,leveldb::Status &s)
 {
     s = db_->Delete(leveldb::WriteOptions(), key);
 }
 
 void kvdb::keys(const std::string pattern,std::vector<std::string> &ret,leveldb::Status &s)
{
    leveldb::Iterator* it = db_->NewIterator( leveldb::ReadOptions( ) );
    for ( it->SeekToFirst( ); it->Valid( ); it->Next( ) )
    {
        if(algo::MatchingString(it->key( ).ToString( ).c_str(),pattern.c_str()))
            ret.push_back(it->key( ).ToString( ) );
    }
    assert( it->status( ).ok( ) ); 
    delete it;
 }