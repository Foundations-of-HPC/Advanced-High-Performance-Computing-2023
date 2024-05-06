CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

.SUFFIXES :

.PHONY : all
all: 1-no-Condition-Variable.x 2-Minimal.x 3-Better.x 4-Condition-Variable.x \
	 5-Bad-Signal.x 6-Broadcast.x

.PHONY : clean
clean :
	rm -f *.x

%.x : %.c 
	$(CC) $(CFLAGS) -pthread -o $@ $<
