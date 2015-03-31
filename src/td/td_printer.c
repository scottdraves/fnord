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
   "$Id: td_printer.c,v 1.3 1993/06/03 14:16:40 rch Exp $";
#endif /* lint */

#ifndef X_TWO_D

/************************************************************************/
/*									*/
/*	The tty printer is a simple but nice widget.  Whenever the 	*/
/* value that it maintains changes, it pretty prints the new set to	*/
/* stdout.  This can be nice when monitoring the action of		*/
/* communications widgets.						*/
/*									*/
/************************************************************************/

/* forward references */

static METsym *td_printer_create();
static void    td_printer_destroy();
static void    td_printer_clear();
static void    td_printer_update();

fnw_type fnw_type_printer = {
   WIDGET_TYPE_PRINTER, /* type */
   STD_ARG_PRINTER,	/* name arg */
   td_printer_create,	/* create */
   td_printer_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   td_printer_clear,	/* clear */
   td_printer_update,	/* update */
   NULL,		/* post update */
   td_cant_request,	/* request */
};

static void td_printer_destroy(printer)
   fnord_widget		*printer;
{
   ME(td_printer_destroy(tty));

   MET_SET_FREE_NULL(printer->info.printer.set);
}

static void td_printer_clear(printer)
   fnord_widget		*printer;
{
   int			changed;
   ME(td_printer_clear(tty));

   if (MET_NULL_SET != printer->info.printer.set) {
      changed = METset_clear_rep(printer->info.printer.set);
      FNW_SET_UPDATE_MET(printer, changed);
   }
}

/* ARGSUSED */   
static void td_printer_update(fw)
   fnord_widget	*fw;
{
   char		new[4000];
   int		len;
   ME(td_printer_update(tty));

   if (MET_NULL_SET != fw->info.printer.set) {

      (void) strcpy(new, fw->name);
      (void) strcat(new, "\t");

      time_begin();
      if (0 == METset_have_rep(fw->info.printer.set, NULL)) {
	 time_end(FNULL, "set");
	 (void) strcat(new, "error");
      } else {
	 time_end(FNULL, "set");
	 METset_print_pretty(FNULL, fw->info.printer.set, new, 0);
      }
      
      (void) strcat(new, "\n");
      fputs(new, stdout);
   }
}

static METsym *td_printer_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget *printer;
   widget_list *list;
   ME(td_printer_create);

   ALLOCM(printer, fnord_widget);
   printer->type = WIDGET_TYPE_PRINTER;
   (void) strcpy(printer->name, name);
   printer->kids = NULL;
   printer->dft_kid_type = WIDGET_TYPE_NONE;
   printer->state = FNW_STATE_START;

   printer->info.printer.set = 
      td_make_set_recalc(alist, STD_ARG_SET, sym->origin);

   ALLOCM(list, widget_list);
   list->w = printer;
   list->next = parent->kids;
   parent->kids = list;

   td_printer_update(printer);

   return METsym_new_control((char *) printer, sym->origin);
}

#elif !defined(MOTIF_WIDGETS)

/************************************************************************/
/*									*/
/*	There is no plain X printer widget.  We work under the 		*/
/* philosophy that if you can't do it nicely, you shouldn't do it at 	*/
/* all.									*/
/*									*/
/************************************************************************/

