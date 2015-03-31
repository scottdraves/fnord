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
   "$Id: td_point.c,v 1.3 1993/06/03 14:16:39 rch Exp $";
#endif /* lint */

#ifndef X_TWO_D

fnw_type fnw_type_point = {
   WIDGET_TYPE_POINT,  	/* type */
   STD_ARG_POINT,	/* name arg */
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

static METsym		*td_point_create();
static void		td_point_destroy();
static FOboolean 	td_point_request();
static void		td_point_step();
static void 		td_point_update();
static void		td_point_secure();

fnw_type fnw_type_point = {
   WIDGET_TYPE_POINT,  	/* type */
   STD_ARG_POINT,	/* name arg */
   td_point_create,	/* create */
   td_point_destroy,	/* destroy */
   td_point_secure,	/* secure */
   td_point_step,	/* step */
   NULL,		/* clear */
   td_point_update,	/* update */
   NULL,		/* post update */
   td_point_request,	/* request */
};

static void td_point_update(point)
   fnord_widget		*point;
{
   int screen_pt[2];
   Real pt[3];
   Camera mycam;
   Real xform[3][4];
   Real new_pt[3];
   ME(td_point_update);

   point_get_loc(point->xw.w, screen_pt);
   if (point->info.point.is_in_view) {
                      /* the output code does this, so i must too .  i */
		      /* don't know why it isn't built into the matrix. */

      /* if dims=2, the results aren't what you expect if you rotate the view.
	 this is ok w/me, since if it's 2d you probably aren't rotating.
	 to do it properly requires choosing z so that the xformed point
	 lies on xy plane.  currently we fiat that z=0. */
      mycam = PictGetCamera(ViewGetPict(point->info.point.view->xw.w));
      pt[0] = screen_pt[0];
      pt[1] = -screen_pt[1]; 
      pt[2] = mycam->xform[2][3];
      CameraGetInverseXform(mycam, xform);
      new_pt[0] = pt[0] * xform[0][0] + pt[1] * xform[0][1] + 
	 pt[2] * xform[0][2] + xform[0][3];
      new_pt[1] = pt[0] * xform[1][0] + pt[1] * xform[1][1] +
	 pt[2] * xform[1][2] + xform[1][3];
      new_pt[2] = pt[0] * xform[2][0] + pt[1] * xform[2][1] +
	 pt[2] * xform[2][2] + xform[2][3];
   } else {
      new_pt[0] = screen_pt[0];
      new_pt[1] = screen_pt[1];
      new_pt[2] = 0.0;
   }

   point->info.point.val[0] = new_pt[0];
   point->info.point.val[1] = new_pt[1];
   point->info.point.val[2] = new_pt[2];

}

/* ARGSUSED */
static void td_point_changed( w, our_info, call_data )
   Widget 	w;
   caddr_t 	our_info;
   caddr_t 	call_data;
{
   PointCallbackStruct	*pcbs = CAST(PointCallbackStruct *, call_data);
   fnord_widget 	*point = CAST(fnord_widget *, our_info);
   ME(td_point_changed);

   switch (pcbs->code) {
    case POINT_MOUSE_DOWN:
      point->info.point.down = YES;
      break;

    case POINT_MOUSE_LIFTED:
      point->info.point.down = NO;
      break;

    case POINT_MOUSE_MOVED:
      break;

    default:
      DEFAULT_ERR(pcbs->code);
   }

   td_point_update(point);
   point->state |= FNW_STATE_CHANGED_NOW;
   if (point->info.point.strobe) {

      point->state |= FNW_STATE_CHANGED_LATER;

      if (FNW_IS_RUNNING(point)) {
	 point->state &= ~FNW_STATE_RUNNING;
	 td_register_running(FALSE);
      }

      point->info.point.strobe_val = TRUE;
      td_update();

      if (!FNW_IS_RUNNING(point)) {
	 point->state |= FNW_STATE_RUNNING;
	 td_register_running(TRUE);
      }
   } else {
      td_update();
   }
}

static void td_point_step(point)
   fnord_widget		*point;
{
   ME(td_point_step);

   point->info.point.strobe_val = FALSE;
   FNW_SET_CHANGED(point, FNW_STATE_CHANGED_NOW);
   point->state &= ~FNW_STATE_RUNNING;
   td_register_running(FALSE);
}

static FOboolean td_point_request(point, req)
   fnord_widget		*point;
   METcontrol_request 	*req;
{
   int 		dims[1];
   METset 	*v_part, *b_part;
   METnumber 	num;
   ME(td_point_request);

   if (FNW_NEEDS_UPDATE(point)) {
      td_point_update(point);
      point->state |= ((point->state & FNW_STATE_UPDATE_NOW) ?
		       FNW_STATE_CHANGED_NOW : 0) |
			  ((point->state & FNW_STATE_UPDATE_LATER) ?
			   FNW_STATE_CHANGED_LATER : 0);
      point->state &= ~FNW_STATE_UPDATE;
      if (YES == point->info.point.down &&
	  !FNW_IS_RUNNING(point)) {
	 point->state |= FNW_STATE_RUNNING;
	 point->info.point.strobe_val = TRUE;
	 td_register_running(TRUE);
      }
   }

   switch (req->code) {
    case MET_CONTROL_REQUEST_CHANGED:
      FNW_SET_REQ_CHANGED(req, point);
      break;

    case MET_CONTROL_REQUEST_TYPE:
      /* too hard to do this right right now */
      req->type = MET_NULL_TYPE;
      break;

    case MET_CONTROL_REQUEST_VALUE:
      dims[0] = point->info.point.dims;
      v_part = METset_new_tensor(MET_FIELD_REAL, 
				 (METnumber *) point->info.point.val,
				 1, dims);

      if (point->info.point.strobe) {
	 num.ztwo = (METztwo) point->info.point.strobe_val;
	 b_part = METset_new_number(MET_FIELD_ZTWO, &num);
	 req->value = METset_form_pair(b_part, v_part);
	 MET_SET_FREE(b_part);
      } else {
	 MET_SET_COPY(req->value, v_part);
      }
      MET_SET_FREE(v_part);
      break;

    default:
      DEFAULT_ERR(req->code);
   }

   return SUCCESS;
}

static void td_point_destroy(point)
   fnord_widget		*point;
{
   ME(td_point_destroy);
   
   XtDestroyWidget(point->xw.w);
}

static void td_point_secure(point)
   fnord_widget		*point;
{
   ME(td_point_secure);
   
   if (point->info.point.is_in_view)
      if (FNW_IS_DOOMED(point->info.point.view)) {
	 point->info.point.is_in_view = FALSE;
	 point->info.point.view = FNW_NULL;
      }
}
	 
static void td_create_point_widget(parent, point)
   Widget 	parent;
   fnord_widget *point;
{
   Arg arg[1];
   int i;
   char buf[200]; /* > 84 + 3 * 10 */
   XtAccelerators accel;
   ME(xtd_create_point_widget);

   sprintf(buf,
	   "%s<Btn2Down>:	TrackStart()\n\
	    %s<Btn2Motion>:	Track()\n\
	    %s<Btn2Up>:		TrackEnd()",
	   point->info.point.modifiers,
	   point->info.point.modifiers,
	   point->info.point.modifiers);

   accel = XtParseAcceleratorTable((String) buf);

   i = 0;
   
   XtSetArg(arg[i], XtNaccelerators, accel); i++;
   point->xw.w = XtCreateWidget("point", pointWidgetClass, parent, arg, i);

   XtAddCallback(point->xw.w, XtNchangeCallback, 
		 td_point_changed, (char *) point);
   if (point->info.point.drag)
      XtAddCallback(point->xw.w, XtNdragCallback, 
		    td_point_changed, (char *) point);
}

static METsym *
td_point_create(parent, name, alist, sym)
   fnord_widget *parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   fnord_widget *point, *camera;
   widget_list *list;
   METset *assoc;
   int len = 9;
   Widget the_view;

   ME(xtd_new_point);

   ALLOCM(point, fnord_widget);

   point->type = WIDGET_TYPE_POINT;
   (void) strcpy(point->name, name);
   point->kids = WL_NULL;
   point->dft_kid_type = WIDGET_TYPE_NONE;
   point->state = FNW_STATE_START;

   point->xw.w = point->xw.top = (Widget) NULL;
   point->info.point.down = NO;
   
   /* initialize val from init */ /* Ack! */

   (void) td_lookup_boolean(alist, std_args[STD_ARG_STROBE],
			    &point->info.point.strobe, NO,
			    (Sym) NULL, NO);

   (void) td_lookup_boolean(alist, std_args[STD_ARG_DRAG],
			    &point->info.point.drag,
			    !point->info.point.strobe,
			    (Sym) NULL, YES);

   (void) td_lookup_int(alist, std_args[STD_ARG_DIMS],
			&point->info.point.dims, 2,
			(Sym) NULL, 2);

   assoc = METset_assoc(alist, std_args[STD_ARG_MODIFIERS]);
   if (MET_NULL_SET == assoc) {
      (void) strcpy(point->info.point.modifiers, "");
   } else if (! METset_get_string(assoc, point->info.point.modifiers,
				  &len)) {
      TD_ERR(sym->origin, ": modifiers not string with length < 10.\n");
      MET_SET_FREE(assoc);
      (void) strcpy(point->info.point.modifiers, "");
   } else {
      point->info.point.modifiers[len] = 0;
      MET_SET_FREE(assoc);
   }
      
   td_link_in(&parent, name, WIDGET_TYPE_POINT, sym);

   if (parent == NULL) {
      FREE(point, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   /*
    * if we are being installed in a view, then, what they really meant is
    * for us to be a sibling of the view.  keep a pointer to the view.
    * this means views must keep pointers to their parents, which they do
    */
   if (WIDGET_TYPE_VIEW == parent->type) {
      the_view = parent->xw.w;
      point->info.point.view = parent;
      camera = parent->info.view.camera;
      parent = parent->info.view.parent;
      point->info.point.is_in_view = 1;
      ALLOCM(list, widget_list);
      list->w = point;
      list->next = camera->info.camera.affects;
      camera->info.camera.affects = list;
   } else {
      the_view = (Widget) NULL;
      point->info.point.is_in_view = 0;
   }
   
   td_create_point_widget(parent->xw.w, point);

#ifdef MOTIF_WIDGETS
   td_winkid_geom(parent, point, alist, sym);
#endif

   XtManageChild(point->xw.w);

   if (point->info.point.is_in_view) {
      XtInstallAccelerators (the_view, point->xw.w);
      push_widget_under(the_view, point->xw.w);
   } else
      XtInstallAccelerators (point->xw.w, point->xw.w);

   ALLOCM(list, widget_list);
   list->w = point;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) point, sym->origin);
}

#endif /* X_TWO_D */

