void process_keypress(int key, int state);
void process_button(int control, int state);
void process_axis(int control, float val);
void process_control_alarm();
void init_control();
void uifunc(int key, int state);

#if defined _INC_STDIO || defined _STDIO_H
void dump_bindings(FILE *file);
#endif
