#ifndef _INC_PING
#define _INC_PING

struct ping_t
{
	uint32_t index;
	double time;
	
	struct ping_t *next;
};

#include "game.h"

void ping_all_clients();
void process_pong(struct player_t *player, struct buffer_t *buffer);


#endif
