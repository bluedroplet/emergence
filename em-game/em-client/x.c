#define _GNU_SOURCE
#define _REENTRANT


#include <stdint.h>
#include <signal.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/epoll.h>


#define __USE_X_SHAREDMEMORY__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xrandr.h>

#include "../common/types.h"
#include "../common/llist.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../gsub/gsub.h"
#include "shared/timer.h"
#include "shared/cvar.h"
#include "console.h"
#include "main.h"
#include "entry.h"
#include "control.h"
#include "render.h"
#include "map.h"
#include "game.h"


Display *xdisplay;
int xscreen;
Window xwindow;

XImage *image;

GC gc;

int x_fd;
int x_render_pipe[2];
int x_kill_pipe[2];

XRRScreenConfiguration *screen_config;
int original_mode;

struct vid_mode_t
{
	int width;
	int height;
	int index;
	
	struct vid_mode_t *next;
		
} *vid_mode0 = NULL;

int vid_mode;

pthread_mutex_t x_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_t x_thread_id;


void update_frame_buffer()
{
	pthread_mutex_lock(&x_mutex);	
	
//	float time1 = get_double_time();
	
	XShmPutImage(xdisplay, xwindow, gc, image, 
		0, 0, 0, 0, vid_width, vid_height, True);
//	float time2 = get_double_time();
	XFlush(xdisplay);
//	float time3 = get_double_time();
	
//	printf("%f %f %f\n", time1, time2, time3);
	
	pthread_mutex_unlock(&x_mutex);	
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
		process_keypress(report.xkey.keycode - 8, 0);
		break;
	
	default:
	
		if(report.type == CompletionType)
		{
			char c;
			write(x_render_pipe[1], &c, 1);
		}
		
		break;
	}
}

