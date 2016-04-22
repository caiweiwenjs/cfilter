TARGET=cfilter
CC=gcc
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
CFLAGS=`cups-config --cflags`
LIBS=`cups-config --libs`

$(TARGET): $(OBJS)
	${CC} -o $@ $^ ${CFLAGS} ${LIBS}

.PHONY:clean
clean:
	-rm -rf $(OBJS) $(TARGET)
