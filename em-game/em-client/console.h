void render_console();
void console_print(const char *fmt, ...);
void console_keypress(char c);
void init_console_cvars();
void init_console();
void kill_console();
void dump_console();



void console_toggle(int state);
void prev_command(int state);
void next_command(int state);
void console_tab(int state);
void console_backspace(int state);
void console_enter(int state);
void console_keypress(char c);



extern int r_ConsoleHeight;
extern int r_ConsoleRed;
extern int r_ConsoleGreen;
extern int r_ConsoleBlue;
extern int r_ConsoleColour;
extern int r_ConsoleAlpha;
extern int r_RconConsoleRed;
extern int r_RconConsoleGreen;
extern int r_RconConsoleBlue;
extern int r_RconConsoleColour;
extern int r_ConsoleTextRed;
extern int r_ConsoleTextGreen;
extern int r_ConsoleTextBlue;
extern int r_ConsoleTextColour;
extern int r_ConsoleActiveTextRed;
extern int r_ConsoleActiveTextGreen;
extern int r_ConsoleActiveTextBlue;
extern int r_ConsoleActiveTextColour;
extern int r_DrawConsole;

