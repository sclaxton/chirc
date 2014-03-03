#include <stdio.h>
#include <pthread.h>
#include "util.h"
#include "handlers.h"
#include "client.h"
#include "channel.h"
#include "hash.h"
#include "constants.h"
#include "reply.h"


/*REGISTER HANDLER*/

void handleRegister_NICK(msg *message, cli *client){
    /*nick is the first and only param*/
    bstring nick = (message->params)->entry[0];
    serv *server = client->server;
    cliTbl table = *(server->clients);
    pthread_rwlock_t *tableLock = server->clientsLock;

    /*search for client in the table*/
    cli *existingClient = findUser(table, nick, tableLock);

    /*if the key was not in the table*/
    if (existingClient == NULL){
        client->nick = bstrcpy(nick);
    }
    else {
        send_ERR_NICKNAMEINUSE(client, nick);
    }
}

void handleRegister_USER(msg *message, cli *client){
    client->user = bstrcpy((message->params)->entry[0]);
    client->fullName = message->trailing ? message->trailing : safe_BfromCstr("Mr/Ms Smith");
}

void handleRegister_QUIT(msg *message, cli *client){
    bstring quitMsg = message->trailing;
    char messageBuff[513];
    char *quitMsgStr =  quitMsg ? charBuffromBstr(quitMsg) : "Client Quit";
    sprintf(messageBuff, "ERROR :Closing Link: %s (%s)\r\n",
            charBuffromBstr(client->hostName), quitMsgStr);
    safeSend(client, messageBuff);
    clientFree(client);
    msgFree(message);
    pthread_exit(NULL);
}


/*USER HANDLERS*/


void handleUser_NICK(msg *message, cli *client){
    handleRegister_NICK(message, client);
}

void handleUser_QUIT(msg *message, cli *client){
    handleRegister_QUIT(message, client);
}

void handleUser_USER(msg *message, cli *client){
    send_ERR_ALREADYREGISTERED(client);
}

void sendMessage(bstring sender, cli *recipient, char *type, bstring name, bstring message){
    char messageBuff[513];
    char tmpBuff[513];
    char *tmp = charBuffromBstr(name);
    if (message){
        sprintf(tmpBuff, "%s :%s", tmp, charBuffromBstr(message));
        tmp = tmpBuff;
    }
    sprintf(messageBuff, ":%s %s %s\r\n", charBuffromBstr(sender), type, tmp);
    safeSend(recipient, messageBuff);
}

void sendPrivmsgChannel(chnl *channel, cli *sender, bstring message){
    chnlUsrTbl channelUserTable = *channel->table;
    chnlUsr *curr, *next;
    HASH_ITER(hh, channelUserTable, curr, next){
    if (curr->user != sender){
        sendMessage(getFullUserId(sender), curr->user, "PRIVMSG", channel->name, message);
        }
    }
}

void handleUser_PRIVMSG(msg *message, cli *client){
    bstring recipNick = bstrcpy((message->params)->entry[0]);
    bstring privMessage = bstrcpy(message->trailing);
    serv *server = client->server;
    if (bstrchr(recipNick, '#') == 0){
        chnl *channel = findChannel(*server->channels, recipNick, server->channelsLock);
        if (channel){
            chnlUsrTbl channelUserTable = *channel->table;
            if (findChannelUser(channelUserTable, client->nick, channel->lock)){
                chnlUsr *curr, *next;
                HASH_ITER(hh, channelUserTable, curr, next){
                    if (curr->user != client){
                        sendMessage(getFullUserId(client), curr->user, "PRIVMSG", recipNick, privMessage);
                    }
                }
            }
            else {
                send_ERR_CANNOTSENDTOCHAN(client, recipNick);
            }
        }
        else {
            send_ERR_NOSUCHNICK(client, recipNick);
        }
    }
    else {
        cliTbl table = *(server->clients);
        pthread_rwlock_t *tableLock = server->clientsLock;
        cli *recipient = findUser(table, recipNick, tableLock);
        if (recipient){
            sendMessage(getFullUserId(client), recipient, "PRIVMSG", recipNick, privMessage);
        }
        else {
            send_ERR_NOSUCHNICK(client, recipNick);
        }
    }
}

