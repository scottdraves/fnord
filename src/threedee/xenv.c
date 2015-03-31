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

#include "threedee/xthreed.h"

#ifndef lint
static char Version[] = "$Id: xenv.c,v 1.20 1993/06/02 18:54:13 rch Exp $";
#endif

static void EnvSetDefaultDBColors();
static void EnvSetDBColors();
static FOboolean EnvAllocDBColors();
static FOboolean EnvMoreColors();
static void EnvDitherPatterns();
static int EnvAllocColor();
static FOboolean EnvReferencedColors();

static void EnvOpenFile();
static void EnvCloseFile();
static void EnvSetupColors();
static void EnvFlipColors();

/* Wonderful little dithering patterns, hand-drawn by Cassidy Curtis */

static short patterns[] = {
   0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000,	/* 0/64 */

   0x8888, 0x0000, 0x0000, 0x0000,
   0x8888, 0x0000, 0x0000, 0x0000,
   0x8888, 0x0000, 0x0000, 0x0000,
   0x8888, 0x0000, 0x0000, 0x0000,     /* 4/64 */

   0x8888, 0x0000, 0x2222, 0x0000,
   0x8888, 0x0000, 0x2222, 0x0000,
   0x8888, 0x0000, 0x2222, 0x0000,
   0x8888, 0x0000, 0x2222, 0x0000,     /* 8/64 */

   0xAAAA, 0x0000, 0x2222, 0x0000,
   0xAAAA, 0x0000, 0x2222, 0x0000,
   0xAAAA, 0x0000, 0x2222, 0x0000,
   0xAAAA, 0x0000, 0x2222, 0x0000,	/* 12/64 */

   0xAAAA, 0x0000, 0xAAAA, 0x0000,
   0xAAAA, 0x0000, 0xAAAA, 0x0000,
   0xAAAA, 0x0000, 0xAAAA, 0x0000,
   0xAAAA, 0x0000, 0xAAAA, 0x0000,     /* 16/64 */

   0xAAAA, 0x4444, 0xAAAA, 0x0000,
   0xAAAA, 0x4444, 0xAAAA, 0x0000,
   0xAAAA, 0x4444, 0xAAAA, 0x0000,
   0xAAAA, 0x4444, 0xAAAA, 0x0000,	/* 20 /64*/

   0xAAAA, 0x4444, 0xAAAA, 0x1111,
   0xAAAA, 0x4444, 0xAAAA, 0x1111,
   0xAAAA, 0x4444, 0xAAAA, 0x1111,
   0xAAAA, 0x4444, 0xAAAA, 0x1111,     /* 24/64 */

   0xAAAA, 0x5555, 0xAAAA, 0x1111,
   0xAAAA, 0x5555, 0xAAAA, 0x1111,
   0xAAAA, 0x5555, 0xAAAA, 0x1111,
   0xAAAA, 0x5555, 0xAAAA, 0x1111,	/* 28 / 64 */

   0xAAAA, 0x5555, 0xAAAA, 0x5555,
   0xAAAA, 0x5555, 0xAAAA, 0x5555,
   0xAAAA, 0x5555, 0xAAAA, 0x5555,
   0xAAAA, 0x5555, 0xAAAA, 0x5555,     /* 32/64 */

   0xEEEE, 0x5555, 0xAAAA, 0x5555,
   0xEEEE, 0x5555, 0xAAAA, 0x5555,
   0xEEEE, 0x5555, 0xAAAA, 0x5555,
   0xEEEE, 0x5555, 0xAAAA, 0x5555,	/* 36/64 */

   0xEEEE, 0x5555, 0xBBBB, 0x5555,
   0xEEEE, 0x5555, 0xBBBB, 0x5555,
   0xEEEE, 0x5555, 0xBBBB, 0x5555,
   0xEEEE, 0x5555, 0xBBBB, 0x5555,     /* 40/64 */

   0xFFFF, 0x5555, 0xBBBB, 0x5555,
   0xFFFF, 0x5555, 0xBBBB, 0x5555,
   0xFFFF, 0x5555, 0xBBBB, 0x5555,
   0xFFFF, 0x5555, 0xBBBB, 0x5555,	/* 44/64 */

   0xFFFF, 0x5555, 0xFFFF, 0x5555,
   0xFFFF, 0x5555, 0xFFFF, 0x5555,
   0xFFFF, 0x5555, 0xFFFF, 0x5555,
   0xFFFF, 0x5555, 0xFFFF, 0x5555,     /* 48/64 */

   0xFFFF, 0xDDDD, 0xFFFF, 0x5555,
   0xFFFF, 0xDDDD, 0xFFFF, 0x5555,
   0xFFFF, 0xDDDD, 0xFFFF, 0x5555,
   0xFFFF, 0xDDDD, 0xFFFF, 0x5555,	/* 52/64 */

   0xFFFF, 0xDDDD, 0xFFFF, 0x7777,
   0xFFFF, 0xDDDD, 0xFFFF, 0x7777,
   0xFFFF, 0xDDDD, 0xFFFF, 0x7777,
   0xFFFF, 0xDDDD, 0xFFFF, 0x7777,     /* 56/64 */

   0xFFFF, 0xFFFF, 0xFFFF, 0x7777,
   0xFFFF, 0xFFFF, 0xFFFF, 0x7777,
   0xFFFF, 0xFFFF, 0xFFFF, 0x7777,
   0xFFFF, 0xFFFF, 0xFFFF, 0x7777,	/* 60/64 */

};

