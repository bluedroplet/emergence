/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vertex.h"
#include "polygon.h"
#include "llist.h"

#define dotproduct(v1, v2) ((v1).x * (v2).x + (v1).y * (v2).y)


void poly_line_clip(struct polygon_t *pin, struct vertex_t *v1, struct vertex_t *v2) // must be in c/w order
{
	struct polygon_t pout;

	if(pin->numverts < 3)
		return;

	struct vertex_t normal;

	double a = v2->y - v1->y;
	double b = v2->x - v1->x;

	double l = hypot(a, b);

	a /= l;
	b /= l;

	normal.x = -a;
	normal.y = b;


	double distance = dotproduct((*v1), normal);

	double curdot = dotproduct(pin->vertex[0], normal);
	int curin = (curdot <= distance);
	
	pout.numverts = 0;

	int k;

	for(k = 0; k != pin->numverts; k++)
	{
		int l = (k + 1) % pin->numverts;

		if(curin)
			pout.vertex[pout.numverts++] = pin->vertex[k];

		double nextdot = dotproduct(pin->vertex[l], normal);
		int nextin = (nextdot <= distance);

		if(curin != nextin)
		{
			double scale = (distance - curdot) / (nextdot - curdot);

			pout.vertex[pout.numverts].x = (float)(pin->vertex[k].x + 
				((pin->vertex[l].x - pin->vertex[k].x) * scale));
			pout.vertex[pout.numverts++].y = (float)(pin->vertex[k].y + 
				((pin->vertex[l].y - pin->vertex[k].y) * scale));
		}

		curdot = nextdot;
		curin = nextin;
	}

	*pin = pout;
}	



void poly_clip(struct polygon_t *pin, const struct polygon_t *pclip) // must be in c/w order
{
	struct polygon_t pout;
	int i, j, k, l;

	for(i = 0; i != pclip->numverts; i++)
	{
		if(pin->numverts < 3)
			return;		// early out once the source poly has been completely clipped

		if(i == pclip->numverts - 1)
			j = 0;
		else
			j = i + 1;

		struct vertex_t normal;

		double a = pclip->vertex[j].y - pclip->vertex[i].y;
		double b = pclip->vertex[j].x - pclip->vertex[i].x;

		double len = hypot(a, b);

		a /= len;
		b /= len;

		normal.x = -a;
		normal.y = b;


		double distance = dotproduct(pclip->vertex[i], normal);

		double curdot = dotproduct(pin->vertex[0], normal);
		int curin = (curdot <= distance);
		
		pout.numverts = 0;

		for(k = 0; k != pin->numverts; k++)
		{
			if(k == pin->numverts - 1)
				l = 0;
			else
				l = k + 1;
			
			if(curin)
				pout.vertex[pout.numverts++] = pin->vertex[k];

			double nextdot = dotproduct(pin->vertex[l], normal);
			int nextin = (nextdot <= distance);

			if(curin != nextin)
			{
				double scale = (distance - curdot) / (nextdot - curdot);

				pout.vertex[pout.numverts].x = (float)(pin->vertex[k].x + 
					((pin->vertex[l].x - pin->vertex[k].x) * scale));
				pout.vertex[pout.numverts++].y = (float)(pin->vertex[k].y + 
					((pin->vertex[l].y - pin->vertex[k].y) * scale));
			}

			curdot = nextdot;
			curin = nextin;
		}

		*pin = pout;
	}
}


double poly_area(const struct polygon_t *poly) // must be in c/w order
{
	if(poly->numverts < 3)
		return 0.0f;

	double area = 0.0f;
	int i, j;
	
	for(i = 0; i != poly->numverts; i++)
	{
		if(i == poly->numverts - 1)
			j = 0;
		else
			j = i + 1;

		area += poly->vertex[j].x * poly->vertex[i].y;
		area -= poly->vertex[i].x * poly->vertex[j].y;
	}

	return area / 2.0f;
}


