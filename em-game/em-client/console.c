#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "../common/types.h"
#include "../shared/cvar.h"
#include "../common/stringbuf.h"
#include "../shared/parse.h"
#include "../gsub/gsub.h"
#include "rcon.h"
#include "render.h"
#include "entry.h"

#define CONSOLE_LL_GRAN 200
#define CONSOLE_INPUT_LENGTH 40

int console_inputting = 0;
char *console_input = NULL;

int newline = 1;

int tabbed = 0;

char *old_console_input = NULL;



int r_ConsoleHeight = 8;
int r_ConsoleRed = 0x56;
int r_ConsoleGreen = 0xef;
int r_ConsoleBlue = 0xf0;
int r_ConsoleColour = 0x7f7f;
int r_ConsoleAlpha = 0x7f;
int r_RconConsoleRed = 0xf6;
int r_RconConsoleGreen = 0x2f;
int r_RconConsoleBlue = 0x20;
int r_RconConsoleColour = 0xafaf;
int r_ConsoleTextRed = 0;
int r_ConsoleTextGreen = 0x7f;
int r_ConsoleTextBlue = 0xff;
int r_ConsoleTextColour = 0xf7f3;
int r_ConsoleActiveTextRed = 0xff;
int r_ConsoleActiveTextGreen = 0xff;
int r_ConsoleActiveTextBlue = 0xff;
int r_ConsoleActiveTextColour = 0xffff;
int r_DrawConsole = 0;


struct consolell_t
{
	int numlines;
	char *line[CONSOLE_LL_GRAN];

	struct consolell_t *prev, *next;

} *console0 = NULL, *scrolling_console = NULL;

int scrolling = 0;
int firstconsoleline = 0;


struct cmdhistll_t
{
	char *cmd;

	struct cmdhistll_t *next, *prev;
} *cmdhist0 = NULL, *ccmdhist = NULL;


void render_console()
{
	if(!r_DrawConsole)
		return;

	int consoleheight = r_ConsoleHeight;

	if(console0)
	{
		int consoleline = consoleheight - 1;

		if(console_inputting)
		{
			blit_text(1, (vid_height - consoleheight * 14) + consoleline * 14 + 1, r_ConsoleActiveTextColour, console_input);
			consoleline--;
		}

		struct consolell_t *console;
		int cline;

		if(scrolling)
		{
			console = scrolling_console;

		}
		else
		{
			console = console0;
		}
		

		while(consoleline >= 0)
		{
			int stop = 0;

			for(cline = console->numlines - 1; cline >= 0; cline--)
			{
				blit_text(1, (vid_height - consoleheight * 14) + consoleline * 14 + 1, r_ConsoleTextColour, console->line[cline]);
				if(consoleline-- == 0)
				{
					stop = 1;
					break;
				}
			}

			if(stop)
				break;

			if(console->prev)
				console = console->prev;
			else
				break;
		}
	}

	blit_destx = 0;
	blit_desty = vid_height - consoleheight * 14;
	blit_width = vid_width / 2;
	blit_height = consoleheight * 14;
	blit_alpha = r_ConsoleAlpha;
	
	if(rconing == RCCON_IN)
		blit_colour = r_RconConsoleColour;
	else
		blit_colour = r_ConsoleColour;

	draw_alpha_rect();
}


void add_cmd_hist(char *cmd)
{
	struct cmdhistll_t *temp = cmdhist0;

	cmdhist0 = malloc(sizeof(struct cmdhistll_t));
	cmdhist0->cmd = malloc(strlen(cmd) + 1);
	strcpy(cmdhist0->cmd, cmd);
	cmdhist0->prev = temp;
	cmdhist0->next = NULL;

	if(temp)
		temp->next = cmdhist0;

	ccmdhist = NULL;
}


void console_print(const char *fmt, ...)
{
	char *msg;
	
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&msg, fmt, ap);
	va_end(ap);

	printf(msg);
	fflush(stdout);
	
	char *temp = msg;
	
	while(1)
	{
		if(newline)
		{
			if(!console0)
			{
				console0 = malloc(sizeof(struct consolell_t));
				console0->numlines = 1;
				console0->prev = NULL;
				console0->next = NULL;
			}
			else
			{
				if(console0->numlines == CONSOLE_LL_GRAN)
				{
					struct consolell_t *temp = console0;
					console0 = malloc(sizeof(struct consolell_t));
					console0->numlines = 1;
					console0->prev = temp;
					console0->prev->next = console0;
					console0->next = NULL;
				}
				else
					console0->numlines++;
			}
		}


		int l = 0;
		char *cc = msg;
		
		while(*cc && *cc != '\n')
			l++, cc++;


		if(!l && newline)
		{
			console0->line[console0->numlines - 1] = malloc(1);
			*console0->line[console0->numlines - 1] = '\0';
		}

		
		if(l)
		{
			if(newline)
			{
				console0->line[console0->numlines - 1] = malloc(l + 1);
				strncpy(console0->line[console0->numlines - 1], msg, l);
				console0->line[console0->numlines - 1][l] = '\0';
			}
			else
			{
				char *temp = console0->line[console0->numlines - 1];

				console0->line[console0->numlines - 1] = malloc(strlen(temp) + l + 1);

				strcpy(console0->line[console0->numlines - 1], temp);

				free(temp);

				strncat(console0->line[console0->numlines - 1], msg, l);
			}
		}


		newline = 0;

		if(*cc == '\n')
		{
			newline = 1;
			cc++;
		}

		if(*cc == '\0')
			break;

		msg = cc;
	}
	
	free(temp);
}


