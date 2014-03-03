#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "bstring/bstrlib.h"
#include "irc_strings.h"

typedef struct {
    bstring prefix, command, trailing;
    blist params;
} msg;

blist extractMessages(bstring recv, bstring *incompletePtr);
msg *parseMessage(bstring message);
void msgFree(msg *message);
void printMessage(msg *message);

#endif
