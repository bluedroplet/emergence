
struct bspnode_t
{
	double x1, y1, x2, y2;
	double tstart, tend;
	double dtstart, dtend;

	struct bspnode_t *front, *back;
};

#ifdef _ZLIB_H
int load_bsp_tree(gzFile file);
#endif
struct bspnode_t *circle_walk_bsp_tree(double xdis, double ydis, double r, double *t_out);
struct bspnode_t *line_walk_bsp_tree(double x1, double y1, double x2, double y2);

extern struct bspnode_t *bspnode0;
