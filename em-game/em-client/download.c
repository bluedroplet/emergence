#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/poll.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "../common/stringbuf.h"
#include "../common/user.h"
#include "shared/network.h"
#include "game.h"
#include "map.h"

int download_in_pipe[2];
int download_out_pipe[2];
pthread_t download_thread_id;

#define DOWNLOAD_THREAD_IN_SHUTDOWN				0
#define DOWNLOAD_THREAD_IN_DOWNLOAD_FILE		1
#define DOWNLOAD_THREAD_IN_STOP_DOWNLOADING		2

#define DOWNLOAD_THREAD_OUT_DOWNLOAD_COMPLETED	0
#define DOWNLOAD_THREAD_OUT_DOWNLOAD_FAILED		1

int download_net_fd = -1, download_file_fd = -1;
off_t download_size, download_offset;

int start_downloading_map(char *map_name)
{
	download_net_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(download_net_fd < 0)
		return 0;
	//	client_libc_error("socket failure");
printf("1\n");
	
	
	struct sockaddr_in sockaddr;
	get_sockaddr_in_from_conn(game_conn, &sockaddr);
	
	if(connect(download_net_fd, &sockaddr, sizeof(struct sockaddr_in)) < 0)
	{
		close(download_net_fd);
		download_net_fd = -1;
		return 0;
	}
	
printf("2\n");
	send(download_net_fd, map_name, strlen(map_name) + 1, 0);
	
	struct string_t *filename = new_string_string(emergence_home_dir);
	string_cat_text(filename, "/maps/");
	string_cat_text(filename, map_name);
	string_cat_text(filename, ".cmap");
	
	download_file_fd = open(filename->text, O_CREAT | O_WRONLY | O_TRUNC | /*O_NONBLOCK |*/ O_NOFOLLOW, 
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	printf(filename->text);
	free_string(filename);
	
printf("3\n");
	if(download_file_fd < 0)
	{
		close(download_file_fd);
		download_file_fd = -1;
		close(download_net_fd);
		download_net_fd = -1;
		return 0;
	}
	
printf("4\n");
	recv(download_net_fd, &download_size, 4, 0);

	fcntl(download_net_fd, F_SETFL, O_NONBLOCK);

	download_offset = 0;

//	sendfile(download_file_fd, download_net_fd, 
//		&download_offset, download_size);
	
printf("5\n");
	return 1;
}


void _stop_downloading_map()
{
	close(download_net_fd);
	download_net_fd = -1;
	
	close(download_file_fd);
	download_file_fd = -1;
}


void *download_thread(void *a)
{
	uint8_t msg;
	char c;
	struct string_t *map_name = new_string();
	
	while(1)
	{
		struct pollfd *fds;
		int fdcount;
			
		if(download_net_fd != -1)
			fdcount = 2;
		else
			fdcount = 1;
			
		fds = calloc(sizeof(struct pollfd), fdcount);
			
		fds[0].fd = download_in_pipe[0];
		fds[0].events = POLLIN;
		
		if(download_net_fd != -1)
		{
			fds[1].fd = download_net_fd;
			fds[1].events = POLLIN;
		}
			
		if(poll(fds, fdcount, -1) == -1)
		{
			if(errno == EINTR)	// why is this necessary?
				continue;
			
			return NULL;
		}
		
		if(fds[0].revents & POLLIN)
		{
			read(download_in_pipe[0], &msg, 1);
			
			switch(msg)
			{
			case DOWNLOAD_THREAD_IN_SHUTDOWN:
				pthread_exit(NULL);
			
			case DOWNLOAD_THREAD_IN_DOWNLOAD_FILE:
				
				read(download_in_pipe[0], &c, 1);
			
				while(c)
				{
					string_cat_char(map_name, c);
					read(download_in_pipe[0], &c, 1);
				}
				
				printf("%s\n", map_name->text);
				
				if(download_net_fd != -1)
					_stop_downloading_map();
				
				if(start_downloading_map(map_name->text))
					;
				
				break;
				
				
			case DOWNLOAD_THREAD_IN_STOP_DOWNLOADING:
				_stop_downloading_map();
				break;
			}
		}
		
		if(download_net_fd != -1)
		{
			if(fds[1].revents & (POLLERR | POLLHUP))
			{
				_stop_downloading_map();
				continue;
			}
			
			if(fds[1].revents & POLLIN)
			{
			//	if(sendfile(download_file_fd, download_net_fd, 
			//		&download_offset, download_size - download_offset) < 0)
			//	perror(NULL);
				
			/*	printf("download_file_fd: %i\ndownload_net_fd: %i\ndownload_offset: %i\ndownload_size: %i\n",
					download_file_fd, download_net_fd, download_offset, download_size);
			*/	
				
				char buf[64];
				
				int r = recv(download_net_fd, buf, 64, 0);
				
				if(r == 0)
				{
					_stop_downloading_map();
					break;
				}
				
				write(download_file_fd, buf, r);
				
				
				if(download_offset == download_size)
				{
					;
				}
			}
		}
	}
}


void download_map(char *map_name)
{
	uint8_t m = DOWNLOAD_THREAD_IN_DOWNLOAD_FILE;
	write(download_in_pipe[1], &m, 1);
	
	char *cc = map_name;
	
	printf("---%s\n", map_name);
	
	while(*cc)
	{
		write(download_in_pipe[1], cc, 1);
		cc++;
	}
	
	write(download_in_pipe[1], cc, 1);
}


void stop_downloading_map()
{
	uint8_t m = DOWNLOAD_THREAD_IN_STOP_DOWNLOADING;
	write(download_in_pipe[1], &m, 1);
}


void init_download()
{
	pipe(download_in_pipe);
	pipe(download_out_pipe);
	pthread_create(&download_thread_id, NULL, download_thread, NULL);
}


void kill_download()
{
	uint8_t m = DOWNLOAD_THREAD_IN_SHUTDOWN;
	write(download_in_pipe[1], &m, 1);
	pthread_join(download_thread_id, NULL);
	close(download_in_pipe[0]);
	close(download_in_pipe[1]);
	close(download_out_pipe[0]);
	close(download_out_pipe[1]);
}


void process_download_out_pipe()
{
	uint8_t msg;
	
	read(download_out_pipe[0], &msg, 1);

	switch(msg)
	{
	case DOWNLOAD_THREAD_OUT_DOWNLOAD_COMPLETED:
		game_process_map_downloaded();
		break;
	
	case DOWNLOAD_THREAD_OUT_DOWNLOAD_FAILED:
		game_process_map_download_failed();
		break;
	}
}
