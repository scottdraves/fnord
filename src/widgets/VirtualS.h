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

/* $Id: VirtualS.h,v 1.6 1992/06/22 17:12:25 mas Exp $ */

/* A virtual sphere controller widget. */
#ifndef VIRTUALSH
#define VIRTUALSH

#include "global/float.h"


/* New resources for the virtual sphere */

#ifndef XtNchangeCallback 
#define XtNchangeCallback "changeCallback"
#endif

#ifndef XtCChangeCallback 
#define XtCChangeCallback "ChangeCallback"
#endif

#ifndef XtNpadding
#define XtNpadding "padding"
#endif

#ifndef XtCPadding
#define XtCPadding "Padding"
#endif

#ifndef XtNthreshold
#define XtNthreshold "threshold"
#endif

#ifndef XtCThreshold
#define XtCThreshold "Threshold"
#endif

#ifndef XtNstopTime
#define XtNstopTime  "stopTime"
#endif

#ifndef XtCStopTime
#define XtCStopTime  "StopTime"
#endif

/* Class record pointer */
extern WidgetClass virtualSphereWidgetClass;

/* C Widget type definition */
typedef struct _VirtualSphereRec *VirtualSphereWidget;

/* Callback struct */
typedef struct _VirtualSphereCallbackStruct {
   int more_follow;
   int draw_now;
   int state;
   int old_state;
} VirtualSphereCallbackStruct;

/* values for the state of a virtual sphere */
#define VS_WAITING 	0
#define VS_TRACKING	1
#define VS_SPINNING	2

extern void vs_spin_get();
extern void vs_spin_incr();

#endif
