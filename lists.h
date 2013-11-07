/*
 * Beleth - SSH Dictionary Attack
 * lists.h -- Linked list header
 */ 

#ifndef LISTS_H
#define LISTS_H

/* Globals */
#define MAX_PW_LENGTH 51

/* thread context structure */
struct t_ctx 
{
	int sock; 				/* SSH connection socket */
	int fd; 				/* Unix IPC Socket */
	int port;
	char host[21];
	LIBSSH2_SESSION *session;
	pid_t pid;
	
	struct t_ctx *next;
};

struct t_ctx *t_head;
struct t_ctx *t_tail;

/* Linked list structure to hold the word list */
struct pw_list
{
    char pw[MAX_PW_LENGTH];
    struct pw_list *next;
};

struct pw_list *pw_head;
struct pw_list *pw_tail;

/* Child process thread tracking linked list functions */
int init_thread_list(pid_t pid, char *host, int port);
int add_thread_list(pid_t pid, char *host, int port);
void destroy_thread_list(void);

/* Password linked list functions */
int init_pw_list(char *pw);
int add_pw_list(char *pw);
void destroy_pw_list(void);

#endif
