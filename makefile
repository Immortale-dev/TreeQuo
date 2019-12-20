.PHONY: all generate_o generate_t generate_libs

CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SRCPATH:=src/
SRCS:=$(wildcard $(SRCPATH)*.cpp)
OBJS:=$(SRCS:%.cpp=%.o)
LIB_BPT:=inc/bplustree
LIB_CACHE:=inc/listcache
LIB_FS:=inc/dbfs
LIB_BPTB:=$(LIB_BPT)/inc/bplustreebase
LIBS:=$(LIB_BPT) $(LIB_CACHE) $(LIB_FS) $(LIB_BPTB)
LIBS_SRC:=$(LIBS:%=%/src)
LIBS_SRC_I:=$(LIBS_SRC:%=-I%)
LIBS_O=
INCL=-Isrc -Itest $(LIBS_SRC_I)


all: generate_o generate_t

generate_libs: $(LIBS)
	LIBS_O:=$(wildcard $(LIBS_SRC)*.o)
	
$(LIBS):
	$(MAKE) dist --directory=$@
	
generate_o: ${OBJS}

generate_t: 
	$(CC) $(CFLAGS) $(INCL) test/test.cpp -o test/test.o
	${CC} ${INCL} -o test.exe test/test.o ${OBJS}

%.o: %.cpp
	${CC} ${CFLAGS} ${INCL} $< -o $@
