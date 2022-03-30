# O Makefile

PREFIX ?=	/usr/local

CC ?=		cc
CFLAGS ?=	-g -O2

PROG =	O
OBJS =	O.o

all: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} ${OBJS}

install:
	install -c -S -s -m 755 ${PROG} ${PREFIX}/bin

clean:
	rm -f ${PROG} ${OBJS} ${PROG}.core
