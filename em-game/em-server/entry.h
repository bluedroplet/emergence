void go_daemon();
void terminate_process();
extern int as_daemon;
void mask_sigs();
void unmask_sigs();
extern struct string_t *username, *home_dir, *emergence_home_dir;
