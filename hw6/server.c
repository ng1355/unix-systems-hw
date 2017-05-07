/* Server code by Nikita Georgiou */ 

#include "communication.h"
#include "common.h"
#include "server.h"

#include <string.h>
#include <pthread.h>
#include <signal.h>

const char *PROGRAM_NAME;

int main(int argc, char** argv){
	PROGRAM_NAME = argv[0];

	int sock;
	uint16_t port = 0;
	sigset_t set;
	pthread_t accepter_th, mcast_th, sig_th;
	pthread_mutex_t barrier_lk;

	/* init mutexes */ 
	if(pthread_mutex_init(queue_lk, NULL) != 0 || 
			pthread_mutex_init(users_lk, NULL) != 0 ||
			pthread_mutex_init(&barrier_lk, NULL) != 0)
		eperror();

	/* init pthread attribute  to detach on creation */ 
	pthread_attr_init(&detach);
	pthread_attr_setdetachstate(&detach, PTHREAD_CREATE_DETACHED);

	/* init condition variable for multicasting */ 
	pthread_cond_init(&msg_exists, NULL);
	pthread_cond_init(&barrier_chk, NULL);

	/* ignore SIGPIPE */ 
	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	if(pthread_sigmask(SIG_SETMASK, &set, NULL) != 0) eperror();

	/* parse command line arguments to configure server */ 
	parse_args(argc, argv, &port);

	/* set up sockety stuff */ 
	if((sock = server_socket_setup(&port)) < 0) exit(EXIT_FAILURE);

	printf("Listening on port %d...\n", port);

	if(listen(sock, BACKLOG_MAX) < 0) eperror();

	/* have one thread handle all sigpipes */ 
	if(pthread_create(&sig_th, &detach, ignore_sigstop, &set) != 0)
		eperror();

	/* thread to accept new connections */ 
	if(pthread_create(&accepter_th, &detach, accept_clients, &sock) != 0)
		eperror();

	/* thread to multicast messages */ 
	if(pthread_create(&mcast_th, &detach, multicaster, (void *) NULL) != 0)
		eperror();	

	/* block until input is provided, in which case clean up & shut down */
	puts("Hit enter to close the server");
	getchar();
	puts("shutting down server...");
	
	pthread_cancel(sig_th); /* sigwait is cancel point */ 
	pthread_cancel(accepter_th); /* accept */ 
	pthread_cancel(mcast_th); /* cond_wait */ 
	/* all other threads exit when they "see" their user has disconnected */ 

	/* removes all users from user_list and closes their connections
	 * this will cause any remaining client_handler threads to exit */
	empty_userlist();

	/* verbose considering barrier is already atomic, but will block the 
	 * main thread until barrier is 0, ie all client_handlers are exited. 
	 * This is done so all mutex may be safely destroyed without 
	 * invoking undefined behavior when the client_handlers call their
	 * own cleanup routines, which lock users_lk */ 
	pthread_mutex_lock(&barrier_lk);
	while(barrier != 0)
		pthread_cond_wait(&barrier_chk, &barrier_lk);
	pthread_mutex_unlock(&barrier_lk);

	/* only the main thread should exist by this point, making all 
	 * deletion safe */ 
	pthread_mutex_destroy(queue_lk);
	pthread_mutex_destroy(users_lk);
	pthread_mutex_destroy(&barrier_lk);
	pthread_cond_destroy(&msg_exists);
	pthread_attr_destroy(&detach);

	/* should be deleted already but just in case */ 
	free(queued_msg.msg);
}

void *ignore_sigstop(void *arg){
	int sig;
	while(1){
		sigset_t set = *((sigset_t *) arg);
		sigwait(&set, &sig);
	}
	return NULL;
}

void empty_userlist(){
	pthread_mutex_lock(users_lk);
	for(int i = 0; i < MAX_CONN; i++)
		if(user_list[i] != NULL)
			remove_user(i);
	pthread_mutex_unlock(users_lk);
}

void parse_args(int argc, char **argv, uint16_t *port){
	switch(argc){
		case 2:
			if((*port = parse_port(argv[2])) < 1)
				exit(EXIT_FAILURE);
			break;
		default:
			*port = 0;
	}
}

void * accept_clients(void *arg){
	int sock = *((int *) arg);
	int clientsock;
	int *csock_addr;
	pthread_t chatter_th;

	while(1){
		/* passing a stack pointer or any static memory address
		 * risks a race condition across thread creation, as by the 
		 * time one thread executes client_handler, any local or
		 * static address from here may be overwritten. */ 
		if((clientsock = establish_client(sock)) < 0){
			perror("Failed to accept client");
			continue;
		}

		/* accept is cancellation point, so only allocate once
		 * we have a socket */ 
		if((csock_addr = malloc(sizeof(int))) == NULL){
			fprintf(stderr, "%s: Failed to allocate memory"
					" for socket\n", PROGRAM_NAME);
			continue;
		}

		*csock_addr = clientsock;

		if(pthread_create(&chatter_th, &detach, client_handler,
						csock_addr) != 0){
			perror("Failed to spawn thread for client");
			shutdown(clientsock, SHUT_RDWR);
		} 
	}
	return NULL;
}

int getuname(int sock, char *uname){
	if(read(sock, uname, UNAME_SIZE) < 1) return -1;
	return 0;
}

