#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#ifdef WIN32
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <zlib.h>

#include "../common/types.h"
#include "../common/minmax.h"
#include "../shared/cvar.h"
#include "../common/llist.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../shared/parse.h"
#include "../shared/timer.h"
#include "../shared/network.h"
#include "../shared/sgame.h"
#include "../shared/objects.h"
#include "../shared/bsp.h"
#include "network.h"
#include "game.h"
#include "console.h"
#include "ping.h"
#include "main.h"



struct pickup_spawn_point_t
{
	uint8_t type;
	double x, y;
	
	int respawn;
	uint32_t respawn_tick;
	
	union
	{
		struct
		{
			int plasmas;
			double angle;
			
		} plasma_cannon_data;
		
		struct
		{
			int bullets;
			double angle;
			
		} minigun_data;
		
		struct
		{
			int rockets;
			double angle;
			
		} rocket_launcher_data;
		
		struct
		{
			int quantity;
			double angle;
			
		} rails_data;
		
		struct
		{
			int shield_energy;
			double angle;
			
		} shield_energy_data;
	};
	
	int respawn_delay;
	
	struct pickup_spawn_point_t *next;
	
} *pickup_spawn_point0 = NULL;


struct string_t *map_filename = NULL;

uint32_t game_tick;
uint32_t cgame_tick;

int game_state = GS_DEAD;

int num_players = 0;
int num_players_active = 0;
int max_players = 20;
int max_players_active = 10;

struct player_t *player0 = NULL;
struct entity_t *entity0;


struct player_t *new_player()
{
	struct player_t *player;
	LL_CALLOC(struct player_t, &player0, &player);
	return player;
}


void remove_player(uint32_t conn)
{
	struct player_t *temp, *player;

	if(!player0)
		return;

	if(player0->conn == conn)
	{
		temp = player0->next;
		free(player0);
		player0 = temp;

		return;
	}

	player = player0;
	
	while(player->next)
	{
		if(player->next->conn == conn)
		{
			temp = player->next->next;

			free(player->next);

			player->next = temp;

			break;
		}
	}
}


struct player_t *get_player_by_conn(uint32_t conn)
{
	struct player_t *player = player0;
	
	while(player)
	{
		if(player->conn == conn)
			return player;

		player = player->next;
	}

	return NULL;
}


struct player_t *get_player_by_craft(struct entity_t *craft)
{
	struct player_t *player = player0;
	
	while(player)
	{
		if(player->craft == craft)
			return player;

		player = player->next;
	}

	return NULL;
}


struct player_t *get_player_by_name(char *name)
{
	if(!name)
		return NULL;
	
	struct player_t *player = player0;
	
	while(player)
	{
		if(strcmp(player->name->text, name) == 0)
			return player;

		player = player->next;
	}

	return NULL;
}


void write_craft_data_to_net(uint32_t conn, struct entity_t *craft)
{
	net_emit_float(conn, craft->craft_data.acc);
	net_emit_float(conn, craft->craft_data.theta);
	net_emit_float(conn, craft->craft_data.omega);
	net_emit_float(conn, craft->craft_data.shield_flare);
}


void write_weapon_data_to_net(uint32_t conn, struct entity_t *weapon)
{
	net_emit_int(conn, weapon->weapon_data.type);
	net_emit_float(conn, weapon->weapon_data.theta);
	net_emit_float(conn, weapon->weapon_data.shield_flare);
}


void write_plasma_data_to_net(uint32_t conn, struct entity_t *plasma)
{
	net_emit_uint32(conn, plasma->plasma_data.weapon_id);
}


void write_rocket_data_to_net(uint32_t conn, struct entity_t *rocket)
{
	net_emit_float(conn, rocket->rocket_data.theta);
	net_emit_uint32(conn, rocket->rocket_data.weapon_id);
}


void write_mine_data_to_net(uint32_t conn, struct entity_t *mine)
{
	;
}


void write_rails_data_to_net(uint32_t conn, struct entity_t *rails)
{
	;
}


void write_shield_data_to_net(uint32_t conn, struct entity_t *shield)
{
	;
}


void propagate_entity(struct entity_t *entity)
{
	struct player_t *player = player0;

	while(player)
	{
		net_emit_uint8(player->conn, EMNETMSG_EVENT);
		net_emit_uint32(player->conn, game_tick);
		net_emit_uint8(player->conn, EMNETEVENT_UPDATE_ENT);
		net_emit_uint32(player->conn, entity->index);
		net_emit_uint8(player->conn, entity->type);
		net_emit_float(player->conn, entity->xdis);
		net_emit_float(player->conn, entity->ydis);
		net_emit_float(player->conn, entity->xvel);
		net_emit_float(player->conn, entity->yvel);
		
		switch(entity->type)
		{
		case ENT_CRAFT:
			write_craft_data_to_net(player->conn, entity);
			break;
		
		case ENT_WEAPON:
			write_weapon_data_to_net(player->conn, entity);
			break;
		
		case ENT_BOGIE:
			write_plasma_data_to_net(player->conn, entity);
			break;
		
		case ENT_ROCKET:
			write_rocket_data_to_net(player->conn, entity);
			break;
		
		case ENT_MINE:
			write_mine_data_to_net(player->conn, entity);
			break;
		
		case ENT_RAILS:
			write_rails_data_to_net(player->conn, entity);
			break;
		
		case ENT_SHIELD:
			write_shield_data_to_net(player->conn, entity);
			break;
		}
		
		net_emit_end_of_stream(player->conn);
		player = player->next;
	}
}


