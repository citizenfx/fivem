#ifndef SHAREDMEMORY_H_777736932196
#define SHAREDMEMORY_H_777736932196

#include <stdlib.h>
#include <string.h>

#include <fcntl.h> /* For O_* constants */

#include "util.h"
#include "conf.h"
#include "client.h"
#include "channel.h"
#include "sharedmemory_struct.h"

void Sharedmemory_init( int bindport, int bindport6 );
void Sharedmemory_update(void);
void Sharedmemory_alivetick(void);
void Sharedmemory_deinit(void);

#endif // SHAREDMEMORY_H_777736932196
