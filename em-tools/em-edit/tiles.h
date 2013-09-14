/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _INC_TILES
#define _INC_TILES


struct tile_t
{
	int x1, y1, x2, y2;		// should this be in blocks ??

	struct surface_t *surface;		// null if not yet rendered
	struct surface_t *scaled_surface;

	struct conn_pointer_t *connp0;		// list of conns that are occluded by this tile
	struct node_pointer_t *nodep0;		// list of node textures that are occluded by this tile
	struct fill_pointer_t *fillp0;		// list of fills that are occluded by this tile
		
	struct tile_t *next;

};


#ifdef _INC_NODES
int tile_node(struct node_t *node);
void remove_node_from_tiles(struct node_t *node);
#endif

#ifdef _INC_CONNS
int tile_conn(struct conn_t *conn);
int conn_fully_rendered(struct conn_t *conn);
void remove_conn_from_tiles(struct conn_t *conn);
void delete_images_of_tiles_with_conn(struct conn_t *conn);
#endif


#ifdef _INC_FILLS
int tile_fill(struct fill_t *fill);
void remove_fill_from_tiles(struct fill_t *fill);
#endif


void collate_all_tiles();

extern struct tile_t *clean_tile0;
void mark_conns_with_tile_as_untiled(struct tile_t *tile);

void draw_tiles();
void draw_boxes();
void make_clean_tiles();
void make_dirty_tiles();
void delete_all_tiles();
void invalidate_all_scaled_tiles();
int check_for_unscaled_tiles();

int count_tiles();

void scale_tiles();
int finished_scaling_tiles();

int check_for_tiling();
void tile();
void finished_tiling();


int check_for_unrendered_tile();
void render_tile();
int finished_rendering_tile();
extern int tile_collation_pending;

#endif	// _INC_TILES
