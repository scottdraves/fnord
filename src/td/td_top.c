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
static char Version[] =
   "$Id: td_top.c,v 1.4 1993/06/03 14:16:44 rch Exp $";
#endif

#include "td/defs.h"

/* forward references */

static void td_top_destroy();

fnw_type fnw_type_top = {
   WIDGET_TYPE_TOP,	/* type */
   STD_ARG_TOP,		/* Arg */
   td_cant_create,	/* Create */
   td_top_destroy,	/* Destroy */
   NULL,		/* Secure */
   NULL,		/* Step */
   NULL,		/* Clear */
   NULL,		/* Update */
   NULL,		/* post update */
   td_cant_request,	/* Request */
};

/************************************************************************/
/*									*/
/*		CREATION OF BASIC WIDGET & INITIALIZATION		*/
/*									*/
/************************************************************************/

/*
 *  Generates Syms for the various alist options that this
 *  module knows how to deal with.
 */

static void td_init_syms()

{
   int i;
   static char *option_names[] =
   {
      "top",
      "left",
      "width",
      "height",
      "numUnits",
      "set",
      "min",
      "max",
      "init",
      "type",
      "parent",
      "right",
      "bottom",
      "start",
      "end",
      "all",
      "name",
      "window",
      "slider",
      "view",
      "vobject",
      "editor",
      "drag",
      "color",
      "printer",
      "append",
      "bitmap",
      "zoom",
      "field",
      "checkbox",
      "typein",
      "tapedeck",
      "speed",
      "evolve",
      "next",
      "button",
      "output",
      "point",
      "dims",
      "strobe",
      "modifiers",
      "read",
      "write",
      "port",
      "host",
      "camera",
      "video",
   };
   ME(td_init_syms);

   for ( i = 0 ; i < STD_ARG_NUM ; i++ )
      std_args[i] = sym_from_string(option_names[i]);
}

#ifdef X_TWO_D

/*
 * R3 doesn't have XtAppInitialize, so we provide something similar
 */

#ifndef X_WINSYS_X11R4

/* ------------------------------------------------------------------------- 
 *  
 * This is from O'Reilly and Associates, X Window Books Volume IV.
 * 
 *	Function Name: XtAppInitializeR3
 *	Description: A convience routine for Initializing the toolkit.
 *	Arguments: app_context_return - The application context of the
 *                                      application
 *                 application_class  - The class of the application.
 *                 options            - The option list.
 *                 num_options        - The number of options in the above list
 *                 argc_in_out, argv_in_out - number and list of command line
 *                                            arguments.
 *	Returns: The shell widget.
 *
 * ------------------------------------------------------------------------- 
 */
	
static Widget
XtAppInitializeR3(app_context_return, application_class, options, num_options,
		argc_in_out, argv_in_out)
   XtAppContext * app_context_return;
   String application_class;
   XrmOptionDescRec *options;
   Cardinal num_options, *argc_in_out;
   String *argv_in_out;
{
    XtAppContext app_con;
    Display *dpy;
    Widget root;

    ME(XtAppIntializeR3);

    XtToolkitInitialize();
    
    app_con = XtCreateApplicationContext();

    dpy = XtOpenDisplay(app_con, (String) NULL, NULL, application_class,
			options, num_options, argc_in_out, argv_in_out);


    if (dpy == NULL)
	XtErrorMsg("invalidDisplay","xtInitialize","XtToolkitError",
                   "Can't Open display", (String *) NULL, (Cardinal *)NULL);

    root = XtAppCreateShell(argv_in_out[0], application_class,
			    applicationShellWidgetClass, 
			    dpy, (ArgList) NULL, 0);
    
    if (app_context_return != NULL)
	*app_context_return = app_con;

    return(root);
}

#endif /* undef(X_WINSYS_X11R4) */

