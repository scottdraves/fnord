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
   "$Id: td_evolve.c,v 1.3 1993/06/03 14:16:33 rch Exp $";
#endif /* lint */

#ifndef MOTIF_WIDGETS

fnw_type fnw_type_evolve = {
   WIDGET_TYPE_EVOLVE,  /* type */
   STD_ARG_EVOLVE,	/* name arg */
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

static METsym	*td_evolve_create();
static void	 td_evolve_destroy();
static FOboolean td_evolve_request();
static void	 td_evolve_step();
static METset	*td_evolve();

/* Make a genuine update function if the evolve
   ever gets a way to indicate its value... */

fnw_type fnw_type_evolve = {
   WIDGET_TYPE_EVOLVE,  /* type */
   STD_ARG_EVOLVE,	/* name arg */
   td_evolve_create,	/* create */
   td_evolve_destroy,	/* destroy */
   NULL,		/* secure */
   td_evolve_step,	/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_evolve_request,	/* request */
};

static void td_evolve_step(evolve)
   fnord_widget		*evolve;
{
   METset *new_value;
   ME(td_evolve_step);

   new_value = td_evolve(evolve->info.evolve.current_value,
			 evolve->info.evolve.next_value_fn);

   if (MET_NULL_SET != new_value) {
      MET_SET_FREE(evolve->info.evolve.current_value);
      MET_SET_COPY(evolve->info.evolve.current_value, new_value);
      MET_SET_FREE(new_value);
   } 
}

static FOboolean td_evolve_request(evolve, req)
   fnord_widget		*evolve;
   METcontrol_request	*req;
{
   FOboolean result;
   int       should_change;
   FOboolean did_change;
   ME(td_evolve_request);

   if (!FNW_IS_TESTED(evolve)) {
      should_change = METset_clear_rep(evolve->info.evolve.initial_value);
      should_change |= METset_clear_rep(evolve->info.evolve.next_value_fn);
      result = METset_have_rep(evolve->info.evolve.current_value,
			       &did_change);
      if (SUCCESS == result) {
	 if (0 != should_change)
	    FNW_SET_CHANGED_MET(evolve, should_change);
	 if (0 != did_change)
	    FNW_SET_CHANGED(evolve, FNW_IS_RUNNING(evolve) ? 
			    FNW_STATE_CHANGED : FNW_STATE_CHANGED_NOW);
      }
      evolve->state |= FNW_STATE_TESTED;
   }

   switch (req->code) {
    case MET_CONTROL_REQUEST_TYPE:
      req->type = MET_NULL_TYPE;
      break;

    case MET_CONTROL_REQUEST_CHANGED:
      FNW_SET_REQ_CHANGED(req, evolve);
      break;

    case MET_CONTROL_REQUEST_VALUE:
      MET_SET_COPY(req->value, evolve->info.evolve.current_value);
      break;

    default:
      DEFAULT_ERR(req->code);
   }

   return SUCCESS;
}

static void td_evolve_destroy(evolve)
   fnord_widget		*evolve;
{
   ME(td_evolve_destroy);

   XtDestroyWidget(evolve->info.evolve.running );
   XtDestroyWidget(evolve->info.evolve.reset );
   XtDestroyWidget(evolve->xw.w );
   XtDestroyWidget(evolve->xw.top );
   MET_SET_FREE(evolve->info.evolve.initial_value);
   MET_SET_FREE(evolve->info.evolve.current_value);
   MET_SET_FREE(evolve->info.evolve.next_value_fn);
}

/* ARGSUSED */
static void td_evolve_changed( w, our_info, call_data )
     Widget w;			/* Evolve bar changed.		*/
     caddr_t our_info;		/* Pointer to Evolve structure	*/
     caddr_t call_data;		/* What generated the callback	*/
{
   fnord_widget *evolve = CAST(fnord_widget *, our_info);
   Boolean now_running;
   ME(td_evolve_changed);

   now_running = XmToggleButtonGetState(w);
   if (now_running != FNW_IS_RUNNING(evolve)) {
      evolve->state ^= FNW_STATE_RUNNING;
      td_register_running(now_running);
   }
}

/* ARGSUSED */
static void td_evolve_reset( w, our_info, call_data )
     Widget w;			/* Evolve bar reset.		*/
     caddr_t our_info;		/* Pointer to Evolve structure	*/
     caddr_t call_data;		/* What generated the callback	*/
{
   fnord_widget *evolve = CAST(fnord_widget *, our_info);
   ME(td_evolve_reset);

   MET_SET_FREE(evolve->info.evolve.current_value);

   MET_SET_COPY(evolve->info.evolve.current_value,
		evolve->info.evolve.initial_value);

   if (!FNW_IS_RUNNING(evolve)) {
      evolve->state |= FNW_STATE_CHANGED_NOW;
      td_update();
   }
}

static void td_create_evolve_widget(parent, evolve)
   Widget 	parent;
   fnord_widget *evolve;
{
   Arg arg[10];
   int i = 0;
   ME(xtd_create_evolve_widget);

   evolve->xw.top = XmCreateFrame(parent, evolve->name, (Arg *) NULL, 0);

   i = 0;
   XtSetArg(arg[i], XmNfractionBase, 2); i++;
   evolve->xw.w = XmCreateForm(evolve->xw.top, evolve->name, arg, i);

   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNleftPosition, 1); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   XtSetArg(arg[i], XmNindicatorType, XmN_OF_MANY); i++;
   XtSetArg(arg[i], XmNtitleString,
	    XmStringCreateLtoR(evolve->name,
			       XmSTRING_DEFAULT_CHARSET)); i++;
   evolve->info.evolve.running =
      XmCreateToggleButton(evolve->xw.w, evolve->name, arg, i);
   XmToggleButtonSetState(evolve->info.evolve.running, False, True);
   XtAddCallback(evolve->info.evolve.running,
		 XmNvalueChangedCallback, td_evolve_changed, 
		 (char *) evolve);


   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNrightPosition, 1); i++;
   XtSetArg(arg[i], XmNtitleString,
	    XmStringCreate("Reset", XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   evolve->info.evolve.reset = XmCreatePushButton(evolve->xw.w, "reset",
						  arg, i);
   XtAddCallback(evolve->info.evolve.reset,
		 XmNactivateCallback, td_evolve_reset, 
		 (char *) evolve);
}

static METsym *td_evolve_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget *evolve;
   widget_list 	*list;

   ME(xtd_new_evolve);

   ALLOCM(evolve, fnord_widget);

   evolve->type = WIDGET_TYPE_EVOLVE;
   (void) strcpy(evolve->name, name);
   evolve->kids = WL_NULL;
   evolve->dft_kid_type = WIDGET_TYPE_NONE;
   evolve->state = FNW_STATE_START;

   evolve->xw.w = evolve->xw.top = (Widget) NULL;

   evolve->info.evolve.initial_value =
      td_make_set_recalc(alist, STD_ARG_INIT, sym->origin);
   MET_SET_COPY(evolve->info.evolve.current_value,
		evolve->info.evolve.initial_value);
   evolve->info.evolve.next_value_fn =
      td_make_set_recalc(alist, STD_ARG_NEXT, sym->origin);

   td_link_in(&parent, name, WIDGET_TYPE_EVOLVE, sym);

   if (parent == NULL) {
      MET_SET_FREE(evolve->info.evolve.initial_value);
      MET_SET_FREE(evolve->info.evolve.current_value);
      MET_SET_FREE(evolve->info.evolve.next_value_fn);
      FREE(evolve, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   
   td_create_evolve_widget(parent->xw.w, evolve);

   td_winkid_geom(parent, evolve, alist, sym);
   
   XtManageChild( evolve->xw.top );
   XtManageChild( evolve->xw.w );
   XtManageChild( evolve->info.evolve.running );
   XtManageChild( evolve->info.evolve.reset );

   ALLOCM(list, widget_list);
   list->w = evolve;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) evolve, sym->origin);
}

/* I do this here to deal with boolean shit until it is fixed. */
#include "ops/ops.h"

/*
 * aply FN to VALUE and return the result.  do so by
 * constructing the proper definition, and then evaluating
 * it.  returns MET_NULL_SET if there were any errors.
 */
static METset *td_evolve(value, fn)
   METset *value, *fn;
{
   METset	*result;
   METsym	*def_sym, *value_sym, *fn_sym;
   ME(td_evolve);

   def_sym = METsym_new_op(MET_SYM_OP_APPLY, (FILOCloc *) NULL);
   fn_sym = METsym_new_set_ref(fn,  (FILOCloc *) NULL);
   value_sym = METsym_new_set_ref(value,  (FILOCloc *) NULL);

   METsym_add_kid(def_sym, fn_sym);
   METsym_add_kid(def_sym, value_sym);

   result = METset_new_with_definition(def_sym, MET_NULL_ENV);
   MET_SYM_FREE(def_sym);
   MET_SYM_FREE(fn_sym);
   MET_SYM_FREE(value_sym);

   return result;
}

#endif /* MOTIF_WIDGETS */

