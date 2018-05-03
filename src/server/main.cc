#include "net/tcp_socket_addr.h"
#include "net/service_loop.h"
#include "acceptor_server.h"
#include "service_handler.h"
#include "util/app.h"
#define LOG4Z_FORMAT_INPUT_ENABLE
#include "util/log4z.h"
#include "util/config.h"
#include "db/kvdb.h"
#include <signal.h>

#define APP_NAME "rtdbserver"
#define APP_VERSION "v1.0.1"

using namespace zsummer::log4z;

static ServiceLoop loop;
static acceptor_server acceptor(&loop);
  
class MyApp : public Application
{
public:

    static void signal_handler(int sig)
    {
        switch (sig)
        {
            case SIGTERM:
            case SIGINT:
            {
                acceptor.stop_thread_pool();
                loop.quit();
                LOGFMT_INFO(LOG4Z_MAIN_LOGGER_ID, "Stop server ...");
                break;
            }
            default:
                break;
        }
    }
    
    virtual void usage(int argc, char **argv)
    {
        printf("Usage:\n");
        printf("    %s [-d] /path/to/config.ini [-s start|stop|restart]\n", argv[0]);
        printf("Options:\n");
        printf("    -d    run as daemon\n");
        printf("    -s    option to start|stop|restart the server\n");
        printf("    -h    show this message\n");
    }

    virtual void welcome()
    {
        fprintf(stderr, "%s %s\n", APP_NAME, APP_VERSION);
        fprintf(stderr, "Copyright (c) 2012-2018 test\n");
        fprintf(stderr, "\n");
    }

    virtual void run()
    {
        std::string logconf=m_ptr_conf->ReadProfileString("Application", "logconf", "");
        ILog4zManager::getRef().config(logconf.c_str());
        ILog4zManager::getRef().start();
        
        LOGFMT_INFO(LOG4Z_MAIN_LOGGER_ID, "Start server ...");

        signal(SIGINT, &MyApp::signal_handler);
        signal(SIGTERM, &MyApp::signal_handler);
        signal(SIGALRM, &MyApp::signal_handler);

        std::string dbfilepath=m_ptr_conf->ReadProfileString("Application", "dbfilepath", "");
        if( !kvdb::instance()->open(dbfilepath))
        {
            LOGFMT_ERROR(LOG4Z_MAIN_LOGGER_ID, "Failed to prepare kvdb files!");
            return;
        }
        
        SocketAddress endpoint(NULL, 6543);
        acceptor.start_thread_pool(4);
        if (0 == acceptor.open(&endpoint))
        {
            loop.run();
        }

    }
 };

int main(int argc, char *argv[])
{
    MyApp app;
    return app.main(argc, argv);
}