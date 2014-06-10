CC=gcc
CFLAGS=-m32 -c -Wall -g
SOURCES=lgdb.c main.c lgdb_cli.c gdb.c lgdb_logger.c profiler.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=lgdb

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf *.o lgdb
