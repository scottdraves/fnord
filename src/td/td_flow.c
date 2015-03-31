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
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>

#ifndef lint
static char Version[] =
   "$Id: td_flow.c,v 1.3 1993/06/03 14:16:35 rch Exp $";
#endif /* defined(lint) */

/* Global variables */

Sym std_args[STD_ARG_NUM];

fnw_type *fnw_types[] = {
   &fnw_type_top,
   &fnw_type_window,
   &fnw_type_view,
   &fnw_type_object,
   &fnw_type_camera,
   &fnw_type_editor,
   &fnw_type_checkbox,
   &fnw_type_button,
   &fnw_type_printer,
   &fnw_type_point,
   &fnw_type_slider,
   &fnw_type_tapedeck,
   &fnw_type_evolve,
   &fnw_type_read,
   &fnw_type_write,
   &fnw_type_bitmap,
   &fnw_type_typein,
   &fnw_type_output,
   &fnw_type_video,
};

fnord_widget *widget__top = FNW_NULL;

static void td_input_here();

/************************************************************************/
/*									*/
/*	RECURSIVE FUNCTIONS FOR ACTING ON THE WIDGET HIERARCHY		*/
/*									*/
/************************************************************************/

/*
 * td_call_step
 * Call the step function to jog the widget's state forward 
 * if the widget is automatically incrementing.
 */

static void td_call_step(fw)
   fnord_widget		*fw;
{
   widget_list		*list;
   ME(td_call_step);

   for (list = fw->kids;
	list != (widget_list *) NULL;
	list = list->next) 
      td_call_step(list->w);

   if (FNW_IS_RUNNING(fw))
      FNW_STEP(fw);
}

/*
 * td_call_clear
 * Call the widget's clear function (if it has one) to reset it
 * before we recalculate things.
 */

static void td_call_clear(fw)
   fnord_widget		*fw;
{
   widget_list		*list;
   ME(td_call_clear);

   for (list = fw->kids;
	list != (widget_list *) NULL;
	list = list->next)
      td_call_clear(list->w);

   FNW_CLEAR(fw);
}

/*
 * td_call_update
 * Call the widget's update function (if it has one) so that any
 * sets it looks at will be recalculated and appropriate changes
 * can be made to the display.
 */

static void td_call_update(fw)
   fnord_widget 	*fw;
{
   widget_list		*list;
   ME(td_call_update);

   for (list = fw->kids;
	list != (widget_list *) NULL;
	list = list->next)
      td_call_update(list->w);

   FNW_UPDATE(fw);
}

/*
 * td_call_post_update
 * Call the widget's post update function (if it has one) so that 
 * all that needs to happen to the widget after all drawing has
 * been done is taken care of.
 */

static void td_call_post_update(fw)
   fnord_widget 	*fw;
{
   widget_list		*list;
   FOboolean		update_again;
   ME(td_call_update);

   for (list = fw->kids;
	list != (widget_list *) NULL;
	list = list->next)
      td_call_post_update(list->w);

   FNW_POST_UPDATE(fw);

   update_again = (0 != (fw->state & FNW_STATE_UPDATE_LATER));
   fw->state &= ~FNW_STATE_UPDATE;
   if (update_again)
      fw->state |= FNW_STATE_UPDATE_NOW;
}

/*
 * td_call_destroy
 * Destroy all the kids, destroy the widget, and get rid of any
 * memory used by it.
 */

static void td_call_destroy(fw)
   fnord_widget 	*fw;
{
   widget_list		*list, *next;
   ME(td_call_destroy);

   list = fw->kids;
   while (list != (widget_list *) NULL) {
      next = list->next;
      td_call_destroy(list->w);
      FREE(list, sizeof(widget_list));
      list = next;
   }

   if (FNW_IS_RUNNING(fw))
      td_register_running(FALSE);

   FNW_DESTROY(fw);
   FREE(fw, sizeof(fnord_widget));
}

/*
 * td_and_state
 * Binary-and the state of the widget and all its children with the
 * passed flag.
 */

static void td_and_state(fw, flag)
   fnord_widget		*fw;
   int			flag;
{
   widget_list		*list;
   ME(td_and_state);

   for (list = fw->kids;
	list != (widget_list *) NULL;
	list = list->next)
      td_and_state(list->w, flag);

   fw->state &= flag;
}

/* 
 * td_or_state
 * Binary-or the state of the widget and all its children with the 
 * passed flag.
 */

static void td_or_state(fw, flag)
   fnord_widget		*fw;
   int			flag;
{
   widget_list		*list;
   ME(td_or_state);

   for (list = fw->kids;
	list != (widget_list *) NULL;
	list = list->next)
      td_or_state(list->w, flag);

   fw->state |= flag;
}

