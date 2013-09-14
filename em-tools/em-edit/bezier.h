/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

struct bezier_t
{
	float x1, y1, x2, y2, x3, y3, x4, y4;
};

void BRZ(struct bezier_t *bezier, float t, float *x, float *y);
void deltaBRZ(struct bezier_t *bezier, float t, float *x, float *y);
int generate_bezier_ts(struct bezier_t *in_bezier, 
	struct t_t **out_t0, int *out_count, float *out_length);
void generate_bezier_bigts(struct bezier_t *in_bezier, 
	struct t_t **out_t0, int *out_count, float *out_length);
