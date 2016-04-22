CC=gcc
CFLAGS=`cups-config --cflags`
LIBS=`cups-config --libs`
OBJS=main.o

main:
	${CC} -o main ${CFLAGS} main.c ${LIBS}

clean:
	rm -rf ${OBJS} main
