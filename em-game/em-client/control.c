#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/epoll.h>

#include "../common/types.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../shared/cvar.h"
#include "../shared/network.h"
#include "../shared/timer.h"
#include "../shared/sgame.h"
#include "main.h"
#include "console.h"
#include "game.h"
#include "control.h"
#include "entry.h"
#include "render.h"
#include "input.h"


int control_timer_fd;
int control_kill_pipe[2];

pthread_mutex_t control_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_t control_thread_id;

struct
{
	char *name;
	void *func;
	void *uifunc;

} controls[] = 
{
	{"",				NULL,	NULL},
	{"escape",		NULL,	NULL},
	{"1",			NULL,	NULL},
	{"2",			NULL,	NULL},
	{"3",			NULL,	NULL},
	{"4",			NULL,	NULL},
	{"5",			NULL,	NULL},
	{"6",			NULL,	NULL},
	{"7",			NULL,	NULL},
	{"8",			NULL,	NULL},
	{"9",			NULL,	NULL},
	{"0",			NULL,	NULL},
	{"-",			NULL,	NULL},
	{"=",			NULL,	NULL},
	{"backspace",	NULL,	console_backspace},
	{"tab",			NULL,	console_tab},
	{"q",			NULL,	NULL},
	{"w",			NULL,	NULL},
	{"e",			NULL,	NULL},
	{"r",			NULL,	NULL},
	{"t",			NULL,	NULL},
	{"y",			NULL,	NULL},
	{"u",			NULL,	NULL},
	{"i",			NULL,	NULL},
	{"o",			NULL,	NULL},
	{"p",			NULL,	NULL},
	{"[",			NULL,	NULL},
	{"]",			NULL,	NULL},
	{"enter",		NULL,	console_enter},
	{"lctrl",		NULL,	NULL},
	{"a",			NULL,	NULL},
	{"s",			NULL,	NULL},//screenshot},
	{"d",			NULL,	NULL},
	{"f",			NULL,	NULL},
	{"g",			NULL,	NULL},
	{"h",			NULL,	NULL},
	{"j",			NULL,	NULL},
	{"k",			NULL,	NULL},
	{"l",			NULL,	NULL},
	{";",			NULL,	NULL},
	{"'",			NULL,	NULL},
	{"`",			NULL,	console_toggle},
	{"lshift",		NULL,	NULL},
	{"\\",			NULL,	NULL},
	{"z",			NULL,	NULL},
	{"x",			NULL,	NULL},
	{"c",			NULL,	NULL},
	{"v",			NULL,	NULL},
	{"b",			NULL,	NULL},
	{"n",			NULL,	NULL},
	{"m",			NULL,	NULL},
	{",",			NULL,	NULL},
	{".",			NULL,	NULL},
	{"/",			NULL,	NULL},
	{"rshift",		NULL,	NULL},
	{"pad*",			NULL,	NULL},
	{"lalt",			NULL,	NULL},
	{"space",		NULL,	NULL},
	{"capslock",		NULL,	NULL},
	{"f1",			NULL,	NULL},
	{"f2",			NULL,	NULL},
	{"f3",			NULL,	NULL},
	{"f4",			NULL,	NULL},
	{"f5",			NULL,	NULL},
	{"f6",			NULL,	NULL},
	{"f7",			NULL,	NULL},
	{"f8",			NULL,	NULL},
	{"f9",			NULL,	NULL},
	{"f10",			NULL,	NULL},
	{"numlock",		NULL,	NULL},
	{"scroll",		NULL,	NULL},
	{"pad7",			NULL,	NULL},
	{"pad8",			NULL,	prev_command},
	{"pad9",			NULL,	NULL},
	{"pad-",			NULL,	NULL},
	{"pad4",			NULL,	NULL},
	{"pad5",			NULL,	NULL},
	{"pad6",			NULL,	NULL},
	{"pad+",			NULL,	NULL},
	{"pad1",			NULL,	NULL},
	{"pad2",			NULL,	next_command},
	{"pad3",			NULL,	NULL},
	{"pad0",			NULL,	NULL},
	{"pad.",			NULL,	NULL},
	{"102",			NULL,	NULL},
	{"f11",			NULL,	NULL},
	{"f12",			NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"padenter",		NULL,	console_enter},
	{"rctrl",		NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"pad,",			NULL,	NULL},
	{"",				NULL,	NULL},
	{"pad/",			NULL,	NULL},
	{"",				NULL,	NULL},
	{"sysrq",		NULL,	NULL},
	{"ralt",			NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"pause",		NULL,	NULL},
	{"",				NULL,	NULL},
	{"home",			NULL,	NULL},
	{"up",			NULL,	prev_command},
	{"pgup",			NULL,	NULL},
	{"",				NULL,	NULL},
	{"left",			NULL,	NULL},
	{"",				NULL,	NULL},
	{"right",		NULL,	NULL},
	{"",				NULL,	NULL},
	{"end",			NULL,	NULL},
	{"down",			NULL,	next_command},
	{"pgdn",			NULL,	NULL},
	{"insert",		NULL,	NULL},
	{"delete",		NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"",				NULL,	NULL},
	{"lwin",			NULL,	NULL},
	{"rwin",			NULL,	NULL},
	{"apps",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"",			NULL,	NULL},
	{"BTN_0",				NULL,	NULL},
	{"BTN_1",				NULL,	NULL},//screenshot},
	{"BTN_2",				NULL,	NULL},
	{"BTN_3",				NULL,	NULL},
	{"BTN_4",				NULL,	NULL},
	{"BTN_5",				NULL,	NULL},
	{"BTN_6",				NULL,	NULL},
	{"BTN_7",				NULL,	NULL},
	{"BTN_8",				NULL,	NULL},
	{"BTN_9",				NULL,	NULL},
	{"BTN_10",				NULL,	NULL},
	{"BTN_11",				NULL,	NULL},
	{"BTN_12",				NULL,	NULL},
	{"BTN_13",				NULL,	NULL},
	{"BTN_14",				NULL,	NULL},
	{"BTN_15",				NULL,	NULL},
	{"BTN_16",				NULL,	NULL},
	{"BTN_17",				NULL,	NULL},
	{"BTN_18",				NULL,	NULL},
	{"BTN_19",				NULL,	NULL},
	{"BTN_20",				NULL,	NULL},
	{"BTN_21",				NULL,	NULL},
	{"BTN_22",				NULL,	NULL},
	{"BTN_23",				NULL,	NULL},
	{"BTN_24",				NULL,	NULL},
	{"BTN_25",				NULL,	NULL},
	{"BTN_26",				NULL,	NULL},
	{"BTN_27",				NULL,	NULL},
	{"BTN_28",				NULL,	NULL},
	{"BTN_29",				NULL,	NULL},
	{"BTN_30",				NULL,	NULL},
	{"BTN_31",				NULL,	NULL},
	{"AXIS_0",				NULL,	NULL},
	{"AXIS_1",				NULL,	NULL},
	{"AXIS_2",				NULL,	NULL},
	{"AXIS_3",				NULL,	NULL},
	{"AXIS_4",				NULL,	NULL},
	{"AXIS_5",				NULL,	NULL},
	{"AXIS_6",				NULL,	NULL},
	{"AXIS_7",				NULL,	NULL},
	{"AXIS_8",				NULL,	NULL},
	{"AXIS_9",				NULL,	NULL},
	{"AXIS_10",				NULL,	NULL},
	{"AXIS_11",				NULL,	NULL},
	{"AXIS_12",				NULL,	NULL},
	{"AXIS_13",				NULL,	NULL},
	{"AXIS_14",				NULL,	NULL},
	{"AXIS_15",				NULL,	NULL},
	{"AXIS_16",				NULL,	NULL},
	{"AXIS_17",				NULL,	NULL},
	{"AXIS_18",				NULL,	NULL},
	{"AXIS_19",				NULL,	NULL},
	{"AXIS_20",				NULL,	NULL},
	{"AXIS_21",				NULL,	NULL},
	{"AXIS_22",				NULL,	NULL},
	{"AXIS_23",				NULL,	NULL},
	{"AXIS_24",				NULL,	NULL},
	{"AXIS_25",				NULL,	NULL},
	{"AXIS_26",				NULL,	NULL},
	{"AXIS_27",				NULL,	NULL},
	{"AXIS_28",				NULL,	NULL},
	{"AXIS_29",				NULL,	NULL},
	{"AXIS_30",				NULL,	NULL},
	{"AXIS_31",				NULL,	NULL}
};

