#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "../../common/types.h"
#include "../../common/string.h"
#include "../../common/buffer.h"
#include "../../common/cvar.h"
#include "../../gsub/gsub.h"
#include "main.h"
#include "graphics.h"
#include "pipe.h"
#include "console.h"
#include "render.h"
#include "game.h"


HANDLE render_thread_handle = NULL;
HANDLE to_render_pipe = NULL, from_render_pipe = NULL;

#define RENDER_REQUEST	0


// definition of the render language

#define RENDER_STOP					0
#define RENDER_BLIT_SURFACE			1
#define RENDER_BLIT_SURFACE_RECT		2
#define RENDER_DRAW_ALPHA_RECT		4
#define RENDER_TEXT					3
#define RENDER_FINISHED				-1

DWORDLONG FRUpdateCount, FROldCount, FDTime, startcount, count, oldcount;

/*
int FROldFrame, frame;
char FRText[8] = "";
char FDText[8] = "";
*/

int g_VidMode = 2;
int g_VSync = 1;
int r_OutputFC = 0;
int r_DrawFPS = 0;
int r_FPSRed = 0;
int r_FPSGreen = 0;
int r_FPSBlue = 0;
int r_FPSColour = 0;



//
// RENDER THREAD FUNCTIONS
//


void process_render_text(buffer *renderlist)
{
	word x = renderlist->read_int();
	word y = renderlist->read_int();
	word colour = renderlist->read_word();
	string *str = renderlist->read_string();

	blit_text(x, y, colour, str);

	delete str;
}


void process_render_alpha_rect(buffer *renderlist)
{
	blit_destx = renderlist->read_int();
	blit_desty = renderlist->read_int();
	blit_width = renderlist->read_int();
	blit_height = renderlist->read_int();
	blit_colour = renderlist->read_word();
	blit_alpha = renderlist->read_byte();

	draw_alpha_rect();
}


void process_render_surface(buffer *renderlist)
{
	blit_source = (surface_t*)renderlist->read_dword();
	blit_destx = renderlist->read_int();
	blit_desty = renderlist->read_int();

	blit_surface();
}


void process_render_surface_rect(buffer *renderlist)
{
	blit_source = (surface_t*)renderlist->read_dword();
	blit_sourcex = renderlist->read_int();
	blit_sourcey = renderlist->read_int();
	blit_destx = renderlist->read_int();
	blit_desty = renderlist->read_int();
	blit_width = renderlist->read_int();
	blit_height = renderlist->read_int();

	blit_surface_rect();
}


DWORD WINAPI render_thread(void *arg)
{
	while(1)
	{
		buffer buf(int(RENDER_REQUEST));

		thread_pipe_send(from_render_pipe, &buf);

		WaitForSingleObject(to_render_pipe, INFINITE);

		buffer *renderlist = thread_pipe_recv(to_render_pipe);

		int stop = 0;

		while(!stop)
		{
			switch(renderlist->read_int())
			{
			case RENDER_STOP:
				ExitThread(0);
				break;

			case RENDER_BLIT_SURFACE:
				process_render_surface(renderlist);
				break;

			case RENDER_BLIT_SURFACE_RECT:
				process_render_surface_rect(renderlist);
				break;

			case RENDER_TEXT:
				process_render_text(renderlist);
				break;

			case RENDER_DRAW_ALPHA_RECT:
				process_render_alpha_rect(renderlist);
				break;

			case RENDER_FINISHED:
#ifdef _DEBUG
				clear_backbuffer();
#else
				update_frame_buffer();
#endif

				stop = 1;

#ifdef _DEBUG
				Sleep(1000);
				console_print("Frame Drawn\n");
#endif
				break;

			default:
				break;
			}
		}

		delete renderlist;
	}

	return 1;
}


//
// SUB FUNCTIONS
//


void start_render_thread()
{
	DWORD thread_id;
	render_thread_handle = CreateThread(NULL, 0, render_thread, NULL, 0, &thread_id);
	
	if(render_thread_handle == NULL)
	{
		shutdown();
	}
}


void stop_render_thread()
{
	if(render_thread_handle)
	{
		buffer buf(int(RENDER_STOP));
		thread_pipe_send(to_render_pipe, &buf);

		if(WaitForSingleObject(render_thread_handle, 10000) == WAIT_TIMEOUT)
			TerminateThread(render_thread_handle, 0);

		CloseHandle(render_thread_handle);
		render_thread_handle = NULL;
	}	
}


void InitFR()
{
//	FROldFrame = frame;
//	FROldCount = startcount;
//	FRUpdateCount = FROldCount + clockfreq;
//	FDTime = 0;
}


