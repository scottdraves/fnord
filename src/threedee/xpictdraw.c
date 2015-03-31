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
static char Version[]=
   "$Id: xpictdraw.c,v 1.22 1993/06/02 18:54:15 rch Exp $";
#endif

#define X_CNVT_PT(v,m,i) \
(short) ((Real) (v)[0] * (m)[i][0] + (Real) (v)[1] * (m)[i][1] +\
	 (Real) (v)[2] * (m)[i][2] + (Real) (m)[i][3] + .5)

#define PS_CNVT_PT(v,m,i) \
((Real) (v)[0] * (m)[i][0] + (Real) (v)[1] * (m)[i][1] +\
	 (Real) (v)[2] * (m)[i][2] + (Real) (m)[i][3])

#define DO_FIRST 1
#define DO_SECOND 2
#define DO_THIRD  4

void xdraw_point();
void xdraw_line();
void xdraw_triangle();
void xfill_point();
void xfill_line();
void xfill_triangle();

static GC xpd_select_gc();
static void xpd_rgb();
static void xpd_intensity();

/* Determine the rgb values with which the passed pict command should be 
   drawn.  Used for postscript output.  The red, green, and blue values
   returned are scaled so that (0,0,0) is black and (1,1,1) is white. In
   this function and in the next, flag is a combination of "DO_FIRST",
   "DO_SECOND", and "DO_THIRD" that indicates what vertices in the object
   should be counted when the function averages over the vertices of the
   object to determine what color to draw in. */

static void xpd_rgb (pict, comm, flag, r, g, b)
   Pict pict;
   PictCommPtr comm;
   int flag;
   Real *r;
   Real *g;
   Real *b;
{
   ColorModel m = comm->source->model;
   int i;
   Real percent;
   int num;
   ME(xpd_rgb);

   switch (m->type)
   {
    case COLOR_MODEL_TYPE_NONE:
      *r = *g = *b = (Real) 0;
      return;

    case COLOR_MODEL_TYPE_CONSTANT:
      *r = ((Real) m->info.constant.color.red) / 65535.0;
      *g = ((Real) m->info.constant.color.green) / 65535.0;
      *b = ((Real) m->info.constant.color.blue) / 65535.0;
      return;

    case COLOR_MODEL_TYPE_LINEAR:
      for (num = 0, percent = 0.0, i = 0; i <= 3; i++)
	 if (flag & (1 << i))
	 {
	    percent += comm->vertex[i][m->info.linear.dimension];
	    num++;
	 }
      percent /= (Real) num;
      if (percent < 0.0)
	 percent = 0.0;
      else if (percent > 1.0)
	 percent = 1.0;
      
      *r = (Real) m->info.linear.start.red +
	 percent * (Real) ((int) m->info.linear.end.red -
			   (int) m->info.linear.start.red);
      *g = (Real) m->info.linear.start.green +
	 percent * (Real) ((int) m->info.linear.end.green -
			   (int) m->info.linear.start.green);
      *b = (Real) m->info.linear.start.blue +
	 percent * (Real) ((int) m->info.linear.end.blue -
			   (int) m->info.linear.start.blue);
      *r /= 65535.0;
      *g /= 65535.0;
      *b /= 65535.0;
      return;

    default:
      DEFAULT_ERR(m->type);
   }
}

/* A simple function, but one often used. */

static void xpd_intensity (pict, comm, flags, i)
   Pict pict;
   PictCommPtr comm;
   int flags;
   Real *i;

{
   Real r, g, b;

   xpd_rgb (pict, comm, flags, &r, &g, &b);

   *i = 0.299 * r + 0.587 * g + 0.114 * b;
}


/* Determine which of the 8 gc's associated with the environment for 
   colormap double-buffering best fits the color that a command
   is supposed to be drawn in; returns that gc. */

