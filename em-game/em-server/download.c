#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../common/user.h"
#include "../common/resource.h"
#include "../common/llist.h"
#include "shared/network.h"
#include "main.h"

struct download_t
{
	struct string_t *filename;
	int net_fd, file_fd;
	off_t size, offset;
	
	struct download_t *next;
		
} *download0 = NULL;

int download_kill_pipe[2];
pthread_t download_thread_id;


int start_download(struct download_t *download)
{
	struct stat buf;
			
	struct string_t *filename = new_string_string(emergence_home_dir);
	string_cat_text(filename, "/maps/");
	string_cat_string(filename, download->filename);
	string_cat_text(filename, ".cmap");
	
	if(stat(filename->text, &buf) == -1)
	{
		free_string(filename);
		filename = new_string_text("stock-maps/");
		string_cat_string(filename, download->filename);
		string_cat_text(filename, ".cmap");
	
		if(stat(find_resource(filename->text), &buf) == -1)
		{
			int i = 0;
			write(download->net_fd, &i, 4);
			return 0;
		}
	}

	download->file_fd = open(filename->text, O_RDONLY | O_NONBLOCK | O_NOFOLLOW);
	free_string(filename);
	
	if(download->file_fd == -1)
	{
		int i = 0;
		write(download->net_fd, &i, 4);
		return 0;
	}
	
	download->size = buf.st_size;
	download->offset = 0;
	
	send(download->net_fd, &download->size, 4, 0);
	
	sendfile(download->net_fd, download->file_fd, 
		&download->offset, download->size - download->offset);
	
	return 1;
}


void remove_download(struct download_t *download)
{
	free_string(download->filename);
	close(download->net_fd);
	close(download->file_fd);
	LL_REMOVE(struct download_t, &download0, download);
}


void *download_thread(void *a)
{
	int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0)
		server_libc_error("socket failure");

	struct sockaddr_in name;
	name.sin_family = AF_INET;
	name.sin_port = htons(EMNET_PORT);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listen_fd, (struct sockaddr *) &name, sizeof (name)) < 0)
		server_libc_error("bind failure");

	if(listen(listen_fd, 1) < 0)
		server_libc_error("listen failure");
	
	fcntl(listen_fd, F_SETFL, O_NONBLOCK);
	
	int fdcount = 2;
	
	while(1)
	{
		struct pollfd *fds;
		int cfd;
		struct download_t *cdownload, *temp;
			
		fds = calloc(sizeof(struct pollfd), fdcount);
			
		fds[0].fd = download_kill_pipe[0];
		fds[0].events = POLLIN;
		
		fds[1].fd = listen_fd;
		fds[1].events = POLLIN;
			
		cdownload = download0;
		cfd = 2;

		while(cdownload)		
		{
			fds[cfd].fd = cdownload->net_fd;
			
			if(cdownload->file_fd == -1)
				fds[cfd].events = POLLIN;
			else
				fds[cfd].events = POLLOUT;
			
			LL_NEXT(cdownload);
			cfd++;
		}
		
		if(poll(fds, fdcount, -1) == -1)
		{
			if(errno == EINTR)	// why is this necessary?
				continue;
			
			return NULL;
		}
		
		if(fds[0].revents & POLLIN)
		{
			cdownload = download0;
			while(cdownload)
			{
				close(cdownload->net_fd);
				close(cdownload->file_fd);
				LL_NEXT(cdownload);
			}
			
			LL_REMOVE_ALL(struct download_t, &download0);
			free(fds);
			pthread_exit(NULL);
		}
		
		if(fds[1].revents & POLLIN)
		{
			struct download_t download;
				
			download.net_fd = accept(listen_fd, NULL, NULL);
			
			if(download.net_fd != -1)
			{
				fcntl(download.net_fd, F_SETFL, O_NONBLOCK);
				int i = 1;
				setsockopt(download.net_fd, SOL_TCP, TCP_CORK, &i, 4);
				download.file_fd = -1;
				download.filename = new_string();
				
				LL_ADD(struct download_t, &download0, &download);
				fdcount++;
				continue;
			}
		}
		
		cdownload = download0;
		cfd = 2;
		while(cdownload)		
		{
			if(fds[cfd].revents & (POLLERR | POLLHUP))
			{
				temp = cdownload->next;
				remove_download(cdownload);
				cdownload = temp;
				cfd++;
				continue;
			}
			
			if(cdownload->file_fd == -1)
			{
				if(fds[cfd].revents & POLLIN)
				{
					char c;
					if(recv(cdownload->net_fd, &c, 1, 0) != -1)
					{
						if(c != 0)
						{
							string_cat_char(cdownload->filename, c);
						}
						else
						{
							if(!start_download(cdownload))
							{
								temp = cdownload->next;
								remove_download(cdownload);
								cdownload = temp;
								cfd++;
								continue;
							}
						}
					}
				}
			}
			else
			{
				if(fds[cfd].revents & POLLOUT)
				{
					sendfile(cdownload->net_fd, cdownload->file_fd, 
						&cdownload->offset, cdownload->size - cdownload->offset);
					
					if(cdownload->offset == cdownload->size)
					{
						printf("%i\n", cdownload->size);
						temp = cdownload->next;
						remove_download(cdownload);
						cdownload = temp;
						cfd++;
						continue;
					}
				}
			}
			
			LL_NEXT(cdownload);
			cfd++;
		}
	}
	
	return NULL;
}


void init_download()
{
	pipe(download_kill_pipe);
	pthread_create(&download_thread_id, NULL, download_thread, NULL);
}


void kill_download()
{
	char c;
	write(download_kill_pipe[1], &c, 1);
	pthread_join(download_thread_id, NULL);
	close(download_kill_pipe[0]);
	close(download_kill_pipe[1]);
}
