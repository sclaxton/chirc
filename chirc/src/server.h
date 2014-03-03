#ifndef SERVER_H_
#define SERVER_H_

#include <pthread.h>
#include "bstring/bstrlib.h"
#include "irc_strings.h"

//DATA STRUCTURES

struct cli;
struct chnl;

typedef struct serv {
    int socket;
    bstring name, created, version, userModes, channelModes, info, password;
    int numClients, numConnections, numChannels;
    pthread_rwlock_t *lock, *clientsLock, *channelsLock;
    struct cli **clients;
    struct chnl **channels;
} serv;

//FUNCTION DECLARATIONS

serv *initServer(char *port, char *password);
void acceptClients(serv *server);
void incrementServerConnections(serv *server);
void incrementServerClients(serv *server);
void decrementServerClients(serv *server);
void decrementServerConnections(serv *server);

#endif
