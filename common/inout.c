/*
	Copyright (C) 1998-2002 Jonathan Brown
	
	This file is part of emergence.
	
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	1.	The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source distribution.
	
	Jonathan Brown
	jbrown@emergence.uk.net
*/


// inside is on right
// there is a better way to do this


#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <math.h>


int inout(float x1, float y1, float x2, float y2, float x, float y)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	
	if(dx == 0.0)	// north or south
	{
		if(dy > 0.0)	// north
		{				
			if(x < x1)
				return 0;
			else
				return 1;
		}
		else			// south
		{
			if(x < x1)
				return 1;
			else
				return 0;
		}
	}
	else if(dy == 0.0)	// east or west
	{
		if(dx > 0.0)	// east
		{
			if(y < y1)
				return 1;
			else
				return 0;
		}
		else			// west
		{
			if(y < y1)
				return 0;
			else
				return 1;
		}
	}
	else	// diagonal
	{
		if(fabs(dx) > fabs(dy))
		{
			double y_inter = y1 + dy * (x - x1) / dx;
			
			if(dy > 0.0)	// north
			{
				if(dx > 0.0)	// north-east
				{
					if(y < y_inter)
						return 1;
					else
						return 0;
				}
				else			// north-west
				{
					if(y < y_inter)
						return 0;
					else
						return 1;
				}
			}
			else			// south
			{
				if(dx > 0.0)	// south-east
				{
					if(y < y_inter)
						return 1;
					else
						return 0;
				}
				else			// south-west
				{
					if(y < y_inter)
						return 0;
					else
						return 1;
				}
			}
		}
		else
		{
			double x_inter = x1 + dx * (y - y1) / dy;
			
			if(dy > 0.0)	// north
			{
				if(dx > 0.0)	// north-east
				{
					if(x < x_inter)
						return 0;
					else
						return 1;
				}
				else			// north-west
				{
					if(x < x_inter)
						return 0;
					else
						return 1;
				}
			}
			else			// south
			{
				if(dx > 0.0)	// south-east
				{
					if(x < x_inter)
						return 1;
					else
						return 0;
				}
				else			// south-west
				{
					if(x < x_inter)
						return 1;
					else
						return 0;
				}
			}
		}
	}			
}
