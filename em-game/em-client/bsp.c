#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "../shared/bsp.h"
#include "line.h"
#include "../gsub/gsub.h"

void write_bsp_node(struct bspnode_t *node, FILE *file)
{
	fwrite(&node->x1, 4, 1, file);
	fwrite(&node->y1, 4, 1, file);
	fwrite(&node->x2, 4, 1, file);
	fwrite(&node->y2, 4, 1, file);
	fwrite(&node->tstart, 4, 1, file);
	fwrite(&node->tend, 4, 1, file);
	fwrite(&node->dtstart, 4, 1, file);
	fwrite(&node->dtend, 4, 1, file);
	
	int b = 0;
	
	if(node->front)
		b = 1;
	
	if(node->back)
		b |= 2;
	
	fwrite(&b, 4, 1, file);
	
	if(node->front)
		write_bsp_node(node->front, file);

	if(node->back)
		write_bsp_node(node->back, file);
}


void write_bsp_tree(FILE *file)
{
	write_bsp_node(bspnode0, file);
}


void draw_bsp_node(struct bspnode_t *node)
{
	blit_colour = 0xffff;
	
	draw_world_clipped_line(node->x1 + node->dtstart * (node->x2 - node->x1), node->y1 + node->dtstart * (node->y2 - node->y1), 
		node->x1 + node->dtend * (node->x2 - node->x1), node->y1 + node->dtend * (node->y2 - node->y1));

	if(node->front)
		draw_bsp_node(node->front);

	if(node->back)
		draw_bsp_node(node->back);
}


void draw_bsp_tree()
{
	if(bspnode0)
		draw_bsp_node(bspnode0);
}
