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

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -m 0777 $(TARGET) $(DEST)/$(TARGET)

check:
	@$(SCRIPTSDIR)/memcheck.bash $(TARGET)
