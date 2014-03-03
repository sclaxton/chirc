#ifndef CLIENT_H_
#define CLIENT_H_

#include "server.h"
#include "uthash/uthash.h"
#include "irc_strings.h"

typedef struct cli {
    int socket;
    bstring user, nick, fullName, hostName, channels, modes, away;
    serv *server;
    pthread_mutex_t *clientLock;
    UT_hash_handle hh;
} cli;

cli *initClient(int socket, bstring hostName, serv *server);
void clientFree(cli *client);
void wait4Register(cli *client, blist messages, bstring incompleteMessage);
void *enterRegister(void *_client);
int  handleRegisterMessages(blist messages, cli *client);
void registerClient(cli *client, blist messages, bstring incompleteMessage);
void handleUser(cli *client, blist leftOverMessages, bstring incompleteMessage);
void handleMessages (blist messageList, cli *client);

#endif
