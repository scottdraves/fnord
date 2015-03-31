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

/* $Id: td_output.h,v 1.2 1992/06/22 17:10:16 mas Exp $*/

#ifndef TD_OUTPUT_H_INCLUDED
#define TD_OUTPUT_H_INCLUDED

#ifndef MOTIF_WIDGETS

typedef int fnw_output;

#else

typedef struct fnw_output {
   Widget file_label;
   Widget file_editor;
   Widget view_label;
   Widget view_editor;
   Widget button;
   Widget label;
   Widget radio;
   Widget tb1, tb2, tb3, tb4;
   int draw_style;
   METenv *env;
} fnw_output;

#endif /* MOTIF_WIDGETS */

extern fnw_type fnw_type_output;

#endif /* TD_OUTPUT_H_INCLUDED */