void load_map_on_player(struct player_t *player)
{
	net_emit_uint8(player->conn, EMNETMSG_LOADMAP);
	net_emit_string(player->conn, "default");
}


void load_all_skins_on_player(struct player_t *player)
{
	net_emit_uint8(player->conn, EMNETMSG_LOADSKIN);
	net_emit_string(player->conn, "default");
	net_emit_uint32(player->conn, 0);			// index
}


void spawn_all_entities_on_player(struct player_t *player)
{
	struct entity_t *centity = entity0;

	while(centity)
	{
		net_emit_uint8(player->conn, EMNETMSG_EVENT);
		net_emit_uint32(player->conn, game_tick);
		net_emit_uint8(player->conn, EMNETEVENT_SPAWN_ENT);
		net_emit_uint32(player->conn, centity->index);
		net_emit_uint8(player->conn, centity->type);
		net_emit_uint32(player->conn, 0);		// skin
		net_emit_float(player->conn, centity->xdis);
		net_emit_float(player->conn, centity->ydis);
		net_emit_float(player->conn, centity->xvel);
		net_emit_float(player->conn, centity->yvel);
		
		switch(centity->type)
		{
		case ENT_CRAFT:
			write_craft_data_to_net(player->conn, centity);
			net_emit_int(player->conn, centity->craft_data.carcass);
			break;
		
		case ENT_WEAPON:
			write_weapon_data_to_net(player->conn, centity);
			break;
		
		case ENT_BOGIE:
			write_plasma_data_to_net(player->conn, centity);
			break;
		
		case ENT_ROCKET:
			write_rocket_data_to_net(player->conn, centity);
			break;
		
		case ENT_MINE:
			write_mine_data_to_net(player->conn, centity);
			break;
		
		case ENT_RAILS:
			write_rails_data_to_net(player->conn, centity);
			break;
		
		case ENT_SHIELD:
			write_shield_data_to_net(player->conn, centity);
			break;
		}
		
		centity = centity->next;
	}
	
	net_emit_end_of_stream(player->conn);

}


void spawn_entity_on_all_players(struct entity_t *entity)
{
	struct player_t *player = player0;
		
	while(player)
	{
		net_emit_uint8(player->conn, EMNETMSG_EVENT);
		net_emit_uint32(player->conn, game_tick);
		net_emit_uint8(player->conn, EMNETEVENT_SPAWN_ENT);
		net_emit_uint32(player->conn, entity->index);
		net_emit_uint8(player->conn, entity->type);
		net_emit_uint32(player->conn, 0);		// skin
		net_emit_float(player->conn, entity->xdis);
		net_emit_float(player->conn, entity->ydis);
		net_emit_float(player->conn, entity->xvel);
		net_emit_float(player->conn, entity->yvel);
		
		switch(entity->type)
		{
		case ENT_CRAFT:
			write_craft_data_to_net(player->conn, entity);
			net_emit_int(player->conn, entity->craft_data.carcass);
			break;
		
		case ENT_WEAPON:
			write_weapon_data_to_net(player->conn, entity);
			break;
		
		case ENT_BOGIE:
			write_plasma_data_to_net(player->conn, entity);
			break;
		
		case ENT_ROCKET:
			write_rocket_data_to_net(player->conn, entity);
			break;
		
		case ENT_MINE:
			write_mine_data_to_net(player->conn, entity);
			break;
		
		case ENT_RAILS:
			write_rails_data_to_net(player->conn, entity);
			break;
		
		case ENT_SHIELD:
			write_shield_data_to_net(player->conn, entity);
			break;
		}
		
		net_emit_end_of_stream(player->conn);
		player = player->next;
	}
}


void remove_entity_from_all_players(struct entity_t *entity)
{
	struct player_t *player = player0;
		
	while(player)
	{
		net_emit_uint8(player->conn, EMNETMSG_EVENT);
		net_emit_uint32(player->conn, game_tick);
		net_emit_uint8(player->conn, EMNETEVENT_KILL_ENT);
		net_emit_uint32(player->conn, entity->index);
		net_emit_end_of_stream(player->conn);
		player = player->next;
	}
}


void make_carcass_on_all_players(struct entity_t *craft)
{
	struct player_t *player = player0;
		
	while(player)
	{
		net_emit_uint8(player->conn, EMNETMSG_EVENT);
		net_emit_uint32(player->conn, game_tick);
		net_emit_uint8(player->conn, EMNETEVENT_CARCASS);
		net_emit_uint32(player->conn, craft->index);
		net_emit_end_of_stream(player->conn);
		player = player->next;
	}
}


void follow_me_on_player(struct player_t *player)
{
	net_emit_uint8(player->conn, EMNETMSG_EVENT);
	net_emit_uint32(player->conn, game_tick);
	net_emit_uint8(player->conn, EMNETEVENT_FOLLOW_ME);
	net_emit_uint32(player->conn, player->craft->index);
	net_emit_end_of_stream(player->conn);
}


void print_on_player(struct player_t *player, char *text)
{
	net_emit_uint8(player->conn, EMNETMSG_PRINT);
	net_emit_string(player->conn, text);
	net_emit_end_of_stream(player->conn);
}


