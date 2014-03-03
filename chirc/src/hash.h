#include "uthash/uthash.h"
#include "bstring/bstrlib.h"
#include "client.h"
#include "channel.h"

typedef cli *cliTbl;
typedef chnlUsr *chnlUsrTbl;
typedef chnl *chnlTbl;

//FUNCTIONS

int addUser(cli *client);
cli *findUser(cliTbl table, bstring nick, pthread_rwlock_t *tableLock);
void deleteUser(cli *client);
void usersFree(cliTbl table);
chnlUsr *addChannelUser(cli *user, chnl *channel);
chnlUsr *findChannelUser(chnlUsrTbl table, bstring nick, pthread_rwlock_t *tableLock);
void deleteChannelUser(chnlUsr *user, chnl *channel);
void channelUsersFree(chnlUsrTbl table);
int addChannel(chnl *channel);
chnl *findChannel(chnlTbl table, bstring nick, pthread_rwlock_t *tableLock);
void deleteChannel(chnl *channel);
void channelsFree(chnlTbl table);

