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
#include "../shared/rdtsc.h"
#include "../shared/cvar.h"
#include "../shared/network.h"
#include "../shared/sgame.h"
#include "rcon.h"
#include "map.h"
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
#include "entry.h"
#include "sound.h"

#ifdef LINUX
#include "../shared/timer.h"
#endif

#ifdef WIN32
#include "../common/win32/math.h"
#endif


struct event_craft_data_t
{
	float acc;
	float theta;
	int braking;
	float shield_flare;
	
	int carcass;	// handled sepatately
};


struct event_weapon_data_t
{
	int type;
	float theta;
	float shield_flare;
};


struct event_rocket_data_t
{
	float theta;
	uint32_t weapon_id;
};


struct event_plasma_data_t
{
	uint32_t weapon_id;
};


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
				struct event_craft_data_t craft_data;
				struct event_weapon_data_t weapon_data;
				struct event_rocket_data_t rocket_data;
				struct event_plasma_data_t plasma_data;
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


struct message_reader_t
{
	uint8_t type;
	struct buffer_t *stream;
	gzFile *gzdemo;

	uint8_t message_type;
	uint32_t event_tick;

} message_reader;


#define MESSAGE_READER_STREAM					0
#define MESSAGE_READER_GZDEMO					1
#define MESSAGE_READER_STREAM_WRITE_GZDEMO		2
#define MESSAGE_READER_STREAM_WRITE_GZDEMO_NO	3	// we are reading a net only message


#define GAMESTATE_DEAD			0
#define GAMESTATE_DEMO			1
#define GAMESTATE_CONNECTING	2
#define GAMESTATE_SPECTATING	3
#define GAMESTATE_PLAYING		4

int game_state = GAMESTATE_DEAD;
int recording = 0;
struct string_t *recording_filename;
gzFile gzrecording;

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


uint32_t game_conn;



struct game_state_t
{
	uint32_t tick;
	struct entity_t *entity0;
	uint32_t follow_me;			// what is this for?
	int tainted;	
	
	struct game_state_t *next;
		
} *game_state0, last_known_game_state, *cgame_state;


uint32_t demo_first_tick;
uint32_t demo_last_tick;
uint32_t demo_follow_me;

gzFile gzdemo;

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
	*screenx = (int)floor(worldx * (double)(vid_width) / 1600.0) - 
		(int)floor(viewx * (double)(vid_width) / 1600.0) + vid_width / 2;
	*screeny = vid_height / 2 - 1 - (int)floor(worldy * (double)(vid_width) / 1600.0) + 
		(int)floor(viewy * (double)(vid_width) / 1600.0);
}


void screen_to_world(int screenx, int screeny, double *worldx, double *worldy)
{
	*worldx = ((double)screenx - vid_width / 2 + 0.5f) / 
		((double)(vid_width) / 1600.0) + viewx;
	*worldy = (((double)(vid_height / 2 - 1 - screeny)) + 0.5f) / 
		((double)(vid_width) / 1600.0) + viewx;
}
*/


void world_to_screen(double worldx, double worldy, int *screenx, int *screeny)
{
	*screenx = (int)floor((worldx - viewx) * ((double)(vid_width) / 1600.0)) + vid_width / 2;
	*screeny = vid_height / 2 - 1 - (int)floor((worldy - viewy) * ((double)(vid_width) / 1600.0));
}


void screen_to_world(int screenx, int screeny, double *worldx, double *worldy)
{
	*worldx = ((double)screenx - vid_width / 2 + 0.5f) / 
		((double)(vid_width) / 1600.0) + viewx;
	*worldy = (((double)(vid_height / 2 - 1 - screeny)) + 0.5f) / 
		((double)(vid_width) / 1600.0) + viewx;
}

/*
void add_offset_view(struct entity_t *entity)
{
	double sin_theta, cos_theta;
	sincos(entity->craft_data.theta, &sin_theta, &cos_theta);
	
	double target_x = - sin_theta * 500;// + entity->xvel * 24.0;
	double target_y = + cos_theta * 500;// + entity->yvel * 24.0;
	
	double deltax = target_x - offset_view_x;
	double deltay = target_y - offset_view_y;
	
	offset_view_x += deltax / 500.0;
	offset_view_y += deltay / 500.0;
	
	viewx += offset_view_x;
	viewy += offset_view_y;
}
*/

void start_moving_view(float x1, float y1, float x2, float y2)
{
	moving_view_x = x2 - x1;
	moving_view_y = y2 - y1;
	
	moving_view_xa = ((moving_view_x) / 2) / (0.5 * (MOVING_VIEW_TRAVEL_TIME * 0.5) * 
		(MOVING_VIEW_TRAVEL_TIME * 0.5));
	moving_view_ya = ((moving_view_y) / 2) / (0.5 * (MOVING_VIEW_TRAVEL_TIME * 0.5) * 
		(MOVING_VIEW_TRAVEL_TIME * 0.5));
	
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
		viewx += 0.5 * moving_view_xa * MOVING_VIEW_TRAVEL_TIME * 0.5 * 
			MOVING_VIEW_TRAVEL_TIME * 0.5;
		viewy += 0.5 * moving_view_ya * MOVING_VIEW_TRAVEL_TIME * 0.5 * 
			MOVING_VIEW_TRAVEL_TIME * 0.5;
		
		viewx += moving_view_xa * MOVING_VIEW_TRAVEL_TIME * 0.5 * 
			(time - MOVING_VIEW_TRAVEL_TIME * 0.5);
		viewy += moving_view_ya * MOVING_VIEW_TRAVEL_TIME * 0.5 * 
			(time - MOVING_VIEW_TRAVEL_TIME * 0.5);
		
		viewx += 0.5 * -moving_view_xa * (time - MOVING_VIEW_TRAVEL_TIME * 0.5) * 
			(time - MOVING_VIEW_TRAVEL_TIME * 0.5);
		viewy += 0.5 * -moving_view_ya * (time - MOVING_VIEW_TRAVEL_TIME * 0.5) * 
			(time - MOVING_VIEW_TRAVEL_TIME * 0.5);
	}
}


