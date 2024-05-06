CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

.SUFFIXES :

.PHONY : all
all: 1-Single-Process.x 2-Fork.x 3-Multiple-Threads.x

.PHONY : clean
clean :
	rm -f *.x


1-Single-Process.x : 1-Single-Process.c
	$(CC) $(CFLAGS) -o $@ $<

2-Fork.x : 2-Fork.c
	$(CC) $(CFLAGS) -o $@ $<

3-Multiple-Threads.x : 3-Multiple-Threads.c
	$(CC) $(CFLAFS) -pthread -o $@ $<