void print_on_all_players(char *text)
{
	struct player_t *player = player0;
		
	while(player)
	{
		print_on_player(player, text);
		player = player->next;
	}
}


int game_process_status(struct player_t *player, struct buffer_t *buffer)
{
	struct string_t *string = new_string();

	struct player_t *cplayer = player0;

	while(cplayer)
	{
		string_cat_string(string, cplayer->name);
		string_cat_char(string, '\n');

		cplayer = cplayer->next;
	}

	net_emit_uint8(player->conn, EMNETMSG_PRINT);
	net_emit_string(player->conn, string->text);
	net_emit_end_of_stream(player->conn);

	free_string(string);
	
	return 1;
}


void spawn_bogie(struct entity_t *weapon)
{
	struct entity_t *plasma = new_entity(&entity0);
	
	plasma->type = ENT_BOGIE;
	
	plasma->xdis = weapon->xdis;
	plasma->ydis = weapon->ydis;
	
	double sin_theta, cos_theta;
	sincos(weapon->weapon_data.theta, &sin_theta, &cos_theta);
	
	plasma->xvel = weapon->xvel - sin_theta * 8.0;
	plasma->yvel = weapon->yvel + cos_theta * 8.0;
	
	plasma->plasma_data.in_weapon = 1;
	plasma->plasma_data.weapon_id = weapon->index;

	spawn_entity_on_all_players(plasma);
}


void spawn_rocket(struct entity_t *weapon)
{
	struct entity_t *rocket = new_entity(&entity0);
	
	rocket->type = ENT_ROCKET;
	
	rocket->xdis = weapon->xdis;
	rocket->ydis = weapon->ydis;
	
	rocket->xvel = weapon->xvel;
	rocket->yvel = weapon->yvel;
	
	rocket->rocket_data.theta = weapon->weapon_data.theta;
	rocket->rocket_data.start_tick = game_tick;
	rocket->rocket_data.in_weapon = 1;
	rocket->rocket_data.weapon_id = weapon->index;
	
	spawn_entity_on_all_players(rocket);
}


void kick(char *args)
{
	char *token = strtok(args, " ");
	
	struct player_t *player = get_player_by_name(token);
	if(!player)
	{
		console_print("Kick who?\n");
		return;
	}

	print_on_player(player, "You have been kicked.\n");
	disconnect(player->conn);	// unimplemented
	remove_player(player->conn);
	
	struct string_t *s = new_string_text(args);
	string_cat_text(s, " was kicked\n");
	print_on_all_players(s->text);
	free_string(s);
	
	num_players--;
}


void craft_destroyed(struct entity_t *craft)
{
	;
}


void weapon_destroyed(struct entity_t *weapon)
{
	;
}


void craft_telefragged(struct entity_t *craft)
{
	struct player_t *player = get_player_by_craft(craft);
	assert(player);
	
//	print_on_all_clients("%s was telefragged.\n", player->name->text);
}


void weapon_telefragged(struct entity_t *weapon)
{
	;
}


void tick_player(struct player_t *player)
{
//	if(!player->craft && player->respawn_tick == game_tick)
//		spawn_player(player);
	
	// see if we need to send a dummy event
	
	if(player->next_dummy_event == game_tick)
	{
		net_emit_uint8(player->conn, EMNETMSG_EVENT);
		net_emit_uint32(player->conn, game_tick);
		net_emit_uint8(player->conn, EMNETEVENT_DUMMY);
		net_emit_end_of_stream(player->conn);

		player->next_dummy_event += 200;
	}
	
	
	// fire guns
	
	if(player->firing_left)
	{
		if(!player->craft->craft_data.left_weapon)
		{
			player->firing_left = 0;
		}
		else
		{
			if(player->craft->craft_data.left_weapon->weapon_data.ammo)		
			{
				int fire = ((game_tick - player->firing_left_start) * 20) / 200 - player->left_fired;
				
				if(fire > 0)
				{
					spawn_bogie(player->craft->craft_data.left_weapon);
					player->left_fired += fire;
					player->craft->craft_data.left_weapon->weapon_data.ammo--;
				}
			}
		}
	}
	
	if(player->firing_right)
	{
		if(!player->craft->craft_data.right_weapon)
		{
			player->firing_right = 0;
		}
		else
		{
			if(player->craft->craft_data.right_weapon->weapon_data.ammo)		
			{
				int fire = ((game_tick - player->firing_right_start) * 20) / 200 - player->right_fired;
				
				if(fire > 0)
				{
					spawn_bogie(player->craft->craft_data.right_weapon);
					player->right_fired += fire;
					player->craft->craft_data.right_weapon->weapon_data.ammo--;
				}
			}
		}
	}
}


void tick_game()
{
	// apply shared tick semantics and associated server-side callbacks
	s_tick_entities(&entity0);
		
	
	// propagate unpredictable entity changes
	
	
	struct player_t *player = player0;
	while(player)
	{
		tick_player(player);
		player = player->next;
	}
	

//	tick_map();
}


