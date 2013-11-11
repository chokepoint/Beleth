CC=cc
CFLAGS=-Wall

all: beleth

beleth: beleth.o lists.o ssh.o
	$(CC) beleth.o lists.o ssh.o -o beleth -lssh2 

beleth.o: beleth.c
	$(CC) $(CFLAGS) -c beleth.c

lists.o: lists.c
	$(CC) $(CFLAGS) -c lists.c

ssh.o: ssh.c
	$(CC) $(CFLAGS) -c ssh.c

clean:
	rm *.o beleth