float thrust;
float roll;
int control_changed;

void action_thrust(float val)
{
	thrust += val / 2.0;
	control_changed = 1;
}


void action_roll(float val)
{
	roll += val;
	control_changed = 1;
}

void thrust_bool(uint32_t state)
{
	if(state)
		thrust = 10000.0f;
	else
		thrust = 0.0f;
	
	control_changed = 1;
}





void fire_rail(uint32_t state)
{
//	if(game_state != GAMESTATE_ALIVE)
//		return;
	
	if(!state)
		return;

	net_emit_uint8(game_conn, EMMSG_FIRERAIL);
	net_emit_end_of_stream(game_conn);
}


void fire_left(uint32_t state)
{
//	if(game_state != GAMESTATE_ALIVE)
//		return;

	net_emit_uint8(game_conn, EMMSG_FIRELEFT);
	net_emit_uint32(game_conn, state);
	net_emit_end_of_stream(game_conn);
}


void fire_right(uint32_t state)
{
//	if(game_state != GAMESTATE_ALIVE)
//		return;

	net_emit_uint8(game_conn, EMMSG_FIRERIGHT);
	net_emit_uint32(game_conn, state);
	net_emit_end_of_stream(game_conn);
}


void drop_mine(uint32_t state)
{
//	if(game_state != GAMESTATE_ALIVE)
		return;

/*	net_write_dword(EMNETMSG_DROPMINE);
	net_write_dword(state);
	finished_writing();
	*/
}





