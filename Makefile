CFLAGS      = -Wall -I/usr/local/include
LDFLAGS     = -L/usr/local/lib/ -lssh2

prefix      = /usr/local
exec_prefix = $(prefix)
bindir      = $(exec_prefix)/bin

all: beleth

beleth: beleth.o lists.o ssh.o
	$(CC) beleth.o lists.o ssh.o $(LDFLAGS) -o $@

beleth.o: beleth.c

lists.o: lists.c

ssh.o: ssh.c

install:
	install -d $(DESTDIR)$(bindir)
	install -m 0755 beleth $(DESTDIR)$(bindir)

uninstall:
	-rm $(DESTDIR)$(bindir)/beleth

clean:
	-rm *.o beleth

.PHONY: clean install uninstall
