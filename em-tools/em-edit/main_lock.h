/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void enter_main_lock();
int try_enter_main_lock();
void worker_enter_main_lock();
int worker_try_enter_main_lock();
void worker_main_lock();
void worker_leave_main_lock();
void leave_main_lock();
void create_main_lock();
void destroy_main_lock();

#ifdef _INC_GSUB
struct surface_t *leave_main_lock_and_rotate_surface(struct surface_t *in_surface, 
	int scale_width, int scale_height, double theta);
struct surface_t *leave_main_lock_and_resize_surface(struct surface_t *in_surface, 
	int width, int height);
#endif