static Pixmap dither[16] = { None, None, None, None, None, None, None,
				None, None, None, None, None, None,
				None, None, None, }
;

/* this is totally at the wrong level of abstraction.  there should be
   PS procs for our primitives, rather than writing out code that
   generates the primitives. */

static char ps_header[] = 
"/m /moveto load def\n\
/p {1 0 360 arc fill} def\n\
/t {moveto show} def\n\
/l /lineto load def\n\
/f {closepath fill} def\n\
/F /fill load def\n\
/s {closepath stroke} def\n\
/S /stroke load def\n\
/g /setgray load def\n\
/G /setgray load def\n\
/setcmykcolor     % cyan magenta yellow black setcmykcolor -\n\
{\n\
1 sub 4 1 roll\n\
3\n\
	{\n\
	3 index add neg dup 0 lt\n\
		{\n\
		pop 0\n\
		} if\n\
	3 1 roll\n\
	} repeat\n\
setrgbcolor\n\
pop\n\
} def\n\
/k /setcmykcolor load def\n\
/K /setcmykcolor load def\n\
/c /setrgbcolor load def\n\
/C /setrgbcolor load def\n\
/B {gsave\n\
fill\n\
grestore\n\
} def\n\
/b {closepath\n\
gsave\n\
fill\n\
grestore\n\
} def";

static void EnvDitherPatterns (env)
   DrawEnv env;
{
   int i;
   int s = DefaultScreen(env->d);
   ME(EnvDitherPatterns);
   
   if (dither[0] == None)
   {
      for (i=0; i<16; i++)
	 dither[i] =
	    XCreateBitmapFromData (env->d, RootWindow(env->d, s), 
				   (char *) (patterns + 16 * i),
				   (unsigned) 16, (unsigned) 16, 0, 0);
   }
   env->patterns = dither;
}
					 
/* Puts a series of gray-scale colors into the 64 colors allocated for
   colormap double-buffering.  In addition, it fills in the values of the
   two 56 color arrays used to flip between the two states of the colormap
   (low 3 bits significant vs. high 3 bits significant).  The colors are
   put in what might be thought of as "reverse order"--since white is
   lowest, moving gradually to black, which is the last color. */

