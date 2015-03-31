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

#ifdef X_TWO_D

#include "td/defs.h"

#ifndef lint
static char Version[] = 
   "$Id: td_x_shared.c,v 1.3 1993/06/03 14:16:50 rch Exp $";
#endif

/*
 *  Returns true if the passed fnord_widget will directly
 *  accept a child of the passed type.  This version of this
 *  this procedure allows for thcomplete version of the
 *  widget hierarchy to be supported, since this is the
 *  file for use when Motif is included.
 */

Boolean td_can_have_child(parent, type)
   fnord_widget *parent;
   int type;

{
   ME(td_can_have_child);
   
#ifdef MOTIF_WIDGETS   
   switch(parent->type)
   {
    case WIDGET_TYPE_TOP:
      if ((WIDGET_TYPE_WINDOW == type) ||
	  (WIDGET_TYPE_EDITOR == type))
	 return YES;
      else
	 return NO;
      
    case WIDGET_TYPE_WINDOW:
      if ((WIDGET_TYPE_OBJECT == type) ||
	  (WIDGET_TYPE_TOP == type))
	 return NO;
      else
	 return YES;
      
    case WIDGET_TYPE_VIEW:
      if (WIDGET_TYPE_OBJECT == type ||
	  WIDGET_TYPE_POINT == type)
	 return YES;
      else
	 return NO;

    case WIDGET_TYPE_PRINTER:
      if (WIDGET_TYPE_OBJECT == type)
	 return YES;
      else
	 return NO;
      
      
    case WIDGET_TYPE_SLIDER:
    case WIDGET_TYPE_TYPEIN:
    case WIDGET_TYPE_CHECKBOX:
    case WIDGET_TYPE_BUTTON:
    case WIDGET_TYPE_EVOLVE:
    case WIDGET_TYPE_TAPEDECK:
    case WIDGET_TYPE_OBJECT:
    case WIDGET_TYPE_EDITOR:
    case WIDGET_TYPE_CAMERA:
    case WIDGET_TYPE_OUTPUT:
    case WIDGET_TYPE_VIDEO:
    case WIDGET_TYPE_POINT:
    case WIDGET_TYPE_READ:
    case WIDGET_TYPE_WRITE:
      return NO;
      
    default:
      DEFAULT_ERR(parent->type);
   }
#else
   switch(parent->type)
   {
    case WIDGET_TYPE_TOP:
      if ((WIDGET_TYPE_VIEW == type) ||
	  (WIDGET_TYPE_CAMERA == type))
	 return YES;
      else
	 return NO;
      
    case WIDGET_TYPE_VIEW:
      if (WIDGET_TYPE_OBJECT == type)
	 return YES;
      else
	 return NO;
      
    case WIDGET_TYPE_OBJECT:
    case WIDGET_TYPE_READ:
    case WIDGET_TYPE_WRITE:
      return NO;
      
    case WIDGET_TYPE_SLIDER:
    case WIDGET_TYPE_PRINTER:
    case WIDGET_TYPE_TYPEIN:
    case WIDGET_TYPE_CHECKBOX:
    case WIDGET_TYPE_BUTTON:
    case WIDGET_TYPE_TAPEDECK:
    case WIDGET_TYPE_WINDOW:
    case WIDGET_TYPE_EDITOR:
    case WIDGET_TYPE_EVOLVE:
    case WIDGET_TYPE_OUTPUT:
    case WIDGET_TYPE_VIDEO:
    case WIDGET_TYPE_POINT:
    default:
      DEFAULT_ERR(parent->type);
   }
#endif
   return NO;
}

/*
 *  Positions the passed child widget at the appropriate
 *  location in the passed parent "window" widget, resizing
 *  the parent as necessary.  Located in this file because,
 *  as it stands, it's only good for positioning widgets
 *  inside Motif form widgets.
 */
#ifdef MOTIF_WIDGETS
void td_attach_in_window(parent, child, t, l, b, r)
   fnord_widget *parent;
   fnord_widget *child;
   int t;
   int l;
   int b;
   int r;

