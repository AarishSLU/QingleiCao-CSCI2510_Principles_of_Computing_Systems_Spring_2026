CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic
TARGET=slush

all: $(TARGET)

$(TARGET): slush.c
	$(CC) $(CFLAGS) slush.c -o $(TARGET)

clean:
	rm -f $(TARGET)