#define _GNU_SOURCE
#define _REENTRANT

#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>

#include "../../common/types.h"
#include "rdtsc.h"

#include "../console.h"


uint64_t start_count, counts_per_second;


uint32_t get_tick()
{
	uint64_t count = rdtsc();
	
	return (uint32_t)(((count - start_count) * 200) / counts_per_second);
}


double get_double_time()
{
	uint64_t count = rdtsc();
	
	return (double)count / (double)counts_per_second;
}


void init_timer()
{
	double mhz;
	FILE *file = popen("grep \"cpu MHz\" /proc/cpuinfo", "r");
	fscanf(file, "%*s%*s%*s%lf", &mhz);
	pclose(file);
	
	counts_per_second = (uint64_t)(mhz * 1000000.0);
	
	console_print("Timer resolution: %u Hz\n", (uint32_t)counts_per_second);
	
	start_count = rdtsc();
}


void reset_start_count()
{
	start_count = rdtsc();
}