{
   int i;
   int new_divs;
   int new_dim;
   Arg arg[8];
   ME(td_attach_in_window);

   new_divs = (r > b) ? r : b;
   if ( new_divs > parent->info.window.subdivs ) {
      XtSetArg(arg[0], XmNfractionBase, new_divs); 
      i = 1;
      if ( parent->info.window.subwin == NO ) {
	 if (new_divs == r) {
	    new_dim = parent->info.window.width =
	       new_divs * parent->info.window.width / 
		  parent->info.window.subdivs;
	    XtSetArg(arg[i], XmNwidth, new_dim);
	    i++;
	 }
	 if (new_divs == b) {
	    new_dim = parent->info.window.height =
	       new_divs * parent->info.window.height /
		  parent->info.window.subdivs;
	    XtSetArg(arg[i], XmNheight, new_dim);
	    i++;
	 }
	 if (XtIsRealized(parent->xw.top))
	    XtSetValues(parent->xw.top, &arg[1], i-1);
      }
      XtSetValues(parent->xw.w, arg, i);
      parent->info.window.subdivs = new_divs;
   }

   i = 0;
   if (t < 0) {
      XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM);
      i++;
   } else {
      XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION);
      XtSetArg(arg[i+1], XmNtopPosition, t);
      i += 2;
   }
   if (b < 0) {
      XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM);
      i++;
   } else {
      XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION);
      XtSetArg(arg[i+1], XmNbottomPosition, b);
      i += 2;
   }
   if (r < 0) {
      XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM);
      i++;
   } else {
      XtSetArg(arg[i], XmNrightAttachment, XmATTACH_POSITION);
      XtSetArg(arg[i+1], XmNrightPosition, r);
      i += 2;
   }
   if (l < 0) {
      XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM);
      i++;
   } else {
      XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION);
      XtSetArg(arg[i+1], XmNleftPosition, l);
      i += 2;
   }

   if (child->xw.top != None)
      XtSetValues(child->xw.top, arg, i);
   else
      XtSetValues(child->xw.w, arg, i);
}
#endif defined(MOTIF_WIDGETS)

/*
 *	Looks up the geometry information appropriate to a child of 
 *	a "window" widget and puts the child at the location specified
 *	(or the default location, or any combination of the two).
 *	Parses top, left, bottom, right, width, and height.  Top and
 *	left can be set to "start", bottom and right to "end", and
 *	width and height to "all".
 */
#ifdef MOTIF_WIDGETS
void td_winkid_geom(parent, child, alist, sym)
   fnord_widget *parent;
   fnord_widget *child;
   METset 	*alist;
   METsym	*sym;
{

   int 		t, l, b, r;
   int 		tset;
   char		*message = ": Inappropriate value for parameter.\n";

   ME(td_winkid_geom);

   if (child->type == WIDGET_TYPE_POINT &&
       child->info.point.is_in_view) {
      t = child->info.point.view->info.view.top;
      l = child->info.point.view->info.view.left;
      b = child->info.point.view->info.view.bottom;
      r = child->info.point.view->info.view.right;
      td_attach_in_window(parent, child, t, l, b, r);
      return;
   }

   tset = td_lookup_int(alist, std_args[STD_ARG_TOP], &t,
			(parent->kids == NULL) ? 0 :
			parent->info.window.subdivs,
			std_args[STD_ARG_START], -1);
   if (t < -1) {
      TD_ERR(sym->origin, message);
      t = parent->info.window.subdivs;
      tset = FALSE;
   }

   (void) td_lookup_int(alist, std_args[STD_ARG_LEFT], &l,
			-1, std_args[STD_ARG_START], -1);

   if (l < -1) {
      TD_ERR(sym->origin, message);
      l = -1;
   }

   if (NO == td_lookup_int(alist, std_args[STD_ARG_BOTTOM], &b,
			   (WIDGET_TYPE_SLIDER == child->type) ? t+1 : t+3,
			   std_args[STD_ARG_END], -1)  &&
       child->type != WIDGET_TYPE_BITMAP) {
      (void) td_lookup_int(alist, std_args[STD_ARG_HEIGHT], &b,
			   (WIDGET_TYPE_SLIDER == child->type) ? 1 : 3,
			   std_args[STD_ARG_ALL], -1);
      if (-1 == b)
	 t = (tset) ? t : -1;
      else if (b > -1)
	 b += t;
   }

   if (b < t && b != -1) {
      TD_ERR(sym->origin, message);
      b = (WIDGET_TYPE_SLIDER == child->type) ? t+1: t+3;
   }

   if (! td_lookup_int(alist, std_args[STD_ARG_RIGHT], &r,
			-1, std_args[STD_ARG_END], -1) &&
       child->type != WIDGET_TYPE_BITMAP) {
      (void) td_lookup_int(alist, std_args[STD_ARG_WIDTH], &r,
			    -1, std_args[STD_ARG_END], -1);
      if (-1 < r)
	 r += l;
   }

   if (r < l && r != -1) {
      TD_ERR(sym->origin, message);
      r = -1;
   }

   td_attach_in_window(parent, child, t, l, b, r);

   if (WIDGET_TYPE_VIEW == child->type) {
      child->info.view.top = t;
      child->info.view.left = l;
      child->info.view.bottom = b;
      child->info.view.right = r;
   }
}
#endif defined(MOTIF_WIDGETS)

