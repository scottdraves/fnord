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


/* $Id: Joystick.h,v 1.2 1992/06/22 17:12:03 mas Exp $ */

#ifndef JOYSTICKH
#define JOYSTICKH

/* New resources for my widget */

#define XtNfillColor "fillColor"
#define XtNhighlightColor "highlightColor"
#define XtCFillColor "FillColor"

#define XtNdelayRepeat "delayRepeat"
#define XtNrepeatGap   "repeatGap"
#define XtCInterval    "Interval"

#define XtNpageSize "pageSize"
#define XtCDistance "Distance"

#ifndef XtNchangeCallback 
#define XtNchangeCallback "changeCallback"
#endif

#ifndef XtCChangeCallback 
#define XtCChangeCallback "ChangeCallback"
#endif

/* Class record pointer */
extern WidgetClass joystickWidgetClass;

/* C Widget type definition */
typedef struct _JoystickRec *JoystickWidget;

/* callback struct */
typedef struct _JoystickCallbackStruct {
   int more_follow;
   int x;
   int y;
} JoystickCallbackStruct;

extern char joystick_accelerators[];
#endif
