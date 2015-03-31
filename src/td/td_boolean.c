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
#include "met/defs.h"

#ifndef lint
static char Version[] =
   "$Id: td_boolean.c,v 1.3 1993/06/03 14:16:28 rch Exp $ ";
#endif /* lint */

#ifndef MOTIF_WIDGETS

fnw_type fnw_type_checkbox = {
   WIDGET_TYPE_CHECKBOX,/* type */
   STD_ARG_CHECKBOX,	/* Arg */
   td_cant_create,	/* Create */
   NULL,		/* Destroy */
   NULL,		/* secure */
   NULL,		/* Step */
   NULL,		/* Clear */
   NULL,		/* Update */
   NULL,		/* post update */
   td_cant_request,	/* Request */
};

fnw_type fnw_type_button = {
   WIDGET_TYPE_BUTTON,	/* type */
   STD_ARG_BUTTON,	/* Arg */
   td_cant_create,	/* Create */
   NULL,		/* Destroy */
   NULL,		/* Secure */
   NULL,		/* Step */
   NULL,		/* Clear */
   NULL,		/* Update */
   NULL,		/* post update */
   td_cant_request,	/* Request */
};

#else

static METsym *td_checkbox_create();
static void    td_boolean_destroy();
static FOboolean td_boolean_request();
static void    td_checkbox_changed();

static METsym *td_button_create();
static void    td_button_changed();
static void    td_button_step();

fnw_type fnw_type_checkbox = {
   WIDGET_TYPE_CHECKBOX,/* type */
   STD_ARG_CHECKBOX,	/* Arg */
   td_checkbox_create,	/* Create */
   td_boolean_destroy,	/* Destroy */
   NULL,		/* Secure */
   NULL,		/* Step */
   NULL,		/* Clear */
   NULL,		/* Update */
   NULL,		/* post update */
   td_boolean_request,	/* Request */
};

fnw_type fnw_type_button = {
   WIDGET_TYPE_BUTTON,	/* type */
   STD_ARG_BUTTON,	/* Arg */
   td_button_create,	/* Create */
   td_boolean_destroy,	/* Destroy */
   NULL,		/* Secure */
   td_button_step,	/* Step */
   NULL,		/* Clear */
   NULL,		/* Update */
   NULL,		/* post update */
   td_boolean_request,	/* Request */
};

static void td_boolean_destroy(bool)
   fnord_widget		*bool;
{
   ME(td_boolean_destroy);

   XtDestroyWidget( bool->xw.w );
   XtDestroyWidget( bool->xw.top );
}

static FOboolean td_boolean_request(bool, req)
   fnord_widget		*bool;
   METcontrol_request	*req;
{
   METnumber 	num;
   ME(td_boolean_request);

   switch(req->code) {
    case MET_CONTROL_REQUEST_TYPE:
      MET_TYPE_NEW_FIELD(req->type, MET_FIELD_ZTWO);
      break;

    case MET_CONTROL_REQUEST_CHANGED:
      FNW_SET_REQ_CHANGED(req, bool);
      break;

    case MET_CONTROL_REQUEST_VALUE:
      num.ztwo = (METztwo) bool->info.boolean.val;
      req->value = METset_new_number(MET_FIELD_ZTWO, &num);
      break;
   }

   return SUCCESS;
}

/* ARGSUSED */
static void td_checkbox_changed( w, our_info, call_data )
     Widget w;			/* Checkbox bar changed.		*/
     caddr_t our_info;		/* Pointer to Checkbox structure	*/
     caddr_t call_data;		/* What generated the callback	*/
{
   fnord_widget *checkbox = CAST(fnord_widget *, our_info);
   ME(td_checkbox_changed);

   checkbox->info.boolean.val = XmToggleButtonGetState(w);
   checkbox->state |= FNW_STATE_CHANGED_NOW;

   td_update();
}

