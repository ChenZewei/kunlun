#.SUFFIXES: .cpp .o
COMMON_OBJS = acceptor.o acceptorOB.o base_server.o cond.o connector.o epoll_engine.o base_client_test.o \
	base_server_test.o event.o file.o inetaddr.o log.o msg_looper.o msg_parser.o \
	msg_queue.o mutex.o rwlock.o sock.o socknotifier.o sockobserver.o sockstream.o sockstreamOB.o \
	stream_msg_packetizer.o thread.o thread_msg_recv.o timed_stream.o
BASE_SERVER_TEST_OBJS = acceptor.o acceptorOB.o base_server.o cond.o connector.o epoll_engine.o base_server_test.o \
	event.o file.o inetaddr.o log.o msg_looper.o msg_parser.o msg_queue.o mutex.o rwlock.o sock.o \
	socknotifier.o sockobserver.o sockstream.o sockstreamOB.o stream_msg_packetizer.o thread.o thread_msg_recv.o \
	timed_stream.o
BASE_CLIENT_TEST_OBJS = base_client_test.o
BIN_DIR = ~/src/kunlun/bin
COMPILE = g++ -pthread -g -D_DEBUG

all : $(EPOLLSERVER_OBJS) $(EPOLLCLIENT_OBJS) base_server_test base_client_test

.o:
	$(COMPILE) -o $@ $<

.cpp:
	$(COMPILE) -o $@ $<

.cpp.o:
	$(COMPILE) -c -o $@ $<

base_server_test : $(BASE_SERVER_TEST_OBJS)
	$(COMPILE) -o $(BIN_DIR)/base_server_test $(BASE_SERVER_TEST_OBJS)

base_client_test : $(BASE_CLIENT_TEST_OBJS)
	$(COMPILE) -o $(BIN_DIR)/base_client_test $(BASE_CLIENT_TEST_OBJS)

.PHONY : clean
clean : 
	-rm -rf $(COMMON_OBJS)