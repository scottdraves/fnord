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

/* $Id: td_object.h,v 1.2 1992/06/22 17:10:11 mas Exp $*/

#ifndef TD_OBJECT_H_INCLUDED
#define TD_OBJECT_H_INCLUDED

#if	defined(X_TWO_D)
typedef ViewObj fnw_object;
#else
typedef int	fnw_object;
#endif

extern fnw_type fnw_type_object;

#endif 
