#ifndef _INC_SGAME
#define _INC_SGAME


struct craft_data_t
{
	float acc;
	float theta, omega;

	struct entity_t *left_weapon, *right_weapon;
	float shield_flare;
	int carcass;
	
	#ifdef EMSERVER
	float shield_strength;
	#endif
	
	#ifdef EMCLIENT
	struct surface_t *surface;
	float particle;
	uint32_t last_tick;
	#endif
};


struct weapon_data_t
{
	int type;
	float theta;
	
	struct entity_t *craft;
	float shield_flare;
	
	#ifdef EMSERVER
	int ammo;
	float shield_strength;
	#endif
	
	#ifdef EMCLIENT
	struct surface_t *surface;
	#endif
};


struct plasma_data_t
{
	int in_weapon;
	uint32_t weapon_id;

	#ifdef EMSERVER
	struct player_t *owner;
	#endif
	
	#ifdef EMCLIENT
	struct surface_t *surface;
	#endif
};


struct rocket_data_t
{
	uint32_t start_tick;
	float theta;
	int in_weapon;
	uint32_t weapon_id;

	#ifdef EMSERVER
	struct player_t *owner;
	#endif
	
	#ifdef EMCLIENT
	struct surface_t *surface;
	float particle;
	uint32_t last_tick;
	#endif
};


struct bullet_data_t
{
	#ifdef EMSERVER
	struct player_t *owner;
	#endif
};


struct mine_data_t
{
	#ifdef EMSERVER
	struct player_t *owner;
	#endif
	
	#ifdef EMCLIENT
	struct surface_t *surface;
	#endif
};


struct rails_data_t
{
	#ifdef EMSERVER
//	int quantity;
	#endif
};


struct shield_data_t
{
	#ifdef EMSERVER
//	double strength;
	#endif
	
	#ifdef EMCLIENT
	struct surface_t *surface;
	#endif
};


struct entity_t
{
	uint32_t index;
	uint8_t type;
	uint8_t in_tick;
	uint8_t kill_me;
	
	float xdis, ydis;
	float xvel, yvel;
	
	union
	{
		struct craft_data_t craft_data;
		struct weapon_data_t weapon_data;
		struct plasma_data_t plasma_data;
		struct bullet_data_t bullet_data;
		struct rocket_data_t rocket_data;
		struct mine_data_t mine_data;
		struct rails_data_t rails_data;
		struct shield_data_t shield_data;
	};
	
	#ifdef EMSERVER
	uint8_t propagate_me;
	#endif
	
	struct entity_t *next;
};


#define ENT_CRAFT		0
#define ENT_WEAPON		1
#define ENT_BOGIE		2
#define ENT_BULLET		3
#define ENT_ROCKET		4
#define ENT_MINE		5
#define	ENT_RAILS		6
#define ENT_SHIELD		7

#define WEAPON_PLASMA_CANNON	0
#define WEAPON_MINIGUN			1
#define WEAPON_ROCKET_LAUNCHER	2


struct spawn_point_t
{
	double x, y;
	double angle;
	int teleport_only;
	int index;
	int respawn_index;
	
	struct spawn_point_t *next;
};


struct teleporter_t
{
	double x, y;
	double radius;
	uint16_t colour;
	int spawn_index;
	
	struct teleporter_t *next;
};


struct speedup_ramp_t
{
	double x, y;
	double theta;
	double width;
	double boost;
	
	struct speedup_ramp_t *next;
};


struct gravity_well_t
{
	double x, y;
	double strength;
	int confined;
	
	struct gravity_well_t *next;
};


extern struct spawn_point_t *spawn_point0;
extern struct teleporter_t *teleporter0;
extern struct speedup_ramp_t *speedup_ramp0;
extern struct gravity_well_t *gravity_well0;

struct entity_t *new_entity(struct entity_t **entity0);
struct entity_t *get_entity(struct entity_t *entity0, uint32_t index);
void remove_entity(struct entity_t **entity0, struct entity_t *entity);

#ifdef _ZLIB_H
int read_spawn_point(gzFile file);
int read_gravity_well(gzFile file);
int read_teleporter(gzFile file);
int read_speedup_ramp(gzFile file);
int read_shield_energy(gzFile file);
#endif

void s_tick_entities(struct entity_t **entity0);
void splash_force(double x, double y, double force);
void strip_weapons_from_craft(struct entity_t *craft);
void strip_craft_from_weapon(struct entity_t *weapon);


#define CRAFT_RADIUS	56.569
#define WEAPON_RADIUS	35.355
#define BOGIE_RADIUS	5.0
#define ROCKET_RADIUS	25.0
#define MINE_RADIUS		20.0
#define RAILS_RADIUS	20.0
#define SHIELD_RADIUS	20.0

#define CRAFT_MASS	100.0
#define WEAPON_MASS	75.0


#endif // _INC_SGAME
