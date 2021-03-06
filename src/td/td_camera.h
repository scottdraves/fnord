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

/* $Id: td_camera.h,v 1.2 1992/06/22 17:09:47 mas Exp $ */

#ifndef TD_CAMERA_H_INCLUDED
#define TD_CAMERA_H_INCLUDED

#ifndef X_TWO_D

typedef int fnw_camera;

#else

typedef struct fnw_camera {
   widget_list *affects;
} fnw_camera;

extern fnord_widget *td_new_vs();

#endif /* X_TWO_D */

extern fnw_type fnw_type_camera;

#endif /* TD_CAMERA_H_INCLUDED */
