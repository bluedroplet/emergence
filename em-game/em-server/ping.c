#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/types.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../common/llist.h"
#include "../shared/timer.h"
#include "../shared/network.h"
#include "network.h"
#include "ping.h"
#include "game.h"
/*

void ping_all_clients()
{
	struct player_t *cplayer = player0;
	
	while(cplayer)
	{
		struct ping_t ping;
		
		ping.index = cplayer->next_ping++;
		ping.time = get_double_time();
		
//		net_write_int(EMNETMSG_PING);
//		net_write_uint32(ping.index);
//		net_write_uint32(game_tick);
//		net_finished_writing(cplayer->conn);
		
		LL_ADD(struct ping_t, &cplayer->ping0, &ping);
		
		cplayer = cplayer->next;
	}
}


void process_pong(struct player_t *player, struct buffer_t *buffer)
{
//	net_write_int(EMNETMSG_PANG);
//	net_write_uint32(buffer_read_uint32(buffer));
//	net_finished_writing(player->conn);
}


*/
