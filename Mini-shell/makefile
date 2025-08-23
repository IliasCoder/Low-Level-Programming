CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic
TARGET=minishell
SOURCE=shell.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: clean debug