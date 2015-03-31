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

#include "ViewP.h"
#include <math.h>

#ifndef lint
static char Version[]=
   "$Id: View.c,v 1.8 1992/06/22 17:12:16 mas Exp $";
#endif

/* Private Definitions */

static void DrawToggle();
static void Initialize();
static void Realize();
static void Redisplay();
static void Resize();
static void Rescale();
static void Destroy();
static void ViewCreatePict();
static void PostscriptOutput();

/* Initialization of defaults */
static char translations[] =
    "<Key>space:	DrawToggle() \n\
     <Key>s:		Rescale()";

#define offset(field) XtOffset(ViewWidget,view.field)

static XtResource resources[] = 
{
{XtNdrawEnv, XtCDrawEnv, XtRPointer, sizeof(char *),
    offset(env), XtRImmediate, NULL},
{XtNpadding, XtCPadding, XtRInt, sizeof(int),
    offset(padding), XtRImmediate, (caddr_t) 30},
};

#undef offset

static XtActionsRec actionsList[] =
{
{"DrawToggle",	DrawToggle},
{"Rescale",	Rescale},
};

ViewClassRec viewClassRec = {
    { /* core fields */
    /* superclass		*/	&widgetClassRec,
    /* class_name		*/	"View",
    /* widget_size		*/	sizeof(ViewRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	actionsList,
    /* num_actions		*/	XtNumber(actionsList),
    /* resources		*/	resources,
    /* resource_count		*/	XtNumber(resources),
    /* xrm_class		*/	NULL,
    /* compress_motion		*/	TRUE,
#ifdef X_WINSYS_X11R4
    /* compress_exposure	*/	XtExposeCompressMultiple,
#else
    /* compress_exposure	*/	TRUE,
#endif
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	Resize,
    /* expose			*/	Redisplay,
    /* set_values		*/	NULL,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	translations,
    /* query_geometry           */	XtInheritQueryGeometry,
    /* display_accelerator      */	XtInheritDisplayAccelerator,
    /* extension                */	NULL,
    },
    { /* view fields */
    /* dummy	                */	0,
    }
};

WidgetClass viewWidgetClass = (WidgetClass) &viewClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void Initialize (request, new)
   ViewWidget request;
   ViewWidget new;
{
   /* A good size is 144 by 144 pixels */
   if (request -> core.width == 0)  new -> core.width  = 144;
   if (request -> core.height == 0) new -> core.height = 144;

   new -> view.pict = NULL;

   if (request -> view.env == NULL)
      XtError ("View widgets must have their env values set!");

   if (request -> view.env -> d != XtDisplay((Widget) new))
      XtError ("Display of widget must match display of env!");
}

static void Resize (vw)
   ViewWidget vw;
{
   Pixmap p;
   Real win_min[3], win_max[3];
   Display *d = vw->view.env->d;

   if (vw -> view.pict == NULL)
      return;

   p = XCreatePixmap (d, DefaultRootWindow(d), vw->core.width,
		      vw->core.height, DefaultDepth(d, DefaultScreen(d)));
   PictChangePixmap (vw->view.pict, p);

   win_min[0] = 0.0;
   win_min[1] = - (Real) vw->core.height;
   win_min[2] = -1.0;
   
   win_max[0] = (Real) vw->core.width;
   win_max[1] = 0.0; /* for flipped coords */
   win_max[2] = 1.0;

   PictRescale(vw->view.pict, win_min, win_max, (Real) vw->view.padding);

}

static void ViewCreatePict(vw)
   ViewWidget vw;
{
   PictXInitData data;
   Display *d = vw->view.env->d;

   vw->view.gc = data.gc = XCreateGC (d, DefaultRootWindow(d), None, 
				      (XGCValues *) NULL);
   data.draw_mode = PICT_WIRE_FRAME;
   data.w = XtWindow((Widget) vw);
   data.p = None;
   data.background.pixel = vw->core.background_pixel;
   XQueryColor (d, DefaultColormap (d, DefaultScreen(d)),
		&data.background);

   vw->view.pict = PictCreate (vw->view.env, &data);
   Resize(vw);
}

Pict ViewGetPict(w)
   Widget w;
{
   ViewWidget vw = (ViewWidget) w;

   if (vw->view.pict == NULL)
      ViewCreatePict(vw);
   
   return vw->view.pict;
}

static void Realize (vw, value_mask, attributes)
   ViewWidget vw;
   XtValueMask *value_mask;
   XSetWindowAttributes *attributes;
{
   XtCreateWindow ((Widget) vw, InputOutput,
		   (Visual *)CopyFromParent, *value_mask,
		   attributes); 

   if (vw->view.pict != NULL)
      PictChangeWindow (vw->view.pict, XtWindow((Widget) vw));
   else
      ViewCreatePict(vw);
}

static void Redisplay (vw, event, region)
   ViewWidget vw;
   XEvent *event;
   Region region;
{
   if (NULL != region)
      XSetRegion (vw->view.env->d, vw->view.gc, region);
   else if (NULL != event)
   {
      XRectangle clipRect;

      clipRect.x = event->xexpose.x;
      clipRect.y = event->xexpose.y;
      clipRect.width = event->xexpose.width;
      clipRect.height = event->xexpose.height;
      XSetClipRectangles(XtDisplay(vw), vw->view.gc, 0, 0,
			 &clipRect, 1, Unsorted);
   }

   EnvDrawAdd( vw->view.env, vw->view.pict, ENV_PIXMAP_MODE);
   EnvFlush( vw->view.env );

   /* reset the clip masks */
   XSetClipMask(vw->view.env->d, vw->view.gc, None);
}

int ViewClear(w)
   Widget w;
{
   ViewWidget vw = (ViewWidget) w;
   ViewObj curro;
   Pict pict = vw->view.pict;
   int result = MET_CLEAR_NOT_AT_ALL;

   for ( curro = pict->objs; curro != NULL; curro = curro->next )
      result |= METset_clear_rep(curro->set);

   return result;
}

void ViewUpdate(w, force, mode)
   Widget w;
   FOboolean force;
   int mode;
{
   ViewWidget vw = (ViewWidget) w;

   if (PictUpdate(vw->view.pict, FALSE) || YES == force)
      EnvDrawAdd(vw->view.env, vw->view.pict, mode);
}

/* ARGSUSED */
static void DrawToggle(vw, event, parms, num)
   ViewWidget vw;
   XEvent *event;
   String parms[];
   int num;
{
   PictChangeDrawMode (vw->view.pict);
   EnvDrawAdd( vw->view.env, vw->view.pict, ENV_PIXMAP_MODE);
   EnvFlush( vw->view.env );
}

static void Destroy(vw)
   ViewWidget vw;
{
   if (vw->view.pict != NULL)
      PictDestroy(vw->view.pict);
}

/* ARGSUSED */
static void Rescale(vw, event, parms, num)
   ViewWidget vw;
   XEvent *event;
   String parms[];
   int num;
{
   PictDefaultScale (vw->view.pict);
   EnvDrawAdd (vw->view.env, vw->view.pict, ENV_PIXMAP_MODE);
   EnvFlush (vw->view.env );
}
