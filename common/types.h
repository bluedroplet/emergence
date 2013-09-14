/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

#ifdef LINUX
typedef unsigned long long dwordlong;
#else
typedef unsigned __int64 dwordlong;
#endif


#ifndef NULL
#define NULL 0
#endif