void spawn_player(struct player_t *player)
{
	player->rails = 10;
	
	struct spawn_point_t *spawn_point = spawn_point0;
	
	int respawn_index = 0;
	
	while(spawn_point)
	{
		if(!spawn_point->teleport_only)
		{
			struct entity_t *entity = entity0;
			
			int occluded = 0;
			
			while(entity)
			{
/*				switch(entity->type)
				{
				case ENT_CRAFT:
					if(circles_intersect(spawn_point->x, spawn_point->y, CRAFT_RADIUS, entity->xdis, entity->ydis, CRAFT_RADIUS))
						occluded = 1;
					break;
					
				case ENT_WEAPON:
					if(circles_intersect(spawn_point->x, spawn_point->y, CRAFT_RADIUS, entity->xdis, entity->ydis, WEAPON_RADIUS))
						occluded = 1;
					break;
					
				case ENT_BOGIE:
					if(circles_intersect(spawn_point->x, spawn_point->y, CRAFT_RADIUS, entity->xdis, entity->ydis, BOGIE_RADIUS))
						occluded = 1;
					break;
					
				case ENT_BULLET:
					if(point_in_circle(entity->xdis, entity->ydis, spawn_point->x, spawn_point->y, CRAFT_RADIUS))
						occluded = 1;
					break;
					
				case ENT_ROCKET:
					if(circles_intersect(spawn_point->x, spawn_point->y, CRAFT_RADIUS, entity->xdis, entity->ydis, ROCKET_RADIUS))
						occluded = 1;
					break;
					
				case ENT_MINE:
					if(circles_intersect(spawn_point->x, spawn_point->y, CRAFT_RADIUS, entity->xdis, entity->ydis, MINE_RADIUS))
						occluded = 1;
					break;
					
				case ENT_RAILS:
					if(circles_intersect(spawn_point->x, spawn_point->y, CRAFT_RADIUS, entity->xdis, entity->ydis, RAILS_RADIUS))
						occluded = 1;
					break;
					
				case ENT_SHIELD:
					if(circles_intersect(spawn_point->x, spawn_point->y, CRAFT_RADIUS, entity->xdis, entity->ydis, SHIELD_RADIUS))
						occluded = 1;
					break;
				}
*/				
				if(occluded)
					break;
				
				entity = entity->next;
			}
			
			if(!occluded)
			{
				spawn_point->respawn_index = respawn_index++;
			}
			else
			{
				spawn_point->respawn_index = -1;
			}
		}
		
		spawn_point = spawn_point->next;
	}
	
	struct entity_t *old_craft = player->craft;
	
	player->craft = new_entity(&entity0);
	player->craft->type = ENT_CRAFT;
	player->craft->craft_data.shield_strength = 1.0;
	
	if(old_craft)
	{
		player->craft->craft_data.acc = old_craft->craft_data.acc;
		player->craft->craft_data.omega = old_craft->craft_data.omega;
	}
		
	
	if(respawn_index)
	{
		int i = (int)(drand48() * respawn_index);
		
		spawn_point = spawn_point0;
		
		while(spawn_point->respawn_index != i)
			spawn_point = spawn_point->next;
		
		player->craft->xdis = spawn_point->x;
		player->craft->ydis = spawn_point->y;
		
		spawn_entity_on_all_players(player->craft);
		follow_me_on_player(player);
	}
	else
	{
		int i = 0; //rand % num_spawn_points;
		
		spawn_point = spawn_point0;
		
		while(respawn_index != i)
			spawn_point = spawn_point->next;
		
		player->craft->xdis = spawn_point->x;
		player->craft->ydis = spawn_point->y;
		
	//	check_craft_teleportation(player->craft);
		
		if(player->craft->kill_me)
		{
//			remove_entity(player->craft);
			player->craft = NULL;
		}
		else
		{
			spawn_entity_on_all_players(player->craft);
			follow_me_on_player(player);
		}
	}
}



void respawn_craft(struct entity_t *craft)
{
	struct player_t *player = player0;
		
	while(player)
	{
		if(player->craft == craft)
		{
			spawn_player(player);
			return;
		}
		
		player = player->next;
	}
}


/*
void tick_map()
{
	struct pickup_spawn_point_t *spawn_point = pickup_spawn_point0;
		
	while(spawn_point)
	{
		if(spawn_point->respawn && spawn_point->respawn_tick == game_tick)
		{
			struct entity_t *entity = new_entity();
			
			entity->xdis = spawn_point->x;
			entity->ydis = spawn_point->y;
			entity->xvel = 0.0;
			entity->yvel = 0.0;
			
			switch(spawn_point->type)
			{
			case OBJECTTYPE_ROCKETLAUNCHER:
				entity->type = ENT_WEAPON;
//				entity->weapon_data.type = WEAPON_ROCKETLAUNCHER;
				entity->weapon_data.craft = NULL;
//				entity->weapon_data.shield_flare = NULL;
//				entity->weapon_data.ammo = spawn_point->rocket_launcher_data.rockets;
//				entity->weapon_data.shield_strength = 100.0;
				break;
			
			case OBJECTTYPE_MINIGUN:
				entity->type = ENT_WEAPON;
//				entity->weapon_data.type = WEAPON_MINIGUN;
				entity->weapon_data.craft = NULL;
//				entity->weapon_data.shield_flare = NULL;
//				entity->weapon_data.ammo = spawn_point->minigun_data.bullets;
//				entity->weapon_data.shield_strength = 100.0;
				break;
			
			case OBJECTTYPE_RAILS:
				entity->type = ENT_RAILS;
//				entity->rails_data.quantity = spawn_point->rails_data.quantity;
				break;
			
			case OBJECTTYPE_SHIELDENERGY:
				entity->type = ENT_SHIELD;
//				entity->shield_data.strength = spawn_point->shield_energy_data.shield_energy;
				break;
			}
			
			spawn_entity_on_all_players(spawn_point);
		}
		
		spawn_point = spawn_point->next;
	}
}
*/


