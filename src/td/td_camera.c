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

#ifndef lint
static char Version[] =
   "$Id: td_camera.c,v 1.4 1993/06/03 14:16:29 rch Exp $";
#endif /* lint */

#include "td/defs.h"

#ifndef X_TWO_D

fnw_type fnw_type_camera = {
   WIDGET_TYPE_CAMERA,  /* type */
   STD_ARG_CAMERA,	/* name arg */
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

static void	td_camera_destroy();
static void 	td_camera_step();
static void 	td_camera_secure();
static FOboolean td_camera_request();

fnw_type fnw_type_camera = {
   WIDGET_TYPE_CAMERA,  /* type */
   STD_ARG_CAMERA,	/* name arg */
   td_cant_create,	/* create */
   td_camera_destroy,	/* destroy */
   td_camera_secure,	/* secure */
   td_camera_step,	/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_camera_request,	/* request */
};

/* This is the function called by the Virtual Sphere widget when 
   its matrix changes, to update all of its views.  */

static void td_vs_callback (w, our_info, call_info)
   Widget w;
   caddr_t our_info;
   caddr_t call_info;
{
   fnord_widget *camera = CAST(fnord_widget *, our_info);
   widget_list *curr;
   VirtualSphereCallbackStruct *s = 
      CAST(VirtualSphereCallbackStruct *, call_info);
   Pict pict;
   Spin spin;
   ME(xtd_vs_callback);
   
   if (s->state != s->old_state) 
      if (VS_SPINNING == s->state || VS_SPINNING == s->old_state) {
	 camera->state ^= FNW_STATE_RUNNING;
	 td_register_running(FNW_IS_RUNNING(camera));
      }
   
   if (s->draw_now) {
      vs_spin_get(w, spin);
      if (FNW_IS_RUNNING(camera))
	 camera->state |= FNW_STATE_CHANGED_LATER;
      camera->state |= FNW_STATE_CHANGED_NOW;

      for (curr = camera->info.camera.affects;
	   curr != WL_NULL; curr = curr->next) 
	 switch(curr->w->type) {
	  case WIDGET_TYPE_VIEW:
	    pict = ViewGetPict (curr->w->xw.w);
	    PictSetSpin (pict, spin);
	    curr->w->state |= FNW_STATE_UPDATE_NOW;
	    break;
	  case WIDGET_TYPE_POINT:
	    if (curr->w->info.point.down == YES)
	       curr->w->state |= FNW_STATE_UPDATE_NOW;
	    break;
	  default:
	    curr->w->state |= FNW_STATE_UPDATE_NOW;
	    break;
	 }

      td_update();
   }
}

/* ARGSUSED */
fnord_widget *td_new_vs(parent, view, name, t, l, b, r)
   fnord_widget *parent, *view;
   char *name;
   int t, l, b, r;
{
   fnord_widget *vs;
   widget_list *list;
   ME(td_new_vs);
   
   ALLOCM(vs, fnord_widget);
   
   vs->type = WIDGET_TYPE_CAMERA;
   (void) strcpy(vs->name, name);
   vs->kids = NULL;
   vs->dft_kid_type = WIDGET_TYPE_NONE;
   vs->state = FNW_STATE_START;

   vs->xw.w = vs->xw.top = None;
   
#ifdef MOTIF_WIDGETS
   vs->xw.w = XtCreateWidget("vs", virtualSphereWidgetClass, 
			     parent->xw.w, (ArgList) NULL, 0);
   td_attach_in_window(parent, vs, t, l, b, r);
#else
   vs->xw.w = XtCreateWidget("vs", virtualSphereWidgetClass, 
			     view->xw.top, NULL, 0);
#endif /* defined(MOTIF_WIDGETS) */
   
   XtManageChild(vs->xw.w);
   
   vs->info.camera.affects = WL_NULL;
   XtAddCallback (vs->xw.w, XtNchangeCallback,
		  td_vs_callback, (caddr_t) vs);
   
   ALLOCM(list, widget_list);
   list->w = vs;
   list->next = parent->kids;
   parent->kids = list;
   
   return vs;
}

static void td_camera_destroy(camera)
   fnord_widget		*camera;
{
   widget_list		*curr, *next;
   ME(td_camera_destroy);
   
   XtDestroyWidget(camera->xw.w);
   
   for (curr = camera->info.camera.affects; curr != WL_NULL; curr = next) {
      next = curr->next;
      FREE(curr, sizeof(widget_list));
   }
}

static void td_camera_secure(camera)
   fnord_widget 	*camera;
{
   widget_list		*curr, *prev, *temp;
   
   prev = curr = camera->info.camera.affects;
   
   while (curr != WL_NULL)
      if (FNW_IS_DOOMED(curr->w)) {
	 temp = curr;
	 if (curr == camera->info.camera.affects)
	    curr = prev = camera->info.camera.affects = curr->next;
	 else
	    curr = prev->next = curr->next;
	 FREE( temp, sizeof(widget_list));
      } else {
	 prev = curr;
	 curr = curr->next;
      }
}

static void td_camera_step(camera)
   fnord_widget		*camera;
{
   widget_list		*curr;
   Pict 		pict;
   Spin 		spin;
   ME(td_camera_step);

   vs_spin_incr(camera->xw.w);
   vs_spin_get(camera->xw.w, spin);
   
   for (curr = camera->info.camera.affects;
	curr != WL_NULL;
	curr = curr->next) 
      switch(curr->w->type) {
       case WIDGET_TYPE_VIEW:
	 pict = ViewGetPict (curr->w->xw.w);
	 PictSetSpin (pict, spin);
	 curr->w->state |= FNW_STATE_UPDATE_NOW;
	 break;
       case WIDGET_TYPE_POINT:
	 if (curr->w->info.point.down == YES)
	    curr->w->state |= FNW_STATE_UPDATE_NOW;
	 break;
       default:
	 curr->w->state |= FNW_STATE_UPDATE_NOW;
	 break;
      }
   	
   camera->state |= FNW_STATE_CHANGED;
}

static FOboolean td_camera_request(camera, req)
   fnord_widget		*camera;
   METcontrol_request 	*req;
{
   int 		i, dims[2];
   Real		rot[3][3], *r;
   METreal	mrot[3][3], *mr;
   Spin		spin;
   ME(td_point_request);

   switch (req->code) {
    case MET_CONTROL_REQUEST_CHANGED:
      FNW_SET_REQ_CHANGED(req, camera);
      break;

    case MET_CONTROL_REQUEST_TYPE:
      req->type = MET_NULL_TYPE;
      break;

    case MET_CONTROL_REQUEST_VALUE:
      vs_spin_get(camera->xw.w, spin);
      spin_to_matrix(rot, spin);
      dims[0] = 3;
      dims[1] = 3;
      if (sizeof(METreal) != sizeof(Real)) {
	 mr = mrot[0], r = rot[0];
	 mr[0] = r[0];
	 mr[1] = r[1];
	 mr[2] = r[2];
	 mr[3] = r[3];
	 mr[4] = r[4];
	 mr[5] = r[5];
	 mr[6] = r[6];
	 mr[7] = r[7];
	 mr[8] = r[8];
	 req->value = METset_new_tensor(MET_FIELD_REAL, 
					(METnumber *) mrot,
					2, dims);
      } else
	 req->value = METset_new_tensor(MET_FIELD_REAL, 
					(METnumber *) rot,
					2, dims);
      break;

    default:
      DEFAULT_ERR(req->code);
   }

   return SUCCESS;
}

#endif /* X_TWO_D */
