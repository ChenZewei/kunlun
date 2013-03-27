OBJECTS = file.o file_test.o global.o inetaddr.o \
			log.o log_test.o lock.o thread.o thread_test.o
HEADERS = common_types.h file.h global.h inetaddr.h \
			log.h lock.h thread.h thread_func.h
FILE_TEST_OBJS = file_test.o file.o 
LOG_TEST_OBJS = log_test.o log.o lock.o file.o
THREAD_TEST_OBJS = thread_test.o thread.o log.o lock.o \
					file.o global.o
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

.PHONY : clean
clean : 
	-rm -rf $(OBJECTS)