static GC xpd_select_gc (pict, alter_gc, comm, flag)
   Pict pict;
   GC alter_gc;
   PictCommPtr comm;
   int flag;
{
   DrawEnv env = pict->env;
   Real r, g, b;
   int i, num;
   long mask;
   Real percent;
   ColorModel m = comm->source->model;
   XGCValues values;
   FOboolean good = YES;
   ME(xpd_select_gc);


   switch (m->type) {
    case COLOR_MODEL_TYPE_NONE:
      mask = GCForeground | GCBackground | GCFillStyle;
      values.foreground = BlackPixel (env->d, DefaultScreen(env->d));
      values.background = WhitePixel (env->d, DefaultScreen(env->d));
      values.fill_style = FillSolid;
      XChangeGC (env->d, alter_gc, mask, &values);
      return alter_gc;
      
    case COLOR_MODEL_TYPE_CONSTANT:
      if (pict->use_cmgcs == NO) {	
	 if (-1 != (values.foreground = m->info.constant.color.pixel)) {
	    values.background = pict->background.pixel;
	    values.fill_style = FillSolid;
	    XChangeGC (env->d, alter_gc, 
		       GCForeground | GCBackground | GCFillStyle,
		       &values);
	    return alter_gc;
	 } else
	    good = NO;
      } else if (env->use_cmind == YES)
	 return env->gc [ m->info.constant.db_index ];
      
      r = (Real) m->info.constant.color.red;
      g = (Real) m->info.constant.color.green;
      b = (Real) m->info.constant.color.blue;
      break;

    case COLOR_MODEL_TYPE_LINEAR:
      for (num = 0, percent = 0.0, i = 0; i <= 3; i++)
	 if (flag & (1 << i)) {
	    percent += comm->vertex[i][m->info.linear.dimension];
	    num++;
	 }
      percent /= (Real) num;
      if (percent < 0.0)
	 percent = 0.0;
      else if (percent > 1.0)
	 percent = 1.0;
      if (pict->use_cmgcs == NO) {
	 mask = GCForeground | GCBackground | GCFillStyle;
	 i = (int) (percent * 3.0);
	 if (i == 3) i--;
	 if (-1 != m->info.linear.index[i+1] &&
	     -1 != m->info.linear.index[i]) {
	    values.foreground = 
	       env->colors[ m->info.linear.index[i+1] ].info.pixel;
	    values.background =
	       env->colors[ m->info.linear.index[i] ].info.pixel;
	    i = IRINT((percent * 3.0 - (Real) i) * 16);
	    if (i == 16)
	       values.fill_style = FillSolid;
	    else {
	       values.fill_style = FillOpaqueStippled;
	       values.stipple = env->patterns[i];
	       mask |= GCStipple;
	    }
	    XChangeGC (env->d, alter_gc, mask, &values);
	    return alter_gc;
	 } else 
	    good = NO;
      }
      else if (env->use_cmind == YES)
      {
	 i = IRINT(percent * 3);
	 return env->gc [ m->info.linear.db_index[i] ];
      }

      r = (Real) m->info.linear.start.red + 
	 percent * (Real) ((int) m->info.linear.end.red -
			   (int) m->info.linear.start.red);
      g = (Real) m->info.linear.start.green + 
	 percent * (Real) ((int) m->info.linear.end.green -
			   (int) m->info.linear.start.green);
      b = (Real) m->info.linear.start.blue + 
	 percent * (Real) ((int) m->info.linear.end.blue -
			   (int) m->info.linear.start.blue);
      break;

    default:
      DEFAULT_ERR(m->type);
   }

   percent = 0.299 * r + 0.587 * g + 0.114 * b;
   
   if (good == YES)
   {
      i = 7 - (int) percent / 16500;
      return (env->gc[i]);
   }
   else
   {
      mask = GCForeground | GCBackground | GCFillStyle | GCStipple;
      values.foreground = BlackPixel (env->d, DefaultScreen(env->d));
      values.background = WhitePixel (env->d, DefaultScreen(env->d));
      values.fill_style = FillOpaqueStippled;
      i = 15 - (int) percent / 4096.0;
      values.stipple = env->patterns[i];
      XChangeGC (env->d, alter_gc, mask, &values);
      return alter_gc;
   }
}

/* Put a point into the vector of points to draw. Relatively quick, but */
/* requires that it doesn't matter what order anything is drawn in. */

void xdraw_point(pict, comm)
   Pict pict;
   PictCommPtr comm;

{
   int x, y;
   Real *vert = comm->vertex[0];

   ME(xdraw_point);
   
   x = X_CNVT_PT(vert, pict->cam.xform, 0);
   y = - X_CNVT_PT(vert, pict->cam.xform, 1);
   XDrawPoint (pict->env->d, pict->dest,
	       xpd_select_gc(pict, pict->gc, comm, DO_FIRST), x, y);
}

void xdraw_text_2d(pict, comm)
   Pict pict;
   PictCommPtr comm;

