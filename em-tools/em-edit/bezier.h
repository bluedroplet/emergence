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