#define ACTIONTYPE_BOOL 0
#define ACTIONTYPE_CONT	1

struct
{
	char *name;
	void *func;
	int type;

} actions[] = 
{
	{"THRUST",				action_thrust,	ACTIONTYPE_CONT}, 
	{"ROLL",				action_roll,	ACTIONTYPE_CONT}, 
	{"RAIL",				fire_rail,		ACTIONTYPE_BOOL}, 
	{"LEFT",				fire_left,		ACTIONTYPE_BOOL}, 
	{"RIGHT",				fire_right,		ACTIONTYPE_BOOL}, 
	{"DropMine",			drop_mine,		ACTIONTYPE_BOOL}, 
	{"THRUSTBOOL",			thrust_bool,	ACTIONTYPE_BOOL}, 
//	{"RollLeftBool",		roll_left,		ACTIONTYPE_BOOL}, 
//	{"RollRightBool",		roll_right,		ACTIONTYPE_BOOL}, 
};


int numactions = 7; //sizeof(controls) / sizeof(controls[0]);

struct
{
	int set;
	float min, max;
} axis_calib[32];

#define CONTROL_TICK_INTERVAL 0.05


double next_control_tick;

void process_control_alarm()
{
	char c;
	while(read(control_timer_fd, &c, 1) != -1);
		
	double time = get_double_time();
		
	if(time > next_control_tick)
	{
		pthread_mutex_lock(&control_mutex);
		
		if(control_changed)
		{
			net_emit_uint8(game_conn, EMMSG_CTRLCNGE);
			net_emit_float(game_conn, thrust);
			net_emit_float(game_conn, roll);
			net_emit_end_of_stream(game_conn);
			control_changed = 0;
			roll = 0.0;
			thrust = 0.0;
		}
	
		pthread_mutex_unlock(&control_mutex);
		
		next_control_tick = ((int)(time / CONTROL_TICK_INTERVAL) + 1) * 
			(double)CONTROL_TICK_INTERVAL;
	}
}


int shift = 0;


