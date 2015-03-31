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
#include "parse/scan.h"

#ifndef lint
static char Version[] =
   "$Id: td_typein.c,v 1.3 1993/06/03 14:16:46 rch Exp $";
#endif /* lint */

#ifndef MOTIF_WIDGETS 

fnw_type fnw_type_typein = {
   WIDGET_TYPE_TYPEIN,  /* type */
   STD_ARG_TYPEIN,	/* name arg */
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

/* forward referenced */

static void	td_typein_destroy();
static METsym	*td_typein_create();
static FOboolean td_typein_request();

fnw_type fnw_type_typein = {
   WIDGET_TYPE_TYPEIN,  /* type */
   STD_ARG_TYPEIN,	/* name arg */
   td_typein_create,	/* create */
   td_typein_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_typein_request,	/* request */
};

/* ARGSUSED */
static void td_typein_changed( w, our_info, call_data )
     Widget w;			/* Typein bar changed.		*/
     caddr_t our_info;		/* Pointer to Typein structure	*/
     caddr_t call_data;		/* What generated the callback	*/

{
   fnord_widget *typein = CAST(fnord_widget *, our_info);
   char *s, *temp;
   char name[120];
   int len;
   METset *result, *cast;
   FOboolean ok;
   ME(td_typein_changed);

   s = XmTextGetString(w);
   /* Eek */
   len = strlen(s);
   s = XtRealloc(s, len + 2);
   s[len++] = ';';
   s[len] = 0;
   temp = widget__top->info.top.text;
   widget__top->info.top.text = s;
   sprintf(name, "typein widget (%s)", typein->name);
   (void) fnord_parse (sym_from_string(name), -1, td_text_read, 
		       PARSE_NO_FLAGS, PARSE_ALL_FLAGS);
   widget__top->info.top.text = temp;
   XtFree(s);
   result = parse_last_expr();
   if ((METset *) NULL == result)
      return;

   cast = td_make_set_cast (result, typein->info.typein.type, &ok);
   MET_SET_FREE (result);

   if (NO == ok)
   {
      TD_MSG(stdout, "Typein: Contents of widget are of wrong type.\n");
      MET_SET_FREE (cast);
      return;
   }

   MET_SET_FREE_NULL (typein->info.typein.val);
   typein->info.typein.val = cast;
   typein->state |= FNW_STATE_CHANGED_NOW;
   td_update();
}

static void td_typein_destroy(typein)
   fnord_widget		*typein;
{
   ME(td_typein_destroy);
 
   XtDestroyWidget( typein->info.typein.label );
   XtDestroyWidget( typein->info.typein.editor );
   XtDestroyWidget( typein->xw.w );
   XtDestroyWidget( typein->xw.top );
   MET_TYPE_FREE ( typein->info.typein.type );
   MET_SET_FREE_NULL (typein->info.typein.val );
}

static FOboolean td_typein_request(typein, req)
   fnord_widget		*typein;
   METcontrol_request 	*req;
{
   int			changed;
   ME(td_typein_request);
   
   if (!FNW_IS_TESTED(typein)) {
      changed = METset_clear_rep(typein->info.typein.val);
      (void) METset_have_rep(typein->info.typein.val, (FOboolean *) NULL);
      FNW_SET_CHANGED_MET(typein, changed);
      typein->state |= FNW_STATE_TESTED;
   }

   switch (req->code) {
    case MET_CONTROL_REQUEST_TYPE:
      req->type = MET_NULL_TYPE;
      break;

    case MET_CONTROL_REQUEST_CHANGED:
      req->changed = FNW_IS_CHANGED(typein);
      break;

    case MET_CONTROL_REQUEST_VALUE:
      MET_SET_COPY_NULL(req->value, typein->info.typein.val);
      break;
   }

   return SUCCESS;
}

static void td_create_typein_widget(parent, typein, init)
   Widget parent;
   fnord_widget *typein;
   char *init;
{
   Arg arg[10];
   int i = 0;
   ME(td_new_typein_widget);

   typein->xw.top = XmCreateFrame(parent, typein->name, (Arg *) NULL, 0);

   i = 0;
   XtSetArg(arg[i], XmNfractionBase, 2); i++;
   typein->xw.w = XmCreateForm(typein->xw.top, typein->name, arg, i);

   i = 0;
   XtSetArg(arg[i], XmNeditMode, XmSINGLE_LINE_EDIT); i++;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNleftPosition, 1); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;

   typein->info.typein.editor = XmCreateText(typein->xw.w, typein->name,
					     arg, i);

   XmTextSetString(typein->info.typein.editor, init);
   XtOverrideTranslations(typein->info.typein.editor, td_text_translations);

   XtAddCallback(typein->info.typein.editor, XmNactivateCallback,
		 td_typein_changed, (char *) typein);


   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNrightPosition, 1); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate(typein->name, XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   typein->info.typein.label = XmCreateLabel(typein->xw.w, typein->name,
					     arg, i);
}

static METsym *td_typein_create(parent, name, alist, sym)
   fnord_widget *parent;
   char *name;
   METset *alist;
   METsym	*sym;
{
   fnord_widget *typein;
   widget_list *list;
   char init_text[1024];
   FOboolean call_changed = NO;
   FOboolean ok;
   int length = 1023;
   METset *init;
   ME(td_typein_create);

   ALLOCM(typein, fnord_widget);

   typein->type = WIDGET_TYPE_TYPEIN;
   (void) strcpy(typein->name, name);
   typein->kids = WL_NULL;
   typein->dft_kid_type = WIDGET_TYPE_NONE;
   typein->state = FNW_STATE_START;

   typein->xw.w = typein->xw.top = (Widget) NULL;

   typein->info.typein.type = 
      td_make_type_recalc(alist, STD_ARG_FIELD, sym->origin);

   typein->info.typein.val = MET_NULL_SET;

   init = METset_assoc (alist, std_args[STD_ARG_INIT]);
   if (MET_NULL_SET == init)
   {
      init_text[0] = 0;
   }
   else if (!METset_get_string(init, init_text, &length))
   {
      init = td_make_set_recalc (alist, STD_ARG_INIT, sym->origin);
      if (MET_NULL_SET != init) {
	 typein->info.typein.val =
	    td_make_set_cast (init, typein->info.typein.type, &ok);
	 if (NO == ok)
	 {
	    MET_SET_FREE (typein->info.typein.val);
	    typein->info.typein.val = MET_NULL_SET;
	 }
	 MET_SET_FREE (init);
      }
      init_text[0] = 0;
   }
   else
   {
      MET_SET_FREE (init);
      call_changed = YES;
   }

   td_link_in(&parent, name, WIDGET_TYPE_TYPEIN, sym);

   if (parent == NULL) {
      FREE(typein, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   
   td_create_typein_widget(parent->xw.w, typein, init_text);

   td_winkid_geom(parent, typein, alist, sym);
   
   XtManageChild( typein->xw.top );
   XtManageChild( typein->xw.w );
   XtManageChild( typein->info.typein.label );
   XtManageChild( typein->info.typein.editor );

   if (YES == call_changed)
      XtCallCallbacks (typein->info.typein.editor, XmNactivateCallback, 
		       (caddr_t) NULL);

   ALLOCM(list, widget_list);
   list->w = typein;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) typein, sym->origin);
}

#endif /* MOTIF_WIDGETS */