static void EnvSetDefaultDBColors(env, background)
   DrawEnv env;
   XColor *background;
{
   int c, h, l;
   ME(EnvSetDefaultDBColors);

   if (NO == env->A_shows) {
      for (c = 0; c < 64; c++) {
	 l = 8 - (c & 7);
	 if (l == 8)
	 {
	    env -> B_to_A[c].red = background->red;
	    env -> B_to_A[c].blue = background->blue;
	    env -> B_to_A[c].green = background->green;
	 }	
	 else
	    env -> B_to_A[c].red = env -> B_to_A[c].green =
	       env -> B_to_A[c].blue = l * 8192 - 1;
      }
   } else {
      for (c = 0; c < 64; c++) {
	 h = 8 - ((c & 56) >> 3);
	 if (h == 8)
	 {	
	    env -> A_to_B[c].red = background->red;
	    env -> A_to_B[c].blue = background->blue;
	    env -> A_to_B[c].green = background->green;
	 }
	 else
	    env -> A_to_B[c].red = env -> A_to_B[c].green =
	       env -> A_to_B[c].blue = h * 8192 - 1;
      }
   }
}

/* Uses the colors specified to set up the environment for colormap double 
   buffering.  The index field should be a set of indices into the color
   array of the environment, indicating the colors used by the objects
   which are going to be drawn in the near future.  Only the first count of
   these indices are used; if fewer than seven colors are specified, the
   rest of the values are not changed.  Note that in 64-color
   colormap double buffering, no more than 7 distinct colors (and a
   background color--which is automatically inserted here) can be
   displayed in the "flipping region" at once. */

static void EnvSetDBColors(env, background, index, count)
   DrawEnv env;
   XColor *background;
   int index[];
   int count;
{
   int c, h, l;
   ME(EnvSetDBColors);
   
   if (NO == env->A_shows) {
      for (c = 0; c < 64; c++) {
	 l =  c & 7 ;
      
	 if (l == 0)
	 {
	    env->B_to_A[c].red = background->red;
	    env->B_to_A[c].blue = background->blue;
	    env->B_to_A[c].green = background->green;
	 }
	 else if (--l < count)
	 {
	    env->B_to_A[c].red = env->colors[index[l]].info.red;
	    env->B_to_A[c].blue = env->colors[index[l]].info.blue;
	    env->B_to_A[c].green = env->colors[index[l]].info.green;
	 }
      }
   } else {
      for (c = 0; c < 64; c++) {
	 h = (c & 56) >> 3;
	 if (h == 0)
	 {
	    env->A_to_B[c].red = background->red;
	    env->A_to_B[c].blue = background->blue;
	    env->A_to_B[c].green = background->green;
	 }
	 else if (--h < count)
	 {
	    env->A_to_B[c].red = env->colors[index[h]].info.red;
	    env->A_to_B[c].blue = env->colors[index[h]].info.blue;
	    env->A_to_B[c].green = env->colors[index[h]].info.green;
	 }
      }
   }
}	

/* This is the initial function, called at creation time, which allocates 
   the color cells in the default colormap, and creates the gc's used to
   access it.  First it calls the X function to allocate 64 colors in the
   appropriate bit plane patterns for double buffering.  Then it creates
   gc's whose foregrounds access the pixels which remain constant when the
   colormaps flip--that is, the ones whose high 3 bits match their low 3
   bits (considering the bits we deal with, only).  Finally, it inserts the
   default gray-scale mapping into the new colors. */

static FOboolean EnvAllocDBColors(env)
   DrawEnv env;
{
   int i, j, c, pix;
   int screen = DefaultScreen(env->d);
   XGCValues values;
   ME(EnvAllocDBColors);
   
   if (! XAllocColorCells (env->d, env -> map, False,
			   env->masks, 6, & env->pixel, 1))
      return FAILURE;
   
   for (c = 0; c < 64; c++)
   {
      for (j = 0, pix = env->pixel; j < 6; j++)
	 if (c & (1 << j))
	    pix |= env->masks[j];
      env -> A_to_B[c].pixel = env -> B_to_A[c].pixel = pix;
      env -> A_to_B[c].flags = env -> B_to_A[c].flags = 
	 DoRed | DoBlue | DoGreen;
   }
   
   values.background = env->pixel;
   for (i = 0; i < 8; i++)
   {
      for (j = 0, pix = env->pixel; j < 3; j++)
	 if (i & (1 << j))
	    pix |= env->masks[j] | env->masks[j+3];
      values.foreground = pix;
      env -> gc[i] = XCreateGC (env->d, RootWindow(env->d, screen),
				GCForeground | GCBackground, &values);
   }
   
   env->A_mask = env->pixel | env->masks[2] | env->masks[1] | env->masks[0];
   env->B_mask = env->pixel | env->masks[5] | env->masks[4] | env->masks[3];
   env->A_shows = TRUE;

   return SUCCESS;
}