#ifdef MOTIF_WIDGETS
static char *new_translations =
   "#override \
    Ctrl<Key>A:     beginning-of-line() \n\
    Ctrl<Key>B:     backward-character() \n\
    Ctrl<Key>D:     delete-next-character() \n\
    Ctrl<Key>E:     end-of-line() \n\
    Ctrl<Key>F:     forward-character() \n\
    Ctrl<Key>H:     delete-previous-character() \n\
    Ctrl<Key>J:     newline-and-indent() \n\
    Ctrl<Key>K:     kill-to-end-of-line() \n\
    Ctrl<Key>L:     redraw-display() \n\
    Ctrl<Key>M:     newline() \n\
    Ctrl<Key>N:     next-line() \n\
    Ctrl<Key>O:     newline-and-backup() \n\
    Ctrl<Key>P:     previous-line() \n\
    Ctrl<Key>V:     next-page() \n\
    Ctrl<Key>W:     kill-selection() \n\
    Ctrl<Key>Y:     unkill() \n\
    Ctrl<Key>Z:     scroll-one-line-up() \n\
    Meta<Key>B:     backward-word() \n\
    Meta<Key>F:     forward-word() \n\
    Meta<Key>V:     previous-page() \n\
    Meta<Key>Z:     scroll-one-line-down() \n\
    Meta<Key>d:    delete-next-word() \n\
    Meta<Key>D:    kill-next-word() \n\
    Meta<Key>h:    delete-previous-word() \n\
    Meta<Key>H:    kill-previous-word() \n\
    Meta<Key>\\<:  beginning-of-file() \n\
    Meta<Key>\\>:  end-of-file() \n\
    Meta<Key>]:    forward-paragraph() \n\
    Meta<Key>[:    backward-paragraph() \n\
    ~Shift Meta<Key>Delete:         delete-previous-word() \n\
     Shift Meta<Key>Delete:         kill-previous-word() \n\
    ~Shift Meta<Key>BackSpace:      delete-previous-word() \n\
     Shift Meta<Key>BackSpace:      kill-previous-word() \n\
    <Key>Delete:    delete-previous-character() \n\
    <Key>BackSpace: delete-previous-character()";

XtTranslations td_text_translations = None;
#endif /* defined(MOTIF_WIDGETS) */

/*
 *	This function makes the top level widget that we 
 *	need for motif, and initializes all its fields
 *	to the proper values.
 */

static fnord_widget *td_top_new(argc, argv)
   int *argc;
   char **argv;
{
   fnord_widget *top;
   ME(td_top_new);
   if (widget__top != NULL) {
      fprintf(stderr, "Only one top-level widget is allowed.");
      return NULL;
   }

   ALLOCM(top, fnord_widget);
   top->type = WIDGET_TYPE_TOP;
   strcpy(top->name, "fnord root widget");
   top->dft_kid_type = WIDGET_TYPE_NONE;
   top->kids = NULL;
   top->state = FNW_STATE_START;

#ifdef X_WINSYS_X11R4
   top->xw.w = XtAppInitialize(&top->info.top.app, "Fnord",
			       (XrmOptionDescRec *) NULL, 0,
			       argc, argv, NULL, NULL, 0);
#else
   top->xw.w = XtAppInitializeR3(&top->info.top.app, "Fnord",
				 (XrmOptionDescRec *) NULL, 0,
				 CAST(Cardinal *, argc), 
				 CAST(String *, argv));
#endif

#ifdef X_THREE_D
   top->xw.top = None;
   top->info.top.env = EnvCreate (XtDisplay(top->xw.w));
#else
   top->info.top.env = NULL;
#endif

#ifdef MOTIF_WIDGETS
   td_text_translations = XtParseTranslationTable(new_translations);
#endif

   top->info.top.text_input = -1;	
   top->info.top.length  = 0;
   top->info.top.ready = NO;
   top->info.top.running = 0;
   return top;
}

static void td_top_destroy(top)
   fnord_widget		*top;
{
   ME(td_top_destroy);
   
   FREE(top->info.top.text, top->info.top.length);
   XCloseDisplay(XtDisplay(top->xw.w));
}

#else /* defined(X_TWO_D) */

static fnord_widget *td_top_new(argc, argv)
   int *argc;
   char *argv;

{
   fnord_widget *top;
   ME(td_top_new);
   if (widget__top != NULL) {
      fprintf(stderr, "Only one top-level widget is allowed.");
      return NULL;
   }

   ALLOCM(top, fnord_widget);
   top->type = WIDGET_TYPE_TOP;
   top->kids = NULL;

   top->info.top.calls = (fnw_call *) NULL;
   top->info.top.nfds = 0;
   top->info.top.destroy_fds = NO;

   top->info.top.text_input = -1;	
   top->info.top.length  = 0;
   top->info.top.ready = NO;
   top->info.top.running = 0;
   return top;
}

static void td_top_destroy(top)
   fnord_widget		*top;
{
   ME(td_top_destroy);

   if (0 != top->info.top.nfds)
      FREEN(top->info.top.calls, fnw_call, top->info.top.nfds);
   FREE(top->info.top.text, top->info.top.length);
}

void td_wfd_remove(fd)
   int			fd;
{
   int			i;
   ME(td_wfd_remove);

   for (i = 0; i < widget__top->info.top.nfds; i++)
      if (widget__top->info.top.calls[i].fd == fd) {
	 widget__top->info.top.calls[i].alive = NO;
	 widget__top->info.top.destroy_fds = YES;
      }
}

void td_wfd_clean()
{
   int 		copyfrom, copyto;
   ME(td_wfd_clean);

   if (NO == widget__top->info.top.destroy_fds)
      return;

   copyfrom = copyto = 0;

   while (copyfrom < widget__top->info.top.nfds) {
      if (NO == widget__top->info.top.calls[copyfrom].alive) {
	 copyfrom++;
	 continue;
      }

      if (copyfrom != copyto) 
	 widget__top->info.top.calls[copyto] = 
	    widget__top->info.top.calls[copyfrom];

      copyfrom++;
      copyto++;
   }

   if (0 == copyto) {
      FREEN(widget__top->info.top.calls, fnw_call, 1);
      widget__top->info.top.calls = (fnw_call *) NULL;
   } else {
      REALLOCNM(widget__top->info.top.calls, fnw_call, copyfrom, copyto);
   }

   widget__top->info.top.nfds = copyto;
   widget__top->info.top.destroy_fds = NO;
}

void td_wfd_add(fd, reader, writer, data)
   int			fd;
   void			(*reader)();
   void			(*writer)();
   caddr_t		data;
{
   int i;
   ME(td_wfd_add);

   for (i = 0; i < widget__top->info.top.nfds; i++)
      if (widget__top->info.top.calls[i].fd == fd)
	 break;

   if (i == widget__top->info.top.nfds) {
      if (0 == i) {
	 ALLOCNM(widget__top->info.top.calls, fnw_call, 1);
      } else {
	 REALLOCNM(widget__top->info.top.calls, fnw_call, i+1, i);
      }
      widget__top->info.top.nfds++;
   }
   widget__top->info.top.calls[i].fd = fd;
   widget__top->info.top.calls[i].alive = YES;
   widget__top->info.top.calls[i].readfunc = reader;
   widget__top->info.top.calls[i].writefunc = writer;
   widget__top->info.top.calls[i].closure = data;
}

#endif /* if defined(X_TWO_D) else */

void td_register_running(is_on)
   FOboolean		is_on;
{
   ME(td_register_running);
   
   if (YES == is_on) {
      widget__top->info.top.running++;
   } else {
      widget__top->info.top.running--;
   }	
}

/*
 *	This function will create the top level widget that
 *	we need to do business in motif.  It will also 
 *	create the symbols we need to parse the alists
 *	we get when people ask to create widgets.
 */

void td_init(argc, argv)
   int *argc;
   char **argv;

{
   METdefaults defaults;
   ME(td_init);
   
   td_init_syms();
   
   defaults.controls.callback = td_control_callback;
   defaults.controls.common_data = (char *) NULL;
   METdefaults_set(&defaults, MET_DEFAULTS_CONTROLS);
   widget__top = td_top_new(argc, argv);
}

