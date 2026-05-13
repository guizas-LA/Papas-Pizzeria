PROG= lab4

SRCS= lab4.c

CFLAGS += -I../lab2

DPADD += ${LIBLCF} ${LIB_NAME}
LDADD += -llcf
LDADD += -L. -lmouse
LDADD += -L../lab2 -ltimer

LIB_NAME = libmouse.a
LIB_SRCS = mouse.c utils.c
LIB_OBJS = ${LIB_SRCS:.c=.o}
CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: timerlib lib
timerlib:
	${MAKE} -C ../lab2 lib

lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

.include <minix.lcom.mk>
