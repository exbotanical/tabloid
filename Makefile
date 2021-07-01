CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c17
LDFLAGS=
OBJFILES=$(wildcard src/*.c)
TARGET=tabloid

SCRIPTSDIR=scripts

DEST=/usr/local/bin

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

debug: CFLAGS += -D debug
debug: $(TARGET)

clean:
	rm $(TARGET)

install: $(TARGET)
	install -m 0777 $(TARGET) $(DEST)/$(TARGET)

check:
	@$(SCRIPTSDIR)/memcheck.bash $(TARGET)
