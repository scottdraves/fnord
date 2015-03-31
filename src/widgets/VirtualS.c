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

/**********************************************************************
 *
 *	A virtual sphere camera control widget
 *
 **********************************************************************/

#ifndef lint
static char Version[]=
   "$Id: VirtualS.c,v 1.10 1992/06/22 17:12:23 mas Exp $";
#endif

#include "VirtualSP.h"

static int init_padding = 20;
static Dimension init_threshold = 10;
static int init_stop_time = 300;

/* Forward definitions of procedures */

/* Called from Xt */
static void Reset();
static void TrackStart();
static void Track();
static void TrackEnd();
static void Initialize();
static void Resize();
static void Realize();
static void LookDownXAxis();
static void LookDownYAxis();
static void LookDownZAxis();
static void LookUpXAxis();
static void LookUpYAxis();
static void LookUpZAxis();

/* Useful Routines */
static void vs_2d_to_3d();
static void vs_pts_to_spin();

static XtActionsRec actionTable[] = {
   	{ "TrackStart", TrackStart },
	{ "Track", Track },
	{ "TrackEnd", TrackEnd },
	{ "Reset", Reset },
	{ "LookDownXAxis", LookDownXAxis },
	{ "LookDownYAxis", LookDownYAxis },
	{ "LookDownZAxis", LookDownZAxis },
	{ "LookUpXAxis", LookUpXAxis },
	{ "LookUpYAxis", LookUpYAxis },
	{ "LookUpZAxis", LookUpZAxis },
};

static char translations[] =
   	"<Key>r:		Reset()\n\
	 :<Key>x:		LookDownXAxis()\n\
	 :<Key>y:		LookDownYAxis()\n\
	 :<Key>z:		LookDownZAxis()\n\
	 :<Key>X:		LookUpXAxis()\n\
	 :<Key>Y:		LookUpYAxis()\n\
	 :<Key>Z:		LookUpZAxis()\n\
         <Btn1Down>:		TrackStart()\n\
	 <Btn1Motion>:		Track()\n\
	 <Btn1Up>:		TrackEnd()";





/* Resource specific to the virtual sphere */
static XtResource resources[] = {
{ XtNpadding, XtCPadding, XtRInt, sizeof(int),
  XtOffset(VirtualSphereWidget, vs.padding),
  XtRInt, (caddr_t) &init_padding },
{ XtNstopTime, XtCStopTime, XtRInt, sizeof(int),
  XtOffset(VirtualSphereWidget, vs.stop_time),
  XtRInt, (caddr_t) &init_stop_time },
{ XtNthreshold, XtCThreshold, XtRDimension, sizeof(Dimension),
  XtOffset(VirtualSphereWidget, vs.d2_threshold),
  XtRDimension, (caddr_t) &init_threshold },
{ XtNchangeCallback, XtCChangeCallback, XtRCallback,
     sizeof(XtCallbackList), XtOffset(VirtualSphereWidget,
				      vs.change_callbacks),
     XtRString, NULL},
{XtNaccelerators, XtCAccelerators, XtRAcceleratorTable,
    sizeof(XtTranslations), XtOffset(VirtualSphereWidget, core.accelerators), 
    XtRString, translations},
};

/* Class record constant */
VirtualSphereClassRec virtualSphereClassRec = {
{
   /* Core class fields */
   /* superclass	*/	(WidgetClass) &widgetClassRec,
   /* class_name	*/	"VirtualSphere",
   /* widget_size	*/	sizeof(VirtualSphereRec),
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
WidgetClass virtualSphereWidgetClass =
   (WidgetClass) &virtualSphereClassRec;


void
vs_spin_get(w, spin)
   Widget w;
   Spin spin;
{
   VirtualSphereWidget vs = (VirtualSphereWidget) w;

   spin[0] = vs->vs.spin[0];
   spin[1] = vs->vs.spin[1];
   spin[2] = vs->vs.spin[2];
   spin[3] = vs->vs.spin[3];
}

void 
vs_spin_incr(w)
   Widget	w;
{
   static int i = 1;
   VirtualSphereWidget vs = CAST(VirtualSphereWidget, w);
   
   if (VS_SPINNING == vs->vs.state ||
       VS_TRACKING == vs->vs.state) {
      
      spin_compose(vs->vs.spin, vs->vs.spin, vs->vs.last_spin);
   
      if (0 == i--) {
	 spin_normalize(vs->vs.spin);
	 i = 100;
      }
   }
}
      
static void
vs_2d_to_3d(vs, x, y, p)
   VirtualSphereWidget vs;
   Real x,y;
   Real p[3];
{
   Real norm2, norm;

   p[0] = (x - vs->vs.center[0]) / vs->vs.radius;
   p[1] = (y - vs->vs.center[1]) / vs->vs.radius;

   /* if the point is outside the projected sphere, it lies on the
    * edge of the projected sphere.  Set the z coordinate to 0 and
    * normalize. */
   norm2 = p[0]*p[0] + p[1]*p[1];
   if (norm2 >= 1.0)
   {
      norm = SQRT(norm2);
      p[0] /= norm;
      p[1] /= norm;
      p[2] = 0.0;
   }
   /* otherwise, the point is inside the projected sphere.  Find the
    * z-coordinate which will make it normalized */
   else
      p[2] = SQRT(1.0 - norm2);
}

static void
vs_pts_to_spin(p0, p1, spin)
   Real p0[3], p1[3];
   Spin   spin;
{
   Real axis[3], angle;

   /* axis is the cross product of p0 and p1 */
   axis[0] = p0[1] * p1[2] - p0[2] * p1[1];
   axis[1] = p0[2] * p1[0] - p0[0] * p1[2];
   axis[2] = p0[0] * p1[1] - p0[1] * p1[0];

   /* angle is acos of dot product */
   angle = ACOS(p0[0] * p1[0] + p0[1] * p1[1] + p0[2] * p1[2]);

   spin_from_axis(spin, axis, angle);
}

static void Initialize(request, new)
   VirtualSphereWidget request;
   VirtualSphereWidget new;
{
   Real axis[3];
   /* A good size is 144 by 144 pixels */
   if (request -> core.width == 0)  new -> core.width  = 144;
   if (request -> core.height == 0) new -> core.height = 144;

   /* Initialize everything */
   new->vs.center[0] = 0.0;
   new->vs.center[1] = 0.0;
   new->vs.radius = 1.0;

   axis[0] = axis[1] = axis[2] = 1.0; /* doesn't matter */
   spin_from_axis(new->vs.spin, axis, 0.0);

   new->vs.state = VS_WAITING;

   /* If this is anything other that zero
      it will generate a bad match! */
   new->core.border_width = 0;

   Resize(new);
}

static void Resize(vs)
   VirtualSphereWidget vs;
{
   vs->vs.center[0] = (Real) vs->core.width / 2.0;
   vs->vs.center[1] = - (Real) vs->core.height / 2.0;
   vs->vs.radius = (Real) (vs->core.width > vs->core.height ?
			     vs->core.width : vs->core.height) / 2;
   vs->vs.radius -= (Real) vs->vs.padding;
}

static void 
Realize(vs, value_mask, attributes)
   Widget vs;
   XtValueMask *value_mask;
   XSetWindowAttributes *attributes;
{
   /* Just to make sure: the border width must be zero */
   vs->core.border_width = 0;
   vs->core.depth = 0;
   /* And only these attributes can be set */
#if 0
   *value_mask &= CWWinGravity | CWEventMask | CWDontPropagate |
      CWOverrideRedirect | CWCursor;
#endif

   *value_mask = CWEventMask;
   attributes->event_mask = NoEventMask;

   XtCreateWindow (vs, InputOnly, (Visual *)CopyFromParent,
		   *value_mask, attributes);

}

/* ARGSUSED */
static void
TrackStart(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   VirtualSphereCallbackStruct vscs;

   switch (vs->vs.state) {

    case VS_TRACKING:
      break;

    case VS_SPINNING:
    case VS_WAITING:
      vscs.old_state = vs->vs.state;
      
      vs->vs.state = VS_TRACKING;
      vs_2d_to_3d(vs, (Real) event->xbutton.x, 
		  - (Real) event->xbutton.y, vs->vs.last_pt);
      vs->vs.last_x = vs->vs.last_y = -1; 
      vs->vs.last_dist = 0;
      vs->vs.last_time = event->xbutton.time;

      vscs.more_follow = True;
      vscs.draw_now = False;
      vscs.state = vs->vs.state;
      XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscs);

      break;
   }
}

/* ARGSUSED */
static void
Track(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   Real   this_pt[3];
   VirtualSphereCallbackStruct vscbs;
   
   if (vs->vs.state != VS_TRACKING)
      return;
   
   vs_2d_to_3d(vs, (Real) event->xmotion.x, 
	       - (Real) event->xmotion.y, this_pt);

   if (-1 != vs->vs.last_x) 
      vs->vs.last_dist = (event->xmotion.x - vs->vs.last_x) *
	 (event->xmotion.x - vs->vs.last_x) +
	    (event->xmotion.y - vs->vs.last_y) *
	       (event->xmotion.y - vs->vs.last_y);

   vs->vs.last_x = event->xmotion.x;
   vs->vs.last_y = event->xmotion.y;
   vs->vs.last_time = event->xmotion.time;

   vs_pts_to_spin(vs->vs.last_pt, this_pt, vs->vs.last_spin);

   vs_spin_incr((Widget) vs);

   vscbs.more_follow = True;
   vscbs.draw_now = True;
   vscbs.state = vs->vs.state;
   vscbs.old_state = vs->vs.state;
   XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscbs);

   vs->vs.last_pt[0] = this_pt[0];
   vs->vs.last_pt[1] = this_pt[1];
   vs->vs.last_pt[2] = this_pt[2];
}

