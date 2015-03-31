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
   "$Id: td_tapedeck.c,v 1.4 1993/06/03 14:16:43 rch Exp $";
#endif /* lint */

#ifndef MOTIF_WIDGETS

fnw_type fnw_type_tapedeck = {
   WIDGET_TYPE_TAPEDECK,/* type */
   STD_ARG_TAPEDECK,	/* name arg */
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

static METsym	*td_tapedeck_create();
static void	 td_tapedeck_destroy();
static FOboolean td_tapedeck_request();
static void	 td_tapedeck_step();

fnw_type fnw_type_tapedeck = {
   WIDGET_TYPE_TAPEDECK,/* type */
   STD_ARG_TAPEDECK,	/* name arg */
   td_tapedeck_create,	/* create */
   td_tapedeck_destroy,	/* destroy */
   NULL,		/* secure */
   td_tapedeck_step,	/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_tapedeck_request,	/* request */
};

static void td_tapedeck_step(tapedeck)
   fnord_widget		*tapedeck;
{
   Arg 		arg[1];
   int 		i;
   char 	buf[50];
   FOboolean	stop = NO;
   ME(td_tapedeck_step);

   tapedeck->info.tapedeck.val +=
      tapedeck->info.tapedeck.deck_state *
	 tapedeck->info.tapedeck.speed;
      
   if (tapedeck->info.tapedeck.val >= tapedeck->info.tapedeck.max) {
      tapedeck->info.tapedeck.val = tapedeck->info.tapedeck.max;
      stop = YES;
   } else if (tapedeck->info.tapedeck.val <= tapedeck->info.tapedeck.min) {
      tapedeck->info.tapedeck.val = tapedeck->info.tapedeck.min;
      stop = YES;
   }

   if (YES == stop) {
      tapedeck->state &= ~FNW_STATE_RUNNING;
      td_register_running(FALSE);
   } else {
      tapedeck->state |= FNW_STATE_CHANGED_LATER;
   }

   (void) sprintf(buf, "%g", tapedeck->info.tapedeck.val);
   i = 0;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate(buf, XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetValues(tapedeck->info.tapedeck.label, arg, i);

   tapedeck->state |= FNW_STATE_CHANGED_NOW;
}

static void td_tapedeck_destroy(tapedeck)
   fnord_widget		*tapedeck;
{
   ME(td_tapedeck_destroy);

   XtDestroyWidget(tapedeck->info.tapedeck.label);
   XtDestroyWidget(tapedeck->info.tapedeck.forward);
   XtDestroyWidget(tapedeck->info.tapedeck.reverse);
   XtDestroyWidget(tapedeck->info.tapedeck.stop);
   XtDestroyWidget(tapedeck->xw.w);
   XtDestroyWidget(tapedeck->xw.top);
}

static FOboolean td_tapedeck_request(tapedeck, req)
   fnord_widget		*tapedeck;
   METcontrol_request	*req;
{
   METnumber num;
   ME(td_tapedeck_request);

   switch(req->code) {
    case MET_CONTROL_REQUEST_CHANGED:
      FNW_SET_REQ_CHANGED(req, tapedeck);
      break;

    case MET_CONTROL_REQUEST_TYPE:
      MET_TYPE_NEW_FIELD(req->type, tapedeck->info.tapedeck.field);
      break;

    case MET_CONTROL_REQUEST_VALUE:
      switch (tapedeck->info.tapedeck.field) {
       case MET_FIELD_ZTWO:
	 num.ztwo = (METztwo) tapedeck->info.tapedeck.val;
	 break;
       case MET_FIELD_INTEGER:
	 num.integer = (METinteger) tapedeck->info.tapedeck.val;
	 break;
       case MET_FIELD_REAL:
	 num.real = (METreal) tapedeck->info.tapedeck.val;
	 break;
       case MET_FIELD_COMPLEX:
	 num.complex.real = (METreal) tapedeck->info.tapedeck.val;
	 num.complex.imaginary = (METreal) 0.0;
	 break;
       default:
	 DEFAULT_ERR(tapedeck->info.tapedeck.field);
      }
      req->value = METset_new_number(tapedeck->info.tapedeck.field, &num);
      break;

    default:
      DEFAULT_ERR(req->code);
   }
   return SUCCESS;
}

/* ARGSUSED */
static void td_tapedeck_stop( w, our_info, call_data )
     Widget w;
     caddr_t our_info;
     caddr_t call_data;
{
   fnord_widget *tapedeck = CAST(fnord_widget *, our_info);
   ME(td_tapedeck_stop);

   if (FNW_IS_RUNNING(tapedeck)) {
      tapedeck->state &= ~FNW_STATE_RUNNING;
      td_register_running(FALSE);
   }
}

/* ARGSUSED */
static void td_tapedeck_forward( w, our_info, call_data )
     Widget w;
     caddr_t our_info;
     caddr_t call_data;
{
   fnord_widget *tapedeck = CAST(fnord_widget *, our_info);
   ME(td_tapedeck_forward);

   tapedeck->info.tapedeck.deck_state = 1.0;
   
   if (!FNW_IS_RUNNING(tapedeck)) {
      tapedeck->state |= FNW_STATE_RUNNING;
      td_register_running(TRUE);
   }
}

/* ARGSUSED */
static void td_tapedeck_reverse( w, our_info, call_data )
     Widget w;
     caddr_t our_info;
     caddr_t call_data;
{
   fnord_widget *tapedeck = CAST(fnord_widget *, our_info);
   ME(td_tapedeck_reverse);

   tapedeck->info.tapedeck.deck_state = -1.0;
   
   if (!FNW_IS_RUNNING(tapedeck)) {
      tapedeck->state |= FNW_STATE_RUNNING;
      td_register_running(TRUE);
   }
}

static void td_create_tapedeck_widget(parent, tapedeck)
   Widget parent;
   fnord_widget *tapedeck;
{
   Arg arg[12];
   int i = 0;
   ME(td_create_tapedeck_widget);

   i = 0;
   tapedeck->xw.top = XmCreateFrame(parent, tapedeck->name, arg, i);

   i = 0;
   XtSetArg(arg[i], XmNfractionBase, 6); i++;
   tapedeck->xw.w = XmCreateForm(tapedeck->xw.top, tapedeck->name, arg, i);

   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopPosition, 3); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNleftPosition, 4); i++;
   XtSetArg(arg[i], XmNarrowDirection, XmARROW_RIGHT); i++;
   tapedeck->info.tapedeck.forward =
      XmCreateArrowButton(tapedeck->xw.w, "forward", arg, i);
   XtAddCallback(tapedeck->info.tapedeck.forward, XmNactivateCallback,
		 td_tapedeck_forward, (char *) tapedeck);
   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopPosition, 3); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNrightPosition, 2); i++;
   XtSetArg(arg[i], XmNarrowDirection, XmARROW_LEFT); i++;
   tapedeck->info.tapedeck.reverse =
      XmCreateArrowButton(tapedeck->xw.w, "reverse", arg, i);
   XtAddCallback(tapedeck->info.tapedeck.reverse, XmNactivateCallback,
		 td_tapedeck_reverse, (char *) tapedeck);
   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNrightPosition, 4); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopPosition, 3); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNleftPosition, 2); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("Stop", XmSTRING_DEFAULT_CHARSET)); i++;
   tapedeck->info.tapedeck.stop =
      XmCreatePushButton(tapedeck->xw.w, "stop", arg, i);
   XtAddCallback(tapedeck->info.tapedeck.stop, XmNactivateCallback,
		 td_tapedeck_stop, (char *) tapedeck);
   i = 0;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNbottomPosition, 3); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("<<time>>", XmSTRING_DEFAULT_CHARSET)); i++;
   tapedeck->info.tapedeck.label =
      XmCreateLabel(tapedeck->xw.w, "label", arg, i);
}

