#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include "handlers.h"
#include "constants.h"

/*log error and then die*/
void errnoExit(const char *reason){
    fprintf(stderr, "%s; %s\n", reason, strerror(errno));
    exit(EXIT_FAILURE);
}

/*malloc or die!*/
void *allocaZam(size_t size) {
    void *buffer = malloc(size);
    if (buffer == NULL) {
        errnoExit("ERROR: malloc was not able to allocate enough space");
    }
    return buffer;
}


/*functions recv's data from socket*/
/*and loads it into a bstring that*/
/*has length the max length of an*/
/*IRC message, so it contains at least*/
/*one message and can be sent to the parser*/
bstring safeRecv(cli *client) {
    int socket = client->socket;
    bstring buffer;
    buffer = safe_BfromCstr("");
    int nbytes;
    if ((nbytes = recv(socket, buffer->data, IRC_MAX_LEN+1, 0)) <= 0){
        /*perror("ERROR: Unexpected error in recv()");*/
        clientFree(client);
        bdestroy(buffer);
        pthread_exit(NULL);
    }
    buffer->slen = nbytes;
    buffer->data[nbytes] = (unsigned char)'\0';
    return buffer;
}

/*send string tosend to client with error logging and exit*/
void safeSend(cli *client, char *tosend){
    int socket = client->socket;
    pthread_mutex_lock(client->clientLock);
    int nbytes = send(socket, tosend, strlen(tosend), 0);
    pthread_mutex_unlock(client->clientLock);
    if (nbytes == -1 && (errno == ECONNRESET || errno == EPIPE)) {
        perror("ERROR: Socket connection closed before call to send()");
        clientFree(client);
        pthread_exit(NULL);
    }
    else if (nbytes == -1){
        perror("ERROR: Unexpected error in send()");
        clientFree(client);
        pthread_exit(NULL);
    }
}

pthread_rwlock_t *newRwlock(){
    /*init the server lock so that multiple threads*/
    /*can update server state*/
    pthread_rwlockattr_t *attrServerLock = (pthread_rwlockattr_t *)allocaZam(sizeof(pthread_rwlockattr_t));
    pthread_rwlockattr_init(attrServerLock);
    /*pthread_rwlockattr_setkind_np(&attrServerLock, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);*/
    pthread_rwlock_t *serverLock = (pthread_rwlock_t *)allocaZam(sizeof(pthread_rwlock_t));
    /*pthread_rwlockattr_destroy(&attrServerLock);*/
    pthread_rwlock_init(serverLock, attrServerLock);
    pthread_rwlockattr_destroy(attrServerLock);
    return serverLock;
}
