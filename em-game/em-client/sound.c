#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>

#include <sys/epoll.h>

#include <alsa/asoundlib.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "../common/llist.h"
#include "../common/minmax.h"
#include "shared/timer.h"
#include "console.h"
#include "entry.h"
#include "tick.h"
#include "sound.h"


snd_pcm_t *playback_handle = NULL;


struct queued_sample_t
{
	struct sample_t *sample;
	uint32_t start_tick;
	int begun;
	int next_frame;
	
	struct queued_sample_t *next;
		
} *queued_sample0 = NULL;


int sound_kill_pipe[2];
int sound_active = 0;
pthread_mutex_t sound_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_t sound_thread_id;

int alsa_fd;


void add_frames(int32_t *dst, int16_t *src, int c)
{
	int i = 0;
	
	while(c)
	{
		dst[i] += src[i];
		i++;
		c--;
	}
}


void saturate_frames(int32_t *dst, int c)
{
	int i = 0;
	
	while(c)
	{
		((int16_t*)dst)[i] = (int16_t)min(max(dst[i], -32768), 32767);
		i++;
		c--;
	}
}


void sound_mutex_lock()
{
	pthread_mutex_lock(&sound_mutex);
}
	

void sound_mutex_unlock()
{
	pthread_mutex_unlock(&sound_mutex);
}


void process_alsa()	// check for off-by-ones
{
	char c;
	while(read(alsa_fd, &c, 1) != -1);
		
	int avail = snd_pcm_avail_update(playback_handle);
//	printf("%u\n", avail);
	
	
	if(avail <= 0)
	{
		if(avail != 0)
		{
     	if (avail == -EPIPE) 
		 {    /* under-run */
                snd_pcm_prepare(playback_handle);			
				avail = snd_pcm_avail_update(playback_handle);
			 printf("under-run\n");
			 }
		 else
			 
			 printf("Write error: %s\n", snd_strerror(avail));
		return;
		}
		else
					return;

	}

	int32_t *buf = malloc(avail * 4);	// get rid of me
	
	memset(buf, 0, avail * 4);
	

	uint32_t start_tick = get_game_tick();
	uint32_t end_tick = start_tick + (avail * counts_per_second) / 44100;
	
	struct queued_sample_t *c_queued_sample = queued_sample0;
	
	while(c_queued_sample)
	{
		// is the sample yet to be started
		
		if(!c_queued_sample->begun)
		{
			uint32_t sample_end_tick = c_queued_sample->start_tick + 
				(c_queued_sample->sample->len * counts_per_second) / 44100;
			
			
			// has the time for the sample to be played already passed?
			
			if(sample_end_tick < start_tick)
			{
				struct queued_sample_t *temp = c_queued_sample->next;
				LL_REMOVE(struct queued_sample_t, &queued_sample0, c_queued_sample);
				c_queued_sample = temp;
				
				continue;
			}
			
			
			// should the sample be started from the beginning?
			
			if(c_queued_sample->start_tick <= start_tick)
			{
				add_frames(buf, c_queued_sample->sample->buf, 
					min(c_queued_sample->sample->len, avail));
				
				
				// have we played the whole sample?
				
				if(avail >= c_queued_sample->sample->len)
				{
					struct queued_sample_t *temp = c_queued_sample->next;
					LL_REMOVE(struct queued_sample_t, &queued_sample0, c_queued_sample);
					c_queued_sample = temp;
					
					continue;
				}
				else
				{
					c_queued_sample->begun = 1;
					c_queued_sample->next_frame = avail;
				}
			}
			
			else
			

			// should the sample be started later in this buffer?
			
			if(c_queued_sample->start_tick <= end_tick)
			{
				int start_frame = ((c_queued_sample->start_tick - start_tick) * avail) / 
					((avail * counts_per_second) / 44100);
				
				add_frames(&buf[start_frame], c_queued_sample->sample->buf, 
					min(c_queued_sample->sample->len, avail - start_frame));
				
				
				// have we played the whole sample?
				
				if(avail - start_frame >= c_queued_sample->sample->len)
				{
					struct queued_sample_t *temp = c_queued_sample->next;
					LL_REMOVE(struct queued_sample_t, &queued_sample0, c_queued_sample);
					c_queued_sample = temp;
					
					continue;
				}
				else
				{
					c_queued_sample->begun = 1;
					c_queued_sample->next_frame = avail - start_frame;
				}
			}
		}
		else
		{
			add_frames(buf, &c_queued_sample->sample->buf[c_queued_sample->next_frame], 
				min(c_queued_sample->sample->len - c_queued_sample->next_frame, avail));
			
			
			// have we played the whole sample?
			
			if(c_queued_sample->next_frame + avail >= c_queued_sample->sample->len)
			{
				struct queued_sample_t *temp = c_queued_sample->next;
				LL_REMOVE(struct queued_sample_t, &queued_sample0, c_queued_sample);
				c_queued_sample = temp;
				
				continue;
			}
			else
			{
				c_queued_sample->next_frame += avail;
			}
		}
		
		c_queued_sample = c_queued_sample->next;
	}
	
	saturate_frames(buf, avail);
	snd_pcm_writei(playback_handle, buf, avail);
	
	free(buf);
}


void start_sample(struct sample_t *sample, uint32_t start_tick)
{
	if(!sound_active)
		return;
	
	struct queued_sample_t queued_sample = {
		sample, start_tick, 0};
		
	sound_mutex_lock();
	LL_ADD(struct queued_sample_t, &queued_sample0, &queued_sample);
	sound_mutex_unlock();
}


