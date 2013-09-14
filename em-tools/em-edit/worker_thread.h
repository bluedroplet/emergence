/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _INC_WORKER_THREAD
#define _INC_WORKER_THREAD

void call_start();
void start_worker_thread();
void kill_worker_thread();
void post_job_finished();
extern int mypipe[2];

#endif	// _INC_WORKER_THREAD
