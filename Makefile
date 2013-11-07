all: beleth

beleth: beleth.o lists.o ssh.o
	gcc beleth.o lists.o ssh.o -o beleth -lssh2

beleth.o: beleth.c
	gcc -Wall -c beleth.c 

lists.o: lists.c
	gcc -Wall -c lists.c

ssh.o: ssh.c
	gcc -Wall -c ssh.c

clean:
	rm *.o beleth
