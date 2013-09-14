/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

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
