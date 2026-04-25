CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic
LIBS=-pthread -lcrypt

all: crack crypt_demo

crack: crack.c
	$(CC) $(CFLAGS) crack.c -o crack $(LIBS)

crypt_demo: crypt_demo.c
	$(CC) $(CFLAGS) crypt_demo.c -o crypt_demo $(LIBS)

clean:
	rm -f crack crypt_demo