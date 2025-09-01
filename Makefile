# O Makefile

PREFIX ?=	/usr/local

CC ?=		cc
CFLAGS ?=	-g -O2

PROG =	O
OBJS =	O.o

ARM64 =	arm64.o
X64 =	x64.o

all: arm64 x64

arm64: ${OBJS} ${ARM64}
	${CC} ${LDFLAGS} -o ${PROG}.$@ ${OBJS} ${ARM64}

x64: ${OBJS} ${X64}
	${CC} ${LDFLAGS} -o ${PROG}.$@ ${OBJS} ${X64}

install:
	install -c -S -s -m 755 ${PROG} ${PREFIX}/bin

clean:
	rm -f ${PROG}.arm64 ${PROG}.x64 ${OBJS} ${ARM64} ${X64} *.core