{
   int x, y;
   Real *vert = comm->vertex[0];

   ME(xdraw_point);
   
   x = X_CNVT_PT(vert, pict->cam.xform, 0);
   y = - X_CNVT_PT(vert, pict->cam.xform, 1);
   XDrawString (pict->env->d, pict->dest,
	       xpd_select_gc(pict, pict->gc, comm, DO_FIRST), x, y,
		comm->info.text_2d, strlen(comm->info.text_2d));
}
void xfill_text_2d(pict, comm)
   Pict pict;
   PictCommPtr comm;

{
   int x, y;
   Real *vert = comm->vertex[0];

   ME(xdraw_point);
   
   x = X_CNVT_PT(vert, pict->cam.xform, 0);
   y = - X_CNVT_PT(vert, pict->cam.xform, 1);
   XDrawString (pict->env->d, pict->dest,
	       xpd_select_gc(pict, pict->gc, comm, DO_FIRST), x, y,
		comm->info.text_2d, strlen(comm->info.text_2d));
}


/* Draw a point right away.  Inefficient but necessary for hidden-surface
removal. */

void xfill_point(pict, comm)
   Pict pict;
   PictCommPtr comm;
{
   int x, y;
   Real *vert = comm->vertex[0];

   ME(xhsr_point);

   x = X_CNVT_PT(vert, pict->cam.xform, 0);
   y =  - X_CNVT_PT(vert, pict->cam.xform, 1);

   XDrawPoint (pict->env->d, pict->dest,
	       xpd_select_gc(pict, pict->gc, comm, DO_FIRST), x, y);
}


/* Put a line onto the list of lines to draw.  Makes things fast, but again */
/* depends on all your lines being able to be displayed together regardless */
/* of what else has been displayed. */

void xdraw_line(pict, comm)
   Pict pict;
   PictCommPtr comm;
{
   int x1, y1, x2, y2;
   Real *vert = comm->vertex[0];

   ME(xdraw_point);
   
   x1 = X_CNVT_PT(vert, pict->cam.xform, 0);
   y1 = - X_CNVT_PT(vert, pict->cam.xform, 1);

   vert = comm->vertex[1];

   x2 = X_CNVT_PT(vert, pict->cam.xform, 0);
   y2 = - X_CNVT_PT(vert, pict->cam.xform, 1);

   XDrawLine (pict->env->d, pict->dest, 
	      xpd_select_gc(pict, pict->gc, comm, DO_FIRST | DO_SECOND),
	      x1, y1, x2, y2);

}


/* Draw a line right away.  It doesn't treat the X server nicely, but if */
/* you need to do hidden surface removal there's not much you can do about */
/* it. */
void xfill_line(pict, comm)
   Pict pict;
   PictCommPtr comm;
{
   int x1, y1, x2, y2;
   Real *vert = comm->vertex[0];

   ME(xhsr_line);
   
   x1 = X_CNVT_PT(vert, pict->cam.xform, 0);
   y1 = - X_CNVT_PT(vert, pict->cam.xform, 1);

   vert = comm->vertex[1];

   x2 = X_CNVT_PT(vert, pict->cam.xform, 0);
   y2 = - X_CNVT_PT(vert, pict->cam.xform, 1);

   XDrawLine(pict->env->d, pict->dest, 
	     xpd_select_gc(pict, pict->gc, comm, DO_FIRST | DO_SECOND),
	     x1, y1, x2, y2);
}


/* This function, designed for drawing wire-frame surfaces, computes the */
/* lines of which the triangle is made and adds them to the list of lines */
/* to be drawn when xfinish is called. */


void xdraw_triangle(pict, command)
   Pict pict;
   PictCommPtr command;
{
   XPoint edges[3];
   int i;
   Real *vert;
   
   ME(xdraw_triangle);

   for(i=0;i<3;i++)
   {
      vert = command->vertex[i];
      edges[i].x =  X_CNVT_PT(vert, pict->cam.xform, 0);
      edges[i].y =  - X_CNVT_PT(vert, pict->cam.xform, 1);
   }

   for(i=0;i<3;i++)
      if (PICT_ALWAYS_DRAW == (PICT_ALWAYS_DRAW & command->edge[i]))
	 XDrawLine(pict->env->d, pict->dest, 
		   xpd_select_gc(pict, pict->gc, command, 
				 (1 << i) | (1 << (i+1)%3)),
		   edges[i].x, edges[i].y,
		   edges[(i+1)%3].x, edges[(i+1)%3].y);

}


/* This function fills the triangle on the screen (in white, for now).  In */
/* addition, it draws the edges of the triangle on the screen (in black, */
/* for now) if the edge flag for that edge is set in the command. */

