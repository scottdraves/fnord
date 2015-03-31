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


/* $Id: xthreed.h,v 1.12 1993/06/02 18:54:17 rch Exp $ */

#ifdef X_THREE_D

#ifndef XTHREEDH
#define XTHREEDH

#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <math.h>

#include "met/met.h"
#include "threedee/threedee.h"
#include "object/object.h"

typedef struct _Color {
   XColor info;
   int ref_count;
} Color;

/* The DrawEnv contains everything that you need to know
   to draw in X, more or less.  It is also the HQ data 
   structure for managing the color resources that it's
   fnord's job to munge as thoroughly as possible */

typedef struct _DrawEnvRec {
   Display *d;
   Pict to_draw;

   FILE *outputfile;
   char *filename;

   Colormap map;

   int dbuf_planes;
   int A_mask;
   XColor B_to_A[64];
   int B_mask;
   XColor A_to_B[64];
   GC gc[8];
   int pixel;
   int masks[6];

   Pixmap *patterns;

   FOboolean need_db;
   FOboolean need_file;
   char use_cmind;
   char A_shows;

   Color *colors;
   int num_colors;
   int num_allocked;
   int num_used;

   Color stat_cols[24];
} DrawEnvRec;

extern void EnvUseMask();

/* bsp_pict_num_verts depends on these */
#define PICT_TYPE_POINT    0
#define PICT_TYPE_LINE     1
#define PICT_TYPE_TRIANGLE 2
#define PICT_TYPE_TEXT_2D  3
#define PICT_TYPE_MARKER   4
#define PICT_TYPE_NO_DRAW  5

#define PICT_NEVER_DRAW  0
#define PICT_DRAW_IN_HSR 1
#define PICT_DRAW_IN_PS  3
#define PICT_ALWAYS_DRAW 7


/* This structure defines each of the mini-operations involved in 
   drawing a pict. */

typedef struct PictPolyStruct PictPolyStruct;
struct PictPolyStruct {
   Real norm[3]; /* Vector normal to the polygon to be drawn.  */
   Real dist; /* Finishes off the plane equation for the polygon.  */
};
   
#define PICT_TEXT_2D_MAX (sizeof(PictPolyStruct))

typedef struct PictCommand {
   
   char type;			/* Type of object to be drawn		*/
   char edge[3];		/* Tells whether each of the edges
				   should be drawn.			*/

   METreal *vertex[3];		/* Points into the vertex array
				   associated with the pict.		*/
   union {
      PictPolyStruct poly;
      char text_2d[PICT_TEXT_2D_MAX];
      int  marker;
   } info;
	 
   struct PictCommand *front;	/* Points to another tree of PictComm's	*/
   struct PictCommand *back;	/* --more of the same--			*/
   ViewObj source;		/* To determine what color to draw in   */
   union {
      struct PictCommand *next;
      short int ref_count;
   } mminfo;
   short int order_data;	/* Random storage for use by ordering
				   function */
#ifdef object__debug_magic
   int magic;
#endif
} PictCommand, *PictCommPtr;

/* This is basically random memory for the zillions of Reals we
   need when we're splitting polygons. */

typedef struct _PictPointRec {
   int num_points;		/* Number of reals in list		*/
   int num_left;		/* Number of those which are not used	*/
   Real *data;			/* Pointer to the vertices.		*/
   struct _PictPointRec *next;	/* Any other blocks of vertices		*/
} PictPointRec, *PictPointList;
#define  PICT_VERTEX_ADD_NUM 2000
#define PICT_STATIC_SIZE 6

/* The Pict manipulates the display of a series of view objects (METsets) 
   in a single window.  It contains not just the window information, but
   also a specification of the camera parameters.... */

typedef struct _PictRec {
   DrawEnv env;
   ViewObj objs;

   PictCommPtr commands;
   int numpts;
   int numlines;
   FOboolean sorted;

   short mode;
   short prev_mode;

   char draw_mode;
#define PICT_WIRE_FRAME 1
#define PICT_HIDDEN_SUR 2
   char dest_type;
#define ENV_DRAW_TO_PIXMAP 1
#define ENV_DRAW_TO_WINDOW 2
#define ENV_DRAW_TO_FILE 3
   char use_cmgcs;

   CameraRec cam;
   Window w;
   Pixmap p;
   Drawable dest;
   GC gc;
   XColor background;

   PictPointList points;
   
   Real max[3];
   Real min[3];
   Real win_min[3];
   Real win_max[3];
   Real padding;

   FOboolean being_drawn;		/* Private to the env which contains it */
   struct _PictRec *next;
} PictRec;

/* X specific structure used to initialize the values of a pict. */

typedef struct _PictXInitData {
   Window w;
   Pixmap p;
   GC gc;
   XColor background;
   int draw_mode;
} PictXInitData;

extern void PictChangePixmap();
extern void PictChangeWindow();
extern Real *pict_new_vertex();

#define VO_STATIC_SIZE (PICT_TEXT_2D_MAX + 1)


/* The ViewObj contains the drawing information peculiar to
   each set.  This consists of: the definition of the set, the
   vertices of the set in a canonical form, and the 
   definition of the way the set uses color. */

typedef struct _ViewBlock {
   int special;
   METconnex *connex;
   Real *vertices;
   int length;			/* Positive if verts need to be freed */
   int text_len;
   METnumber *verts[VO_STATIC_SIZE];
   int width[VO_STATIC_SIZE];
   struct _ViewBlock *next;
} ViewBlock, *ViewBlockList;

#define COLOR_MODEL_TYPE_NONE     0
#define COLOR_MODEL_TYPE_CONSTANT 1
#define COLOR_MODEL_TYPE_LINEAR   2

typedef struct _ColorModelRec {
   int type;
   FOboolean modelled;
   union {
      struct {
	 int dummy;
      } none;
      struct {
	 XColor color;
	 int index;
	 int db_index;
      } constant;
      struct {
	 int dimension;
	 XColor start;
	 XColor end;
	 int index[4];
	 int db_index[4];
      } linear;
   } info;
} ColorModelRec, *ColorModel;

typedef struct _ViewObjRec {
   METset *set;
   ViewBlockList blocks;
   int width;
#define VIEW_MAX_DIMS 6
   Real min[VIEW_MAX_DIMS];
   Real max[VIEW_MAX_DIMS];
   ColorModel model;
   struct _ViewObjRec *next;	/* Private to pict which contains it */
} ViewObjRec;

#define ALLOC_COMMAND(ptr) object_alloc(ptr, pict_free_cmd_head,\
					pict_new_list, object_type_bsp_node)

#define FREE_COMMAND(ptr) object_free(ptr, pict_object_free,\
				      object_type_bsp_node)
#define BSP_CARELESSNESS_FACTOR 10000.0

extern void pict_object_free();
extern void pict_new_list();
extern PictCommPtr pict_free_cmd_head;
#endif
#endif
