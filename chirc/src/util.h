#ifndef UTIL_H_
#define UTIL_H_

#include "client.h"

void errnoExit(const char *reason);
void *allocaZam(size_t size);
bstring safeRecv(cli *client);
void safeSend(cli *client, char *tosend);
pthread_rwlock_t *newRwlock(void);

#endif
