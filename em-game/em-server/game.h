#include "ping.h"

#ifndef _INC_GAME
#define _INC_GAME


struct player_t
{
	uint32_t conn;
	int state;
	struct string_t *name;
	int in_rcon;

	uint32_t last_control_change;
	
	struct entity_t *craft;
	uint32_t respawn_tick;
	
	uint32_t next_pulse_event;
	
	int rails;
	int mines;
	
	uint32_t firing_left, firing_right;
	uint32_t firing_left_start, firing_right_start;
	uint32_t left_fired, right_fired;
	
	int frags;
	int propagate_frags;
	
	uint8_t rail_inner_red, rail_inner_green, rail_inner_blue;
	uint8_t rail_outer_red, rail_outer_green, rail_outer_blue;

	uint8_t plasma_red, plasma_green, plasma_blue;

	uint8_t magic_smoke;
	uint8_t smoke_start_red, smoke_start_green, smoke_start_blue;
	uint8_t smoke_end_red, smoke_end_green, smoke_end_blue;

	uint8_t shield_red, shield_green, shield_blue;

	struct ping_t *ping0;
	int next_ping;
	
	double latencies[11];
	int clatency;
	int nlatencies;

	double latency;
	
	struct player_t *next;
};


struct pickup_spawn_point_t
{
	uint8_t type;
	float x, y;
	
	int respawn;
	uint32_t respawn_tick;
	
	float angle;
	
	union
	{
		struct
		{
			int plasmas;
			
		} plasma_cannon_data;
		
		struct
		{
			int bullets;
			
		} minigun_data;
		
		struct
		{
			int rockets;
			
		} rocket_launcher_data;
		
		struct
		{
			int quantity;
			
		} rails_data;
		
		struct
		{
			int shield_energy;
			
		} shield_energy_data;
	};
	
	int respawn_delay;
	
	struct pickup_spawn_point_t *next;
};



#define GS_DEAD		0
#define GS_ALIVE	1

#define PLAYER_STATE_ASLEEP		0
#define PLAYER_STATE_PLAYING	1
#define PLAYER_STATE_SPECTATING	2

#define CRAFT_MAX_RAILS	20

void update_game();
void game_process_joined(uint32_t conn);
void game_process_disconnection(uint32_t conn);
void game_process_conn_lost(uint32_t conn);
void game_process_stream_timed(uint32_t conn, uint32_t index, uint64_t *stamp, struct buffer_t *stream);
void game_process_stream_untimed(uint32_t conn, uint32_t index, struct buffer_t *stream);
void game_process_stream_timed_ooo(uint32_t conn, uint32_t index, uint64_t *stamp, struct buffer_t *stream);
void game_process_stream_untimed_ooo(uint32_t conn, uint32_t index, struct buffer_t *stream);
void init_game();
void kill_game();

void print_on_all_players(const char *fmt, ...);
void remove_entity_from_all_players(struct entity_t *entity);
void respawn_craft(struct entity_t *craft, struct player_t *responsibility);
void make_carcass_on_all_players(struct entity_t *craft);
void schedule_respawn(struct pickup_spawn_point_t *spawn_point);
void craft_telefragged(struct player_t *victim, struct player_t *telefragger);
void propagate_colours(struct entity_t *entity);
void respawn_weapon(struct entity_t *weapon);

void emit_teleport_to_all_players();
void emit_speedup_to_all_players();
void emit_explosion(float xdis, float ydis, float xvel, float yvel, float size, uint8_t magic, 
	uint8_t start_red, uint8_t start_green, uint8_t start_blue,
	uint8_t end_red, uint8_t end_green, uint8_t end_blue);

extern uint32_t cgame_tick;


#endif // _INC_GAME
