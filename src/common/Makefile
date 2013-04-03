#.SUFFIXES: .cpp .o
ALL_OBJS = acceptor.o acceptorOB.o base_server.o cond.o connector.o epoll_engine.o epollclient.o \
	epollserver.o event.o file.o file_test.o inetaddr.o log.o log_test.o msg_looper.o msg_parser.o \
	msg_queue.o mutex.o rwlock.o sock.o socknotifier.o sockobserver.o sockstream.o sockstreamOB.o \
	stream_msg_packetizer.o thread.o thread_msg_recv.o thread_test.o
EPOLLSERVER_OBJS = acceptor.o acceptorOB.o base_server.o cond.o connector.o epoll_engine.o epollserver.o \
	event.o file.o inetaddr.o log.o msg_looper.o msg_parser.o msg_queue.o mutex.o rwlock.o sock.o \
	socknotifier.o sockobserver.o sockstream.o sockstreamOB.o stream_msg_packetizer.o thread.o thread_msg_recv.o
EPOLLCLIENT_OBJS = epollclient.o
BIN_DIR = ~/src/kunlun/bin
COMPILE = g++ -pthread -g -D_DEBUG

all : $(EPOLLSERVER_OBJS) $(EPOLLCLIENT_OBJS) epollserver epollclient

.o:
	$(COMPILE) -o $@ $<

.cpp:
	$(COMPILE) -o $@ $<

.cpp.o:
	$(COMPILE) -c -o $@ $<

epollserver : $(EPOLLSERVER_OBJS)
	$(COMPILE) -o $(BIN_DIR)/epollserver $(EPOLLSERVER_OBJS)

epollclient : $(EPOLLCLIENT_OBJS)
	$(COMPILE) -o $(BIN_DIR)/epollclient $(EPOLLCLIENT_OBJS)

.PHONY : clean
clean : 
	-rm -rf $(ALL_OBJS)