void console_toggle(int state)
{
	if(!state)
		return;

	r_DrawConsole = !r_DrawConsole;

	if(r_DrawConsole)
	{
		console_inputting = 0;
		console_input[0] = '\0';
	}
}


void prev_command(int state)
{
	if(!state)
		return;

	if(!r_DrawConsole)
		return;

	if(!ccmdhist)
	{
		ccmdhist = cmdhist0;

		if(ccmdhist)
		{
			strcpy(console_input, ccmdhist->cmd);

			console_inputting = 1;
		}
	}
	else
	{
		ccmdhist = ccmdhist->prev;

		if(!ccmdhist)
			ccmdhist = cmdhist0;

		strcpy(console_input, ccmdhist->cmd);

		console_inputting = 1;
	}
}


void next_command(int state)
{
	if(!state)
		return;

	if(!r_DrawConsole)
		return;

	if(!ccmdhist)
	{
		ccmdhist = cmdhist0;

		if(ccmdhist)
		{
			while(ccmdhist->prev)
				ccmdhist = ccmdhist->prev;

			strcpy(console_input, ccmdhist->cmd);

			console_inputting = 1;
		}
	}
	else
	{
		ccmdhist = ccmdhist->next;

		if(!ccmdhist)
		{
			ccmdhist = cmdhist0;

			while(ccmdhist->prev)
				ccmdhist = ccmdhist->prev;
		}

		strcpy(console_input, ccmdhist->cmd);

		console_inputting = 1;
	}
}


void console_tab(int state)
{
	if(!state)
		return;

	if(!r_DrawConsole)
		return;

	if(tabbed)
	{
		strcpy(console_input, old_console_input);
		list_cvars(console_input);
	}
	else
	{
		strcpy(old_console_input, console_input);

		if(console_inputting)
			complete_cvar(console_input);

		tabbed = 1;
	}
}


void console_backspace(int state)
{
	if(!state)
		return;

	if(!r_DrawConsole)
		return;

	int l = strlen(console_input);

	if(l != 0)
		console_input[l - 1] = '\0';

	tabbed = 0;
}


void console_enter(int state)
{
	if(!state)
		return;

	if(!r_DrawConsole)
		return;

	int l = strlen(console_input);

	if(l != 0 && console_inputting)
	{
		console_print(console_input);
		console_print("\n");
		add_cmd_hist(console_input);
		struct string_t *s = new_string_text(console_input);
		console_input[0] = '\0';	// why can't we do this after?

		console_inputting = 0;

		tabbed = 0;

		if(rconing == RCCON_IN)
			rcon_command(s->text);
		else
			parse_command(s->text);

		free_string(s);

		return;
	}

	console_inputting = 0;

	tabbed = 0;
}


void console_keypress(char c)
{
	if(!r_DrawConsole)
		return;

	int l = strlen(console_input);

	console_inputting = 1;

	if(strlen(console_input) < CONSOLE_INPUT_LENGTH - 1)
	{
		console_input[l++] = c;
		console_input[l] = '\0';
	}

	tabbed = 0;
}


void dump_console()
{
	struct string_t *string = new_string_string(emergence_home_dir);
	string_cat_text(string, "/nfcl.log");
	
	FILE *file = fopen(string->text, "w");
	
	free_string(string);
	
	if(!file)
		return;

	int l;
	struct consolell_t *console = console0;

	if(console)
	{
		while(console->prev)
			console = console->prev;

		do
		{
			for(l = 0; l != console->numlines; l++)
			{
				fputs(console->line[l], file);
				fputc('\n', file);
			}

			console = console->next;

		} while(console);
	}

	fclose(file);
}


void calc_r_ConsoleColour()
{
	r_ConsoleColour = convert_24bit_to_16bit(r_ConsoleRed, r_ConsoleGreen, r_ConsoleBlue);
}


void qc_r_ConsoleRed(int val)
{
	r_ConsoleRed = val;
	calc_r_ConsoleColour();
}


void qc_r_ConsoleGreen(int val)
{
	r_ConsoleGreen = val;
	calc_r_ConsoleColour();
}


void qc_r_ConsoleBlue(int val)
{
	r_ConsoleBlue = val;
	calc_r_ConsoleColour();
}


void calc_r_RconConsoleColour()
{
	r_RconConsoleColour = convert_24bit_to_16bit(r_RconConsoleRed, r_RconConsoleGreen, r_RconConsoleBlue);
}


void qc_r_RconConsoleRed(int val)
{
	r_RconConsoleRed = val;
	calc_r_RconConsoleColour();
}


