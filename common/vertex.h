/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _INC_VERTEX
#define _INC_VERTEX

struct vertex_t
{
	float x, y;
};

struct vertex_ll_t
{
	float x, y;
	struct vertex_ll_t *next;
};

#endif
