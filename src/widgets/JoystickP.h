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


/* $Id: JoystickP.h,v 1.2 1992/06/22 17:12:06 mas Exp $ */
#ifndef JOYSTICKPH
#define JOYSTICKPH

#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <math.h>

#include "Joystick.h"
/* New fields in the joystick record */

typedef struct {
/* Settable resources */
   Pixel edge;
   Pixel middle_hit;
   Pixel middle_over;
   int initial_interval;
   int repeat_interval;
   int page_size;
   XtCallbackList change_callback;

/* Data derived from resources */
   GC edge_GC;
   GC mid_hit_GC;
   GC mid_over_GC;
   GC erase_GC;
   Pixmap pix;
   int mouse_state;
   XtIntervalId timer;
   XPoint points[4][4];
   int radius;
   XPoint center;
   XPoint offset;
   int constrained;
   int trigger_key;
} JoystickPart;

/* Full instance record declaration */
typedef struct _JoystickRec {
   CorePart core;
   JoystickPart joystick;
} JoystickRec;

/* New fields for the Joystick widget class record */
typedef struct { int empty; } JoystickClassPart;

/* Full class record declaration */
typedef struct _JoystickClassRec {
   CoreClassPart core_class;
   JoystickClassPart joystick_class;
} JoystickClassRec;

/* Class record variable */
extern JoystickClassRec joystickClassRec;

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define CIRCLE 4

#define OUT 0
#define IN 0x0100
#define CLICK 0x0200
#define DRAG 0x0400
#define KEY 0x800

#define PLAIN_METHOD 0
#define OUT_METHOD 1
#define OVER_METHOD 2
#define HIT_METHOD 3
#define ERASE_METHOD 4

#define UNCONSTRAINED 0
#define ISCONSTRAINED 1
#define YCONSTRAINED 2
#define XCONSTRAINED 3

#define FULL_CIRCLE (64 * 360)
#endif
