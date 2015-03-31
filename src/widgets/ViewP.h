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


/* $Id: ViewP.h,v 1.2 1992/06/22 17:12:20 mas Exp $ */

#ifndef _ViewP_h
#define _ViewP_h


#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <math.h>

#include "widgets/View.h"
#include "threedee/xthreed.h"

/* $Id: ViewP.h,v 1.2 1992/06/22 17:12:20 mas Exp $ */

/* New fields for the view widget instance record */
typedef struct {
   DrawEnv	env;		/* environment to draw it in */
   int		padding;	/* padding around the bounding box */
   
   /* instance variables, not xrdb-settable */
   Pict		pict;		/* picture structure */
   GC		gc;		/* pointer to GraphicsContext */

} ViewPart;

/* Full instance record declaration */
typedef struct _ViewRec {
   CorePart core;
   ViewPart view;
} ViewRec;

/* New fields for the View widget class record */
typedef struct {
   int dummy;
} ViewClassPart;

/* Full class record declaration. */
typedef struct _ViewClassRec {
   CoreClassPart core_class;
   ViewClassPart view_class;
} ViewClassRec;

/* Class pointer. */
extern ViewClassRec viewClassRec;

#endif /* _ViewP_h */
