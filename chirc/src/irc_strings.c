#include <stdio.h>
#include <stdlib.h>
#include "irc_strings.h"
#include "bstring/bstrlib.h"
#include "constants.h"

/*NOTE: this function assumes c string is an IRC message*/
/*or series of IRC messages of length <= 512 characters*/
bstring safe_BfromCstr(const char *c){
    bstring b;
    if ((b = bfromcstralloc(IRC_MAX_LEN+1, c))){
        (b->data)[b->slen] = (unsigned char)'\0';
        return b;
    }
    else {
        fprintf(stderr, "ERROR: bfromcstr failed to allocate space for bstring\n");
        exit(EXIT_FAILURE);
    }
}

blist safe_bSplit(bstring b, unsigned char splitChar){
    blist bs;
    if ((bs = bsplit(b, splitChar))){
        return bs;
    }
    else {
        fprintf(stderr, "ERROR: bsplit failed\n");
        exit(EXIT_FAILURE);
    }
}

blist safe_splitBfromCstr(bstring b, const char *cstr){
    bstring split = safe_BfromCstr(cstr);
    blist bs;
    if ((bs = bsplitstr(b, split))){
        return bs;
    }
    else {
        fprintf(stderr, "ERROR: bsplitstr failed\n");
        exit(EXIT_FAILURE);
    }
}

void safe_bConcat(bstring one, bstring two){
    int err = bconcat(one, two);
    if (err == BSTR_ERR){
        fprintf(stderr, "ERROR: bconcat failed\n");
        exit(EXIT_FAILURE);
    }
}

/*for sending IRC messages out on the wire*/
char *charBuffromBstr(bstring b){
    char *c = "";
    if(b){
        c = (char *) b->data;
    }
    return c;
}

/*deletes num items from list starting at startIndex*/
void blistDeleteRange(blist list, int startIndex, int num){
    bstring *entries = list->entry;
    int len = list->qty;
    if (startIndex < len && num > 0){
        int lim = startIndex + num < len ? startIndex + num : len;
        int i;
        for (i = startIndex; i < lim; i++){
            bdestroy(entries[i]);
        }
        for (i = startIndex; i < len - num; i++){
            entries[i] = entries[num + i];
        }
        list->qty = len - num;
    }
}

/*deletes a single index and returns the bstring at that index*/
bstring blistPopIndex(blist list, int index){
    bstring ret = NULL;
    if (list->qty > 0){
        ret = list->entry[index];
        int i;
        for (i = index; i < list->qty - 1; i++){
            list->entry[i] = list->entry[i + 1];
        }
        list->qty--;
    }
    return ret;
}

void blistConcat(blist list1, blist list2){
    if (list1 && list2){
        int len1 = list1->qty;
        int len2 = list2->qty;
        int resLen = len1 + len2;
        bstrListAlloc(list1, resLen);
        int i;
        for (i = 0; i < len2; i++){
            list1->entry[len1+i] = list2->entry[i];
        }
        list1->qty = resLen;
    }
}

int bstrContains(bstring b, const char *literal){
    bstring search = safe_BfromCstr(literal);
    int index = binchr(b, 0, search);
    bdestroy(search);
    return index;
}
