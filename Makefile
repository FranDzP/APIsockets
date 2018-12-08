CC = gcc
CFLAGS = 
#Descomentar la siguiente linea para olivo
#LIBS = -lsocket -lnsl
#Descomentar la siguiente linea para linux
LIBS =

PROGS = servidor cliente

all: ${PROGS}

servidor: servidor.c
	${CC} ${CFLAGS} servidor.c -o servidor ${LIBS}
	
cliente: cliente.c
	${CC} ${CFLAGS} cliente.c -o cliente ${LIBS}


clean:
	rm *.o ${PROGS}
