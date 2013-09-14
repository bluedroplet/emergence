/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

struct rotate_t
{
	struct surface_t *in_surface, **out_surface;
	float red, green, blue, alpha;
	
	struct rotate_t *next;
};

void do_rotate(struct rotate_t *rotate0, int out_width, int out_height, int frames);
