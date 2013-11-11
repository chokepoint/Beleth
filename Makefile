CFLAGS+=	-Wall -I/usr/local/include
LDFLAGS+=	-L/usr/local/lib/ -lssh2

all: beleth

beleth: beleth.o lists.o ssh.o
	$(CC) $(LDFLAGS) beleth.o lists.o ssh.o -o beleth

beleth.o: beleth.c

lists.o: lists.c

ssh.o: ssh.c

clean:
	rm *.o beleth