void RenderFR()
{
/*	if(count >= FRUpdateCount)
	{
		_gcvt((((float)(LONGLONG)clockfreq) / ((float)(LONGLONG)(count - FROldCount))) * 
			(float)(frame - FROldFrame), 4, FRText);

//		_itoa( ftol(( ((float)(LONGLONG)FDTime) / ((float)(LONGLONG)(count - FROldCount)))  * 100.0f),
//			FDText, 10);

	//	strcat(FDText, "%");

		FROldFrame = frame;
		FROldCount = count;
		FDTime = 0;

		FRUpdateCount = ((count - startcount) / clockfreq + 1) * 
			clockfreq + startcount;
	}

	string frstr(FRText);

	render_text(0, 0, r_FPSColour, &frstr);

	frame++;

	oldcount = count;

	QueryPerformanceCounter((LARGE_INTEGER*)&count);
*/
}


void calc_r_FPSColour()
{
	r_FPSColour = convert_24bit_to_16bit(r_FPSRed, r_FPSGreen, r_FPSBlue);
}


void qc_r_FPSRed(int val)
{
	r_FPSRed = val;
	calc_r_FPSColour();
}


void qc_r_FPSGreen(int val)
{
	r_FPSGreen = val;
	calc_r_FPSColour();
}


void qc_r_FPSBlue(int val)
{
	r_FPSBlue = val;
	calc_r_FPSColour();
}




//
// INTERFACE FUNCTIONS
//


void init_render_cvars()
{
	create_cvar_int("r_OutputFC", &r_OutputFC, 0);
	create_cvar_int("r_DrawConsole", &r_DrawConsole, 0);
	create_cvar_int("r_DrawFPS", &r_DrawFPS, 0);
	create_cvar_int("r_FPSRed", &r_FPSRed, 0);
	create_cvar_int("r_FPSGreen", &r_FPSGreen, 0);
	create_cvar_int("r_FPSBlue", &r_FPSBlue, 0);
	create_cvar_int("r_FPSColour", &r_FPSColour, 0);
}


void init_render()
{
	to_render_pipe = create_thread_pipe();
	from_render_pipe = create_thread_pipe();

#ifdef _DEBUG

	debug_init_graphics();

#else

	init_graphics();

#endif

	init_gsub();

	QueryPerformanceCounter((LARGE_INTEGER*)&startcount);

//	frame = 0;

//	count = startcount;

//	InitFR();

	set_cvar_qc_function("r_FPSRed", qc_r_FPSRed);
	set_cvar_qc_function("r_FPSGreen", qc_r_FPSGreen);
	set_cvar_qc_function("r_FPSBlue", qc_r_FPSBlue);

	start_render_thread();
}


void kill_render()
{
	stop_render_thread();

	kill_graphics();
	kill_gsub();
}


void change_vid_mode()
{
	stop_render_thread();
	//dosomething
	start_render_thread();
}


buffer renderlist;


void render_surface(surface_t *surface, int x, int y)
{
	renderlist.cat(int(RENDER_BLIT_SURFACE));
	renderlist.cat(dword(surface));
	renderlist.cat(x);
	renderlist.cat(y);
}


void render_surface_rect(surface_t *surface, int src_x, int src_y, int dst_x, int dst_y, int width, int height)
{
	renderlist.cat(int(RENDER_BLIT_SURFACE_RECT));
	renderlist.cat(dword(surface));
	renderlist.cat(src_x);
	renderlist.cat(src_y);
	renderlist.cat(dst_x);
	renderlist.cat(dst_y);
	renderlist.cat(width);
	renderlist.cat(height);
}


void render_text(int x, int y, word colour, string *str)
{
	renderlist.cat(int(RENDER_TEXT));
	renderlist.cat(x);
	renderlist.cat(y);
	renderlist.cat(colour);
	renderlist.cat(str);
}


void render_alpha_rect(int x, int y, int width, int height, word colour, byte alpha)
{
	renderlist.cat(int(RENDER_DRAW_ALPHA_RECT));
	renderlist.cat(x);
	renderlist.cat(y);
	renderlist.cat(width);
	renderlist.cat(height);
	renderlist.cat(colour);
	renderlist.cat(alpha);
}


void render_finish()
{
	renderlist.cat(int(RENDER_FINISHED));
	thread_pipe_send(to_render_pipe, &renderlist);
	renderlist.clear();
}


void process_render_message()
{
	buffer *buf = thread_pipe_recv(from_render_pipe);

	delete buf;

	render_game();

	render_console();

	RenderFR();

	render_finish();
}