void propagate_entities()
{
	struct entity_t *entity = entity0;
		
	while(entity)
	{
		if(entity->propagate_me)
		{
			propagate_entity(entity);
			entity->propagate_me = 0;
		}
		
		entity = entity->next;
	}
}


void update_game()
{
//	if(!(timer_count % 100))
	//	ping_all_clients();
	
	uint32_t tick = get_tick();

	while(game_tick != tick)
	{
		game_tick++;
		cgame_tick = game_tick;
		tick_game();
	}
	
	propagate_entities();
}


void game_process_join(uint32_t conn, uint32_t index, struct buffer_t *stream)
{
	if(game_state != GS_ALIVE)
	{
		net_emit_uint8(conn, EMNETMSG_PRINT);
		net_emit_string(conn, "This server is not active. Go Away!\n");
		net_emit_end_of_stream(conn);
		disconnect(conn);
		return;
	}
	
	if(num_players >= max_players)
	{
		net_emit_uint8(conn, EMNETMSG_PRINT);
		net_emit_string(conn, "No room!\n");
		net_emit_end_of_stream(conn);
		disconnect(conn);
		return;
	}
	
	num_players++;
		
	int spec = 0;
	
	if(num_players_active >= max_players_active)
		spec = 1;
	else
	{
		if(num_players_active > 1)
			spec = 1;
	}
	
	struct string_t *name = buffer_read_string(stream);
	
	struct string_t *s = new_string_string(name);
		
	if(spec)
	{
		string_cat_text(s, " joined the spectators\n");
	}
	else
	{
		string_cat_text(s, " joined the game\n");
		num_players_active++;
	}
	
	console_print(s->text);
	print_on_all_players(s->text);
	free_string(s);
	
	struct player_t *player = new_player();
	player->conn = conn;
	player->name = name;
	player->last_control_change = index;
	player->next_dummy_event = game_tick + 200;

	
	print_on_player(player, "Welcome to this server.\n");
	
	load_map_on_player(player);
	
	if(spec)
	{
		player->state = PLAYER_STATE_SPECTATING;
		print_on_player(player, "You are spectating.\n");
		net_emit_uint8(player->conn, EMNETMSG_SPECTATING);
		net_emit_end_of_stream(player->conn);
	}
	else
	{
		load_all_skins_on_player(player);
		
		player->state = PLAYER_STATE_ACTIVE;
		net_emit_uint8(player->conn, EMNETMSG_PLAYING);
		net_emit_uint32(player->conn, game_tick);
		net_emit_end_of_stream(player->conn);

		spawn_all_entities_on_player(player);
		
		spawn_player(player);
	}
}


void destroy_player(struct player_t *player)
{
	struct entity_t *craft = player->craft;
	remove_player(player->conn);
	remove_entity_from_all_players(craft);
	remove_entity(&entity0, craft);
}


int game_process_play(struct player_t *player, struct buffer_t *stream)
{
	if(player->state != PLAYER_STATE_SPECTATING)
		return 0;
	
	if(num_players_active >= max_players_active)
	{
		net_emit_string(player->conn, "No room!\n");
		net_emit_end_of_stream(player->conn);
		return 1;
	}
	
	num_players_active++;
	
	struct string_t *s = new_string_string(player->name);
	string_cat_text(s, " started playing\n");
	print_on_all_players(s->text);
	console_print(s->text);
	free_string(s);
	
	net_emit_uint8(player->conn, EMNETMSG_PLAYING);
	net_emit_end_of_stream(player->conn);
	
	player->state = PLAYER_STATE_ACTIVE;
	
	return 1;
}


int game_process_spectate(struct player_t *player, struct buffer_t *stream)
{
	if(player->state != PLAYER_STATE_ACTIVE)
		return 0;
	
	num_players_active--;
	
	struct string_t *s = new_string_string(player->name);
	string_cat_text(s, " started spectating\n");
	print_on_all_players(s->text);
	console_print(s->text);
	free_string(s);
	
	net_emit_uint8(player->conn, EMNETMSG_SPECTATING);
	net_emit_end_of_stream(player->conn);
	
	player->state = PLAYER_STATE_SPECTATING;
	
	return 1;
}


void game_process_disconnection(uint32_t conn)
{
	struct player_t *player = get_player_by_conn(conn);
	if(!player)
		return;
	
	num_players--;
	
	if(player->state == PLAYER_STATE_ACTIVE)
		num_players_active--;
	
	struct string_t *s = new_string_string(player->name);
	string_cat_text(s, " left\n");
	destroy_player(player);
	
	console_print(s->text);
	print_on_all_players(s->text);
}


void game_process_conn_lost(uint32_t conn)
{
	struct player_t *player = get_player_by_conn(conn);
	if(!player)
		return;
	
	num_players--;
	
	if(player->state == PLAYER_STATE_ACTIVE)
		num_players_active--;
	
	struct string_t *s = new_string_string(player->name);
	string_cat_text(s, " lost connectivity\n");
	destroy_player(player);
	
	console_print(s->text);
	print_on_all_players(s->text);
}


