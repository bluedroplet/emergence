/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#define CVAR_TYPE		0x0f
#define CVAR_FLAGS		0xf0

#define CVAR_INT		0x00
#define CVAR_FLOAT		0x01
#define CVAR_STRING		0x02
#define CVAR_CMD		0x03
#define CVAR_UNDEF		0x0f

#define CVAR_PROTECTED	0x10

void create_cvar_int(char *name, int *addr, int flags);
void create_cvar_int_qc(char *name, int *addr, void (*qc_func)(int));
void create_cvar_float(char *name, float *addr, int flags);
void create_cvar_float_qc(char *name, float *addr, void (*qc_func)(float));
void create_cvar_string(char *name, char *string, int flags);
void create_cvar_string_qc(char *name, char *string, void (*qc_func)(char*));
void create_cvar_command(char *name, void (*cmd_func)(char*));
void set_int_cvar_qc_function(char *name, void (*qc_func)(int));
void set_int_cvar_qc_function_wv(char *name, void (*qc_func)(int*, int));
void set_float_cvar_qc_function(char *name, void (*qc_func)(float));
void set_string_cvar_qc_function(char *name, void (*qc_func)(char*));
void set_cvar_int(char *name, int val);
void set_cvar_float(char *name, float val);
void set_cvar_string(char *name, char *string);
void execute_cvar_command(char *name, char *param);
int is_cvar(char *name);
void complete_cvar(char *partial);
void list_cvars(char *partial);
int get_cvar_type(char *name);
int get_cvar_int(char *name);
float get_cvar_float(char *name);
char *get_cvar_string(char *name);
void clear_cvars();


#if defined _INC_STDIO || defined _STDIO_H
void dump_cvars(FILE *file);
#endif
