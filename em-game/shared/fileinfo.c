#define _GNU_SOURCE
#define _REENTRANT

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <openssl/evp.h>

#include "fileinfo.h"

#define FILEINFO_READ_BUF_SIZE 65536


int get_file_info(char *filename, int *size, uint8_t *hash)
{
	int fd;
	struct stat stat_buf;
	const EVP_MD *m;
	EVP_MD_CTX *ctx;
	uint8_t *read_buf;


	fd = open(filename, O_RDONLY | O_NOFOLLOW);
	
	if(fd < 0)
		return 0;
	
	if(fstat(fd, &stat_buf) < 0)
	{
		close(fd);
		return 0;
	}
	
	*size = stat_buf.st_size;
	
	if(!(ctx = EVP_MD_CTX_create()))
	{
		close(fd);
		return 0;
	}
	
	if(!(m = EVP_sha1()))
	{
		
		close(fd);
		EVP_MD_CTX_destroy(ctx);
		return 0;
	}
	
	if(!EVP_DigestInit_ex(ctx, m, NULL))
	{
		close(fd);
		EVP_MD_CTX_destroy(ctx);
		return 0;
	}
	
	read_buf = malloc(FILEINFO_READ_BUF_SIZE);
	
	while(1)
	{
		int b = TEMP_FAILURE_RETRY(read(fd, read_buf, FILEINFO_READ_BUF_SIZE));
		if(b < 0)
		{
			close(fd);
			EVP_MD_CTX_destroy(ctx);
			free(read_buf);
			return 0;
		}
		
		if(b == 0)
			break;
		
		EVP_DigestUpdate(ctx, read_buf, b);
	}
	
	EVP_DigestFinal_ex(ctx, hash, NULL);
	
	close(fd);
	EVP_MD_CTX_destroy(ctx);
	free(read_buf);
	
	return 1;
}


int compare_hashes(uint8_t *hash1, uint8_t *hash2)
{
	if(memcmp(hash1, hash2, FILEINFO_DIGEST_SIZE) == 0)
		return 1;
	else
		return 0;
}
