CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

.SUFFIXES :

.PHONY : all
all: 1-No-Sync.x 2-Atomic.x 3-Mutex.x 4-Timed-Lock.x \
	 5-Try-Lock.x 6-Busy.x 7-not-Busy.x 8-Deadlock.x \
	 9-no-Deadlock-Hierarchy.x A-no-Deadlock-Try-Backoff.x

.PHONY : clean
clean :
	rm -f *.x

%.x : %.c timing.h
	$(CC) $(CFLAGS) -pthread -o $@ $<
