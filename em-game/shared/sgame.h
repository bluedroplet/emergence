#ifndef _INC_SGAME
#define _INC_SGAME


// server -> client

#define EMMSGCLASS_MASK		(0x3 << 6)

#define EMMSGCLASS_STND			(0x0 << 6)
#define EMMSGCLASS_NETONLY		(0x1 << 6)		// not written to demo files
#define EMMSGCLASS_EVENT		(0x2 << 6)

#define EMMSG_PROTO_VER			(EMMSGCLASS_STND | 0x00)


/*

changes above this line and in the network code must not break backward
compatibility of the protocol version detection mechanism

-------------------------------------------------------------------------------

if changes below this line or in the game code (either server or client side)
break backward compatibility, EM_PROTO_VER must be incremented accordingly

*/


#define EM_PROTO_VER									0x01



// server -> client

#define EMMSG_LOADMAP			(EMMSGCLASS_STND | 0x01)
#define EMMSG_LOADSKIN			(EMMSGCLASS_STND | 0x02)

#define EMNETMSG_PRINT			(EMMSGCLASS_NETONLY | 0x00)
#define EMNETMSG_PLAYING		(EMMSGCLASS_NETONLY | 0x01)
#define EMNETMSG_SPECTATING		(EMMSGCLASS_NETONLY | 0x02)
#define EMNETMSG_INRCON			(EMMSGCLASS_NETONLY | 0x03)
#define EMNETMSG_NOTINRCON		(EMMSGCLASS_NETONLY | 0x04)
#define EMNETMSG_JOINED			(EMMSGCLASS_NETONLY | 0x05)

#define EMEVENT_PULSE			(EMMSGCLASS_EVENT | 0x00)
#define EMEVENT_PRINT			(EMMSGCLASS_EVENT | 0x01)
#define EMEVENT_SPAWN_ENT		(EMMSGCLASS_EVENT | 0x02)
#define EMEVENT_UPDATE_ENT		(EMMSGCLASS_EVENT | 0x03)
#define EMEVENT_KILL_ENT		(EMMSGCLASS_EVENT | 0x04)
#define EMEVENT_FOLLOW_ME		(EMMSGCLASS_EVENT | 0x05)
#define EMEVENT_CARCASS			(EMMSGCLASS_EVENT | 0x06)
#define EMEVENT_RAILTRAIL		(EMMSGCLASS_EVENT | 0x07)
#define EMEVENT_DETACH			(EMMSGCLASS_EVENT | 0x08)



// client -> server

#define EMMSG_JOIN				0x00
#define EMMSG_PLAY				0x01
#define EMMSG_SPECTATE			0x02
#define EMMSG_SAY				0x03

#define EMMSG_THRUST			0x04
#define EMMSG_BRAKE				0x05
#define EMMSG_NOBRAKE			0x06
#define EMMSG_ROLL				0x07
#define EMMSG_FIRERAIL			0x08
#define EMMSG_FIRELEFT			0x09
#define EMMSG_FIRERIGHT			0x0a
#define EMMSG_DROPMINE			0x0b
#define EMMSG_ENTERRCON			0x0c
#define EMMSG_LEAVERCON			0x0d
#define EMMSG_RCONMSG			0x0e
#define EMMSG_STATUS			0x0f
#define EMMSG_NAMECNGE			0x10
#define EMMSG_SUICIDE			0x11




struct craft_data_t
{
	float acc;
	float theta;
	int braking;

	struct entity_t *left_weapon, *right_weapon;
	float shield_flare;
	int carcass;
	
	#ifdef EMSERVER
	float shield_strength;
	#endif
	
	#ifdef EMCLIENT
	float old_theta;
	int skin;
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
	int detached;
	
	#ifdef EMSERVER
	int ammo;
	float shield_strength;
	struct pickup_spawn_point_t *spawn_point;
	#endif
	
	#ifdef EMCLIENT
	int skin;
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
	int skin;
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
	int skin;
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
	int skin;
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
	int skin;
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
	
	int teleporting;
	uint32_t teleporting_tick;
	uint32_t teleport_spawn_index;
	
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

#define NO_ENT_INDEX	-1

#define ENT_CRAFT		0
#define ENT_WEAPON		1
#define ENT_PLASMA		2
#define ENT_BULLET		3
#define ENT_ROCKET		4
#define ENT_MINE		5
#define	ENT_RAILS		6
#define ENT_SHIELD		7

#define WEAPON_PLASMA_CANNON	0
#define WEAPON_MINIGUN			1
#define WEAPON_ROCKET_LAUNCHER	2

#define TELEPORT_FADE_TIME		0.125
#define TELEPORT_TRAVEL_TIME	1.0

#define TELEPORTING_FINISHED		0
#define TELEPORTING_DISAPPEARING	1
#define TELEPORTING_TRAVELLING		2
#define TELEPORTING_APPEARING		3

#define PLASMA_DAMAGE	0.1
#define BULLET_DAMAGE	0.05
#define RAIL_DAMAGE		0.5

struct spawn_point_t
{
	double x, y;
	double angle;
	int teleport_only;
	int index;
	int respawn_index;
	
	struct spawn_point_t *next;
};

#if defined (EMSERVER) || defined (_INC_PARTICLES)

struct teleporter_t
{
	double x, y;
	double radius;
	uint16_t colour;
	int sparkles;
	int spawn_index;
	
	int rotation_type;
	double rotation_angle;
	int divider;
	double divider_angle;
	
	#ifdef EMCLIENT
	struct particle_t particles[1000];
	float particle_power;
	int next_particle;
	#endif
	
	struct teleporter_t *next;
};

#endif	// _INC_PARTICLES


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
void explode_craft(struct entity_t *craft);
void get_spawn_point_coords(uint32_t index, float *x, float *y);
int line_in_circle(double lx1, double ly1, double lx2, double ly2, 
	double cx, double cy, double cr);
int line_in_circle_with_coords(double lx1, double ly1, double lx2, double ly2, 
	double cx, double cy, double cr, float *out_x, float *out_y);

int craft_force(struct entity_t *craft, double force);
int weapon_force(struct entity_t *weapon, double force);
int rocket_force(struct entity_t *rocket, double force);
int mine_force(struct entity_t *mine, double force);
int rails_force(struct entity_t *rails, double force);
int shield_force(struct entity_t *shield, double force);



#define CRAFT_RADIUS	56.569
#define WEAPON_RADIUS	35.355
#define PLASMA_RADIUS	5.0
#define ROCKET_RADIUS	25.0
#define MINE_RADIUS		20.0
#define RAILS_RADIUS	20.0
#define SHIELD_RADIUS	20.0

#define CRAFT_MASS	100.0
#define WEAPON_MASS	75.0


#endif // _INC_SGAME
