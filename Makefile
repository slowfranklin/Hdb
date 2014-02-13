CC = gcc

BGMERGE = -DBGMERGE
DEBUG =        -g -ggdb -DINFO

#detect OS,support Linux and Mac OS
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
        CFLAGS =-c -std=c99 -W -Wall -Werror -fPIC $(DEBUG) $(BGMERGE)
        LDFLAGS=-fPIC -shared
        LIB_EXTENSION=exe
endif
ifeq ($(UNAME), Darwin)
        CFLAGS =-c -std=c99 -W -Wall -Werror $(DEBUG) $(BGMERGE)
        LDFLAGS=-std=c99 -W -Wall -Werror -dynamiclib -flat_namespace
        LIB_EXTENSION=dylib
endif

CFLAGS = -pipe -Wall -I -g -O0 -ggdb


LIB_OBJS = \
        ./Hdb.o\
		./Page.o\
		./PageCache.o\
		./VCode.o\
		./ByteOrder.o\
		./Hashtable.o\
		./DList.o\
		./Vector.o\
		./LinkList.o\
		./util.o\
		./FileOperate.o\
		./Strategy.o\
		./Journal.o\
		./Trace.o\
		./HdbExpand.o\
		./HdbIndex.o\
		./crc32.o\
		./test.o



LIBRARY = hdb.$(LIB_EXTENSION)

all: $(LIBRARY)
clear:
	-rm $(LIB_OBJS)
	-rm hdb.$(LIB_EXTENSION)
cleandb:
	-rm hdb.db hdb.index
	-rm *.log
	-rm core*

$(LIBRARY): $(LIB_OBJS)
	$(CC) -pthread  $(CFLAGS) $(LIB_OBJS) -o hdb.$(LIB_EXTENSION) -lm -lrt