void sendNoticeChannel(chnl *channel, cli *sender, bstring notice){
    chnlUsrTbl channelUserTable = *channel->table;
    chnlUsr *curr, *next;
    HASH_ITER(hh, channelUserTable, curr, next){
    if (curr->user != sender){
        sendMessage(getFullUserId(sender), curr->user, "NOTICE", channel->name, notice);
        }
    }
}


void handleUser_NOTICE(msg *message, cli *client){
    bstring recipNick = bstrcpy((message->params)->entry[0]);
    bstring notice = bstrcpy(message->trailing);
    serv *server = client->server;
    if (bstrchr(recipNick, '#') == 0){
        chnl *channel = findChannel(*server->channels, recipNick, server->channelsLock);
        if (channel){
            if (bstrchr(channel->modes, 'm') >= 0){
                chnlUsrTbl channelUserTable = *channel->table;
                chnlUsr *curr, *next;
                HASH_ITER(hh, channelUserTable, curr, next){
                    if (curr->user != client){
                        sendMessage(getFullUserId(client), curr->user, "NOTICE", recipNick, notice);
                    }
                }
            }
        }
    }
    cliTbl table = *(server->clients);
    pthread_rwlock_t *tableLock = server->clientsLock;
    cli *recipient = findUser(table, recipNick, tableLock);
    if (recipient){
        sendMessage(getFullUserId(client), recipient, "NOTICE", recipNick, notice);
    }
}

void handleUser_LUSERS(msg *message, cli *client){
    serv *server = client->server;
    pthread_rwlock_t *lock = server->lock;
    pthread_rwlock_rdlock(lock);
    int numConnections = server->numConnections;
    int numClients = server->numClients;
    pthread_rwlock_unlock(server->lock);
    int numUnknown = numConnections - numClients;

    /*send replies*/
    send_RPL_LUSERCLIENT(client, numClients);
    send_RPL_LUSEROP(client);
    send_RPL_LUSERUNKNOWN(client, numUnknown);
    send_RPL_LUSERCHANNELS(client);
    send_RPL_LUSERME(client, numConnections);
}

void handleUser_WHOIS(msg *message, cli *client){
    bstring queriedNick = (message->params)->entry[0];
    serv *server = client->server;
    cliTbl table = *(server->clients);
    pthread_rwlock_t *tableLock = server->clientsLock;
    cli *queriedUser = findUser(table, queriedNick, tableLock);
    if (queriedUser){
        send_RPL_WHOISUSER(client, queriedUser);
        send_RPL_WHOISSERVER(client, queriedUser);
        send_RPL_ENDOFWHOIS(client, queriedUser);
    }
    else{
        send_ERR_NOSUCHNICK(client, queriedNick);
    }
}

