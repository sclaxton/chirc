#include "channel.h"
#include "server.h"
#include "util.h"
#include "hash.h"


chnl *initChannel(bstring name, serv *server){
    chnl *newChannel = allocaZam(sizeof(chnl));
    newChannel->server = server;
    newChannel->name = bstrcpy(name);
    newChannel->modes = safe_BfromCstr("");
    newChannel->topic = NULL;
    newChannel->lock = newRwlock();
    chnlUsrTbl *channelUserTable = (chnlUsrTbl*) allocaZam(sizeof(chnlUsrTbl));
    *channelUserTable = NULL;
    newChannel->table = channelUserTable;
    return newChannel;
}

void channelFree(chnl *channel){
    bdestroy(channel->name);
    bdestroy(channel->modes);
    channelUsersFree(*channel->table);
    pthread_rwlock_destroy(channel->lock);
    free(channel->table);
}

chnlUsr *initChannelUser(cli *client){
    chnlUsr *newChannelUser = allocaZam(sizeof(chnlUsr));
    newChannelUser->nick = client->nick;
    newChannelUser->modes = safe_BfromCstr("");
    newChannelUser->user = client;
    return newChannelUser;
}

void channelUserFree(chnlUsr *user){
    bdestroy(user->modes);
    free(user);
}