/* EnvReferenceColors makes a tabulation of all the colors that will be 
   used in the drawing of everything currently set to be displayed in the
   environment.  It scans through each of the objects in each of the picts
   on the drawing list, counting the colors used in the objects' color
   models.  Each time it encounters a color, the index into the color
   array is stored in the passed array, and the double buffering index of
   that color is set to one more than the current number of colors.  (This
   `one more' arises from the fact that the first color, never passed, will
   always be the background.)  If more than the specified limit on the
   number of colors is encountered, the function returns failure, and
   gray-scale double buffering should be used.  Otherwise, the function
   returns success, indicating that if the double buffering colors are
   reset using the returned array and maximum, then the view objects can be
   double buffered in the correct colors. */

static FOboolean EnvReferencedColors (env, background, index, max)
   DrawEnv env;
   XColor *background;
   int index[];
   int *max;
{
   int count = 0, i;
   int look;
   FOboolean safe;
   Pict curr_pict;
   ViewObj curr_obj;
   ME(EnvReferencedColors);

   curr_pict = env->to_draw;
   *background = curr_pict->background;

   for (; curr_pict != NULL; curr_pict = curr_pict->next) {
      if (NO == curr_pict->use_cmgcs)
	 continue;

      for (curr_obj = curr_pict->objs; curr_obj != NULL;
	   curr_obj = curr_obj->next)
      {
	 switch (curr_obj->model->type)
	 {
	  case COLOR_MODEL_TYPE_NONE:
	    break;
	  case COLOR_MODEL_TYPE_CONSTANT:
	    /* Look to see whether we have this color already */
	    for (look = 0, safe = False; look < count; look++)
	       if (index[look] == curr_obj->model->info.constant.index)
	       {
		  curr_obj->model->info.constant.db_index = look+1;
		  safe = True;
		  break;
	       }
	    if (True == safe) break;
	    if (count == *max)
	       return FAILURE;

	    index[count++] = curr_obj->model->info.constant.index;
	    curr_obj->model->info.constant.db_index = count;
	    break;

	  case COLOR_MODEL_TYPE_LINEAR:
	    for (i=0; i<4; i++)
	    {
	       /* Look to see whether we have this color already */
	       for (look = 0, safe = False; look < count; look++)
		  if (index[look] == curr_obj->model->info.constant.index)
		  {
		     curr_obj->model->info.constant.db_index = look+1;
		     safe = True;
		     break;
		  }

	       if (True == safe) break;
	       if (count == *max)
		  return FAILURE;

	       index[count++] = curr_obj->model->info.linear.index[i];
	       curr_obj->model->info.linear.db_index[i] = count;
	    }
	    break;

	  default:
	    DEFAULT_ERR(curr_obj->model->type);
	 }
      }
   }
   
   *max = count;
   return SUCCESS;
}

/* This function allocates a new block of colors for later appropriation
   by fnord.  If it fails, it should first try to reclaim the colors used
   for colormap double-buffering.  */

static FOboolean EnvMoreColors (env)
   DrawEnv env;
{
   int i;
   int pixels[24];
   ME(EnvMoreColors);

   if (!XAllocColorCells (env->d, env->map, False, &i, 0, pixels, 24))
      return FAILURE;

   if (env->colors == env->stat_cols)
   {
      ALLOCNM (env->colors, Color, env->num_colors + 24);
   }
   else
   {
      REALLOCNM (env->colors, Color, env->num_colors + 24, 
		 env->num_colors);
   }

   for (i=0; i<24; i++)
   {
      env->colors[env->num_colors + i].info.pixel = pixels[i];
      env->colors[env->num_colors + i].ref_count = 0;
   }

   env->num_colors += 24;
   env->num_allocked += 24;
   
   return SUCCESS;
}

