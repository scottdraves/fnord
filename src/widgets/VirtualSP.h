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

/* $Id: VirtualSP.h,v 1.7 1992/06/22 17:12:28 mas Exp $ */

/* The virtual sphere camera control widget continues. */

#ifndef VIRTUALSPH
#define VIRTUALSPH

#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <math.h>
#include "widgets/spin.h"

#include "VirtualS.h"

/* New fields in the virtual sphere record */

typedef struct {
/* Settable Resource--there are no graphics on a VS */
   XtCallbackList change_callbacks;
   int 		  padding;
   Dimension      d2_threshold;
   int		  stop_time;

/* Data representing the VS state */
   int  state;
   Real center[2];
   Real radius;
   Spin spin;
   Real last_pt[3];
   Spin last_spin;
   int  last_dist;
   int  last_time;
   int  last_x;
   int  last_y;

} VirtualSpherePart;

/* Full instance record declaration */
typedef struct _VirtualSphereRec {
   CorePart core;
   VirtualSpherePart vs;
} VirtualSphereRec;

/* New fields for the Virtual sphere widget class record */
typedef struct { int empty; } VirtualSphereClassPart;

/* Full class record declaration */
typedef struct _VirtualSphereClassRec {
   CoreClassPart core_class;
   VirtualSphereClassPart vs_class;
} VirtualSphereClassRec;

/* Class record variable */
extern VirtualSphereClassRec virtualSphereClassRec;

#define VS_EPSILON	(1.0e-5)
#define VS_PI		(3.14159265359)

#ifndef NULL
#define NULL	0
#endif

#endif 