/*
 *	This function creates the appropriate chain of widgets between
 *	a parent and child.  This allows new things to be put onto
 *	the screen automatically, without the user having to worry
 *	about the connecting layers.  Parent is returned as the "real"
 *	parent widget to use as child's parent.  If the desired 
 *	connection pattern is impossible, parent is returned as NULL.
 */

void td_link_in(parent, name, child_type, sym)
   fnord_widget **parent;
   char 	*name;
   int 		child_type;
   METsym 	*sym;
{
   widget_list 	*curr;
   fnord_widget *temp;
   METsym	*new;
   ME(td_link_in);

#ifdef MOTIF_WIDGETS
   while (!td_can_have_child(*parent, child_type))
   {
      switch(child_type)
      {

       case WIDGET_TYPE_TOP:
	 TD_ERR(sym->origin, "The top widget can have no parent.\n");
	 *parent = NULL;
	 return;

       case WIDGET_TYPE_WINDOW:
       case WIDGET_TYPE_EDITOR:
	 FILOCloc_print(sym->origin, stdout, CNULL, 0);
	 TD_MSG(stdout,": %s cannot be the children of %s",
	       (child_type == WIDGET_TYPE_WINDOW) ? "Windows" : "Editors",
	       ((*parent)->type == WIDGET_TYPE_SLIDER) ? "sliders" :
	       ((*parent)->type == WIDGET_TYPE_VIEW) ? "views" :
	       "objects");
	 *parent = NULL;
	 return;

       case WIDGET_TYPE_SLIDER:
       case WIDGET_TYPE_CHECKBOX:
       case WIDGET_TYPE_BUTTON:
       case WIDGET_TYPE_TAPEDECK:
       case WIDGET_TYPE_TYPEIN:
       case WIDGET_TYPE_BITMAP:
       case WIDGET_TYPE_VIEW:
       case WIDGET_TYPE_PRINTER:
       case WIDGET_TYPE_EVOLVE:
       case WIDGET_TYPE_OUTPUT:
       case WIDGET_TYPE_VIDEO:
       case WIDGET_TYPE_POINT:
	 if ((*parent)->type != WIDGET_TYPE_TOP)
	 {
	    FILOCloc_print(sym->origin, stdout, CNULL, 0);
	    TD_MSG(stdout, ": that cannot be the child of a %s",
		  ((*parent)->type == WIDGET_TYPE_SLIDER) ? "slider" :
		  ((*parent)->type == WIDGET_TYPE_VIEW) ? "view" :
		  "objects");
	    *parent = NULL;
	    return;
	 }
	 for (curr = (*parent)->kids; curr != NULL ; curr = curr->next )
	    if ( child_type == curr->w->dft_kid_type )
	       break;
	 
	 if (curr == NULL) {
	    if (WIDGET_TYPE_SLIDER == child_type)
	       temp = td_new_window(*parent, "sliders", MET_NULL_SET, 
				    sym, WIDGET_TYPE_SLIDER);
	    else if (WIDGET_TYPE_CHECKBOX == child_type)
	       temp = td_new_window(*parent, "checkboxes", MET_NULL_SET, 
				    sym, WIDGET_TYPE_CHECKBOX);
	    else if (WIDGET_TYPE_PRINTER == child_type)
	       temp = td_new_window(*parent, "printers", MET_NULL_SET, 
				    sym, WIDGET_TYPE_PRINTER);
	    else if (WIDGET_TYPE_TYPEIN == child_type)
	       temp = td_new_window(*parent, "typeins", MET_NULL_SET, 
				    sym, WIDGET_TYPE_TYPEIN);
	    else if (WIDGET_TYPE_TAPEDECK == child_type)
	       temp = td_new_window(*parent, "tapedecks", MET_NULL_SET, 
				    sym, WIDGET_TYPE_TAPEDECK);
	    else if (WIDGET_TYPE_EVOLVE == child_type)
	       temp = td_new_window(*parent, "evolvers", MET_NULL_SET, 
				    sym, WIDGET_TYPE_EVOLVE);
	    else if (WIDGET_TYPE_BUTTON == child_type)
	       temp = td_new_window(*parent, "buttons", MET_NULL_SET, 
				    sym, WIDGET_TYPE_BUTTON);
	    else if (WIDGET_TYPE_OUTPUT == child_type)
	       temp = td_new_window(*parent, "output managers", MET_NULL_SET, 
				    sym, WIDGET_TYPE_OUTPUT);
	    else if (WIDGET_TYPE_VIDEO == child_type)
	       temp = td_new_window(*parent, "video recorders",
				    MET_NULL_SET,
				    sym, WIDGET_TYPE_VIDEO);
	    else
	       temp = td_new_window(*parent, name, MET_NULL_SET,
				    sym, WIDGET_TYPE_NONE);
	 } else
	    temp = curr->w;

	 *parent = temp;
	 break;
	 
       case WIDGET_TYPE_OBJECT:
	 switch((*parent)->type) {
	  case WIDGET_TYPE_TOP:
	    for (curr = (*parent)->kids; curr != NULL; curr = curr->next)
	       if (WIDGET_TYPE_VIEW == curr->w->dft_kid_type)
		  break;

	    if (curr == NULL) {
	       temp = td_new_window(*parent, name, MET_NULL_SET,
				    sym, WIDGET_TYPE_NONE);
	    } else
	       temp = curr->w;

	    *parent = temp;
	    break;
	    
	  case WIDGET_TYPE_WINDOW:
	    for (curr = (*parent)->kids; curr != NULL; curr = curr->next)
	       if (WIDGET_TYPE_VIEW == curr->w->dft_kid_type)
		  break;

	    if (curr == NULL) {
	       new = (FNW_CREATE(WIDGET_TYPE_VIEW))
		  (*parent, name, MET_NULL_SET, sym);
	       temp = CAST(fnord_widget *, new->info.control.data);
	       MET_SYM_FREE(new);
	    } else
	       temp = curr->w;

	    *parent = temp;
	    break;

	  case WIDGET_TYPE_OBJECT:
	  case WIDGET_TYPE_SLIDER:
	  case WIDGET_TYPE_CHECKBOX:
	  case WIDGET_TYPE_BUTTON:
	  case WIDGET_TYPE_TAPEDECK:
	  case WIDGET_TYPE_TYPEIN:
	  case WIDGET_TYPE_EDITOR:
	  case WIDGET_TYPE_EVOLVE:
	  case WIDGET_TYPE_OUTPUT:
	  case WIDGET_TYPE_VIDEO:
	    FILOCloc_print(sym->origin, stdout, CNULL, 0);
	    TD_MSG(stdout,": Objects can not be the children of %s.\n",
		  (WIDGET_TYPE_OBJECT == (*parent)->type) ?
		  "other objects" : 
		  (WIDGET_TYPE_EDITOR == (*parent)->type) ?
		  "editors" : "sliders");
	    *parent = NULL;
	    return;

	  default:
	    DEFAULT_ERR((*parent)->type);
	 }
	 break;

       default:
	 DEFAULT_ERR(child_type);
      }
   }
#else

   while (!td_can_have_child(*parent, child_type)) {
      switch(child_type) {

       case WIDGET_TYPE_TOP:
	 TD_ERR(sym->origin, ": The top widget can have no parent.\n");
	 *parent = NULL;
	 return;

       case WIDGET_TYPE_WINDOW:
       case WIDGET_TYPE_CHECKBOX:
       case WIDGET_TYPE_BUTTON:
       case WIDGET_TYPE_TAPEDECK:
       case WIDGET_TYPE_TYPEIN:
       case WIDGET_TYPE_EDITOR:
       case WIDGET_TYPE_SLIDER:
       case WIDGET_TYPE_EVOLVE:
       case WIDGET_TYPE_OUTPUT:
       case WIDGET_TYPE_VIDEO:
	 TD_ERR(sym->origin, ": These widgets need Motif.\n");
	 *parent = NULL;
	 return;

       case WIDGET_TYPE_VIEW:
	 TD_ERR(sym->origin, ": Parent of view has wrong type.\n");
	 *parent = NULL;
	 return;
	 
       case WIDGET_TYPE_OBJECT:
	 if ((*parent)->type != WIDGET_TYPE_TOP) {
	    TD_ERR(sym->origin, ": Parent of object has wrong type.\n");
	    *parent = NULL;
	    return;
	 }
	 new = (FNW_CREATE(WIDGET_TYPE_VIEW))
	    (*parent, name, MET_NULL_SET, sym);
	 *parent = CAST(fnord_widget *, new->info.control.data);
	 MET_SYM_FREE(new);
	 break;

       default:
	 DEFAULT_ERR((*parent)->type);
      }
   }
#endif defined(MOTIF_WIDGETS)
}

void 
push_widget_under(ends_up_on_top, w)
   Widget ends_up_on_top, w;
{
   XtWidgetGeometry request;
   ME(push_widget_under);

   request.request_mode = CWStackMode;
   request.stack_mode = BottomIf;
   request.sibling = ends_up_on_top;
   if (XtGeometryNo == XtMakeGeometryRequest(w, &request, NULL)) {
      fprintf(stderr,"can't stack widgets properly, input might fail.\n");
   }
}

#endif /* defined(X_TWO_D) */
