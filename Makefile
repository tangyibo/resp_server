#
#
BINDIR = ./bin

S_EXES = resp_server
C_EXES = resp_client
LIB_NET = respnet

RM :=rm -f 

LIB_OBJS +=src/net/tcp_socket_addr.o \
	src/net/async_acceptor.o  \
	src/net/connection_handler.o \
	src/net/stream_buffer.o \
	src/net/event_dispatcher.o \
	src/net/service_loop.o  \
	src/net/service_loop_thread.o \
	src/net/thpool_service_loop.o \
	src/net/net_base.o 
	

S_OBJS +=	src/server/main.o \
	src/server/acceptor_server.o \
	src/server/service_handler.o \
	src/util/app.o \
	src/util/log4z.o \
	src/db/algo.o \
	src/db/kvdb.o \
	src/deps/libresp/resp.o 


C_OBJS +=src/client/main.o

CXXFLAGS = -g -Wall
CPPFLAGS = -I./include -I./src
LIBS =-L./lib -lpthread -lleveldb

all: libs server client

dir:
	if [ ! -d $(BINDIR) ]; then mkdir $(BINDIR) ; fi;

libs:  dir $(LIB_NET)
	
$(LIB_NET): $(LIB_OBJS)
	ar -cr $(BINDIR)/lib$@.a  $^

server:  $(S_OBJS)
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $(BINDIR)/$(S_EXES)  $(S_OBJS) -L$(BINDIR) -l$(LIB_NET) $(LIBS)

client:  
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $(BINDIR)/$(C_EXES)  src/client/main.cc -L$(BINDIR) -l$(LIB_NET) 
	
clean:
	$(RM) $(LIB_OBJS) $(S_OBJS) $(C_OBJS)
	$(RM) $(BINDIR)/*
#
#
