void key_verify_session(uint32_t conn, char session_key[16]);
void init_key();
void kill_key();
int key_create_session();
void process_key_out_pipe();

extern int key_out_pipe[2];