int game_process_say(struct player_t *player, struct buffer_t *stream)
{
	struct string_t *s = new_string_string(player->name);
	string_cat_text(s, ": ");
	struct string_t *says = buffer_read_string(stream);
	string_cat_string(s, says);	
	free_string(says);
	string_cat_text(s, "\n");
	
	console_print(s->text);
	print_on_all_players(s->text);
	free_string(s);
	return 1;
}


int game_process_name_change(struct player_t *player, struct buffer_t *stream)
{
	struct string_t *s = new_string_string(player->name);
		
	free_string(player->name);
	player->name = buffer_read_string(stream);
	
	string_cat_text(s, " is now ");
	string_cat_string(s, player->name);
	string_cat_text(s, "\n");

	console_print(s->text);
	print_on_all_players(s->text);
	free_string(s);
	return 1;
}


int game_process_control_change(struct player_t *player, uint32_t index, struct buffer_t *stream)
{
	if(index <= player->last_control_change)
		index += EMNETINDEX_MAX + 1;
	
	if(!(index < player->last_control_change + 100))
		return 0;

	player->last_control_change = index;
	
	float thrust = buffer_read_float(stream);
	float roll = buffer_read_float(stream);
	
	thrust = max(thrust, 0.0);
	thrust = min(thrust, 1.0);
	player->craft->craft_data.acc = thrust * 0.020;
	
	roll = max(roll, -1.0);
	roll = min(roll, 1.0);
	player->craft->craft_data.omega = -roll * 0.075;
	
	propagate_entity(player->craft);
	
	return 1;
}


void propagate_rail_trail(float x1, float y1, float x2, float y2)
{
	struct player_t *player = player0;

	while(player)
	{
		net_emit_uint8(player->conn, EMNETMSG_EVENT);
		net_emit_uint32(player->conn, game_tick);
		net_emit_uint8(player->conn, EMNETEVENT_RAILTRAIL);
		net_emit_float(player->conn, x1);
		net_emit_float(player->conn, y1);
		net_emit_float(player->conn, x2);
		net_emit_float(player->conn, y2);
		
		net_emit_end_of_stream(player->conn);
		player = player->next;
	}
}


int game_process_fire_rail(struct player_t *player)
{
	if(!player->rails)
		return 1;
	
	player->rails--;
	
	double sin_theta, cos_theta;
	sincos(player->craft->craft_data.theta, &sin_theta, &cos_theta);
	
	player->craft->xvel += 0.8 * sin_theta;
	player->craft->yvel -= 0.8 * cos_theta;
	
	propagate_entity(player->craft);
	
	float x1 = player->craft->xdis - CRAFT_RADIUS * sin_theta;
	float y1 = player->craft->ydis + CRAFT_RADIUS * cos_theta;
	float x2 = player->craft->xdis - (CRAFT_RADIUS + 1000) * sin_theta;
	float y2 = player->craft->ydis + (CRAFT_RADIUS + 1000) * cos_theta;
	
	
	propagate_rail_trail(x1, y1, x2, y2);
	
	return 1;
}


int game_process_fire_left(struct player_t *player, struct buffer_t *stream)
{
	if(!player->craft->craft_data.left_weapon)
		return 0;
	
	uint32_t state = buffer_read_uint32(stream);
	
	switch(player->craft->craft_data.left_weapon->weapon_data.type)
	{
	case WEAPON_PLASMA_CANNON:
		player->firing_left = state;
	
		if(state && player->craft->craft_data.left_weapon->weapon_data.ammo)
		{
			spawn_bogie(player->craft->craft_data.left_weapon);
			player->firing_left_start = game_tick + 1;
			player->left_fired = 0;
			player->craft->craft_data.left_weapon->weapon_data.ammo--;
		}

		break;
	
	case WEAPON_MINIGUN:
		break;
	
	case WEAPON_ROCKET_LAUNCHER:
		
		if(state && player->craft->craft_data.left_weapon->weapon_data.ammo)
		{
			spawn_rocket(player->craft->craft_data.left_weapon);
			player->craft->craft_data.left_weapon->weapon_data.ammo--;
		}
		
		break;
	}
	
	return 1;
}


int game_process_fire_right(struct player_t *player, struct buffer_t *stream)
{
	if(!player->craft->craft_data.right_weapon)
		return 0;
	
	uint32_t state = buffer_read_uint32(stream);
	
	switch(player->craft->craft_data.right_weapon->weapon_data.type)
	{
	case WEAPON_PLASMA_CANNON:
		player->firing_right = state;
	
		if(state && player->craft->craft_data.right_weapon->weapon_data.ammo)
		{
			spawn_bogie(player->craft->craft_data.right_weapon);
			player->firing_right_start = game_tick + 1;
			player->right_fired = 0;
			player->craft->craft_data.right_weapon->weapon_data.ammo--;
		}

		break;
	
	case WEAPON_MINIGUN:
		break;
	
	case WEAPON_ROCKET_LAUNCHER:
		
		if(state && player->craft->craft_data.right_weapon->weapon_data.ammo)
		{
			spawn_rocket(player->craft->craft_data.right_weapon);
			player->craft->craft_data.right_weapon->weapon_data.ammo--;
		}
		
		break;
	}
	
	return 1;
}