/* Fills a colormap entry in the environment with the color passed
   to it.  If the environment has no more colors, the function tries to get
   more from the X server.  If this fails, the function just returns the
   index of the closest color so far allocated---rather dumb, but this
   should be a fairly rare occurrence.  If there have been no colors
   successfully allocated (ie. we're in black and white) it returns
   BAD_INDEX. */
#define BAD_INDEX (-1)

static int EnvAllocColor (env, color)
   DrawEnv env;
   XColor *color;
{
   int i;
   Real dist;
   Real mdist;
   ME(EnvAllocColor);

   /* Look to see if the user has already asked for this color, or if
      there is a free color around to use. */

   for (i = 0; i < env->num_used; i++)
      if ((env->colors[i].info.red == color->red) &&
	  (env->colors[i].info.green == color->green) &&
	  (env->colors[i].info.blue == color->blue))
      {
	 env->colors[i].ref_count ++;
	 color->pixel = env->colors[i].info.pixel;
	 return i;
      }
      else if (env->colors[i].ref_count == 0)
	 break;

   /* If not, grab some more colors from the X Server if you can.
      If you can't, get the closest color you can find that we've asked for
      already.  If even that fails, return BAD_INDEX. */

   if (env->num_used == env->num_allocked)
      if (FAILURE == EnvMoreColors (env))
      {
	 if (env->num_allocked == 0)
	    return BAD_INDEX;
	 
	 mdist = 3.0 * 65535 * 65535;
	 for ( i = 0; i < env->num_allocked; i++)
	 {
	    dist = (Real) ((env->colors[i].info.red - color->red) *
			   (env->colors[i].info.red - color->red)) +
		   (Real) ((env->colors[i].info.blue - color->blue) *
			   (env->colors[i].info.blue - color->blue)) +
	           (Real) ((env->colors[i].info.green - color->green) *
			   (env->colors[i].info.green - color->green));
	    if (mdist > dist)
	       mdist = dist;
	 }
      }

   env->num_used++;
   color->pixel = env->colors[i].info.pixel;
   env->colors[i].info = *color;
   env->colors[i].ref_count = 1;
   XStoreColor (env->d, env->map, &env->colors[i].info);

   return i;
}

/* EnvCreate generates a drawing environment for X.  It allocates space, 
   installs default values (NULLS, etc.), and sets up the color resources
   that fnord will try to take advantage of.  This involves allocating the
   64 colrors used in colormap double-buffering, along with an initial set
   of colors for use in drawing in the window normally.  (These colors will
   be given interesting values when view objects which need additional
   colors are declared and put into view widgets....*/

DrawEnv EnvCreate (display)
   Display *display;
   
{
   DrawEnv result;
   int pixels[24];
   int i;
   int screen = DefaultScreen(display);
   ME(EnvCreate);
   
   ALLOCM(result, DrawEnvRec);

   /* Do some random initialization. */

   result -> d = display;
   result -> to_draw = (Pict) NULL;
   result -> colors = result -> stat_cols;
   result -> num_colors = 0;
   result -> map = DefaultColormap(display, screen);
   result -> filename = NULL;
   result -> outputfile = NULL;

   /* Make the dither patterns used by the env */
   EnvDitherPatterns(result);

   /* Allocate a few colors for use in drawing. */
   if (XAllocColorCells (display, result->map, False, 
		     &i, 0, pixels, 24))
   {
      for (i = 0; i < 24; i++)
      {
	 result->colors[i].info.pixel = pixels[i];
	 result->colors[i].ref_count = 0;
      }
      result->num_allocked = 24;
      result->num_used = 0;
   }
   else
   {
      result->num_allocked = 0;
      result->num_used = 0;
      result->colors = NULL;
   }

   /* Set things up for double-buffering using the colormap. */
   if (EnvAllocDBColors(result) == SUCCESS) {
      EnvFlipColors(result);
      result->dbuf_planes = 3;
   }
   else
      result->dbuf_planes = 0;

   result->need_db = result->need_file = NO;

   return result;
}

