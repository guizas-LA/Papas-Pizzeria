PROG= lab3

SRCS= lab3.c kbc.c utils.c

CFLAGS += -DLAB3
CFLAGS += -I../lab2

DPADD += ${LIBLCF}
LDADD += -llcf 
LDADD += -L../lab2 -ltimer

.include <minix.lcom.mk>


