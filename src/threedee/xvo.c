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
#include "port/time.h"
#include <string.h>

#ifndef lint
static char Version[] =
   "$Id: xvo.c,v 1.15 1992/06/22 17:11:49 mas Exp $";
#endif /* undef lint */

static ColorModel ParseColors ();
static FOboolean VOConvertPoints ();
static Real *VOCopyPoints ();
static Real *VOConvertReps ();
static FOboolean VOSetToBlocks ();
static void VOFreeBlocks ();

/* Take the string used to describe the color to be used to display the 
   function and determine the actual X colors that should be used to put it
   on the screen.  This function just determines the kind of mapping that
   the user wants and the rgb values of the colors that should come up.  It
   makes no changes to the colormap to ensure that those colors are
   available--(that change is made when we know we're actually going to
   display the function so that nothing is wasted).  The nice thing is that
   it can parse any color that X understands...

   The format for messages right now is this:
   color<-"green"  where the function is displayed entirely in green.
   color<-"4:green->blue"  where the range of the fourth (h or w)
   coordinate of the function is mapped onto the line in r g b space
   between the colors the X server thinks of as green and blue. */

static ColorModel ParseColors (env, colors)
   DrawEnv env;
   char *colors;
{
   char *first_token, *second_token;
   char color_temp[200];
   ColorModel result;
   ME(VOParseColors);

   ALLOCM(result, ColorModelRec); 
   result -> modelled = NO;

   if (CNULL == colors) {
      result->type = COLOR_MODEL_TYPE_NONE;
      return result;
   }

   strcpy(color_temp, colors);
   first_token = strtok (color_temp, ":");
   second_token = strtok (CNULL, "->");

   if (second_token == NULL)
   {
      result -> type = COLOR_MODEL_TYPE_CONSTANT;
      /* blargus attempt to prevent totally black window effect */
      result->info.constant.color.pixel =
	 BlackPixel(env->d, DefaultScreen(env->d));
      if (! XParseColor (env->d, env->map, color_temp,
			 &result->info.constant.color))
      {
	 fprintf(stderr, "Mysterious color name %s\n", color_temp);
	 FREE(result, sizeof(ColorModelRec));
	 return NULL;
      }
      return result;
   }

   result -> type = COLOR_MODEL_TYPE_LINEAR;
   if ((result -> info.linear.dimension = atoi(first_token)) < 1)
   {
      fprintf(stderr, "Illegal value for dimension in color spec\n");
      FREE(result, sizeof(ColorModelRec));
      return NULL;
   }

   result -> info.linear.dimension--;
   first_token = strtok( CNULL, "->");
   if (first_token == NULL)
   {
      fprintf(stderr,
	      "Two color_temp must be specified in dimension mapping\n");
      FREE(result, sizeof(ColorModelRec));
      return NULL;
   }

   if (! XParseColor (env->d, env->map, first_token,
		      &result->info.linear.end))
   {
      fprintf(stderr, "Mysterious color name %s\n", first_token);
      FREE(result, sizeof(ColorModelRec));
      return NULL;
   }

   if (! XParseColor (env->d, env->map, second_token,
		      &result->info.linear.start))
   {
      fprintf(stderr, "Mysterious color name %s\n", second_token);
      FREE(result, sizeof(ColorModelRec));
      return NULL;
   }

   return result;

}

/* Copies the points in the METnumbers into a contiguous array of 
   reals.  Assumes that width and field were allocated in memory
   with at least three dimensions to them.  If this is not the
   case, memory will be eaten. */

static Real *VOCopyPoints(data, size, length, width, field)
   METnumber **data;
   int size;   
   int length;
   int width[];
   int field[];

{
   int i,j;
   METreal *cur_real[VO_STATIC_SIZE];
   METinteger *cur_int[VO_STATIC_SIZE];
   Real *new_array, *curr;
   METreal temp = (METreal) 0.0;
   ME(VOCopyPoints);

   /* Initialize the array of counters.  (Bear in mind two things: to cast */
   /* the data from the right type; and to ensure that we have at least 3 */
   /* dimensions from which to steal data. */

   for(i=0; i<size; i++)
      if (MET_FIELD_REAL == field[i])
      {
	 cur_real[i] = (METreal *) data[i];
	 cur_int[i] = (METinteger *) NULL;
      }
      else
      {
	 cur_real[i] = (METreal *) NULL;
	 cur_int[i] = (METinteger *) data[i];
      }
   
   if (2 == size)
   {
      cur_real[2] = (METreal *) &temp;
      width[2] = 0;
      field[2] = MET_FIELD_REAL;
      size++;
   }

   ALLOCNM(new_array, METreal, size*length);

   /* Make the cast and copy.  */

   for (j = 0; j < size; j++)
      if (field[j] == MET_FIELD_REAL)
	 for (i = 0, curr = new_array + j; i < length;
	      cur_real[j] += width[j], i++, curr += size)
	    *curr = (Real) *cur_real[j];
      else
	 for (i = 0, curr = new_array + j; i < length;
	      cur_int[j] += width[j], i++, curr += size)
	    *curr = (Real) *cur_int[j];

   return new_array;
}

