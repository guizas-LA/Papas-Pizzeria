PROG= proj

.PATH: src src/model src/view ../lab2

SRCS= main.c loop.c interrupts.c timer.c game_state.c rtc.c draw.c draw_elements.c draw_utils.c sprites.c

CFLAGS += -Iinclude
CFLAGS += -Iimages
CFLAGS += -I../lab2
CFLAGS += -I../lab3
CFLAGS += -I../lab4
CFLAGS += -I../lab5

DPADD += ${LIBLCF}
DPADD += ../lab3/libkbc.a
DPADD += ../lab4/libmouse.a
DPADD += ../lab5/libgraphics.a

LDADD += -llcf
LDADD += -L../lab3 -lkbc
LDADD += -L../lab4 -lmouse
LDADD += -L../lab5 -lgraphics

all: libs
libs:
	${MAKE} -C ../lab2 lib
	${MAKE} -C ../lab3 lib
	${MAKE} -C ../lab4 lib
	${MAKE} -C ../lab5 lib

.include <minix.lcom.mk>
