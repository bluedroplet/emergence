void add_game_tick(uint32_t game_tick, uint64_t *tsc);
void update_tick_parameters();
uint32_t get_game_tick();
double get_tsc_from_game_tick(double tick);
void init_tick_cvars();