char get_ascii(int key)
{
	switch(key)
	{
	case 57:
		return ' ';

	case 11:
		return '0';
	
	case 2:
		return '1';
	
	case 3:
		return '2';
	
	case 4:
		return '3';
	
	case 5:
		return '4';
	
	case 6:
		return '5';
	
	case 7:
		return '6';
	
	case 8:
		return '7';
	
	case 9:
		return '8';
	
	case 10:
		return '9';

	case 52:
		return '.';

	case 0x0c:
		if(shift)
			return '_';
		else
			return '-';

	case 0x1e:
		return 'a' - shift;

	case 0x30:
		return 'b' - shift;

	case 0x2e:
		return 'c' - shift;

	case 0x20:
		return 'd' - shift;

	case 0x12:
		return 'e' - shift;

	case 0x21:
		return 'f' - shift;

	case 0x22:
		return 'g' - shift;

	case 0x23:
		return 'h' - shift;

	case 0x17:
		return 'i' - shift;

	case 0x24:
		return 'j' - shift;

	case 0x25:
		return 'k' - shift;

	case 0x26:
		return 'l' - shift;

	case 0x32:
		return 'm' - shift;

	case 0x31:
		return 'n' - shift;

	case 0x18:
		return 'o' - shift;

	case 0x19:
		return 'p' - shift;

	case 0x10:
		return 'q' - shift;

	case 0x13:
		return 'r' - shift;

	case 0x1f:
		return 's' - shift;

	case 0x14:
		return 't' - shift;

	case 0x16:
		return 'u' - shift;

	case 0x2f:
		return 'v' - shift;

	case 0x11:
		return 'w' - shift;

	case 0x2d:
		return 'x' - shift;

	case 0x15:
		return 'y' - shift;

	case 0x2c:
		return 'z' - shift;

	default:
		return '\0';
	}
}


void process_keypress(int key, int state)
{
	pthread_mutex_lock(&control_mutex);		// called from x thread
	
	if(key >= 256)
		goto end;
	
	void (*func)(int);

	func = controls[key].uifunc;
	if(func)
		func(state);
	
	func = controls[key].func;
	if(func)
		func(state);
	
	if(!state)
		goto end;

	if(key == 42 || key == 54)
	{
		if(state)
			shift = 0x20;
		else
			shift = 0x0;
	}
	else
	{
		if(state)
		{
			char c = get_ascii(key);

			if(c)
			{
				console_keypress(c);
			}
		}
	}

	end:
	pthread_mutex_unlock(&control_mutex);
}


void process_button(int control, int state)
{
	if(control >= 32)
		return;

	control += 256;
	
	void (*func)(int);

	func = controls[control].uifunc;

	if(func)
		func(state);

	func = controls[control].func;

	if(func)
		func(state);
}


void process_axis(int axis, float val)
{
	if(axis >= 32)
		return;

/*	if(!axis_calib[axis].set)
	{
		axis_calib[axis].set = 1;
		axis_calib[axis].min = val;
		axis_calib[axis].max = val;
	}
	else
	{
		if(val < axis_calib[axis].min)
			axis_calib[axis].min = val;
		
		if(val > axis_calib[axis].max)
			axis_calib[axis].max = val;
	}
	
	if(val > 0.0)
	{
		val /= axis_calib[axis].max;
		
		if(val > 0.9f)
			val = 1.0f;
	}
	else if(val < 0.0)
	{
		val /= -axis_calib[axis].min;
		
		if(val < -0.9f)
			val = -1.0f;
	}
	
	if(val > -0.1f && val < 0.1f)
		val = 0.0f;
	*/
//	printf("%f\n", val);
	
	void (*func)(float);

	func = controls[axis + 288].func;
	
//	printf("%f\n", val);

	if(func)
		func(val);
}


void cf_controls(char *params)
{
	pthread_mutex_lock(&control_mutex);

	int c;

	for(c = 0; c < 320; c++)
	{
		if(controls[c].name[0] != '\0')
		{
			console_print(controls[c].name);
			console_print("\n");
		}
	}

	pthread_mutex_unlock(&control_mutex);
}


void cf_actions(char *params)
{
	pthread_mutex_lock(&control_mutex);

	int a;

	for(a = 0; a < numactions; a++)
	{
		if(actions[a].name[0] != '\0')
		{
			console_print(actions[a].name);
			console_print("\n");
		}
	}

	pthread_mutex_unlock(&control_mutex);
}	


