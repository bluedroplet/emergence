#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <memory.h>
#include <assert.h>

#include <zlib.h>

#include "../common/types.h"
#include "../common/minmax.h"
#include "../common/llist.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../shared/cvar.h"
#include "../shared/network.h"
#include "../shared/sgame.h"
#include "rcon.h"
#include "map.h"
#include "network.h"
#include "console.h"
#include "../gsub/gsub.h"
#include "render.h"
#include "main.h"
#include "stars.h"
#include "ping.h"
#include "tick.h"
#include "skin.h"
#include "game.h"
#include "particles.h"
#include "line.h"

#ifdef LINUX
#include "../shared/timer.h"
#endif

#ifdef WIN32
#include "../common/win32/math.h"
#endif

struct event_t
{
	uint32_t tick;
	uint8_t type;
	int ooo;
	uint32_t index;
	
	union
	{
		struct
		{
			uint32_t index;
			uint8_t type;
			
			uint32_t skin;
			
			float xdis, ydis;
			float xvel, yvel;
			
			union
			{
				struct
				{
					float acc;
					float theta, omega;
					float shield_flare;
					int carcass;
					
				} craft_data;
				
				struct
				{
					int type;
					float theta;
					float shield_flare;
					
				} weapon_data;
				
				struct
				{
					float theta;
					uint32_t weapon_id;
					
				} rocket_data;
				
				struct
				{
					uint32_t weapon_id;
					
				} plasma_data;
			};
			
		} ent_data;
		
		struct
		{
			uint32_t index;
			
		} follow_me_data;
		
		struct
		{
			uint32_t index;
			
		} carcass_data;
		
		struct
		{
			float x1, y1;
			float x2, y2;
			
		} railtrail_data;
	};
	
	struct event_t *next;
	
} *event0 = NULL;


#define GAMESTATE_DEAD			0
#define GAMESTATE_DEMO			1
#define GAMESTATE_CONNECTING	2
#define GAMESTATE_SPECTATING	3
#define GAMESTATE_PLAYING		4

int game_state = GAMESTATE_DEAD;


uint32_t cgame_tick;
struct entity_t *centity0;

double viewx = 0.0, viewy = 0.0;


int moving_view;
float moving_view_time;

float moving_view_x;
float moving_view_y;

float moving_view_xa;
float moving_view_ya;


#define MOVING_VIEW_TRAVEL_TIME 1.0


float offset_view_x;
float offset_view_y;



struct surface_t *s_plasma, *s_craft_shield, *s_weapon_shield;




struct game_state_t
{
	uint32_t tick;
	struct entity_t *entity0;
	uint32_t follow_me;
	int tainted;	
	
	struct game_state_t *next;
		
} *game_state0, last_known_game_state, *cgame_state;


#define RAIL_TRAIL_EXPAND_TIME 0.5
#define RAIL_TRAIL_EXPAND_DISTANCE 10.0
#define RAIL_TRAIL_INITIAL_EXPAND_VELOCITY ((2 * RAIL_TRAIL_EXPAND_DISTANCE) / RAIL_TRAIL_EXPAND_TIME)
#define RAIL_TRAIL_EXPAND_ACC ((-RAIL_TRAIL_INITIAL_EXPAND_VELOCITY) / RAIL_TRAIL_EXPAND_TIME)




struct rail_trail_t
{
	float start_time;
	float x1, y1;
	float x2, y2;

	struct rail_trail_t *next;
		
} *rail_trail0;


/*
void world_to_screen(double worldx, double worldy, int *screenx, int *screeny)
{
	*screenx = (int)floor(worldx * (double)(vid_width) / 1600.0) - (int)floor(viewx * (double)(vid_width) / 1600.0) + vid_width / 2;
	*screeny = vid_height / 2 - 1 - (int)floor(worldy * (double)(vid_width) / 1600.0) + (int)floor(viewy * (double)(vid_width) / 1600.0);
}


void screen_to_world(int screenx, int screeny, double *worldx, double *worldy)
{
	*worldx = ((double)screenx - vid_width / 2 + 0.5f) / ((double)(vid_width) / 1600.0) + viewx;
	*worldy = (((double)(vid_height / 2 - 1 - screeny)) + 0.5f) / ((double)(vid_width) / 1600.0) + viewx;
}
*/


void world_to_screen(double worldx, double worldy, int *screenx, int *screeny)
{
	*screenx = (int)floor((worldx - viewx) * ((double)(vid_width) / 1600.0)) + vid_width / 2;
	*screeny = vid_height / 2 - 1 - (int)floor((worldy - viewy) * ((double)(vid_width) / 1600.0));
}


void screen_to_world(int screenx, int screeny, double *worldx, double *worldy)
{
	*worldx = ((double)screenx - vid_width / 2 + 0.5f) / ((double)(vid_width) / 1600.0) + viewx;
	*worldy = (((double)(vid_height / 2 - 1 - screeny)) + 0.5f) / ((double)(vid_width) / 1600.0) + viewx;
}


void add_offset_view(struct entity_t *entity)
{
	double sin_theta, cos_theta;
	sincos(entity->craft_data.theta, &sin_theta, &cos_theta);
	
	double target_x = - sin_theta * 400;// + entity->xvel * 24.0;
	double target_y = + cos_theta * 400;// + entity->yvel * 24.0;
	
	double deltax = target_x - offset_view_x;
	double deltay = target_y - offset_view_y;
	
	offset_view_x += deltax / 250.0;
	offset_view_y += deltay / 250.0;
	
	viewx += offset_view_x;
	viewy += offset_view_y;
}
	

void start_moving_view(float x1, float y1, float x2, float y2)
{
	moving_view_x = x2 - x1;
	moving_view_y = y2 - y1;
	
	moving_view_xa = ((moving_view_x) / 2) / (0.5 * (MOVING_VIEW_TRAVEL_TIME * 0.5) * (MOVING_VIEW_TRAVEL_TIME * 0.5));
	moving_view_ya = ((moving_view_y) / 2) / (0.5 * (MOVING_VIEW_TRAVEL_TIME * 0.5) * (MOVING_VIEW_TRAVEL_TIME * 0.5));
	
	moving_view_time = get_double_time();
}