static void EnvOpenFile (env)
   DrawEnv	env;
{
   char 	*filename = ((env->filename == CNULL) ? 
			     "fnord.ps" : env->filename);
   Pict		curr;
   Real		min[2], max[2];
   FOboolean 	set;

   int		num_read;
   char		*ps_include;
   char		buffer[10240];	/* do we really want to snarf this much? */
   FILE		*ps_file;
   ME(EnvOpenFile);

   /* Actually open the file.... */

   env->outputfile = fopen(filename, "w");
   
   if (env->outputfile == FNULL) {
      (void) fprintf(stderr, "Unable to open text output file %s.\n",
		     filename);
      return;
   }


   /* compute bounding box */

   curr = env->to_draw; 
   set = FALSE;
   while (curr != NULL) {
      if (ENV_DRAW_TO_FILE == curr->dest_type)
	 if (FALSE == set) {
	    min[0] = curr->win_min[0]; 
	    min[1] = curr->win_min[1];
	    max[0] = curr->win_max[0];
	    max[1] = curr->win_max[1];
	    set = TRUE;
	 } else {
	    min[0] = MIN(min[0], curr->win_min[0]);
	    min[1] = MIN(min[1], curr->win_min[1]);
	    max[0] = MAX(max[0], curr->win_max[0]);
	    max[1] = MAX(max[1], curr->win_max[1]);
	 }
      curr = curr->next;
   }

   /* include the postscript header information */

   fprintf(env->outputfile, "%%!\n%%%%Creator: fnord\n");
   fprintf(env->outputfile, "%%%%BoundingBox: %g %g %g %g\n", 
	   min[0], max[1], max[0], - min[1]);
   
   if ((ps_include = getenv ("FNORD_PS_INCLUDE")) == CNULL ||
       (ps_file = fopen(ps_include, "r")) == FNULL)
   {
      fwrite (ps_header, sizeof(char), 
	      sizeof(ps_header)-1, env->outputfile);
   }
   else
   {
      while (!feof(ps_file))
      {
	 num_read = fread(buffer, sizeof(char), 10240, ps_file);
	 fwrite(buffer, sizeof(char), num_read, env->outputfile);
      }
      fclose(ps_file);
   }
   
   /*  argghh... why do this rather than fix the camera? */
   fprintf(env->outputfile, "\n0 %g translate\n", - min[1]);
   fprintf(env->outputfile, "%%%%EndProlog\n%%%%BeginSetup\n");
   fprintf(env->outputfile, "/Times-Roman findfont 10 scalefont setfont\n");
   fprintf(env->outputfile, "0 setlinewidth\n%%%%EndSetup\n");
}   

static void EnvCloseFile (env)
   DrawEnv	env;
{
   ME(EnvCloseFile);

   if (FNULL != env->outputfile) {
      (void) fprintf(env->outputfile, "%%%%Trailer\nshowpage\n");
      fclose (env->outputfile);
   }
   env->outputfile = FNULL;
}

static void EnvSetupColors (env)
   DrawEnv	env;
{
   XColor	background;
   int	        i, colors[7];
   ME(EnvSetupColors);

   i = (1 << env->dbuf_planes) - 1;
   if (EnvReferencedColors(env, &background, colors, &i)) {
      env->use_cmind = YES;
      EnvSetDBColors(env, &background, colors, i);
   }
   else {
      env->use_cmind = NO;
      EnvSetDefaultDBColors(env, &background);
   }
}

void EnvUseMask (env, mask)
   DrawEnv	env;
   int 		mask;
{
   int  	i;
   ME(EnvUseMask);

   if (env->dbuf_planes != 0)
      for (i = 0; i < 8; i++)
	 XSetPlaneMask (env->d, env->gc[i], mask);
}

