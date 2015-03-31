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

#ifndef lint
static char Version[] =
   "$Id: td_window.c,v 1.3 1993/06/03 14:16:49 rch Exp $";
#endif /* lint */

#ifndef MOTIF_WIDGETS

fnw_type fnw_type_window = {
   WIDGET_TYPE_WINDOW,  /* type */
   STD_ARG_WINDOW,	/* name arg */
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

static METsym *td_window_create();
static void    td_window_destroy();

fnw_type fnw_type_window = {
   WIDGET_TYPE_WINDOW,  /* type */
   STD_ARG_WINDOW,	/* name arg */
   td_window_create,	/* create */
   td_window_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_cant_request,	/* request */
};

/*
 *	This is a callback that the form widget should call every
 *	time it gets resized so that when new objects are put in
 *	the form the form knows how much bigger to make itself.
 */

static void td_form_resize_call(w, our_info, event)
   Widget 	w;
   caddr_t 	our_info;
   XEvent 	*event;
{
   fnord_widget *form = CAST(fnord_widget *, our_info);
   Arg arg[2];
   ME(td_form_resize_call);

   if (event->type == MapNotify)
   {
      XtSetArg(arg[0], XmNwidth, &form->info.window.width);
      XtSetArg(arg[1], XmNheight, &form->info.window.height);
      XtGetValues(w, arg, 2);
      XStoreName(event->xmap.display, event->xmap.window, form->name);
   }
   else if (event->type == ConfigureNotify)
   {
      form->info.window.width = event->xconfigure.width;
      form->info.window.height = event->xconfigure.height;
   }
}

/*
 *	This function creates a window widget.  (This is a combination of
 *	a shell popup widget and a form widget.)  It responds to any 
 *      parameters that might have been set in the alist.
 */

fnord_widget *td_new_window(parent, name, alist, sym, dtype)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
   int 		dtype;
{
   Arg 		arg[10];
   int 		i = 0;
   int 		value;
   fnord_widget *window;
   widget_list 	*list;
   ME(td_window_create);

   ALLOCM(window, fnord_widget);
   window->type = WIDGET_TYPE_WINDOW;
   (void) strcpy(window->name, name);
   window->kids = NULL;
   window->dft_kid_type = dtype;
   window->state = FNW_STATE_START;

   window->xw.w = window->xw.top = None;
   td_link_in(&parent, name, WIDGET_TYPE_WINDOW, sym);

   if (parent == NULL) {
      FREE(window, sizeof(fnord_widget));
      return FNW_NULL;
   }

   if (parent->type == WIDGET_TYPE_TOP) {
      /*
       * width
       */
      if (!td_lookup_int(alist, std_args[STD_ARG_WIDTH], 
			  &window->info.window.width,
			  360, (Sym) NULL, 360))
	 switch (dtype) {
	  case WIDGET_TYPE_SLIDER:
	    window->info.window.width = DEFAULT_SLIDER_WINDOW_WIDTH;
	    break;
	  case WIDGET_TYPE_CHECKBOX:
	    window->info.window.width = DEFAULT_CHECKBOX_WINDOW_WIDTH;
	    break;
	  case WIDGET_TYPE_BUTTON:
	    window->info.window.width = DEFAULT_BUTTON_WINDOW_WIDTH;
	    break;
	  case WIDGET_TYPE_PRINTER:
	    window->info.window.width = DEFAULT_PRINTER_WINDOW_WIDTH;
	    break;
	  case WIDGET_TYPE_TYPEIN:
	    window->info.window.width = DEFAULT_TYPEIN_WINDOW_WIDTH;
	    break;
	  case WIDGET_TYPE_TAPEDECK:
	    window->info.window.width = DEFAULT_TAPEDECK_WINDOW_WIDTH;
	    break;
	  case WIDGET_TYPE_EVOLVE:
	    window->info.window.width = DEFAULT_EVOLVE_WINDOW_WIDTH;
	    break;
	  case WIDGET_TYPE_OUTPUT:
	    window->info.window.width = DEFAULT_OUTPUT_WINDOW_WIDTH;
	    break;
	  case WIDGET_TYPE_VIDEO:
	    window->info.window.width = DEFAULT_VIDEO_WINDOW_WIDTH;
	    break;
	  default:
	    break;
	 }
      XtSetArg(arg[i], XmNwidth, window->info.window.width);
      i++;
      /*
       * height
       */
      if (!td_lookup_int(alist, std_args[STD_ARG_HEIGHT], 
			 &window->info.window.height,
			 360, (Sym) NULL, 360))
	 switch (dtype) {
	  case WIDGET_TYPE_SLIDER:
	    window->info.window.height = DEFAULT_SLIDER_WINDOW_HEIGHT;
	    break;
	  case WIDGET_TYPE_CHECKBOX:
	    window->info.window.height = DEFAULT_CHECKBOX_WINDOW_HEIGHT;
	    break;
	  case WIDGET_TYPE_BUTTON:
	    window->info.window.height = DEFAULT_BUTTON_WINDOW_HEIGHT;
	    break;
	  case WIDGET_TYPE_PRINTER:
	    window->info.window.height = DEFAULT_PRINTER_WINDOW_HEIGHT;
	    break;
	  case WIDGET_TYPE_TYPEIN:
	    window->info.window.height = DEFAULT_TYPEIN_WINDOW_HEIGHT;
	    break;
	  case WIDGET_TYPE_TAPEDECK:
	    window->info.window.height = DEFAULT_TAPEDECK_WINDOW_HEIGHT;
	    break;
	  case WIDGET_TYPE_EVOLVE:
	    window->info.window.height = DEFAULT_EVOLVE_WINDOW_HEIGHT;
	    break;
	  case WIDGET_TYPE_OUTPUT:
	    window->info.window.height = DEFAULT_OUTPUT_WINDOW_HEIGHT;
	    break;
	  case WIDGET_TYPE_VIDEO:
	    window->info.window.height = DEFAULT_VIDEO_WINDOW_HEIGHT;
	    break;
	  default:
	    break;
	 }
      XtSetArg(arg[i], XmNheight, window->info.window.height);
      i++;

      if (td_lookup_int(alist, std_args[STD_ARG_TOP], &value,
			 0, (Sym) NULL, 0)) {
	 XtSetArg(arg[i], XmNx, value);
	 i++;
      }
      if (td_lookup_int(alist, std_args[STD_ARG_LEFT], &value,
			 0, (Sym) NULL, 0)) {
	 XtSetArg(arg[i], XmNy, value);
	 i++;
      }
      window->xw.top = XtAppCreateShell("fnord", "Fnord",
					applicationShellWidgetClass,
					XtDisplay(parent->xw.w), arg, i);
   } else
      window->xw.top = XmCreateFrame(parent->xw.w, window->name,
				     (Arg *) NULL, 0);

   (void) td_lookup_int(alist, std_args[STD_ARG_DIVIDE],
			 &window->info.window.subdivs,
			 (WIDGET_TYPE_SLIDER == dtype) ? 1:3, (Sym) NULL,
			 (WIDGET_TYPE_SLIDER == dtype) ? 1:3);
   XtSetArg(arg[i], XmNfractionBase, window->info.window.subdivs); i++;
   window->xw.w = XmCreateForm(window->xw.top, window->name, arg, i);

   XtAddEventHandler(window->xw.top, StructureNotifyMask, False,
		     td_form_resize_call, (caddr_t) window);

   if (parent->type == WIDGET_TYPE_WINDOW) {
      window->info.window.subwin = YES;
      td_winkid_geom(parent, window, alist, sym);
      XtManageChild(window->xw.top);
   } else
      window->info.window.subwin = NO;

   XtManageChild(window->xw.w);

   ALLOCM(list, widget_list);
   list->w = window;
   list->next = parent->kids;
   parent->kids = list;
   XtRealizeWidget(window->xw.top);
   return window;
}

static METsym *td_window_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget	*window;
   ME(td_window_create);

   window = td_new_window(parent, name, alist, sym, WIDGET_TYPE_NONE);

   if (FNW_NULL == window)
      return MET_NULL_SYM;

   return METsym_new_control((char *) window, sym->origin);
}

static void td_window_destroy(window)
   fnord_widget		*window;
{
   ME(td_window_destroy);

   XtDestroyWidget(window->xw.w);
   XtDestroyWidget(window->xw.top);
}

#endif /* MOTIF_WIDGETS */
