/* 
	Copyright (C) 1998-2002 Jonathan Brown

    This file is part of em-edit.

    em-edit is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    em-edit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with em-edit; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Jonathan Brown
	jbrown@emergence.uk.net
*/

#ifndef _INC_WORKER
#define _INC_WORKER

int check_stop_callback();
int in_lock_check_stop_callback();
void start_working();
void stop_working();
void job_complete_callback();
int init_worker();
void kill_worker();
void start();

extern int compiled;
extern int job_type;	// the job to be / being done in worker thread


#define JOB_TYPE_SLEEP					0
#define JOB_TYPE_BSP					1
#define JOB_TYPE_UI_BSP					2
#define JOB_TYPE_RESAMPLING_OBJECT		3
#define JOB_TYPE_SCALING_OBJECT			4
#define JOB_TYPE_SCALING				5
#define JOB_TYPE_CONN_VERTICIES			6
#define JOB_TYPE_CONN_SQUISH			7
#define JOB_TYPE_FILL_VERTICIES			8
#define JOB_TYPE_NODE_VERTICIES			9
#define JOB_TYPE_TILING					10
#define JOB_TYPE_RENDERING				11
#define JOB_TYPE_CAMERON				12


#endif	// _INC_WORKER
