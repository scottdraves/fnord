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

/* $Id: PointP.h,v 1.2 1992/06/22 17:12:13 mas Exp $ */

/* The virtual sphere camera control widget continues. */

#ifndef POINTPH_INCLUDED
#define POINTPH_INCLUDED

#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <math.h>

#include "Point.h"

typedef struct {
   XtCallbackList change_callbacks, drag_callbacks;

/* the state */
   int  x, y;

} PointPart;

/* Full instance record declaration */
typedef struct _PointRec {
   CorePart  core;
   PointPart point;
} PointRec;

/* New fields for the Virtual sphere widget class record */
typedef struct { int empty; } PointClassPart;

/* Full class record declaration */
typedef struct _PointClassRec {
   CoreClassPart core_class;
   PointClassPart point_class;
} PointClassRec;

/* Class record variable */
extern PointClassRec pointClassRec;

#endif 
