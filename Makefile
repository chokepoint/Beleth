all: beleth

beleth: beleth.c
	gcc beleth.c -o beleth -lssh2 -Wall

clean:
	rm beleth