void qc_r_RconConsoleGreen(int val)
{
	r_RconConsoleGreen = val;
	calc_r_RconConsoleColour();
}


void qc_r_RconConsoleBlue(int val)
{
	r_RconConsoleBlue = val;
	calc_r_RconConsoleColour();
}


void calc_r_ConsoleTextColour()
{
	r_ConsoleTextColour = convert_24bit_to_16bit(r_ConsoleTextRed, r_ConsoleTextGreen, r_ConsoleTextBlue);
}


void qc_r_ConsoleTextRed(int val)
{
	r_ConsoleTextRed = val;
	calc_r_ConsoleTextColour();
}


void qc_r_ConsoleTextGreen(int val)
{
	r_ConsoleTextGreen = val;
	calc_r_ConsoleTextColour();
}


void qc_r_ConsoleTextBlue(int val)
{
	r_ConsoleTextBlue = val;
	calc_r_ConsoleTextColour();
}


void calc_r_ConsoleActiveTextColour()
{
	r_ConsoleActiveTextColour = convert_24bit_to_16bit(r_ConsoleActiveTextRed, r_ConsoleActiveTextGreen, r_ConsoleActiveTextBlue);
}


void qc_r_ConsoleActiveTextRed(int val)
{
	r_ConsoleActiveTextRed = val;
	calc_r_ConsoleActiveTextColour();
}


void qc_r_ConsoleActiveTextGreen(int val)
{
	r_ConsoleActiveTextGreen = val;
	calc_r_ConsoleActiveTextColour();
}


void qc_r_ConsoleActiveTextBlue(int val)
{
	r_ConsoleActiveTextBlue = val;
	calc_r_ConsoleActiveTextColour();
}


void init_console_cvars()
{
	create_cvar_int("r_ConsoleHeight", &r_ConsoleHeight, 0);
	create_cvar_int("r_ConsoleRed", &r_ConsoleRed, 0);
	create_cvar_int("r_ConsoleGreen", &r_ConsoleGreen, 0);
	create_cvar_int("r_ConsoleBlue", &r_ConsoleBlue, 0);
	create_cvar_int("r_ConsoleColour", &r_ConsoleColour, 0);
	create_cvar_int("r_ConsoleAlpha", &r_ConsoleAlpha, 0);
	create_cvar_int("r_RconConsoleRed", &r_RconConsoleRed, 0);
	create_cvar_int("r_RconConsoleGreen", &r_RconConsoleGreen, 0);
	create_cvar_int("r_RconConsoleBlue", &r_RconConsoleBlue, 0);
	create_cvar_int("r_RconConsoleColour", &r_RconConsoleColour, 0);
	create_cvar_int("r_ConsoleTextRed", &r_ConsoleTextRed, 0);
	create_cvar_int("r_ConsoleTextGreen", &r_ConsoleTextGreen, 0);
	create_cvar_int("r_ConsoleTextBlue", &r_ConsoleTextBlue, 0);
	create_cvar_int("r_ConsoleTextColour", &r_ConsoleTextColour, 0);
	create_cvar_int("r_ConsoleActiveTextRed", &r_ConsoleActiveTextRed, 0);
	create_cvar_int("r_ConsoleActiveTextGreen", &r_ConsoleActiveTextGreen, 0);
	create_cvar_int("r_ConsoleActiveTextBlue", &r_ConsoleActiveTextBlue, 0);
	create_cvar_int("r_ConsoleActiveTextColour", &r_ConsoleActiveTextColour, 0);
}


void init_console()
{
	console_input = malloc(CONSOLE_INPUT_LENGTH);
	old_console_input = malloc(CONSOLE_INPUT_LENGTH);
	console_input[0] = '\0';

	set_int_cvar_qc_function("r_ConsoleRed", qc_r_ConsoleRed);
	set_int_cvar_qc_function("r_ConsoleGreen", qc_r_ConsoleGreen);
	set_int_cvar_qc_function("r_ConsoleBlue", qc_r_ConsoleBlue);
	set_int_cvar_qc_function("r_RconConsoleRed", qc_r_RconConsoleRed);
	set_int_cvar_qc_function("r_RconConsoleGreen", qc_r_RconConsoleGreen);
	set_int_cvar_qc_function("r_RconConsoleBlue", qc_r_RconConsoleBlue);
	set_int_cvar_qc_function("r_ConsoleTextRed", qc_r_ConsoleTextRed);
	set_int_cvar_qc_function("r_ConsoleTextGreen", qc_r_ConsoleTextGreen);
	set_int_cvar_qc_function("r_ConsoleTextBlue", qc_r_ConsoleTextBlue);
	set_int_cvar_qc_function("r_ConsoleActiveTextRed", qc_r_ConsoleActiveTextRed);
	set_int_cvar_qc_function("r_ConsoleActiveTextGreen", qc_r_ConsoleActiveTextGreen);
	set_int_cvar_qc_function("r_ConsoleActiveTextBlue", qc_r_ConsoleActiveTextBlue);
}


void kill_console()
{
	free(console_input);
	free(old_console_input);

	console_input = NULL;
	old_console_input = NULL;

}
