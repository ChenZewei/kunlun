INC_PATH = -I../common -I../proxynode -I../sqlite -I/usr/local/include
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
SQLITE_OBJS = ../sqlite/sqlite_engine.o ../sqlite/tb_account.o ../sqlite/tb_container.o \
	../sqlite/tb_delete_file_record.o ../sqlite/tb_hashcode.o ../sqlite/sqlite3.o
STORAGENODE_OBJS = storage_global.o storage_msg_parser.o storage_server.o thread_beat_heart.o \
	thread_sync_data.o vnode_info.o storage_conf_parser.o
PROGRAM = kunlun_storaged storage_conf_parser_test
BIN_DIR = ~/src/kunlun/bin
COMPILE = g++ -g -pthread -D_DEBUG
CFLAGS = -ldl

all : $(COMMON_OBJS) $(STORAGENODE_OBJS) $(SQLITE_OBJS) $(PROGRAM)

.o :
	$(COMPILE) -o $@ $< $(COMMON_OBJS) $(STORAGENODE_OBJS) $(SQLITE_OBJS) $(LIB_PATH) $(INC_PATH) $(CFLAGS)

.cpp :
	$(COMPILE) -o $@ $< $(COMMON_OBJS) $(STORAGENODE_OBJS) $(SQLITE_OBJS) $(LIB_PATH) $(INC_PATH) $(CFLAGS)

.cpp.o :
	$(COMPILE) -c -o $@ $< $(INC_PATH)

.c.o :
	gcc -c -o $@ $< $(INC_PATH)

.PHONY : install
install : 
	-cp -f $(PROGRAM) $(BIN_DIR)/
.PHONY : clean
clean : 
	-rm -rf $(COMMON_OBJS) $(STORAGENODE_OBJS) $(SQLITE_OBJS) $(PROGRAM)
