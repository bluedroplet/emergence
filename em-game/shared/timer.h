uint32_t get_tick();
double get_double_time();
void init_timer();
void reset_start_count();
extern uint64_t start_count, counts_per_second;
void kill_timer();
int create_timer_listener();
void destroy_timer_listener(int read_fd);