void xfill_triangle(pict, command)
   Pict pict;
   PictCommPtr command;
{
   XPoint edges[3];
   int i;
   GC gc;
   Real *vert;
   
   ME(xfill_triangle);

   for ( i = 0 ; i < 3 ; i++)
   {
      vert = command->vertex[i];
      edges[i].x =  X_CNVT_PT(vert, pict->cam.xform, 0);
      edges[i].y =  - X_CNVT_PT(vert, pict->cam.xform, 1);
   }
   XFillPolygon(pict->env->d, pict->dest, 
		xpd_select_gc(pict, pict->gc, command, 
			      DO_FIRST | DO_SECOND | DO_THIRD),
		edges, 3, Convex, CoordModeOrigin);
   
   if (pict->use_cmgcs == YES)
      gc = pict->env->gc[7];
   else
   {
      gc = pict->gc;
      XSetForeground (pict->env->d, gc, 
#ifdef SITE_BROWN
		      /* this is what we want here */
		      BlackPixel(pict->env->d,
				  DefaultScreen(pict->env->d))
#else		      
		      /* even if this looks better elsewhere */
		      WhitePixel(pict->env->d,
				  DefaultScreen(pict->env->d))
#endif
		      );
      XSetFillStyle (pict->env->d, gc, FillSolid);
   }

   for (i = 0; i < 3 ; i++)
      if (PICT_DRAW_IN_HSR == (PICT_DRAW_IN_HSR & command->edge[i]))
	 XDrawLine(pict->env->d, pict->dest, gc, edges[i].x,
		   edges[i].y, edges[(i+1)%3].x, edges[(i+1)%3].y);
}

/* Adobe illustrator works in a flavor of postscript---but the color
   can only be set using the printing cmyk convention.  So this is
   a quick and easy conversion from rgb.  This is *not* the most
   sophisticated conversion, but it works. */

void rgb_to_cmyk(r,g,b,c,m,y,k)
   Real		r,g,b;
   Real		*c, *m, *y, *k;
{
   ME(rgb_to_cmyk);

   *c = 1.0 - r;
   *m = 1.0 - g;
   *y = 1.0 - b;
   *k = MIN(*c, *m);
   *k = MIN(*k, *y);
}

/* This function writes the appropriate description for a point out to
   the open file. */

void print_point (pict, comm)
   Pict pict;
   PictCommPtr comm;
{
   Real x0, y0;
   Real *vert = comm->vertex[0];
   Real r,g,b;
   Real c,m,y,k;
   ME(print_point);

   x0 = PS_CNVT_PT(vert, pict->cam.xform, 0);
   y0 = PS_CNVT_PT(vert, pict->cam.xform, 1);
   if (pict->mode != ENV_BW_POSTSCRIPT_MODE )
   {
      if (pict->mode == ENV_GRAY_POSTSCRIPT_MODE)
      {
	 xpd_intensity (pict, comm, DO_FIRST, &b);
	 fprintf (pict->env->outputfile, "%.3f G ", b);
      }
      else
      {
	 xpd_rgb (pict, comm, DO_FIRST, &r, &g, &b);
	 rgb_to_cmyk (r, g, b, &c, &m, &y, &k);
	 fprintf (pict->env->outputfile, "%.3f %.3f %.3f %.3 K ", 
		  c, m, y, k);

      }
   }
   fprintf(pict->env->outputfile, "%.2f %.2f p S\n", x0, y0);
}

void print_text_2d (pict, comm)
   Pict pict;
   PictCommPtr comm;
{
   Real x, y;
   Real *vert = comm->vertex[0];
   ME(print_point);

   x = PS_CNVT_PT(vert, pict->cam.xform, 0);
   y = PS_CNVT_PT(vert, pict->cam.xform, 1);
   fprintf(pict->env->outputfile, "(%s) %.2f %.2f t S\n",
	   comm->info.text_2d, x, y);
}

/* This function writes the appropriate description for a line out to
   the open file. */