int add_user(user_t *client, int sock, char *uname){
	for(int i = 0; i < MAX_CONN - 1; i++){
		if(user_list[i] == (user_t *) NULL){
			user_list[i] = client;
			user_list[i]->sock = sock;
			user_list[i]->uname = uname;
			return i;
		}
	}
	return -1;
}

void *client_handler(void *sock){
	int clientsock = *((int *) sock);
	int client_id;
	char uname[UNAME_SIZE];
	char msg[MSG_MAX];
	char login[UNAME_SIZE + sizeof(" has logged in")];
	user_t client; /* all user info stored in this thread's stack */ 

	/* allocated from accept_clients */ 
	free(sock);

	pthread_mutex_lock(users_lk);

	/* get username from user as beginning of handshake, try adding
	 * them to the userlist. */ 
	if(getuname(clientsock, uname) != 0 ||
			(client_id = add_user(&client, clientsock, uname)) == -1){
		fprintf(stderr, "%s: Error adding client\n", PROGRAM_NAME);
		shutdown(clientsock, SHUT_RDWR);
		pthread_mutex_unlock(users_lk);
		return NULL;
	}

	/* thread is only truly considered "active" and part of the barrier
	 * when the user has been added to the list. ie, when the user
	 * has to be removed by a call to remove_user() */ 
	++barrier;

	/* give the user a list of online users, this completes handshake */ 
	if(send_unamelist(client_id, &client) != 0){
		remove_user(client_id);
		pthread_mutex_unlock(users_lk);
		return NULL;
	}

	/* let everyone know user has logged in */ 
	snprintf(login, sizeof(login), "%s has logged in", client.uname);
	multicast(SERVER_MSG, login);
	pthread_mutex_unlock(users_lk);

	/* block on user input, queueing any messages recieved for multicasting */
	while(1){
		memset(msg, 0, MSG_MAX);
		if(read_from_client(client.sock, msg, MSG_MAX) != 0){
			/* user is unreachable, ie. offline */ 
			pthread_mutex_lock(users_lk);
			remove_user(client_id);
			pthread_mutex_unlock(users_lk);
			return NULL;
		}
		queue_msg(client_id, msg);
	}
	return NULL;
}

int send_unamelist(int client_id, user_t *client){
	char unamelist[UNAMELIST_SIZE];
	char unamelist_cpy[UNAMELIST_SIZE];

	snprintf(unamelist, UNAMELIST_SIZE, "Users online: \n");

	/* snprintf really is a godsend. unamelist_cpy is so snprintf
	 * can append however many usernames are in the user_list */ 
	for(int i = 0; i < MAX_CONN; i++){
		if(user_list[i] != (user_t *) NULL && i != client_id){
			strncpy(unamelist_cpy, unamelist, UNAMELIST_SIZE);
			snprintf(unamelist, UNAMELIST_SIZE, 
				"%s\t%s\n", unamelist_cpy, user_list[i]->uname);
		}
	}

	if(write(client->sock, unamelist, UNAMELIST_SIZE) < 0){
		fprintf(stderr, "%s: Failed to send unamelist to client %s\n",
				PROGRAM_NAME, client->uname);
		return -1; /* probably means they went offline already */ 
	}
	return 0;
}

/* in all contexts this function is called, the user's thread 
 * is guarinteed to be alive until remove_user returns. 
 * Note that this function is idempotent. This is required during server
 * shutdown as client_handler threads will run this even though empty_userlist
 * has removed all users */ 
void remove_user(int user_id){
	if(user_list[user_id] == (user_t *) NULL) return;

	char logoff[UNAME_SIZE + sizeof(" logged off")];

	shutdown(user_list[user_id]->sock, SHUT_RDWR);
	snprintf(logoff, sizeof(logoff), "%s logged off", 
			user_list[user_id]->uname);
	user_list[user_id] = (user_t *) NULL;
	--barrier;
	pthread_cond_signal(&barrier_chk); /* nop if server isn't shutting down */
	multicast(SERVER_MSG, logoff);
}

void queue_msg(int client_id, char *msg){
	pthread_mutex_lock(queue_lk);
	queued_msg.client_id = client_id;
	/* msg is put on the heap since the client's thread can exit
	 * before the message is multicast. 
	 * failure should be rare, these messages will silently be discarded */ 
	if((queued_msg.msg = strdup(msg)) == NULL){
		perror("strdup failed to allocate for message queue.");
		pthread_mutex_unlock(queue_lk);
		return;
	}
	pthread_cond_signal(&msg_exists);
	pthread_mutex_unlock(queue_lk);
}

void *multicaster(){
	while(1){
		pthread_mutex_lock(queue_lk);

		while(queued_msg.msg == (char *) NULL)
			pthread_cond_wait(&msg_exists, queue_lk);

		pthread_mutex_lock(users_lk);
		multicast(queued_msg.client_id, queued_msg.msg);
		pthread_mutex_unlock(users_lk);

		free(queued_msg.msg);
		queued_msg.msg = (char *) NULL;
		pthread_mutex_unlock(queue_lk);
	}
	return NULL;
}

void multicast(int user_id, char *msg){
	user_t *client;
	for(int i = 0; i < MAX_CONN; i++){
		if(user_list[i] != (user_t *) NULL && user_id != i){
			client = user_list[i];
			if(write(client->sock, msg, MSG_MAX) < 0){
				fprintf(stderr, "%s: multicast error: failed "
						"to message %s\n", PROGRAM_NAME,
						client->uname);
			}
		}
	}
	if(user_id == SERVER_MSG) puts(msg);
}
