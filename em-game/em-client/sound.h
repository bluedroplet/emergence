struct sample_t
{
	int len;
	int16_t *buf;
};


void start_sample(struct sample_t *sample, uint32_t start_tick);
void init_sound();
void kill_sound();
void process_alsa();

extern struct sample_t railgun_sample;