static void td_create_checkbox_widget(parent, checkbox)
   Widget parent;
   fnord_widget *checkbox;
{
   Arg arg[2];
   int i = 0;
   ME(td_create_checkbox_widget);

   checkbox->xw.top = XmCreateFrame(parent, checkbox->name, 
				    (Arg *) NULL, 0);
   XtSetArg(arg[i], XmNindicatorType, XmN_OF_MANY); i++;
   XtSetArg(arg[i], XmNtitleString,
	    XmStringCreateLtoR(checkbox->name,
			       XmSTRING_DEFAULT_CHARSET)); i++;
   checkbox->xw.w = XmCreateToggleButton(checkbox->xw.top,
					 checkbox->name, arg, i);
   XmToggleButtonSetState(checkbox->xw.w,
			  (Boolean) checkbox->info.boolean.val,
			  True);
   
   XtAddCallback(checkbox->xw.w, XmNvalueChangedCallback,
		 td_checkbox_changed, (caddr_t) checkbox);
}

static METsym *td_checkbox_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget *checkbox;
   widget_list 	*list;
   ME(td_new_checkbox);

   ALLOCM(checkbox, fnord_widget);

   checkbox->type = WIDGET_TYPE_CHECKBOX;
   (void) strcpy(checkbox->name, name);
   checkbox->kids = WL_NULL;
   checkbox->dft_kid_type = WIDGET_TYPE_NONE;
   checkbox->state = FNW_STATE_START;

   checkbox->xw.w = checkbox->xw.top = NULL;

   (void) td_lookup_boolean(alist, std_args[STD_ARG_INIT],
			    &checkbox->info.boolean.val,
			    FALSE, (Sym) NULL, FALSE);

   td_link_in(&parent, name, WIDGET_TYPE_CHECKBOX, sym);

   if (parent == NULL) {
      FREE(checkbox, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   
   td_create_checkbox_widget(parent->xw.w, checkbox);
   td_winkid_geom(parent, checkbox, alist, sym);
   
   XtManageChild( checkbox->xw.top );
   XtManageChild( checkbox->xw.w );

   ALLOCM(list, widget_list);
   list->w = checkbox;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) checkbox, sym->origin);
}

/* ARGSUSED */
static void td_button_changed( w, our_info, call_data )
     Widget w;			/* Button bar changed.		*/
     caddr_t our_info;		/* Pointer to Button structure	*/
     caddr_t call_data;		/* What generated the callback	*/
{
   fnord_widget *button = CAST(fnord_widget *, our_info);
   ME(td_button_changed);

   button->info.boolean.val = 1;
   button->state |= FNW_STATE_CHANGED;

   td_update();
   
   button->info.boolean.val = 0;
   button->state |= FNW_STATE_RUNNING | FNW_STATE_CHANGED_NOW;
   td_register_running(TRUE);
}

static void td_button_step(button)
   fnord_widget		*button;
{
   ME(td_button_step);

   button->state &= ~FNW_STATE_RUNNING;
   td_register_running(FALSE);
}

static void td_create_button_widget(parent, button)
   Widget 	parent;
   fnord_widget *button;
{
   Arg arg[5];
   int i = 0;
   ME(td_create_button_widget);

   button->xw.top = XmCreateFrame(parent, button->name, 
				  (Arg *) NULL, 0);
   XtSetArg(arg[i], XmNtitleString,
	    XmStringCreateLtoR(button->name,
			       XmSTRING_DEFAULT_CHARSET)); i++;
   button->xw.w = XmCreatePushButton(button->xw.top, button->name, arg, i);
   
   XtAddCallback(button->xw.w, XmNactivateCallback, 
		 td_button_changed, (caddr_t) button);
}

static METsym *td_button_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget *button;
   widget_list 	*list;

   ME(td_button_create);

   ALLOCM(button, fnord_widget);

   button->type = WIDGET_TYPE_BUTTON;
   (void) strcpy(button->name, name);
   button->kids = WL_NULL;
   button->dft_kid_type = WIDGET_TYPE_NONE;
   button->state = FNW_STATE_START;

   button->xw.w = button->xw.top = NULL;

   button->info.boolean.val = 0;

   td_link_in(&parent, name, WIDGET_TYPE_BUTTON, sym);

   if (parent == NULL) {
      FREE(button, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   
   td_create_button_widget(parent->xw.w, button);
   td_winkid_geom(parent, button, alist, sym);
   
   XtManageChild( button->xw.top );
   XtManageChild( button->xw.w );

   ALLOCM(list, widget_list);
   list->w = button;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) button, sym->origin);
}

#endif /* MOTIF_WIDGETS */