void *sound_thread(void *a)
{
	int epoll_fd = epoll_create(2);
	
	struct epoll_event ev = 
	{
		.events = EPOLLIN | EPOLLET
	};

	ev.data.u32 = 0;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, alsa_fd, &ev);

	ev.data.u32 = 1;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sound_kill_pipe[0], &ev);

	while(1)
	{
		epoll_wait(epoll_fd, &ev, 1, -1);
		
		switch(ev.data.u32)
		{
		case 0:
			sound_mutex_lock();
			process_alsa();
			sound_mutex_unlock();
			break;
		
		case 1:
			pthread_exit(NULL);
			break;
		}
	}
}


struct sample_t *load_sample(char *filename)
{
	OggVorbis_File vf;

	FILE *file = fopen(filename, "r");
	
	if(ov_open(file, &vf, NULL, 0) < 0) {
	console_print("Input does not appear to be an Ogg bitstream.\n");
	}
	
	vorbis_info *vi=ov_info(&vf,-1);
	
	struct sample_t *sample = malloc(sizeof(struct sample_t));
	
	sample->len = ov_pcm_total(&vf,-1);
	
	console_print("%i\n", sample->len);
	
	sample->buf = malloc(sample->len * 2);
	
	int current_section = 0;
	
	int j = 0;
	while(ov_read(&vf, &sample->buf[j], 2, 0, 2, 1, &current_section) > 0)
		{
		j++;
		}

	ov_clear(&vf);
		
	return sample;
}
	

void init_sound()
{
	int err;
	snd_pcm_hw_params_t *hw_params;
	
	if ((err = snd_pcm_open (&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 
		SND_PCM_NONBLOCK)) < 0) {
		console_print("cannot open audio device default (%s)\n", 
			 snd_strerror (err));
		return;
	}
	
	
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		console_print ("cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		return;
	}
			 
	if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
		console_print ("cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		return;
	}

	if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		console_print ("cannot set access type (%s)\n",
			 snd_strerror (err));
		return;
	}

	if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		console_print ("cannot set sample format (%s)\n",
			 snd_strerror (err));
		return;
	}

	if ((err = snd_pcm_hw_params_set_rate (playback_handle, hw_params, 44100, 0)) < 0) {
		console_print ("cannot set sample rate (%s)\n",
			 snd_strerror (err));
		return;
	}

	if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 1)) < 0) {
		console_print ("cannot set channel count (%s)\n",
			 snd_strerror (err));
		return;
	}

    if (snd_pcm_hw_params_set_periods(playback_handle, hw_params, 2, 0) < 0) {
      fprintf(stderr, "Error setting periods.\n");
		return;
    }
	
  /* Set buffer size (in frames). The resulting latency is given by */
    /* latency = periodsize * periods / (rate * bytes_per_frame)     */
    if (snd_pcm_hw_params_set_buffer_size(playback_handle, hw_params, 2048) < 0) {
      fprintf(stderr, "Error setting buffer size.\n");
 		return;
   }
	
	
/*	int l;
    if (snd_pcm_hw_params_get_buffer_size(hw_params, &l)) {
    }	
*/	

	if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
		console_print ("cannot set parameters (%s)\n",
			 snd_strerror (err));
		return;
	}

	snd_pcm_hw_params_free (hw_params);

	snd_pcm_sw_params_t *sw_params;
	
	/* tell ALSA to wake us up whenever 4096 or more frames
		   of playback data can be delivered. Also, tell
		   ALSA that we'll start the device ourselves.
		*/
	
/*		if ((err = snd_pcm_sw_params_malloc (&sw_params)) < 0) {
			fprintf (stderr, "cannot allocate software parameters structure (%s)\n",
				 snd_strerror (err));
			exit (1);
		}
		if ((err = snd_pcm_sw_params_current (playback_handle, sw_params)) < 0) {
			fprintf (stderr, "cannot initialize software parameters structure (%s)\n",
				 snd_strerror (err));
			exit (1);
		}
		if ((err = snd_pcm_sw_params_set_avail_min (playback_handle, sw_params, 1)) < 0) {
			fprintf (stderr, "cannot set minimum available count (%s)\n",
				 snd_strerror (err));
			exit (1);
		}
	/*	if ((err = snd_pcm_sw_params_set_sleep_min (playback_handle, sw_params, 30)) < 0) {
			fprintf (stderr, "cannot set minimum available sleep (%s)\n",
				 snd_strerror (err));
			exit (1);
		}
		if ((err = snd_pcm_sw_params_set_start_threshold (playback_handle, sw_params, 0)) < 0) {
			fprintf (stderr, "cannot set start mode (%s)\n",
				 snd_strerror (err));
			exit (1);
		}
		if ((err = snd_pcm_sw_params (playback_handle, sw_params)) < 0) {
			fprintf (stderr, "cannot set software parameters (%s)\n",
				 snd_strerror (err));
			exit (1);
		}
	
		/* the interface will interrupt the kernel every 4096 frames, and ALSA
		   will wake up this program very soon after that.
		*/
	

	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
		console_print ("cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		return;
	}
	
	sound_active = 1;
	
	pthread_mutex_init(&sound_mutex, NULL);
	pipe(sound_kill_pipe);
	alsa_fd = create_timer_listener();
	pthread_create(&sound_thread_id, NULL, sound_thread, NULL);
}


void kill_sound()
{
	if(!sound_active)
		return;
	
	char c;
	write(sound_kill_pipe[1], &c, 1);
	pthread_join(sound_thread_id, NULL);

	if(playback_handle)
	{
		snd_pcm_close(playback_handle);
		playback_handle = NULL;
	}
}
