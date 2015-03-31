/*
 * Copyright 1990, 1991, 1992, Brown University, Providence, RI
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef X_THREE_D

#include "threedee/xthreed.h"
#include "threedee/xpictsortp.h"

#ifndef lint
static char Version[] =
   "$Id: xpictsort.c,v 1.10 1992/06/22 17:11:41 mas Exp $";
#endif

static PictCommPtr bsp_choose();
static int bsp_compute_norm();
static int bsp_compare();
static void bsp_flatten();
static void bsp_split_segment();
static void bsp_split_triangle();
static PictCommPtr bsp_stuff_tree();
static PictCommPtr bsp_new_plane();
static PictCommPtr bsp_make_tree();

static Real Epsilon;

int bsp_pict_num_verts[] = {1, 2, 3, 1, 1, 0};

/* Picks a triangle at random from the list passed and removes it from the */
/* list, returning the list. */

static PictCommPtr
bsp_choose(polies, num_polies)
   PictCommPtr polies;
   int num_polies;
{
   extern long random();
   PictCommPtr prev = polies;
   PictCommPtr curr = polies->front;
   int lim = random() % num_polies-1;
   int i;

   for(i=0;i<lim;i++, prev = curr, curr = curr->front);
   prev->front = curr->front;
   curr->front = NULL;
   return curr;
}


/* Computes the normal vector to a triangle to be ordered.  If the triangle */
/* we're looking at has problems (ie. a zero-length normal, or close */
/* enough), we return a coordinate to "zap" that is duplicated.  Otherwise */
/* we return false.  Hopefully we'll be smart enough not to split on */
/* polygons this function returns an index for. */

static int bsp_compute_norm(poly, area)
   PictCommPtr poly;
   Real *area;

{
   Real first[3];
   Real second[3];

   BSP_SUBTRACT_VECTORS(poly->vertex[1], poly->vertex[0], first);
   BSP_SUBTRACT_VECTORS(poly->vertex[2], poly->vertex[1], second);

   poly->info.poly.norm[0] = first[1]*second[2] - first[2]*second[1];
   poly->info.poly.norm[1] = second[0]*first[2] - first[0]*second[2];
   poly->info.poly.norm[2] = first[0]*second[1] - first[1]*second[0];

   poly->info.poly.dist = BSP_PLANE_DIST(poly->info.poly.norm, 0.0,
					 poly->vertex[1]); 

   *area = poly->info.poly.norm[0]*poly->info.poly.norm[0] +
      poly->info.poly.norm[1]*poly->info.poly.norm[1] +
	 poly->info.poly.norm[2]*poly->info.poly.norm[2];

   if (BSP_ZERO(poly->info.poly.norm))
   {
      if (BSP_ZERO(first))
	 return FIRST_BAD;
      if (BSP_ZERO(second))
	 return SECOND_BAD;
      return THIRD_BAD;
   }

   return SAFE;
}


/* Takes an element to go into the bsp tree and computes the distance */
/* between each of its vertices and the plane defined by the passed plane. */
/* It returns a code indicatinq whether all the vertices of the polygon */
/* are above the plane (or close enough), all the vertices of the polygon */
/* are below the plane (or close enough), or the polygon will have to be */
/* split. */
 
static int bsp_compare(plane, poly, dists)
   PictCommPtr plane, poly;
   Real *dists;

{
   Real d;
   int i;
   int above, below;

   above = below = 0;

   for(i=0; i < bsp_pict_num_verts[poly->type]; i++)
   {
      d = dists[i] = BSP_PLANE_DIST(plane->info.poly.norm,
				    plane->info.poly.dist, 
				    poly->vertex[i]);
      if (d > Epsilon)
	 (above)++;
      else if (d < - Epsilon)
	 (below)++;
   }
   
   if (!(below) && !(above))
      return DONT_CARE;
   if (!(below))
      return THIS_SIDE;
   if (!(above))
      return THAT_SIDE;
   return MUST_SPLIT;
}


/* This function is used to create balanced trees out of the linear list of */
/* lines and points left around when the polygons have all been taken care */
/* of in bsp_make_tree. */

static void bsp_flatten(tree, list)
   PictCommPtr tree;
   PictCommPtr list;

