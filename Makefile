CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lcurl -lssl -lcrypto

SRC = src/bipass.c
TARGET = bipass

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall
