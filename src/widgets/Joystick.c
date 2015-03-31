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

/* This file contains code for a widget developed more or less exclusively */
/* for fnord.  It operates a "virtual joystick" on the screen, and is, with */
/* luck, going to be featured prominently in future camera controls. */

#include "JoystickP.h"

#ifndef lint
static char Version[] = "$Id: Joystick.c,v 1.5 1992/06/22 17:12:01 mas Exp $";
#endif

static init_delay_repeat = 250;
static init_repeat_gap = 100;
static init_page_size = 800;

/* Resources specific to joystick */
static XtResource resources[] = {
{ XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
     XtOffset(JoystickWidget, joystick.edge),
     XtRString, "Black" },
{ XtNfillColor, XtCFillColor, XtRPixel, sizeof(Pixel),
     XtOffset(JoystickWidget, joystick.middle_hit),
     XtRString, "Gray60" },
{ XtNhighlightColor, XtCFillColor, XtRPixel, sizeof(Pixel),
     XtOffset(JoystickWidget, joystick.middle_over),
     XtRString, "Gray80" },
{ XtNdelayRepeat, XtCInterval, XtRInt, sizeof(int),
     XtOffset(JoystickWidget, joystick.initial_interval),
     XtRInt, (caddr_t) &init_delay_repeat },
{ XtNrepeatGap, XtCInterval, XtRInt, sizeof(int),
     XtOffset(JoystickWidget, joystick.repeat_interval),
     XtRInt, (caddr_t) &init_repeat_gap },
{ XtNpageSize, XtCDistance, XtRInt, sizeof(int),
     XtOffset(JoystickWidget, joystick.page_size),
     XtRInt, (caddr_t) &init_page_size },
{ XtNchangeCallback, XtCChangeCallback, XtRCallback, 
     sizeof(XtCallbackList), XtOffset(JoystickWidget,
				      joystick.change_callback),
     XtRString, NULL},
};

/* Forward definitions of procedures */
/* Called by Xt */
static void Timeout();
static void Initialize();
static void Resizer();
static void Redisplay();
static void MouseDown();
static void MouseUp();
static void Motion();
static void Leave();
static void KeyDown();
static void KeyUp();

/* Not called by Xt */
static void TriangleBox();
static Boolean MoveCircle();
static Boolean Inside();
static int InsideWhich();
static void ProcessClick();
static void RedrawComponent();

/* Interpretation schedule for event translation tables for this widget. */
static XtActionsRec actionTable[] = {
	{ "MouseDown", MouseDown },
	{ "MouseDown", MouseDown },
	{ "Motion", Motion },
        { "MouseUp", MouseUp },
	{ "Leave", Leave },
    	{ "KeyDown", KeyDown },
	{ "KeyUp", KeyUp },
};

/* The standard actions associated with this widget. */
static char translations[] = 
	"!Shift<BtnDown>:	MouseDown(constrained)\n\
	 None<BtnDown>:		MouseDown(unconstrained)\n\
         <BtnUp>:       	MouseUp()\n\
         <Leave>:		Leave()\n\
         <Motion>:      	Motion()";	

/* The default mappings of keys to be interpreted as corresponding to */
/* things in this widget. */
char joystick_accelerators[] = 
   	"#override \n\
	 <KeyRelease>:		KeyUp()\n\
         None<Key>Right:	KeyDown(+,0)\n\
	 None<Key>Left: 	KeyDown(-,0)\n\
	 None<Key>Up:   	KeyDown(0,-)\n\
	 None<Key>Down: 	KeyDown(0,+)";

/* Class record constant */
JoystickClassRec joystickClassRec = {
{
   /* Core class fields */
   /* superclass	*/	(WidgetClass) &widgetClassRec,
   /* class_name	*/	"Joystick",
   /* widget_size	*/	sizeof(JoystickRec),
   /* class_initialize	*/	NULL,
   /* class_part_init 	*/	NULL,
   /* class_inited	*/	FALSE,
   /* initialize	*/	Initialize,
   /* initialize_hook	*/	NULL,
   /* realize		*/	XtInheritRealize,
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
   /* resize		*/	Resizer,
   /* expose		*/	Redisplay,
   /* set_values	*/      NULL,
   /* set_values_hook	*/	NULL,
   /* set_vals almost	*/	XtInheritSetValuesAlmost,
   /* get_values_hook	*/	NULL,
   /* accept_focus	*/	NULL,
   /* version		*/ 	XtVersion,
   /* callback offsets	*/	NULL,
   /* tm table		*/	translations,
	}
};

/* Class record pointer */
WidgetClass joystickWidgetClass = 
   (WidgetClass) &joystickClassRec;

/* Called when a new widget is created, but after any default values have */
/* been set, this function fills in those values of the widget that remain */
/* unset, and make a few important allocations.  They get the GC's involved */
/* in drawing from the communal GC pool, create initial values for the */
/* triangles, and build the pixmap for double-buffered drawing.        */

static void Initialize(request, new)
   JoystickWidget request;
   JoystickWidget new;
{
   XGCValues values;
   XtGCMask  valueMask;

   /* A good size is 72 by 72 pixels */
   if (request -> core.width == 0)  new -> core.width  += 72;
   if (request -> core.height == 0) new -> core.height += 72;

   /* We need several GC's to draw with, but minimizing
      conversation with the server is a good thing. */
   valueMask = GCForeground | GCBackground | GCFillStyle;
   values.background = new -> core.background_pixel;
   values.fill_style = FillSolid;

   values.foreground = new -> joystick.edge;
   new -> joystick.edge_GC =
      XtGetGC ((Widget) new, valueMask, &values);

   values.foreground = new -> joystick.middle_hit;
   new -> joystick.mid_hit_GC =
      XtGetGC ((Widget) new, valueMask, &values);

   values.foreground = new -> joystick.middle_over;
   new -> joystick.mid_over_GC =
      XtGetGC ((Widget) new, valueMask, &values);

   values.foreground = new -> core.background_pixel;
   new -> joystick.erase_GC =
      XtGetGC ((Widget) new, valueMask, &values);

   new -> joystick.timer = None;

   /* Determine how big everything should be ... */
   Resizer (new);

}

/* This function is the expose method for the widget.  It just draws each */
/* of the components of the widget . */

/* ARGSUSED */
static void Redisplay(jw, event, region)
   JoystickWidget jw;
   XEvent *event;
   Region region;
{
   int method;
   int i;

   for( i = 1; i <= 5; i++)
   {
      if (( jw -> joystick.mouse_state & IN ) &&
	  ( jw -> joystick.mouse_state & ~IN ) == i)
	 method = OVER_METHOD;
      else if (( jw -> joystick.mouse_state & CLICK ) &&
	       ( jw -> joystick.mouse_state & ~CLICK ) == i)
	 method = HIT_METHOD;
      else if (( jw ->  joystick.mouse_state & DRAG ) &&
	       ( jw -> joystick.mouse_state & ~DRAG ) == i)
	 method = HIT_METHOD;
      else
	 method = PLAIN_METHOD;
      RedrawComponent(jw, event, i, method);
   }
}

/* This function redraws a piece of the widget.  The five components of a */
/* joystick are the four triangular buttons and the draggable sphere.  The */
/* function uses the data contained in the passed widget.  The event is */
/* used to specify the display and drawable where the action is to take */
/* place.  Comp is a number between 1 and 5 which indicates which of the */
/* peices of the joystick to draw.  Method is an indicator of what state */
/* the component should be displayed in: empty, highlighted (for when the */
/* pointer is over the component), hit (when there is an activation of the */
/* component), out (when the thing has become empty and needs to be cleared */
/* out), or erased (when the thing moves). */

static void RedrawComponent(jw, event, comp, method)
   JoystickWidget jw;
   XEvent *event;
   int comp;
   int method;
{
   GC right_GC;
   GC edge_GC;

   if (comp == OUT)
      return;
   

   switch (method)
   {
    case OUT_METHOD:
      right_GC = jw->joystick.erase_GC;
      edge_GC = jw->joystick.edge_GC;
      break;
    case ERASE_METHOD:
      right_GC = jw->joystick.erase_GC;
      edge_GC = jw->joystick.erase_GC;
      break;
    case OVER_METHOD:
      right_GC = jw->joystick.mid_over_GC;
      edge_GC = jw->joystick.edge_GC;
      break;
    case HIT_METHOD:
      right_GC = jw->joystick.mid_hit_GC;
      edge_GC = jw->joystick.edge_GC;
      break;
    default:
      right_GC = None;
      edge_GC = jw->joystick.edge_GC;
   }
   
   if (comp == CIRCLE + 1)
   {
      if (method == ERASE_METHOD)
      {
	 XFillRectangle (event->xany.display, event->xany.window, right_GC, 
			 jw->joystick.center.x - jw->joystick.radius - 1,
			 jw->joystick.center.y - jw->joystick.radius - 1,
			 (unsigned) 2 * jw->joystick.radius + 2,
			 (unsigned) 2 * jw->joystick.radius + 2);
      }
      else
      {
	 if (right_GC != None)
	    XFillArc (event->xany.display, event->xany.window, right_GC,
		      jw->joystick.center.x - jw->joystick.radius,
		      jw->joystick.center.y - jw->joystick.radius,
		      2 * jw->joystick.radius, 2 * jw->joystick.radius,
		      0, FULL_CIRCLE);
      
	 XDrawArc (event->xany.display, event->xany.window, edge_GC,
		   jw->joystick.center.x - jw->joystick.radius,
		   jw->joystick.center.y - jw->joystick.radius,
		   2 * jw->joystick.radius, 2 * jw->joystick.radius,
		   0, FULL_CIRCLE);
      }
   }

   else
   {
      if (right_GC != None)
	 XFillPolygon (event->xany.display, event->xany.window,
		       right_GC, jw->joystick.points[comp-1],
		       3, Convex, CoordModeOrigin);
      XDrawLines (event->xany.display, event->xany.window, edge_GC,
		  jw->joystick.points[comp-1], 4, CoordModeOrigin);
   }	
}

/* This function adjusts the positions of the buttons (a data structure for */
/* the buttons must be updated so that where clicks occur can be */
/* deciphered).  It also changes the information associated with the */
/* circle, and allocates a new pixmap for the pixmap double buffering that */
/* the widget does to make circle movement snazzier. */

static void Resizer(jw)
   JoystickWidget jw;
{
   int x = jw->core.width / 3;
   int y = jw->core.height / 3;

   TriangleBox(jw->joystick.points[UP], x, 0, x, y, UP);
   TriangleBox(jw->joystick.points[LEFT], 0, y, x, y, LEFT);
   TriangleBox(jw->joystick.points[RIGHT], 2 * x, y, x, y, RIGHT);
   TriangleBox(jw->joystick.points[DOWN], x, 2 * y, x, y, DOWN);
   jw->joystick.center.x = jw->core.width / 2;
   jw->joystick.center.y = jw->core.height / 2;
   jw->joystick.radius = (jw -> core.width > jw->core.height) ?
      jw->core.height : jw->core.width;
   jw -> joystick.radius /= 8;
   if (jw->joystick.pix != None)
      XFreePixmap(XtDisplay(jw), jw->joystick.pix);
   jw -> joystick.pix = 
      XCreatePixmap (XtDisplay(jw), 
		     DefaultRootWindow(XtDisplay(jw)),
		     jw -> core.width, jw -> core.height,
		     DefaultDepth(XtDisplay(jw),
				  DefaultScreen(XtDisplay(jw))));
   if (jw->joystick.erase_GC != None)
      XFillRectangle (XtDisplay(jw), jw->joystick.pix,
		      jw->joystick.erase_GC, 0, 0, jw->core.width,
		      jw->core.height);

}

/* This fuction computes the points of a triangle pointing in direction */
/* dir, which is o be contained in the box with upper left hand corner at */
/* (x,y) with widht and height w and h.  The four points (the starting and */
/* ending points are the same to facilitate drawing) are stored in points. */

static void TriangleBox(points, x, y, w, h, dir)
   XPoint points[4];
   int x, y, w, h, dir;
{
   switch( dir )
   {
    case UP:
      points[0].x = x;
      points[0].y = y + h;
      points[1].x = x + w / 2;
      points[1].y = y;
      points[2].x = x + w;
      points[2].y = y + h;
      break;
    case DOWN:
      points[0].x = x;
      points[0].y = y;
      points[1].x = x + w / 2;
      points[1].y = y + h;
      points[2].x = x + w;
      points[2].y = y;
      break;
    case LEFT:
      points[0].x = x;
      points[0].y = y + h / 2;
      points[1].x = x + w;
      points[1].y = y;
      points[2].x = x + w;
      points[2].y = y + h;
      break;
    default:
      points[0].x = x;
      points[0].y = y;
      points[1].x = x + w;
      points[1].y = y + h / 2;
      points[2].x = x;
      points[2].y = y + h;
      break;
   }
   points[3] = points[0];
}   

/* Inside returns true of the point (x, y) is inside the triangle specified */
/* by tri.  It uses a vatiant of the ccw test provided by sedgewick in his */
/* classic tome to determine the relationship of the point with each of the */
/* segments of the triangle.  If a clockwise or counterclockwide result is */
/* obtained from all the segments, then the point is inside the trinagle. */

static Boolean Inside(x, y, tri)
   int x, y;
   XPoint tri[3];

{
   XPoint diff[3];
   Boolean val1, val2, val3;
   int i;

   for ( i = 0 ; i < 3 ; i++ )
   {
      diff[i].x = tri[i].x - x;
      diff[i].y = tri[i].y - y;
   }
   
   val1 = diff[0].x * diff[1].y - diff[1].x * diff[0].y > 0;
   val2 = diff[1].x * diff[2].y - diff[2].x * diff[1].y > 0;
   val3 = diff[2].x * diff[0].y - diff[0].x * diff[2].y > 0;

   return ( val1 == val2 && val1 == val3 );
}

/* Inside which returns the code of the component in which the click */
/* specified by the event occurs.  (The xbutton reference is more general */
/* than it looks because x event structures are pretty consistent in what */
/* is placed where inside them.)  It uses the exact geometrical formulas of */
/* the objects to determine location. */

static int InsideWhich(jw, event)
   JoystickWidget jw;
   XEvent *event;
{
   int key_x = jw -> core.width / 3;
   int key_y = jw -> core.height / 3;
   int dist;

   if (event->xbutton.x < key_x)
      return (Inside (event->xbutton.x, event->xbutton.y,
		      jw -> joystick.points[LEFT]) ?
	      LEFT : -1);
   if (event->xbutton.y < key_y)
      return (Inside (event->xbutton.x, event->xbutton.y,
		      jw -> joystick.points[UP]) ?
	      UP : -1);
   if (event->xbutton.x > 2 * key_x)
      return (Inside (event->xbutton.x, event->xbutton.y,
		      jw -> joystick.points[RIGHT]) ?
	      RIGHT : -1);

   if (event->xbutton.y > 2 * key_y)
      return (Inside (event->xbutton.x, event->xbutton.y,
		      jw -> joystick.points[DOWN]) ?
	      DOWN : -1);

   dist = (event->xbutton.x - jw->joystick.center.x) *
      (event->xbutton.x - jw->joystick.center.x) +
	 (event->xbutton.y - jw->joystick.center.y) *
	    (event->xbutton.y - jw->joystick.center.y);

   return (dist < jw->joystick.radius*jw->joystick.radius) ? CIRCLE : -1;

}

/* Process click calls the callbacks of the joystick widget with a data */
/* structure prepared based on the widget's state. */

static void ProcessClick(jw)
   JoystickWidget jw;
{
   JoystickCallbackStruct j;

   switch (jw->joystick.mouse_state & ~(CLICK|DRAG|KEY|IN))
   {
    case UP + 1:
      j.x = 0;
      j.y = jw->joystick.page_size;
      break;
    case LEFT + 1:
      j.x = -jw->joystick.page_size;
      j.y = 0;
      break;
    case RIGHT + 1:
      j.x = jw->joystick.page_size;
      j.y = 0;
      break;
    case DOWN + 1:
      j.x = 0;
      j.y = -jw->joystick.page_size;
      break;
    case CIRCLE + 1:
      j.x = (jw->joystick.center.x - ((int) jw->core.width / 2)) *
	 333 / jw->joystick.radius;
      j.y = (((int) jw->core.height / 2) - jw->joystick.center.y) *
	 333 / jw->joystick.radius;
      break;
   }
   j.more_follow = (jw->joystick.mouse_state & (CLICK|DRAG|KEY)) ?
      TRUE : FALSE;
   XtCallCallbacks((Widget) jw, XtNchangeCallback, (caddr_t) &j);
}

/* This function takes care of interactive dragging of the circle.  It */
/* enforces a series of constraints on motion so that the joystick will do */
/* the appropriate things when the "shift" button is down or when the */
/* pointer is moved outside of the window.  Changes to the drawing are made */
/* in a pixmap and then blt'd on to the screen to eliminate (or minimize, I */
/* guess) flicker. */

static Boolean MoveCircle(jw, event, x, y)
   JoystickWidget jw;
   XEvent *event;
   int x, y;

{
   XPoint start, old, new;
   int width, height;
   Window window;
   double x_dist, y_dist, max, length, x_abs, y_abs;

   old = jw->joystick.center;

   /* Calculate the new position: first determine what it
      would be in the simplest case */

   new.x = x + jw->joystick.offset.x;
   new.y = y + jw->joystick.offset.y;
   
   max = (double) (jw->joystick.radius * 3);
   y_dist = (double) (new.y) - 
      (double) (jw->core.height) / 2;
   x_dist = (double) (new.x) -
      (double) (jw->core.width) / 2;
   
   /* If the motion of the joystick is constrained, zero out
      the displacement in the correct direction */

   if (jw->joystick.constrained == ISCONSTRAINED)
   {
      x_abs = ((x_dist > 0) ? x_dist : - x_dist);
      y_abs = ((y_dist > 0) ? y_dist : - y_dist);
      if (x_abs > (double) jw->joystick.radius ||
	  y_abs > (double) jw->joystick.radius)
	 jw->joystick.constrained = (x_abs > y_abs) ?
	    XCONSTRAINED : YCONSTRAINED;
      else
      {
	 x_dist = 0.0;
	 new.x = jw->core.width / 2;
	 y_dist = 0.0;
	 new.y = jw->core.height / 2;
      }
   }

   if (jw->joystick.constrained == YCONSTRAINED)
   {
      x_dist = 0.0;
      new.x = jw->core.width / 2;
   }
   else if (jw->joystick.constrained == XCONSTRAINED)
   {
      y_dist = 0.0;
      new.y = jw->core.height / 2;
   }

   length = sqrt (x_dist * x_dist + y_dist * y_dist);

   /* Constrain the position so that all of the joystick ball
      lies within the largest circle that can fit inside the widget. */

   if (length > max)
   {
      x_dist *= max/length;
      y_dist *= max/length;
      new.x = jw->core.width / 2 + (int) x_dist;
      new.y = jw->core.height / 2 + (int) y_dist;
   }
      
   /* Determine the smallest rectangle inside which the graphics 
      have changed, so that you can copy as little of the pixmap
      as possible onto the screen */

   if (old.x != new.x ||
       old.y != new.y)
   {
      if (new.x > old.x)
      {
	 start.x = old.x - jw->joystick.radius;
	 width = new.x - old.x + 2 * jw->joystick.radius;
      }
      else
      {
	 start.x = new.x - jw->joystick.radius;
	 width =  old.x - new.x + 2 * jw->joystick.radius;
      }
      if (new.y > old.y)
      {
	 start.y = old.y - jw->joystick.radius;
	 height = new.y - old.y  + 2 * jw->joystick.radius;
      }
      else
      {
	 start.y = new.y - jw->joystick.radius;
	 height = old.y - new.y + 2 * jw->joystick.radius;
      }
      
      /* Draw the whole picture to the pixmap (at the cost of just a few
	 redundant lines), and then copy the interesting piece into
	 our display window. */
      
      /* Remember to draw to our pixmap */
      
      window = event->xany.window;
      event->xany.window = jw->joystick.pix;
      
      /* Erase the old circle, but remember its position */
      
      RedrawComponent(jw, event, CIRCLE+1, ERASE_METHOD);
      jw->joystick.center = new;
      Redisplay(jw, event, (Region) NULL);
      XCopyArea(event->xany.display, event->xany.window,
		window, jw->joystick.edge_GC, start.x, start.y,
		width+2, height+2, start.x, start.y);
      event->xany.window = window;
      return True;
   }
   return False;
}

/* Adjust the state of the joystick when the mouse button is released, and */
/* update the picture. */

/* ARGSUSED */
static void MouseUp(jw, event, parms, num)
   JoystickWidget jw;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   int old_hit = (jw->joystick.mouse_state & ~(IN|CLICK|DRAG));
   int new_hit;

   if (!(jw->joystick.mouse_state & (CLICK|DRAG)))
      return;

   if (((event->xbutton.state & Button1Mask) > 0) +
       ((event->xbutton.state & Button2Mask) > 0) +
       ((event->xbutton.state & Button3Mask) > 0) > 1)
      return;

   if (jw->joystick.timer != None)
   {
      XtRemoveTimeOut( jw->joystick.timer );
      jw->joystick.timer = None;
   }

   jw->joystick.mouse_state &= ~(CLICK|DRAG);
   ProcessClick(jw);

   if (old_hit == CIRCLE + 1)
      (void) MoveCircle (jw, event,
			 (int)(jw->core.width / 2 - jw->joystick.offset.x),
			 (int)(jw->core.height / 2 - jw->joystick.offset.y));

   jw -> joystick.mouse_state =
      InsideWhich(jw, event) + 1;
   if (jw -> joystick.mouse_state)
      jw -> joystick.mouse_state |= IN;

   if ((new_hit = (jw -> joystick.mouse_state & ~IN)) != old_hit)
      RedrawComponent(jw, event, old_hit, OUT_METHOD);
   
   if (new_hit != OUT)
      RedrawComponent(jw, event, new_hit, OVER_METHOD);
}

/* Start processing a click.  First determine what object the click is in, */
/* then set the joystick's state to describe the current clicking process. */
/* Update the display of the joystick. */

static void MouseDown(jw, event, parms, num)
   JoystickWidget jw;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   if (!(jw->joystick.mouse_state & IN))
      return;

   if ((event->xbutton.state & Button1Mask) ||
       (event->xbutton.state & Button2Mask) ||
       (event->xbutton.state & Button3Mask))
      return;

   if (jw->joystick.mouse_state != (IN | (CIRCLE + 1)))
   {
      jw->joystick.mouse_state ^= CLICK | IN;
      
      if (jw->joystick.timer == None)
	 jw->joystick.timer = 
	    XtAddTimeOut (jw->joystick.initial_interval,
			  Timeout, (char *) jw);
      
      RedrawComponent(jw, event, jw->joystick.mouse_state & ~CLICK, 
		      HIT_METHOD);
   }
   else
   {
      if (num > 0 && parms[0][0] == 'c')
	 jw->joystick.constrained = ISCONSTRAINED;
      else
	 jw->joystick.constrained = UNCONSTRAINED;
      jw->joystick.offset.x = jw->joystick.center.x - event->xbutton.x;
      jw->joystick.offset.y = jw->joystick.center.y - event->xbutton.y;
      jw->joystick.mouse_state ^= DRAG | IN;
      RedrawComponent(jw, event, jw->joystick.mouse_state & ~DRAG, 
		      HIT_METHOD);
   }
}

/* Process a motion event.  Motion events normally just require checking to */
/* see if the current highlighting patterns need to be adjusted (and */
/* adjusting them if they do).  However, when the ball is being dragged, */
/* then the position of the ball must be updated and anybody interested in */
/* this must find out, in addition to the redrawing. */

