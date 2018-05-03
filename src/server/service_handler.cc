#include "service_handler.h"
#include "acceptor_server.h"
#include <iostream>
#define LOG4Z_FORMAT_INPUT_ENABLE
#include "util/log4z.h"
#include "deps/libresp/resp.h"
#include "db/kvdb.h"
#include <algorithm>  

void service_handler::on_connect(const SocketAddress &remote)
{
    LOGFMT_INFO(LOG4Z_MAIN_LOGGER_ID, "Connect:%s",remote.to_string().c_str());
}

int service_handler::on_read(const input_stream_buffer &input)
{
    std::string msg = input.to_string();
    int org_msg_bytes = msg.size();
    int handle_offset = 0;
        
    LOGFMT_INFO(LOG4Z_MAIN_LOGGER_ID, "Recv:%s", msg.c_str());

    bool find_header=false;
    if ('*' == msg[0])
    {
        find_header = true;
    }
    else
    {
        std::string::size_type pos = msg.find("*");
        if (pos != std::string::npos)
        {
            msg = msg.substr(pos);
            handle_offset += pos;

            find_header = true;
        }
    }

    if (find_header)
    {
        int new_msg_bytes = msg.size();
        int offset = 0;
        int last_offset=0;

        do
        {
            respObject *r = NULL;
            if(offset>=new_msg_bytes)
                break;
            
            offset = respDecode(&r, (unsigned char*) msg.c_str() + offset);
            if (offset > 0 && offset <= new_msg_bytes)
            {
                std::vector<std::string> commands;
                parse_command(r, commands);
                handle_command(commands);
                last_offset = offset;
            }
        }
        while (offset > 0 && offset < new_msg_bytes);
        
        handle_offset+=last_offset;
    }

    return org_msg_bytes - handle_offset;
}

void service_handler::on_close()
{
    acceptor_->remove_handler(this);
    LOGFMT_INFO(LOG4Z_MAIN_LOGGER_ID, "Close...:");
}

void service_handler::parse_command(respObject *resp, std::vector<std::string> &commands)
{
    std::string temp;

    switch (resp->type)
    {
        case RESP_OBJECT_ARRAY:
        {
            for (unsigned int i = 0; i < resp->elements; ++i)
            {
                respObject *item = resp->element[i];
                switch (item->type)
                {
                    case RESP_OBJECT_BINARY:
                    {
                        temp.assign((char *) item->str, item->len);
                        temp.append("\0");
                        commands.push_back(temp);
                        break;
                    }
                    case RESP_OBJECT_ARRAY:
                    case RESP_OBJECT_INTEGER:
                    case RESP_OBJECT_STATUS:
                    case RESP_OBJECT_ERROR:
                        break;
                }
            }
            break;
        }
        case RESP_OBJECT_BINARY:
        case RESP_OBJECT_INTEGER:
        case RESP_OBJECT_STATUS:
        case RESP_OBJECT_ERROR:
        default:
            break;
    }
}

