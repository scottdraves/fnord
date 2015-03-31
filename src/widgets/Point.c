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

#ifndef lint
static char Version[]=
   "$Id: Point.c,v 1.4 1992/06/22 17:12:08 mas Exp $";
#endif

#include "PointP.h"

/* Forward definitions of procedures */

/* Called from Xt */
static void Initialize();
static void Resize();
static void Realize();
static void TrackStart();
static void Track();
static void TrackEnd();

static XtActionsRec actionTable[] = {
{ "TrackStart", TrackStart },
{ "Track", Track },
{ "TrackEnd", TrackEnd },
} ;

static char translations[] =
   "<Btn2Down>:		TrackStart()\n\
    <Btn2Motion>:	Track()\n\
    <Btn2Up>:		TrackEnd()"
   ;

static XtResource resources[] = {
{ XtNchangeCallback, XtCChangeCallback, XtRCallback,
     sizeof(XtCallbackList), XtOffset(PointWidget,
				      point.change_callbacks),
     XtRString, NULL},
{ XtNdragCallback, XtCChangeCallback, XtRCallback,
     sizeof(XtCallbackList), XtOffset(PointWidget,
				      point.drag_callbacks),
     XtRString, NULL},
{XtNaccelerators, XtCAccelerators, XtRAcceleratorTable,
    sizeof(XtTranslations), XtOffset(PointWidget, core.accelerators), 
    XtRString, translations},
};

/* Class record constant */
PointClassRec pointClassRec = {
{
   /* Core class fields */
   /* superclass	*/	(WidgetClass) &widgetClassRec,
   /* class_name	*/	"Point",
   /* widget_size	*/	sizeof(PointRec),
   /* class_initialize	*/	NULL,
   /* class_part_init 	*/	NULL,
   /* class_inited	*/	FALSE,
   /* initialize	*/	Initialize,
   /* initialize_hook	*/	NULL,
   /* realize		*/	Realize,
   /* actions		*/	actionTable,
   /* num_actions	*/	XtNumber(actionTable),
   /* resources		*/	resources,
   /* num_resources	*/	XtNumber(resources),
   /* xrm_class		*/	NULLQUARK,
   /* compress_motion	*/	TRUE,
   /* compress_exposure	*/	TRUE,
   /* compress_enter/lv	*/	TRUE,
   /* visible interest	*/	FALSE,
   /* destroy		*/	NULL,
   /* resize		*/	Resize,
   /* expose		*/	XtInheritExpose,
   /* set_values	*/      NULL,
   /* set_values_hook	*/	NULL,
   /* set_vals almost	*/	XtInheritSetValuesAlmost,
   /* get_values_hook	*/	NULL,
   /* accept_focus	*/	NULL,
   /* version		*/ 	XtVersion,
   /* callback offsets	*/	NULL,
   /* tm table		*/	translations,
   /* query_geometry     */	XtInheritQueryGeometry,
   /* display_accelerator*/	XtInheritDisplayAccelerator,
   /* extension          */	NULL
   }
};

/* Class record pointer */
WidgetClass pointWidgetClass =
   (WidgetClass) &pointClassRec;

/*
 * place last location into loc
 */
void
point_get_loc(w, loc)
   Widget	w;
   int	       *loc;
{
   PointWidget	pw = (PointWidget) w;

   loc[0] = pw->point.x;
   loc[1] = pw->point.y;
}

static void Initialize(request, new)
   PointWidget request;
   PointWidget new;
{
   /* A good size is 144 by 144 pixels */
   if (request -> core.width == 0)  new -> core.width  = 144;
   if (request -> core.height == 0) new -> core.height = 144;

   /* If this is anything other that zero
      it will generate a bad match! */
   new->core.border_width = 0;

   Resize(new);
}

/* ARGSUSED */
static void Resize(point)
   PointWidget point;
{
}

static void 
Realize(point, value_mask, attributes)
   Widget point;
   XtValueMask *value_mask;
   XSetWindowAttributes *attributes;
{
   /* Just to make sure: the border width must be zero */
   point->core.border_width = 0;
   point->core.depth = 0;
   /* And only these attributes can be set */
   *value_mask &= CWWinGravity | CWEventMask | CWDontPropagate |
      CWOverrideRedirect | CWCursor;

   *value_mask = CWEventMask;
   attributes->event_mask = NoEventMask;

   XtCreateWindow (point, InputOnly, (Visual *) CopyFromParent,
		   *value_mask, attributes);
}

/* ARGSUSED */
static void
TrackStart(point, event, parms, num)
   PointWidget point;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   PointCallbackStruct pcbs;
   
   point->point.x = event->xbutton.x;
   point->point.y = event->xbutton.y;
   pcbs.code = POINT_MOUSE_DOWN;
   XtCallCallbacks ((Widget) point, XtNdragCallback, (caddr_t) &pcbs);
}
/* ARGSUSED */
static void
Track(point, event, parms, num)
   PointWidget point;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   PointCallbackStruct pcbs;

   point->point.x = event->xmotion.x;
   point->point.y = event->xmotion.y;
   pcbs.code = POINT_MOUSE_MOVED;
   XtCallCallbacks ((Widget) point, XtNdragCallback, (caddr_t) &pcbs);
}
/* ARGSUSED */
static void
TrackEnd(point, event, parms, num)
   PointWidget point;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   PointCallbackStruct pcbs;
   pcbs.code = POINT_MOUSE_LIFTED;
   XtCallCallbacks ((Widget) point, XtNchangeCallback, (caddr_t) &pcbs);
}
