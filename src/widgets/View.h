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


/* $Id: View.h,v 1.5 1992/06/22 17:12:18 mas Exp $ */

#ifndef _View_h
#define _View_h

#include "threedee/threedee.h"

/***********************************************************************
 *
 * View Widget
 *
 ***********************************************************************/

/* Resource names used in the View widget */

#ifndef XtNdrawEnv
#define XtNdrawEnv "drawEnv"
#endif
#ifndef XtCDrawEnv
#define XtCDrawEnv "DrawEnv"
#endif
#ifndef XtNpadding
#define XtNpadding "padding"
#endif
#ifndef XtCPadding 
#define XtCPadding "Padding"
#endif

typedef struct _ViewRec *ViewWidget;
		/* completely defined in ViewP.h */
typedef struct _ViewClassRec *ViewWidgetClass;
		/* completely defined in ViewP.h */

extern WidgetClass viewWidgetClass;

extern int  ViewClear();
extern void ViewUpdate();
extern Pict ViewGetPict();

#endif /* _View_h */
/* DON'T ADD STUFF AFTER THIS #endif */
