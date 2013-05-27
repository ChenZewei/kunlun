INC_PATH = -I../common -I../proxynode -I/usr/local/include
LIB_PATH = -L/usr/local/lib $(LIBS)

COMMON_OBJS = ../common/acceptor.o ../common/acceptorOB.o ../common/base_server.o \
	../common/cond.o ../common/connector.o ../common/epoll_engine.o ../common/event.o \
	../common/file.o ../common/inetaddr.o ../common/log.o ../common/msg_looper.o \
	../common/msg_parser.o ../common/msg_queue.o ../common/mutex.o ../common/rwlock.o \
	../common/sock.o ../common/socknotifier.o ../common/sockobserver.o \
	../common/sockstream.o ../common/sockstreamOB.o ../common/stream_msg_packetizer.o \
	../common/thread.o ../common/thread_msg_recv.o ../common/timed_stream.o \
	../common/directory.o ../common/file_reader_stream.o ../common/file_writer_stream.o \
	../common/hash.o ../common/base_conf_parser.o
PROXYNODE_OBJS = node_container.o node_info.o proxy_global.o proxy_msg_parser.o proxy_server.o \
	vnode_balancer.o proxy_conf_parser.o
PROGRAM = kunlun_proxyd proxy_conf_parser_test
BIN_DIR = ~/src/kunlun/bin
COMPILE = g++ -pthread -g -D_DEBUG

all : $(COMMON_OBJS) $(PROXYNODE_OBJS) $(SQLITE_OBJS) $(PROGRAM)
.o :
	$(COMPILE) -o $@ $< $(COMMON_OBJS) $(PROXYNODE_OBJS) $(LIB_PATH) $(INC_PATH)

.cpp :
	$(COMPILE) -o $@ $< $(COMMON_OBJS) $(PROXYNODE_OBJS) $(LIB_PATH) $(INC_PATH)

.cpp.o :
	$(COMPILE) -c -o $@ $< $(INC_PATH) $(CFLAGS)

.PHONY : install
install : 
	-cp -f $(PROGRAM) $(BIN_DIR)/
.PHONY : clean
clean : 
	-rm -rf $(COMMON_OBJS) $(PROXYNODE_OBJS) $(PROGRAM)