/* We expect our data to be in a fairly precise form for bsp trees
   and other things, which the MET set that the data comes from is not
   guaranteed to be in.  We have to make sure we have a continuous 
   array of reals with at least three coordinates per pixel.... */

/* for markers and text SIZE does not include the marker number or text. 
   the number of additional coords beyond size is in TEXT_LENGTH.  for
   non-special stuff, text_length is 0.  */

static Real *VOConvertReps ( vo, verts, size, length,
			     width, field, newlen, newwid, special,
			    text_length) 
   ViewObj vo;
   METnumber **verts;
   int size;
   int length;
   int width[];
   int field[];
   int *newlen;
   int *newwid;
   int special, text_length;
{
   Real *min = vo->min;
   Real *max = vo->max;
   Real *data;
   FOboolean need_to_convert = NO;
   int i,j;
   ME(VOConvertReps);

   /* Are the points already in an appropriate representation? */
   if (size == 2) 
      need_to_convert = YES;
   else if (sizeof(Real) != sizeof(METreal))
      need_to_convert = YES;
   else 
      for (i = 0; i < size; i++)
	 if (field[i] != MET_FIELD_REAL)
	    need_to_convert = YES;
   
   if (need_to_convert == NO)
      for (i = 1; i < size; i++)
	 if ((METreal *) verts[i] != ((METreal *) verts[i-1]) + 1)
	    need_to_convert = YES;


   /* Fix the vertices if the data we've gotten from MET is not exactly */
   /* right. */

   if (need_to_convert == YES)
   {
      data = VOCopyPoints(verts, size, length, width, field);
      *newlen = length;
   }
   else			/* Vertex coordinates are stored together */
   {
      data = (Real *) *verts;
      *newlen = -length;
   }

   *newwid = (size == 2) ? 3 : size;

   /* Update our record of the extents of the object (which is important */
   /* for color and resizing). */

   if (NULL == vo->blocks)
   {
      min[0] = max[0] = *data;
      min[1] = max[1] = data[1];
      min[2] = max[2] = data[2];
   }

   for(i = 0; i < *newwid * length; i += *newwid)
      for(j=0; j<size; j++)
      {
	 if (min[j] > data[i+j])
	    min[j] = data[i+j];
	 if (max[j] < data[i+j])
	    max[j] = data[i+j];
      }

   return data;
}

/* Fill in the list of blocks that we need to describe the points in the 
   set, and add them to the list of blocks of the view object.  The
   dimensionalities acceptable to the function depend on the color model
   specified for the object.  The first three dimensions of the function
   always appear on the screen (if there are only two, the function knows
   what to do).  In addition to these three, if there is a linear color
   dimension, it can occupy any position (1 2 3 or 4): this must be
   specified in advance and the function must have at this many dimensions.
   When the color model does not depend on a reference to the third
   coordinate of the object, the width of the object is set to 2, to
   indicate that the third parameter is optional.

   This whole process should work if the set data is stored either as
   integers or as reals, in any combination, in whatever ways MET can
   conjure up. */

static FOboolean VOSetToBlocks (vo, set)
   ViewObj vo;
   METset *set;

{
   FOboolean any = NO;
   int i=0, j;
   int size;
   int real_size = VO_STATIC_SIZE;
   int length, pt_part;
   int new_width, special;
   FOboolean possible = YES;
   METconnex *connex;
   METnumber *verts[VO_STATIC_SIZE];
   int width[VO_STATIC_SIZE];
   int field[VO_STATIC_SIZE];
   ViewBlock *new_block;
   char *message;
   ME(VOSetToBlocks);

   vo->blocks = NULL;

   while(1)
   {
      size = real_size;
      connex = METset_access_vertices(set, i, verts, width, field,
				      &length, &size, &special, &pt_part);

      if (special == MET_ACCESS_VERTICES_SPECIAL_NONE)
	 pt_part = size;
      
      if (MET_NULL_CONNEX == connex)  	/* problems */
      {
	 if (size > real_size)		/* array too small */
	 {
	    fprintf(stderr, "Error generating view object from set.\n");
	    i++;
	    any = YES;
	    continue;
	 }
	 else 				/* End of set */
	    break;
      }

      /* Can we make the conversion ? */
      any = YES;
      possible = YES;

      if (pt_part != vo->width)
	 if (vo->width != 2 || pt_part != 3)
	 {
	    message = "dimension is unaccounted for";
	    possible = NO;
	 }

      if (possible == YES)
	 for (j = 0; j < pt_part; j++)
	    if (field[j] != MET_FIELD_REAL &&
		field[j] != MET_FIELD_INTEGER)
	    {
	       possible = NO;
	       message = "type of set has unshowable field";
	       break;
	    }
      
      /* ok, then, let's do it */
      if (possible == YES)
      {
	 ALLOCM(new_block, ViewBlock);
	 new_block->vertices = VOConvertReps (vo, verts, pt_part, length, width,
					      field, &new_block->length,
					      &new_width, special,
					      size - pt_part);
	 MET_CONNEX_COPY(new_block->connex, connex);
	 /* remember where the text is because it isn't part of the */
	 /* vertices array (see how we only passed pt_part for its width) */
	 for (j = 0; j < size - pt_part; j++)
	    new_block->verts[j] = verts[pt_part + j];
	 for (j = 0; j < size - pt_part; j++)
	    new_block->width[j] = width[pt_part + j];
	 new_block->text_len = size - pt_part;
	 new_block->special = special;
	 new_block->next = vo->blocks;
	 vo->blocks = new_block;
      }
      else
      {
	 printf("Impossible to create a view object for this set.\n");
	 printf("%s.\n", message);
      }
      i++;		/* next block */
   }
   
   /* we've broken out of the loop.  If the view object has some */
   /* information, it means we could show some of the set. */
   
   return vo->blocks != NULL || !any;
}