int message_reader_more()
{
	switch(message_reader.type)
	{
	case MESSAGE_READER_STREAM:
	case MESSAGE_READER_STREAM_WRITE_GZDEMO:
	case MESSAGE_READER_STREAM_WRITE_GZDEMO_NO:
		return buffer_more(message_reader.stream);
	
	case MESSAGE_READER_GZDEMO:
		return !gzeof(message_reader.gzdemo);
	}
	
	return 0;
}


uint8_t message_reader_read_uint8()
{
	uint8_t d;
	
	switch(message_reader.type)
	{
	case MESSAGE_READER_STREAM:
	case MESSAGE_READER_STREAM_WRITE_GZDEMO_NO:
		return buffer_read_uint8(message_reader.stream);
	
	case MESSAGE_READER_GZDEMO:
		gzread(message_reader.gzdemo, &d, 1);
		return d;

	case MESSAGE_READER_STREAM_WRITE_GZDEMO:
		d = buffer_read_uint8(message_reader.stream);
		gzwrite(message_reader.gzdemo, &d, 1);
		return d;
	}
	
	return 0;
}


uint32_t message_reader_read_uint32()
{
	uint32_t d;
	
	switch(message_reader.type)
	{
	case MESSAGE_READER_STREAM:
	case MESSAGE_READER_STREAM_WRITE_GZDEMO_NO:
		return buffer_read_uint32(message_reader.stream);
	
	case MESSAGE_READER_GZDEMO:
		gzread(message_reader.gzdemo, &d, 4);
		return d;

	case MESSAGE_READER_STREAM_WRITE_GZDEMO:
		d = buffer_read_uint32(message_reader.stream);
		gzwrite(message_reader.gzdemo, &d, 4);
		return d;
	}
	
	return 0;
}


int message_reader_read_int()
{
	uint32_t d = message_reader_read_uint32();
	return *(int*)(&d);
}


float message_reader_read_float()
{
	uint32_t d = message_reader_read_uint32();
	return *(float*)(void*)(&d);
}


struct string_t *message_reader_read_string()
{
	struct string_t *s;
	
	switch(message_reader.type)
	{
	case MESSAGE_READER_STREAM:
	case MESSAGE_READER_STREAM_WRITE_GZDEMO_NO:
		return buffer_read_string(message_reader.stream);
	
	case MESSAGE_READER_GZDEMO:
		s = gzread_string(message_reader.gzdemo);
		return s;

	case MESSAGE_READER_STREAM_WRITE_GZDEMO:
		s = buffer_read_string(message_reader.stream);
		gzwrite_string(message_reader.gzdemo, s);
		return s;
	}
	
	return NULL;
}


int message_reader_new_message()
{
	if(!message_reader_more())
		return 0;
	
	int old_reader_type = message_reader.type;
	
	if(old_reader_type == MESSAGE_READER_STREAM_WRITE_GZDEMO_NO)
		old_reader_type = MESSAGE_READER_STREAM_WRITE_GZDEMO;
		
	if(message_reader.type == MESSAGE_READER_STREAM_WRITE_GZDEMO)
		message_reader.type = MESSAGE_READER_STREAM_WRITE_GZDEMO_NO;
	
	message_reader.message_type = message_reader_read_uint8();
	
	switch((message_reader.message_type & EMMSGCLASS_MASK))
	{
	case EMMSGCLASS_STND:
		if(old_reader_type == MESSAGE_READER_STREAM_WRITE_GZDEMO)
			gzwrite(message_reader.gzdemo, &message_reader.message_type, 1);
		message_reader.type = old_reader_type;
		break;
	
	case EMMSGCLASS_NETONLY:
		if(old_reader_type == MESSAGE_READER_GZDEMO)
			;	// do something: the demo file is corrupt
		break;
	
	case EMMSGCLASS_EVENT:
		if(message_reader.message_type != EMEVENT_DUMMY)
		{
			if(old_reader_type == MESSAGE_READER_STREAM_WRITE_GZDEMO)
				gzwrite(message_reader.gzdemo, &message_reader.message_type, 1);
			message_reader.type = old_reader_type;
		}
		
		message_reader.event_tick = message_reader_read_uint32();
		break;
	
	default:
		// do something: the demo file is corrupt
		break;
	}
	
	return 1;
}


void read_craft_data(struct event_craft_data_t *craft_data)
{
	craft_data->acc = message_reader_read_float();
	craft_data->theta = message_reader_read_float();
	craft_data->braking = message_reader_read_int();
	craft_data->shield_flare = message_reader_read_float();
}


