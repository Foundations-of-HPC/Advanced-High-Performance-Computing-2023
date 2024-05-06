CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

.SUFFIXES :

.PHONY : all
all: 1-no-Join.x 2-Master-Join.x 3-Multiple-Threads.x 4-Detach.x 5-Input-Output.x 6-Input-Output-better.x

.PHONY : clean
clean :
	rm -f *.x

%.x : %.c
	$(CC) $(CFLAGS) -pthread -o $@ $<