/* 
 * Make sure that all of the widgets in the passed tree are
 * either about to die, or have been told that some widgets
 * that they have pointers to are about to die.
 */

static void td_call_secure(fw)
   fnord_widget		*fw;
{
   widget_list		*list;
   ME(td_call_secure);

   if (FNW_IS_DOOMED(fw))
      return;

   FNW_SECURE(fw);

   for (list = fw->kids;
	list != (widget_list *) NULL;
	list = list->next)
      td_call_secure(list->w);
}

/*
 * Mark any widgets that are not fixed as about to die.
 */

static void td_doom_new(fw)
   fnord_widget		*fw;
{
   widget_list		*list;
   ME(td_or_state);

   if (!FNW_IS_FIXED(fw)) 
      td_or_state(fw, FNW_STATE_DOOMED);
   else
      for (list = fw->kids;
	   list != (widget_list *) NULL;
	   list = list->next)
	 td_doom_new(list->w);
}

/*
 * Just what you think.
 */

static void td_destroy_doomed(fw)
   fnord_widget		*fw;
{
   widget_list		*curr, *prev, *temp;
   FOboolean		destroyed = NO;
   ME(td_destroy_new);

   prev = curr = fw->kids;
   
   while(curr != NULL)	 {

      if (FNW_IS_DOOMED(curr->w)) {
	 destroyed = YES;
	 td_call_destroy(curr->w);
	 temp = curr;
	 if (curr == fw->kids)
	    curr = prev = fw->kids = curr->next;
	 else
	    curr = prev->next = curr->next;
	 
	 FREE( temp, sizeof(widget_list));
      } else {
	 if (curr->w->kids != NULL)
	    td_destroy_doomed(curr->w);
	 prev = curr;
	 curr = curr->next;
      }
   }
   
   if (YES == destroyed) 
      FNW_UPDATE(fw);
}

/************************************************************************/
/*									*/
/*	GLOBAL FUNCTIONS FOR PLAYING WITH THE TWO-D INTERFACE		*/
/*									*/
/************************************************************************/

/*
 * td_update
 * Cause all the automatically running widgets to be incremented 
 * by one step, redisplay all of the output widgets, and note 
 * that current changes have been incorporated into two-d state.
 */

void td_update()
{
   ME(td_update);

   td_call_step(widget__top);
   td_call_clear(widget__top);
   td_call_update(widget__top);

#ifdef X_TWO_D
   EnvFlush(widget__top->info.top.env);
#endif /* X_TWO_D  ACK! */

   td_call_post_update(widget__top);
   td_and_state(widget__top, ~(FNW_STATE_CHANGED | FNW_STATE_TESTED));
}

/*
 * td_control_callback
 * Respond to the request from the math engine for information about
 * the state and value of a widget.
 */

FOboolean td_control_callback(req)
   METcontrol_request	*req;
{
   fnord_widget		*control = CAST(fnord_widget *, req->data);
   ME(td_control_callback);

   return FNW_REQUEST(control, req);
}

/*
 * td_destroy
 * gets rid of all of the widgets
 */

void td_destroy()
{
   ME(td_destroy);

   if (FNW_NULL == widget__top)
      return;

   td_call_destroy(widget__top);
   widget__top = FNW_NULL;
}
      
/*
 * destroy_new_widgets()
 * gets rid of all widgets not marked as fixed.
 */

void destroy_new_widgets()
{
   ME(destroy_new_widgets);

   if (FNW_NULL == widget__top)
      return;

   td_doom_new(widget__top);
   td_call_secure(widget__top);
   td_destroy_doomed(widget__top);
}

/*
 * fix_widgets
 * marks all the descendants of the top widget that currently
 * exist as established permanently
 */

void fix_widgets()
{
   ME(fix_widgets);

   if (FNW_NULL == widget__top)
      return;

   td_or_state(widget__top, FNW_STATE_FIXED);
}

/*
 * widget--called by the parser to generate a widget.
 */