void read_weapon_data(struct event_weapon_data_t *weapon_data)
{
	weapon_data->type = message_reader_read_int();
	weapon_data->theta = message_reader_read_float();
	weapon_data->shield_flare = message_reader_read_float();
}


void read_rocket_data(struct event_rocket_data_t *rocket_data)
{
	rocket_data->theta = message_reader_read_float();
	rocket_data->weapon_id = message_reader_read_uint32();
}


void read_plasma_data(struct event_plasma_data_t *plasma_data)
{
	plasma_data->weapon_id = message_reader_read_uint32();
}


void read_spawn_ent_event(struct event_t *event)
{
	event->ent_data.index = message_reader_read_uint32();
	event->ent_data.type = message_reader_read_uint8();
	event->ent_data.skin = message_reader_read_uint32();
	event->ent_data.xdis = message_reader_read_float();
	event->ent_data.ydis = message_reader_read_float();
	event->ent_data.xvel = message_reader_read_float();
	event->ent_data.yvel = message_reader_read_float();
	
	switch(event->ent_data.type)
	{
	case ENT_CRAFT:
		read_craft_data(&event->ent_data.craft_data);
	
		// separate because carcass data is not sent in update
		event->ent_data.craft_data.carcass = message_reader_read_uint8();
		break;
	
	case ENT_WEAPON:
		read_weapon_data(&event->ent_data.weapon_data);
		break;
	
	case ENT_BOGIE:
		read_plasma_data(&event->ent_data.plasma_data);
		break;
	
	case ENT_ROCKET:
		read_rocket_data(&event->ent_data.rocket_data);
		break;
	
	case ENT_MINE:
		break;
	}
}


void read_update_ent_event(struct event_t *event)
{
	event->ent_data.index = message_reader_read_uint32();
	event->ent_data.type = message_reader_read_uint8();
	event->ent_data.xdis = message_reader_read_float();
	event->ent_data.ydis = message_reader_read_float();
	event->ent_data.xvel = message_reader_read_float();
	event->ent_data.yvel = message_reader_read_float();
	
	switch(event->ent_data.type)
	{
	case ENT_CRAFT:	
		read_craft_data(&event->ent_data.craft_data);
		break;
	
	case ENT_WEAPON:
		read_weapon_data(&event->ent_data.weapon_data);
		break;
	
	case ENT_BOGIE:
		read_plasma_data(&event->ent_data.plasma_data);
		break;
	
	case ENT_ROCKET:
		read_rocket_data(&event->ent_data.rocket_data);
		break;
	
	case ENT_MINE:
		break;
	}
}


void read_kill_ent_event(struct event_t *event)
{
	event->ent_data.index = message_reader_read_uint32();
}


void read_follow_me_event(struct event_t *event)
{
	event->follow_me_data.index = message_reader_read_uint32();
}


void read_carcass_event(struct event_t *event)
{
	event->carcass_data.index = message_reader_read_uint32();
}


void read_railtrail_event(struct event_t *event)
{
	event->railtrail_data.x1 = message_reader_read_float();
	event->railtrail_data.y1 = message_reader_read_float();
	event->railtrail_data.x2 = message_reader_read_float();
	event->railtrail_data.y2 = message_reader_read_float();
}


void read_event(struct event_t *event)
{
	event->tick = message_reader.event_tick;
	event->type = message_reader.message_type;
	
	switch(event->type)
	{
	case EMEVENT_SPAWN_ENT:
		read_spawn_ent_event(event);
		break;
	
	case EMEVENT_UPDATE_ENT:
		read_update_ent_event(event);
		break;
	
	case EMEVENT_KILL_ENT:
		read_kill_ent_event(event);
		break;
	
	case EMEVENT_FOLLOW_ME:
		read_follow_me_event(event);
		break;
	
	case EMEVENT_CARCASS:
		read_carcass_event(event);
		break;
	
	case EMEVENT_RAILTRAIL:
		read_railtrail_event(event);
		break;
	}
}


void game_process_connecting()
{
	console_print(">");
}


void game_process_cookie_echoed()
{
	console_print("<\xbb");
}


void game_process_connection(uint32_t conn)
{
	game_state = GAMESTATE_CONNECTING;
	game_conn = conn;
	
	if(!recording)
		message_reader.type = MESSAGE_READER_STREAM;
	
	console_print("\xab\nConnected!\n");
}


void game_process_connection_failed()
{
	console_print("\n");
}


void game_process_disconnection(uint32_t conn)
{
	game_state = GAMESTATE_DEAD;
	
//	LL_REMOVE_ALL(struct entity_t, &entity0);
//	game_state = GAMESTATE_DEAD;
	console_print("Disconnected!\n");
}


void game_process_conn_lost(uint32_t conn)
{
	game_state = GAMESTATE_DEAD;
	console_print("Connection lost.\n");
}


int game_process_print()
{
	struct string_t *s = message_reader_read_string();
	console_print(s->text);
	free_string(s);
	return 1;
}


int game_process_proto_ver()
{
	if(game_state != GAMESTATE_CONNECTING && game_state != GAMESTATE_DEMO)
		return 0;
	
	uint8_t proto_ver = message_reader_read_uint8();
	
	if(proto_ver == EM_PROTO_VER)
	{
		console_print("Correct protocol version\n");
		
		if(game_state != GAMESTATE_DEMO)
		{
			net_emit_uint8(game_conn, EMMSG_JOIN);
			net_emit_string(game_conn, get_cvar_string("name"));
			net_emit_end_of_stream(game_conn);
		}
	}
	else
	{
		console_print("Incorrect protocol version\n");
		game_state = GAMESTATE_DEAD;
		em_disconnect(game_conn);
	}
	
	return 1;
}


