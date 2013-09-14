/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

struct sample_t
{
	int len;
	int16_t *buf;
};

uint32_t start_global_sample(struct sample_t *sample, uint32_t start_tick);
uint32_t start_static_sample(struct sample_t *sample, float x, float y, uint32_t start_tick);
uint32_t start_entity_sample(struct sample_t *sample, uint32_t ent_index, uint32_t start_tick);

void stop_sample(uint32_t index);
void init_sound();
void kill_sound();
void process_alsa();

struct sample_t *load_sample(char *filename);
