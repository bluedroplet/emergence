HANDLE create_thread_pipe();
void thread_pipe_send(HANDLE pipe_handle, buffer *buf);
buffer *thread_pipe_recv(HANDLE pipe_handle);
void init_thread_pipes();
void kill_thread_pipes();
