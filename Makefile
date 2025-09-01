# O Makefile

PREFIX ?=	/usr/local

CC ?=		cc
CFLAGS =	-g -O2 -DTARGET=${TARGET}

PROG =	O
OBJS =	O.o arm64.o x64.o

TARGET =	"\"`${CC} -dumpmachine | cut -d '-' -f 1`\""

all: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} ${OBJS}

install:
	install -c -S -s -m 755 ${PROG} ${PREFIX}/bin

clean:
	rm -f ${PROG} ${OBJS} ${PROG}.core