{
   PictCommPtr curr_node, curr_list, temp, prev;

   for(curr_node = tree, curr_list  = list;
       curr_node != NULL;
       curr_node = temp)
   {
      temp = curr_node->front;
      curr_node->front = curr_list;
      if (NULL != curr_list)
      {
	 prev = curr_list;
	 curr_list = curr_list->front;
      }
      curr_node->back = curr_list;
      if (NULL != curr_list)
      {
	 prev = curr_list;
	 curr_list = curr_list->front;
      }
   }
   
   if (curr_list != NULL)
   {
      prev->front = NULL;
      bsp_flatten(list, curr_list);
   }
   else
      for(curr_list = list; curr_list != NULL; curr_list = temp)
      {
	 temp = curr_list->front;
	 curr_list->front = NULL;
      }
}


/* Takes a segment known to be split by a plane and that plane, along with */
/* two lists to add the new segments to, and the distance information about */
/* the endpoints of the segment computed in the bsp_compare function, and */
/* does what you'd think.  It creates two subsegments of the passed */
/* segment-- the parts above and below the plane.  The part above the plane */
/* is added to the list pointed to by front; the part below the plane is */
/* added to the part of the list pointed to by back. */

static void bsp_split_segment(pict, seg, front, back, dists)
   Pict pict;
   PictCommPtr seg;
   PictCommPtr *front, *back;
   Real *dists;
{
   int i;
   int dim = seg->source->width == 2 ? 3 : seg->source->width;
   PictCommPtr temp;
   Real *vert;
   Real ratio = dists[0] / (dists[0] - dists[1]);

   ME(bsp_split_segment);

   /* Get the memory we need. */

   vert = pict_new_vertex(pict, dim);
   ALLOC_COMMAND(temp);

   /* Fill in data and link into lists.	*/

   *temp = *seg;

   temp->front = *front;
   *front = temp;

   seg->front = *back;
   (*back) = seg;

   /* Compute the intersection point and adjust endpoints */

   for (i = 0; i < dim; i++)
      vert[i] = seg->vertex[0][i] + 
	 ratio * (seg->vertex[1][i] - seg->vertex[0][i]);

   if (dists[0] > 0)
      (*front)->vertex[1] = (*back)->vertex[0] = vert;
   else
      (*back)->vertex[1] = (*front)->vertex[0] = vert;

   pict->numlines++;

}


/* This is another function that does basically nothing surprising.  It */
/* takes the plane passed and splits the polygon into either two or three */
/* triangles none of which is cut by the plane.  Those triangles that are */
/* above the plane are added onto the list specified by front; those below */
/* it are added to the back list.  Dists is the vertex location information */
/* calculated in the comparison function. */
 
static void bsp_split_triangle(pict, poly, front, back, dists)
   Pict pict;
   PictCommPtr poly;
   PictCommPtr *front, *back;
   Real *dists;

