


#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main()
{
	uint8_t data[11];
	FILE *file = fopen("random.data", "r");
	
	char chars[32] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
	
	int k;
	
	for(k = 0; k < 300829; k++)
	{
		printf("\"");
		
		fread(data, 1, 10, file);
		
		int cB, cc = 0, cb = 0;
		
		for(cB = 0; cB < 16; cB++)
		{
			uint8_t w[2] = {data[cc+1], data[cc]};
			uint16_t x = *(uint16_t*)w;
			
			x <<= cb;
			x >>= 11;
			
			printf("%c", chars[x]);
			
			cb += 5;
			if(cb > 7)
			{
				cb -= 8;
				cc++;
			}
		}
		
		printf("\", 0, NULL\n");
	}

	fclose(file);
	return 0;
}
