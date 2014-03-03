#include <stdio.h>
#include "constants.h"
#include "bstring/bstrlib.h"
#include "irc_strings.h"
#include "server.h"
#include "util.h"
#include "message.h"

blist extractMessages(bstring recv, bstring *incompletePtr){
    int len;
    bstring incompleteMessage = *incompletePtr;
    if (recv->slen){
        /*split string on message boundaries*/
        blist messageList = safe_splitBfromCstr(recv, END_MSG);

        /*this is the case where we read only a fragment of a message*/
        if ((len = messageList->qty) == 1){
            bconcat(incompleteMessage, (messageList->entry)[0]);
            bstrListDestroy(messageList);
            return NULL;
        }
        /*there is at least one message*/
        else {
            /*the first message in the list will be the*/
            /*continuation of the last incomplete message*/
            bstring first = (messageList->entry)[0];

            /*combine the last incomplete message and the*/
            /*first in the list to obtain a whole message*/
            safe_bConcat(incompleteMessage, first);

            /*this message goes first in our message list now*/
            (messageList->entry)[0] = incompleteMessage;

            /*free the old first in the list*/
            bdestroy(first);

            /*now the last message in the list becomes our*/
            /*incomplete message*/
            bstring last = (messageList->entry)[len-1];
            *incompletePtr = last;

            /*cut off the last element and*/
            /*return a list of one or more complete messages*/
            messageList->qty = len - 1;
            return messageList;
        }
    }
    return NULL;
}

msg *parseMessage(bstring message){
    /*trim white space before and after message*/
    bltrimws(message);

    bstring prefixCommandParams, commandParams, command;
    blist params, commandParamsList, prefixCommandParamsList;
    bstring trailing = NULL;
    bstring prefix = NULL;

    /*split raw message string by the ':' character*/
    blist extractPrefixTrailing = bsplit(message, TRAIL);

    /*if the first entry in the resulting split list is the empty string*/
    /*this means that ':' was the first character and that there is a prefix*/
    if ((extractPrefixTrailing->entry[0])->slen == 0){
        /*get rid of the empty string*/
        blistDeleteRange(extractPrefixTrailing, 0, 1);

        /*pop the string that contains the prefix, command and parameters*/
        prefixCommandParams = blistPopIndex(extractPrefixTrailing, 0);

        /*split the string into the seprate strings*/
        prefixCommandParamsList = safe_bSplit(prefixCommandParams, SPACE);

        /*first is the prefix*/
        prefix = blistPopIndex(prefixCommandParamsList, 0);

        /*then the commands*/
        command = blistPopIndex(prefixCommandParamsList, 0);

        /*finally the rest of the list of strings is the parameters*/
        params = prefixCommandParamsList->qty ? prefixCommandParamsList : NULL;
    }
    /*if there is no prefix*/
    else {
        commandParams = blistPopIndex(extractPrefixTrailing, 0);
        commandParamsList = safe_bSplit(commandParams, SPACE);
        command = blistPopIndex(commandParamsList, 0);
        params = commandParamsList->qty ? commandParamsList : NULL;
    }
    /*if there is a trailing parameter then trailing will be set to that string*/
    /*if not trailing will be set to NULL*/
    trailing = blistPopIndex(extractPrefixTrailing, 0);

    /*package up the parsed message into a struct*/
    msg *parsed = malloc(sizeof(msg));
    parsed->prefix = prefix;
    parsed->command = command;
    parsed->params = params;
    parsed->trailing = trailing;
    return parsed;
}

void msgFree(msg *message){
    bdestroy(message->command);
    bdestroy(message->trailing);
    bdestroy(message->prefix);
    bstrListDestroy(message->params);
    free(message);
}

void printMessage(msg *message){
    fprintf(stderr, "prefix: %s\n", charBuffromBstr(message->prefix));
    fprintf(stderr, "command: %s\n", charBuffromBstr(message->command));
    int i;
    int len = message->params->qty;
    bstring *list = message->params->entry;
    for (i = 0; i < len; i++){
        fprintf(stderr, "param %d: %s\n", i, charBuffromBstr(list[i]));
    }
    fprintf(stderr, "trailing: %s\n", charBuffromBstr(message->trailing));
}