void game_process_stream(uint32_t conn, uint32_t index, struct buffer_t *stream)
{
	update_game();
	
	struct player_t *player = get_player_by_conn(conn);
	assert(player);

	while(buffer_more(stream))
	{		
		switch(buffer_read_uint8(stream))
		{
		case EMNETMSG_PLAY:
			if(!game_process_play(player, stream))
				return;
			break;
		
		case EMNETMSG_SPECTATE:
			if(!game_process_spectate(player, stream))
				return;
			break;
		
		case EMNETMSG_STATUS:
			if(!game_process_status(player, stream))
				return;
			break;
		
		case EMNETMSG_SAY:
			if(!game_process_say(player, stream))
				return;
			break;
		
		case EMNETMSG_NAMECNGE:
			if(!game_process_name_change(player, stream))
				return;
			break;
			
		case EMNETMSG_CTRLCNGE:
			if(!game_process_control_change(player, index, stream))
				return;
			break;
			
		case EMNETMSG_FIRERAIL:
			if(!game_process_fire_rail(player))
				return;
			break;
			
		case EMNETMSG_FIRELEFT:
			if(!game_process_fire_left(player, stream))
				return;
			break;
			
		case EMNETMSG_FIRERIGHT:
			if(!game_process_fire_right(player, stream))
				return;
			break;
			
		default:
			return;
		}
	}
}


void game_process_stream_ooo(uint32_t conn, uint32_t index, struct buffer_t *stream)
{
	update_game();
	
	struct player_t *player = get_player_by_conn(conn);
	assert(player);

	while(buffer_more(stream))
	{		
		switch(buffer_read_uint8(stream))
		{
		case EMNETMSG_CTRLCNGE:
			if(!game_process_control_change(player, index, stream))
				return;
			break;
			
		default:
			return;
		}
	}
}


void game_process_stream_timed(uint32_t conn, uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
	game_process_stream(conn, index, stream);
}


void game_process_stream_untimed(uint32_t conn, uint32_t index, struct buffer_t *stream)
{
	game_process_stream(conn, index, stream);
}


void game_process_stream_timed_ooo(uint32_t conn, uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
	game_process_stream_ooo(conn, index, stream);
}


void game_process_stream_untimed_ooo(uint32_t conn, uint32_t index, struct buffer_t *stream)
{
	game_process_stream_ooo(conn, index, stream);
}


int read_plasma_cannon(gzFile file)
{
	struct pickup_spawn_point_t pickup_spawn_point;
	
	pickup_spawn_point.type = OBJECTTYPE_PLASMACANNON;
	
	if(gzread(file, &pickup_spawn_point.x, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.y, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.plasma_cannon_data.plasmas, 4) != 4)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.plasma_cannon_data.angle, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.respawn_delay, 4) != 4)
		goto error;
	
	LL_ADD(struct pickup_spawn_point_t, &pickup_spawn_point0, &pickup_spawn_point);
	
	return 1;
	
error:
	
	return 0;
}


int read_minigun(gzFile file)
{
	struct pickup_spawn_point_t pickup_spawn_point;
	
	pickup_spawn_point.type = OBJECTTYPE_MINIGUN;
	
	if(gzread(file, &pickup_spawn_point.x, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.y, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.minigun_data.bullets, 4) != 4)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.minigun_data.angle, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.respawn_delay, 4) != 4)
		goto error;
	
	LL_ADD(struct pickup_spawn_point_t, &pickup_spawn_point0, &pickup_spawn_point);
	
	return 1;
	
error:
	
	return 0;
}


int read_rocket_launcher(gzFile file)
{
	struct pickup_spawn_point_t pickup_spawn_point;
	
	pickup_spawn_point.type = OBJECTTYPE_ROCKETLAUNCHER;
	
	if(gzread(file, &pickup_spawn_point.x, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.y, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.rocket_launcher_data.rockets, 4) != 4)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.rocket_launcher_data.angle, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.respawn_delay, 4) != 4)
		goto error;
	
	LL_ADD(struct pickup_spawn_point_t, &pickup_spawn_point0, &pickup_spawn_point);
	
	return 1;
	
error:
	
	return 0;
}


int read_rails(gzFile file)
{
	struct pickup_spawn_point_t pickup_spawn_point;
	
	pickup_spawn_point.type = OBJECTTYPE_RAILS;
	
	if(gzread(file, &pickup_spawn_point.x, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.y, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.rails_data.quantity, 4) != 4)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.rails_data.angle, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.respawn_delay, 4) != 4)
		goto error;
	
	LL_ADD(struct pickup_spawn_point_t, &pickup_spawn_point0, &pickup_spawn_point);
	
	return 1;
	
error:
	
	return 0;
}


int read_shield_energy(gzFile file)
{
	struct pickup_spawn_point_t pickup_spawn_point;
	
	pickup_spawn_point.type = OBJECTTYPE_SHIELDENERGY;
	
	if(gzread(file, &pickup_spawn_point.x, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.y, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.shield_energy_data.shield_energy, 4) != 4)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.shield_energy_data.angle, 8) != 8)
		goto error;
	
	if(gzread(file, &pickup_spawn_point.respawn_delay, 4) != 4)
		goto error;
	
	LL_ADD(struct pickup_spawn_point_t, &pickup_spawn_point0, &pickup_spawn_point);
	
	return 1;
	
error:
	
	return 0;
}


