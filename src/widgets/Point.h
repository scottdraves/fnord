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

/* $Id: Point.h,v 1.3 1992/06/22 17:12:10 mas Exp $ */

/* a widget for picking points in the plane */

#ifndef POINTH_INCLUDED
#define POINTH_INCLUDED

#include "global/float.h"


#ifndef XtNchangeCallback 
#define XtNchangeCallback "changeCallback"
#endif

#ifndef XtNdragCallback 
#define XtNdragCallback "dragCallback"
#endif

#ifndef XtCChangeCallback 
#define XtCChangeCallback "ChangeCallback"
#endif

/* Class record pointer */
extern WidgetClass pointWidgetClass;

/* C Widget type definition */
typedef struct _PointRec *PointWidget;

typedef struct {
   int 		code;
} PointCallbackStruct;

#define POINT_MOUSE_DOWN	1
#define POINT_MOUSE_MOVED	2
#define POINT_MOUSE_LIFTED	3

extern void point_get_loc();

#endif
