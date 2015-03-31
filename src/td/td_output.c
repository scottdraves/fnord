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

/* let's eliminate this widget sometime soon ! */

#ifndef lint
static char Version[] =
   "$Id: td_output.c,v 1.4 1993/06/03 14:16:37 rch Exp $";
#endif /* lint */

#include "td/defs.h"
#include "parse/parse.h"

#ifndef MOTIF_WIDGETS

fnw_type fnw_type_output = {
   WIDGET_TYPE_OUTPUT,  /* type */
   STD_ARG_OUTPUT,	/* name arg */
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

static METsym 	*td_output_create();
static void	td_output_destroy();

/* Fnord widget definition for outputs */

fnw_type fnw_type_output = {
   WIDGET_TYPE_OUTPUT,  /* type */
   STD_ARG_OUTPUT,	/* name arg */
   td_output_create,	/* create */
   td_output_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_cant_request,	/* request */
};

/* ARGSUSED */
static void td_output_set_style (widget, our_info, call_info)
   Widget widget;
   caddr_t our_info;
   caddr_t call_info;
{
   fnord_widget *output = CAST(fnord_widget *, our_info);
   XmRowColumnCallbackStruct *rccs =
      CAST(XmRowColumnCallbackStruct *, call_info);
   XmToggleButtonCallbackStruct *tbcs =
      CAST(XmToggleButtonCallbackStruct *, rccs->callbackstruct);

   if (tbcs->set == False)
      return;

   if (rccs->widget == output->info.output.tb1)
      output->info.output.draw_style = ENV_BW_POSTSCRIPT_MODE;
   else if (rccs->widget == output->info.output.tb2)
      output->info.output.draw_style = ENV_GRAY_POSTSCRIPT_MODE;
   else if (rccs->widget == output->info.output.tb3)
      output->info.output.draw_style = ENV_COLOR_POSTSCRIPT_MODE;
}

/* ARGSUSED */
static void td_output_print(widget, our_info, call_info)
   Widget widget;
   caddr_t our_info;
   caddr_t call_info;
{
   fnord_widget *output = CAST(fnord_widget *, our_info);
   fnord_widget *view;
   char *text, name[80];
   int i, j;
   Sym sym;
   METset *set;
   METsym *view_def;

   text = XmTextGetString(output->info.output.file_editor);
   EnvSetOutputFile (widget__top->info.top.env, text);
   XtFree(text);
   
   text = XmTextGetString (output->info.output.view_editor);
   
   i = 0;

   do {
      while (text[i] == ',' || text[i] == ' ' || text[i] == '\t')
	 i++;
      
      if (text[i] == 0)
	 break;

      j = 0;
      while ((text[i] != ' ' && text[i] != ',' && 
	      text[i] != '\t' && text[i] != 0))
	 name[j++] = text[i++];

      name[j] = 0;
      sym = sym_from_string(name);
      set = METenv_lookup(output->info.output.env, sym);
      if (set == MET_NULL_SET)
      {
	 fprintf(stderr, "Printing: unknown widget %s.\n", name);
	 continue;
      }
      
      view_def = set->definition;
      if (MET_SYM_TYPE_CONTROL != view_def->type)
      {
	 fprintf(stderr, "Printing: unknown widget %s.\n", name);
	 MET_SET_FREE(set);
	 continue;
      }
      
      view = CAST(fnord_widget *, view_def->info.control.data);
      MET_SET_FREE(set);
      if (WIDGET_TYPE_VIEW != view->type)
      {
	 fprintf(stderr, "Printing: can only output from view widgets.\n");
	 continue;
      }
      
      EnvDrawAdd (widget__top->info.top.env, 
		  ViewGetPict(view->xw.w),
		  output->info.output.draw_style);

   } while (1);

   EnvFlush (widget__top->info.top.env);
   XtFree(text);
}

static void td_create_output_widget(parent, output)
   Widget parent;
   fnord_widget *output;

{
   Arg arg[10];
   char s[100];
   int i = 0;
   XtCallbackRec callbacks[2];
   ME(td_create_output_widget);
   
   output->xw.top = XmCreateFrame(parent, output->name, NULL, 0);

   i = 0;
   XtSetArg(arg[i], XmNfractionBase, 20); i++;
   output->xw.w = XmCreateForm (output->xw.top, output->name, arg, i);

   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION); i++;
   
   XtSetArg(arg[i], XmNeditMode, XmSINGLE_LINE_EDIT); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;

   XtSetArg(arg[i], XmNtopPosition, 11); i++;
   XtSetArg(arg[i], XmNbottomPosition, 14); i++;
   XtSetArg(arg[i], XmNleftPosition, 7); i++;
   XtSetArg(arg[i], XmNrightPosition, 20); i++;

   output->info.output.view_editor = XmCreateText(output->xw.w, output->name,
						  arg, i);
   s[0] = 0;
   XmTextSetString(output->info.output.view_editor, s);
   XtOverrideTranslations(output->info.output.view_editor, 
			  td_text_translations);
   
   i = 6;
   XtSetArg(arg[i], XmNtopPosition, 14); i++;
   XtSetArg(arg[i], XmNbottomPosition, 17); i++;
   XtSetArg(arg[i], XmNleftPosition, 7); i++;
   XtSetArg(arg[i], XmNrightPosition, 20); i++;

   output->info.output.file_editor = XmCreateText(output->xw.w, output->name,
						  arg, i);
   
   XmTextSetString(output->info.output.file_editor, "fnord.ps");
   XtOverrideTranslations(output->info.output.file_editor, 
			  td_text_translations);

   i = 4;
   XtSetArg(arg[i], XmNtopPosition, 11); i++;
   XtSetArg(arg[i], XmNbottomPosition, 14); i++;
   XtSetArg(arg[i], XmNleftPosition, 0); i++;
   XtSetArg(arg[i], XmNrightPosition, 7); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("include views:", XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   output->info.output.view_label = XmCreateLabel(output->xw.w, output->name,
						  arg, i);

   i = 4;
   XtSetArg(arg[i], XmNtopPosition, 14); i++;
   XtSetArg(arg[i], XmNbottomPosition, 17); i++;
   XtSetArg(arg[i], XmNleftPosition, 0); i++;
   XtSetArg(arg[i], XmNrightPosition, 7); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("to file:", XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   output->info.output.file_label = XmCreateLabel(output->xw.w, output->name,
						  arg, i);

   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopPosition, 17); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNleftPosition, 13); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("Print", XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.button =
      XmCreatePushButton(output->xw.w, "print", arg, i);
   XtAddCallback(output->info.output.button, XmNactivateCallback,
		 td_output_print, output);

   i = 0;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNbottomPosition, 3); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate(output->name, XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.label = XmCreateLabel(output->xw.w, output->name, 
					     arg, i);

   i = 0;
   callbacks[0].callback = td_output_set_style;
   callbacks[0].closure = (caddr_t) output;
   callbacks[1].callback = NULL;
   callbacks[1].closure = NULL;
   XtSetArg(arg[i], XmNentryCallback, callbacks); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopPosition, 3); i++;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNbottomPosition, 10); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNentryClass, xmToggleButtonGadgetClass); i++;
   output->info.output.radio =
      XmCreateRadioBox (output->xw.w, output->name, arg, i);

   i = 0;
   XtSetArg (arg[i], XmNset, True); i++;
   XtSetArg (arg[i], XmNlabelString, 
	     XmStringCreateLtoR("black and white postscript",
				XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.tb1 = 
      XmCreateToggleButtonGadget (output->info.output.radio,
				  output->name, arg, i);
   XtManageChild (output->info.output.tb1);

   i = 0;
   XtSetArg (arg[i], XmNset, False); i++;
   XtSetArg (arg[i], XmNlabelString, 
	     XmStringCreateLtoR("gray scale postscript",
				XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.tb2 =
      XmCreateToggleButtonGadget (output->info.output.radio,
				  output->name, arg, i);
   XtManageChild (output->info.output.tb2);

   i = 0;
   XtSetArg (arg[i], XmNset, False); i++;
   XtSetArg (arg[i], XmNlabelString, 
	     XmStringCreateLtoR("full color postscript",
				XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.tb3 =
      XmCreateToggleButtonGadget (output->info.output.radio,
				  output->name, arg, i);
   XtManageChild (output->info.output.tb3);

}

static METsym *td_output_create(parent, name, alist, sym)
   fnord_widget *parent;
   char *name;
   METset *alist;
   METsym *sym;
{
   fnord_widget	*output;
   widget_list  *list;
   ME(td_output_create);
   
   ALLOCM(output, fnord_widget);
   output->type = WIDGET_TYPE_OUTPUT;
   (void) strcpy (output->name, name);
   output->kids = WL_NULL;
   output->dft_kid_type = WIDGET_TYPE_NONE;

   output->xw.w = output->xw.top = None;
   td_link_in(&parent, name, WIDGET_TYPE_OUTPUT);
   
   if (parent == NULL)
   {
      FREE(output, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }

   output->info.output.draw_style = ENV_BW_POSTSCRIPT_MODE;
   MET_ENV_COPY(output->info.output.env, parse_root_environment);

   td_create_output_widget (parent->xw.w, output);
   td_winkid_geom(parent, output, alist, sym);

   XtManageChild (output->xw.top);
   XtManageChild (output->xw.w);
   XtManageChild (output->info.output.button);
   XtManageChild (output->info.output.label);
   XtManageChild (output->info.output.file_editor);
   XtManageChild (output->info.output.file_label);
   XtManageChild (output->info.output.view_editor);   
   XtManageChild (output->info.output.view_label);
   XtManageChild (output->info.output.radio);

   ALLOCM(list, widget_list);
   list->w = output;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) output, sym->origin);
}

static void td_output_destroy(output)
   fnord_widget		*output;
{
   ME(td_output_destroy);

   XtDestroyWidget(output->info.output.file_label );
   XtDestroyWidget(output->info.output.view_label );
   XtDestroyWidget(output->info.output.file_editor );
   XtDestroyWidget(output->info.output.view_editor );
   XtDestroyWidget(output->info.output.label );
   XtDestroyWidget(output->info.output.button );
   XtDestroyWidget(output->xw.w );
   XtDestroyWidget(output->xw.top );
   MET_ENV_FREE_NULL(output->info.output.env );
}

#endif /* MOTIF_WIDGETS */