METsym *widget(name, alist, env, sym)
   char		*name;
   METset	*alist;
   METenv	*env;
   METsym	*sym;
{
   METset *assoc;
   Sym type_sym;
   Sym parent_sym;
   METsym *parent_def;
   fnord_widget *parent;
   METsym *new_widget;
   char new_name[80];
   int length = 79;
   int i;
   ME(widget);

   /* Discover the type specified in the alist */

   assoc = METset_assoc(alist, std_args[STD_ARG_TYPE]);
   if (assoc == MET_NULL_SET) {
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout, ": widget defintion without type value.\n");
      return MET_NULL_SYM;
   }
   if (!td_find_symbol(assoc, &type_sym)) {
      MET_SET_FREE(assoc);
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout,": type must be associated with symbol.\n");
      return MET_NULL_SYM;
   }
   MET_SET_FREE(assoc);

   /* Discover the widget specified as the parent of the 
      new one.  This really is a process of discovery, since the
      function must first bypass the quoting to get to the set,
      and then read in the control data from the symbol which
      defines the set.  Yuk! */

   assoc = METset_assoc(alist, std_args[STD_ARG_PARENT]);
   if (MET_NULL_SET == assoc) {
      parent = widget__top;
   } else if (!td_find_symbol(assoc, &parent_sym)) {
      MET_SET_FREE(assoc);
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout,": Parent must be associated with a symbol\n");
      TD_MSG(stdout, "\t\t\tthat names a widget.\n");
      parent = widget__top;
   } else {
      MET_SET_FREE(assoc);
      assoc = METenv_lookup(env, parent_sym);
      if (assoc == MET_NULL_SET) {
	 FILOCloc_print(sym->origin, stdout, CNULL, 0);
	 TD_MSG(stdout, "Undefined variable %s\n", SYM_STRING(parent_sym));
	 parent = widget__top;
      } else {
	 parent_def = assoc->definition;
	 if (MET_SYM_TYPE_CONTROL != parent_def->type) {
	    FILOCloc_print(sym->origin, stdout, CNULL, 0);
	    TD_MSG(stdout, ": Parent is not a widget.\n");
	    parent = widget__top;
	 } else {
	    parent = CAST(fnord_widget *, parent_def->info.control.data);
	 }
	 MET_SET_FREE(assoc);
      }
   }

   /* See if the user wanted a more elaborate name to appear on the
      screen with his widget than the one specified as the symbol
      to bind it to.  After all, you might want to have "a" as
      your symbol but "Radius of first ellipse" as your title. */

   assoc = METset_assoc(alist, std_args[STD_ARG_NAME]);
   if (MET_NULL_SET == assoc) {
      (void) strcpy(new_name, name);
   } else if (! METset_get_string(assoc, new_name, &length)) {
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout, ": name of widget must be short string.\n");
      MET_SET_FREE(assoc);
      (void) strcpy(new_name,name);
   } else {
      new_name[length] = 0;
      MET_SET_FREE(assoc);
   }

   new_widget = MET_NULL_SYM;

   for (i = 0; i < sizeof(fnw_types) / sizeof(fnw_types[0]); i++)
      if (SYM_EQ(std_args[fnw_types[i]->name_arg], type_sym)) {
	 new_widget = (FNW_CREATE(i))(parent, new_name, alist, sym);
	 break;
      }

   if (i == sizeof(fnw_types) / sizeof(fnw_types[0])) {
      FILOCloc_print(sym->origin, stdout, CNULL, 0);
      TD_MSG(stdout, ": widget type unknown.\n");
   }

   td_update();

   return new_widget;
}

/*
 * I've eliminated all of the timers---automatic updating is done
 * whenever td_update is called.  Our philosophy on looking for 
 * events is this: if there's something to do, it will either be
 * short and sweet (taking, we approximate, 0 ms) or it will call
 * td_update.  Either way, the automatic guys won't have to wait.
 * If there's nothing waiting, we update automatic guys, since there's
 * nothing better to do anyway.  If we can't even do that, we select
 * and let UNIX run some more deserving program.
 */

#ifdef X_TWO_D

void
td_work()
{
   ME(td_work);

   if (0 != XtAppPending(widget__top->info.top.app) ||
       0 == widget__top->info.top.running)
      XtAppProcessEvent(widget__top->info.top.app, XtIMAll);
   else 
      td_update();
}

#else

void
td_work()
{
   fd_set	rfds;
   fd_set	wfds;
   struct timeval time;
   fnw_call	*call;
   int		i;
   ME(td_work);

   FD_ZERO(&rfds);
   FD_ZERO(&wfds);
   for (i = 0, call = widget__top->info.top.calls;
	i < widget__top->info.top.nfds; 
	i++, call++) 
      if (YES == call->alive) {
	 if (FUNC_NULL != call->readfunc)
	    FD_SET(call->fd, &rfds);
	 if (FUNC_NULL != call->writefunc)
	    FD_SET(call->fd, &wfds);
      }
   
   if (widget__top->info.top.text_input != -1)
      FD_SET(widget__top->info.top.text_input, &rfds);

   if (widget__top->info.top.running != 0) {
      time.tv_sec = 0;
      time.tv_usec = 0;
      i = select(256, &rfds, &wfds, (fd_set *) NULL, &time);
   } else {
      i = select(256, &rfds, &wfds, (fd_set *) NULL, 
		 (struct timeval *) NULL);
   }

   if (0 == i)
      td_update();

   if (1 > i)
      return;

   for (i = 0, call = widget__top->info.top.calls;
	i < widget__top->info.top.nfds; 
	i++, call++) {
      if (FD_ISSET(call->fd, &rfds))
	 (call->readfunc)(call->closure);
      if (FD_ISSET(call->fd, &wfds))
	 (call->writefunc)(call->closure);
   }
   
   /* In case one of these operations
      removes the socket being worked with... */

   td_wfd_clean();

   if (widget__top->info.top.text_input != -1 &&
       FD_ISSET(widget__top->info.top.text_input, &rfds))
      td_input_here((caddr_t) widget__top);
}

