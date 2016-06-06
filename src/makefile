CFLAGS= -O2 -std=c99 -Wshadow -g
BINDIR=bin
TARGET=ricagr
LDFLAGS= -lm

.PHONY: all clean rcag

all: target

target: main.o smpl.o rand.o pqueue.o
	$(CC) -o "../$(BINDIR)/$(TARGET)" -Bstatic $^ $(LDFLAGS)

smpl.o: smpl.c smpl.h
	$(CC) -c smpl.c -o $@ $(CFLAGS)

main.o: main.c smpl.h
	$(CC) -c main.c -o $@ $(CFLAGS)

rand.o: rand.c
	$(CC) -c rand.c -o $@ $(CFLAGS)

pqueue.o: pqueue.c
	$(CC) -c pqueue.c -o $@ $(CFLAGS)

clean:
	@rm -f *.o "../$(BINDIR)/$(TARGET)"