fnw_type fnw_type_printer = {
   WIDGET_TYPE_PRINTER, /* type */
   STD_ARG_PRINTER,	/* name arg */
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

/************************************************************************/
/*									*/
/*	The motif printer is of course the fanciest, with its own	*/
/* window and scroll bars, and the option to keep the entire history 	*/
/* of the object you're looking at in the window....			*/
/*									*/
/************************************************************************/

/* forward references */

static METsym *td_printer_create();
static void    td_printer_destroy();
static void    td_printer_clear();
static void    td_printer_update();

fnw_type fnw_type_printer = {
   WIDGET_TYPE_PRINTER, /* type */
   STD_ARG_PRINTER,	/* name arg */
   td_printer_create,	/* create */
   td_printer_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   td_printer_clear,	/* clear */
   td_printer_update,	/* update */
   NULL,		/* post update */
   td_cant_request,	/* request */
};


static METsym *td_printer_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget *printer;
   widget_list *list;
   Arg		arg[4];
   int		i;
   ME(td_printer_create);

   ALLOCM(printer, fnord_widget);
   printer->type = WIDGET_TYPE_PRINTER;
   (void) strcpy(printer->name, name);
   printer->kids = NULL;
   printer->dft_kid_type = WIDGET_TYPE_NONE;
   printer->state = FNW_STATE_START;

   printer->xw.w = printer->xw.top = None;
   printer->info.printer.set = MET_NULL_SET;
   printer->info.printer.append = NO;

   td_link_in(&parent, name, WIDGET_TYPE_PRINTER, sym);

   if (parent == NULL) {
      FREE(printer, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }

   printer->xw.top = XmCreateFrame(parent->xw.w, printer->name,
				   (Arg *) NULL, 0);

   i = 0;
   XtSetArg(arg[i], XmNeditMode, XmMULTI_LINE_EDIT); i++;
   printer->xw.w = XmCreateScrolledText(printer->xw.top,
					printer->name, arg, i);
   XtOverrideTranslations(printer->xw.w, td_text_translations);

   td_winkid_geom(parent, printer, alist, sym);
   XtManageChild( printer->xw.top );
   XtManageChild( printer->xw.w );

   printer->info.printer.set = 
      td_make_set_recalc(alist, STD_ARG_SET, sym->origin);
   (void) td_lookup_boolean(alist, std_args[STD_ARG_APPEND],
			    &printer->info.printer.append,
			    NO, (Sym) NULL, NO);
   
      

   ALLOCM(list, widget_list);
   list->w = printer;
   list->next = parent->kids;
   parent->kids = list;

   td_printer_update(printer);

   return METsym_new_control((char *) printer, (FILOCloc *) NULL);
}

static void td_printer_destroy(printer)
   fnord_widget		*printer;
{
   ME(td_printer_destroy);

   XtDestroyWidget(printer->xw.w);
   XtDestroyWidget(printer->xw.top);
   MET_SET_FREE_NULL(printer->info.printer.set);
}

static void td_printer_clear(printer)
   fnord_widget		*printer;
{
   int		changed;
   ME(td_printer_clear);

   if (MET_NULL_SET != printer->info.printer.set) {
      changed = METset_clear_rep(printer->info.printer.set);
      FNW_SET_UPDATE_MET(printer, changed);
   }
}

static void td_printer_update(fw)
   fnord_widget	*fw;
{
   char		*text, *new;
   int		len;
   Boolean	prepend_name = NO;
   ME(xtd_printer_update);

   if (MET_NULL_SET != fw->info.printer.set) {

      if (YES == fw->info.printer.append) {
	 text = XmTextGetString(fw->xw.w);
	 if (0 == strlen(text))
	    prepend_name = YES;
      } else {
	 text = "";
	 prepend_name = YES;
      }
      len = strlen(text);
      ALLOCNM(new, char, len + 4000);
      if (YES == prepend_name) {
	 (void) strcpy(new, fw->name);
	 (void) strcat(new, "\n");
      } else
	 new[0] = '\0';

      (void) strcat(new, text);

      if (!prepend_name && fw->info.printer.append)
	 (void) strcat(new, "\n");
      
      time_begin();
      if (0 == METset_have_rep(fw->info.printer.set, (FOboolean *) NULL)) {
	 time_end(FNULL, "set");
	 (void) strcat(new, "error");
      } else {
	 time_end(FNULL, "set");
	 METset_print_pretty(FNULL, fw->info.printer.set, new, 0);
      }
      XmTextSetString(fw->xw.w, new);
      FREEN(new, char, len + 4000);
      if (YES == fw->info.printer.append)
	 XtFree(text);
   } else {
      XmTextSetString(fw->xw.w, "no set specified");
   }
}

#endif /* defined MOTIF_WIDGETS */
