CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c17
LDFLAGS=
OBJFILES=$(wildcard src/*.c)
TARGET=cnano

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
