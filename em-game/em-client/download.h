int download_map(char *map_name);
void stop_downloading_map();
void init_download();
void kill_download();
void process_download_out_pipe();
extern int download_out_pipe[2];
