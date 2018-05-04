#ifndef SERVICE_HANDLER_H
#define SERVICE_HANDLER_H
#include "net/tcp_socket_addr.h"
#include "net/connection_handler.h"
#include <vector>

class acceptor_server;
struct respObject;

/*
 * 请求处理
 */
class service_handler : public ConnectionHandler
{
public:
    void set_acceptor(acceptor_server* acceptor)  {     acceptor_ = acceptor;   }

    virtual void on_connect(const SocketAddress &remote);

    virtual int on_read(const input_stream_buffer &input);

    virtual void on_close ( );

protected:
    void parse_command ( respObject *r, std::vector<std::string> &commands );
    void handle_command ( const std::vector<std::string> commands );

private:
    acceptor_server *acceptor_;
};

#endif /* SERVICE_HANDLER_H */