/* 
	Copyright (C) 1998-2002 Jonathan Brown

    This file is part of em-tools.

    em-tools is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    em-tools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with em-tools; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Jonathan Brown
	jbrown@emergence.uk.net
*/


#ifndef _INC_CONNS
#define _INC_CONNS


#define CONN_TYPE_STRAIGHT	0
#define CONN_TYPE_CONIC		1
#define CONN_TYPE_BEZIER	2

struct t_t
{
	float t1, t2;
	float x1, y1, x2, y2;
	float dist;	// distance between (x1, y1) and (x2, y2)
	
	float deltax1, deltay1;
	float deltax2, deltay2;

	struct t_t *next;
};


struct conn_t
{
	uint32_t index;	// for saving

	uint8_t orientation;
	uint8_t type;

	struct node_t *node1, *node2;
	uint8_t sat1, sat2;

	struct t_t *t0;
	struct t_t *bigt0;
		
	uint32_t t_count;
	float t_length;
	
	uint32_t bigt_count;
	float bigt_length;
	
	struct vertex_t *verts;		// calculated by generate_verticies

	struct surface_t *squished_texture;

	uint8_t tiled;
	uint8_t fully_rendered;

	struct conn_t *next;
};


struct conn_pointer_t
{
	struct conn_t *conn;
	struct conn_pointer_t *next;
};


int insert_straight_conn(struct node_t *node1, struct node_t *node2);
int insert_conic_conn(struct node_t *node1, struct node_t *node2);
int insert_bezier_conn(struct node_t *node1, struct node_t *node2);
void update_fully_rendered_conns();

extern struct conn_t *conn0;

struct conn_t *get_conn_from_sat(struct node_t *node, int sat);
void delete_conn(struct conn_t *conn);
struct conn_t *get_conn_from_node(struct node_t *node);
void draw_conn(struct conn_t *conn);
void get_conn_limits(struct conn_t *conn, float *minx, float *maxx, float *miny, float *maxy);
int add_conn_pointer(struct conn_pointer_t **connp0, struct conn_t *conn);
void delete_all_conns();
void remove_conn_pointer(struct conn_pointer_t **connp0, struct conn_t *conn);
void get_width_sats(struct conn_t *conn, int *left1, int *right1, int *left2, int *right2);
int node_in_straight_conn(struct node_t *node);

void generate_t_values_for_conns_with_node(struct node_t *node);
void generate_verticies_for_all_conns();
int generate_verticies(struct conn_t *conn);
void generate_squished_textures_for_all_conns();
void delete_conns(struct node_t *node);
void invalidate_conns_with_node(struct node_t *node);
void fix_conic_conns_from_node(struct node_t *node);
void finished_tiling_all_conns();

int tile_all_conns();
int generate_t_values(struct conn_t *conn);
void generate_bigt_values(struct conn_t *conn);

int conn_in_connp_list(struct conn_pointer_t *connp0, struct conn_t *conn);


int check_for_untiled_conns();
int check_for_unsquished_conns();
int check_for_unverticied_conns();


#if defined _ZLIB_H
void gzwrite_conns(gzFile file);
int gzread_conns(gzFile file);
void gzwrite_conn_pointer_list(gzFile file, struct conn_pointer_t *connp0);
struct conn_pointer_t *gzread_conn_pointer_list(gzFile file);
#endif

#endif	// _INC_CONNS