int game_process_playing()
{
	if(game_state == GAMESTATE_DEAD)
		return 0;
	
	
	game_state0 = calloc(1, sizeof(struct game_state_t));
	game_state0->tick = message_reader_read_uint32();		// oh dear
	
	last_known_game_state.tick = game_state0->tick;
	
	
//	game_tick = 
//	console_print("start tick: %u\n", game_tick);
	
	game_state = GAMESTATE_PLAYING;

	return 1;
}


int game_process_spectating()
{
	if(game_state == GAMESTATE_DEAD)
		return 0;
	
	game_state = GAMESTATE_SPECTATING;

	return 1;
}


void add_spawn_ent_event(struct event_t *event)
{
	event->ent_data.index = message_reader_read_uint32();
	event->ent_data.type = message_reader_read_uint8();
	event->ent_data.skin = message_reader_read_uint32();
	event->ent_data.xdis = message_reader_read_float();
	event->ent_data.ydis = message_reader_read_float();
	event->ent_data.xvel = message_reader_read_float();
	event->ent_data.yvel = message_reader_read_float();
	
	switch(event->ent_data.type)
	{
	case ENT_CRAFT:
		read_craft_data(&event->ent_data.craft_data);
	
		// separate because carcass data is not sent in update_event
		event->ent_data.craft_data.carcass = message_reader_read_uint8();
		break;
	
	case ENT_WEAPON:
		read_weapon_data(&event->ent_data.weapon_data);
		break;
	
	case ENT_BOGIE:
		read_plasma_data(&event->ent_data.plasma_data);
		break;
	
	case ENT_ROCKET:
		read_rocket_data(&event->ent_data.rocket_data);
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
		entity->craft_data.braking = event->ent_data.craft_data.braking;
		entity->craft_data.shield_flare = event->ent_data.craft_data.shield_flare;
		entity->craft_data.carcass = event->ent_data.craft_data.carcass;
		entity->craft_data.skin = event->ent_data.skin;
		entity->craft_data.surface = skin_get_craft_surface(event->ent_data.skin);
		entity->craft_data.particle = 0.0;
		break;
	
	case ENT_WEAPON:
		entity->weapon_data.type = event->ent_data.weapon_data.type;
		entity->weapon_data.theta = event->ent_data.weapon_data.theta;
		entity->weapon_data.shield_flare = event->ent_data.weapon_data.shield_flare;
		entity->weapon_data.skin = event->ent_data.skin;

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


void add_update_ent_event(struct event_t *event)
{
	event->ent_data.index = message_reader_read_uint32();
	event->ent_data.type = message_reader_read_uint8();
	event->ent_data.xdis = message_reader_read_float();
	event->ent_data.ydis = message_reader_read_float();
	event->ent_data.xvel = message_reader_read_float();
	event->ent_data.yvel = message_reader_read_float();
	
	switch(event->ent_data.type)
	{
	case ENT_CRAFT:	
		read_craft_data(&event->ent_data.craft_data);
		break;
	
	case ENT_WEAPON:
		read_weapon_data(&event->ent_data.weapon_data);
		break;
	
	case ENT_BOGIE:
		read_plasma_data(&event->ent_data.plasma_data);
		break;
	
	case ENT_ROCKET:
		read_rocket_data(&event->ent_data.rocket_data);
		break;
	
	case ENT_MINE:
		break;
	}
}


void process_update_ent_event(struct event_t *event)
{
	struct entity_t *entity = get_entity(centity0, event->ent_data.index);

	if(!entity)
	{
		printf("consistency error in process_update_ent_event\n");
		return;		// due to ooo
	}
	
	entity->xdis = event->ent_data.xdis;
	entity->ydis = event->ent_data.ydis;
	entity->xvel = event->ent_data.xvel;
	entity->yvel = event->ent_data.yvel;
	
	switch(entity->type)
	{
	case ENT_CRAFT:
		entity->craft_data.acc = event->ent_data.craft_data.acc;
		entity->craft_data.theta = event->ent_data.craft_data.theta;
		entity->craft_data.braking = event->ent_data.craft_data.braking;
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


void add_kill_ent_event(struct event_t *event)
{
	event->ent_data.index = message_reader_read_uint32();
}


void process_kill_ent_event(struct event_t *event)
{
	struct entity_t *entity = get_entity(centity0, event->ent_data.index);

	if(!entity)
		return;		// due to ooo
	
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


void add_follow_me_event(struct event_t *event)
{
	event->follow_me_data.index = message_reader_read_uint32();
}


void process_follow_me_event(struct event_t *event)
{
	struct entity_t *entity = get_entity(centity0, event->follow_me_data.index);
	
	if(!entity)
		return;		// due to ooo

	if(game_state == GAMESTATE_DEMO)
		demo_follow_me = event->follow_me_data.index;	
	else
		cgame_state->follow_me = event->follow_me_data.index;
	start_moving_view(viewx, viewy, entity->xdis, entity->ydis);	// violates gamestate scheme
		// should possibly not call this second time around after ooo
}


void add_carcass_event(struct event_t *event)
{
	event->carcass_data.index = message_reader_read_uint32();
}


void process_carcass_event(struct event_t *event)
{
	struct entity_t *craft = get_entity(centity0, event->carcass_data.index);

	if(!craft)
		return;		// due to ooo

	craft->craft_data.carcass = 1;
	craft->craft_data.particle = 0.0;
	
	strip_weapons_from_craft(craft);
}


void add_railtrail_event(struct event_t *event)
{
	event->railtrail_data.x1 = message_reader_read_float();
	event->railtrail_data.y1 = message_reader_read_float();
	event->railtrail_data.x2 = message_reader_read_float();
	event->railtrail_data.y2 = message_reader_read_float();
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
	start_sample(&railgun_sample, event->tick);
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
			case EMEVENT_SPAWN_ENT:
			//	console_print("process_spawn_ent_event; tick: %u; index: %u\n", tick, event->index);
				process_spawn_ent_event(event);
				break;
			
			case EMEVENT_UPDATE_ENT:
				process_update_ent_event(event);
				break;
			
			case EMEVENT_KILL_ENT:
				process_kill_ent_event(event);
				break;
			
			case EMEVENT_FOLLOW_ME:
			//	console_print("process_follow_me_event; tick: %u; index: %u\n", tick, event->index);
				process_follow_me_event(event);
				break;
			
			case EMEVENT_CARCASS:
				process_carcass_event(event);
				break;
			
			case EMEVENT_RAILTRAIL:
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
			case EMEVENT_SPAWN_ENT:
				process_spawn_ent_event(event);
				any = 1;
				break;
			
			case EMEVENT_UPDATE_ENT:
				process_update_ent_event(event);
				any = 1;
				break;
			
			case EMEVENT_KILL_ENT:
				process_kill_ent_event(event);
				any = 1;
				break;

			case EMEVENT_FOLLOW_ME:
				process_follow_me_event(event);
				any = 1;
				break;
			
			case EMEVENT_CARCASS:
				process_carcass_event(event);
				break;

			case EMEVENT_RAILTRAIL:
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
			console_print("ooo neutralized\n");
			
			event->ooo = 0;
			return 1;
		}
		
		event = event->next;
	}
	
	return 0;
}


void insert_event_in_order(struct event_t *event)
{
	struct event_t *cevent, *temp;


	// if event0 is NULL, then create new event here

	if(!event0)
	{
		event0 = malloc(sizeof(struct event_t));
		memcpy(event0, event, sizeof(struct event_t));
		event0->next = NULL;
		return;
	}

	
	if(event0->index > event->index)
	{
		temp = event0;
		event0 = malloc(sizeof(struct event_t));
		memcpy(event0, event, sizeof(struct event_t));
		event0->next = temp;
		return;
	}


	// search through the rest of the list
	// to find the event before the first
	// event to have a index greater
	// than event->index
	
	cevent = event0;
	
	while(cevent->next)
	{
		if(cevent->next->index > event->index)
		{
			temp = cevent->next;
			cevent->next = malloc(sizeof(struct event_t));
			cevent = cevent->next;
			memcpy(cevent, event, sizeof(struct event_t));
			cevent->next = temp;
			return;
		}

		cevent = cevent->next;
	}


	// we have reached the end of the list and not found
	// an event that has a index greater than or equal to event->index

	cevent->next = malloc(sizeof(struct event_t));
	cevent = cevent->next;
	memcpy(cevent, event, sizeof(struct event_t));
	cevent->next = NULL;
}


int game_demo_process_event()
{
	if(message_reader.message_type == EMEVENT_DUMMY)
		return 1;
	
	struct event_t event;
	
	event.tick = message_reader.event_tick;
	
	event.type = message_reader.message_type;
	event.ooo = 0;
	
	switch(event.type)
	{
	case EMEVENT_SPAWN_ENT:
		add_spawn_ent_event(&event);
		break;
	
	case EMEVENT_UPDATE_ENT:
		add_update_ent_event(&event);
		break;
	
	case EMEVENT_KILL_ENT:
		add_kill_ent_event(&event);
		break;
	
	case EMEVENT_FOLLOW_ME:
		add_follow_me_event(&event);
		break;
	
	case EMEVENT_CARCASS:
		add_carcass_event(&event);
		break;
	
	case EMEVENT_RAILTRAIL:
		add_railtrail_event(&event);
		break;
	}
	
	LL_ADD_TAIL(struct event_t, &event0, &event);
	
	
	return 1;
}


int game_process_event_timed(uint32_t index, uint64_t *stamp)
{
	struct event_t event;
	int ooon = 0;
	
	event.tick = message_reader.event_tick;
	
	add_game_tick(event.tick, stamp);
	
	if(get_event_by_ooo_index(index))
		ooon = 1;
	
	event.type = message_reader.message_type;
	event.ooo = 0;
	event.index = index;
	
	switch(event.type)
	{
	case EMEVENT_SPAWN_ENT:
		add_spawn_ent_event(&event);
		break;
	
	case EMEVENT_UPDATE_ENT:
		add_update_ent_event(&event);
		break;
	
	case EMEVENT_KILL_ENT:
		add_kill_ent_event(&event);
		break;
	
	case EMEVENT_FOLLOW_ME:
		add_follow_me_event(&event);
		break;
	
	case EMEVENT_CARCASS:
		add_carcass_event(&event);
		break;
	
	case EMEVENT_RAILTRAIL:
		add_railtrail_event(&event);
		break;
	}
	
	if(!ooon)
		insert_event_in_order(&event);
	
	
	return 1;
}


int game_process_event_untimed(uint32_t index)
{
	struct event_t event;
	int ooon = 0;
	
	event.tick = message_reader.event_tick;
	event.type = message_reader.message_type;
	
	if(get_event_by_ooo_index(index))
		ooon = 1;
	
	event.ooo = 0;
	event.index = index;
	
	switch(event.type)
	{
	case EMEVENT_SPAWN_ENT:
		add_spawn_ent_event(&event);
		break;
	
	case EMEVENT_UPDATE_ENT:
		add_update_ent_event(&event);
		break;
	
	case EMEVENT_KILL_ENT:
		add_kill_ent_event(&event);
		break;
	
	case EMEVENT_FOLLOW_ME:
		add_follow_me_event(&event);
		break;
	
	case EMEVENT_CARCASS:
		add_carcass_event(&event);
		break;
	
	case EMEVENT_RAILTRAIL:
		add_railtrail_event(&event);
		break;
	}
	
	if(!ooon)
		insert_event_in_order(&event);
	
	
	return 1;
}


int game_process_event_timed_ooo(uint32_t index, uint64_t *stamp)
{
	struct event_t event;
	
	event.tick = message_reader.event_tick;
	add_game_tick(event.tick, stamp);
	event.type = message_reader.message_type;
	event.ooo = 1;
	event.index = index;
	
	switch(event.type)
	{
	case EMEVENT_SPAWN_ENT:
		add_spawn_ent_event(&event);
		break;
	
	case EMEVENT_UPDATE_ENT:
		add_update_ent_event(&event);
		break;
	
	case EMEVENT_KILL_ENT:
		add_kill_ent_event(&event);
		break;
	
	case EMEVENT_FOLLOW_ME:
		add_follow_me_event(&event);
		break;
	
	case EMEVENT_CARCASS:
		add_carcass_event(&event);
		break;
	
	case EMEVENT_RAILTRAIL:
		add_railtrail_event(&event);
		break;
	}
	
	insert_event_in_order(&event);
	
	
	return 1;
}


int game_process_event_untimed_ooo(uint32_t index)
{
	struct event_t event;
	
	event.tick = message_reader.event_tick;
	event.type = message_reader.message_type;
	event.ooo = 1;
	event.index = index;
	
	switch(event.type)
	{
	case EMEVENT_SPAWN_ENT:
		add_spawn_ent_event(&event);
		break;
	
	case EMEVENT_UPDATE_ENT:
		add_update_ent_event(&event);
		break;
	
	case EMEVENT_KILL_ENT:
		add_kill_ent_event(&event);
		break;
	
	case EMEVENT_FOLLOW_ME:
		add_follow_me_event(&event);
		break;
	
	case EMEVENT_CARCASS:
		add_carcass_event(&event);
		break;
	
	case EMEVENT_RAILTRAIL:
		add_railtrail_event(&event);
		break;
	}
	
	insert_event_in_order(&event);
	
	
	return 1;
}


int game_process_message()
{
	switch(message_reader.message_type)
	{
	case EMMSG_PROTO_VER:
		return game_process_proto_ver();
	
	case EMMSG_LOADMAP:
		return game_process_load_map();
	
	case EMMSG_LOADSKIN:
		return game_process_load_skin();
		
	case EMNETMSG_PRINT:
		return game_process_print();
	
	case EMNETMSG_PLAYING:
		return game_process_playing();
	
	case EMNETMSG_SPECTATING:
		return game_process_spectating();
	
	case EMNETMSG_INRCON:
		break;
	
	case EMNETMSG_NOTINRCON:
		break;
	
	case EMNETMSG_JOINED:
		break;
	}
	
	return 0;
}


void game_process_stream_timed(uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
	message_reader.stream = stream;
	
	while(message_reader_new_message())
	{
		switch(message_reader.message_type & EMMSGCLASS_MASK)
		{
		case EMMSGCLASS_STND:
		case EMMSGCLASS_NETONLY:
			if(!game_process_message())
				return;
			break;
			
		case EMMSGCLASS_EVENT:
			if(!game_process_event_timed(index, stamp))
				return;
			break;
			
		default:
			return;
		}
	}
}


void game_process_stream_untimed(uint32_t index, struct buffer_t *stream)
{
	message_reader.stream = stream;
	
	while(message_reader_new_message())
	{
		switch(message_reader.message_type & EMMSGCLASS_MASK)
		{
		case EMMSGCLASS_STND:
		case EMMSGCLASS_NETONLY:
			if(!game_process_message())
				return;
			break;
			
		case EMMSGCLASS_EVENT:
			if(!game_process_event_untimed(index))
				return;
			break;
			
		default:
			return;
		}
	}
}


void game_process_stream_timed_ooo(uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
	message_reader.stream = stream;
	
	while(message_reader_new_message())
	{
		switch(message_reader.message_type & EMMSGCLASS_MASK)
		{
		case EMMSGCLASS_EVENT:
			if(!game_process_event_timed_ooo(index, stamp))
				return;
			break;
			
		default:
			return;
		}
	}
}


void game_process_stream_untimed_ooo(uint32_t index, struct buffer_t *stream)
{
	message_reader.stream = stream;
	
	while(message_reader_new_message())
	{
		switch(message_reader.message_type & EMMSGCLASS_MASK)
		{
		case EMMSGCLASS_EVENT:
			if(!game_process_event_untimed_ooo(index))
				return;
			break;
			
		default:
			return;
		}
	}
}


void game_resolution_change()
{
	reload_map();
	reload_skins();
	
	// getting entities' new surfaces
	
	struct game_state_t *game_state = game_state0;
		
	while(game_state)
	{
		struct entity_t *entity = game_state->entity0;
	
		while(entity)
		{
			switch(entity->type)
			{
			case ENT_CRAFT:
				entity->craft_data.surface = skin_get_craft_surface(entity->craft_data.skin);
				break;
			
			case ENT_WEAPON:
				switch(entity->weapon_data.type)
				{
				case WEAPON_PLASMA_CANNON:
					entity->weapon_data.surface = 
						skin_get_plasma_cannon_surface(entity->weapon_data.skin);
					break;
				
				case WEAPON_MINIGUN:
					entity->weapon_data.surface = 
						skin_get_minigun_surface(entity->weapon_data.skin);
					break;
				
				case WEAPON_ROCKET_LAUNCHER:
					entity->weapon_data.surface = 
						skin_get_rocket_launcher_surface(entity->weapon_data.skin);
					break;
				}
	
				break;
			}
			
			entity = entity->next;
		}
		
		game_state = game_state->next;
	}
}


void cf_status(char *c)
{
/*	if(net_state != NETSTATE_CONNECTED)
		console_print("You are not connected.\n");
	else
	{
		net_emit_uint8(game_conn, EMMSG_STATUS);
		net_emit_end_of_stream(game_conn);
	}
*/}


void cf_say(char *c)
{
/*	if(net_state != NETSTATE_CONNECTED)
		console_print("You are not connected.\n");
	else
	{
		net_emit_uint8(game_conn, EMMSG_SAY);
		net_emit_string(game_conn, c);
		net_emit_end_of_stream(game_conn);
	}
*/}


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
		net_emit_uint8(game_conn, EMMSG_SPECTATE);
		net_emit_end_of_stream(game_conn);
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
		net_emit_uint8(game_conn, EMMSG_PLAY);
		net_emit_end_of_stream(game_conn);
		break;
	
	case GAMESTATE_PLAYING:
		console_print("You are already playing.\n");
		break;
	}	
}


void cf_record(char *c)
{
	if(recording)
		;
	
	switch(game_state)
	{
	case GAMESTATE_DEAD:
	case GAMESTATE_DEMO:
	case GAMESTATE_CONNECTING:
		console_print("Recording will begin when you start playing or spectating.\n");
		break;
	
	case GAMESTATE_SPECTATING:
	case GAMESTATE_PLAYING:
		console_print("Recording...\n");
		break;
	}
	
	message_reader.type = MESSAGE_READER_STREAM_WRITE_GZDEMO;
	recording_filename = new_string_text(c);
	gzrecording = gzopen(c, "wb9");
	message_reader.gzdemo = gzrecording;
	
	recording = 1;
}


void cf_stop(char *c)
{
	recording = 0;
	gzclose(gzrecording);
	message_reader.type = MESSAGE_READER_STREAM;
}


void cf_demo(char *c)
{
	switch(game_state)
	{
	case GAMESTATE_DEAD:
		gzdemo = gzopen(c, "rb");
		game_state = GAMESTATE_DEMO;
		message_reader.gzdemo = gzdemo;
		message_reader.type = MESSAGE_READER_GZDEMO;
		break;
		
	case GAMESTATE_DEMO:
	case GAMESTATE_CONNECTING:
	case GAMESTATE_SPECTATING:
	case GAMESTATE_PLAYING:
		break;
	}
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
		
		switch(game_state)
		{
		case GAMESTATE_PLAYING:
			particle.creation = particle.last = get_time_from_game_tick(cgame_tick + 
				(float)p / (float)np - 1.0);
			break;
		
		case GAMESTATE_DEMO:
			particle.creation = particle.last = (double)(demo_last_tick - demo_first_tick) / 200.0;
			break;
		}
		
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
			

			switch(game_state)
			{
			case GAMESTATE_PLAYING:
				particle.creation = particle.last = get_time_from_game_tick(cgame_tick + 
					(float)p / (float)np - 1.0);
				break;
			
			case GAMESTATE_DEMO:
				particle.creation = particle.last = (double)(demo_last_tick + 
					(float)p / (float)np - 1.0 - demo_first_tick) / 200.0;
				break;
			}
			
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
			
			
	
			switch(game_state)
			{
			case GAMESTATE_PLAYING:
				particle.creation = particle.last = get_time_from_game_tick(cgame_tick + 
					(float)p / (float)np - 1.0);
				break;
			
			case GAMESTATE_DEMO:
				particle.creation = particle.last = (double)(demo_last_tick + 
					(float)p / (float)np - 1.0 - demo_first_tick) / 200.0;
				break;
			}
			
		
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

		switch(game_state)
		{
		case GAMESTATE_PLAYING:
			particle.creation = particle.last = get_time_from_game_tick(cgame_tick + 
				(float)p / (float)np - 1.0);
			break;
		
		case GAMESTATE_DEMO:
				particle.creation = particle.last = (double)(demo_last_tick + 
					(float)p / (float)np - 1.0 - demo_first_tick) / 200.0;
			break;
		}
		
		
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
			params.source_y = (lrint((entity->craft_data.theta / (2 * M_PI) + 0.5) * ROTATIONS) % 
				ROTATIONS) * entity->craft_data.surface->width;
		
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
			
				alpha_blit_surface(&params);
			}
		
			break;
		
		case ENT_WEAPON:
			params.source = entity->weapon_data.surface;
		
			params.source_x = 0;
			params.source_y = (lrint((entity->weapon_data.theta / (2 * M_PI) + 0.5) * ROTATIONS) % 
				ROTATIONS) * entity->weapon_data.surface->width;
		
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
		
			alpha_blit_surface(&params);
		
			break;
		
		case ENT_BOGIE:

			params.source = s_plasma;
		
			world_to_screen(entity->xdis, entity->ydis, &x, &y);
		
			params.red = 0x32;
			params.green = 0x73;
			params.blue = 0x71;
			
			params.dest_x = x - s_plasma->width / 2;
			params.dest_y = y - s_plasma->width / 2;
			
			blit_surface(&params);
		
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
			
			blit_surface(&params);
		
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
				centity->craft_data.left_weapon = get_entity(new_game_state->entity0, 
					centity->craft_data.left_weapon->index);
			
			if(centity->craft_data.right_weapon)
				centity->craft_data.right_weapon = get_entity(new_game_state->entity0, 
					centity->craft_data.right_weapon->index);
			break;
		
		case ENT_WEAPON:
			if(centity->weapon_data.craft)
				centity->weapon_data.craft = get_entity(new_game_state->entity0, 
					centity->weapon_data.craft->index);
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


void update_demo()
{
	uint32_t tick;
	
	if(demo_first_tick)		// i.e. not the first event
	{
		tick = get_tick() + demo_first_tick;
	}
	else
	{
		message_reader_new_message();
	}
	
	
	do
	{
		int stop = 0;
		switch(message_reader.message_type & EMMSGCLASS_MASK)
		{
		case EMMSGCLASS_STND:
			if(!game_process_message())
				return;
			break;
			
		case EMMSGCLASS_EVENT:
			if(!demo_first_tick)
			{
				demo_last_tick = demo_first_tick = 
					tick = message_reader.event_tick;
				
				reset_start_count();	// demo_first_tick is being rendered now
			}
			
			if(message_reader.event_tick > tick)
			{
				stop = 1;
				break;
			}
				
			if(!game_demo_process_event())
			{
				return;
			}
			break;
			
		default:
			return;
		}
		
		if(stop)
			break;
	}
	while(message_reader_new_message());
	
	
	while(demo_last_tick <= tick)
	{
		s_tick_entities(&centity0);
		process_tick_events(demo_last_tick);
		cgame_tick = ++demo_last_tick;		// cgame_tick is presumed to start at 0!!!
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
	params.red = red;
	params.green = green;
	params.blue = blue;
	params.dest = s_backbuffer;
	
	params.dest_x = x;
	params.dest_y = y;
	
	params.alpha = alpha;
	
	alpha_plot_pixel(&params);
	
	params.alpha >>= 1;
	
	params.dest_x++;
	alpha_plot_pixel(&params);
	
	params.dest_x--;
	params.dest_y++;
	alpha_plot_pixel(&params);
	
	params.dest_y -= 2;
	alpha_plot_pixel(&params);
	
	params.dest_x--;
	params.dest_y++;
	alpha_plot_pixel(&params);
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


void render_recording()
{
	if(recording)
	{
		blit_text_centered(((vid_width / 3) - (vid_width / 200)) / 2, vid_height / 6, 
			0xff, 0xff, 0xff, s_backbuffer, "recording %s", recording_filename->text);
	}
}


void render_game()
{
	struct entity_t *entity;
	switch(game_state)
	{
	case GAMESTATE_PLAYING:
		update_game();
		entity = get_entity(centity0, cgame_state->follow_me);
		break;
	
	case GAMESTATE_DEMO:
		update_demo();
		entity = get_entity(centity0, demo_follow_me);
		break;
	
	default:
		return;
	}
	
	
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
	render_recording();
	
	blit_text(((vid_width * 2) / 3) + (vid_width / 200), 
		vid_height / 6, 0xef, 0x6f, 0xff, s_backbuffer, "[virus] where are you?");
}


void qc_name(char *new_name)
{
	if(game_state == GAMESTATE_PLAYING || 
		game_state == GAMESTATE_SPECTATING)
	{
		net_emit_uint8(game_conn, EMMSG_NAMECNGE);
		net_emit_string(game_conn, new_name);
		net_emit_end_of_stream(game_conn);
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
	
	create_cvar_command("record", cf_record);
	create_cvar_command("stop", cf_stop);
	create_cvar_command("demo", cf_demo);
	
	create_cvar_command("connect", em_connect);
	
	
	set_string_cvar_qc_function("name", qc_name);
	
	
	struct surface_t *temp = read_png_surface(PKGDATADIR "/stock-object-textures/plasma.png");
		
	s_plasma = resize(temp, 14, 14, NULL);
	
	temp = read_png_surface(PKGDATADIR "/stock-object-textures/craft-shield.png");
	
//	s_craft_shield = resize(temp, 57, 57, NULL);
//	s_weapon_shield = resize(temp, 36, 36, NULL);
	
	s_craft_shield = resize(temp, 73, 73, NULL);
	s_weapon_shield = resize(temp, 46, 46, NULL);
}


void kill_game()
{
}
