
struct bspnode_t
{
	float x1, y1, x2, y2;
	float tstart, tend;
	float dtstart, dtend;

	struct bspnode_t *front, *back;
};

#ifdef ZLIB_H
int load_bsp_tree(gzFile file);
#endif
struct bspnode_t *circle_walk_bsp_tree(float xdis, float ydis, float r);
struct bspnode_t *line_walk_bsp_tree(float x1, float y1, float x2, float y2);
void rail_walk_bsp(float x1, float y1, float x2, float y2, float *out_x, float *out_y);

extern struct bspnode_t *bspnode0;
