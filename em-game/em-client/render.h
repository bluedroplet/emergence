void init_render_cvars();
void init_render();
void kill_render();
void init_fr();

void render_frame();

void start_rendering();
void stop_rendering();

extern float frame_time, last_frame_start_time;
extern uint32_t frame;

void screenshot(int state);

extern uint32_t vid_width, vid_height;

extern struct surface_t *s_backbuffer;