void add_moving_view()
{
	double time = get_double_time();
	
	time -= moving_view_time;
	
	if(time > MOVING_VIEW_TRAVEL_TIME)
		return;
	
	viewx -= moving_view_x;
	viewy -= moving_view_y;
	
	if(time < MOVING_VIEW_TRAVEL_TIME * 0.5)
	{
		viewx += 0.5 * moving_view_xa * time * time;
		viewy += 0.5 * moving_view_ya * time * time;
	}
	else
	{
		viewx += 0.5 * moving_view_xa * MOVING_VIEW_TRAVEL_TIME * 0.5 * MOVING_VIEW_TRAVEL_TIME * 0.5;
		viewy += 0.5 * moving_view_ya * MOVING_VIEW_TRAVEL_TIME * 0.5 * MOVING_VIEW_TRAVEL_TIME * 0.5;
		
		viewx += moving_view_xa * MOVING_VIEW_TRAVEL_TIME * 0.5 * (time - MOVING_VIEW_TRAVEL_TIME * 0.5);
		viewy += moving_view_ya * MOVING_VIEW_TRAVEL_TIME * 0.5 * (time - MOVING_VIEW_TRAVEL_TIME * 0.5);
		
		viewx += 0.5 * -moving_view_xa * (time - MOVING_VIEW_TRAVEL_TIME * 0.5) * (time - MOVING_VIEW_TRAVEL_TIME * 0.5);
		viewy += 0.5 * -moving_view_ya * (time - MOVING_VIEW_TRAVEL_TIME * 0.5) * (time - MOVING_VIEW_TRAVEL_TIME * 0.5);
	}
}




void game_process_connection()
{
	game_state = GAMESTATE_CONNECTING;
	
	console_print("Connected!\n");
}


void game_process_disconnection()
{
	game_state = GAMESTATE_DEAD;
	
//	LL_REMOVE_ALL(struct entity_t, &entity0);
//	game_state = GAMESTATE_DEAD;
	console_print("Disconnected!\n");
}


void game_process_conn_lost()
{
	game_state = GAMESTATE_DEAD;
	console_print("Connection lost.\n");
}


int game_process_print(struct buffer_t *stream)
{
	struct string_t *s = buffer_read_string(stream);
	console_print(s->text);
	free_string(s);
	return 1;
}


int game_process_proto_ver(struct buffer_t *stream)
{
	if(game_state != GAMESTATE_CONNECTING)
		return 0;
	
	uint8_t proto_ver = buffer_read_uint8(stream);
	
	if(proto_ver == EMNET_PROTO_VER)
	{
		console_print("Correct protocol version\n");
		net_emit_uint8(EMNETMSG_JOIN);
		net_emit_string(get_cvar_string("name"));
		net_emit_end_of_stream();
	}
	else
	{
		console_print("Incorrect protocol version\n");
		game_state = GAMESTATE_DEAD;
		em_disconnect(NULL);
	}
	
	return 1;
}


int game_process_playing(struct buffer_t *stream)
{
	if(game_state == GAMESTATE_DEAD)
		return 0;
	
	
	game_state0 = calloc(1, sizeof(struct game_state_t));
	game_state0->tick = buffer_read_uint32(stream);
	
	last_known_game_state.tick = game_state0->tick;
	
	
//	game_tick = 
//	console_print("start tick: %u\n", game_tick);
	
	game_state = GAMESTATE_PLAYING;

	return 1;
}


int game_process_spectating(struct buffer_t *stream)
{
	if(game_state == GAMESTATE_DEAD)
		return 0;
	
	game_state = GAMESTATE_SPECTATING;

	return 1;
}


void read_craft_data_from_stream(struct event_t *event, struct buffer_t *stream)
{
	event->ent_data.craft_data.acc = buffer_read_float(stream);
	event->ent_data.craft_data.theta = buffer_read_float(stream);
	event->ent_data.craft_data.omega = buffer_read_float(stream);
	event->ent_data.craft_data.shield_flare = buffer_read_float(stream);
}


void read_weapon_data_from_stream(struct event_t *event, struct buffer_t *stream)
{
	event->ent_data.weapon_data.type = buffer_read_int(stream);
	event->ent_data.weapon_data.theta = buffer_read_float(stream);
	event->ent_data.weapon_data.shield_flare = buffer_read_float(stream);
}


void read_rocket_data_from_stream(struct event_t *event, struct buffer_t *stream)
{
	event->ent_data.rocket_data.theta = buffer_read_float(stream);
	event->ent_data.rocket_data.weapon_id = buffer_read_uint32(stream);
}

void read_plasma_data_from_stream(struct event_t *event, struct buffer_t *stream)
{
	event->ent_data.plasma_data.weapon_id = buffer_read_uint32(stream);
}




void add_spawn_ent_event(struct event_t *event, struct buffer_t *stream)
{
	event->ent_data.index = buffer_read_uint32(stream);
	event->ent_data.type = buffer_read_uint8(stream);
	event->ent_data.skin = buffer_read_uint32(stream);
	event->ent_data.xdis = buffer_read_float(stream);
	event->ent_data.ydis = buffer_read_float(stream);
	event->ent_data.xvel = buffer_read_float(stream);
	event->ent_data.yvel = buffer_read_float(stream);
	
	switch(event->ent_data.type)
	{
	case ENT_CRAFT:
		read_craft_data_from_stream(event, stream);
		event->ent_data.craft_data.carcass = buffer_read_int(stream);
		break;
	
	case ENT_WEAPON:
		read_weapon_data_from_stream(event, stream);
		break;
	
	case ENT_BOGIE:
		read_plasma_data_from_stream(event, stream);
		break;
	
	case ENT_ROCKET:
		read_rocket_data_from_stream(event, stream);
		break;
	
	case ENT_MINE:
		break;
	}
}


