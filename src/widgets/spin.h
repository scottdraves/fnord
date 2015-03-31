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

/* $Id: spin.h,v 1.3 1992/06/22 17:12:32 mas Exp $ */

#ifndef SPIN_H_INCLUDED
#define SPIN_H_INCLUDED

#include "global/global.h"
#include "global/float.h"

typedef Real Spin[4];

extern void spin_from_axis(/* spin, axis, angle */);
extern void spin_compose(/* result, first, second */);
extern void spin_to_matrix(/* matrix, spin */);
extern void spin_normalize(/* spin */);

#endif SPIN_H_INCLUDED
