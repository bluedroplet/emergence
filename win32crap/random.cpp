#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "types.h"

dword randn;


void seed_random()
{
	DWORDLONG randseed;
	QueryPerformanceCounter((LARGE_INTEGER*)&randseed);

	randn = (dword)randseed;

/*	string s("Seeded RNG: ");

	s.cat(randn);
	s.cat("\n");

	ConsolePrint(s.text);
*/
}


dword rand_dword()
{
	randn = randn * 1664525 + 1013904223;

	return randn;
}


int rand_int()
{
	randn = randn * 1664525 + 1013904223;

	return *(int*)&randn;
}


float rand_float()
{
	randn = randn * 1664525 + 1013904223;

	return ((float)randn) / 4294967295.0f;
}