void print_line (pict, comm)
   Pict pict;
   PictCommPtr comm;
{
   Real x1, y1, x2, y2;
   Real *vert = comm->vertex[0];
   Real r, g, b;
   Real c, m, y, k;
   ME(print_point);
   
   x1 = PS_CNVT_PT(vert, pict->cam.xform, 0);
   y1 = PS_CNVT_PT(vert, pict->cam.xform, 1);

   vert = comm->vertex[1];

   x2 = PS_CNVT_PT(vert, pict->cam.xform, 0);
   y2 = PS_CNVT_PT(vert, pict->cam.xform, 1);


   if (pict->mode != ENV_BW_POSTSCRIPT_MODE)
   {
      if (pict->mode == ENV_GRAY_POSTSCRIPT_MODE)
      {
	 xpd_intensity (pict, comm, DO_FIRST | DO_SECOND, &b);
	 fprintf (pict->env->outputfile, "%.3f G ", b);
      }
      else
      {
	 xpd_rgb (pict, comm, DO_FIRST | DO_SECOND, &r, &g, &b);
	 rgb_to_cmyk (r, g, b, &c, &m, &y, &k);
	 fprintf (pict->env->outputfile, "%.3f %.3f %.3f %.3f K ", c, m, y, k);
      }
   }
   fprintf(pict->env->outputfile, "%.2f %.2f m %.2f %.2f l S\n",
	   x1, y1, x2, y2);

}

/* This function writes the appropriate description for a triangle out to
   the open file. */

void print_triangle (pict, command)
   Pict pict;
   PictCommPtr command;
{
   int i;
   METreal *vert;
   Real xedges[3], yedges[3];
   Real r, g, b;
   Real c, m, y, k;
   ME(print_triangle);
   
   for(i=0;i<3;i++)
   {
      vert = command->vertex[i];
      xedges[i] =  PS_CNVT_PT(vert, pict->cam.xform, 0);
      yedges[i] =  PS_CNVT_PT(vert, pict->cam.xform, 1);
   }
   
   for(i=0;i<3;i++)
      if (PICT_ALWAYS_DRAW == (PICT_ALWAYS_DRAW & command->edge[i]))
      {
	 if (pict->mode != ENV_BW_POSTSCRIPT_MODE)
	 {
	    if (pict->mode == ENV_GRAY_POSTSCRIPT_MODE)
	    {
	       xpd_intensity (pict, command, 
			      (1 << i) | (1 << (i+1)%3), &b);
	       fprintf (pict->env->outputfile, "%.3f G ", b);
	    }
	    else
	    {
	       xpd_rgb (pict, command, 
			(1 << i) | (1 << (i+1)%3), &r, &g, &b);
	       rgb_to_cmyk (r, g, b, &c, &m, &y, &k);
	       fprintf (pict->env->outputfile, "%.3f %.3f %.3f %.3f K ", c,
			m, y, k);
	    }
	 }
	 fprintf(pict->env->outputfile, "%.2f %.2f m %.2f %.2f l S\n",
		 xedges[i], yedges[i], xedges[(i+1)%3], yedges[(i+1)%3]);
      }
}

/* This function writes the appropriate description for a filled triangle
   out to the open file. */

void print_filled_triangle (pict, command)
   Pict pict;
   PictCommPtr command;
{
   int i;
   METreal *vert;
   Real xedges[3], yedges[3];
   Real r, g, b;
   Real c, m, y, k;
   ME(print_filled_triangle);
   
   for(i=0;i<3;i++)
   {
      vert = command->vertex[i];
      xedges[i] =  PS_CNVT_PT(vert, pict->cam.xform, 0);
      yedges[i] =  PS_CNVT_PT(vert, pict->cam.xform, 1);
   }
   
   if (pict->mode != ENV_BW_POSTSCRIPT_MODE)
   {
      if (pict->mode == ENV_GRAY_POSTSCRIPT_MODE)
      {
	 xpd_intensity (pict, command, DO_FIRST | DO_SECOND | DO_THIRD, &b);
	 fprintf (pict->env->outputfile, "%.3f g ", b);
      }
      else
      {
	 xpd_rgb (pict, command, DO_FIRST | DO_SECOND | DO_THIRD, 
		  &r, &g, &b);
	 rgb_to_cmyk (r, g, b, &c, &m, &y, &k);
	 fprintf (pict->env->outputfile, "%.3f %.3f %.3f %.3f k ",
		  c, m, y, k);
      }
   } else {
      fprintf (pict->env->outputfile, "1 g ");
   }

   fprintf(pict->env->outputfile, 
	   "%.2f %.2f m %.2f %.2f l %.2f %.2f l f S 0 G\n",
	   xedges[0], yedges[0], xedges[1], yedges[1], xedges[2],
	   yedges[2]);

   for(i=0;i<3;i++)
      if (PICT_DRAW_IN_PS == (PICT_DRAW_IN_PS & command->edge[i]))
      {
	 fprintf(pict->env->outputfile, "%.2f %.2f m %.2f %.2f l S\n",
		 xedges[i], yedges[i], xedges[(i+1)%3], yedges[(i+1)%3]);
      }
}

