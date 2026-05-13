PROG= lab5

SRCS= lab5.c utils.c

LDADD += -llcf
LDADD += -L. -lgraphics
DPADD += ${LIBLCF} ${LIB_NAME}

LIB_NAME = libgraphics.a
LIB_SRCS = graphics.c
LIB_OBJS = ${LIB_SRCS:.c=.o}
CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: lib
lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

.include <minix.lcom.mk>
