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

/* $Id: defs.h,v 1.15 1993/06/03 14:16:23 rch Exp $*/

#ifndef TD_DEFS_H_INCLUDED
#define TD_DEFS_H_INCLUDED

#include "global/global.h"
#include "global/alloc.h"
#include "met/met.h"
#include "td/std_args.h"
#include "td/td.h"
#include "threedee/threedee.h"
#include "port/time.h"

#ifdef X_TWO_D
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#ifdef MOTIF_WIDGETS
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Frame.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/ArrowB.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>
#include <Xm/SelectioB.h>
#include <Xm/DialogS.h>
#include <Xm/ToggleBG.h>
#include <Xm/RowColumn.h>
#endif /* MOTIF_WIDGETS */

#include "widgets/View.h"
#include "widgets/Point.h"
#include "widgets/VirtualS.h"
#include "widgets/PixelEditor.h"
#endif /* X_TWO_D */

/************************************************************************/
/*									*/
/*				CONSTANTS				*/
/*									*/
/************************************************************************/

#define WIDGET_TYPE_NONE   (-1)
#define WIDGET_TYPE_TOP      0
#define WIDGET_TYPE_WINDOW   1
#define WIDGET_TYPE_VIEW     2
#define WIDGET_TYPE_OBJECT   3
#define WIDGET_TYPE_CAMERA   4
#define WIDGET_TYPE_EDITOR   5
#define WIDGET_TYPE_CHECKBOX 6
#define WIDGET_TYPE_BUTTON   7
#define WIDGET_TYPE_PRINTER  8
#define WIDGET_TYPE_POINT    9
#define WIDGET_TYPE_SLIDER   10
#define WIDGET_TYPE_TAPEDECK 11
#define WIDGET_TYPE_EVOLVE   12
#define WIDGET_TYPE_READ     13
#define WIDGET_TYPE_WRITE    14
#define WIDGET_TYPE_BITMAP   15
#define WIDGET_TYPE_TYPEIN   16
#define WIDGET_TYPE_OUTPUT   17
#define WIDGET_TYPE_VIDEO    18

#define DEFAULT_SLIDER_WINDOW_WIDTH  300
#define DEFAULT_SLIDER_WINDOW_HEIGHT 80

#define DEFAULT_CHECKBOX_WINDOW_WIDTH  100
#define DEFAULT_CHECKBOX_WINDOW_HEIGHT 40

#define DEFAULT_PRINTER_WINDOW_WIDTH  300
#define DEFAULT_PRINTER_WINDOW_HEIGHT 80

#define DEFAULT_TYPEIN_WINDOW_WIDTH  200
#define DEFAULT_TYPEIN_WINDOW_HEIGHT 50

#define DEFAULT_TAPEDECK_WINDOW_WIDTH  200
#define DEFAULT_TAPEDECK_WINDOW_HEIGHT 80

#define DEFAULT_EVOLVE_WINDOW_WIDTH  200
#define DEFAULT_EVOLVE_WINDOW_HEIGHT 40

#define DEFAULT_OUTPUT_WINDOW_WIDTH  300
#define DEFAULT_OUTPUT_WINDOW_HEIGHT 300

#define DEFAULT_BUTTON_WINDOW_WIDTH  100
#define DEFAULT_BUTTON_WINDOW_HEIGHT 40

#define DEFAULT_VIDEO_WINDOW_WIDTH  300
#define DEFAULT_VIDEO_WINDOW_HEIGHT 80

#define FUNC_NULL		((void (*)()) NULL)
#define FNW_NULL		((fnord_widget *) NULL)
#define WL_NULL			((widget_list *) NULL)

#define FNW_STATE_START		0x0000
#define FNW_STATE_FIXED		0x0001
#define FNW_STATE_DOOMED	0x0002
#define	FNW_STATE_RUNNING	0x0004
#define FNW_STATE_LAST_STEP     0x0008
#define FNW_STATE_TESTED	0x0010

#define	FNW_STATE_CHANGED_NOW	0x0020
#define FNW_STATE_CHANGED_LATER 0x0040
#define FNW_STATE_CHANGED	0x0060

#define FNW_STATE_UPDATE_NOW	0x0080
#define FNW_STATE_UPDATE_LATER  0x0100
#define FNW_STATE_UPDATE	0x0180

/************************************************************************/
/*									*/
/*				MACROS					*/
/*									*/
/************************************************************************/

#define	FNW_IS_FLAG_SET(FW, FLAG)	((FLAG) == ((FW)->state & (FLAG)))
#define FNW_SAFE_CALL(F,A)      { if (FUNC_NULL != F) (F) A; }

#define FNW_IS_FIXED(FW)	FNW_IS_FLAG_SET(FW, FNW_STATE_FIXED) 
#define FNW_IS_DOOMED(FW)	FNW_IS_FLAG_SET(FW, FNW_STATE_DOOMED) 
#define FNW_IS_RUNNING(FW)	FNW_IS_FLAG_SET(FW, FNW_STATE_RUNNING)
#define FNW_IS_LAST_STEP(FW)	FNW_IS_FLAG_SET(FW, FNW_STATE_LAST_STEP)
#define FNW_IS_TESTED(FW)	FNW_IS_FLAG_SET(FW, FNW_STATE_TESTED)
#define FNW_IS_CHANGED(FW)	(0 != ((FW)->state & FNW_STATE_CHANGED)) 
#define FNW_NEEDS_UPDATE(FW)    (0 != ((FW)->state & FNW_STATE_UPDATE))