/* This is the creation function for the view object object, as it were. 
   It requires an environment variable, for looking up the database color
   values on the X server (might as well let them take care of it, no?).
   It also takes the set that the view is to feature and the color
   specifications for the interpretation of the dimensions of the figure
   and the general beautification of the object.  The color specs should be
   a string as is explained in the ParseColors function; it may be NULL.
   The function returns NULL after printing out elucidating error
   messages if it fails; otherwise, you get an object back. */

ViewObj VOCreate (env, set, color)
   DrawEnv env;
   METset *set;
   char *color;

{
   ViewObj result;
   ME(VOCreate);

   ALLOCM(result, ViewObjRec);

   result->model = ParseColors(env, color);

   if (result->model == NULL)
   {	
      FREE(result, sizeof(ViewObjRec));
      return NULL;
   }

   switch (result->model->type) {
    case COLOR_MODEL_TYPE_NONE:
    case COLOR_MODEL_TYPE_CONSTANT:
      result->width = 2;
      break;
    case COLOR_MODEL_TYPE_LINEAR:
      if (result->model->info.linear.dimension < 2)
	 result->width = 2;
      else if (result->model->info.linear.dimension < 5)
	 result->width = result->model->info.linear.dimension + 1;
      else
      {
	 fprintf(stderr, "Dimension unaccounted for in display specs\n");
	 FREE(result->model, sizeof(ColorModelRec));
	 FREE(result, sizeof(ViewObjRec));
	 return NULL;
      }
      break;
    default:
      DEFAULT_ERR(result->model->type);
   }

   MET_SET_COPY(result->set, set);
   
   if (! VOSetToBlocks(result, set))
   {
      VODestroy(result);
      return NULL;
   }

   return result;
}

/* This is a helper function, for deleting all the information associated 
   with one particular "incarnation" of a set in a view object.  It is
   called both during updates and during destroys. */

static void VOFreeBlocks (vo)
   ViewObj vo;
{
   ViewBlockList curr, temp;
   int size;
   ME(VOFreeBlocks);
   
   curr = vo->blocks;

   while (curr != NULL) 
   {
      MET_CONNEX_FREE(curr->connex);
      if (curr->length > 0)
      {
	 size = curr->length * (vo->width == 2 ? 3 : vo->width);
	 FREEN(curr->vertices, Real, size);
      }
      temp = curr->next;
      FREE(curr, sizeof(ViewBlock));
      curr = temp;
   }
   vo->blocks = NULL;
}

/* Eliminates all the storage associated with the vo, including the 
   reference to the METset, the various points that might have been
   allocated in representation conversion, and the colormodel.  While the
   colormodel is freed, the actual colors in it are not: this happens when
   the view object is removed from the pict in which it resides.  So if you
   want the colors for other use, remove the vo from the pict it's in. */

void VODestroy (vo)
   ViewObj vo;

{
   ME(VODestroy);

   MET_SET_FREE(vo->set);
   VOFreeBlocks (vo);
   FREE(vo->model, sizeof(ColorModelRec));
   FREE(vo, sizeof(ViewObjRec));
}

/* If a call to METset have rep indicates that the set's values have 
   changed, then redo the conversion of data required for drawing,
   recalculate the extents of the object, and return true.  Otherwise,
   return false */

FOboolean VOUpdate (vo)
   ViewObj vo;
{
   FOboolean changed, t;
   ME(VOUpdate);
   
   time_begin();
   t = METset_have_rep(vo->set, &changed);
   time_end(FNULL, "set");
   if (0 == t)
      printf("\nfailed * *  * *  * *  * *  * *  * *  * *\n\n");
   if (changed == YES)
   {
      VOFreeBlocks(vo);
      VOSetToBlocks(vo, vo->set);
   }
   return changed;
}

#endif
