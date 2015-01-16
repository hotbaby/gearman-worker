# Makefile for remote control service

CC = gcc
CFLAGS = -Werror
LDFLAGS = -lgearman
TARGET = worker.so

all:$(TARGET)

worker.so:
	$(CC) worker.c $(CFLAGS) $(LDFLAGS) -fPIC -shared -o $@

clean:
	rm $(TARGET)
