/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void add_game_tick(uint32_t game_tick, uint64_t *tsc);
void update_tick_parameters();
uint32_t get_game_tick();
double get_time_from_game_tick(float tick);
void init_tick_cvars();
void clear_ticks();
