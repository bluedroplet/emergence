/* 
	Copyright (C) 1998-2004 Jonathan Brown

    This file is part of em-tools.

    em-tools is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    em-tools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with em-tools; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Jonathan Brown
	jbrown@emergence.uk.net
*/


#define _GNU_SOURCE
#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>

#include "worker_thread.h"


pthread_mutex_t mutex;


void enter_main_lock()
{
	pthread_mutex_lock(&mutex);
}


void worker_enter_main_lock()
{
	pthread_mutex_lock(&mutex);
}


int worker_try_enter_main_lock()
{
	if(!pthread_mutex_trylock(&mutex))
		return 1;
	else
		return 0;
}


void leave_main_lock()
{
	pthread_mutex_unlock(&mutex);
}


void worker_leave_main_lock()
{
	pthread_mutex_unlock(&mutex);
}


void create_main_lock()
{
	pthread_mutex_init(&mutex, NULL);
}


void destroy_main_lock()
{
	pthread_mutex_destroy(&mutex);
}
