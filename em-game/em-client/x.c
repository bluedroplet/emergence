#define _GNU_SOURCE
#define _REENTRANT


#include <stdint.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define __USE_X_SHAREDMEMORY__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XShm.h>

#include "../common/types.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../gsub/gsub.h"
#include "console.h"
#include "main.h"
#include "entry.h"
#include "control.h"
#include "render.h"

Display *xdisplay;
int xscreen;
Window xwindow;

XImage *image;

GC gc;

int x_fd;


void update_frame_buffer()
{
	XShmPutImage(xdisplay, xwindow, gc, image, 
		0, 0, 0, 0, 800, 600, True);
	XFlush(xdisplay);
}


void process_x()
{
	XEvent report;
	
	XNextEvent(xdisplay, &report);
	
	int CompletionType = XShmGetEventBase (xdisplay) + ShmCompletion;

	switch(report.type)
	{
	case Expose:
		// unless this is the last contiguous expose,
		// don't draw the window 
		if (report.xexpose.count != 0)
			break;

		break;
	case KeyPress:
		//	XKeysymToKeycode(xdisplay, xsym);

		process_keypress(report.xkey.keycode - 8, 1);
		
//		process_keypress(((XKeyEvent*)&report)->keycode, 1);
		break;
	
	case KeyRelease:
		break;
	
	default:
	
		if(report.type == CompletionType)
			buffer_cat_uint32(msg_buf, (uint32_t)MSG_RENDER);
		
		break;
	}
}


void init_x()
{
	// connect to X server 
	if(!(xdisplay = XOpenDisplay(NULL)))
		client_error("cannot connect to X server %s\n", XDisplayName(NULL));
	
	
	xscreen = DefaultScreen(xdisplay);
	
 /*   screen_w = DisplayWidth(SDL_Display, SDL_Screen);
    screen_h = DisplayHeight(SDL_Display, SDL_Screen);
    get_real_resolution(this, &real_w, &real_h);
    if ( current_w > real_w ) {
        real_w = MAX(real_w, screen_w);
    }
    if ( current_h > real_h ) {
        real_h = MAX(real_h, screen_h);
    }
    XMoveResizeWindow(SDL_Display, FSwindow,
                      xinerama_x, xinerama_y, real_w, real_h);
					  */

	x_fd = ConnectionNumber(xdisplay);		// fuck x from behind

	console_print("Getting x to generate signals: ");
	
	if(fcntl(x_fd, F_SETOWN, getpid()) == -1)
		goto error;
	
	if(fcntl(x_fd, F_SETFL, O_ASYNC) == -1)
		goto error;
	
	
	console_print("ok\n");
	
    XSetWindowAttributes xattr;

    xattr.override_redirect = True;
    xattr.background_pixel = BlackPixel(xdisplay, xscreen);
    xattr.border_pixel = 0;
    xattr.colormap = DefaultColormap(xdisplay, xscreen);

    xwindow = XCreateWindow(xdisplay, RootWindow(xdisplay, xscreen), 0, 0, 800, 600, 0,
			     16, InputOutput, DefaultVisual(xdisplay, xscreen),
			     CWOverrideRedirect | CWBackPixel | CWBorderPixel
			     | CWColormap,
			     &xattr);
	
//	xwindow = XCreateWindow(xdisplay, RootWindow(xdisplay, xscreen),
  //                                 0, 0, 800, 600, 0, 16, InputOutput,
	//			   DefaultVisual(xdisplay, xscreen), 0, NULL);

	XSelectInput(xdisplay, xwindow, KeyPressMask | KeyReleaseMask | ExposureMask);
   
	XMapRaised(xdisplay, xwindow);
	
	XSetInputFocus(xdisplay, xwindow, RevertToNone, CurrentTime);
	
  gc=XCreateGC(xdisplay, xwindow, 0, NULL);	
	
	vid_width = 800;
	vid_height = 600;

	vid_pitch = vid_width;
	vid_halfwidth = vid_width / 2;
	vid_heightm1 = vid_height - 1;
	vid_halfheight = vid_height / 2;
	vid_halfheightm1 = vid_halfheight - 1;
	vid_byteswidth = vid_width * 2;
	vid_bbsize = vid_byteswidth * vid_height;

//	vid_backbuffer = (word*)malloc(vid_bbsize);
	

      int ShmMajor,ShmMinor;
      Bool ShmPixmaps;

     XShmQueryVersion(xdisplay,&ShmMajor,&ShmMinor,&ShmPixmaps);

	XShmSegmentInfo *shmseginfo = (XShmSegmentInfo *)malloc(sizeof(XShmSegmentInfo));

    memset(shmseginfo,0, sizeof(XShmSegmentInfo));

	image=XShmCreateImage(xdisplay, DefaultVisual(xdisplay, xscreen), 16, ZPixmap,
                                  NULL, shmseginfo, 800, 600);	
   
   	if(!image)
		client_shutdown();
   
   
 	shmseginfo->shmid=shmget(IPC_PRIVATE, image->bytes_per_line*
			         image->height, IPC_CREAT|0777);
	
   	if(shmseginfo->shmid < 0)
		client_shutdown();
	
  	shmseginfo->shmaddr=shmat(shmseginfo->shmid,NULL,0);
   	if(!shmseginfo->shmaddr)
		client_shutdown();
	shmseginfo->readOnly=False;
	
	vid_backbuffer = (uint16_t*)image->data = shmseginfo->shmaddr;
	
	XShmAttach(xdisplay, shmseginfo);

	sigio_process |= SIGIO_PROCESS_X;
	
	return;

error:
	
	client_error("fail\n");
}
