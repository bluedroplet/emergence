/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

struct polygon_t
{
	int numverts;
	struct vertex_t vertex[8];
};

void poly_line_clip(struct polygon_t *pin, struct vertex_t *v1, struct vertex_t *v2); // must be in c/w order
void poly_clip(struct polygon_t *pin, const struct polygon_t *pclip); // must be in c/w order
double poly_area(const struct polygon_t *poly); // must be in c/w order
double poly_clip_area(struct polygon_t *pin, const struct polygon_t *pclip); // must be in c/w order

void poly_arb_line_clip(struct vertex_ll_t **pin, struct vertex_t *v1, struct vertex_t *v2); // must be in c/w order
void poly_arb_clip(struct vertex_ll_t *pin, const struct vertex_ll_t *pclip); // must be in c/w order
double poly_arb_area(const struct vertex_ll_t *poly); // must be in c/w order
double poly_arb_clip_area(struct vertex_ll_t **pin, struct vertex_ll_t *pclip); // must be in c/w order