void process_spawn_ent_event(struct event_t *event)
{
	struct entity_t *entity = new_entity(&centity0);
		
	assert(entity);

	entity->index = event->ent_data.index;
	entity->type = event->ent_data.type;
	entity->xdis = event->ent_data.xdis;
	entity->ydis = event->ent_data.ydis;
	entity->xvel = event->ent_data.xvel;
	entity->yvel = event->ent_data.yvel;
	
	switch(entity->type)
	{
	case ENT_CRAFT:
		entity->craft_data.acc = event->ent_data.craft_data.acc;
		entity->craft_data.theta = event->ent_data.craft_data.theta;
		entity->craft_data.omega = event->ent_data.craft_data.omega;
		entity->craft_data.shield_flare = event->ent_data.craft_data.shield_flare;
		entity->craft_data.surface = skin_get_craft_surface(event->ent_data.skin);
		entity->craft_data.particle = 0.0;
		break;
	
	case ENT_WEAPON:
		entity->weapon_data.type = event->ent_data.weapon_data.type;
		entity->weapon_data.theta = event->ent_data.weapon_data.theta;
		entity->weapon_data.shield_flare = event->ent_data.weapon_data.shield_flare;
	
		switch(entity->weapon_data.type)
		{
		case WEAPON_PLASMA_CANNON:
			entity->weapon_data.surface = skin_get_plasma_cannon_surface(event->ent_data.skin);
			break;
		
		case WEAPON_MINIGUN:
			entity->weapon_data.surface = skin_get_minigun_surface(event->ent_data.skin);
			break;
		
		case WEAPON_ROCKET_LAUNCHER:
			entity->weapon_data.surface = skin_get_rocket_launcher_surface(event->ent_data.skin);
			break;
		}
	
		break;
	
	case ENT_BOGIE:
		entity->plasma_data.in_weapon = 1;
		entity->plasma_data.weapon_id = event->ent_data.plasma_data.weapon_id;
		break;
	
	case ENT_ROCKET:
		entity->rocket_data.theta = event->ent_data.rocket_data.theta;
		entity->rocket_data.start_tick = cgame_tick;
		entity->rocket_data.in_weapon = 1;
		entity->rocket_data.weapon_id = event->ent_data.rocket_data.weapon_id;
		break;
	
	case ENT_MINE:
		break;
	}
}


void add_update_ent_event(struct event_t *event, struct buffer_t *stream)
{
	event->ent_data.index = buffer_read_uint32(stream);
	event->ent_data.type = buffer_read_uint8(stream);
	event->ent_data.xdis = buffer_read_float(stream);
	event->ent_data.ydis = buffer_read_float(stream);
	event->ent_data.xvel = buffer_read_float(stream);
	event->ent_data.yvel = buffer_read_float(stream);
	
	switch(event->ent_data.type)
	{
	case ENT_CRAFT:	
		read_craft_data_from_stream(event, stream);
		break;
	
	case ENT_WEAPON:
		read_weapon_data_from_stream(event, stream);
		break;
	
	case ENT_BOGIE:
		read_plasma_data_from_stream(event, stream);
		break;
	
	case ENT_ROCKET:
		break;
	
	case ENT_MINE:
		break;
	}
}


void process_update_ent_event(struct event_t *event)
{
	struct entity_t *entity = get_entity(centity0, event->ent_data.index);

	if(!entity)
		return;
	
	entity->xdis = event->ent_data.xdis;
	entity->ydis = event->ent_data.ydis;
	entity->xvel = event->ent_data.xvel;
	entity->yvel = event->ent_data.yvel;
	
	switch(entity->type)
	{
	case ENT_CRAFT:
		entity->craft_data.acc = event->ent_data.craft_data.acc;
		entity->craft_data.theta = event->ent_data.craft_data.theta;
		entity->craft_data.omega = event->ent_data.craft_data.omega;
		entity->craft_data.shield_flare = event->ent_data.craft_data.shield_flare;
		break;
	
	case ENT_WEAPON:
		entity->weapon_data.shield_flare = event->ent_data.weapon_data.shield_flare;
		break;
	
	case ENT_BOGIE:
		break;
	
	case ENT_ROCKET:
		break;
	
	case ENT_MINE:
		break;
	}
}


void add_kill_ent_event(struct event_t *event, struct buffer_t *stream)
{
	event->ent_data.index = buffer_read_uint32(stream);
}


void process_kill_ent_event(struct event_t *event)
{
	struct entity_t *entity = get_entity(centity0, event->ent_data.index);

	if(!entity)
		return;
	
	switch(entity->type)
	{
	case ENT_CRAFT:
		explosion(entity);
		strip_weapons_from_craft(entity);
		break;
	
	case ENT_WEAPON:
		explosion(entity);
		strip_craft_from_weapon(entity);
		break;
	}
	
	remove_entity(&centity0, entity);
}


void add_follow_me_event(struct event_t *event, struct buffer_t *stream)
{
	event->follow_me_data.index = buffer_read_uint32(stream);
}


void process_follow_me_event(struct event_t *event)
{
	cgame_state->follow_me = event->follow_me_data.index;
	struct entity_t *entity = get_entity(centity0, cgame_state->follow_me);
	start_moving_view(viewx, viewy, entity->xdis, entity->ydis);
}


void add_carcass_event(struct event_t *event, struct buffer_t *stream)
{
	event->carcass_data.index = buffer_read_uint32(stream);
}


void process_carcass_event(struct event_t *event)
{
	struct entity_t *craft = get_entity(centity0, event->carcass_data.index);

	if(!craft)
		return;

	craft->craft_data.carcass = 1;
	craft->craft_data.particle = 0.0;
	
	strip_weapons_from_craft(craft);
}


void add_railtrail_event(struct event_t *event, struct buffer_t *stream)
{
	event->railtrail_data.x1 = buffer_read_float(stream);
	event->railtrail_data.y1 = buffer_read_float(stream);
	event->railtrail_data.x2 = buffer_read_float(stream);
	event->railtrail_data.y2 = buffer_read_float(stream);
}


void process_railtrail_event(struct event_t *event)
{
	struct rail_trail_t rail_trail;
		
	rail_trail.start_time = get_double_time();
	rail_trail.x1 = event->railtrail_data.x1;
	rail_trail.y1 = event->railtrail_data.y1;
	rail_trail.x2 = event->railtrail_data.x2;
	rail_trail.y2 = event->railtrail_data.y2;
	
	LL_ADD(struct rail_trail_t, &rail_trail0, &rail_trail);
}


