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
};



#define GS_DEAD		0
#define GS_ALIVE	1

#define PLAYER_STATE_ACTIVE		0
#define PLAYER_STATE_SPECTATING	1


void update_game();
void game_process_join(uint32_t conn, uint32_t index, struct buffer_t *stream);
void game_process_disconnection(uint32_t conn);
void game_process_conn_lost(uint32_t conn);
void game_process_stream_timed(uint32_t conn, uint32_t index, uint64_t *stamp, struct buffer_t *stream);
void game_process_stream_untimed(uint32_t conn, uint32_t index, struct buffer_t *stream);
void game_process_stream_timed_ooo(uint32_t conn, uint32_t index, uint64_t *stamp, struct buffer_t *stream);
void game_process_stream_untimed_ooo(uint32_t conn, uint32_t index, struct buffer_t *stream);
void init_game();
void kill_game();

void print_on_all_players(char *text);
void remove_entity_from_all_players(struct entity_t *entity);
void respawn_craft(struct entity_t *craft);
void make_carcass_on_all_players(struct entity_t *craft);
void calculate_respawn_tick(struct pickup_spawn_point_t *spawn_point);


extern uint32_t cgame_tick;


#endif // _INC_GAME