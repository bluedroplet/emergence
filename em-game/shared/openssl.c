#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <pthread.h>

#include <openssl/crypto.h>

static pthread_mutex_t *mutex_buf = NULL;


static unsigned long id_function()
{
	return (unsigned long)pthread_self();
}


static void locking_function(int mode, int n, const char *file, int line)
{
	if(mode & CRYPTO_LOCK)
		pthread_mutex_lock(&(mutex_buf[n]));
	else
		pthread_mutex_unlock(&(mutex_buf[n]));
}	


int init_openssl()
{
	int i;
	
	mutex_buf = (pthread_mutex_t *)malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	if(!mutex_buf)
		return 0;
	
	for(i = 0; i < CRYPTO_num_locks(); i++)
		pthread_mutex_init(&(mutex_buf[i]), NULL);
	
	CRYPTO_set_id_callback(id_function);
	CRYPTO_set_locking_callback(locking_function);
	
	return 1;
}


void kill_openssl()
{
	int i;
	
	if(!mutex_buf)
		return;
	
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);
	
	for(i = 0; i < CRYPTO_num_locks(); i++)
		pthread_mutex_destroy(&(mutex_buf[i]));
	
	free(mutex_buf);
	mutex_buf = NULL;
	
	return;
}