void process_tick_events(uint32_t tick)
{
	if(!event0)
		return;
	
	struct event_t *event = event0;
	
	while(event)
	{
		if(event->tick == tick && !event->ooo)
		{
			switch(event->type)
			{
			case EMNETEVENT_SPAWN_ENT:
				process_spawn_ent_event(event);
				break;
			
			case EMNETEVENT_UPDATE_ENT:
				process_update_ent_event(event);
				break;
			
			case EMNETEVENT_KILL_ENT:
				process_kill_ent_event(event);
				break;
			
			case EMNETEVENT_FOLLOW_ME:
				process_follow_me_event(event);
				break;
			
			case EMNETEVENT_CARCASS:
				process_carcass_event(event);
				break;
			
			case EMNETEVENT_RAILTRAIL:
				process_railtrail_event(event);
				break;
			}
			
			struct event_t *next = event->next;
			LL_REMOVE(struct event_t, &event0, event);
			event = next;
			continue;
		}
		
		event = event->next;
	}
}


int process_tick_events_do_not_remove(uint32_t tick)
{
	if(!event0)
		return 0;
	
	struct event_t *event = event0;
	int any = 0;
	
	while(event)
	{
		if(event->tick == tick)
		{
			switch(event->type)
			{
			case EMNETEVENT_SPAWN_ENT:
				process_spawn_ent_event(event);
				any = 1;
				break;
			
			case EMNETEVENT_UPDATE_ENT:
				process_update_ent_event(event);
				any = 1;
				break;
			
			case EMNETEVENT_KILL_ENT:
				process_kill_ent_event(event);
				any = 1;
				break;

			case EMNETEVENT_FOLLOW_ME:
				process_follow_me_event(event);
				any = 1;
				break;
			
			case EMNETEVENT_CARCASS:
				process_carcass_event(event);
				break;

			case EMNETEVENT_RAILTRAIL:
				process_railtrail_event(event);
				break;
			}
		}
		
		event = event->next;
	}
	
	return any;
}


int get_event_by_ooo_index(uint32_t index)
{
	struct event_t *event = event0;
		
	while(event)
	{
		if(event->ooo && event->index == index)
		{
		//	printf("ooo neutralized\n");
			
			event->ooo = 0;
			return 1;
		}
		
		event = event->next;
	}
	
	return 0;
}


int game_process_event_timed(uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
//	printf("game_process_event_timed: %u\n", index);
	
	struct event_t event;
	
	event.tick = buffer_read_uint32(stream);
	
//	printf("event tick: %u\n", event.tick);
	
	add_game_tick(event.tick, stamp);
	
	if(get_event_by_ooo_index(index))
		return 0;
	
	event.type = buffer_read_uint8(stream);
	event.ooo = 0;
	
	switch(event.type)
	{
	case EMNETEVENT_SPAWN_ENT:
		add_spawn_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_UPDATE_ENT:
		add_update_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_KILL_ENT:
		add_kill_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_FOLLOW_ME:
		add_follow_me_event(&event, stream);
		break;
	
	case EMNETEVENT_CARCASS:
		add_carcass_event(&event, stream);
		break;
	
	case EMNETEVENT_RAILTRAIL:
		add_railtrail_event(&event, stream);
		break;
	}
	
	LL_ADD_TAIL(struct event_t, &event0, &event);
	
	
	return 1;
}


int game_process_event_untimed(uint32_t index, struct buffer_t *stream)
{
//	printf("game_process_event_untimed: %u\n", index);
	
	struct event_t event;
	
	event.tick = buffer_read_uint32(stream);
	event.type = buffer_read_uint8(stream);
	
	if(get_event_by_ooo_index(index))
		return 0;
	
	event.ooo = 0;
	
	switch(event.type)
	{
	case EMNETEVENT_SPAWN_ENT:
		add_spawn_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_UPDATE_ENT:
		add_update_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_KILL_ENT:
		add_kill_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_FOLLOW_ME:
		add_follow_me_event(&event, stream);
		break;
	
	case EMNETEVENT_CARCASS:
		add_carcass_event(&event, stream);
		break;
	
	case EMNETEVENT_RAILTRAIL:
		add_railtrail_event(&event, stream);
		break;
	}
	
	LL_ADD_TAIL(struct event_t, &event0, &event);
	
	
	return 1;
}


int game_process_event_timed_ooo(uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
//	printf("game_process_event_timed_ooo: %u\n", index);
	
	struct event_t event;
	
	event.tick = buffer_read_uint32(stream);
	add_game_tick(event.tick, stamp);
	event.type = buffer_read_uint8(stream);
	event.ooo = 1;
	event.index = index;
	
	switch(event.type)
	{
	case EMNETEVENT_SPAWN_ENT:
		add_spawn_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_UPDATE_ENT:
		add_update_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_KILL_ENT:
		add_kill_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_FOLLOW_ME:
		add_follow_me_event(&event, stream);
		break;
	
	case EMNETEVENT_CARCASS:
		add_carcass_event(&event, stream);
		break;
	
	case EMNETEVENT_RAILTRAIL:
		add_railtrail_event(&event, stream);
		break;
	}
	
	LL_ADD_TAIL(struct event_t, &event0, &event);
	
	
	return 1;
}


int game_process_event_untimed_ooo(uint32_t index, struct buffer_t *stream)
{
//	printf("game_process_event_untimed_ooo: %u\n", index);
	
	struct event_t event;
	
	event.tick = buffer_read_uint32(stream);
	event.type = buffer_read_uint8(stream);
	event.ooo = 1;
	event.index = index;
	
	switch(event.type)
	{
	case EMNETEVENT_SPAWN_ENT:
		add_spawn_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_UPDATE_ENT:
		add_update_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_KILL_ENT:
		add_kill_ent_event(&event, stream);
		break;
	
	case EMNETEVENT_FOLLOW_ME:
		add_follow_me_event(&event, stream);
		break;
	
	case EMNETEVENT_CARCASS:
		add_carcass_event(&event, stream);
		break;
	
	case EMNETEVENT_RAILTRAIL:
		add_railtrail_event(&event, stream);
		break;
	}
	
	LL_ADD_TAIL(struct event_t, &event0, &event);
	
	
	return 1;
}