void handleUser_JOIN(msg *message, cli *client){
    bstring channelName = (message->params)->entry[0];
    bstring nick = client->nick;
    serv *server = client->server;
    pthread_rwlock_t *serverLock = server->lock;
    chnlTbl table = *(server->channels);
    chnl *existingChannel = findChannel(table, channelName, serverLock);
    if(!existingChannel){
        chnl *newChannel = initChannel(channelName, server);
        addChannel(newChannel);
        existingChannel = newChannel;
        chnlUsr *creator = addChannelUser(client, existingChannel);
        bconchar(creator->modes, 'o');
        pthread_rwlock_t *userTableLock = existingChannel->lock;
        pthread_rwlock_rdlock(userTableLock);
        sendMessage(getFullUserId(client), client, "JOIN", channelName, NULL);
        send_RPL_NAMREPLY(client, existingChannel);
        send_RPL_ENDOFNAMES(client, existingChannel);
        pthread_rwlock_unlock(userTableLock);
    }
    else {
        chnlUsrTbl userTable = *(existingChannel->table);
        pthread_rwlock_t *userTableLock = existingChannel->lock;
        if(!findChannelUser(userTable, nick, userTableLock)){
            chnlUsr *new = addChannelUser(client, existingChannel);
            chnlUsr *curr, *next;
            pthread_rwlock_rdlock(userTableLock);
            HASH_ITER(hh, *(existingChannel->table), curr, next){
                sendMessage(getFullUserId(client), curr->user, "JOIN", channelName, NULL);
            }
            if (existingChannel->topic){
                send_RPL_TOPIC(client, channelName, existingChannel->topic);
            }
            send_RPL_NAMREPLY(client, existingChannel);
            send_RPL_ENDOFNAMES(client, existingChannel);
            pthread_rwlock_unlock(userTableLock);
        }
    }
}

void handleUser_PART(msg *message, cli *client){
    bstring channelName = blistPopIndex(message->params, 0);
    serv *server = client->server;
    chnl *existingChannel = findChannel(*server->channels, channelName, server->channelsLock);
    if (existingChannel){
        pthread_rwlock_t *lock = existingChannel->lock;
        chnlUsr *existingUser = findChannelUser(*existingChannel->table, client->nick, lock);
        if (existingUser){
            bstring partMessage = message->trailing;
            chnlUsr *curr, *next;
            pthread_rwlock_rdlock(lock);
            HASH_ITER(hh, *existingChannel->table, curr, next){
                sendMessage(getFullUserId(client), curr->user, "PART", channelName, partMessage);
            }
            pthread_rwlock_unlock(lock);
            deleteChannelUser(existingUser, existingChannel);
            if (!existingChannel->numUsers){
                printf("%p\n", existingChannel);
                deleteChannel(existingChannel);
            }
        }
        else {
            send_ERR_NOTONCHANNEL(client, channelName);
        }
    }
    else {
        send_ERR_NOSUCHCHANNEL(client, channelName);
    }
}

void handleUser_TOPIC(msg *message, cli *client){
    bstring channelName = blistPopIndex(message->params, 0);
    serv *server = client->server;
    chnl *existingChannel = findChannel(*server->channels, channelName, server->lock);
    if (existingChannel){
        pthread_rwlock_t *lock = existingChannel->lock;
        chnlUsr *existingUser = findChannelUser(*existingChannel->table, client->nick, lock);
        if (existingUser){
            bstring newTopic = message->trailing;
            chnlUsr *curr, *next;
            if (newTopic){
                pthread_rwlock_wrlock(lock);
                existingChannel->topic = newTopic;
                pthread_rwlock_unlock(lock);
                pthread_rwlock_rdlock(lock);
                HASH_ITER(hh, *existingChannel->table, curr, next){
                    sendMessage(getFullUserId(client), curr->user, "TOPIC", channelName, newTopic);
                }
                pthread_rwlock_unlock(lock);
            }
            else if (existingChannel->topic) {
                send_RPL_TOPIC(client, channelName, existingChannel->topic);
            }
            else {
                send_RPL_NOTOPIC(client, channelName);
            }
        }
        else {
            send_ERR_NOTONCHANNEL(client, channelName);
        }
    }
    else {
        send_ERR_NOTONCHANNEL(client, channelName);
    }
}

void handleUser_OPER(msg *message, cli *client){
    bstring password = (message->params)->entry[1];
    serv *server = client->server;
    if (!bstricmp(password, server->password)){
        if (bstrContains(client->modes, "o")){
            bstring operMode = safe_BfromCstr("o");
            safe_bConcat(client->modes, operMode);
            bdestroy(operMode);
        }
        send_RPL_YOUREOPER(client);
    }
    else {
        send_ERR_PASSWDMISMATCH(client);
    }
}

