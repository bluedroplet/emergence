#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <ddraw.h>

#include "../../common/string.h"
#include "../../common/cvar.h"
#include "../../gsub/gsub.h"
#include "main.h"
#include "winmain.h"
#include "console.h"

LPDIRECTDRAW4			dd4Object	= NULL;
LPDIRECTDRAWSURFACE4	dds4Primary	= NULL;
LPDIRECTDRAWSURFACE4	dds4Back	= NULL;


int vid_bbsize, vid_heightm1, vid_halfwidth, vid_halfheight, vid_halfheightm1;


void update_frame_buffer()
{
	DDSURFACEDESC2	ddsd2;

	memset(&ddsd2, 0, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);

	if(dds4Back->Lock(NULL, &ddsd2, DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL) != DD_OK)
		return;

	char *src = (char*)vid_backbuffer;
	char *dst = (char*)ddsd2.lpSurface;


	int s;
	
	if(ddsd2.lPitch == vid_byteswidth)
	{
		s = vid_bbsize / 4;

		__asm
		{
			mov esi, src
			mov edi, dst
			mov ecx, s

			rep movsd


			mov edi, src
			xor eax, eax
			mov ecx, s

			rep stosd
		}
	}
	else
	{
		s = vid_byteswidth / 4;

		for(int y = 0; y != vid_height; y++)
		{
			__asm
			{
				mov esi, src
				mov edi, dst
				mov ecx, s

				rep movsd


				mov edi, src
				xor eax, eax
				mov ecx, s

				rep stosd
			}

			src += vid_byteswidth;
			dst += ddsd2.lPitch;
		}
	}


	dds4Back->Unlock(NULL);

//	if(g_VSync)
//		dds4Primary->Flip(NULL, 0);
//	else
		dds4Primary->Flip(NULL, DDFLIP_NOVSYNC);
}



int vid_nummodes = 0;

struct
{
	int width, height;

} vid_moderes[25];


HRESULT WINAPI enum_modes_callback(LPDDSURFACEDESC2 lpddsd, void *lpContext)
{
	if(lpddsd->ddpfPixelFormat.dwRGBBitCount != 16 || (lpddsd->dwWidth * 3) != (lpddsd->dwHeight * 4))
		return DDENUMRET_OK;

	vid_moderes[vid_nummodes].width = lpddsd->dwWidth;
	vid_moderes[vid_nummodes++].height = lpddsd->dwHeight;

	if(vid_nummodes == 25)
		return DDENUMRET_CANCEL;
	else
		return DDENUMRET_OK;
}


void sort_vid_modes()
{
	int i, temp, stop;

	do
	{
		stop = 1;

		for(i = 0; i != vid_nummodes - 1; i++)
			if(vid_moderes[i].width > vid_moderes[i + 1].width)
			{
				temp = vid_moderes[i].width;
				vid_moderes[i].width = vid_moderes[i + 1].width;
				vid_moderes[i + 1].width = temp;

				temp = vid_moderes[i].height;
				vid_moderes[i].height = vid_moderes[i + 1].height;
				vid_moderes[i + 1].height = temp;

				stop = 0;
			}

	} while(!stop);
}


void list_vid_modes(char*)
{
	int i;
	
	char *text = new char[50];

	for(i = 0; i != vid_nummodes; i++)
	{
		console_print("Video mode ");
		itoa(i, text, 10);
		console_print(text);
		console_print(": ");
		itoa(vid_moderes[i].width, text, 10);
		console_print(text);
		console_print("x");
		itoa(vid_moderes[i].height, text, 10);
		console_print(text);
		console_print("\n");
	}

	delete[] text;
}


int set_vid_mode(int mode)
{
	if(mode >= vid_nummodes)
	{
		console_print("That video mode does not exist.\n");
		return 0;
	}

//	vid_width = vid_moderes[4].width;
//	vid_height = vid_moderes[4].height;

	vid_width = 1280;
	vid_height = 960;

	string s("Setting video mode ");
	s.cat(1);
	s.cat(": ");
	s.cat(vid_width);
	s.cat("x");
	s.cat(vid_height);
	s.cat("\n");
	console_print(s.text);

	free(vid_backbuffer);

	if(dds4Primary != NULL)
		dds4Primary->Release();

	vid_pitch = vid_width;
	vid_halfwidth = vid_width / 2;
	vid_heightm1 = vid_height - 1;
	vid_halfheight = vid_height / 2;
	vid_halfheightm1 = vid_halfheight - 1;
	vid_byteswidth = vid_width * 2;
	vid_bbsize = vid_byteswidth * vid_height;


	if(dd4Object->SetDisplayMode(vid_width, vid_height, 16, 85, 0) != DD_OK)
		shutdown();

	DDSURFACEDESC2	ddsd2;
	memset(&ddsd2, 0, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);
	ddsd2.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
	ddsd2.dwBackBufferCount = 2;
	
	if(dd4Object->CreateSurface(&ddsd2, &dds4Primary, NULL) != DD_OK)
		shutdown();

	ddsd2.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

	if(dds4Primary->GetAttachedSurface(&ddsd2.ddsCaps, &dds4Back) != DD_OK)
		shutdown();

	vid_backbuffer = (word*)malloc(vid_bbsize);

	return 1;
}


void init_graphics()
{
	console_print("Creating DirectDraw object: ");

	if(CoCreateInstance(CLSID_DirectDraw, NULL, CLSCTX_ALL, IID_IDirectDraw4, 
		(void**)&dd4Object) != S_OK)
		shutdown();

	console_print("ok\nInitialising DirectDraw object: ");

	if(dd4Object->Initialize(NULL) != DD_OK)
		shutdown();

	console_print("ok\nEnumerating display modes: ");

	if(dd4Object->EnumDisplayModes(0, 0, NULL, &enum_modes_callback))
		shutdown();

	if(vid_nummodes == 0)
	{
		console_print("fail\n No valid display modes.\n");
		shutdown();
	}

	sort_vid_modes();

	console_print("ok\nSetting coop level: ");

	if(dd4Object->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT) != DD_OK)
		shutdown();

	console_print("ok\n");

	if(!set_vid_mode(get_cvar_int("g_VidMode")))
		shutdown();

	create_cvar_command("g_ListVidModes", list_vid_modes);
//	set_cvar_qc_function("g_VidMode", set_vid_mode);
}


#ifdef _DEBUG

void debug_init_graphics()
{
	vid_width = 1280;
	vid_height = 960;

	vid_pitch = vid_width;
	vid_halfwidth = vid_width / 2;
	vid_heightm1 = vid_height - 1;
	vid_halfheight = vid_height / 2;
	vid_halfheightm1 = vid_halfheight - 1;
	vid_byteswidth = vid_width * 2;
	vid_bbsize = vid_byteswidth * vid_height;

	vid_backbuffer = (word*)malloc(vid_bbsize);
}

#endif


void kill_graphics()
{
	free(vid_backbuffer);

	if(dds4Primary != NULL)
		dds4Primary->Release();

	if(dd4Object != NULL)
	{
		dd4Object->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
		dd4Object->Release();
	}
}

