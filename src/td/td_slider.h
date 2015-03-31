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

/* $Id: td_slider.h,v 1.2 1992/06/22 17:10:35 mas Exp $*/

#ifndef TD_SLIDER_H_INCLUDED
#define TD_SLIDER_H_INCLUDED

#ifndef MOTIF_WIDGETS

typedef int fnw_slider;

#else

typedef struct fnw_slider {
   FOboolean	drag_update;
   int 		field;
   Real 	min;
   Real 	max;
   Real 	val;
} fnw_slider;   

#endif /* MOTIF_WIDGETS */

extern fnw_type fnw_type_slider;

#endif /* TD_SLIDER_H_INCLUDED */
