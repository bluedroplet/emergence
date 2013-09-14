/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
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