{
   int i, k, next;
   int dim = poly->source->width == 2 ? 3 : poly->source->width;
   Real ratio;
   int act, bct, edge_ct;
   Real *above[4];
   Real *below[4];
   char aedge[4], bedge[4];
   PictCommPtr temp;
   
   ME(bsp_split_triangle);

   for(i=0;i<4;i++)
      aedge[i] = bedge[i] = -1;
   for(edge_ct=0, i=0; i<3; i++)
      if (PICT_ALWAYS_DRAW == (PICT_ALWAYS_DRAW & poly->edge[i]))
	 edge_ct--;

   /* Put the old and new points on to 2 new polygons */
   for(i=0, act=0, bct=0; i<3; i++)
   {
      next = (i+1)%3;
      if (dists[i] > Epsilon)
      {
	 aedge[act] = poly->edge[i];
	 aedge[(act+3)%4] = poly->edge[(i+2)%3];
	 above[act++] = poly->vertex[i];
      }
      else if (dists[i] < -Epsilon)
      {
	 bedge[bct] = poly->edge[i];
	 bedge[(bct+3)%4] = poly->edge[(i+2)%3];
	 below[bct++] = poly->vertex[i];
      }
      else 
      {
	 above[act++] = poly->vertex[i];
	 below[bct++] = poly->vertex[i];
      }
      if ((dists[i] > Epsilon && dists[next] < -Epsilon) ||
	  (dists[i] < -Epsilon && dists[next] > Epsilon))
      {
	 above[act] = below[bct] = pict_new_vertex(pict, dim);
	 ratio = dists[i] / (dists[i] - dists[next]);
	 for(k=0;k<dim;k++)
	    above[act][k] = 
	       ratio * (poly->vertex[next][k] - poly->vertex[i][k])  +
		  poly->vertex[i][k];
	 act++; bct++;
      }
   }

   /* Fill in all the information for command structures
      on the polygons we've just created		*/

   ALLOC_COMMAND(temp);
   temp->type = PICT_TYPE_TRIANGLE;
   temp->edge[0] = (-1 == aedge[0]) ? PICT_NEVER_DRAW : aedge[0];
   temp->edge[1] = (-1 == aedge[1]) ? PICT_NEVER_DRAW : aedge[1];
   temp->edge[2] = (4 == act) ? PICT_NEVER_DRAW : 
      ((-1 != aedge[2]) ? aedge[2] :
	 ((-1 == aedge[3]) ? FALSE : aedge[3]));
   for(i=0;i<3;i++)
   {
      temp->info.poly.norm[i] = poly->info.poly.norm[i];
      temp->vertex[i] = above[i];
      if (PICT_ALWAYS_DRAW == (PICT_ALWAYS_DRAW & temp->edge[i]))
	 edge_ct++;
   }
   temp->info.poly.dist = poly->info.poly.dist;
   temp->source = poly->source;
   temp->order_data = poly->order_data;
   temp->back = NULL;
   temp->front = *front;
   *front = temp;

   if (4==act)
   {
      ALLOC_COMMAND(temp);
      temp->type = PICT_TYPE_TRIANGLE;
      temp->edge[0] = (-1 == aedge[2]) ? PICT_NEVER_DRAW : aedge[2];
      temp->edge[1] = (-1 == aedge[3]) ? PICT_NEVER_DRAW : aedge[3];
      temp->edge[2] = PICT_NEVER_DRAW;
      for(i=0;i<3;i++)
      {
	 temp->info.poly.norm[i] = poly->info.poly.norm[i];
	 if (PICT_ALWAYS_DRAW == (PICT_ALWAYS_DRAW & temp->edge[i]))
	    edge_ct++;
      }
      temp->info.poly.dist = poly->info.poly.dist;
      temp->source = poly->source;
      temp->vertex[0] = above[2];
      temp->vertex[1] = above[3];
      temp->vertex[2] = above[0];
      temp->back = NULL;
      temp->front = *front;
      temp->order_data = DUPLICATE_POLY;
      *front = temp;
   }
   
   poly->edge[0] = (-1 == bedge[0]) ? PICT_NEVER_DRAW : bedge[0];
   poly->edge[1] = (-1 == bedge[1]) ? PICT_NEVER_DRAW : bedge[1];
   poly->edge[2] = (4 == bct) ? PICT_NEVER_DRAW : 
      ((-1 != bedge[2]) ? bedge[2] :
	 ((-1 == bedge[3]) ? PICT_NEVER_DRAW : bedge[3]));
   for(i=0;i<3;i++)
   {
      poly->vertex[i] = below[i];
      if (PICT_ALWAYS_DRAW == (PICT_ALWAYS_DRAW & poly->edge[0]))
	 edge_ct++;
   }
   poly->front = *back;
   *back = poly;
   
   if (4 == bct)
   {
      ALLOC_COMMAND(temp);
      temp->type = PICT_TYPE_TRIANGLE;
      temp->edge[0] = (-1 == bedge[2]) ? PICT_NEVER_DRAW : bedge[2];
      temp->edge[1] = (-1 == bedge[3]) ? PICT_NEVER_DRAW : bedge[3];
      temp->edge[2] = PICT_NEVER_DRAW;
      for(i=0;i<3;i++)
      {
	 temp->info.poly.norm[i] = poly->info.poly.norm[i];
	 if (PICT_ALWAYS_DRAW == (PICT_ALWAYS_DRAW & temp->edge[i]))
	    edge_ct++;
      }
      temp->info.poly.dist = poly->info.poly.dist;
      temp->source = poly->source;
      temp->vertex[0] = below[2];
      temp->vertex[1] = below[3];
      temp->vertex[2] = below[0];
      temp->front = *back;
      temp->back = NULL;
      temp->order_data = DUPLICATE_POLY;
      *back = temp;
   }
   
   pict->numlines += edge_ct;
   return;
}


/* This function makes a BSP tree full of items that are not order */
/* important even in hidden surface removal: lines and points. */

static PictCommPtr bsp_stuff_tree(stuff)
   PictCommPtr stuff;
{
   PictCommPtr ret;

   if (NULL == stuff || NULL == stuff->front)
      return stuff;

   ret = stuff;
   stuff = stuff->front;
   ret->front = NULL;
   bsp_flatten(ret, stuff);
   return ret;
}


/* This function generates a new plane whose normal is along the axis of */
/* greatest extent and whose distance divides that extent in half. */

static PictCommPtr bsp_new_plane(min, max, fmin, fmax, bmin, bmax)
   Real *min, *max, *fmin, *fmax, *bmin, *bmax;
{
   Real dist;
   int i;
   int axis;
   PictCommPtr ret;

   ME(bsp_new_plane);

   ALLOC_COMMAND(ret);
   ret->type = PICT_TYPE_NO_DRAW;
   ret->info.poly.norm[0] = ret->info.poly.norm[1] = ret->info.poly.norm[2]
      = 0.0;
   for(dist = 0.0, i=0, axis = 0 ; i<3; i++)
      if ((max[i] - min[i]) > dist)
      {
	 dist = max[i] - min[i];
	 axis = i;
      }
   ret->info.poly.norm[axis] = 1.0;
   ret->info.poly.dist = dist/2 + min[axis];
   for(i=0; i<3; i++)
   {
      if (i==axis)
	 fmin[i] = bmax[i] = ret->info.poly.dist;
      else
      {
	 fmin[i] = min[i];
	 bmax[i] = max[i];
      }
      fmax[i] = max[i];
      bmin[i] = min[i];
   }

   return ret;
}
   

/* This is the recursive function that actually does the bsp sort/divide */
/* thing.  First it checks to make sure there are triangles to split on. */
/* If there aren't, then it happily chucks the "stuff" that remains into a */
/* balanced tree, using bsp_flatten, and returns that.  Otherwise it */
/* finds a good polygon to split on (the last one if this one might be, or */
/* else one chosen at random) and splits the two lists on it.  This */
/* involves going through each item in the list and putting it (or its */
/* parts) on one or the other side of the random polygon.  The number of */
/* polygons left is approximated (not calculated), but that's OK since all */
/* we do with it is choose which polygon to split on next. */

static PictCommPtr bsp_make_tree(pict, polies, stuff, num_polies, min, max,
				 mode) 
   Pict pict;
   PictCommPtr polies;
   PictCommPtr stuff;
   int num_polies;
   Real *min;
   Real *max;
   int mode;

{
   PictCommPtr ret;
   PictCommPtr front=NULL, back=NULL;
   PictCommPtr front_stuff = NULL, back_stuff = NULL;
   PictCommPtr curr, temp;
   Real dists[3];
   Real fmin[3], fmax[3];
   Real bmin[3], bmax[3];
   int num_front=0, num_back=0;
   int decision;

   ME(bsp_make_tree);

   /* Set at the end */
   switch(mode)
   {
    case FIRST_MODE:
      ret = polies;
      polies = polies->front;
      break;
      
    case RANDOM_MODE:
      ret = bsp_choose(polies, num_polies);
      break;

    case PLANE_MODE:
      ret = bsp_new_plane(min, max, fmin, fmax, bmin, bmax);
      break;
   }

   /* Split all the "stuff"--the things that are not triangles */
   for( curr = stuff ; curr != NULL ; curr = temp )
   {
      temp = curr->front;
      decision = bsp_compare(ret, curr, dists);
      switch(decision)
      {
       case THIS_SIDE:
	 curr->front = front_stuff;
	 front_stuff = curr;
	 num_front++;
	 break;
	 
       case DONT_CARE:
	 if (num_front < num_back)
	 {
	    curr->front = front_stuff;
	    front_stuff = curr;
	    num_front++;
	    break;
	 }
	 /* fall through */
	 
       case THAT_SIDE:
	 curr->front = back_stuff;
	 back_stuff = curr;
	 num_back++;
	 break;

       case MUST_SPLIT:
	 num_front++;
	 num_back++;
	 bsp_split_segment(pict, curr, &front_stuff, &back_stuff,
			   dists);
	 break;

       default:
	 DEFAULT_ERR(decision);
      }
   }
   
   /* Split all the triangles. */
   num_front = 0;
   num_back = 0;
   for ( curr = polies ; curr != NULL ; curr = temp )
   {
      temp = curr->front;
      decision = bsp_compare(ret, curr, dists);
      switch(decision)
      {
       case THIS_SIDE:
	 curr->front = front;
	 front = curr;
	 if (ORIGINAL_POLY == curr->order_data)
	    num_front++;
	 break;

       case DONT_CARE:
	 if (num_front < num_back)
	 {
	    curr->front = front;
	    front = curr;
	    if (ORIGINAL_POLY == curr->order_data)
	       num_front++;
	    break;
	 }
	 
       case THAT_SIDE:
	 curr->front = back;
	 back = curr;
	 if (ORIGINAL_POLY == curr->order_data)
	    num_back++;
	 break;

       default:
	 if (ORIGINAL_POLY == curr->order_data)
	 {
	    num_front++;
	    num_back++;
	 }
	 bsp_split_triangle(pict, curr, &front, &back, dists);
      }
   }
   
   if (NULL == front)
      ret->front = bsp_stuff_tree(front_stuff);
   else 
   {
      mode = (2 > num_front) ? FIRST_MODE : 
	 (BSP_SWITCH_PT > num_front) ? RANDOM_MODE :
	    (PLANE_MODE==mode) ? PLANE_MODE : RANDOM_MODE;
      ret->front = bsp_make_tree(pict, front, front_stuff, num_front, fmin,
				 fmax, mode);
   }

   if (NULL == back)
      ret->back = bsp_stuff_tree(back_stuff);
   else 
   {
      mode = (2 > num_back) ? FIRST_MODE : 
	 (BSP_SWITCH_PT > num_back) ? RANDOM_MODE :
	    (PLANE_MODE==mode) ? PLANE_MODE : RANDOM_MODE;
      ret->back = bsp_make_tree(pict, back, back_stuff, num_back, bmin,
				bmax, mode);
   }

   return ret;
}

   

/* This is the general function by which a pict can bsp-order itself.  It */
/* first goes through all of the pict commands associated with the pict and */
/* separates them into those that can be split on and those that can't. */
/* Those that can be split on have a little processing done on them: they */
/* are counted, and their plane equation (normals, and distance) are */
/* calculated.  Triangles whose plane equations cannot be calculated are */
/* "lined"--their offending coordinate is returned by bsp_compute_norm, and */
/* removed, making the ex-degenerate triangle the line it really is. The */
/* two lists thus created are passed off to bsp_make_tree, */
/* and the resultant tree is hooked back in as pict->commands.  Then the */
/* pict is marked as ordered. */

void PictSort (pict)
   Pict pict;

{
   PictCommPtr triangles= NULL;
   PictCommPtr other_stuff = NULL;
   PictCommPtr curr, next;
   int num_tris=0;
   Real min[3], max[3];
   Real bigdiff = 0.0;
   Real area, sum;
   int i;
   ME(PictSort);

   curr = pict->commands;

   for(i=0;i<3;i++)
   {
      min[i] = pict->min[i];
      max[i] = pict->max[i];
      bigdiff = MAX(max[i]-min[i], bigdiff);
   }

   sum = 0.0;
   while(curr != NULL)
   {
      next = curr->front;
      if (PICT_TYPE_TRIANGLE == curr->type)
	 switch(bsp_compute_norm(curr, &area))
	 {
	  case SAFE:
	    curr->front = triangles;
	    curr->order_data = ORIGINAL_POLY;
	    triangles = curr;
	    num_tris++;
	    sum += LOG(area);
	    break;
	    
	  case FIRST_BAD:
	    curr->vertex[0] = curr->vertex[2];
	    curr->type = PICT_TYPE_LINE;
	    break;
	    
	  case SECOND_BAD:
	    curr->vertex[1] = curr->vertex[2];
	    curr->type = PICT_TYPE_LINE;
	    break;

	  case THIRD_BAD:
	    curr->type = PICT_TYPE_LINE;
	    break;

	  default:
	    DEFAULT_ERR(bsp_compute_norm(curr, &area));
	 }
      if (!(PICT_TYPE_TRIANGLE == curr->type))
      {
	 curr->front = other_stuff;
	 curr->order_data = LINE_OR_POINT;
	 other_stuff = curr;
      }
      curr = next;
   }

   /* We're doing computer graphics, not rocket science, here.
      The efficiency of the algorithm can be greatly improved 
      by not worrying about details (small things) */

   if (num_tris > 0)
      Epsilon = SQRT (EXP (sum / num_tris)) / BSP_CARELESSNESS_FACTOR;
   else
      Epsilon = 1.0;

   pict->commands = bsp_make_tree(pict, triangles, other_stuff, num_tris,
				  min, max, PLANE_MODE);
   pict->sorted  = YES;
}

#endif