void game_process_stream_timed(uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
	while(buffer_more(stream))
	{
		switch(buffer_read_uint8(stream))
		{
		case EMNETMSG_PROTO_VER:
			if(!game_process_proto_ver(stream))
				return;
			break;
		
		case EMNETMSG_PLAYING:
			if(!game_process_playing(stream))
				return;
			break;
		
		case EMNETMSG_SPECTATING:
			if(!game_process_spectating(stream))
				return;
			break;
		
		case EMNETMSG_PRINT:
			if(!game_process_print(stream))
				return;
			break;
			
		case EMNETMSG_LOADMAP:
			if(!game_process_load_map(stream))
				return;
			break;
			
		case EMNETMSG_LOADSKIN:
			if(!game_process_load_skin(stream))
				return;
			break;
			
		case EMNETMSG_EVENT:
			if(!game_process_event_timed(index, stamp, stream))
				return;
			break;
		
		default:
			return;
		}
	}
}


void game_process_stream_untimed(uint32_t index, struct buffer_t *stream)
{
	while(buffer_more(stream))
	{
		switch(buffer_read_uint8(stream))
		{
		case EMNETMSG_PROTO_VER:
			if(!game_process_proto_ver(stream))
				return;
			break;
		
		case EMNETMSG_PLAYING:
			if(!game_process_playing(stream))
				return;
			break;
		
		case EMNETMSG_SPECTATING:
			if(!game_process_spectating(stream))
				return;
			break;
		
		case EMNETMSG_PRINT:
			if(!game_process_print(stream))
				return;
			break;
			
		case EMNETMSG_LOADSKIN:
			if(!game_process_load_skin(stream))
				return;
			break;
			
		case EMNETMSG_EVENT:
			if(!game_process_event_untimed(index, stream))
				return;
			break;
		
		default:
			return;
		}
	}
}


void game_process_stream_timed_ooo(uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
	while(buffer_more(stream))
	{
		switch(buffer_read_uint8(stream))
		{
		case EMNETMSG_EVENT:
			if(!game_process_event_timed_ooo(index, stamp, stream))
				return;
			break;
		
		default:
			return;
		}
	}
}


void game_process_stream_untimed_ooo(uint32_t index, struct buffer_t *stream)
{
	while(buffer_more(stream))
	{
		switch(buffer_read_uint8(stream))
		{
		case EMNETMSG_EVENT:
			if(!game_process_event_untimed_ooo(index, stream))
				return;
			break;
		
		default:
			return;
		}
	}
}


void cf_status(char *c)
{
	if(net_state != NETSTATE_CONNECTED)
		console_print("You are not connected.\n");
	else
	{
		net_emit_uint8(EMNETMSG_STATUS);
		net_emit_end_of_stream();
	}
}


void cf_say(char *c)
{
	if(net_state != NETSTATE_CONNECTED)
		console_print("You are not connected.\n");
	else
	{
		net_emit_uint8(EMNETMSG_SAY);
		net_emit_string(c);
		net_emit_end_of_stream();
	}
}


void cf_spectate(char *c)
{
	switch(game_state)
	{
	case GAMESTATE_DEAD:
	case GAMESTATE_DEMO:
	case GAMESTATE_CONNECTING:
		console_print("You are not connected.\n");
		break;
	
	case GAMESTATE_SPECTATING:
		console_print("You are already spectating.\n");
		break;
	
	case GAMESTATE_PLAYING:
		net_emit_uint8(EMNETMSG_SPECTATE);
		net_emit_end_of_stream();
		break;
	}	
}


void cf_play(char *c)
{
	switch(game_state)
	{
	case GAMESTATE_DEAD:
	case GAMESTATE_DEMO:
	case GAMESTATE_CONNECTING:
		console_print("You are not connected.\n");
		break;
	
	case GAMESTATE_SPECTATING:
		net_emit_uint8(EMNETMSG_PLAY);
		net_emit_end_of_stream();
		break;
	
	case GAMESTATE_PLAYING:
		console_print("You are already playing.\n");
		break;
	}	
}


void thrust_bool(uint32_t state)
{
//	if(game_state != GAMESTATE_ALIVE)
//		return;

//	net_write_dword(EMNETMSG_CTRLCNGE);
//	net_write_dword(EMCTRL_THRUST);

//	if(state)
//		net_write_float(1.0f);
//	else
//		net_write_float(0.0f);

//	finished_writing();
}



void roll_left(uint32_t state)
{
//	if(game_state != GAMESTATE_ALIVE)
		return;
/*
	net_write_dword(EMNETMSG_CTRLCNGE);
	net_write_dword(EMCTRL_ROLL);

	if(state)
		net_write_float(-1.0f);
	else
		net_write_float(0.0f);

	finished_writing();
	*/
}


void roll_right(uint32_t state)
{
//	if(game_state != GAMESTATE_ALIVE)
		return;
/*
	net_write_dword(EMNETMSG_CTRLCNGE);
	net_write_dword(EMCTRL_ROLL);

	if(state)
		net_write_float(1.0f);
	else
		net_write_float(0.0f);

	finished_writing();
	*/
}


void explosion(struct entity_t *entity)
{
	double force = 0;
	int np, p;
	
	switch(entity->type)
	{
	case ENT_CRAFT:
		force = 2000;
		break;
	
	case ENT_WEAPON:
		force = 1000;
		break;
	
	case ENT_ROCKET:
		force = 500;
		break;
	
	case ENT_MINE:
		force = 200;
		break;
	
	case ENT_RAILS:
		force = 50;
		break;
	}
	
	np = lrint(force / 5);
	
	
	
	struct particle_t particle;
	particle.xpos = entity->xdis;
	particle.ypos = entity->ydis;
	
	for(p = 0; p < np; p++)
	{
		double sin_theta, cos_theta;
		sincos(drand48() * 2 * M_PI, &sin_theta, &cos_theta);
		
		
		float r = drand48();
		
		particle.xvel = entity->xvel - sin_theta * force * r;
		particle.yvel = entity->xvel + cos_theta * force * r;
		
		particle.creation = particle.last = get_double_time();
		
		create_upper_particle(&particle);
	}
}