Cursor create_blank_cursor()
{
	Cursor cursor;
	XGCValues GCvalues;
	GC        GCcursor;
	XImage *data_image, *mask_image;
	Pixmap  data_pixmap, mask_pixmap;
	int       clen, i;
	char     *x_data, *x_mask;
	static XColor black = {  0,  0,  0,  0 };
	static XColor white = { 0xffff, 0xffff, 0xffff, 0xffff };
	
	uint8_t data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t mask[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	/* Mix the mask and the data */
	clen = 8;
	x_data = (char *)malloc(clen);
	if ( x_data == NULL ) {
	//	SDL_OutOfMemory();
		return(NULL);
	}
	x_mask = (char *)malloc(clen);
	if ( x_mask == NULL ) {
		free(x_data);
	//	SDL_OutOfMemory();
		return(NULL);
	}
	for ( i=0; i<clen; ++i ) {
		/* The mask is OR'd with the data to turn inverted color
		   pixels black since inverted color cursors aren't supported
		   under X11.
		 */
		x_mask[i] = data[i] | mask[i];
		x_data[i] = data[i];
	}


	/* Create the data image */
	data_image = XCreateImage(xdisplay, 
			DefaultVisual(xdisplay, xscreen),
					1, XYBitmap, 0, x_data, 8, 8, 8, 1);
	data_image->byte_order = MSBFirst;
	data_image->bitmap_bit_order = MSBFirst;
	data_pixmap = XCreatePixmap(xdisplay, xwindow, 8, 8, 1);

	/* Create the data mask */
	mask_image = XCreateImage(xdisplay, 
			DefaultVisual(xdisplay, xscreen),
					1, XYBitmap, 0, x_mask, 8, 8, 8, 1);
	mask_image->byte_order = MSBFirst;
	mask_image->bitmap_bit_order = MSBFirst;
	mask_pixmap = XCreatePixmap(xdisplay, xwindow, 8, 8, 1);

	/* Create the graphics context */
	GCvalues.function = GXcopy;
	GCvalues.foreground = ~0;
	GCvalues.background =  0;
	GCvalues.plane_mask = AllPlanes;
	GCcursor = XCreateGC(xdisplay, data_pixmap,
			(GCFunction|GCForeground|GCBackground|GCPlaneMask),
								&GCvalues);

	/* Blit the images to the pixmaps */
	XPutImage(xdisplay, data_pixmap, GCcursor, data_image,
							0, 0, 0, 0, 8, 8);
	XPutImage(xdisplay, mask_pixmap, GCcursor, mask_image,
							0, 0, 0, 0, 8, 8);
	XFreeGC(xdisplay, GCcursor);
	/* These free the x_data and x_mask memory pointers */
	XDestroyImage(data_image);
	XDestroyImage(mask_image);

	/* Create the cursor */
	cursor = XCreatePixmapCursor(xdisplay, data_pixmap,
				mask_pixmap, &black, &white, 0, 0);


	return(cursor);
}


void query_vid_modes()
{
	console_print("Querying available video modes\n");
	
	screen_config = XRRGetScreenInfo(xdisplay, RootWindow(xdisplay, xscreen));
	
	if(!screen_config)
		client_error("XRRScreenConfig failed");
	
	int nsizes;
	XRRScreenSize *screens = XRRConfigSizes(screen_config, &nsizes);
	
	
	int n = 0;
	int modes = 0;
	while(nsizes)
	{
		int width = screens[n].width;
		int height = screens[n].height;

		n++;
		nsizes--;
		
		if(width * 3 != height * 4)
			continue;

		double d;
		if(modf((double)width / 8, &d) != 0.0)	// make sure 200x200 blocks scale to integer
									// (change this to integer)
			continue;
			
		modes++;
		
		struct vid_mode_t vid_mode = {width, height, n-1};
		LL_ADD(struct vid_mode_t, &vid_mode0, &vid_mode);
	}
	
	console_print("Found %u modes:\n", modes);
	
	// sort modes
	
	struct vid_mode_t *new_vid_mode0 = NULL;
	
	while(vid_mode0)
	{
		struct vid_mode_t *max_vid_mode = vid_mode0;
		
		struct vid_mode_t *nvid_mode = max_vid_mode->next;
			
		while(nvid_mode)
		{
			if(nvid_mode->width > max_vid_mode->width)
				max_vid_mode = nvid_mode;
			
			nvid_mode = nvid_mode->next;
		}
		
		console_print("%ux%u\n", max_vid_mode->width, max_vid_mode->height);
		
		LL_ADD_TAIL(struct vid_mode_t, &new_vid_mode0, max_vid_mode);
		LL_REMOVE(struct vid_mode_t, &vid_mode0, max_vid_mode);
	}
	
	vid_mode0 = new_vid_mode0;
	
	int i;
	original_mode = XRRConfigCurrentConfiguration(screen_config, &i);
}


void set_vid_mode(int mode)	// use goto error crap
{
	pthread_mutex_lock(&x_mutex);	
	
	
	struct vid_mode_t *cvid_mode = vid_mode0;
		
	while(mode)
	{
		mode--;
		
		cvid_mode = cvid_mode->next;
		if(!cvid_mode)
			return;
	}
	

	XRRSetScreenConfig (xdisplay, screen_config,
			   RootWindow(xdisplay, xscreen),
			   cvid_mode->index,
			   RR_Rotate_0,
			   CurrentTime);
	
    XSetWindowAttributes xattr;

    xattr.override_redirect = True;
    xattr.background_pixel = BlackPixel(xdisplay, xscreen);
    xattr.border_pixel = 0;
    xattr.colormap = DefaultColormap(xdisplay, xscreen);

	vid_width = cvid_mode->width;
	vid_height = cvid_mode->height;

    xwindow = XCreateWindow(xdisplay, RootWindow(xdisplay, xscreen), 0, 0, vid_width, vid_height, 0,
			     24, InputOutput, DefaultVisual(xdisplay, xscreen),
			     CWOverrideRedirect | CWBackPixel | CWBorderPixel
			     | CWColormap,
			     &xattr);


	XSelectInput(xdisplay, xwindow, KeyPressMask | KeyReleaseMask | ExposureMask);
   
	XMapRaised(xdisplay, xwindow);
	
	XSetInputFocus(xdisplay, xwindow, RevertToNone, CurrentTime);
	
	Cursor xcursor = create_blank_cursor();
	
	XDefineCursor(xdisplay, xwindow, xcursor);
	
  gc=XCreateGC(xdisplay, xwindow, 0, NULL);	
	

	

      int ShmMajor,ShmMinor;
      Bool ShmPixmaps;

     XShmQueryVersion(xdisplay,&ShmMajor,&ShmMinor,&ShmPixmaps);

	XShmSegmentInfo *shmseginfo = (XShmSegmentInfo *)malloc(sizeof(XShmSegmentInfo));

    memset(shmseginfo,0, sizeof(XShmSegmentInfo));

	image=XShmCreateImage(xdisplay, DefaultVisual(xdisplay, xscreen), 24, ZPixmap,
                                  NULL, shmseginfo, vid_width, vid_height);	
   
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
	
	s_backbuffer = new_surface_no_buf(SURFACE_24BITPADDING8BIT, vid_width, vid_height);
	
	s_backbuffer->buf = image->data = shmseginfo->shmaddr;
	s_backbuffer->pitch = image->bytes_per_line;
	
	XShmAttach(xdisplay, shmseginfo);

	
	pthread_mutex_unlock(&x_mutex);
}


void vid_mode_qc(int mode)
{
	vid_mode = mode;
	set_vid_mode(mode);
	
	game_resolution_change();
}


void create_x_cvars()
{
	create_cvar_int("vid_mode", &vid_mode, 0);
}


void *x_thread(void *a)
{
	int epoll_fd = epoll_create(2);
	
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.u32 = 0;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, x_fd, &ev);

	ev.events = EPOLLIN | EPOLLET;
	ev.data.u32 = 1;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, x_kill_pipe[0], &ev);

	while(1)
	{
		epoll_wait(epoll_fd, &ev, 1, -1);
		
		switch(ev.data.u32)
		{
		case 0:
			pthread_mutex_lock(&x_mutex);
			process_x();
			pthread_mutex_unlock(&x_mutex);
			break;
		
		case 1:
			pthread_exit(NULL);
			break;
		}
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

	x_fd = ConnectionNumber(xdisplay);

	console_print("Getting x to generate signals: ");
	
	fcntl(x_fd, F_SETFL, O_NONBLOCK);
	
	
	console_print("ok\n");
	
	
	XAutoRepeatOff(xdisplay);
	
	query_vid_modes();
	set_int_cvar_qc_function("vid_mode", vid_mode_qc);
	set_vid_mode(vid_mode);

	pipe(x_render_pipe);
	fcntl(x_render_pipe[0], F_SETFL, O_NONBLOCK);
	
	pipe(x_kill_pipe);
	
	pthread_create(&x_thread_id, NULL, x_thread, NULL);

	
	return;

error:
	
	client_error("fail\n");
}


void kill_x()
{
	char c;
	write(x_kill_pipe[1], &c, 1);
	pthread_join(x_thread_id, NULL);
	
	XAutoRepeatOn(xdisplay);
	
	XRRSetScreenConfig (xdisplay, screen_config,
			   xwindow,
			   original_mode,
			   RR_Rotate_0,
			   CurrentTime);
	
	XRRFreeScreenConfigInfo(screen_config);
}
