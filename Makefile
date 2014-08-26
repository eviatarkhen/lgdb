CC=gcc
CFLAGS=-m32 -c -Wall -g
LIBS=-lpthread
SOURCES=lgdb.c main.c profiler.c lgdb_cli.c gdb.c lgdb_logger.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=lgdb

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf *.o lgdb
