#ifndef IMPL_LEVELDB_H
#define IMPL_LEVELDB_H
#include "leveldb/db.h"
#include "base/singleton.h"
#include <string>
#include <vector>

class kvdb:public singleton<kvdb>
{
public:
        kvdb ();
        virtual ~ kvdb ();
        
        bool open(const std::string path);
        void close();
        
        //strings
        void set(const std::string key,const  std::string value, leveldb::Status &s);
        void get(const std::string key,std::string &value,leveldb::Status &s);
        
        //set
        void sadd(const std::string key,const std::string &value,leveldb::Status &s);
        void srem(const std::string key,const std::string &value,leveldb::Status &s);
        void smember(const std::string key,std::vector<std::string> &ret,leveldb::Status &s);
        void scard(const std::string key,int &ret,leveldb::Status &s);
      
        //common
        void del(const std::string key,leveldb::Status &s);
        void keys(const std::string pattern,std::vector<std::string> &ret,leveldb::Status &s);
        
        
private:
        leveldb::DB* db_;
        leveldb::Options options_;
} ;

#endif /* LEVELDB_H */

