CC=gcc
CFLAGS=-m32 -c -Wall

all: lgdb

debug: CC += -DDEBUG -g
debug: lgdb

lgdb: main.o     \
	lgdb_cli.o \
	gdb.o \
	lgdb.o     
	$(CC)   main.o		\
 		lgdb_cli.o 	\
		lgdb.o     	\
		gdb.o      	\
		-o lgdb

main.o: main.c
	$(CC) $(CFLAGS) main.c

lgdb_cli.o: lgdb_cli.c
	$(CC) $(CFLAGS) lgdb_cli.c

gdb.o: gdb.c
	$(CC) $(CFLAGS) gdb.c

lgdb.o: lgdb.c
	$(CC) $(CFLAGS) lgdb.c

clean:
	rm -rf *.o lgdb
