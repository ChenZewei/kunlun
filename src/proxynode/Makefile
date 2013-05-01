INC_PATH = -I../common -I/usr/local/include
LIB_PATH = -L/usr/local/lib $(LIBS)

COMMON_OBJS = ../common/acceptor.o ../common/acceptorOB.o ../common/base_server.o \
	../common/cond.o ../common/connector.o ../common/epoll_engine.o ../common/event.o \
	../common/file.o ../common/inetaddr.o ../common/log.o ../common/msg_looper.o \
	../common/msg_parser.o ../common/msg_queue.o ../common/mutex.o ../common/rwlock.o \
	../common/sock.o ../common/socknotifier.o ../common/sockobserver.o \
	../common/sockstream.o ../common/sockstreamOB.o ../common/stream_msg_packetizer.o \
	../common/thread.o ../common/thread_msg_recv.o ../common/timed_stream.o
PROXYNODE_OBJS = node_container.o node_info.o proxy_global.o proxy_msg_parser.o proxy_server.o
PROGRAM = kunlun_proxyd
BIN_DIR = ~/src/kunlun/bin
COMPILE = g++ -pthread -g -D_DEBUG

all : $(COMMON_OBJS) $(PROXYNODE_OBJS) $(PROGRAM)
.o :
	$(COMPILE) -o $@ $< $(COMMON_OBJS) $(PROXYNODE_OBJS) $(LIB_PATH) $(INC_PATH)

.cpp :
	$(COMPILE) -o $@ $< $(COMMON_OBJS) $(PROXYNODE_OBJS) $(LIB_PATH) $(INC_PATH)

.cpp.o :
	$(COMPILE) -c -o $@ $< $(INC_PATH)

.PHONY : install
install : 
	-cp -f $(PROGRAM) $(BIN_DIR)/
.PHONY : clean
clean : 
	-rm -rf $(COMMON_OBJS) $(PROXYNODE_OBJS) $(PROGRAM)
