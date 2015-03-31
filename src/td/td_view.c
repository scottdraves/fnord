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

#include "td/defs.h"
#include "parse/parse.h"

#ifndef lint
static char Version[] =
   "$Id: td_view.c,v 1.3 1993/06/03 14:16:47 rch Exp $";
#endif /* defined(lint) */

#ifndef X_TWO_D

fnw_type fnw_type_view = {
   WIDGET_TYPE_VIEW,    /* type */
   STD_ARG_VIEW,	/* name arg */
   td_cant_create,	/* create */
   NULL,		/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_cant_request,	/* request */
};

#else 

/* forward references */

static METsym *td_view_create();
static void    td_view_destroy();
static void    td_view_clear();
static void    td_view_update();
static FOboolean td_view_req_cam();

fnw_type fnw_type_view = {
   WIDGET_TYPE_VIEW,	/* type */
   STD_ARG_VIEW,	/* name arg */
   td_view_create,	/* create */
   td_view_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   td_view_clear,	/* clear */
   td_view_update,	/* update */
   NULL,		/* post update */
   td_view_req_cam,	/* request */
};

static void td_view_camera(view, alist, sym)
   fnord_widget		*view;
   METset		*alist;
   METsym		*sym;
{
   METset		*assoc;
   fnord_widget		*camera = FNW_NULL;
   Sym			camera_sym;
   METsym		*camera_def;
   widget_list		*list;
   Spin			spin;
   Pict			pict;
   ME(td_view_camera);

   if (MET_NULL_SET == alist)
      goto make_camera;

   assoc = METset_assoc(alist, std_args[STD_ARG_CAMERA]);
   if (MET_NULL_SET == assoc) 
      goto make_camera;

   if (!td_find_symbol(assoc, &camera_sym)) {
      MET_SET_FREE(assoc);
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout,": Camera must be associated with a symbol\n");
      TD_MSG(stdout, "\t\t\tthat names a widget.\n");
      goto make_camera;
   }
    
   MET_SET_FREE(assoc);
   assoc = METenv_lookup(parse_root_environment, camera_sym);
   if (assoc == MET_NULL_SET) {
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout, "Undefined variable %s\n", SYM_STRING(camera_sym));
      goto make_camera;
   }

   camera_def = assoc->definition;
   if (MET_SYM_TYPE_CONTROL != camera_def->type) {
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout, ": camera is not a widget.\n");
      MET_SET_FREE(assoc);
      goto make_camera;
   } 

   camera = CAST(fnord_widget *, camera_def->info.control.data);
   MET_SET_FREE(assoc);

   switch (camera->type) {
    case WIDGET_TYPE_CAMERA:
      break;
      
    case WIDGET_TYPE_POINT:
      if (FALSE == camera->info.point.is_in_view)
	 goto error;
      camera = camera->info.point.view;
      /* fall through */

    case WIDGET_TYPE_VIEW:
      camera = camera->info.view.camera;
      break;

    default:
    error:
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout, ": cannot retrieve camera info from widget.\n");
      camera = FNW_NULL;
      break;
   }

 make_camera:
   if (FNW_NULL == camera) {
      fnord_widget *parent = view->info.view.parent;
      camera = td_new_vs(parent, view, "vs",
			 view->info.view.top,
			 view->info.view.left,
			 view->info.view.bottom,
			 view->info.view.right);
      XtInstallAccelerators(view->xw.w, camera->xw.w);
#ifdef MOTIF_WIDGETS
      XtSetKeyboardFocus(parent->xw.top, None);
#endif
      push_widget_under(view->xw.top, camera->xw.w);
   }

   ALLOCM(list, widget_list);
   list->w = view;
   list->next = camera->info.camera.affects;
   camera->info.camera.affects = list;

   /* NO GOOD WITH CAMERAS BESIDES VS! */
   vs_spin_get(camera->xw.w, spin);
   pict = ViewGetPict(view->xw.w);
   PictSetSpin (pict, spin);

   view->info.view.camera = camera;
}


