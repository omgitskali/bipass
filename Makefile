CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lssl -lcrypto

SRC = src/bipass.c
TARGET = bipass
WORDLIST_HEADER = src/wordlist.h

all: $(TARGET)

$(TARGET): $(WORDLIST_HEADER) $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

$(WORDLIST_HEADER):
	./scripts/generate_wordlist.sh

clean:
	rm -f $(TARGET) $(WORDLIST_HEADER)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall
