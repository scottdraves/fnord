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
#include "widgets/spin.h"

#ifndef lint
static char Version[] = "$Id: xpict.c,v 1.15 1992/06/22 17:11:33 mas Exp $";
#endif /* undef lint */

PictCommPtr pict_free_cmd_head;
void pict_object_free();
void pict_new_list();

Real *pict_new_vertex();
static void PictCommandsForPoints();
static void PictCommandsForCurve();
static void PictCommandsForTriangles();
static void PictCommandsForMesh();
static void PictAddCurves();
static void PictAddSurfaces();
static void PictAddBlock();
static void PictCommandsForObject();
static void pict_free_commands();
static void PictClear();
static void PictExecuteWFCommands();
static void PictExecuteHSCommands();
static void PictWriteWFCommands();
static void PictWriteHSCommands();

/* Externs to drawing functions */
extern void xdraw_point();
extern void xdraw_line();
extern void xdraw_triangle();
extern void xdraw_text_2d();
extern void xfill_point();
extern void xfill_line();
extern void xfill_triangle();
extern void xfill_text_2d();
extern void print_point();
extern void print_line();
extern void print_triangle();
extern void print_filled_triangle();
extern void print_text_2d();

/* Memory allocation/management functions: */

/* Pict commands. */

#define FUNC pict_new_list
#define LIST pict_free_cmd_head
#define C_TYP PictCommand
#define MM_TYPE object_type_bsp_node
#define CHUNK_SIZE 2000
#include "object/object_new_list.c"
#undef FUNC
#undef LIST
#undef C_TYP
#undef MM_TYPE
#undef CHUNK_SIZE


/* This function is the quickie function used to return the memory */
/* associated with a pict command to the free list.  It does just that-- */
/* the bit about the magic is some debugging bookkeeping.	*/

void pict_object_free(ptr)
   PictCommPtr ptr;

{
   ptr->mminfo.next = pict_free_cmd_head;
   pict_free_cmd_head = ptr;
#ifdef object__debug_magic
   ptr->magic = object__magic_free;
#endif
}

/* Random reals that we'll need in splitting things */
/* This function is called to allocate new vertices for the pict: when */
/* polygons and segments are split, there needs to be additional memory */
/* provided for the new vertices made.  This is where they come from. */

Real *pict_new_vertex (pict, dim)
   Pict pict;
   int dim;
{
   PictPointRec *temp = pict->points;
   Real *result;
   ME(pict_new_vertex);
   
   if (NULL == temp ||
       temp->num_left < dim)
   {
      ALLOCM(pict->points, PictPointRec);
      ALLOCNM(pict->points->data, Real, PICT_VERTEX_ADD_NUM);
      pict->points->num_left = PICT_VERTEX_ADD_NUM;
      pict->points->num_points = PICT_VERTEX_ADD_NUM;
      pict->points->next = temp;
   }

   result = pict->points->data + 
      (pict->points->num_points - pict->points->num_left);
   pict->points->num_left -= dim;

   return result;
}

/* This function makes a new pict. (Surprised?)  Specifically, it allocates */
/* memory for the object, and initializes all its fields to their default */
/* (NULL, 0) values.  The sole exception to the rather dull values which */
/* the pict is originally given is the value of the callbacks, which tell */
/* the pict how to put itself on the screen somewhere.  This is set to be a */
/* copy of the passed structure. */

Pict PictCreate(env, data)
   DrawEnv env;
   PictXInitData *data;

{
   Pict pict;
   Real axis[3];
   ME(pict_create);

   ALLOCM(pict, PictRec);

   pict->env = env;
   pict->objs = NULL;
   pict->being_drawn = NO;

   pict->commands = NULL;
   pict->numpts = 0;
   pict->numlines = 0;
   pict->sorted = NO;

   axis[0] = 1.0;
   axis[1] = 1.0;
   axis[2] = 1.0;
   spin_from_axis(pict->cam.spin, axis, 0.0);
   pict->cam.spin[3] = 0.0;

   pict->w = data->w;
   pict->p = data->p;
   pict->gc = data->gc;
   pict->background = data->background;
   pict->draw_mode = data->draw_mode;

   pict->win_min[0] = pict->win_max[0] = 0.0;
   pict->win_min[1] = pict->win_max[1] = 0.0;
   pict->win_min[2] = pict->win_max[2] = 0.0;

   pict->points = NULL;
   
   pict->prev_mode = pict->mode = ENV_NO_MODE;
   
   return pict;
}

/* Take the externally defined constants specifying how the thing should 
   draw itself and translate that into a setting of the various flags of
   the pict environment.  Most of the time this is straightforward; now,
   colormap mode first checks to see whether the env actually supports
   messing with the colormap before it sets the mode.  If double-buffering
   is not supported, the pict acts like it's in pixmap mode. */

void PictSetDrawStyle (pict, mode)
   Pict	pict;
   int	mode;
{
   DrawEnv	env = pict->env;
   ME(PictSetDrawStyle);

   pict->mode = mode;
   switch (mode)
   {
    case ENV_NO_MODE:
    case ENV_DUMB_MODE:
      pict->dest_type = ENV_DRAW_TO_WINDOW;
      pict->use_cmgcs = NO;
      break;
    case ENV_COLORMAP_MODE:
      if (env->dbuf_planes > 0)
      {
	 pict->dest_type = pict->prev_mode == ENV_COLORMAP_MODE ? 
	    ENV_DRAW_TO_WINDOW : ENV_DRAW_TO_PIXMAP;
	 pict->use_cmgcs = YES;
	 break;
      }
    case ENV_PIXMAP_MODE:
      pict->dest_type = ENV_DRAW_TO_PIXMAP;
      pict->use_cmgcs = NO;
      break;
    case ENV_BW_POSTSCRIPT_MODE:
    case ENV_GRAY_POSTSCRIPT_MODE:
    case ENV_COLOR_POSTSCRIPT_MODE:
      pict->dest_type = ENV_DRAW_TO_FILE;
      pict->use_cmgcs = NO;
      break;
    default:
      DEFAULT_ERR(mode);
   }
}

/* The following are helper routines for the function which adds a view
   object to a pict.  Their use depends on the connex of the view object */

static void PictCommandsForPoints (pict, vo, data, width,
				   ndims, dims, types)
   Pict pict;
   ViewObj vo;
   Real *data;
   int width;
   int ndims;
   int dims[];
   int types[];
{
   int npoints, dim;
   PictCommPtr curr = pict->commands;

   ME(PictCommandsForPoints);

   npoints = 1;
   for (dim = 0; dim < ndims; dim++)
      if (types[dim] == MET_CONNEX_TYPE_MESH)
	 npoints *= ABS(dims[dim]);
      else
	 npoints *= ABS(dims[dim]) * (ABS(dims[dim]) + 1) / 2;

   pict->numpts += npoints;

   while (npoints--)
   {
      curr = pict->commands;
      ALLOC_COMMAND(pict->commands);
      pict->commands->front = curr;
      pict->commands->back = NULL;
      pict->commands->type = PICT_TYPE_POINT;
      pict->commands->vertex[0] = data + npoints*width;
      pict->commands->source = vo;
   }
}

static void PictCommandsForSpecial (pict, vo, block, width)
   Pict pict;
   ViewObj vo;
   ViewBlock *block;
   int width;
{
   int npoints, t, i;
   PictCommPtr curr = pict->commands;

   ME(PictCommandsForPoints);

   t = (block->special == MET_ACCESS_VERTICES_SPECIAL_TEXT)
      ? PICT_TYPE_TEXT_2D
	 : PICT_TYPE_MARKER;

   npoints = ABS(block->length);
   while (npoints--)
   {
      curr = pict->commands;
      ALLOC_COMMAND(pict->commands);
      pict->commands->front = curr;
      pict->commands->back = NULL;
      pict->commands->type = t;
      pict->commands->vertex[0] = block->vertices + npoints*width;
      pict->commands->source = vo;
      for (i = 0; i < block->text_len; i++)
	 pict->commands->info.text_2d[i] =
	    * (((int *)block->verts[i]) + block->width[i] * npoints);
      pict->commands->info.text_2d[i] = '\0';
   }
}

static void PictCommandsForCurve (pict, vo, data, step, dec, num)
   Pict pict;
   ViewObj vo;
   Real *data;
   int step;
   int dec;
   int num;
{
   PictCommPtr curr;
   int i;

   ME(PictCommandsForCurve);
   pict->numlines += num-1;

   for (i = 0 ; i < num-1; 
	i++, data += step, step -= dec)
   {
      ALLOC_COMMAND(curr);
      curr->type =  PICT_TYPE_LINE;
      curr->vertex[0] = data;
      curr->vertex[1] = data + step;
      curr->front = pict->commands;
      curr->back = NULL;
      curr->source = vo;
      pict->commands = curr;
   }
}

static void PictCommandsForTriangles (pict, vo, data, step, count)
   Pict pict;
   ViewObj vo;
   Real *data;
   int step;
   int count;
{
   int i, j, k;
   Real *cur_vert = data;
   PictCommPtr curr, next;
   ME(PictCommandsForTriangles);
   
   pict->numlines += 3 * count * (count+1) / 2;

   for(i = 0; i < count - 1; i++, cur_vert += step)
   {
      ALLOC_COMMAND(curr);
      curr->front = pict->commands;
      curr->back = NULL;
      pict->commands = curr;
      curr->type = PICT_TYPE_TRIANGLE;
      for(k=0;k<3;k++)
	 curr->edge[k] = PICT_ALWAYS_DRAW;
      curr->vertex[0] = cur_vert;
      curr->vertex[1] = cur_vert + step;
      curr->vertex[2] = cur_vert + step * ( count - i );
      curr->source = vo;
      for(j=0, cur_vert += step; j<count-i-2; j++, cur_vert += step)
      {
	 ALLOC_COMMAND(curr);
	 ALLOC_COMMAND(next);
	 next->front = pict->commands;
	 curr->front = next;
	 curr->back = next->back = NULL;
	 pict->commands = curr;
	 next->type = curr->type = PICT_TYPE_TRIANGLE;
	 next->source = curr->source = vo;
	 for(k=0; k<3; k++)
	 {
	    curr->edge[k] = PICT_DRAW_IN_PS;
	    next->edge[k] = PICT_ALWAYS_DRAW;
	 }
	 curr->vertex[0] = next->vertex[0] = cur_vert;
	 curr->vertex[1] = cur_vert + step * ( count - i - 1);
	 curr->vertex[2] = next->vertex[1] = 
	    cur_vert + step * ( count - i );
	 next->vertex[2] = cur_vert + step;
      }
   }
}

static void PictCommandsForMesh (pict, vo, verts, first_step, first_ct,
				 second_step, second_ct)
   Pict pict;
   ViewObj vo;
   Real *verts;
   int first_step;
   int first_ct;
   int second_step;
   int second_ct;
{
   PictCommPtr curr, next;
   int i, j;
   Real *curr_verts = verts;

   ME(PictCommandsForMesh);

   pict->numlines += (2 * first_ct-- * second_ct--);

   for(i=0; i< first_ct; i++, curr_verts = verts+ i*first_step)
      for(j=0; j < second_ct; j++, curr_verts += second_step)
      {
	 ALLOC_COMMAND(curr);
	 ALLOC_COMMAND(next);
	 next->front = pict->commands;
	 curr->front = next;
	 curr->back = next->back = NULL;
	 pict->commands = curr;

	 next->type = curr->type = PICT_TYPE_TRIANGLE;
	 next->source = curr->source = vo;
	 next->edge[0] = curr->edge[0]  = PICT_ALWAYS_DRAW;
	 next->edge[2] = curr->edge[2] = PICT_DRAW_IN_HSR /*PICT_NEVER_DRAW*/;
	 next->edge[1] = (i==first_ct-1) ?
	    PICT_ALWAYS_DRAW : PICT_DRAW_IN_PS;
	 curr->edge[1] = (j==second_ct-1) ?
	    PICT_ALWAYS_DRAW : PICT_DRAW_IN_PS;
	 next->vertex[0] = curr->vertex[0] = curr_verts;
	 curr->vertex[1] = curr_verts + second_step;
	 next->vertex[1] = curr_verts + first_step;
	 next->vertex[2] = curr->vertex[2] =
	    curr_verts + second_step + first_step;
      }
}   

static void PictAddCurves (pict, vo, verts, ndims, dims, types, 
			   multiples, cur_index)
   Pict pict;
   ViewObj vo;
   Real *verts;
   int ndims;
   int dims[];
   int types[];
   int multiples[];
   int cur_index[];
{
   int i, dim, cur_dim;
   Real *cur_vertex;
   int curve_dim;
   int number;

   /* Find the dimension(s) along which the curves run.	*/
   for( dim = 0; dim < ndims; dim++)
   {
      if (dims[dim] < 0)
	 continue;
      
      /* Mark the initial position in the vertex array, 
	 establish our indexing into that array, and set
	 the dimension along which we're evaluating.      	*/
      cur_vertex = verts;
      curve_dim = dim;
      for(i=0; i<ndims; i++)
	 cur_index[i] = 0;
      cur_dim = 0;
      number=dims[curve_dim];

      /* Draw the curves along each dimension, as appropriate */
      do
      {
	 /* If you're drawing a square mesh, it's easy */
	 if (MET_CONNEX_TYPE_MESH == types[curve_dim])
	    PictCommandsForCurve(pict, vo, cur_vertex, 
				 multiples[curve_dim], 0, number);
	 else
	 {
	    for (i=0; i< number - 1; i++)
	    {
	       PictCommandsForCurve(pict, vo, cur_vertex +
				    -(i*(i - 2*number - 1)) / 2  * multiples[curve_dim],
				    multiples[curve_dim],
				    0, number-i);
	       PictCommandsForCurve(pict, vo, cur_vertex +
				    i * multiples[curve_dim],
				    number * multiples[curve_dim],
				    multiples[curve_dim],
				    number-i);
	       PictCommandsForCurve(pict, vo, cur_vertex +
				    (number - 1 - i) * multiples[curve_dim],
				    (number-1) * multiples[curve_dim],
				    multiples[curve_dim],
				    number-i); 
	    }
	 }

	 /* Move on to the next point */
	 for( cur_dim = 0; cur_dim < ndims; cur_dim++)
	    if (cur_dim != curve_dim)
	    {
	       cur_vertex += multiples[cur_dim];
	       if (MET_CONNEX_TYPE_MESH == types[cur_dim])
	       {
		  if ((++cur_index[cur_dim]) < ABS(dims[cur_dim]))
		     break;
	       }
	       else
	       {
		  if ((++cur_index[cur_dim]) < 
		      (ABS(dims[cur_dim])*(ABS(dims[cur_dim])+1)/2))
		     break;
	       }
	    }
	 /* If we had to "carry", reset the counter correctly */
	 for( i=0; i < cur_dim; i++)
	 {
	    cur_vertex -= multiples[i]*cur_index[i];
	    cur_index[i] = 0;
	 }
       
	 
      } while ( cur_dim < ndims );
   }
}

static void PictAddSurfaces(pict, vo, verts, ndims, dims, types, 
			    multiples, cur_index, dims_mask)
   Pict pict;
   ViewObj vo;
   Real *verts;	
   int ndims;	
   int dims[];	
   int types[];
   int multiples[];
   int cur_index[];
   int dims_mask[];
{
   int dim, surf_dim1, surf_dim2, curdim;
   Real *cur_vertex;
   
   /* Find the dimensions where the surfaces vary	*/
   for(surf_dim1 = 0; surf_dim1 < ndims; surf_dim1++)
      if ( dims[ surf_dim1 ] > 0 )
	 break;
   if (MET_CONNEX_TYPE_MESH == types[ surf_dim1 ])
   {
      for(surf_dim2 = surf_dim1 + 1; surf_dim2 < ndims; surf_dim2++)
	 if (dims[ surf_dim2 ] > 0 )
	    break;
   }
   else
   {
      surf_dim2 = surf_dim1;
   }

   /* initialize the various arrays */
   cur_vertex = verts;
   for( curdim = 0; curdim < ndims; curdim++)
      cur_index[curdim] = dims_mask[curdim] = 0;
   dims_mask[surf_dim1] = dims_mask[surf_dim2] = 1;
   
   do
   {
      /* draw the surface from cur_verts with the given steps */
      /* if we're in triangle mode, drop for the triangles,
	 else drop for square mesh. */
      if (MET_CONNEX_TYPE_TRIANGLE==types[surf_dim1])
      {
	 PictCommandsForTriangles(pict, vo, cur_vertex, multiples[surf_dim1],
				  dims[surf_dim1]);
      }
      else
      {
	 PictCommandsForMesh(pict, vo, cur_vertex, multiples[surf_dim1], 
			     dims[surf_dim1], multiples[surf_dim2],
			     dims[surf_dim2]);
      }

      /* increment curdims along the dimensions not being drawn */
      for(curdim = 0; curdim < ndims; curdim++)
	 if (dims_mask[curdim] == 0)
	 {
	    cur_vertex += multiples[curdim];
	    if (MET_CONNEX_TYPE_MESH==types[curdim])
	    {
	       if ((++cur_index[curdim]) < ABS(dims[curdim]))
		  break;
	    }
	    else
	    {
	       if ((++cur_index[curdim]) < ABS(dims[curdim]) *
		   (ABS(dims[curdim]) + 1) / 2)
		  break;
	    }
	 }
      /* where we've had to "carry", reduce the pointer appropriately */
      for( dim = 0; dim < curdim; dim++)
      {
	 cur_vertex -= cur_index[dim] * multiples[dim];
	 cur_index[dim] = 0;
      }
   } while( curdim < ndims );
}

static void PictAddBlock (pict, vo, block, cycle)
   Pict pict;
   ViewObj vo;
   ViewBlock *block;
   int cycle;

{
   int i;

   /*
    * note horrible violation of abstraction
    */ /* Ack! */ /* Ack! */
   int ndims = block->connex->ndims;
   int *dims = block->connex->dims;
   int *types = block->connex->types;
   Real *data = block->vertices;
   int connected_dims = 0;

   int static_multiples[PICT_STATIC_SIZE];
   int static_curdims[PICT_STATIC_SIZE];
   int static_dims_mask[PICT_STATIC_SIZE];
   int *multiples;
   int *curdims;
   int *dims_mask;
   
   ME(pict_plane_draw);

   if (block->special != MET_ACCESS_VERTICES_SPECIAL_NONE) {
      /* this means we have text or blocks.  ignore the connex (treat them
	 as points), and generate pict commands with the strings in them.
	 */
      PictCommandsForSpecial(pict, vo, block, cycle);
      return;
   }
   if (0 == ndims)
   {
      connected_dims = 1;
      PictCommandsForPoints(pict, vo, data, cycle, 1, &connected_dims, types);
      return;
   }

   if (ndims > PICT_STATIC_SIZE) /* which it almost never is	    */
   {
      ALLOCNM( multiples, int, ndims);
      ALLOCNM( curdims, int, ndims);
      ALLOCNM( dims_mask, int, ndims);
   }
   else
   {
      multiples = static_multiples;
      curdims = static_curdims;
      dims_mask = static_dims_mask;
   }

   for(i = 0; i < ndims; i++)
      if (dims[i] > 0)
	 connected_dims += (MET_CONNEX_TYPE_MESH==types[i])?1:2;
   
   multiples[ndims - 1] = cycle;
   for(i = ndims - 2; i >= 0; i--)
      multiples[i] = multiples[i + 1] * 
	 ((MET_CONNEX_TYPE_MESH==types[i+1]) ? ABS(dims[i + 1]) :
	  (ABS(dims[i + 1])*(ABS(dims[i+1])+1))/2);

   
   switch (connected_dims)
   {
    case 0:
      /* draw points only */
      PictCommandsForPoints(pict, vo, data, cycle, ndims, dims, types);
      break;
    case 1:
      /* draw lines only */
      PictAddCurves(pict, vo, data, ndims, dims, types,
		    multiples, curdims);
      break;
    case 2:
      /* draw surfaces only */
      PictAddSurfaces(pict, vo, data, ndims, dims, types,
		      multiples, curdims, dims_mask);
      break;
    default:
      /* draw lines only for now */
      PictAddCurves(pict, vo, data, ndims, dims, types,
		    multiples, curdims);
      break;
   }
   
   if (ndims > PICT_STATIC_SIZE)
   {
      FREEN( multiples, int, ndims);
      FREEN( curdims, int, ndims);
      FREEN( dims_mask, int, ndims);
   }
}

static void PictCommandsForObject (pict, vo)
   Pict pict;
   ViewObj vo;
{
   int i;
   ViewBlockList curr;
   ME(PictAddObject);

   if (pict->commands == NULL)
   {
      pict->min[0] = vo->min[0];
      pict->min[1] = vo->min[1];
      pict->min[2] = vo->min[2];
      pict->max[0] = vo->max[0];
      pict->max[1] = vo->max[1];
      pict->max[2] = vo->max[2];
   }
   else
      for ( i = 0; i < 3; i++ )
      {
	 if (pict->min[i] > vo->min[i])
	    pict->min[i] = vo->min[i];
	 if (pict->max[i] < vo->max[i])
	    pict->max[i] = vo->max[i];
      }

   for ( curr = vo->blocks; curr != NULL; curr = curr->next )
      PictAddBlock ( pict, vo, curr, vo->width == 2 ? 3 : vo->width );
}

void PictAddObject (pict, vo)
   Pict pict;
   ViewObj vo;
{
   ME(PictAddObject);

   vo->next = pict->objs;
   pict->objs = vo;

   PictCommandsForObject (pict, vo);

   EnvRealizeColors (pict->env, vo->model); 
}


/* remove a set from the pict - be careful with this since it does not */
/* automatically update the pict; nor does it destroy the view object, */
/* although it does free the colors required to display it.  */

void PictRemoveObject (pict, vo)
   Pict pict;
   ViewObj vo;
{
   ViewObj curr, prev;
   
   ME(pict_remove);
   
   for (prev = curr = pict->objs; curr != NULL;
	prev= curr, curr = curr->next)
      if (curr == vo)
      {
	 EnvFreeColors (pict->env, curr->model); 
	 if (curr == pict->objs)
	    pict->objs = curr->next;
	 else
	    prev->next = curr->next;
	 break;
      }
}

/* this function makes sure that all the view objects in the pict are up */
/* to date.  If they are, the function returns false; otherwise, the */
/* function redumps all of the objects' commands to the pict. */

FOboolean PictUpdate (pict, force)
   Pict pict;
   FOboolean force;  	/* force the update? */
{
   FOboolean need_to_redraw = force;
   ViewObj curro;
   ME(PictUpdate);

   for ( curro = pict->objs; curro != NULL; curro = curro->next )
      if (VOUpdate(curro))
	 need_to_redraw = YES;

   if (need_to_redraw == YES)
   {
      PictClear (pict);
      for ( curro = pict->objs; curro != NULL; curro = curro->next )
	 PictCommandsForObject ( pict, curro );
   }

   return need_to_redraw;
}


static void pict_free_commands(commands)
   PictCommPtr commands;

{
   PictCommPtr next;

   ME(pict_free_commands);

   while (NULL != commands) {
      if (NULL == commands->front) {
	 next = commands->back;
      } else if (NULL == commands->back) {
	 next = commands->front;
      } else {
	 pict_free_commands(commands->front);
	 next = commands->back;
      }
      FREE_COMMAND(commands);
      commands = next;
   }
}

static void PictClear(pict)
   Pict pict;

{
   PictPointList pts, oldpts;

   ME(pict_clear);

   pict->numpts = 0;
   pict->numlines = 0;
   pict->sorted = NO;

   /* free any vertices that may have been created	*/
   pts = pict->points;
   while(pts != NULL)
   {
      FREEN(pts->data, Real, pts->num_points);
      oldpts = pts;
      pts = pts->next;
      FREE(oldpts, sizeof(PictPointRec));
   }
   pict->points = NULL;

   /* free the memory associated with the commands the pict executed */
   pict_free_commands(pict->commands);
   pict->commands = NULL;

}
	
void PictDestroy(pict)
   Pict pict;

{
   ViewObj curr;
   ME(pict_destroy);

   /* free the colors of the referenced objects */
   for (curr = pict->objs; curr != NULL; curr = curr->next)
      /* EnvFreeColors (pict->env, curr->model) */;
   
   /* Free the drawing information */
   XFreePixmap (pict->env->d, pict->p);
   XFreeGC (pict->env->d, pict->gc);
   PictClear(pict);

   /* free the pict itself	*/
   FREE(pict, sizeof(Pict));
}

void PictDefaultScale(pict)
   Pict pict;
{
   int i;
   Real real_min[3], real_max[3];
   ME(PictDefaultScale);

   for (i=0; i<3; i++)
   {
      real_min[i] = pict->min[i] + pict->padding;
      real_max[i] = pict->max[i] - pict->padding;
   }
   CameraRescale (&pict->cam, pict->min, pict->max,
		  pict->win_min, pict->win_max);
   CameraBuildXform ( &pict->cam );
}

void PictRescale(pict, min, max, padding)
   Pict pict;
   Real min[3];
   Real max[3];
   Real padding;
{
   int i;
   Real real_min[3], real_max[3];
   Real size, dist, scale;
   ME(PictRescale);

   if (padding < 0)
      padding= pict->padding;

   if (pict->win_min[0] == pict->win_max[0] &&
       pict->win_min[0] == 0.0)
   {
      for (i=0; i<3; i++)
      {
	 pict->win_min[i] = min[i];
	 pict->win_max[i] = max[i];
	 real_min[i] = min[i] + padding;
	 real_max[i] = max[i] - padding;
      }
      pict->padding = padding;
      CameraRescale ( &pict->cam, pict->min, pict->max, real_min, real_max );
      CameraBuildXform ( &pict->cam );
      return;
   }

   dist = MIN(pict->win_max[0] - pict->win_min[0] - 2 * pict->padding,
	      pict->win_max[1] - pict->win_min[1] - 2 * pict->padding);
   size = dist / pict->cam.scale[0];
   dist = MIN(max[0] - min[0] - 2 * padding, max[1] - min[1] - 2 * padding);
   scale = dist / size;

   for (i=0; i<3; i++)
      pict->cam.scale[i] = scale;

   for (i=0; i<3; i++)
   {
      pict->cam.posttrans[i] = ( max[i] + min[i] ) / 2.0;
      pict->win_max[i] = max[i];
      pict->win_min[i] = min[i];
   }
   pict->padding = padding;
   CameraBuildXform (&pict->cam);
}

/* This function takes care of causing X requests to be generated which 
   will draw all of the triangles, line segments and points which are
   found in the tree of which the passed command is the head.  The
   parameters expressed in the passed pict are used to do this.  This is no
   surprise.  Since everything is drawn as a wire frame object, there is no
   need to order drawing in any particular way.  Consequently, this
   function uses a "convenient" binary tree drawing algorithm which
   minimizes the number of recursive calls needed. */

static void PictExecuteWFCommands (pict, command)
   Pict pict;
   PictCommPtr command;
{
   PictCommPtr next;
   ME(PictExecuteWFCommands);

   while (NULL != command)
   {
      if (NULL == command->front)
	 next = command->back;
      else if (NULL == command->back)
	 next = command->front;
      else
      {
	 PictExecuteWFCommands (pict, command->front);
	 next = command->back;
      }
      switch (command->type)
      {
       case PICT_TYPE_POINT:
	 xdraw_point (pict, command);
	 break;
       case PICT_TYPE_LINE:
	 xdraw_line (pict, command);
	 break;
       case PICT_TYPE_TRIANGLE:
	 xdraw_triangle (pict, command);
	 break;
       case PICT_TYPE_TEXT_2D:
	 xdraw_text_2d (pict, command);
	 break;
       case PICT_TYPE_MARKER:
	 printf("no markers yet\n");
	 break;
	 
       case PICT_TYPE_NO_DRAW:
	 break;

       default:
	 DEFAULT_ERR(command->type);
      }
      command = next;
   }
}


/* This is the auxiliary recursive function called by bsp_traverse to scan */
/* through the bsp tree, drawing on its merry way.  If the current command */
/* is a line, the function draws the stuff in back of it first, then it, */
/* then the stuff in front of it (not geometrically: along the back and */
/* front subtree paths; it doesn't matter anyway).  If the current command */
/* is a triangle, then the function checks to see whether the "front" tree */
/* or the "back" tree is closer to the viewer and then draws the other */
/* side, the triangle, and that closer side. */

static void PictExecuteHSCommands (pict, command, view)
   Pict pict;
   PictCommPtr command;
   Real view[3];

{
   Real d;
   PictCommPtr first, second;
   ME(PictExecuteHSCommands);

   if (command != NULL)
   {
      first = command->back;
      second = command->front;

      if (PICT_TYPE_TRIANGLE == command->type ||
	  PICT_TYPE_NO_DRAW == command->type)
      {
	 d = view[0] * command->info.poly.norm[0] +
	    view[1] * command->info.poly.norm[1] +
	       view[2] * command->info.poly.norm[2];
      
	 if (d > 0)
	 {
	    first = command->front;
	    second = command->back;
	 }
      }

      PictExecuteHSCommands (pict, first, view);
      switch (command->type)
      {
       case PICT_TYPE_POINT:
	 xfill_point (pict, command);
	 break;
       case PICT_TYPE_LINE:
	 xfill_line (pict, command);
	 break;
       case PICT_TYPE_TRIANGLE:
	 xfill_triangle (pict, command);
	 break;
       case PICT_TYPE_TEXT_2D:
	 xfill_text_2d (pict, command);
	 break;
       case PICT_TYPE_MARKER:
	 printf("no markers yet\n");
	 break;
       case PICT_TYPE_NO_DRAW:
	 break;
       default:
	 DEFAULT_ERR(command->type);
      }
      PictExecuteHSCommands (pict, second, view);
   }
}

/* This function takes care of writing to the currently active output file 
   information required (in the currently active format) to draw all of the
   triangles, line segments and points which are
   found in the tree of which the passed command is the head.  The
   parameters expressed in the passed pict are used to do this.  This is no
   surprise.  Since everything is drawn as a wire frame object, there is no
   need to order drawing in any particular way.  Consequently, this
   function uses a "convenient" binary tree drawing algorithm which
   minimizes the number of recursive calls needed. */

static void PictWriteWFCommands (pict, command)
   Pict pict;
   PictCommPtr command;
{
   PictCommPtr next;
   ME(PictWriteWFCommands);

   while (NULL != command)
   {
      if (NULL == command->front)
	 next = command->back;
      else if (NULL == command->back)
	 next = command->front;
      else
      {
	 PictWriteWFCommands (pict, command->front);
	 next = command->back;
      }
      switch (command->type)
      {
       case PICT_TYPE_POINT:
	 print_point (pict, command);
	 break;
       case PICT_TYPE_LINE:
	 print_line (pict, command);
	 break;
       case PICT_TYPE_TRIANGLE:
	 print_triangle (pict, command);
	 break;
       case PICT_TYPE_TEXT_2D:
	 print_text_2d (pict, command);
	 break;
       case PICT_TYPE_MARKER:
	 printf("no markers yet\n");
	 break;
	 
       case PICT_TYPE_NO_DRAW:
	 break;

       default:
	 DEFAULT_ERR(command->type);
      }
      command = next;
   }
}


/* This is the auxiliary recursive function called when hidden surface */
/* removal is on, to scan through the bsp tree.  At each step, the function */
/* outputs a text description of what is happenning at that node, using the */
/* printing functions defined in the xpictdraw file.  Just as a note, to */
/* those few who might be concerned...  If the current command */
/* is a line, the function draws the stuff in back of it first, then it, */
/* then the stuff in front of it (not geometrically: along the back and */
/* front subtree paths; it doesn't matter anyway).  If the current command */
/* is a triangle, then the function checks to see whether the "front" tree */
/* or the "back" tree is closer to the viewer and then draws the other */
/* side, the triangle, and that closer side. */

static void PictWriteHSCommands (pict, command, view)
   Pict pict;
   PictCommPtr command;
   Real view[3];

