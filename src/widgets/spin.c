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

#ifndef lint
static char Version[]=
   "$Id: spin.c,v 1.4 1992/06/22 17:12:30 mas Exp $";
#endif

#include "widgets/spin.h"

void
spin_from_axis(spin, axis, angle)
   Spin spin;
   Real axis[3];
   Real angle;
{
   Real len, s;

   len = SQRT(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
   if (len >= 1e-9) {
      s = SIN(angle / 2.0) / len;
      spin[0] = s * axis[0];
      spin[1] = s * axis[1];
      spin[2] = s * axis[2];
      spin[3] = COS(angle / 2.0);
   } else {
      spin[0] = 0.0;
      spin[1] = 0.0;
      spin[2] = 0.0;
      spin[3] = 1.0;
   }
}

void
spin_compose(result, first, second)
   Spin result;
   Spin first;
   Spin second;
{
   Real t1[3], t2[3], t3[3];
   Spin tf;

   t1[0] = second[3] * first[0];
   t1[1] = second[3] * first[1];
   t1[2] = second[3] * first[2];
   t2[0] = first[3] * second[0];
   t2[1] = first[3] * second[1];
   t2[2] = first[3] * second[2];
   t3[0] = second[1] * first[2] - second[2] * first[1];
   t3[1] = second[2] * first[0] - second[0] * first[2];
   t3[2] = second[0] * first[1] - second[1] * first[0];
   tf[0] = t1[0] + t2[0] + t3[0];
   tf[1] = t1[1] + t2[1] + t3[1];
   tf[2] = t1[2] + t2[2] + t3[2];
   tf[3] = first[3] * second[3] -
          (first[0] * second[0] +
	   first[1] * second[1] +
	   first[2] * second[2]);
   result[0] = tf[0];
   result[1] = tf[1];
   result[2] = tf[2];
   result[3] = tf[3];
}

void
spin_to_matrix(matrix, spin)
   Real matrix[3][3];
   Spin spin;
{
   matrix[0][0] = 1.0 - 2.0 * (spin[1] * spin[1] + spin[2] * spin[2]);
   matrix[0][1] = 2.0       * (spin[0] * spin[1] - spin[2] * spin[3]);
   matrix[0][2] = 2.0       * (spin[2] * spin[0] + spin[1] * spin[3]);
   
   matrix[1][0] = 2.0       * (spin[0] * spin[1] + spin[2] * spin[3]);
   matrix[1][1] = 1.0 - 2.0 * (spin[2] * spin[2] + spin[0] * spin[0]);
   matrix[1][2] = 2.0       * (spin[1] * spin[2] - spin[0] * spin[3]);
   
   matrix[2][0] = 2.0       * (spin[2] * spin[0] - spin[1] * spin[3]);
   matrix[2][1] = 2.0       * (spin[1] * spin[2] + spin[0] * spin[3]);
   matrix[2][2] = 1.0 - 2.0 * (spin[1] * spin[1] + spin[0] * spin[0]);
}


void
spin_normalize(spin)
   Spin spin;
{
   int  which, i;
   Real gr;
   
   which = 0;
   gr = spin[which];
   for (i = 1 ; i < 4 ; i++) {
      if (FABS(spin[i]) > FABS(gr)) {
	 gr = spin[i];
	 which = i;
      }
   }
   
   spin[which] = 0.0;
   spin[which] = SQRT(1.0 - (spin[0]*spin[0] + spin[1]*spin[1] +
			     spin[2]*spin[2] + spin[3]*spin[3]));
   
   if (gr < 0.0)
      spin[which] = -spin[which];
}
