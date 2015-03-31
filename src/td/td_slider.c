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
   "$Id: td_slider.c,v 1.3 1993/06/03 14:16:41 rch Exp $";
#endif /* lint */

#ifndef MOTIF_WIDGETS

fnw_type fnw_type_slider = {
   WIDGET_TYPE_SLIDER,	/* type */
   STD_ARG_SLIDER,	/* name arg */
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

static METsym	*td_slider_create();
static void	 td_slider_destroy();
static FOboolean td_slider_request();

fnw_type fnw_type_slider = {
   WIDGET_TYPE_SLIDER, 	/* type */
   STD_ARG_SLIDER,	/* name arg */
   td_slider_create,	/* create */
   td_slider_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_slider_request,	/* request */
};

/* ARGSUSED */
static void td_slider_changed( w, our_info, call_data )
     Widget w;			/* Slider bar changed.		*/
     caddr_t our_info;		/* Pointer to Slider structure	*/
     caddr_t call_data;		/* What generated the callback	*/

{
   fnord_widget *slider = CAST(fnord_widget *, our_info);
   int negexp = 0;		/* True if the widget multiplied*/
   int temp;			/* Value of the widget's param	*/
   short dpts;			/* Exponent of ten by which this
				   value is shifted in display	*/
   double tens;			/* 10 ** dpts			*/
   Arg args[1];			/* Used in obtaining value.	*/
   ME(xtd_slider_changed);

   /* Compute the value currently stored in the slider bar.	*/
   
   XtSetArg(args[0], XmNdecimalPoints, &dpts);
   XtGetValues(w, args, 1);
   if (dpts < 0) {
      dpts = -dpts;
      negexp = 1;
   } 
   for(temp = 0, tens = 1.0; temp < dpts; temp++)
      tens *= 10.0;
   XmScaleGetValue(w, &temp);
   slider->info.slider.val = (double) temp;
   if (negexp)
      slider->info.slider.val *= tens;
   else
      slider->info.slider.val /= tens;

   slider->state |= FNW_STATE_CHANGED_NOW;

   /* Do something (hopefully) interesting with this value.	*/
   
   td_update();
}

static void td_create_slider_widget(parent, slider, sym)
   Widget 	parent;
   fnord_widget *slider;
   METsym	*sym;
{
   Arg arg[9];
   int i = 0;
   int value;
   int scale;
   int digits;
   ME(td_create_slider_widget);

   switch (slider->info.slider.field) {
    case MET_FIELD_ZTWO:
    case MET_FIELD_INTEGER:
      digits = 0; scale = 1; break;
    case MET_FIELD_REAL:
    case MET_FIELD_COMPLEX:
      digits = 2; scale = 100; break;
    default:
      TD_ERR(sym->origin, ": forcing slider value to real number.\n");
      digits = 2; scale = 100;
      slider->info.slider.field = MET_FIELD_REAL;
      break;
   }

   slider->xw.top = XmCreateFrame(parent, slider->name, (Arg *) NULL, 0);
   XtSetArg(arg[i], XmNdecimalPoints, digits); i++;
   value = (int) (scale * slider->info.slider.max);
   value = (value < 0) ? value - 1 : value;
   XtSetArg(arg[i], XmNmaximum, value); i++;
   value = (int) (scale * slider->info.slider.min);
   value = (value < 0) ? value - 1 : value;
   XtSetArg(arg[i], XmNminimum, value); i++;
   value = (int) (scale * slider->info.slider.val);
   XtSetArg(arg[i], XmNvalue, value); i++;
   XtSetArg(arg[i], XmNtitleString,
	    XmStringCreateLtoR(slider->name,
			       XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   XtSetArg(arg[i], XmNprocessingDirection, XmMAX_ON_RIGHT); i++;
   XtSetArg(arg[i], XmNshowValue, True); i++;

   slider->xw.w = XmCreateScale( slider->xw.top, slider->name, arg, i);

   XtAddCallback( slider->xw.w, XmNvalueChangedCallback,
		 td_slider_changed, (char *) slider);
   if (YES == slider->info.slider.drag_update)
      XtAddCallback( slider->xw.w, XmNdragCallback,
		    td_slider_changed, (char *) slider);
}

static METsym *td_slider_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget *slider;
   widget_list 	*list;
   ME(td_slider_create);

   ALLOCM(slider, fnord_widget);

   slider->type = WIDGET_TYPE_SLIDER;
   (void) strcpy(slider->name, name);
   slider->kids = WL_NULL;
   slider->dft_kid_type = WIDGET_TYPE_NONE;
   slider->state = FNW_STATE_START;

   slider->xw.w = slider->xw.top = NULL;

   (void) td_lookup_real(alist, std_args[STD_ARG_MIN],
			 &slider->info.slider.min, 
			 0.0, (Sym) NULL, 0.0);
   
   (void) td_lookup_real(alist, std_args[STD_ARG_MAX],
			 &slider->info.slider.max, 
			 slider->info.slider.min + 1.0, (Sym) NULL, 0.0);

   if (slider->info.slider.min > slider->info.slider.max) {
      TD_ERR(sym->origin, ": slider min greater than max.\n");
      slider->info.slider.max = slider->info.slider.min + 1.0;
   }

   (void) td_lookup_boolean(alist, std_args[STD_ARG_DRAG],
			    &slider->info.slider.drag_update, 1,
			    (Sym) NULL, 0);

   (void) td_lookup_real(alist, std_args[STD_ARG_INIT],
			 &slider->info.slider.val,
			 slider->info.slider.min, std_args[STD_ARG_MAX], 
			 slider->info.slider.max);

   (void) td_lookup_field(alist, std_args[STD_ARG_FIELD],
			  &slider->info.slider.field,
			  MET_FIELD_REAL, (Sym) NULL,
			  MET_FIELD_REAL);

   td_link_in(&parent, name, WIDGET_TYPE_SLIDER, sym);

   if (parent == NULL) {
      FREE(slider, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   
   td_create_slider_widget(parent->xw.w, slider, sym);

   td_winkid_geom(parent, slider, alist, sym);
   
   XtManageChild( slider->xw.top );
   XtManageChild( slider->xw.w );

   ALLOCM(list, widget_list);
   list->w = slider;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) slider, sym->origin);
}

static void td_slider_destroy(slider)
   fnord_widget		*slider;
{
   ME(td_slider_destroy);

   XtDestroyWidget(slider->xw.w);
   XtDestroyWidget(slider->xw.top);
}

static FOboolean td_slider_request(slider, req)
   fnord_widget		*slider;
   METcontrol_request	*req;
{
   METnumber num;
   ME(td_slider_request);

   switch(req->code) {
    case MET_CONTROL_REQUEST_CHANGED:
      FNW_SET_REQ_CHANGED(req, slider);
      break;

    case MET_CONTROL_REQUEST_TYPE:
      MET_TYPE_NEW_FIELD(req->type, slider->info.slider.field);
      break;

    case MET_CONTROL_REQUEST_VALUE:
      switch (slider->info.slider.field) {
       case MET_FIELD_ZTWO:
	 num.ztwo = (METztwo) slider->info.slider.val;
	 break;
       case MET_FIELD_INTEGER:
	 num.integer = (METinteger) slider->info.slider.val;
	 break;
       case MET_FIELD_REAL:
	 num.real = (METreal) slider->info.slider.val;
	 break;
       case MET_FIELD_COMPLEX:
	 num.complex.real = (METreal) slider->info.slider.val;
	 num.complex.imaginary = (METreal) 0.0;
	 break;
       default:
	 DEFAULT_ERR(slider->info.slider.field);
      }
      req->value = METset_new_number(slider->info.slider.field, &num);
      break;

    default:
      DEFAULT_ERR(req->code);
   }
   return SUCCESS;
}
#endif /* MOTIF_WIDGETS */

