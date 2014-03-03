#ifndef DISPATCH_H_
#define DISPATCH_H_

#include "client.h"
#include "message.h"

void dispatchRegister(msg *message, cli *client);
void dispatchUser(msg *message, cli *client);

#endif
