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
   "$Id: td_video.c,v 1.1 1993/06/03 17:00:20 rch Exp $";
#endif /* lint */

#include "td/defs.h"
#include "parse/parse.h"

#ifndef MOTIF_WIDGETS

fnw_type fnw_type_video = {
   WIDGET_TYPE_VIDEO,   /* type */
   STD_ARG_VIDEO,	/* name arg */
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

static METsym 	*td_video_create();
static void	td_video_destroy();
static void 	td_video_clear();
static void	td_video_update();
static void	td_video_frame();

/* Fnord widget definition for videos */

fnw_type fnw_type_video = {
   WIDGET_TYPE_VIDEO,   /* type */
   STD_ARG_VIDEO,	/* name arg */
   td_video_create,	/* create */
   td_video_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   td_video_clear,	/* clear */
   td_video_update,	/* update */
   td_video_frame,	/* post update */
   td_cant_request,	/* request */
};

static void td_video_clear(video)
   fnord_widget		*video;
{
   int	changed;
   ME(td_video_clear);

   if (!FNW_IS_TESTED(video) && 
       MET_NULL_SET != video->info.video.ref_set) {
      changed = METset_clear_rep(video->info.video.ref_set);
      FNW_SET_UPDATE_MET(video, changed);
      video->state |= FNW_STATE_TESTED;
   }
}

static void td_video_update(video)
   fnord_widget		*video;
{
   ME(td_video_update);

   if (MET_NULL_SET != video->info.video.ref_set)
      METset_have_rep(video->info.video.ref_set, (FOboolean *) NULL);
}

static void td_video_frame(video)
   fnord_widget		*video;
{
   ME(td_video_frame);

   if (FNW_NEEDS_UPDATE(video) &&
       YES == video->info.video.on) {
      system(getenv("FNORD_VIDEO_CMD"));
   }
}

/* ARGSUSED */
static void td_video_toggle (widget, our_info, call_info)
   Widget widget;
   caddr_t our_info;
   caddr_t call_info;
{
   fnord_widget *video = CAST(fnord_widget *, our_info);
   XmToggleButtonCallbackStruct *tbcs =
      CAST(XmToggleButtonCallbackStruct *, call_info);

   if (tbcs->set == False)
      video->info.video.on = NO;
   else
      video->info.video.on = YES;
}

/* ARGSUSED */
static void td_video_changed( w, our_info, call_data )
     Widget w;			/* Video bar changed.		*/
     caddr_t our_info;		/* Pointer to Video structure	*/
     caddr_t call_data;		/* What generated the callback	*/

{
   fnord_widget *video = CAST(fnord_widget *, our_info);
   char *s, *temp;
   char name[120];
   int len;
   METset *result;
   FOboolean ok;
   ME(td_video_changed);

   s = XmTextGetString(w);
   /* Eek */
   len = strlen(s);
   s = XtRealloc(s, len + 2);
   s[len++] = ';';
   s[len] = 0;
   temp = widget__top->info.top.text;
   widget__top->info.top.text = s;
   sprintf(name, "video widget (%s)", video->name);
   (void) fnord_parse (sym_from_string(name), -1, td_text_read, 
		       PARSE_NO_FLAGS, PARSE_ALL_FLAGS);
   widget__top->info.top.text = temp;
   XtFree(s);
   result = parse_last_expr();
   if ((METset *) NULL == result)
      return;

   MET_SET_FREE_NULL (video->info.video.ref_set);
   video->info.video.ref_set = result;

   td_update();
}

static Widget td_create_video_widget(parent, video, init_text)
   Widget parent;
   fnord_widget *video;
   char	*init_text;
{
   Arg arg[10];
   int i = 0;
   XtCallbackRec callbacks[2];
   Widget label;
   Widget widget;
   ME(td_create_video_widget);
   
   video->xw.top = XmCreateFrame(parent, video->name, NULL, 0);

   i = 0;
   XtSetArg(arg[i], XmNfractionBase, 2); i++;
   video->xw.w = XmCreateForm (video->xw.top, video->name, arg, i);

   i = 0;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNbottomPosition, 1); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_NONE); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("reference set:", XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   label = XmCreateLabel(video->xw.w, video->name, arg, i);
   XtManageChild(label);
   
   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopPosition, 1); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg (arg[i], XmNset, False); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("Recording on", XmSTRING_DEFAULT_CHARSET)); i++;
   widget = XmCreateToggleButton(video->xw.w, "button", arg, i);
   XtAddCallback(widget, XmNvalueChangedCallback,
		 td_video_toggle, CAST(char *, video));
   XtManageChild(widget);

   i = 0;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_WIDGET); i++;
   XtSetArg(arg[i], XmNleftWidget, label); i++;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNbottomPosition, 1); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   
   XtSetArg(arg[i], XmNeditMode, XmSINGLE_LINE_EDIT); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;

   widget = XmCreateText(video->xw.w, video->name, arg, i);
   XmTextSetString(widget, init_text);
   XtOverrideTranslations(widget, td_text_translations);
   XtAddCallback(widget, XmNactivateCallback, 
		 td_video_changed, CAST(char *, video));
   XtManageChild(widget);

   return widget;
}

static METsym *td_video_create(parent, name, alist, sym)
   fnord_widget *parent;
   char *name;
   METset *alist;
   METsym *sym;
{
   METset	*init;
   char		init_text[1024];
   int		length = 1023;
   fnord_widget	*video;
   widget_list  *list;
   FOboolean	call_changed = NO;
   Widget	widget;
   ME(td_video_create);
   
   ALLOCM(video, fnord_widget);
   video->type = WIDGET_TYPE_VIDEO;
   (void) strcpy (video->name, name);
   video->kids = WL_NULL;
   video->dft_kid_type = WIDGET_TYPE_NONE;

   if (CNULL == getenv("FNORD_VIDEO_CMD")) {
      (void) fprintf(stderr, "must set environment variable \
FNORD_VIDEO_CMD.\n");
      FREE(video, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }

   video->xw.w = video->xw.top = None;
   video->info.video.ref_set = MET_NULL_SET;
   td_link_in(&parent, name, WIDGET_TYPE_VIDEO);
   
   if (parent == NULL)
   {
      FREE(video, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }

   init = METset_assoc (alist, std_args[STD_ARG_INIT]);
   if (MET_NULL_SET == init)
   {
      init_text[0] = 0;
   }
   else if (!METset_get_string(init, init_text, &length))
   {
      MET_SET_FREE(init);
      init_text[0] = 0;
   }
   else
   {
      MET_SET_FREE(init);
      call_changed = YES;
   }

   video->info.video.on = NO;

   widget = td_create_video_widget (parent->xw.w, video, init_text);
   td_winkid_geom(parent, video, alist, sym);

   XtManageChild (video->xw.top);
   XtManageChild (video->xw.w);

   if (YES == call_changed)
      XtCallCallbacks (widget, XmNactivateCallback, (caddr_t) NULL);

   ALLOCM(list, widget_list);
   list->w = video;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) video, sym->origin);
}

static void td_video_destroy(video)
   fnord_widget		*video;
{
   ME(td_video_destroy);

   XtDestroyWidget(video->xw.w );
   XtDestroyWidget(video->xw.top );
   MET_SET_FREE_NULL(video->info.video.ref_set);
}

#endif /* MOTIF_WIDGETS */
