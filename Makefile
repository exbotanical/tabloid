CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c17
LDFLAGS=
SRC=$(wildcard src/*.c)
TEST=$(wildcard t/*.c)
DEPS=$(wildcard deps/**/*.c)

TARGET=tabloid

SCRIPTSDIR=scripts

DEST=/usr/local/bin

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(DEPS) $(LDFLAGS)

debug: CFLAGS += -D debug
debug: $(TARGET)

clean:
	rm $(TARGET)

install: $(TARGET)
	install -m 0777 $(TARGET) $(DEST)/$(TARGET)

check:
	@$(SCRIPTSDIR)/memcheck.bash $(TARGET)

test:
	$(CC) $(CFLAGS) $(LDFLAGS) $(TEST) $(DEPS) -o $(TARGET)
