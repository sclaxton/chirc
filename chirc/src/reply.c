#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include "uthash/uthash.h"
#include "util.h"
#include "handlers.h"
#include "server.h"
#include "irc_strings.h"
#include "constants.h"
#include "reply.h"


/*helper to produce the full userid of a registered user*/
bstring getFullUserId(cli *client){
    bstring nick = client->nick;
    bstring user = client->user;
    bstring hostName = client->hostName;
    bstring idBuff = safe_BfromCstr("");
    sprintf(charBuffromBstr(idBuff), "%s!%s@%s", charBuffromBstr(nick),
            charBuffromBstr(user), charBuffromBstr(hostName));
    return idBuff;
}

/*helper function that provides general frame work for REPL message*/
void _send(cli *client, char *rplNum, char *toSend){
    serv *server =  client->server;
    bstring nick = client->nick;
    char messageBuff[513];
    sprintf(messageBuff, ":%s %s %s %s\r\n",
            charBuffromBstr(server->name), rplNum, charBuffromBstr(nick), toSend);
    safeSend(client, messageBuff);
}



/*REPL messages to be sent in response to corresponding commands*/

void send_RPL_WELCOME(cli *client){
    bstring fullId = getFullUserId(client);
    char messageBuff[513];
    sprintf(messageBuff, ":Welcome to the Internet Relay Network %s", charBuffromBstr(fullId));
    _send(client, REPL_WELCOME, messageBuff);
}

void send_RPL_YOURHOST(cli *client){
    serv *server = client->server;
    bstring name = server->name;
    bstring version = server->version;
    char messageBuff[513];
    sprintf(messageBuff, ":Your host is %s, running version %s",
            charBuffromBstr(name), charBuffromBstr(version));
    _send(client, RPL_YOURHOST, messageBuff);
}

void send_RPL_CREATED(cli *client){
    serv *server = client->server;
    bstring created = server->created;
    char messageBuff[513];
    sprintf(messageBuff, ":This server was created %s", charBuffromBstr(created));
    _send(client, RPL_CREATED, messageBuff);
}

void send_RPL_MYINFO(cli *client){
    serv *server = client->server;
    bstring name = server->name;
    bstring version = server->version;
    bstring userModes = server->userModes;
    bstring channelModes = server->channelModes;
    char messageBuff[513];
    sprintf(messageBuff, "%s %s %s %s", charBuffromBstr(name), charBuffromBstr(version),
            charBuffromBstr(userModes), charBuffromBstr(channelModes));
    _send(client, RPL_MYINFO, messageBuff);
}

void send_RPL_LUSERCLIENT(cli *client, int numClients){
    char messageBuff[513];
    sprintf(messageBuff, ":There are %d users and 0 services on 1 servers", numClients);
    _send(client, RPL_LUSERCLIENT, messageBuff);
}

void send_RPL_LUSEROP(cli *client){
    _send(client, RPL_LUSEROP, "0 :operator(s) online");
}

void send_RPL_LUSERUNKNOWN(cli *client, int numUnknown){
    char messageBuff[513];
    sprintf(messageBuff, "%d :unknown connection(s)", numUnknown);
    _send(client, RPL_LUSERUNKNOWN, messageBuff);
}

void send_RPL_LUSERCHANNELS(cli *client){
    _send(client, RPL_LUSERCHANNELS, "0 :channels formed");
}

void send_RPL_LUSERME(cli *client, int numConnections){
    char messageBuff[513];
    sprintf(messageBuff, ":I have %d clients and 1 servers", numConnections);
    _send(client, RPL_LUSERME, messageBuff);
}

void send_RPL_WHOISUSER(cli *sender, cli *queried){
    bstring nick = queried->nick;
    bstring user = queried->user;
    bstring host = queried->hostName;
    bstring fullName = queried->fullName;
    char messageBuff[513];
    sprintf(messageBuff, "%s %s %s * :%s", charBuffromBstr(nick),
            charBuffromBstr(user), charBuffromBstr(host), charBuffromBstr(fullName));
    _send(sender, RPL_WHOISUSER, messageBuff);
}

void send_RPL_WHOISSERVER(cli *sender, cli *queried){
    bstring nick = queried->nick;
    serv *server = queried->server;
    bstring serverName = server->name;
    bstring serverInfo = server->info;
    char messageBuff[513];
    sprintf(messageBuff, "%s %s :%s", charBuffromBstr(nick),
            charBuffromBstr(serverName), charBuffromBstr(serverInfo));
    _send(sender, RPL_WHOISSERVER, messageBuff);
}

void send_RPL_ENDOFWHOIS(cli *sender, cli *queried){
    bstring nick = queried->nick;
    char messageBuff[513];
    sprintf(messageBuff, "%s :End of WHOIS list", charBuffromBstr(nick));
    _send(sender, RPL_ENDOFWHOIS, messageBuff);
}

void send_RPL_NAMREPLY(cli *client, chnl *channel){
    bstring channelName = channel->name;
    char messageBuff[513];
    sprintf(messageBuff, "= %s :foobar1 foobar2 foobar3", charBuffromBstr(channelName));
    _send(client, RPL_NAMREPLY, messageBuff);
}

