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


/* $Id: xpictsortp.h,v 1.3 1992/06/22 17:11:44 mas Exp $ */

#define EPSILON (1e-7)

#define MUST_SPLIT 0x8
#define THIS_SIDE 0x4
#define THAT_SIDE 0x2
#define DONT_CARE 0x1

#define FIRST_BAD 1
#define SECOND_BAD 2
#define THIRD_BAD 3
#define SAFE 0

#define FIRST_MODE 0
#define RANDOM_MODE 1
#define PLANE_MODE 2

#define BSP_SWITCH_PT 50

#define ORIGINAL_POLY 1
#define DUPLICATE_POLY 0
#define LINE_OR_POINT 0

#define BACK_BACK_BACK 0x222
#define FRONT_FRONT_FRONT 0x444
#define COPLANAR 0x111

#define BSP_PLANE_DIST(norm, dist, pt)\
   ((norm)[0]*(pt)[0] + (norm)[1]*(pt)[1] + (norm)[2]*(pt)[2] - (dist))

#define BSP_SUBTRACT_VECTORS(first, second, dest)\
{ (dest)[0] = (first)[0] - (second)[0];\
  (dest)[1] = (first)[1] - (second)[1];\
  (dest)[2] = (first)[2] - (second)[2]; }

#define BSP_ZERO(v) ((EPSILON > ABS((v)[0])) &&\
		     (EPSILON > ABS((v)[1])) && (EPSILON > ABS((v)[2])))
