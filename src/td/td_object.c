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
   "$Id: td_object.c,v 1.3 1993/06/03 14:16:36 rch Exp $";
#endif /* defined(lint) */

#ifndef X_TWO_D

fnw_type fnw_type_object = {
   WIDGET_TYPE_OBJECT,  /* type */
   STD_ARG_OBJECT,	/* name arg */
   td_cant_create,	/* create */
   NULL,		/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_cant_request,	/* request */
};

#else /* if !defined(X_TWO_D) */

/* forward references */

static METsym *td_object_create();
static void td_object_destroy();

fnw_type fnw_type_object = {
   WIDGET_TYPE_OBJECT,  /* type */
   STD_ARG_OBJECT,	/* name arg */
   td_object_create,	/* create */
   td_object_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_cant_request,	/* request */
};

/*
 *	This function adds the "viewable surface" which contains the
 *	specified set to the fnord widget specified--or a new one,
 *	if none is specified.
 */

static METsym *td_object_create(parent, name, alist, sym)
   fnord_widget	*parent;
   char 	*name;
   METset 	*alist;
   METsym	*sym;
{
   char 	buf[80], *color = buf;
   int 		length = 80;
   fnord_widget *object;
   METset 	*set, *good;
   widget_list 	*list;
   Pict 	pict;
   ME(td_object_create);

   ALLOCM(object, fnord_widget);

   object->type = WIDGET_TYPE_OBJECT;
   (void) strcpy(object->name, name);
   object->kids = WL_NULL;
   object->dft_kid_type = WIDGET_TYPE_NONE;
   object->state = FNW_STATE_START;

   object->xw.w = object->xw.top = None;

   /* Determine the color mapping specified by the user... */

   set = METset_assoc(alist, std_args[STD_ARG_COLOR]);
   if (MET_NULL_SET == set)
   {
      color = "#ffff00";
   }
   else if (! METset_get_string(set, color, &length))
   {
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout, ": color is not short string.\n");
      MET_SET_FREE(set);
      color = "#ffff00";
   }
   else
   {
      color[length] = 0;
      MET_SET_FREE(set);
   }

   /* Create a set that can be recalculated from the alist  */
   good = td_make_set_recalc(alist, STD_ARG_SET, sym->origin);
   if (MET_NULL_SET == good) {
      FREE(object, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   
   if (NULL == (object->info.object = VOCreate (widget__top->info.top.env,
						good, color)))
   {
      MET_SET_FREE(good);
      return MET_NULL_SYM;
   }
   MET_SET_FREE(good);
   
   td_link_in(&parent, name, WIDGET_TYPE_OBJECT, sym);
   if (parent == NULL)
   {
      VODestroy (object->info.object);
      FREE(object, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }
   
   object->xw.w = parent->xw.w;
   switch (parent->type) {
    case WIDGET_TYPE_VIEW:
      pict = ViewGetPict(parent->xw.w);
      PictAddObject(pict, object->info.object);
      PictDefaultScale(pict);
      EnvDrawAdd(widget__top->info.top.env, pict, ENV_PIXMAP_MODE);
      EnvFlush(widget__top->info.top.env);
      break;

    default:
      DEFAULT_ERR(parent->type);
   }

   ALLOCM(list, widget_list);
   list -> next = parent->kids;
   list -> w = object;
   parent->kids = list;
   return METsym_new_control((char *) object, sym->origin);
}

static void td_object_destroy(obj)
   fnord_widget		*obj;
{
   ME(td_object_destroy);

   PictRemoveObject(ViewGetPict(obj->xw.w), obj->info.object);
   VODestroy(obj->info.object);
}

#endif /* if !defined(X_TWO_D) else */
