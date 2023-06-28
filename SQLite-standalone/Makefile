CC=gcc
CFLAGS=-Wall
IFLAGS=-I inc
SRCDIR=src

all: CFLAGS += -O3
all: sqlite3_bench
debug: CFLAGS += -g -O0
debug: sqlite3_bench 

sqlite3_bench: $(SRCDIR)/sqlite3.o $(SRCDIR)/bench_sqlite3.o
	$(CC) $^ -o $@

$(SRCDIR)/sqlite3.o: $(SRCDIR)/sqlite3.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $^ -o $@

$(SRCDIR)/bench_sqlite3.o: $(SRCDIR)/bench_sqlite3.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $^ -o $@

clean:
	rm src/*.o
	rm sqlite3_bench
