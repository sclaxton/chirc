#ifndef HANDLERS_H_
#define HANDLERS_H_

#include "client.h"
#include "message.h"

void handleRegister_NICK(msg *message, cli *client);
void handleRegister_USER(msg *message, cli *client);
void handleRegister_QUIT(msg *message, cli *client);

void handleUser_NICK(msg *message, cli *client);
void handleUser_QUIT(msg *message, cli *client);
void handleUser_USER(msg *message, cli *client);
void handleUser_PRIVMSG(msg *message, cli *client);
void handleUser_NOTICE(msg *message, cli *client);
void handleUser_LUSERS(msg *message, cli *client);
void handleUser_WHOIS(msg *message, cli *client);
void handleUser_JOIN(msg *message, cli *client);
void handleUser_PART(msg *message, cli *client);
void handleUser_TOPIC(msg *message, cli *client);
void handleUser_OPER(msg *message, cli *client);
void handleUser_MODE(msg *message, cli *client);
void handleUser_AWAY(msg *message, cli *client);

#endif
