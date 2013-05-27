#.SUFFIXES: .cpp .o
INC_PATH = -I/usr/local/include
LIB_PATH = -L/usr/local/lib $(LIBS)
COMMON_OBJS = acceptor.o acceptorOB.o base_server.o cond.o connector.o epoll_engine.o event.o \
	file.o inetaddr.o log.o msg_looper.o msg_parser.o msg_queue.o mutex.o rwlock.o sock.o \
	socknotifier.o sockobserver.o sockstream.o sockstreamOB.o stream_msg_packetizer.o thread.o \
	thread_msg_recv.o timed_stream.o directory.o hash.o file_reader_stream.o file_writer_stream.o \
	base_conf_parser.o
#BASE_SERVER_TEST_OBJS = acceptor.o acceptorOB.o base_server.o cond.o connector.o epoll_engine.o base_server_test.o \
#	event.o file.o inetaddr.o log.o msg_looper.o msg_parser.o msg_queue.o mutex.o rwlock.o sock.o \
#	socknotifier.o sockobserver.o sockstream.o sockstreamOB.o stream_msg_packetizer.o thread.o thread_msg_recv.o \
#	timed_stream.o
#BASE_CLIENT_TEST_OBJS = base_client_test.o timed_stream.o sockstream.o sock.o log.o inetaddr.o file.o rwlock.o
PROGRAM = base_server_test base_client_test hash_test base_conf_parser_test
BIN_DIR = ~/src/kunlun/bin
COMPILE = g++ -pthread -g -D_DEBUG

#all : $(EPOLLSERVER_OBJS) $(EPOLLCLIENT_OBJS) base_server_test base_client_test
all : $(COMMON_OBJS) $(PROGRAM)
.o:
	$(COMPILE) -o $@ $< $(COMMON_OBJS) $(LIB_PATH) $(INC_PATH)

.cpp:
	$(COMPILE) -o $@ $< $(COMMON_OBJS) $(LIB_PATH) $(INC_PATH)

.cpp.o:
	$(COMPILE) -c -o $@ $< $(INC_PATH)

install : 
	-cp -f $(PROGRAM) $(BIN_DIR)
#base_server_test : $(BASE_SERVER_TEST_OBJS)
#	$(COMPILE) -o $(BIN_DIR)/base_server_test $(BASE_SERVER_TEST_OBJS)

#base_client_test : $(BASE_CLIENT_TEST_OBJS)
#	$(COMPILE) -o $(BIN_DIR)/base_client_test $(BASE_CLIENT_TEST_OBJS)

.PHONY : clean
clean : 
	-rm -rf $(COMMON_OBJS) $(PROGRAM)