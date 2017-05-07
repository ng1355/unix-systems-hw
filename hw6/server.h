#ifndef _SERVER_H
#define _SERVER_H

#include "communication.h"
#include <signal.h>

/* client_id for messages sent from the server itself (eg. log offs).
 * multicasts using this send the msg to all clients */ 
#define SERVER_MSG -1

/* defines a chatroom user by their socket fd and their username */ 
typedef struct user_t{
	int sock;
	char *uname;
} user_t;

/* used to track who sent a queued message */ 
struct msg_queue{
	int client_id;
	char *msg;
};

/* list of pointers to user struct. The list is not null terminated
 * and may have "NULL" holes */ 
user_t *user_list[MAX_CONN]; 

/* a queue of size 1 essentially. Messages to be multicasted are stored
 * here with the index in the user_list of who sent it */ 
struct msg_queue queued_msg;

pthread_mutex_t queue_lock; /* mutex for accessing the message queue */ 
pthread_mutex_t users_lock; /* mutex for accessing user_list */ 

/* a little heretical to have in a header but its convenient */ 
pthread_mutex_t *queue_lk = &queue_lock;
pthread_mutex_t *users_lk = &users_lock;

pthread_attr_t detach; /* skips having to call pthread_detach() */ 

/* multicast() blocks until there's a message to multicast */
pthread_cond_t msg_exists;

/* used for cleanup. sig_atomic_t is a macro for int, which is interestingly
 * guarinteed to be atomic on POSIX systems. Used to avoid having to lock
 * barrier */ 
sig_atomic_t barrier;

/* Main thread will wait on this after the server begins shutting down. 
 * Waits for all user threads to exist before destroying mutexes, etc. */ 
pthread_cond_t barrier_chk;

/* *** server functions *** */ 

/* Either uses the command line supplied port if provided, or binds the 
 * server to any port */ 
void parse_args(int argc, char **argv, uint16_t *port);

/* run as a thread, accepts clients, and passes their socket to a new thread
 * running client_handler. Never quits unless cancelled */ 
void *accept_clients(void *arg);

/* Gets username from a connected client as part of an initial handshake. 
 * returns -1 on error and 0 on success */
int getuname(int sock, char *uname);

/* Adds a user to user_list, referencing the stack memory of the thread 
 * handling the user. This is safe as a user's thread shares the user's
 * lifetime. Returns -1 on failure and 0 on success. Does not lock
 * the user_list */ 
int add_user(user_t *client, int sock, char *uname);

/* Performs the initial handshake when establishing a client - getting
 * their username and sending them the client list - before recieveing
 * messages from them for queueing. */ 
void *client_handler(void *user_id);

/* Removes a user from the queue by shutting down their socket and setting 
 * their pointer in the user_list to NULL. Multicasts that the user has
 * logged off. Does not lock  user_list */ 
void remove_user(int user_id);

/* Configures msg_queue with a client id and uses strdup to clone msg
 * to msg_queue's msg pointer. This memory is expected to be freed by
 * multicaster(). Signals multicaster() to multicast on success, or does
 * nothing on failure. Locks msg_queue */ 
void queue_msg(int client_id, char *msg);

/* Driver function for multicast(). Blocks on a condition variable 
 * until signalled by queue_msg. Multicasts the message pointed to by
 * queue_msg and then frees this memory, while setting the pointer to NULL. 
 * Locks both queue_msg and user_list */ 
void *multicaster();

/* Sends a message to all users except user_id. If user_id is SERVER_MSG,
 * then also echoes the message to the server's stdout. Does not lock user_list */ 
void multicast();

/* generates a formatted string displays all users currently connected,
 * and sends it to the socket specified by client. Returns -1 on failure
 * and 0 on success. Does not lock user_list */ 
int send_unamelist(int client_id, user_t *client);

/* Combs the user_list, calling remove_user() on any present users */ 
void empty_userlist();

/* run on a thread to consume blocked SIGPIPEs. */ 
void *ignore_sigstop(void *arg);

#endif