static void EnvFlipColors (env)
   DrawEnv	env;
{
   ME(EnvFlipColors);
   
   XStoreColors (env->d, env->map, 
		 (env->A_shows) ? env->A_to_B : env->B_to_A, 64);
   env->A_shows = ! env->A_shows;
}

void EnvFlush (env)
   DrawEnv	env;
{
   Pict		curr;
   ME(EnvFlush);

   if (YES == env->need_file)
      EnvOpenFile (env);

   if (YES == env->need_db)
      EnvSetupColors (env);

   for (curr = env->to_draw; curr != NULL; curr = curr->next) {
      PictOutput (curr);
      curr->being_drawn = NO;
   }

   if (YES == env->need_db)
      EnvFlipColors (env);

   if (YES == env->need_file)
      EnvCloseFile (env);

   XFlush(env->d);

   env->need_db = env->need_file = NO;
   env->to_draw = (Pict) NULL;
}

void EnvDestroy (env)
   DrawEnv env;
{
   ME(EnvDestroy);

   XFreeColors (env->d, env->map, &env->pixel, 1,
		env->A_mask | env->B_mask);
   
   FREE(env, sizeof(DrawEnvRec));
}

void EnvDrawAdd (env, pict, mode)
   DrawEnv env;
   Pict pict;
   int mode;

{
   ME(EnvDrawAdd);

   if (YES != pict->being_drawn)
   {
      pict->being_drawn = YES;
      PictSetDrawStyle(pict, mode);
      if (ENV_DRAW_TO_FILE == pict->dest_type)
	 env->need_file = YES;
      if (YES == pict->use_cmgcs)
	 env->need_db = YES;
      pict -> next = env -> to_draw;
      env -> to_draw = pict;
   }
}


FOboolean EnvRealizeColors (env, cmodel)
   DrawEnv env;
   ColorModel cmodel;
{
   int i, rinc, ginc, binc;
   XColor temp;
   ME(EnvRealizeColors);

   switch (cmodel->type)
   {
    case COLOR_MODEL_TYPE_NONE:
      break;
    case COLOR_MODEL_TYPE_CONSTANT:
      cmodel->info.constant.index = 
	 EnvAllocColor (env, &cmodel->info.constant.color);
      break;
    case COLOR_MODEL_TYPE_LINEAR:
      rinc = ((int) cmodel->info.linear.end.red - (int)
	 cmodel->info.linear.start.red) / 3;
      ginc = ((int) cmodel->info.linear.end.green - (int)
	 cmodel->info.linear.start.green) / 3;
      binc = ((int) cmodel->info.linear.end.blue - (int)
	 cmodel->info.linear.start.blue) / 3;
      for (i = 0, temp = cmodel->info.linear.start; i < 4; i++,
	   temp.red += rinc, temp.blue += binc, temp.green += ginc)
      {
	 cmodel->info.linear.index[i] =
	    EnvAllocColor (env, &temp);
      }
      break;
    default:
      DEFAULT_ERR(cmodel->type);
   }
   
   cmodel->modelled = TRUE;
}

FOboolean EnvFreeColors (env, cmodel)
   DrawEnv env;
   ColorModel cmodel;
{
   int i;
   ME(EnvFreeColors);

   if (! cmodel->modelled)
      return;

   cmodel->modelled = FALSE;

   switch (cmodel->type)
   {
    case COLOR_MODEL_TYPE_NONE:
      break;
    case COLOR_MODEL_TYPE_CONSTANT:
      i = cmodel->info.constant.index;
      if (BAD_INDEX != i)
	 env->colors[i].ref_count--;
      break;
    case COLOR_MODEL_TYPE_LINEAR:
      break;
    default:
      DEFAULT_ERR(cmodel->type);
   }
}

/* Sets the file that the environment will read to as passed */

void EnvSetOutputFile (env, s)
   DrawEnv env;
   char *s;

{
   int len;
   ME(EnvSetOutputFile);
  
   if (env->filename != NULL)
      FREE_STR(env->filename);

   len = strlen(s);
   ALLOCNM(env->filename, char, len);
   (void) strcpy(env->filename, s);
}
