/* 
	Copyright (C) 1998-2002 Jonathan Brown

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

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "worker.h"

pthread_t thread;
pthread_cond_t cond;
pthread_mutex_t lock;
int mypipe[2];
int exit_worker_thread = 0;


void *worker_thread(void *a)
{
	while(1)
	{
		pthread_cond_wait(&cond, &lock);
		
		if(exit_worker_thread)
			pthread_exit(NULL);
		
		start();
	}

	return NULL;
}


void signal_thread()
{
	pthread_mutex_lock(&lock);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
}


void call_start()
{
	signal_thread();
}


void post_job_finished()
{
	char c;
	write(mypipe[1], &c, 1);
}


void start_worker_thread()
{
	pipe(mypipe);
	pthread_mutex_init(&lock, NULL);
	pthread_mutex_lock(&lock);
	pthread_cond_init(&cond, NULL);
	pthread_create(&thread, NULL, worker_thread, NULL);
}


void kill_worker_thread()
{
	exit_worker_thread = 1;
	signal_thread();
	pthread_join(thread, NULL);
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&lock);
	close(mypipe[0]);
	close(mypipe[1]);
}