{
   Real d;
   PictCommPtr first, second;
   ME(PictWriteHSCommands);

   if (command != NULL)
   {
      first = command->back;
      second = command->front;

      if (PICT_TYPE_TRIANGLE == command->type ||
	  PICT_TYPE_NO_DRAW == command->type)
      {
	 d = view[0] * command->info.poly.norm[0] +
	    view[1] * command->info.poly.norm[1] +
	       view[2] * command->info.poly.norm[2];
      
	 if (d > 0)
	 {
	    first = command->front;
	    second = command->back;
	 }
      }

      PictWriteHSCommands (pict, first, view);
      switch (command->type)
      {
       case PICT_TYPE_POINT:
	 print_point (pict, command);
	 break;
       case PICT_TYPE_LINE:
	 print_line (pict, command);
	 break;
       case PICT_TYPE_TRIANGLE:
	 print_filled_triangle (pict, command);
	 break;
       case PICT_TYPE_TEXT_2D:
	 print_text_2d (pict, command);
	 break;
       case PICT_TYPE_MARKER:
	 printf("no markers yet\n");
	 break;
       case PICT_TYPE_NO_DRAW:
	 break;
       default:
	 DEFAULT_ERR(command->type);
      }
      PictWriteHSCommands (pict, second, view);
   }
}

void PictOutput (pict)
   Pict 	pict;
{
   Drawable 	d;
   Real 	view[3];
   void		(*wireframe)();
   void		(*hidden)();
   ME(PictOutput);
   
   switch (pict->dest_type) {
    case ENV_DRAW_TO_PIXMAP:
      pict->dest = d = pict->p;
      wireframe = PictExecuteWFCommands;
      hidden = PictExecuteHSCommands;
      EnvUseMask(pict->env, 0xff);
      break;

    case ENV_DRAW_TO_WINDOW:
      pict->dest = d = pict->w;
      wireframe = PictExecuteWFCommands;
      hidden = PictExecuteHSCommands;
      if (YES == pict->use_cmgcs)
	 if (pict->env->A_shows)
	    EnvUseMask(pict->env, pict->env->B_mask);
	 else
	    EnvUseMask(pict->env, pict->env->A_mask);
      break;

    case ENV_DRAW_TO_FILE:
      if (FNULL == pict->env->outputfile)
	 return;
      wireframe = PictWriteWFCommands;
      hidden = PictWriteHSCommands;
      /* skip the setup */
      goto writeout;

    default:
      DEFAULT_ERR(pict->dest_type);
   }
   
   if (pict->use_cmgcs == YES)
      XFillRectangle (pict->env->d, d, pict->env->gc[0],
		      (int) pict->win_min[0], (int) - pict->win_max[1],
		      (unsigned int) (pict->win_max[0] - pict->win_min[0]),
		      (unsigned int) (pict->win_max[1] - pict->win_min[1]));
   else
   {
      XSetForeground (pict->env->d, pict->gc, pict->background.pixel);
      XSetFillStyle (pict->env->d, pict->gc, FillSolid);
      XFillRectangle (pict->env->d, d, pict->gc,
		      (int) pict->win_min[0], (int) - pict->win_max[1],
		      (unsigned int) (pict->win_max[0] - pict->win_min[0]),
		      (unsigned int) (pict->win_max[1] - pict->win_min[1]));
   }

 writeout:
   
   if (pict->commands != NULL) 
      if (pict->draw_mode == PICT_WIRE_FRAME)
	 (wireframe) (pict, pict->commands);
      else
      {
	 Real mat[3][3];
	 if (pict->sorted == NO)
	    PictSort (pict);
	 spin_to_matrix(mat, pict->cam.spin);
	 view[0] = - mat[2][0];
	 view[1] = - mat[2][1];
	 view[2] = - mat[2][2];
	 (hidden) (pict, pict->commands, view);
      }
   
   if (pict->dest_type == ENV_DRAW_TO_PIXMAP)
      XCopyArea (pict->env->d, pict->p, pict->w, pict->gc,
		 (int) pict->win_min[0], (int) - pict->win_max[1],
		 (int) (pict->win_max[0] - pict->win_min[0]),
		 (int) (pict->win_max[1] - pict->win_min[1]), 
		 (int) pict->win_min[0], (int) - pict->win_max[1]);

   pict->prev_mode = pict->mode;
}

void PictChangePixmap (pict, p)
   Pict pict;
   Pixmap p;
{
   if (pict->p != None)
      XFreePixmap(pict->env->d, pict->p);
   pict->p = p;
}

void PictChangeWindow (pict, w)
   Pict pict;
   Window w;
{
   pict->w = w;
}

void PictChangeDrawMode (pict)
   Pict pict;
{
   if (pict->draw_mode == PICT_WIRE_FRAME)
      pict->draw_mode = PICT_HIDDEN_SUR;
   else
      pict->draw_mode = PICT_WIRE_FRAME;
}

void PictSetSpin (pict, spin)
   Pict pict;
   Spin spin;
{
   CameraSetSpin( &pict->cam, spin);
   CameraBuildXform( &pict->cam);
}

Camera
PictGetCamera(pict)
   Pict pict;
{
   return &pict->cam;
}

#endif defined(X_THREE_D)
   