void sendChannelModeMessage(bstring sender, cli *recipient, bstring name, bstring mode){
    char messageBuff[513];
    sprintf(messageBuff, ":%s MODE %s %s\r\n", charBuffromBstr(sender),
            charBuffromBstr(name), charBuffromBstr(mode));
    safeSend(recipient, messageBuff);
}

void sendChannelUserModeMessage(bstring sender, cli *recipient, bstring name, bstring mode, bstring nick){
    char messageBuff[513];
    sprintf(messageBuff, ":%s MODE %s %s %s\r\n", charBuffromBstr(sender),
            charBuffromBstr(name), charBuffromBstr(mode), charBuffromBstr(nick));
    safeSend(recipient, messageBuff);
}

void setUserMode(cli *client, bstring nick, bstring mode){
    bstring takeModes = safe_BfromCstr("o");
    bstring ignoreTakeModes = safe_BfromCstr("a");
    bstring ignoreGrantModes = safe_BfromCstr("ao");
    bstring grantModes = safe_BfromCstr("");
    if ( bstricmp(client->nick, nick) ){
        send_ERR_USERSDONTMATCH(client);
        return;
    }
    if (mode->slen == 2){
        char sign = bchar(mode, 0);
        char flag = bchar(mode, 1);
        if(sign == '+'){
            if (bstrchr(grantModes, flag) >= 0){
                bconchar(client->modes, flag);
            }
            else {
                if (bstrchr(ignoreGrantModes, flag) <  0){
                    send_ERR_UMODEUNKOWNFLAG(client);
                }
                return;
            }
        }
        else if (sign == '-'){
            if (bstrchr(takeModes, flag) >= 0){
                int index;
                if ((index = bstrchr(client->modes, flag)) >= 0){
                        bdelete(client->modes, index, 1);
                }
            }
            else {
                if (bstrchr(ignoreTakeModes, flag) <  0){
                    send_ERR_UMODEUNKOWNFLAG(client);
                }
                return;
            }
        }
        else {
            send_ERR_UMODEUNKOWNFLAG(client);
            return;
        }
        sendMessage(nick, client, "MODE", nick, mode);
    }
    else {
        send_ERR_UMODEUNKOWNFLAG(client);
        return;
    }
}

void setChannelMode(cli *oper, chnl *channel, bstring name, bstring mode){
    bstring takeModes = safe_BfromCstr("mt");
    bstring grantModes = safe_BfromCstr("mt");
    if (mode->slen == 2){
        char sign = bchar(mode, 0);
        char flag = bchar(mode, 1);
        if(sign == '+'){
            if (bstrchr(grantModes, flag) >= 0){
                if (bstrchr(channel->modes, flag) < 0){
                    bconchar(channel->modes, flag);
                }
            }
            else {
                send_ERR_UNKOWNMODE(oper, flag, name);
                return;
            }
        }
        else if (sign == '-'){
            if (bstrchr(takeModes, flag) >= 0){
                int index;
                if ((index = bstrchr(channel->modes, flag)) >= 0){
                        bdelete(channel->modes, index, 1);
                }
            }
            else {
                send_ERR_UNKOWNMODE(oper, flag, name);
                return;
            }
        }
        else {
            send_ERR_UNKOWNMODE(oper, flag, name);
            return;
        }
        pthread_rwlock_rdlock(channel->lock);
        chnlUsr *curr, *next;
        HASH_ITER(hh, *channel->table, curr, next){
            sendChannelModeMessage(getFullUserId(oper), curr->user, name, mode);
        }
        pthread_rwlock_unlock(channel->lock);
    }
    else {
    }
}