void tick_craft(struct entity_t *craft, float xdis, float ydis)
{
	if(cgame_tick <= craft->craft_data.last_tick)
		return;
	
	craft->craft_data.last_tick = cgame_tick;
	
	int np = 0, p;
	double sin_theta, cos_theta;
	struct particle_t particle;


	if(craft->craft_data.carcass)
	{
		double vel = hypot(craft->xvel, craft->yvel);

		craft->craft_data.particle += vel * 0.2;

		while(craft->craft_data.particle >= 1.0)
		{
			craft->craft_data.particle -= 1.0;
			np++;
		}
		
		for(p = 0; p < np; p++)
		{
			sincos(drand48() * 2 * M_PI, &sin_theta, &cos_theta);
			
			float r = drand48();
			
			float nxdis = craft->xdis + (xdis - craft->xdis) * (float)p / (float)np;
			float nydis = craft->ydis + (ydis - craft->ydis) * (float)p / (float)np;
			
	
			particle.creation = particle.last = get_tsc_from_game_tick(cgame_tick + (float)p / (float)np - 1.0);
			
			particle.xpos = nxdis;
			particle.ypos = nydis;
			
			particle.xvel = craft->xvel - sin_theta * 500.0 * r;
			particle.yvel = craft->yvel + cos_theta * 500.0 * r;
				
			create_upper_particle(&particle);
			
		}
	}
	else
	{
		craft->craft_data.particle += craft->craft_data.acc * 200;
		
		
		
		while(craft->craft_data.particle >= 1.0)
		{
			craft->craft_data.particle -= 1.0;
			np++;
		}
	
		for(p = 0; p < np; p++)
		{
			sincos(craft->craft_data.theta, &sin_theta, &cos_theta);
			
			float r = drand48();
			
			float nxdis = craft->xdis + (xdis - craft->xdis) * (float)p / (float)np;
			float nydis = craft->ydis + (ydis - craft->ydis) * (float)p / (float)np;
			
			
	
			particle.creation = particle.last = get_tsc_from_game_tick(cgame_tick + (float)p / (float)np - 1.0);
			
			
			particle.xpos = nxdis + sin_theta * 25;
			particle.ypos = nydis - cos_theta * 25;
	
			sincos(craft->craft_data.theta + M_PI / 2, &sin_theta, &cos_theta);
			
			particle.xpos -= sin_theta * 15;
			particle.ypos += cos_theta * 15;
			
			sincos(craft->craft_data.theta + (drand48() - 0.5) * 0.1, &sin_theta, &cos_theta);
			particle.xvel = craft->xvel + craft->craft_data.acc * sin_theta * 100000 * r;
			particle.yvel = craft->yvel + -craft->craft_data.acc * cos_theta * 100000 * r;
				
			create_lower_particle(&particle);
	
			particle.xpos = nxdis + sin_theta * 25;
			particle.ypos = nydis - cos_theta * 25;
	
			sincos(craft->craft_data.theta + M_PI / 2, &sin_theta, &cos_theta);
			
			particle.xpos += sin_theta * 15;
			particle.ypos -= cos_theta * 15;
			
			sincos(craft->craft_data.theta + (drand48() - 0.5) * 0.1, &sin_theta, &cos_theta);
			particle.xvel = craft->xvel + craft->craft_data.acc * sin_theta * 100000 * r;
			particle.yvel = craft->yvel + -craft->craft_data.acc * cos_theta * 100000 * r;
				
			create_lower_particle(&particle);
		}
	}
}


void tick_rocket(struct entity_t *rocket, float xdis, float ydis)
{
	if(cgame_tick <= rocket->rocket_data.last_tick)
		return;
	
	rocket->rocket_data.last_tick = cgame_tick;
	
	rocket->rocket_data.particle += 15.0;
	
	
	int np = 0, p;
	
	while(rocket->rocket_data.particle >= 1.0)
	{
		rocket->rocket_data.particle -= 1.0;
		np++;
	}
	
	

	for(p = 0; p < np; p++)	
	{
		double sin_theta, cos_theta;
		sincos(rocket->rocket_data.theta, &sin_theta, &cos_theta);
		
		float r = drand48();
		

		float nxdis = rocket->xdis + (xdis - rocket->xdis) * (float)p / (float)np;
		float nydis = rocket->ydis + (ydis - rocket->ydis) * (float)p / (float)np;
		
		
		struct particle_t particle;

		particle.creation = particle.last = get_tsc_from_game_tick(cgame_tick + (float)p / (float)np - 1.0);
		
		
		particle.xpos = nxdis;
		particle.ypos = nydis;

		sincos(rocket->rocket_data.theta + (drand48() - 0.5) * 0.1, &sin_theta, &cos_theta);
		particle.xvel = rocket->xvel + sin_theta * 1000 * r;
		particle.yvel = rocket->yvel - cos_theta * 1000 * r;
			
		create_lower_particle(&particle);
	}
}




void render_entities()
{
	struct entity_t *entity = centity0;
	struct blit_params_t params;
	params.dest = s_backbuffer;
			

	while(entity)
	{
		uint32_t x, y;
		
		switch(entity->type)
		{
		case ENT_CRAFT:
			
				
			params.source = entity->craft_data.surface;
		
			params.source_x = 0;
			params.source_y = (lrint((entity->craft_data.theta / (2 * M_PI) + 0.5) * ROTATIONS) % ROTATIONS) * entity->craft_data.surface->width;
		
			world_to_screen(entity->xdis, entity->ydis, &x, &y);
		
			params.dest_x = x - entity->craft_data.surface->width / 2;
			params.dest_y = y - entity->craft_data.surface->width / 2;
		
			params.width = entity->craft_data.surface->width;
			params.height = entity->craft_data.surface->width;
		
			blit_partial_surface(&params);
		
		
			if(!entity->craft_data.carcass)
			{				
				params.source = s_craft_shield;
			
				params.dest_x = x - s_craft_shield->width / 2;
				params.dest_y = y - s_craft_shield->width / 2;

				params.red = params.green = params.blue = 0xff;
				params.alpha = lrint(entity->craft_data.shield_flare * 255.0);
			
				blit_alpha_surface(&params);
			}
		
			break;
		
		case ENT_WEAPON:
			params.source = entity->weapon_data.surface;
		
			params.source_x = 0;
			params.source_y = (lrint((entity->weapon_data.theta / (2 * M_PI) + 0.5) * ROTATIONS) % ROTATIONS) * entity->weapon_data.surface->width;
		
			world_to_screen(entity->xdis, entity->ydis, &x, &y);
		
			params.dest_x = x - entity->weapon_data.surface->width / 2;
			params.dest_y = y - entity->weapon_data.surface->width / 2;
		
			params.width = entity->weapon_data.surface->width;
			params.height = entity->weapon_data.surface->width;
		
			blit_partial_surface(&params);
		
			params.source = s_weapon_shield;
		
			params.dest_x = x - s_weapon_shield->width / 2;
			params.dest_y = y - s_weapon_shield->width / 2;
			
			params.red = params.green = params.blue = 0xff;
			params.alpha = lrint(entity->weapon_data.shield_flare * 255.0);
		
			blit_alpha_surface(&params);
		
			break;
		
		case ENT_BOGIE:

			params.source = s_plasma;
		
			world_to_screen(entity->xdis, entity->ydis, &x, &y);
		
			params.red = 0x32;
			params.green = 0x73;
			params.blue = 0x71;
			
			params.dest_x = x - s_plasma->width / 2;
			params.dest_y = y - s_plasma->width / 2;
			
			blit_alpha_surface(&params);
		
			break;
		
		case ENT_BULLET:
			break;
		
		case ENT_ROCKET:
			
			params.source = s_plasma;
		
			world_to_screen(entity->xdis, entity->ydis, &x, &y);
		
			params.red = 0x32;
			params.green = 0x73;
			params.blue = 0x71;
			
			params.dest_x = x - s_plasma->width / 2;
			params.dest_y = y - s_plasma->width / 2;
			
			blit_alpha_surface(&params);
		
			break;
		
		case ENT_MINE:
			break;
		
		case ENT_RAILS:
			break;
		
		case ENT_SHIELD:
			break;
		}
		
		entity = entity->next;
	}
}