#define FNW_SET_CHANGED(FW, FLAG)  \
   (FW)->state = ((FW)->state & ~FNW_STATE_CHANGED) | (FLAG)

#define FNW_SET_REQ_CHANGED(REQ, FW) \
   (REQ)->changed = (((FW)->state & FNW_STATE_CHANGED_NOW) ? \
		     MET_CLEAR_JUST_ONCE : 0) | \
   		    (((FW)->state & FNW_STATE_CHANGED_LATER) ? \
		     MET_CLEAR_LATER : 0)

#define FNW_SET_CHANGED_MET(FW, FLAG) \
   (FW)->state |= (((FLAG) & MET_CLEAR_JUST_ONCE) ? \
		   FNW_STATE_CHANGED_NOW : 0) |     \
		  (((FLAG) & MET_CLEAR_LATER) ?     \
                   FNW_STATE_CHANGED_LATER : 0)

#define FNW_SET_UPDATE_MET(FW, FLAG) \
   (FW)->state |= (((FLAG) & MET_CLEAR_JUST_ONCE) ? \
		   FNW_STATE_UPDATE_NOW : 0) |     \
		  (((FLAG) & MET_CLEAR_LATER) ?     \
                   FNW_STATE_UPDATE_LATER : 0)

#define FNW_CREATE(TYPE)	(fnw_types[TYPE]->create)
#define FNW_DESTROY(FW)		(fnw_types[(FW)->type]->destroy)(FW)
#define FNW_STEP(FW)		(fnw_types[(FW)->type]->step)(FW)
#define FNW_REQUEST(FW, REQ)	(fnw_types[(FW)->type]->request)((FW), (REQ))

#define FNW_CLEAR(FW)           \
   FNW_SAFE_CALL(fnw_types[(FW)->type]->clear, (FW))
#define FNW_UPDATE(FW)		\
   FNW_SAFE_CALL(fnw_types[(FW)->type]->update, (FW))
#define FNW_POST_UPDATE(FW)	\
   FNW_SAFE_CALL(fnw_types[(FW)->type]->post_update, (FW))
#define FNW_SECURE(FW)    \
   FNW_SAFE_CALL(fnw_types[(FW)->type]->secure, (FW))
   
#define TD_MSG			(void) fprintf

#define TD_ERR(LOC, ARG)	do { \
   FILOCloc_print((LOC), stdout, CNULL, 0); \
   TD_MSG(stdout, (ARG)); } while (0);

/************************************************************************/
/*									*/
/*				TYPES					*/
/*									*/
/************************************************************************/

typedef struct fnord_widget fnord_widget;

typedef struct widget_list {
   struct fnord_widget	*w;
   struct widget_list	*next;
} widget_list;


typedef struct fnw_type {
   int			type;
   int			name_arg;
   METsym		*(*create)();
   void			(*destroy)();
   void			(*secure)();
   void			(*step)();
   void			(*clear)();
   void			(*update)();
   void			(*post_update)();
   FOboolean		(*request)();
} fnw_type;

extern fnw_type		*fnw_types[];


#include "td/td_top.h"
#include "td/td_window.h"
#include "td/td_view.h"
#include "td/td_object.h"
#include "td/td_camera.h"
#include "td/td_editor.h"
#include "td/td_boolean.h"
#include "td/td_printer.h"
#include "td/td_point.h"
#include "td/td_slider.h"
#include "td/td_tapedeck.h"
#include "td/td_evolve.h"
#include "td/td_comm.h"
#include "td/td_bitmap.h"
#include "td/td_typein.h"
#include "td/td_output.h"
#include "td/td_video.h"

struct fnord_widget {
   int			type;
   int			state;
   char			name[80];
   widget_list		*kids;
   int			dft_kid_type;
#ifdef X_TWO_D
   struct {
      Widget 		w;
      Widget		top;
   } 			xw;
#endif
   union {
      fnw_top		top;
      fnw_window	window;
      fnw_view		view;
      fnw_object	object;
      fnw_camera 	camera;
      fnw_editor	editor;
      fnw_boolean 	boolean;
      fnw_printer	printer;
      fnw_point		point;
      fnw_slider	slider;
      fnw_tapedeck	tapedeck;
      fnw_evolve	evolve;
      fnw_read		read;
      fnw_write		write;
      fnw_bitmap	bitmap;
      fnw_typein	typein;
      fnw_output	output;
      fnw_video		video;
   } info;
};

/************************************************************************/
/*									*/
/*				FUNCTIONS				*/
/*									*/
/************************************************************************/

/* in td_shared.c */
extern FOboolean
   td_find_symbol(), td_lookup_int(), td_lookup_boolean(),
   td_lookup_real(), td_lookup_field(), td_lookup_type();

extern METset
   *td_make_set_recalc(), *td_make_type_recalc(), *td_make_set_cast();

extern METsym *td_cant_create();
extern FOboolean td_cant_request();

/* in td_x_shared */
#ifdef X_TWO_D
Boolean td_can_have_child();
void td_link_in();
void push_widget_under();
#endif

#ifdef MOTIF_WIDGETS
void td_winkid_geom();
void td_attach_in_window();
#endif

extern fnord_widget *widget__top;

#endif /* TD_DEFS_H_INCLUDED */
