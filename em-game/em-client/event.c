#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/types.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../common/llist.h"
#include "../shared/network.h"
#include "../shared/sgame.h"

#include "game.h"
#include "tick.h"
#include "network.h"
