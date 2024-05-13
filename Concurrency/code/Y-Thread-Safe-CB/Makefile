CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

.SUFFIXES :

.PHONY : all
all: libtscircbuf.so test.x

.PHONY : clean
clean :
	rm -f *.x *.o *.so

circbuf.o : circbuf.c circbuf.h
	$(CC) -c -fPIC $(CFLAGS) -o $@ $<

tscircbuf.o : tscircbuf.c tscircbuf.h
	$(CC) -c -fPIC $(CFLAGS) -pthread -o $@ $<

libtscircbuf.so : circbuf.o tscircbuf.o
	$(CC) -pthread -shared -o $@ $^

test.x: test.c libtscircbuf.so
	$(CC) $(CFLAGS) -pthread -I. -L. '-Wl,-rpath=$$ORIGIN' -ltscircbuf -o $@ $<
