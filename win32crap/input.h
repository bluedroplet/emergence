void init_input();
void kill_input();

void keyboard_callback();

extern HANDLE hKeyboardEvent;
void get_buffered_keypress(DWORD *key, int *state);

