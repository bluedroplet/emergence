#ifdef _INC_NFBUFFER
void game_process_stream_timed(uint32_t index, uint64_t *stamp, struct buffer_t *stream);
void game_process_stream_untimed(uint32_t index, struct buffer_t *stream);
void game_process_stream_timed_ooo(uint32_t index, uint64_t *stamp, struct buffer_t *stream);
void game_process_stream_untimed_ooo(uint32_t index, struct buffer_t *stream);
#endif

void game_resolution_change();


void game_process_conn_lost();
void game_process_connection();
void game_process_disconnection();
void render_game();
void init_game();
void kill_game();




void world_to_screen(double worldx, double worldy, int *screenx, int *screeny);
void screen_to_world(int screenx, int screeny, double *worldx, double *worldy);

void rev_thrust(int state);
void fire_bogie();
void fire_rail();
void fire_left();
void fire_right();
void drop_mine();

extern uint32_t cgame_tick;

#ifdef _INC_SGAME
void tick_craft(struct entity_t *craft, float xdis, float ydis);
void tick_rocket(struct entity_t *rocket, float xdis, float ydis);
void explosion(struct entity_t *entity);
#endif	

extern double viewx, viewy;

void update_game();


#define ROTATIONS 80
