#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "server.h"
#include "dispatch.h"
#include "bstring/bstrlib.h"
#include "irc_strings.h"
#include "constants.h"
#include "util.h"
#include "reply.h"
#include "handlers.h"
#include "hash.h"
#include "message.h"


/*FUNCTIONS*/

/*start server listening on port*/
serv *initServer(char *port, char *password){
    int serverSocket;
    /*adress info structs to be packed*/
    struct sockaddr_in serverAddr;
    int yes = 1;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port));
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if ((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        errnoExit("ERROR: Could not find a socket to bind to");
    }
    /*multiple sockets on the same address*/
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0){
        close(serverSocket);
        errnoExit("ERROR: Could not set socket options");
    }
    if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
        close(serverSocket);
        errnoExit("ERROR: Could not bind to socket");
    }
    if(listen(serverSocket, SERVER_BACKLOG) < 0){
        close(serverSocket);
        errnoExit("ERROR: Fail on listen");
    }
    printf("Listening on port %s...\n", port);

    serv *newServer = (serv *) allocaZam(sizeof(serv));

    /*get time that server was created*/
    time_t currTime = time(NULL);
    char *createdStr = ctime(&currTime);
    int len = strlen(createdStr);
    createdStr[len - 1] = '\0';
    bstring created = safe_BfromCstr(createdStr);
    newServer->created = created;

    /*set server name*/
    bstring serverName = safe_BfromCstr("");
    int rc = gethostname(charBuffromBstr(serverName), 513);
    serverName->slen = strlen(charBuffromBstr(serverName));
    if (rc != 0 || !serverName->slen){
        bdestroy(serverName);
        serverName = safe_BfromCstr("unresolved host");
    }
    newServer->name = serverName;

    /*initialize reader/writer lock to eliminate race conditions*/
    /*between threads trying modify server state*/
    newServer->lock = newRwlock();

    /*initialize reader/writer locks to eliminate race conditions*/
    /*between threads trying to write the same key to the tree*/
    /*concurrently, read a key that's being changed, etc.*/
    newServer->clientsLock = newRwlock();
    newServer->channelsLock = newRwlock();

    /*initialize client hash table* to hold server
     * state and hash client data by nick*/
    cliTbl *clients = (cliTbl*) allocaZam(sizeof(cliTbl));
    *clients = NULL;
    newServer->clients = clients;

    chnlTbl *channels = (chnlTbl*) allocaZam(sizeof(chnlTbl));
    *channels = NULL;
    newServer->channels = channels;

    /*set additional server info*/
    newServer->socket = serverSocket;
    newServer->password = safe_BfromCstr(password);
    newServer->info = safe_BfromCstr("chir-0.0.2");
    newServer->version = safe_BfromCstr("0.0.2");
    newServer->channelModes = safe_BfromCstr("mtov");
    newServer->userModes = safe_BfromCstr("ao");
    newServer->numClients = 0;
    newServer->numConnections = 0;
    newServer->numChannels = 0;

    return newServer;
}

void incrementServerConnections(serv *server){
    pthread_rwlock_wrlock(server->lock);
    server->numConnections++;
    pthread_rwlock_unlock(server->lock);
}

void decrementServerConnections(serv *server){
    pthread_rwlock_wrlock(server->lock);
    server->numConnections--;
    pthread_rwlock_unlock(server->lock);
}

void incrementServerClients(serv *server){
    pthread_rwlock_wrlock(server->lock);
    server->numClients++;
    pthread_rwlock_unlock(server->lock);
}

void decrementServerClients(serv *server){
    pthread_rwlock_wrlock(server->lock);
    server->numClients--;
    pthread_rwlock_unlock(server->lock);
}

void serverFree(serv *server){
    bdestroy(server->name);
    bdestroy(server->created);
    bdestroy(server->userModes);
    bdestroy(server->channelModes);
    pthread_rwlock_destroy(server->lock);
    pthread_rwlock_destroy(server->clientsLock);
    pthread_rwlock_destroy(server->channelsLock);
    usersFree(*server->clients);
    free(server->clients);
    channelsFree(*server->channels);
    free(server->channels);
    close(server->socket);
    free(server);
}

void acceptClients(serv *server){
    int socket = server->socket;

    struct sockaddr_in clientAddr;
    int clientSocket;

    pthread_attr_t worker_attr;
    pthread_attr_init(&worker_attr);
    pthread_attr_setdetachstate(&worker_attr, PTHREAD_CREATE_DETACHED);
    pthread_t worker_thread;

    socklen_t sinSizeClient = sizeof(struct sockaddr_in);

    /*the accept loop*/
    while(1){
        if((clientSocket = accept(socket, (struct sockaddr *) &clientAddr, &sinSizeClient)) < 0){
            perror("ERROR: Could not accept client");
            continue;
        }

        /*resolve the clients host name*/
        bstring clientHostName = safe_BfromCstr("");
        int rc = getnameinfo((struct sockaddr *) &clientAddr, sinSizeClient,
                    charBuffromBstr(clientHostName), IRC_MAX_LEN+1, NULL, 0, 0);
        clientHostName->slen = strlen(charBuffromBstr(clientHostName));
        if (rc != 0 || !clientHostName->slen){
            bdestroy(clientHostName);
            clientHostName = safe_BfromCstr("unresolved");
        }
        /*init the client*/
        cli *newClient = initClient(clientSocket, clientHostName, server);
        /*create thread to wait for the client to register and then handle client request*/
        if (pthread_create(&worker_thread, &worker_attr, &enterRegister, newClient) != 0){
            clientFree(newClient);
            close(socket);
            errnoExit("ERROR: Could not create thread");
        }
    }

    /*free server*/
    serverFree(server);
}