void service_handler::handle_command(const std::vector<std::string> commands)
{
    unsigned char buf[64 * 1024];
    leveldb::Status s;
    respObject *r = NULL;
    
    if(commands.size()<1)
        return;
    
    std::string cmd=commands[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);  
    
    if ("set" == cmd)
    {
        if (commands.size() >= 3)
        {
            kvdb::instance()->set(commands[1], commands[2], s);
            r = createRespString(RESP_OBJECT_STATUS, s.ok() ? (unsigned char *) "OK" : (unsigned char *) s.ToString().c_str());
        }
        else
        {
            r = createRespString(RESP_OBJECT_ERROR, (unsigned char*) "no key or value\0");
        }
    }
    else if ("get" ==cmd)
    {
        if (commands.size() >= 2)
        {
            std::string value;
            kvdb::instance()->get(commands[1], value, s);

            if (s.ok())
            {
                if (!value.empty())
                    r = createRespBulk((unsigned char *) value.c_str(), value.size());
                else
                    r = createRespNil();
            }
            else
            {
                r = createRespString(RESP_OBJECT_ERROR, (unsigned char *) s.ToString().c_str());
            }
        }
        else
        {
            r = createRespString(RESP_OBJECT_ERROR, (unsigned char*) "no key\0");
        }
    }
    else if ("sadd" == cmd)
    {
        if (commands.size() >= 3)
        {
            kvdb::instance()->sadd(commands[1], commands[2], s);
            r = createRespString(RESP_OBJECT_STATUS, s.ok() ? (unsigned char *) "OK" : (unsigned char *) s.ToString().c_str());
        }
        else
        {
            r = createRespString(RESP_OBJECT_ERROR, (unsigned char*) "no key or value\0");
        }
    }
    else if ("srem" == cmd)
    {
        if (commands.size() >= 3)
        {
            kvdb::instance()->srem(commands[1], commands[2], s);
            r = createRespString(RESP_OBJECT_STATUS, s.ok() ? (unsigned char *) "OK" : (unsigned char *) s.ToString().c_str());
        }
        else
        {
            r = createRespString(RESP_OBJECT_ERROR, (unsigned char*) "no key or value\0");
        }
    }
    else if ("smember" == cmd)
    {
        if (commands.size() >= 2)
        {
            std::vector<std::string> strlist;
            kvdb::instance()->smember(commands[1], strlist, s);
            if (strlist.size() > 0)
            {
                r = createRespArray(strlist.size());
                for (size_t k = 0; k < strlist.size(); ++k)
                {
                    r->element[k] = createRespBulk((unsigned char *) strlist[k].c_str(), strlist[k].size());
                }
            }
            else
            {
                r = createRespNil();
            }
        }
        else
        {
            r = createRespString(RESP_OBJECT_ERROR, (unsigned char*) "no key\0");
        }
    }
    else if ("scard" == cmd)
    {
        if (commands.size() >= 2)
        {
            int count = 0;
            kvdb::instance()->scard(commands[1], count, s);
            if (s.ok())
            {
                r = createRespInteger(count);
            }
            else
            {
                r = createRespString(RESP_OBJECT_ERROR, (unsigned char*) s.ToString().c_str());
            }
        }
        else
        {
            r = createRespString(RESP_OBJECT_ERROR, (unsigned char*) "no key\0");
        }
    }
    else if ("del" == cmd)
    {
        if (commands.size() >= 2)
        {
            for (size_t i = 1; i < commands.size(); ++i)
                kvdb::instance()->del(commands[i], s);

            r = createRespString(RESP_OBJECT_STATUS, s.ok() ? (unsigned char *) "OK" : (unsigned char *) s.ToString().c_str());
        }
        else
        {
            r = createRespString(RESP_OBJECT_ERROR, (unsigned char*) "no key\0");
        }
    }
    else if ("keys" == cmd)
    {
        if (commands.size() >= 2)
        {
            std::vector<std::string> strlist;
            kvdb::instance()->keys(commands[1], strlist, s);
            if (strlist.size() > 0)
            {
                r = createRespArray(strlist.size());
                for (size_t k = 0; k < strlist.size(); ++k)
                {
                    r->element[k] = createRespBulk((unsigned char *) strlist[k].c_str(), strlist[k].size());
                }
            }
            else
            {
                r = createRespNil();
            }
        }
        else
        {
            r = createRespString(RESP_OBJECT_ERROR, (unsigned char*) "no patten\0");
        }
    }
    else
    {
        s = leveldb::Status::NotFound("unsurpported");
        r = createRespString(RESP_OBJECT_STATUS, (unsigned char *) s.ToString().c_str());
    }

    int bytes = respEncode(r, buf);
    freeRespObject(r);

    if (bytes > 0)
    {
        send((const char*) buf, bytes);
    }
}