/* ARGSUSED */
static void Motion(jw, event, parms, num)
   JoystickWidget jw;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   int new_hit;
   int old_hit;

   if (jw->joystick.mouse_state & (CLICK|KEY))
      return;

   if (jw->joystick.mouse_state & DRAG)
   {
      if (MoveCircle(jw, event, event->xmotion.x, event->xmotion.y))
      {
	 if (jw->joystick.timer != None)
	    XtRemoveTimeOut(jw->joystick.timer);
	 ProcessClick(jw);
	 if ((jw->joystick.center.x != jw->core.width / 2) ||
	     (jw->joystick.center.y != jw->core.height / 2))
	    jw->joystick.timer = XtAddTimeOut (jw->joystick.initial_interval,
					       Timeout, (caddr_t) jw);
	 else
	    jw->joystick.timer = None;
      }      
   }
   else
   {
      new_hit = InsideWhich(jw, event) + 1;
      
      if (new_hit == (old_hit = (jw->joystick.mouse_state & ~(CLICK | IN))))
	 return;
      
      jw->joystick.mouse_state = new_hit | (new_hit ? IN : OUT);
      
      if (old_hit != OUT)
         RedrawComponent(jw, event, old_hit, OUT_METHOD);

      if (new_hit != OUT)
         RedrawComponent(jw, event, new_hit, OVER_METHOD);
   }
}

/* Takes care of leave events so that things unhighlight themselves */

/* ARGSUSED */
static void Leave(jw, event, parms, num)
   JoystickWidget jw;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   if (!(jw->joystick.mouse_state & IN))
      return;

   RedrawComponent(jw, event, jw->joystick.mouse_state & ~IN, OUT_METHOD);

   jw->joystick.mouse_state = OUT;

}

/* Process a key click as a button press with the directionality specified */
/* as a parameter to the function in the translation table reference. */

static void KeyDown(jw, event, parms, num)
   JoystickWidget jw;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   int direction;

   if (*num != 2)
   {
      XtWarning("Action KeyDown called with too few parameters.\n");
      return;
   }

   if (jw->joystick.mouse_state & (CLICK|DRAG|KEY))
      return;

   jw->joystick.trigger_key = event->xkey.keycode;

   switch (parms[0][0])
   {
    case '+':
      direction = RIGHT + 1;
      break;
    case '-':
      direction = LEFT + 1;
      break;
    default:
      switch (parms[1][0])
      {
       case '+':
	 direction = DOWN + 1;
	 break;
       case '-':
	 direction = UP + 1;
	 break;
       default:
	 return;
      }
   }
   
   event->xany.window = XtWindow(jw);
   RedrawComponent(jw, event, jw->joystick.mouse_state & ~IN, 
		   OUT_METHOD);
   jw->joystick.mouse_state = KEY | direction;

   if (jw->joystick.timer == None)
      jw->joystick.timer = 
	 XtAddTimeOut (jw->joystick.initial_interval,
		       Timeout, (caddr_t) jw);
   
   RedrawComponent(jw, event, jw->joystick.mouse_state & ~KEY, 
		   HIT_METHOD);
}

/* Process a key release..... */

static void KeyUp(jw, event, parms, num)
   JoystickWidget jw;
   XEvent *event;
   String parms[];
   Cardinal *num;
{
   if (!(jw->joystick.mouse_state & KEY))
      return;

   if (event->xkey.keycode != jw->joystick.trigger_key)
      return;

   XtRemoveTimeOut(jw->joystick.timer);

   jw->joystick.timer = None;
   ProcessClick(jw);
   event->xany.window = XtWindow(jw);
   RedrawComponent(jw, event, jw->joystick.mouse_state & ~KEY, OUT);
   jw->joystick.mouse_state = OUT;
   Motion(jw, event, parms, num);
}

/* Make sure that if the user is holding down, we generate virtual */
/* auto-repeat events on the state of the widget */

/* ARGSUSED */
static void Timeout(jw, id)
   JoystickWidget jw;
   XtIntervalId *id;
{

   ProcessClick(jw);

   jw->joystick.timer = 
      XtAddTimeOut (jw->joystick.repeat_interval,
		    Timeout, (caddr_t) jw);
}