int read_object(gzFile file)
{
	uint8_t type;

	if(gzread(file, &type, 1) != 1)
		goto error;
	
	switch(type)
	{
		case OBJECTTYPE_PLASMACANNON:
			if(!read_plasma_cannon(file))
				goto error;
			break;
		
		case OBJECTTYPE_MINIGUN:
			if(!read_minigun(file))
				goto error;
			break;
		
		case OBJECTTYPE_ROCKETLAUNCHER:
			if(!read_rocket_launcher(file))
				goto error;
			break;
		
		case OBJECTTYPE_RAILS:
			if(!read_rails(file))
				goto error;
			break;
		
		case OBJECTTYPE_SHIELDENERGY:
			if(!read_shield_energy(file))
				goto error;
			break;
		
		case OBJECTTYPE_SPAWNPOINT:
			if(!read_spawn_point(file))
				goto error;
			break;
			
		case OBJECTTYPE_SPEEDUPRAMP:
			if(!read_speedup_ramp(file))
				goto error;
			break;
			
		case OBJECTTYPE_TELEPORTER:
			if(!read_teleporter(file))
				goto error;
			break;
			
		case OBJECTTYPE_GRAVITYWELL:
			if(!read_gravity_well(file))
				goto error;
			break;
	}

	return 1;
	
error:
	
	return 0;
}


void map(char *args)
{
	if(game_state == GS_ALIVE)
		return;
	
//	string_cat_text(map_filename, args);
	
	gzFile file = gzopen(args, "rb");
	if(!file)
	{
		console_print("Map not found.\n");
		return;
	}
	
//	uint16_t format_id;
	
//	if(gzread(file, &format_id, 2) != 2)
//		goto error;
	
	if(!load_bsp_tree(file))
		goto error;
	
	uint32_t num_objects;
	
	if(gzread(file, &num_objects, 4) != 4)
		goto error;
	
	int o;
	for(o = 0; o < num_objects; o++)
	{
		if(!read_object(file))
		goto error;
	}
	
	gzclose(file);
	
	
	// spawn objects
	
	struct pickup_spawn_point_t *pickup_spawn_point = pickup_spawn_point0;
		
	while(pickup_spawn_point)
	{
		struct entity_t *entity;
		
		switch(pickup_spawn_point->type)
		{
		case OBJECTTYPE_PLASMACANNON:
			entity = new_entity(&entity0);
			entity->type = ENT_WEAPON;
			entity->xdis = pickup_spawn_point->x;
			entity->ydis = pickup_spawn_point->y;
			entity->weapon_data.type = WEAPON_PLASMA_CANNON;
			entity->weapon_data.ammo = 400;
			entity->weapon_data.shield_strength = 1.0;
			break;
		
		case OBJECTTYPE_MINIGUN:
			entity = new_entity(&entity0);
			entity->type = ENT_WEAPON;
			entity->xdis = pickup_spawn_point->x;
			entity->ydis = pickup_spawn_point->y;
			entity->weapon_data.type = WEAPON_MINIGUN;
			entity->weapon_data.ammo = 400;
			entity->weapon_data.shield_strength = 1.0;
			break;
		
		case OBJECTTYPE_ROCKETLAUNCHER:
			entity = new_entity(&entity0);
			entity->type = ENT_WEAPON;
			entity->xdis = pickup_spawn_point->x;
			entity->ydis = pickup_spawn_point->y;
			entity->weapon_data.type = WEAPON_ROCKET_LAUNCHER;
			entity->weapon_data.ammo = 10;
			entity->weapon_data.shield_strength = 1.0;
			break;
		}
		
		pickup_spawn_point = pickup_spawn_point->next;
	}
	
	
	game_state = GS_ALIVE;
	console_print("Map loaded.\n");
	return;

error:
	
	gzclose(file);
	console_print("Map file is corrupt!\n");
}


void status()
{
	;
}

void leave()
{
	;
}



void init_game()
{
	create_cvar_string("password", "", 0);
	create_cvar_string("admin_password", "", 0);
	create_cvar_int("num_players", &num_players, CVAR_PROTECTED);
	create_cvar_int("num_players_active", &num_players_active, CVAR_PROTECTED);
	create_cvar_int("max_players", &max_players, 0);
	create_cvar_int("max_players_active", &max_players_active, 0);
	create_cvar_command("status", status);
	create_cvar_command("map", map);
	create_cvar_command("leave", leave);
	create_cvar_command("kick", kick);
	
	map_filename = new_string();
}


void kill_game()
{
	;
}

















/*
int process_enterrcon(struct player_t *player, struct buffer_t *buffer)
{
	if(player->in_rcon)
		return;

	player->in_rcon = 1;

//	net_write_int(EMNETMSG_INRCON);
//	net_finished_writing(player->conn);
}


int process_leavercon(struct player_t *player, struct buffer_t *buffer)
{
	if(!player->in_rcon)
		return;

	player->in_rcon = 0;

//	net_write_int(EMNETMSG_NOTINRCON);
//	net_finished_writing(player->conn);
}


int process_rconmsg(struct player_t *player, struct buffer_t *buffer)
{
	if(!player->in_rcon)
		return;

	console_rcon = player;

	struct string_t *string = buffer_read_string(buffer);

	parse_command(string->text);

	free(string);

	console_rcon = NULL;
}



void leave(char *args)
{
	if(console_rcon)
	{
//		net_write_int(EMNETMSG_NOTINRCON);
//		net_finished_writing(console_rcon->conn);
		console_rcon->in_rcon = 0;
	}
	else
	{
		console_print("you are not in rcon!\n");
	}
}



*/