static METsym *td_tapedeck_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym 	*sym;
{
   fnord_widget *tapedeck;
   widget_list 	*list;

   ME(td_tapedeck_create);

   ALLOCM(tapedeck, fnord_widget);

   tapedeck->type = WIDGET_TYPE_TAPEDECK;
   (void) strcpy(tapedeck->name, name);
   tapedeck->kids = WL_NULL;
   tapedeck->dft_kid_type = WIDGET_TYPE_NONE;
   tapedeck->state = FNW_STATE_START;

   tapedeck->xw.w = tapedeck->xw.top = (Widget) NULL;

   (void) td_lookup_real(alist, std_args[STD_ARG_MIN],
			 &tapedeck->info.tapedeck.min, 
			 0.0, (Sym) NULL, 0.0);
   
   (void) td_lookup_real(alist, std_args[STD_ARG_MAX],
			 &tapedeck->info.tapedeck.max, 
			 tapedeck->info.tapedeck.min + 1.0, (Sym) NULL, 0.0);

   if (tapedeck->info.tapedeck.min > tapedeck->info.tapedeck.max) {
      TD_ERR(sym->origin, ": tapedeck min greater than tapedeck max.\n");
      tapedeck->info.tapedeck.max = tapedeck->info.tapedeck.min + 1.0;
   }

   (void) td_lookup_real(alist, std_args[STD_ARG_INIT],
			 &tapedeck->info.tapedeck.val,
			 tapedeck->info.tapedeck.min, std_args[STD_ARG_MAX], 
			 tapedeck->info.tapedeck.max);

   (void) td_lookup_real(alist, std_args[STD_ARG_SPEED],
			 &tapedeck->info.tapedeck.speed,
			 (tapedeck->info.tapedeck.max -
			  tapedeck->info.tapedeck.min) / 100,
			 (Sym) NULL, 0.0);

   (void) td_lookup_field(alist, std_args[STD_ARG_FIELD],
			  &tapedeck->info.tapedeck.field,
			  MET_FIELD_REAL, (Sym) NULL,
			  MET_FIELD_REAL);
   
   /* make sure this is different so first time we are called we report */
   /* that we have changed */
   tapedeck->info.tapedeck.deck_state = 0.0;

   td_link_in(&parent, name, WIDGET_TYPE_TAPEDECK, sym);

   if (parent == NULL) {
      FREE(tapedeck, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   
   td_create_tapedeck_widget(parent->xw.w, tapedeck);

   td_winkid_geom(parent, tapedeck, alist, sym);
   
   XtManageChild( tapedeck->xw.top );
   XtManageChild( tapedeck->xw.w );
   XtManageChild( tapedeck->info.tapedeck.forward);
   XtManageChild( tapedeck->info.tapedeck.reverse);
   XtManageChild( tapedeck->info.tapedeck.stop);
   XtManageChild( tapedeck->info.tapedeck.label);

   ALLOCM(list, widget_list);
   list->w = tapedeck;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) tapedeck, sym->origin);
}
#endif /* MOTIF_WIDGETS */
