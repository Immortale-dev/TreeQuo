.PHONY: all rc generate_o generate_t generate_libs custom

CC=g++
CFLAGS=-c -Wall -std=c++17
LDFLAGS=
SRCPATH:=src/
SRCS:=$(wildcard $(SRCPATH)*.cpp)
OBJS:=$(SRCS:%.cpp=%.o)

LIB_CACHE:=inc/listcache
LIB_FS:=inc/dbfs
LIB_BPTB:=inc/bplustree
LIB_LOG:=inc/logger

LIBS:=$(LIB_CACHE) $(LIB_FS) $(LIB_BPTB) $(LIB_LOG)
LIBS_SRC:=$(LIBS:%=%/src)
LIBS_SRC_I:=$(LIBS_SRC:%=-I%)
LIBS_SRCPATH:=$(LIBS_SRC:%=%/)
LIBS_SRCS:=$(wildcard inc/dbfs/src/*.cpp) $(wildcard inc/logger/src/*.cpp)
LIBS_O=$(LIBS_SRCS:%.cpp=%.o)
INCL=-Isrc -Itest $(LIBS_SRC_I)


all: generate_libs generate_o generate_t

rc: generate_libs generate_o 
	$(CC) $(CFLAGS) $(INCL) test/rc_test.cpp -o test/rc_test.o
	${CC} ${INCL} -o rc_test.exe test/rc_test.o ${OBJS} ${LIBS_O}

generate_libs: ${LIBS_O}
	
$(LIBS):
	$(MAKE) dist --directory=$@
	
generate_o: ${OBJS}

generate_t: 
	$(CC) $(CFLAGS) $(INCL) test/test.cpp -o test/test.o
	${CC} ${INCL} -o test.exe test/test.o ${OBJS} ${LIBS_O}
	
custom: generate_o
	$(CC) $(CFLAGS) $(INCL) test/mtest.cpp -o test/mtest.o
	${CC} ${INCL} -o mtest.exe test/mtest.o ${OBJS} ${LIBS_O}

%.o: %.cpp
	${CC} ${CFLAGS} ${INCL} $< -o $@
