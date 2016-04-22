TARGET=client
CC=gcc
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
CFLAGS=-g

$(TARGET): $(OBJS)
	$(CC) -g -o $@ $^

.PHONY: clean
clean:
	-rm -rf $(OBJS) $(TARGET)