#endif /* X_TWO_D */

/************************************************************************/
/*									*/
/* GLOBAL FUNCTIONS FOR DEALING WITH FILES THROUGH THE TWO-D INTERFACE	*/
/*									*/
/************************************************************************/

/***********************************************************************
 *
 * register that there is input ready on some fd---header in X version
 * reflects dummy parameters X sends us....
 *
 ***********************************************************************/

#ifndef X_TWO_D
static void td_input_here(client_data)
   caddr_t client_data;
#else
/* ARGSUSED */
static void
td_input_here(client_data, source, id)
   caddr_t client_data;
   int *source;
   XtInputId *id;
#endif
{
   fnord_widget *top = CAST(fnord_widget *, client_data);
   int flags;
   int snarfed;
   int length = strlen( top->info.top.text );
   extern int errno; /* Ack! why don't we have this? */
   ME(xtd_input_here);

   flags = fcntl(top->info.top.text_input, F_GETFL, 0);
   fcntl(top->info.top.text_input, F_SETFL, flags | O_NDELAY);
   snarfed = read(top->info.top.text_input,
		  top->info.top.text + length, 
		  top->info.top.desired - length );
   
   /* check the return value from read() */
   if (snarfed < 0)	/* some error */
   {
      if (errno != EWOULDBLOCK) /* nothing available yet */
	 perror("Error reading file");
      
      fcntl(top->info.top.text_input, F_SETFL, flags);
      top->info.top.ready = YES;
   }
   else /* got something, either data or EOF.  return it */
   {
      *(top->info.top.text + length + snarfed) = 0;
      fcntl(top->info.top.text_input, F_SETFL, flags);
      top->info.top.ready = YES;
   }
}

void
td_fd_add(fd, len)
   int fd;
   int len;
{
   int t;
   ME(td_fd_add);

#ifdef X_TWO_D /* Forget what we told X about the last file.... */
   if (widget__top->info.top.text_input != fd)
   {
      t = widget__top->info.top.text_input;
      if (t != -1) {
	 XtRemoveInput( widget__top->info.top.x_input);
      }

      widget__top->info.top.x_input =
	 XtAppAddInput(widget__top->info.top.app, fd, XtInputReadMask,
		       (XtInputCallbackProc) td_input_here,
		       (caddr_t) widget__top);

   }
#endif

   widget__top->info.top.text_input = fd;

   if (widget__top->info.top.length == 0)
   {
      ALLOCNM( widget__top->info.top.text, char, len + 1);
      widget__top->info.top.text[0] = '\0';
      widget__top->info.top.length = len + 1;
   }
   else if (widget__top->info.top.length <= len)
   {
      REALLOCNM( widget__top->info.top.text, char, len + 1,
		widget__top->info.top.length );
      widget__top->info.top.length = len + 1;
   }
   widget__top->info.top.desired = len;
}

/**********************************************************************
 *
 *	Remove a file descriptor from the input list
 *
 **********************************************************************/

void
td_fd_remove(fd)
   int fd;
{
   ME(td_fd_remove);

#ifdef X_TWO_D
   XtRemoveInput(widget__top->info.top.x_input);
   widget__top->info.top.x_input = (XtInputId) 0; /* non-portable */
#endif

   widget__top->info.top.text_input = -1;	
}

/**********************************************************************
 *
 *	Find a fd with input ready
 *
 **********************************************************************/

int
td_fd_check()

{
   ME(td_fd_check);

   if (widget__top->info.top.ready)
      return YES;
   else
      return NO;
}

/*
 * get the string that the top level widget is "holding" for the
 * scanner.  
 */

int td_fd_get_string(buf, count)
   char *buf;
   int count;
{
   int length;
   ME(td_fd_get_string);

   length = strlen( widget__top->info.top.text );
   if (count >= length)
   {
      strncpy(buf, widget__top->info.top.text, length);
      *widget__top->info.top.text = 0;
      widget__top->info.top.ready = FALSE;
      return length;
   }
   strncpy( buf, widget__top->info.top.text, count );
   bcopy( widget__top->info.top.text + count, 
	  widget__top->info.top.text, length - count + 1 );
   return count;
}
