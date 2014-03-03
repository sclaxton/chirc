#include <stdio.h>
#include <unistd.h>
#include "util.h"
#include "hash.h"
#include "message.h"
#include "dispatch.h"
#include "handlers.h"
#include "reply.h"


cli *initClient(int socket, bstring hostName, serv *server){
    cli *newClient = allocaZam(sizeof(cli));
    newClient->socket = socket;
    newClient->user = NULL;
    newClient->nick = NULL;
    newClient->fullName = NULL;
    newClient->server = server;
    newClient->hostName = hostName;
    newClient->modes = safe_BfromCstr("");
    newClient->away = NULL;
    newClient->channels = NULL;
    /*init the client lock to serialize simultaneous writes to the clients socket*/
    pthread_mutex_t *clientLock = (pthread_mutex_t*)allocaZam(sizeof(pthread_mutex_t));
    pthread_mutex_init(clientLock, NULL);
    newClient->clientLock = clientLock;
    incrementServerConnections(server);
    return newClient;
}

void clientFree(cli *client){
    serv *server = client->server;
    decrementServerConnections(server);
    if(!(client->nick && client->user)){
        deleteUser(client);
    }
    close(client->socket);
    bdestroy(client->nick);
    bdestroy(client->user);
    bdestroy(client->fullName);
    bdestroy(client->hostName);
    pthread_mutex_destroy(client->clientLock);
    free(client);
}

void handleMessages (blist messageList, cli *client){
    msg *parsed;
    bstring *enteries = messageList->entry;
    int len = messageList->qty;
    int i;
    for(i = 0; i < len; i++){
        parsed = parseMessage(enteries[i]);
        dispatchUser(parsed, client);
    }
}

void handleUser(cli *client, blist leftOverMessages, bstring incompleteMessage){
    /*take care of any messages left over from registration*/
    if (leftOverMessages->qty){
        handleMessages(leftOverMessages, client);
    }
    bstrListDestroy(leftOverMessages);

    blist messages;
    bstring raw;

    incompleteMessage = incompleteMessage ? incompleteMessage : safe_BfromCstr("");

    /*recv messages until you receive a nick and user msg*/
    while (1){
        raw = safeRecv(client);
        messages = extractMessages(raw, &incompleteMessage);
        if (messages){
            handleMessages(messages, client);
            bstrListDestroy(messages);
        }
    }
    /*exit pthread*/
}

void registerClient(cli *client, blist messages, bstring incompleteMessage){
    int rc = addUser(client);
    fprintf(stderr, "here\n");
    if (rc < 0){
        send_ERR_NICKNAMEINUSE(client, client->nick);
        client->nick = NULL;
        wait4Register(client, messages, incompleteMessage);
    }
    else {
        /*send welcome messages*/
        send_RPL_WELCOME(client);
        send_RPL_YOURHOST(client);
        send_RPL_CREATED(client);
        send_RPL_MYINFO(client);
        handleUser_LUSERS(NULL, client);
        send_ERR_NOMOTD(client);

        handleUser(client, messages, incompleteMessage);
    }
}

int  handleRegisterMessages(blist messages, cli *client){
    int i = -1;
    if (messages){
        for(i = 0; (i < messages->qty) && !(client->nick && client->user); i++){
            msg *parsedMessage = parseMessage(messages->entry[i]);
            dispatchRegister(parsedMessage, client);
        }
    }
    return i;
}

void wait4Register(cli *client, blist messages, bstring incompleteMessage){
    incompleteMessage = incompleteMessage ? incompleteMessage : safe_BfromCstr("");

    /*recv messages until you receive a nick and user msg*/
    bstring raw;
    int i = handleRegisterMessages(messages, client);
    while (!(client->nick && client->user)){
        raw = safeRecv(client);
        messages = extractMessages(raw, &incompleteMessage);
        i = handleRegisterMessages(messages, client);
    }

    /*trim message list to be the list of messages recv'd*/
    /*after the client completed registration*/
    blistDeleteRange(messages, 0, i);

    /*complete client registration*/
    registerClient(client, messages, incompleteMessage);

}

void *enterRegister(void *_client){
    cli *client = (cli *) _client;
    wait4Register(client, NULL, NULL);
    return NULL;
}
