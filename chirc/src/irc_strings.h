#ifndef IRC_STR_
#define IRC_STR_

#include "bstring/bstrlib.h"

typedef struct bstrList *blist;

char *charBuffromBstr(bstring b);
blist safe_bSplit(bstring b, unsigned char splitChar);
bstring blistPopIndex(blist list, int index);
void blistDeleteRange(blist list, int startIndex, int num);
blist safe_splitBfromCstr(bstring b, const char *cstr);
void safe_bConcat(bstring one, bstring two);
bstring safe_BfromCstr(const char *c);
void blistConcat(blist list1, blist list2);
int bstrContains(bstring b, const char *literal);

#endif