void duplicate_game_state(struct game_state_t *old_game_state, struct game_state_t *new_game_state)
{
	new_game_state->tick = old_game_state->tick;
	new_game_state->tainted = old_game_state->tainted;
	new_game_state->entity0 = NULL;
	new_game_state->follow_me = old_game_state->follow_me;
	new_game_state->next = NULL;
	
	
	struct entity_t *centity = old_game_state->entity0;
		
	while(centity)
	{
		LL_ADD(struct entity_t, &new_game_state->entity0, centity);
		centity = centity->next;
	}
	
	
	// fix weapons
	
	centity = new_game_state->entity0;
		
	while(centity)
	{
		switch(centity->type)
		{
		case ENT_CRAFT:
			
			if(centity->craft_data.left_weapon)
				centity->craft_data.left_weapon = get_entity(new_game_state->entity0, centity->craft_data.left_weapon->index);
			
			if(centity->craft_data.right_weapon)
				centity->craft_data.right_weapon = get_entity(new_game_state->entity0, centity->craft_data.right_weapon->index);
			break;
		
		case ENT_WEAPON:
			if(centity->weapon_data.craft)
				centity->weapon_data.craft = get_entity(new_game_state->entity0, centity->weapon_data.craft->index);
			break;
		}
		
		centity = centity->next;
	}
}


void free_game_state_list(struct game_state_t **game_state0)
{
	assert(game_state0);
	
	while(*game_state0)
	{
		LL_REMOVE_ALL(struct entity_t, &(*game_state0)->entity0);
		struct game_state_t *temp = (*game_state0)->next;
		free(*game_state0);
		*game_state0 = temp;
	}
}


void update_game()
{
	// get render_tick and last_known_tick
	
	update_tick_parameters();
	uint32_t render_tick = get_game_tick();
	
	uint32_t new_io_tick = 0, new_ooo_tick = 0;
	uint32_t first_io_tick = 0, last_io_tick = 0, first_ooo_tick = 0;
	
	struct event_t *cevent = event0;
		
	while(cevent)
	{
		if(cevent->tick <= render_tick)
		{
			if(cevent->ooo)
			{
				if(!new_ooo_tick)
				{
					new_ooo_tick = 1;
					first_ooo_tick = cevent->tick;
				}
				else
				{
					first_ooo_tick = min(first_ooo_tick, cevent->tick);
				}
			}
			else
			{
				if(!new_io_tick)
				{
					new_io_tick = 1;
					last_io_tick = first_io_tick = cevent->tick;
				}
				else
				{
					first_io_tick = min(first_io_tick, cevent->tick);
					last_io_tick = max(last_io_tick, cevent->tick);
				}
			}
		}
		
		cevent = cevent->next;
	}


	if(new_io_tick)
	{
		if(last_known_game_state.tick != first_io_tick)
		{
			if(!game_state0->tainted)
			{
				struct game_state_t *game_state = game_state0;
					
				while(game_state->tick < first_io_tick && game_state->next)
				{
					if(game_state->next->tainted)
						break;
					
					game_state = game_state->next;
				}
				
				LL_REMOVE_ALL(struct entity_t, &last_known_game_state.entity0);
				duplicate_game_state(game_state, &last_known_game_state);
			}
		}
		
		cgame_state = &last_known_game_state;
		centity0 = last_known_game_state.entity0;
		cgame_tick = last_known_game_state.tick;
		process_tick_events(last_known_game_state.tick);
		last_known_game_state.entity0 = centity0;
		
		while(last_known_game_state.tick < last_io_tick)
		{
			centity0 = last_known_game_state.entity0;
			cgame_tick = ++last_known_game_state.tick;
			s_tick_entities(&centity0);
			process_tick_events(last_known_game_state.tick);
			last_known_game_state.entity0 = centity0;
		}
			
		free_game_state_list(&game_state0);
		game_state0 = malloc(sizeof(struct game_state_t));
		duplicate_game_state(&last_known_game_state, game_state0);
	}
	else
	{
		if(game_state0->tainted)
		{
			free_game_state_list(&game_state0);
			game_state0 = malloc(sizeof(struct game_state_t));
			duplicate_game_state(&last_known_game_state, game_state0);
		}
	}

	
	struct game_state_t *game_state = game_state0;
		
	while(game_state->next)
	{
		if(new_ooo_tick && game_state->tick >= first_ooo_tick)
			break;
		
		if(game_state->next->tainted)
			break;
		
		game_state = game_state->next;
	}
	
	free_game_state_list(&game_state->next);
	
	cgame_state = game_state;
	centity0 = game_state->entity0;
	cgame_tick = game_state->tick;
	
	if(process_tick_events_do_not_remove(cgame_tick))
		game_state->tainted = 1;
	
	game_state->entity0 = centity0;
	
	while(game_state->tick < render_tick)
	{
		game_state->next = malloc(sizeof(struct game_state_t));
		duplicate_game_state(game_state, game_state->next);
		game_state->next->tick = game_state->tick + 1;
		game_state = game_state->next;
		
		cgame_state = game_state;
		centity0 = game_state->entity0;
		cgame_tick = game_state->tick;
	
		s_tick_entities(&centity0);
		
		if(process_tick_events_do_not_remove(cgame_tick))
			game_state->tainted = 1;
		
		game_state->entity0 = centity0;
	}
}


