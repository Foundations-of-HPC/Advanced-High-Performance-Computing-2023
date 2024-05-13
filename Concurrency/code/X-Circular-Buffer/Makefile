CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

.SUFFIXES :

.PHONY : all
all: libcircbuf.so test.x

.PHONY : clean
clean :
	rm -f *.x *.o *.so

circbuf.o : circbuf.c
	$(CC) -c -fPIC $(CFLAGS) -o $@ $<

libcircbuf.so : circbuf.o
	$(CC) -shared -o $@ $<

test.x : test.c libcircbuf.so
	$(CC) $(CFLAGS) -I. -L. '-Wl,-rpath=$$ORIGIN' -lcircbuf -o $@ $<