/* ARGSUSED */
static void
TrackEnd(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   VirtualSphereCallbackStruct vscbs;

   vscbs.old_state = vs->vs.state;

   if (-1 == vs->vs.last_x)
      vs->vs.state = VS_WAITING;

   else {
      if (vs->vs.last_dist > vs->vs.d2_threshold &&
	  event->xbutton.time - vs->vs.last_time < vs->vs.stop_time)
	 vs->vs.state = VS_SPINNING;
      else 
	 vs->vs.state = VS_WAITING;
   }

   vscbs.more_follow = False;
   vscbs.draw_now = False;
   vscbs.state = vs->vs.state;

   XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscbs);
}

/* ARGSUSED */
static void
Reset(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   Real axis[3];
   VirtualSphereCallbackStruct vscbs;

   axis[0] = axis[1] = axis[2] = 1.0;
   spin_from_axis(vs->vs.spin, axis, 0.0);

   vscbs.more_follow = False;
   vscbs.draw_now = True;
   vscbs.state = vs->vs.state;
   vscbs.old_state = vs->vs.state;
   XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscbs);
}

/* gag */
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/* ARGSUSED */
static void
LookDownXAxis(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   Real axis[3];
   VirtualSphereCallbackStruct vscbs;

   axis[0] = axis[2] = 0.0;
   axis[1] = 1.0;
   spin_from_axis(vs->vs.spin, axis, -M_PI_2);

   vscbs.more_follow = False;
   vscbs.draw_now = True;
   vscbs.state = vs->vs.state;
   vscbs.old_state = vs->vs.state;
   XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscbs);
}

/* ARGSUSED */
static void
LookUpXAxis(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   Real axis[3];
   VirtualSphereCallbackStruct vscbs;

   axis[0] = axis[2] = 0.0;
   axis[1] = 1.0;
   spin_from_axis(vs->vs.spin, axis, M_PI_2);

   vscbs.more_follow = False;
   vscbs.draw_now = True;
   vscbs.state = vs->vs.state;
   vscbs.old_state = vs->vs.state;
   XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscbs);
}

/* ARGSUSED */
static void
LookDownYAxis(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   Real axis[3];
   VirtualSphereCallbackStruct vscbs;

   axis[1] = axis[2] = 0.0;
   axis[0] = 1.0;
   spin_from_axis(vs->vs.spin, axis, M_PI_2);

   vscbs.more_follow = False;
   vscbs.draw_now = True;
   vscbs.state = vs->vs.state;
   vscbs.old_state = vs->vs.state;
   XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscbs);
}

/* ARGSUSED */
static void
LookUpYAxis(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   Real axis[3];
   VirtualSphereCallbackStruct vscbs;

   axis[1] = axis[2] = 0.0;
   axis[0] = 1.0;
   spin_from_axis(vs->vs.spin, axis, -M_PI_2);

   vscbs.more_follow = False;
   vscbs.draw_now = True;
   vscbs.state = vs->vs.state;
   vscbs.old_state = vs->vs.state;
   XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscbs);
}
/* ARGSUSED */
static void
LookDownZAxis(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   Real axis[3];
   VirtualSphereCallbackStruct vscbs;

   axis[1] = axis[2] = 1.0;
   axis[0] = 1.0;
   spin_from_axis(vs->vs.spin, axis, 0.0);

   vscbs.more_follow = False;
   vscbs.draw_now = True;
   vscbs.state = vs->vs.state;
   vscbs.old_state = vs->vs.state;
   XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscbs);
}

/* ARGSUSED */
static void
LookUpZAxis(vs, event, parms, num)
   VirtualSphereWidget vs;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   Real axis[3];
   VirtualSphereCallbackStruct vscbs;

   axis[1] = axis[2] = 0.0;
   axis[0] = 1.0;
   spin_from_axis(vs->vs.spin, axis, M_PI);

   vscbs.more_follow = False;
   vscbs.draw_now = True;
   vscbs.state = vs->vs.state;
   vscbs.old_state = vs->vs.state;
   XtCallCallbacks ((Widget) vs, XtNchangeCallback, (caddr_t) &vscbs);
}