void send_RPL_ENDOFNAMES(cli *client, chnl *channel){
    bstring channelName = channel->name;
    char messageBuff[513];
    sprintf(messageBuff, "%s :End of NAMES list", charBuffromBstr(channelName));
    _send(client, RPL_ENDOFNAMES, messageBuff);
}

void send_RPL_TOPIC(cli *client, bstring channelName, bstring channelTopic){
    char messageBuff[513];
    sprintf(messageBuff, "%s :%s", charBuffromBstr(channelName), charBuffromBstr(channelTopic));
    _send(client, RPL_TOPIC, messageBuff);
}

void send_RPL_NOTOPIC(cli *client, bstring channelName){
    char messageBuff[513];
    sprintf(messageBuff, "%s :No topic is set", charBuffromBstr(channelName));
    _send(client, RPL_NOTOPIC, messageBuff);
}

void send_RPL_YOUREOPER(cli *client){
    _send(client, RPL_YOUREOPER, ":You are now an IRC operator");
}

void send_RPL_CHANNELMODEIS(cli *client, bstring channelName, bstring channelMode){
    char messageBuff[513];
    sprintf(messageBuff, "%s +%s", charBuffromBstr(channelName), charBuffromBstr(channelMode));
    _send(client, RPL_CHANNELMODEIS, messageBuff);
}

void send_RPL_AWAY(cli *client, bstring nick, bstring message){
    char messageBuff[513];
    sprintf(messageBuff, "%s :%s", charBuffromBstr(nick), charBuffromBstr(message));
    _send(client, RPL_AWAY, messageBuff);
}

void send_RPL_UNAWAY(cli *client){
    _send(client, RPL_UNAWAY, ":You are no longer marked as being away");
}

void send_RPL_NOWAWAY(cli *client){
    _send(client, RPL_NOWAWAY, ":You have been marked as being away");
}

/*ERR messages to be sent when a user command results in an error*/

void send_ERR_NOMOTD(cli *client){
    char *tosend = ":MOTD File is missing";
    _send(client, ERR_NOMOTD, tosend);
}

void send_ERR_NICKNAMEINUSE(cli *client, bstring nick){
    serv *server = client->server;
    bstring clientNick = client->nick ? client->nick : safe_BfromCstr("*");
    char messageBuff[513];
    sprintf(messageBuff, ":%s %s %s %s :Nickname is already in use\r\n",
            charBuffromBstr(server->name), ERR_NICKNAMEINUSE,
            charBuffromBstr(clientNick), charBuffromBstr(nick));
    safeSend(client, messageBuff);
}

void send_ERR_ALREADYREGISTERED(cli *client){
    _send(client, ERR_ALREADYREGISTRED, ":Unauthorized command (already registered)");
}

void send_ERR_NOSUCHNICK(cli *client, bstring nick){
    char messageBuff[513];
    sprintf(messageBuff, "%s :No such nick/channel", charBuffromBstr(nick));
    _send(client, ERR_NOSUCHNICK, messageBuff);
}

void send_ERR_CANNOTSENDTOCHAN(cli *client, bstring channelName){
    char messageBuff[513];
    sprintf(messageBuff, "%s :Cannot send to channel", charBuffromBstr(channelName));
    _send(client, ERR_CANNOTSENDTOCHAN, messageBuff);
}

void send_ERR_NOTONCHANNEL(cli *client, bstring channelName){
    char messageBuff[513];
    sprintf(messageBuff, "%s :You're not on that channel", charBuffromBstr(channelName));
    _send(client, ERR_NOTONCHANNEL, messageBuff);
}

void send_ERR_NOSUCHCHANNEL(cli *client, bstring channelName){
    char messageBuff[513];
    sprintf(messageBuff, "%s :No such channel", charBuffromBstr(channelName));
    _send(client, ERR_NOSUCHCHANNEL, messageBuff);
}

void send_ERR_PASSWDMISMATCH(cli *client){
    _send(client, ERR_PASSWDMISMATCH, ":Password incorrect");
}

void send_ERR_UMODEUNKOWNFLAG(cli *client){
    _send(client, ERR_UMODEUNKNOWNFLAG, ":Unknown MODE flag");
}

void send_ERR_USERSDONTMATCH(cli *client){
    _send(client, ERR_USERSDONTMATCH, ":Cannot change mode for other users");
}

void send_ERR_UNKOWNMODE(cli *client, char modeChar, bstring channelName){
    char messageBuff[513];
    sprintf(messageBuff, "%c :is unknown mode char to me for %s", modeChar, charBuffromBstr(channelName));
    _send(client, ERR_UNKNOWNMODE, messageBuff);
}

void send_ERR_CHANOPRIVSNEEDED(cli *client, bstring channelName){
    char messageBuff[513];
    sprintf(messageBuff, "%s :You're not channel operator", charBuffromBstr(channelName));
    _send(client, ERR_CHANOPRIVSNEEDED, messageBuff);
}

void send_ERR_USERNOTINCHANNEL(cli *client, bstring nick, bstring channelName){
    char messageBuff[513];
    sprintf(messageBuff, "%s %s :They aren't on that channel", charBuffromBstr(nick), charBuffromBstr(channelName));
    _send(client, ERR_USERNOTINCHANNEL, messageBuff);
}
