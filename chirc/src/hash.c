#include <stdio.h>
#include "hash.h"
#include "server.h"

int addUser(cli *client){
    serv *server = client->server;
    bstring nick = client->nick;
    cliTbl table = *(server->clients);
    pthread_rwlock_t *tableLock = server->clientsLock;

    /*check first to see if the clients nick exists yet*/
    cli *existingUser;
    existingUser = findUser(table, nick, tableLock);

    /*if the user already exists*/
    if(existingUser){
        return -1;
    }
    else {
        /*lock the table for writing*/
        pthread_rwlock_wrlock(tableLock);

        /*add key to the table*/
        HASH_ADD_KEYPTR(hh, table, charBuffromBstr(nick), (nick)->slen, client);

        /*update table*/
        *(server->clients) = table;

        pthread_rwlock_unlock(tableLock);

        serv *server = client->server;

        incrementServerClients(server);

        return 0;
    }
}

void deleteUser(cli *client) {
    serv *server = client->server;
    cliTbl table = *(server->clients);
    pthread_rwlock_t *tableLock = server->clientsLock;
    pthread_rwlock_wrlock(tableLock);
    HASH_DEL(table, client);
    *(server->clients) = table;
    pthread_rwlock_unlock(tableLock);
    decrementServerClients(server);
    clientFree(client);
}

void usersFree(cliTbl table) {
    cli *current_user, *tmp;

    /*iteratively delete the entire table, freeing users as you go*/
    HASH_ITER(hh, table, current_user, tmp) {
        HASH_DEL(table, current_user);
        clientFree(current_user);
    }
}

cli *findUser(cliTbl table, bstring nick, pthread_rwlock_t *tableLock){
    cli *existingUser;
    char *nickStr = charBuffromBstr(nick);
    pthread_rwlock_rdlock(tableLock);
    HASH_FIND_STR(table, nickStr, existingUser);
    pthread_rwlock_unlock(tableLock);
    return existingUser;
}

chnlUsr *addChannelUser(cli *user, chnl *channel){
    bstring nick = user->nick;
    chnlUsrTbl table = *(channel->table);
    pthread_rwlock_t *tableLock = channel->lock;

    /*check first to see if the clients nick exists yet*/
    chnlUsr *existingUser;
    existingUser = findChannelUser(table, nick, tableLock);

    /*if the user already exists*/
    if(existingUser){
        return existingUser;
    }
    else {

        chnlUsr *channelUser = initChannelUser(user);
        /*lock the table for writing*/
        pthread_rwlock_wrlock(tableLock);

        /*add key to the table*/
        HASH_ADD_KEYPTR(hh, table, charBuffromBstr(nick), nick->slen, channelUser);

        /*update table*/
        *(channel->table) = table;
        channel->numUsers++;

        pthread_rwlock_unlock(tableLock);

        return channelUser;
    }
}

void deleteChannelUser(chnlUsr *user, chnl *channel) {
    chnlUsrTbl table = *(channel->table);
    pthread_rwlock_t *tableLock = channel->lock;
    pthread_rwlock_wrlock(tableLock);
    HASH_DEL(table, user);
    *(channel->table) = table;
    channel->numUsers--;
    pthread_rwlock_unlock(tableLock);
    channelUserFree(user);
}

void channelUsersFree(chnlUsrTbl table) {
    chnlUsr *current_user, *tmp;

    /*iteratively delete the entire table, freeing users as you go*/
    HASH_ITER(hh, table, current_user, tmp) {
        HASH_DEL(table, current_user);
        channelUserFree(current_user);
    }
}

chnlUsr *findChannelUser(chnlUsrTbl table, bstring nick, pthread_rwlock_t *tableLock){
    chnlUsr *existingUser;
    char *nickStr = charBuffromBstr(nick);
    pthread_rwlock_rdlock(tableLock);
    HASH_FIND_STR(table, nickStr, existingUser);
    pthread_rwlock_unlock(tableLock);
    return existingUser;
}

int addChannel(chnl *channel){
    serv *server = channel->server;
    bstring name = channel->name;
    chnlTbl table = *(server->channels);
    pthread_rwlock_t *tableLock = server->channelsLock;

    /*check first to see if the clients nick exists yet*/
    chnl *existingChannel;
    existingChannel = findChannel(table, name, tableLock);

    /*if the user already exists*/
    if(existingChannel){
        return -1;
    }
    else {

        /*lock the table for writing*/
        pthread_rwlock_wrlock(tableLock);

        /*add key to the table*/
        HASH_ADD_KEYPTR(hh, table, charBuffromBstr(name), name->slen, channel);

        /*update table*/
        *(server->channels) = table;

        pthread_rwlock_unlock(tableLock);

        return 0;
    }
}

chnl *findChannel(chnlTbl table, bstring name, pthread_rwlock_t *tableLock){
    chnl *existingChannel;
    char *nameStr = charBuffromBstr(name);
    pthread_rwlock_rdlock(tableLock);
    HASH_FIND_STR(table, nameStr, existingChannel);
    pthread_rwlock_unlock(tableLock);
    return existingChannel;
}

void deleteChannel(chnl *channel){
    serv *server = channel->server;
    chnlTbl table = *(server->channels);
    pthread_rwlock_t *tableLock = server->channelsLock;
    pthread_rwlock_wrlock(tableLock);
    HASH_DEL(table, channel);
    *(server->channels) = table;
    pthread_rwlock_unlock(tableLock);
    channelFree(channel);
}

void channelsFree(chnlTbl table) {
    chnl *current_channel, *tmp;

    /*iteratively delete the entire table, freeing users as you go*/
    HASH_ITER(hh, table, current_channel, tmp) {
        HASH_DEL(table, current_channel);
        channelFree(current_channel);
    }
}