void cf_bind(char *params)
{
	pthread_mutex_lock(&control_mutex);
	
	char *token = strtok(params, " ");
	
	if(!token)
	{
		console_print("usage: bind <control> <action>\n");
		goto end;
	}

	int c;

	for(c = 0; c < 320; c++)
	{
		if(strcmp(controls[c].name, token) == 0)
			break;
	}

	if(c == 320)
	{
		struct string_t *s = new_string_text("control \"");
		string_cat_text(s, token);
		string_cat_text(s, "\" not found\n");
		
		console_print(s->text);
		
		free_string(s);
		
		goto end;
	}

	token = strtok(NULL, " ");

	if(!token)
	{
		controls[c].func = NULL;
		goto end;
	}

	int a;

	for(a = 0; a < numactions; a++)
	{
		if(strcmp(actions[a].name, token) == 0)
			break;
	}

	if(a == numactions)
	{
		struct string_t *s = new_string_text("action \"");
		string_cat_text(s, token);
		string_cat_text(s, "\" not found\n");
		
		console_print(s->text);
		
		free_string(s);
		
		goto end;
	}
	
	if(c < 288 && actions[a].type != ACTIONTYPE_BOOL)
	{
		console_print("Boolean controls must be bound to boolean actions.\n");
		goto end;
	}

	if(c >= 288 && actions[a].type != ACTIONTYPE_CONT)
	{
		console_print("Continuous controls must be bound to continuous actions.\n");
		goto end;
	}

	controls[c].func = actions[a].func;
	
	end:
	pthread_mutex_unlock(&control_mutex);
}


void dump_bindings(FILE *file)
{
	pthread_mutex_lock(&control_mutex);
	
	int c, a;

	for(c = 0; c < 320; c++)
	{
		if(controls[c].func)
		{
			for(a = 0; a < numactions; a++)
			{
				if(actions[a].func == controls[c].func)
					break;
			}
			
			if(a == numactions)
				assert(0);
			
			struct string_t *s = new_string_text("bind ");
			string_cat_text(s, controls[c].name);
			string_cat_char(s, ' ');
			string_cat_text(s, actions[a].name);
			string_cat_char(s, '\n');
			
			fputs(s->text, file);
			
			free_string(s);
		}
	}

	pthread_mutex_unlock(&control_mutex);
}


void *control_thread(void *a)
{
	int epoll_fd = epoll_create(3);
	
	struct epoll_event ev;
	
	ev.events = EPOLLIN | EPOLLET;
	ev.data.u32 = 0;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fd, &ev) == -1)
		printf("err\n");

	ev.events = EPOLLIN | EPOLLET;
	ev.data.u32 = 1;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, control_timer_fd, &ev) == -1)
		printf("err2\n");

	ev.events = EPOLLIN | EPOLLET;
	ev.data.u32 = 2;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, control_kill_pipe[0], &ev);

	while(1)
	{
		epoll_wait(epoll_fd, &ev, 1, -1);
		
		switch(ev.data.u32)
		{
		case 0:
			printf("a\n");
			pthread_mutex_lock(&control_mutex);
			process_input();
			pthread_mutex_unlock(&control_mutex);
			break;
		
		case 1:
			process_input();
			process_control_alarm();
			break;
		
		case 2:
			pthread_exit(NULL);
			break;
		}
	}
}


void init_control()
{
	double time = get_double_time();
	next_control_tick = ((int)(time / CONTROL_TICK_INTERVAL) + 1) * (double)CONTROL_TICK_INTERVAL;
	
	create_cvar_command("bind", cf_bind);
	create_cvar_command("controls", cf_controls);
	create_cvar_command("actions", cf_actions);
	
	int a;
	for(a = 0; a < 32; a++)
	{
		axis_calib[a].set = 0;
	}
	
	control_timer_fd = create_timer_listener();
	pipe(control_kill_pipe);
	pthread_create(&control_thread_id, NULL, control_thread, NULL);
}


void kill_control()
{
	char c;
	write(control_kill_pipe[1], &c, 1);
	pthread_join(control_thread_id, NULL);
}