/*
  
here's how views and their associated widgets work.  A view widget never
appears alone.  there is always a virtual sphere input to control the
rotation.  sometimes there are other input widgets, like a point or pick
widget.  the widgets are siblings, like this:

       app Shell
          |
        XmForm
        /  \  \ 
  XmFrame  VS  point
     |
   View

the shell/form and frame/view pairs can be found in the top and w fields of
the window and view fnord_widget structures.

Now, because all the widgets occupy the same space on the screen, all
events will only go to whichever one is on top.  we need to do something
special so that they can all get events.  Here's what happens: when each of
these widgets is created push_widget_under is called, which uses
XtMakeGeometryRequest() to guarantee that the view widget is on top of the
stack.  They also call XtInstallAccelerators(); their translations were
also placed in the XtNaccelerators resource.  The view widget's
translations now detects the other widget's events and passes them along
when they occur.  BUT, since we are the children of an XmForm widget, focus
policy is in force.  That means that keyboard events are automagically
redirected to the "active" widget.  To work around this, I disable the
redirection with XtSetKeyboardFocus().  This isn't a permanent solution;
there *should* be a control-panel or dialog for the view widget rather than
using the kbd commands.

*/

static METsym *td_view_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget *view;
   widget_list *list;
   METset *set;
   METsym *baby;
   Arg arg[2];
   ME(td_view_create);

   ALLOCM(view, fnord_widget);
   
   view->type = WIDGET_TYPE_VIEW;
   (void) strcpy( view->name, name );
   view->kids = WL_NULL;
   view->dft_kid_type = WIDGET_TYPE_NONE;
   view->state = FNW_STATE_START;

   view->xw.w = view->xw.top = None;

   td_link_in(&parent, name, WIDGET_TYPE_VIEW, sym);

   if (parent == NULL) {
      FREE(view, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   view->info.view.parent = parent;

#ifdef MOTIF_WIDGETS
   view->xw.top = XmCreateFrame(parent->xw.w, view->name, NULL, 0);

   XtSetArg (arg[0], XtNdrawEnv, widget__top->info.top.env);
   view->xw.w = XtCreateWidget("funk", viewWidgetClass, view->xw.top, arg, 1);
   td_winkid_geom(parent, view, alist, sym);
   XtManageChild( view->xw.top );
   XtManageChild( view->xw.w );
#else
   view->xw.top = XtAppCreateShell(NULL, view->name,
				   topLevelShellWidgetClass,
				   XtDisplay(parent->xw.w), NULL, 0);
   XtSetArg (arg[0], XtNdrawEnv, widget__top->info.top.env);
   view->xw.w = XtCreateWidget("funk", viewWidgetClass, view->xw.top,
			    arg, 1);

   XtManageChild( view->xw.w );
   XtRealizeWidget( view->xw.top );
#endif MOTIF_WIDGETS

   ALLOCM(list, widget_list);
   list->w = view;
   list->next = parent->kids;
   parent->kids = list;

   td_view_camera(view, alist, sym);

   if (alist != MET_NULL_SET)
      if ((set = METset_assoc(alist,
			      std_args[STD_ARG_SET])) != MET_NULL_SET) {
	 baby = (FNW_CREATE(WIDGET_TYPE_OBJECT))(view, name, alist, sym);
	 MET_SYM_FREE_NULL(baby);
	 MET_SET_FREE(set);
      }
   
   return METsym_new_control((char *) view, sym->origin);
}

static void td_view_destroy(view)
   fnord_widget		*view;
{
   ME(td_view_destroy);

   XtDestroyWidget(view->xw.w);
   XtDestroyWidget(view->xw.top);
}

static void td_view_clear(view) 
   fnord_widget		*view;
{
   int	changed;
   ME(td_view_clear);

   changed = ViewClear(view->xw.w);
   FNW_SET_UPDATE_MET(view, changed);
}

static void td_view_update(view)
   fnord_widget		*view;
{
   int mode;
   ME(td_view_update);

   if (0 == (view->state & FNW_STATE_UPDATE_LATER))
      mode = ENV_PIXMAP_MODE;
   else
      mode = ENV_COLORMAP_MODE;

   ViewUpdate(view->xw.w, FNW_NEEDS_UPDATE(view), mode);
}

static FOboolean td_view_req_cam(view, req)
   fnord_widget		*view;
   METcontrol_request	*req;
{
   return FNW_REQUEST(view->info.view.camera, req);
}

#endif /* if !defined(X_TWO_D) else */
