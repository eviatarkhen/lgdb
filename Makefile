CC=gcc
CFLAGS=-m32 -c -Wall

all: lgdb

lgdb: main.o     \
	lgdb_cli.o \
	lgdb.o     
	$(CC) main.o lgdb_cli.o lgdb.o -o lgdb

main.o: main.c
	$(CC) $(CFLAGS) main.c

lgdb_cli.o: lgdb_cli.c
	$(CC) $(CFLAGS) lgdb_cli.c

lgdb.o: lgdb.c
	$(CC) $(CFLAGS) lgdb.c

clean:
	rm -rf *.o lgdb
