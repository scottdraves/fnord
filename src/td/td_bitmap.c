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
static char Version[]=
   "$Id: td_bitmap.c,v 1.3 1993/06/03 14:16:25 rch Exp $";
#endif /* lint */

#ifndef X_TWO_D

fnw_type fnw_type_bitmap = {
   WIDGET_TYPE_BITMAP,  /* type */
   STD_ARG_BITMAP,	/* name arg */
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

static void	td_bitmap_destroy();
static METsym	*td_bitmap_create();
static FOboolean td_bitmap_request();

fnw_type fnw_type_bitmap = {
   WIDGET_TYPE_BITMAP,  /* type */
   STD_ARG_BITMAP,	/* name arg */
   td_bitmap_create,	/* create */
   td_bitmap_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_bitmap_request,	/* request */
};

/* ARGSUSED */
static void td_bitmap_changed( w, our_info, call_data )
   Widget w;
   caddr_t our_info;
   caddr_t call_data;
{
   fnord_widget	*bitmap = CAST(fnord_widget *, our_info);
   ME(td_bitmap_changed);

   bitmap->state |= FNW_STATE_CHANGED_NOW;

   td_update();
}

static void td_create_bitmap_widget(parent, bitmap)
   Widget 	parent;
   fnord_widget *bitmap;
{
   Arg args[4];
   int i;
   ME(td_create_bitmap_widget);

#ifdef MOTIF_WIDGETS
   bitmap->xw.top = XmCreateFrame(parent, bitmap->name, (Arg *) NULL, 0);
#endif
   
   i = 0;
   XtSetArg(args[i], XtNimageWidth, bitmap->info.bitmap.width); i++;
   XtSetArg(args[i], XtNimageHeight, bitmap->info.bitmap.height); i++;
   XtSetArg(args[i], XtNzoom, bitmap->info.bitmap.zoom); i++;
   bitmap->xw.w = XtCreateWidget(bitmap->name, pixelEditorWidgetClass,
#ifdef MOTIF_WIDGETS
			      bitmap->xw.top,
#else
			      parent,
#endif
			      args, i);
   XtAddCallback(bitmap->xw.w, XtNdataChangeCallback,
		 td_bitmap_changed, (caddr_t) bitmap);
}

static FOboolean td_bitmap_request(bitmap, req)
   fnord_widget		*bitmap;
   METcontrol_request	*req;
{
   Arg		args[1];
   int		dims[2];
   Pixel	*pixels;
   ME(xtd_bitmap_callback);

   switch (req->code) {
    case MET_CONTROL_REQUEST_TYPE:
      req->type = MET_NULL_TYPE;
      break;

    case MET_CONTROL_REQUEST_CHANGED:
      FNW_SET_REQ_CHANGED(req, bitmap);
      break;

    case MET_CONTROL_REQUEST_VALUE:
      dims[0] = bitmap->info.bitmap.width;
      dims[1] = bitmap->info.bitmap.height;
      XtSetArg(args[0], XtNimagePixels, &pixels);
      XtGetValues(bitmap->xw.w, args, 1);
      req->value = METset_new_tensor(MET_FIELD_INTEGER, 
				  CAST(METnumber *, pixels), 2, dims);
      break;

    default:
      DEFAULT_ERR(req->code);
   }

   return SUCCESS;
}

static void td_bitmap_destroy(bitmap)
   fnord_widget		*bitmap;
{
   ME(td_bitmap_destroy);

   XtDestroyWidget(bitmap->xw.w);
#ifdef MOTIF_WIDGETS
   XtDestroyWidget(bitmap->xw.top);
#endif
}

static METsym *td_bitmap_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget *bitmap;
   widget_list 	*list;
   ME(td_new_bitmap);

   ALLOCM(bitmap, fnord_widget);

   bitmap->type = WIDGET_TYPE_BITMAP;
   (void) strcpy(bitmap->name, name);
   bitmap->kids = WL_NULL;
   bitmap->dft_kid_type = WIDGET_TYPE_NONE;
   bitmap->state = FNW_STATE_START;

   bitmap->xw.w = bitmap->xw.top = (Widget) NULL;

   (void) td_lookup_int(alist, std_args[STD_ARG_WIDTH],
			 &bitmap->info.bitmap.width, 
			 16, (Sym) NULL, 16);

   (void) td_lookup_int(alist, std_args[STD_ARG_HEIGHT],
			 &bitmap->info.bitmap.height, 
			 16, (Sym) NULL, 16);

   (void) td_lookup_boolean(alist, std_args[STD_ARG_DRAG],
			     &bitmap->info.bitmap.drag_update, YES,
			     (Sym) NULL, YES);

   (void) td_lookup_int(alist, std_args[STD_ARG_ZOOM],
			 &bitmap->info.bitmap.zoom,
			 16, (Sym) NULL, 16);


   td_link_in(&parent, name, WIDGET_TYPE_BITMAP, sym);

   if (parent == NULL) {
      FREE(bitmap, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   
   td_create_bitmap_widget(parent->xw.w, bitmap);

#ifdef MOTIF_WIDGETS
   td_winkid_geom(parent, bitmap, alist, sym);
   XtManageChild( bitmap->xw.top );
#endif

   XtManageChild( bitmap->xw.w );

   ALLOCM(list, widget_list);
   list->w = bitmap;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) bitmap, sym->origin);
}

#endif /* defined(X_TWO_D) */
