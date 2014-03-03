#ifndef CHANNEL_H_
#define CHANNEL_H_

#include "client.h"
#include "uthash/uthash.h"

typedef struct {
    bstring nick, modes;
    cli *user;
    UT_hash_handle hh;
} chnlUsr;


typedef struct chnl {
    bstring name, modes, topic;
    int numUsers;
    serv *server;
    pthread_rwlock_t *lock;
    chnlUsr **table;
    UT_hash_handle hh;
} chnl;


chnl *initChannel(bstring name, serv *server);
chnlUsr *initChannelUser(cli *client);
void channelUserFree(chnlUsr *channel);
void channelFree(chnl *channel);

#endif
