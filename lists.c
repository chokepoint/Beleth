/*
 * Beleth - SSH Dictionary Attack
 * lists.c -- Linked list functions
 */ 
 
#include <libssh2.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/un.h>
#include <time.h>
#include <sys/wait.h>

#include "beleth.h"
#include "lists.h"

/* 
 * Initiate the linked list with the first element
 * Returns -1 on error. 1 on success
 */
int init_pw_list(char *pw) {
    struct pw_list *ptr = (struct pw_list*)malloc(sizeof(struct pw_list));
	
    if(ptr == NULL)
    {
		if (verbose >= VERBOSE_DEBUG)
			fprintf(stderr,"[!] Creating password linked list failed.\n");
        return -1;
    }
	strncpy(ptr->pw, pw, MAX_PW_LENGTH);
    ptr->next = NULL;

    pw_head = pw_tail = ptr;
    return 1;	
}

/*
 * Add entry to the end of the linked list
 * Returns -1 on error. 1 on success
 */
int add_pw_list(char *pw) {
	if(pw_head == NULL)  
        return (init_pw_list(pw));
	
    struct pw_list *ptr = (struct pw_list*)malloc(sizeof(struct pw_list));
    if(ptr == NULL) {
		if (verbose >= VERBOSE_DEBUG)
			fprintf(stderr,"[!] Couldn't add password to list.\n");
        return -1;
    }

    strncpy(ptr->pw,pw,MAX_PW_LENGTH);
    ptr->next = NULL;
	
    pw_tail->next = ptr;
    pw_tail = ptr;

    return 1;
}

/*
 * Destroy the linked list and free the memory
 */
void destroy_pw_list(void) {
	struct pw_list *ptr = pw_head;
	
	while (ptr != NULL) {
		ptr = pw_head->next;
		free(pw_head);
		pw_head = ptr;
	}
}

/* 
 * Initiate the linked list with the first element
 * Returns -1 on error. 1 on success
 */
int init_thread_list(pid_t pid, char *host, int port) {
    struct t_ctx *ptr = (struct t_ctx*)malloc(sizeof(struct t_ctx));

    if(ptr == NULL)
    {
		if (verbose >= VERBOSE_DEBUG)
			fprintf(stderr,"[!] Creating thread linked list failed.\n");
        return -1;
    }
    
    if ((ptr->fd = connect_sock()) == -1) {
		free(ptr);
		return -1;
	}
	
	ptr->pid = pid;
	ptr->port = port;
    strncpy(ptr->host,host,sizeof(ptr->host)-1);
    ptr->session = libssh2_session_init();
    ptr->next = NULL;

    t_head = t_tail = ptr;
    return 1;	
}

/*
 * Add entry to the end of the thread list
 * Returns -1 on error. 1 on success
 */
int add_thread_list(pid_t pid, char *host, int port) {
	if(t_head == NULL)  
        return (init_thread_list(pid, host, port));
	
    struct t_ctx *ptr = (struct t_ctx*)malloc(sizeof(struct t_ctx));
    if(ptr == NULL) {
		if (verbose >= VERBOSE_DEBUG)
			fprintf(stderr,"[!] Couldn't add thread to list.\n");
        return -1;
    }
    
    if ((ptr->fd = connect_sock()) == -1) {
		free(ptr);
		return -1;
	}
	
    ptr->pid = pid;
    ptr->port = port;
    strncpy(ptr->host,host,sizeof(ptr->host)-1);
    ptr->session = libssh2_session_init();
    ptr->next = NULL;
	
    t_tail->next = ptr;
    t_tail = ptr;

    return 1;
}

/*
 * Destroy the linked list and free the memory
 */
void destroy_thread_list(void) {
	struct t_ctx *ptr = t_head;
	
	while (ptr != NULL) {
		session_cleanup(ptr->sock,ptr->session);
		ptr = t_head->next;
		free(t_head);
		t_head = ptr;
	}
}
