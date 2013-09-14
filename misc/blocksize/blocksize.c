/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <math.h>

int main()
{
	int i = 0;

	while(1)
	{
		i++;
		printf("%i\n", i);

		double d;

		if(modf((double)i * 0.875, &d) != 0.0)
			continue;
			
		if(modf((double)i * 0.8, &d) != 0.0)
			continue;
			
		if(modf((double)i * 0.72, &d) != 0.0)
			continue;
			
		if(modf((double)i * 0.64, &d) != 0.0)
			continue;
			
		if(modf((double)i * 0.5, &d) != 0.0)
			continue;
		
		if(modf((double)i * 0.4, &d) != 0.0)
			continue;
			
		if(modf((double)i * 0.32, &d) != 0.0)
			continue;
			
		if(modf((double)i * 0.25, &d) != 0.0)
			continue;

		if(modf((double)i * 0.2, &d) != 0.0)
			continue;

		break;
	}
}
