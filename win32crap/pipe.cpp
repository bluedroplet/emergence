#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../../common/string.h"
#include "../../common/buffer.h"


struct pipe_t
{
	HANDLE pipe_handle;
	buffer buf;

	pipe_t *next;

} *pipe0 = NULL;

CRITICAL_SECTION pipe_critical_section;


HANDLE create_thread_pipe()
{
	pipe_t *cpipe;

	EnterCriticalSection(&pipe_critical_section);

	if(!pipe0)
		cpipe = pipe0 = new pipe_t;
	else
	{
		cpipe = pipe0;

		while(cpipe->next)
			cpipe = cpipe->next;

		cpipe->next = new pipe_t;
		cpipe = cpipe->next;
	}

	cpipe->next = NULL;

	HANDLE pipe_handle = cpipe->pipe_handle = CreateEvent(NULL, 0, 0, NULL);

	LeaveCriticalSection(&pipe_critical_section);

	return pipe_handle;
}


void thread_pipe_send(HANDLE pipe_handle, buffer *buf)
{
	EnterCriticalSection(&pipe_critical_section);

	pipe_t *cpipe = pipe0;

	while(cpipe->pipe_handle != pipe_handle)
	{
		cpipe = cpipe->next;
		if(!cpipe)
			break;
	}

	if(!cpipe)
		return;

	cpipe->buf.cat(buf);

	SetEvent(pipe_handle);

	LeaveCriticalSection(&pipe_critical_section);
}


buffer *thread_pipe_recv(HANDLE pipe_handle)
{
	EnterCriticalSection(&pipe_critical_section);

	pipe_t *cpipe = pipe0;

	while(cpipe->pipe_handle != pipe_handle)
	{
		cpipe = cpipe->next;
		if(!cpipe)
			break;
	}

	if(!cpipe)
		return NULL;

	buffer *buf = new buffer(&cpipe->buf);
	cpipe->buf.clear();

	LeaveCriticalSection(&pipe_critical_section);

	return buf;
}


void init_thread_pipes()
{
	InitializeCriticalSection(&pipe_critical_section);
}


void kill_thread_pipes()
{
	DeleteCriticalSection(&pipe_critical_section);

	pipe_t *cpipe = pipe0, *temp;

	while(cpipe)
	{
		CloseHandle(cpipe->pipe_handle);

		temp = cpipe->next;
		delete cpipe;
		cpipe = temp;
	}
}