double poly_clip_area(struct polygon_t *pin, const struct polygon_t *pclip) // must be in c/w order
{
	double area = 0.0f;
	
	struct polygon_t pout;
	int i, j, k, l;

	for(i = 0; i != pclip->numverts; i++)
	{
		if(pin->numverts < 3)
			return 0.0;		// early out once the source poly has been completely clipped

		if(i == pclip->numverts - 1)
			j = 0;
		else
			j = i + 1;

		struct vertex_t normal;

		double a = pclip->vertex[j].y - pclip->vertex[i].y;
		double b = pclip->vertex[j].x - pclip->vertex[i].x;

		double len = hypot(a, b);

		a /= len;
		b /= len;

		normal.x = -a;
		normal.y = b;


		double distance = dotproduct(pclip->vertex[i], normal);

		double curdot = dotproduct(pin->vertex[0], normal);
		int curin = (curdot <= distance);
		
		pout.numverts = 0;

		for(k = 0; k != pin->numverts; k++)
		{
			if(k == pin->numverts - 1)
				l = 0;
			else
				l = k + 1;
			
			if(curin)
				pout.vertex[pout.numverts++] = pin->vertex[k];

			double nextdot = dotproduct(pin->vertex[l], normal);
			int nextin = (nextdot <= distance);

			if(curin != nextin)
			{
				double scale = (distance - curdot) / (nextdot - curdot);

				pout.vertex[pout.numverts].x = (float)(pin->vertex[k].x + 
					((pin->vertex[l].x - pin->vertex[k].x) * scale));
				pout.vertex[pout.numverts++].y = (float)(pin->vertex[k].y + 
					((pin->vertex[l].y - pin->vertex[k].y) * scale));
			}

			curdot = nextdot;
			curin = nextin;
		}

		*pin = pout;
	}
	
	for(i = 0; i != pin->numverts; i++)
	{
		if(i == pin->numverts - 1)
			j = 0;
		else
			j = i + 1;

		area += pin->vertex[j].x * pin->vertex[i].y;
		area -= pin->vertex[i].x * pin->vertex[j].y;
	}
	
	return area / 2.0f;
}




/*
void poly_arb_clip(struct polygon_t *pin, const struct polygon_t *pclip) // must be in c/w order
{
	struct polygon_t pout;
	int i, j, k, l;

	for(i = 0; i != pclip->numverts; i++)
	{
		if(pin->numverts == 0)
			return;		// early out once the source poly has been completely clipped

		if(i == pclip->numverts - 1)
			j = 0;
		else
			j = i + 1;

		struct vertex_t normal;

		double a = pclip->vertex[j].y - pclip->vertex[i].y;
		double b = pclip->vertex[j].x - pclip->vertex[i].x;

		double len = hypot(a, b);

		a /= len;
		b /= len;

		normal.x = -a;
		normal.y = b;


		double distance = dotproduct(pclip->vertex[i], normal);

		double curdot = dotproduct(pin->vertex[0], normal);
		int curin = (curdot <= distance);
		
		pout.numverts = 0;

		for(k = 0; k != pin->numverts; k++)
		{
			if(k == pin->numverts - 1)
				l = 0;
			else
				l = k + 1;
			
			if(curin)
				pout.vertex[pout.numverts++] = pin->vertex[k];

			double nextdot = dotproduct(pin->vertex[l], normal);
			int nextin = (nextdot <= distance);

			if(curin != nextin)
			{
				double scale = (distance - curdot) / (nextdot - curdot);

				pout.vertex[pout.numverts].x = (float)(pin->vertex[k].x + 
					((pin->vertex[l].x - pin->vertex[k].x) * scale));
				pout.vertex[pout.numverts++].y = (float)(pin->vertex[k].y + 
					((pin->vertex[l].y - pin->vertex[k].y) * scale));
			}

			curdot = nextdot;
			curin = nextin;
		}

		*pin = pout;
	}
}


double poly_arb_area(const struct polygon_t *poly) // must be in c/w order
{
	if(poly->numverts == 0)
		return 0.0f;

	double area = 0.0f;
	int i, j;
	
	for(i = 0; i != poly->numverts; i++)
	{
		if(i == poly->numverts - 1)
			j = 0;
		else
			j = i + 1;

		area += poly->vertex[j].x * poly->vertex[i].y;
		area -= poly->vertex[i].x * poly->vertex[j].y;
	}

	return area / 2.0f;
}

*/