void setChannelUserMode(cli *oper, chnl *channel, chnlUsr *user, bstring mode, bstring nick){
    bstring takeModes = safe_BfromCstr("ov");
    bstring grantModes = safe_BfromCstr("ov");
    if (mode->slen == 2){
        char sign = bchar(mode, 0);
        char flag = bchar(mode, 1);
        if(sign == '+'){
            if (bstrchr(grantModes, flag) >= 0){
                if (bstrchr(user->modes, flag) < 0){
                    printf("here\n");
                    bconchar(user->modes, flag);
                }
            }
            else {
                send_ERR_UNKOWNMODE(oper, flag, channel->name);
                return;
            }
        }
        else if (sign == '-'){
            if (bstrchr(takeModes, flag) >= 0){
                int index;
                if ((index = bstrchr(user->modes, flag)) >= 0){
                        bdelete(user->modes, index, 1);
                }
            }
            else {
                send_ERR_UNKOWNMODE(oper, flag, channel->name);
                return;
            }
        }
        else {
            send_ERR_UNKOWNMODE(oper, flag, channel->name);
            return;
        }
        pthread_rwlock_rdlock(channel->lock);
        chnlUsr *curr, *next;
        HASH_ITER(hh, *channel->table, curr, next){
            sendChannelUserModeMessage(getFullUserId(oper), curr->user, channel->name, mode, nick);
        }
        pthread_rwlock_unlock(channel->lock);
    }
    else {
    }
}

void handleUser_MODE(msg *message, cli *client){
    blist params = message->params;
    serv *server = client->server;
    bstring name = params->entry[0];
    if (params->qty == 1){
        if (bstrchr(name, '#') == 0){
            chnl *channel = findChannel(*server->channels, name, server->channelsLock);
            if (channel){
                send_RPL_CHANNELMODEIS(client, name, channel->modes);
            }
            else {
                send_ERR_NOSUCHCHANNEL(client, name);
            }
        }
    }
    else if (params->qty == 2){
        bstring mode = params->entry[1];
        if (bstrchr(name, '#') == 0){
            chnl *channel = findChannel(*server->channels, name, server->channelsLock);
            if (channel){
                chnlUsr *channelUser = findChannelUser(*channel->table, client->nick, channel->lock);
                if (channelUser){
                    if (bstrchr(channelUser->modes, 'o') >= 0){
                        setChannelMode(client, channel, name, mode);
                    }
                    else {
                        send_ERR_CHANOPRIVSNEEDED(client, name);
                    }
                }
                else {
                    if (bstrchr(client->modes, 'o') >= 0){
                        setChannelMode(client, channel, name, mode);
                    }
                    else {
                        send_ERR_CHANOPRIVSNEEDED(client, name);
                    }
                }
            }
            else {
                send_ERR_NOSUCHCHANNEL(client, name);
            }
        }
        else {
            setUserMode(client, name, mode);
        }
    }
    else if (params->qty == 3){
        bstring mode = params->entry[1];
        bstring nick = params->entry[2];
        printf("here\n");
        if (bstrchr(name, '#') == 0){
            chnl *channel = findChannel(*server->channels, name, server->channelsLock);
            if (channel){
                chnlUsr *channelUserOper = findChannelUser(*channel->table, client->nick, channel->lock);
                chnlUsr *channelUser = findChannelUser(*channel->table, nick, channel->lock);
                if (channelUser){
                    if (channelUserOper){
                        if (bstrchr(channelUserOper->modes, 'o') >= 0){
                            setChannelUserMode(client, channel, channelUser, mode, nick);
                        }
                            else {
                            send_ERR_CHANOPRIVSNEEDED(client, name);
                        }
                    }
                    else {
                        if (bstrchr(client->modes, 'o') >= 0){
                            setChannelUserMode(client, channel, channelUser, mode, nick);
                        }
                        else {
                            send_ERR_CHANOPRIVSNEEDED(client, name);
                        }
                    }
                }
                else {
                    send_ERR_USERNOTINCHANNEL(client, nick, name);
                }
            }
            else {
                send_ERR_NOSUCHCHANNEL(client, name);
            }
        }
    }
    else {
    }
}
