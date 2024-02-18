CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib

all: clean thread-worker.a

thread-worker.a: thread-worker.o
	$(AR) libthread-worker.a thread-worker.o
	$(RANLIB) libthread-worker.a

thread-worker.o: dlist.h hashtable.h thread-worker.h thread-worker_types.h mutex_types.h

ifeq ($(SCHED), RR)
	$(CC) $(CFLAGS) -DRR dlist.c hashtable.c thread-worker.c 
else ifeq ($(SCHED), MLFQ)
	$(CC) $(CFLAGS) -DMLFQ dlist.c hashtable.c thread-worker.c
else
	echo "no such scheduling algorithm"
endif

clean:
	rm -rf testfile *.o *.a