void poly_arb_line_clip(struct vertex_ll_t **pin, struct vertex_t *v1, struct vertex_t *v2) // must be in c/w order
{
	struct vertex_ll_t *pout = NULL;

	if(!*pin)
		return;

	double a = v2->y - v1->y;
	double b = v2->x - v1->x;

	double l = hypot(a, b);

	struct vertex_t normal = {-a / l, b / l};
		
	double distance = dotproduct((*v1), normal);

	struct vertex_ll_t *cvert = *pin;
	double curdot = dotproduct((*cvert), normal);
	int curin = (curdot <= distance);

	while(cvert)
	{
		struct vertex_ll_t *nextvert;
			
		if(cvert->next)
			nextvert = cvert->next;
		else
			nextvert = *pin;
		
		if(curin)
			LL_ADD(struct vertex_ll_t, &pout, cvert);

		double nextdot;
			
		nextdot = dotproduct((*nextvert), normal);
			
		int nextin = (nextdot <= distance);

		if(curin != nextin)
		{
			double scale = (distance - curdot) / (nextdot - curdot);

			struct vertex_ll_t vertex;
			
			vertex.x = (float)(cvert->x + ((nextvert->x - cvert->x) * scale));
			vertex.y = (float)(cvert->y + ((nextvert->y - cvert->y) * scale));
			
			LL_ADD(struct vertex_ll_t, &pout, &vertex);
		}

		curdot = nextdot;
		curin = nextin;
		
		cvert = cvert->next;
	}

	struct vertex_ll_t **temp = pin;
	LL_REMOVE_ALL(struct vertex_ll_t, pin);
	*temp = pout;
}


double poly_arb_clip_area(struct vertex_ll_t **pin, struct vertex_ll_t *pclip) // must be in c/w order
{
	double area = 0.0;
	
	struct vertex_ll_t *pout = NULL;
	struct vertex_ll_t *clipvert = pclip;
	
	while(clipvert)
	{
		struct vertex_ll_t *nextclipvert;
			
		if(clipvert->next)
			nextclipvert = clipvert->next;
		else
			nextclipvert = pclip;
		
		if(!*pin)
			return 0.0;
	
		double a = nextclipvert->y - clipvert->y;
		double b = nextclipvert->x - clipvert->x;
	
		double l = hypot(a, b);
	
		struct vertex_t normal = {-a / l, b / l};
			
		double distance = dotproduct((*clipvert), normal);
	
		struct vertex_ll_t *cvert = *pin;
		double curdot = dotproduct((*cvert), normal);
		int curin = (curdot <= distance);
	
		while(cvert)
		{
			struct vertex_ll_t *nextvert;
				
			if(cvert->next)
				nextvert = cvert->next;
			else
				nextvert = *pin;
			
			if(curin)
				LL_ADD(struct vertex_ll_t, &pout, cvert);
	
			double nextdot;
				
			nextdot = dotproduct((*nextvert), normal);
				
			int nextin = (nextdot <= distance);
	
			if(curin != nextin)
			{
				double scale = (distance - curdot) / (nextdot - curdot);
	
				struct vertex_ll_t vertex;
				
				vertex.x = (float)(cvert->x + ((nextvert->x - cvert->x) * scale));
				vertex.y = (float)(cvert->y + ((nextvert->y - cvert->y) * scale));
				
				LL_ADD(struct vertex_ll_t, &pout, &vertex);
			}
	
			curdot = nextdot;
			curin = nextin;
			
			cvert = cvert->next;
		}
		
		struct vertex_ll_t **temp = pin;
		LL_REMOVE_ALL(struct vertex_ll_t, pin);
		*temp = pout;
		pout = NULL;
		clipvert = clipvert->next;
	}

	struct vertex_ll_t *cvert = *pin;

	while(cvert)
	{
		struct vertex_ll_t *nextvert;
			
		if(cvert->next)
			nextvert = cvert->next;
		else
			nextvert = *pin;
		
		area += nextvert->x * cvert->y;
		area -= cvert->x * nextvert->y;
		
		cvert = cvert->next;
	}
	
	return area / 2.0f;
}
