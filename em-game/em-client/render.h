void init_render_cvars();
void init_render();
void kill_render();


void render_frame();

void start_rendering();
void stop_rendering();

extern double frame_time, last_frame_start_time;

void screenshot(int state);
