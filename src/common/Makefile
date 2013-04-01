OBJECTS = acceptor.o acceptorOB.o base_server.o cond.o connector.o epoll_engine.o epollclient.o \
	epollserver.o event.o file.o file_test.o inetaddr.o log.o log_test.o msg_looper.o msg_parser.o \
	msg_queue.o mutex.o rwlock.o sock.o socknotifier.o sockobserver.o sockstream.o sockstreamOB.o \
	stream_msg_packetizer.o thread.o thread_msg_recv.o thread_test.o
HEADERS = ./*.h
FILE_TEST_OBJS = file_test.o file.o 
LOG_TEST_OBJS = log_test.o log.o lock.o file.o
THREAD_TEST_OBJS = thread_test.o thread.o log.o lock.o file.o global.o
EPOLLSERVER_OBJS = acceptor.o acceptorOB.o base_server.o cond.o connector.o epoll_engine.o epollserver.o \
	event.o file.o inetaddr.o log.o msg_looper.o msg_parser.o msg_queue.o mutex.o rwlock.o sock.o \
	socknotifier.o sockobserver.o sockstream.o sockstreamOB.o stream_msg_packetizer.o thread.o thread_msg_recv.o
EPOLLCLIENT_OBJS = epollclient.o
BIN_DIR = ~/src/kunlun/bin
COMPILE = g++ 
CFLAGS = -Wall -pthread -g -D_DEBUG

file_test : $(FILE_TEST_OBJS)
	$(COMPILE) -o $(BIN_DIR)/file_test $(FILE_TEST_OBJS) $(CFLAGS)
$(FILE_TEST_OBJS) : $(HEADERS)

log_test : $(LOG_TEST_OBJS)
	$(COMPILE) -o $(BIN_DIR)/log_test $(LOG_TEST_OBJS) $(CFLAGS)
$(LOG_TEST_OBJS) : $(HEADERS)

thread_test : $(THREAD_TEST_OBJS)
	$(COMPILE) -o $(BIN_DIR)/thread_test $(THREAD_TEST_OBJS) $(CFLAGS)
$(THREAD_TEST_OBJS) : $(HEADERS)

epollserver : $(EPOLLSERVER_OBJS)
	$(COMPILE) -o $(BIN_DIR)/epollserver $(EPOLLSERVER_OBJS) $(CFLAGS)
$(EPOLLSERVER_OBJS) : $(HEADERS)

epollclient : $(EPOLLCLIENT_OBJS)
	$(COMPILE) -o $(BIN_DIR)/epollclient $(EPOLLCLIENT_OBJS) $(CFLAGS)
$(EPOLLCLIENT_OBJS) : $(HEADERS)

.PHONY : clean
clean : 
	-rm -rf $(OBJECTS)