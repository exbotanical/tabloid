CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c17
LDFLAGS=
OBJFILES=src/main.c src/error.c src/editor.c src/buffer.c src/render.c
TARGET=cnano

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