/*

void update_game()
{
	update_tick_parameters();
	uint32_t tick = get_game_tick();
	uint32_t last_known_tick = min(last_event_tick, tick);
	
	cgame_tick = game_tick;
	centity0 = entity0;
	
	
//	if(event0)
	{
		if(cgame_tick <= last_known_tick)
		{
			while(cgame_tick < last_known_tick)
			{
				process_tick_events(cgame_tick);
				cgame_tick++;
				s_tick_entities(&centity0);
			}
			
			process_tick_events(cgame_tick);
	
			game_tick = cgame_tick;
			entity0 = centity0;
		}
		
		pgame_tick = game_tick;
		LL_REMOVE_ALL(struct entity_t, &pentity0);
		pentity0 = duplicate_entities(entity0);
	}
	
	
	cgame_tick = pgame_tick;
	centity0 = pentity0;
	

	process_tick_events_do_not_remove(cgame_tick);	// process remaining ooo streams
	
	while(cgame_tick < tick)
	{
		cgame_tick++;
		s_tick_entities(&centity0);
		process_tick_events_do_not_remove(cgame_tick);
	}
	
	pgame_tick = cgame_tick;
	pentity0 = centity0;
}



*/


void render_particle(float wx, float wy, uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue)
{
	int x, y;
	
	world_to_screen(wx, wy, &x, &y);

	struct blit_params_t params;
	params.dest = s_backbuffer;
	
	params.dest_x = x;
	params.dest_y = y;
	
	params.alpha = alpha;
	
	plot_alpha_pixel(&params);
	
	params.alpha >>= 1;
	
	params.dest_x++;
	plot_alpha_pixel(&params);
	
	params.dest_x--;
	params.dest_y++;
	plot_alpha_pixel(&params);
	
	params.dest_y -= 2;
	plot_alpha_pixel(&params);
	
	params.dest_x--;
	params.dest_y++;
	plot_alpha_pixel(&params);
}



void render_rail_trails()
{
	struct rail_trail_t *rail_trail = rail_trail0;
		
	double time = get_double_time();
	
	while(rail_trail)
	{
		double rail_time = time - rail_trail->start_time;
		
		if(rail_time > 2.5)
		{
			struct rail_trail_t *next = rail_trail->next;
			LL_REMOVE(struct rail_trail_t, &rail_trail0, rail_trail);
			rail_trail = next;
			continue;
		}
			
		
		double expand_time = min(rail_time, 0.5);
		
		double r = RAIL_TRAIL_INITIAL_EXPAND_VELOCITY * expand_time + 
			0.5 * RAIL_TRAIL_EXPAND_ACC * expand_time * expand_time;
		
		double deltax = rail_trail->x2 - rail_trail->x1;
		double deltay = rail_trail->y2 - rail_trail->y1;
		
		double length = hypot(deltax, deltay);
		
		deltax /= length;
		deltay /= length;
		
		int p, np = floor(length / 5);
		
		for(p = 0; p < np; p++)
		{
			double t = (double)p / (double)np;
			
			double cx = rail_trail->x1 + (rail_trail->x2 - rail_trail->x1) * t;
			double cy = rail_trail->y1 + (rail_trail->y2 - rail_trail->y1) * t;

			uint8_t alpha;

			if(rail_time < 0.5)
				alpha  = 0xff;
			else
				alpha  = (uint8_t)(255 - floor((rail_time - 0.5) / 2.0 * 255.0f));
			
			render_particle(cx, cy, alpha, 0xff, 0xff, 0xff);
			
			double theta = 30 * M_PI * t;
			
			double nr = cos(theta) * r;
			
			
			
			cx -= deltay * nr;
			cy += deltax * nr;
	
			if(rail_time < 0.5)
				alpha  = 0xff;
			else
				alpha  = (uint8_t)(255 - floor((rail_time - 0.5) / 2.0 * 255.0f));
			render_particle(cx, cy, alpha, 0, 7, 0xff);
		}
		
		rail_trail = rail_trail->next;
	}
}


void render_game()
{
	if(game_state != GAMESTATE_PLAYING)
		return;
	
	update_game();
	
	
	struct entity_t *entity = get_entity(centity0, cgame_state->follow_me);
		
	if(entity)
	{
		viewx = entity->xdis;
		viewy = entity->ydis;
	}
	else
	{
		viewx = 0.0;
		viewy = 0.0;
	}
	
//	add_offset_view(entity);
	add_moving_view();
	
	render_stars();
	render_lower_particles();
	render_rail_trails();
	render_entities();
	render_upper_particles();
	render_map();
}


void qc_name(char *new_name)
{
	if(game_state == GAMESTATE_PLAYING || 
		game_state == GAMESTATE_SPECTATING)
	{
		net_emit_uint8(EMNETMSG_NAMECNGE);
		net_emit_string(new_name);
		net_emit_end_of_stream();
	}		
}


void init_game()
{
	init_stars();
	init_particles();
	
	create_cvar_command("status", cf_status);
	create_cvar_command("say", cf_say);
	create_cvar_command("play", cf_play);
	create_cvar_command("spectate", cf_spectate);
	
	set_string_cvar_qc_function("name", qc_name);
	
	
	struct surface_t *temp = read_png_surface(PKGDATADIR "/stock-object-textures/plasma.png");
		
	s_plasma = resize(temp, 14, 14, NULL);
	
	temp = read_png_surface(PKGDATADIR "/stock-object-textures/craft-shield.png");
	
	s_craft_shield = resize(temp, 57, 57, NULL);
	s_weapon_shield = resize(temp, 36, 36, NULL);
	
}


void kill_game()
{
}
