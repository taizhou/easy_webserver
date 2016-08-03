CC=gcc
VPATH=./src
WORKDIR=.
CFLAGS=-Wall -g -I$(WORKDIR)/inc/ 


EXEC=server

all:$(EXEC)

server:Wserver.o Wsocket.o Wbase.o chttp.o cstring.o clinklist.o  cutils.o
	$(CC) $^ -o $@ -lpthread

	
%.o:%.c
	$(CC) -c $< -o $@  $(CFLAGS)

.PHONY:clean
clean:
	rm -f *.o $(EXEC)


