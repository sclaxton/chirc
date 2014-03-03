#include "dispatch.h"
#include "handlers.h"

typedef void (*handler_function)(msg *message, cli *client);

struct handler_entry {
    char *name;
    handler_function func;
};

#define REGISTERHANDLER_ENTRY(NAME) { #NAME, handleRegister_ ## NAME}
#define USERHANDLER_ENTRY(NAME) { #NAME, handleUser_ ## NAME}

struct handler_entry registerHandlers[] = {
    REGISTERHANDLER_ENTRY(NICK),
    REGISTERHANDLER_ENTRY(USER),
    REGISTERHANDLER_ENTRY(QUIT),
};

struct handler_entry userHandlers[] = {
    USERHANDLER_ENTRY(PRIVMSG),
    USERHANDLER_ENTRY(NOTICE),
    USERHANDLER_ENTRY(NICK),
    USERHANDLER_ENTRY(USER),
    USERHANDLER_ENTRY(QUIT),
    USERHANDLER_ENTRY(LUSERS),
    USERHANDLER_ENTRY(WHOIS),
    USERHANDLER_ENTRY(JOIN),
    USERHANDLER_ENTRY(PART),
    USERHANDLER_ENTRY(TOPIC),
    USERHANDLER_ENTRY(OPER),
    USERHANDLER_ENTRY(MODE),
    USERHANDLER_ENTRY(AWAY),
    /*USERHANDLER_ENTRY(MOTD),*/
    /*USERHANDLER_ENTRY(PING),*/
    /*USERHANDLER_ENTRY(PONG),*/

};

int num_registerHandlers = sizeof(registerHandlers) / sizeof(struct handler_entry);
int num_userHandlers = sizeof(userHandlers) / sizeof(struct handler_entry);

void dispatch(msg *message, cli *client, struct handler_entry *table, int num_handlers){
    char *command = charBuffromBstr(message->command);
    int i;
    for(i=0; i < num_handlers; i++){
        if (!strcmp(table[i].name, command)){
            table[i].func(message, client);
            break;
        }
        if(i == num_handlers){
            /*invalid command or not registered*/
        }
    }
}

/*the dispatch table for an unregistered client*/
void dispatchRegister(msg *message, cli *client){
    dispatch(message, client, registerHandlers, num_registerHandlers);
}

/*the dispatch table for handling a registered user*/
void dispatchUser(msg *message, cli *client){
    dispatch(message, client, userHandlers, num_userHandlers);
}
