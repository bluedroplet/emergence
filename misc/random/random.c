


#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main()
{
	uint8_t *data = malloc(3008290);
	FILE *file = fopen("random.data", "r");
	fread(data, 300829, 10, file);
	fclose(file);
	
	int a, b, c = 0, dups = 0;
	
	for(a = 1; a < 300829; a++)
	{
		if(++c == 1000)
		{
			printf("%i\n", a);
			c = 0;
		}
		
		for(b = 0; b < a; b++)
		{
			if(!memcmp(&data[b * 10], 
				&data[a * 10], 10))
			{
				dups++;
				printf("dup\n");
			}
		}
	}
	
	free(data);
	printf("dups: %i\n", dups);
	printf("OK\n");
	return 0;
}
