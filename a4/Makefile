CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = a4_practice.exe
SRC = main.c fs_indexed.c

all: $(TARGET)

$(TARGET): $(SRC) fs_indexed.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
