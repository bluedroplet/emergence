/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void init_timer();
uint64_t timestamp();
uint32_t get_tick_from_wall_time();
double get_wall_time();
extern uint64_t start_count, counts_per_second;
