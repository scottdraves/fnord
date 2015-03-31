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
static char Version[] =
   "$Id: op_cat.c,v 1.4 1992/06/22 17:05:59 mas Exp $";
#endif

#include "defs.h"

#define CAT_NUM_KIDS		2
#define CAT_LEFT		0
#define CAT_RIGHT		1



METset *
MET_op_cat(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[CAT_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[CAT_NUM_KIDS];
   METtype	*arg_type[CAT_NUM_KIDS];
   METtype	*my_result_type = MET_NULL_TYPE, *guess;
   METverts	*s1, *s2, *dst;
   METdata	*d1, *d2;
   int		how_many, total;
   MME(MET_op_cat);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, CAT_NUM_KIDS);
#endif
   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_VECTOR == type->type) {
      MET_TYPE_NEW_VECTOR(guess, type->info.vector.of,
			  TYPE_DIMENSION_UNKNOWN,
			  type->info.vector.is_row);
   } else
      MET_TYPE_NEW_UNKNOWN(guess);

   arg_set[CAT_LEFT] =
      MET_sym_be_set_ref(sym->kids[CAT_LEFT], guess,
			 &arg_type[CAT_LEFT],
			 &arg_set_type[CAT_LEFT]);

   arg_set[CAT_RIGHT] =
      MET_sym_be_set_ref(sym->kids[CAT_RIGHT], guess,
			 &arg_type[CAT_RIGHT],
			 &arg_set_type[CAT_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[CAT_LEFT] ||
       MET_NULL_TYPE == arg_type[CAT_RIGHT])
      goto bail1;
   
   if (arg_type[CAT_LEFT]->type == MET_TYPE_TYPE_VECTOR &&
       arg_type[CAT_RIGHT]->type == MET_TYPE_TYPE_VECTOR &&
       (arg_type[CAT_RIGHT]->info.vector.is_row ==
	arg_type[CAT_LEFT]->info.vector.is_row)) {
      if (SUCCESS == MET_type_cast(arg_type[CAT_LEFT]->info.vector.of,
				   arg_type[CAT_RIGHT]->info.vector.of,
				   &guess, TYPE_CAST_MAKE_BOGUS)) {
	 if ((arg_type[CAT_LEFT]->info.vector.dimension ==
	      TYPE_DIMENSION_UNKNOWN) ||
	     (arg_type[CAT_RIGHT]->info.vector.dimension ==
	      TYPE_DIMENSION_UNKNOWN))
	    total = TYPE_DIMENSION_UNKNOWN;
	 else
	    total = arg_type[CAT_LEFT]->info.vector.dimension +
	       arg_type[CAT_RIGHT]->info.vector.dimension;
	 MET_TYPE_NEW_VECTOR(my_result_type, guess, total,
			     arg_type[CAT_RIGHT]->info.vector.is_row);
	 MET_TYPE_FREE(guess);
      } else {
	 MET_TYPE_FREE(guess);
	 guess = MET_NULL_TYPE;
      }
   }
   
   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "arguments of :: are of wrong type", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, CAT_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE)) {
      goto bail2;
   }

   MET_op_reconcile_sets(arg_set, CAT_NUM_KIDS);

   d1 = arg_set[CAT_LEFT]->info.bare.data;
   d2 = arg_set[CAT_RIGHT]->info.bare.data;
   s1 = d1->verts;
   s2 = d2->verts;

   MAX_VERTS(how_many, d1, d2);
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, CAT_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);

   dst = result_set->info.bare.data->verts;
   MET_verts_copy_vertices(dst, 0, 0,
			   s1, 0, d1->first_col,
			   d1->num_cols, dst->num_verts,
			   1, (how_many != s1->num_verts));
   MET_verts_copy_vertices(dst, 0, d1->num_cols,
			   s2, 0, d2->first_col,
			   d2->num_cols, dst->num_verts,
			   1, (how_many != s2->num_verts));
 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(CAT_LEFT);
   COND_FREE(CAT_RIGHT);


   return result_set;
}
