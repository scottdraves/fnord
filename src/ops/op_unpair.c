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
   "$Id: op_unpair.c,v 1.12 1992/06/22 17:07:36 mas Exp $";
#endif

#include "defs.h"

#define UNPAIR_NUM_KIDS	1
#define UNPAIR_ARG	0


METset *
MET_op_unpair(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[UNPAIR_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[UNPAIR_NUM_KIDS], ind;
   METtype	*arg_type[UNPAIR_NUM_KIDS], *guess, *t;
   METtype	*my_result_type = MET_NULL_TYPE;
   Boolean	pair_left;
   METdata	*first_of_right;
   MME(MET_op_unpair);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, UNPAIR_NUM_KIDS);
#endif
   pair_left = (sym->info.op.code == MET_SYM_OP_PAIR_LEFT);
   MET_TYPE_NEW_UNKNOWN(guess);
   if (MET_NULL_TYPE != type) {
      MET_TYPE_FREE(guess);
      if (MET_TYPE_TYPE_SET == type->type) {
	 MET_TYPE_COPY(guess, type);
      } else {
	 MET_TYPE_NEW_UNKNOWN(t);
	 if (pair_left) {
	    MET_TYPE_NEW_PAIR(guess, type, t);
	 } else {
	    MET_TYPE_NEW_PAIR(guess, t, type);
	 }
	 MET_TYPE_FREE(t);
      }
   }
   
   arg_set[UNPAIR_ARG] =
      MET_sym_be_set_ref(sym->kids[UNPAIR_ARG], guess,
			 &arg_type[UNPAIR_ARG],
			 &arg_set_type[UNPAIR_ARG]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[UNPAIR_ARG])
      goto bail1;
   
   MET_op_reconcile_sets(arg_set, UNPAIR_NUM_KIDS);

   if (MET_NULL_SET != arg_set[UNPAIR_ARG] &&
       MET_SET_TYPE_TYPE == arg_set[UNPAIR_ARG]->type) {
      COND_FREE(UNPAIR_ARG);
      return MET_op_type_expr(sym, type, result_type, set_type);
   }
   /*
    * make sure it is a pair
    */
   if (MET_TYPE_TYPE_PAIR == arg_type[UNPAIR_ARG]->type) {
      if (pair_left) {
	 MET_TYPE_COPY(my_result_type,
		       arg_type[UNPAIR_ARG]->info.pair.left);
      } else {
	 MET_TYPE_COPY(my_result_type,
		       arg_type[UNPAIR_ARG]->info.pair.right);
      }
   }
   
   if (MET_NULL_TYPE == my_result_type) {
      MET_op_error_bad_arg_type(1, arg_type[UNPAIR_ARG], "(?, ?)", sym);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, UNPAIR_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_UNKNOWN))
       goto bail2;

   result_set = MET_set_new(MET_SET_TYPE_BARE);
   result_set->info.bare.how_many = arg_set[UNPAIR_ARG]->info.bare.how_many;
   MET_tags_initialize(&result_set->info.bare.tags_bare);
   MET_tags_initialize(&result_set->info.bare.tags_in);
   MET_tags_copy(&result_set->info.bare.tags_bare, 
		 &arg_set[UNPAIR_ARG]->info.bare.tags_bare);
   MET_tags_copy(&result_set->info.bare.tags_in, 
		 &arg_set[UNPAIR_ARG]->info.bare.tags_in);
   MET_TYPE_COPY(result_set->info.bare.type, my_result_type);
   /*
    * first_of_right divides the original data into two parts, one for
    * the left and one for the right.  we either copy it, or we copy up
    * to it.
    */
   ind = MET_type_count_data(arg_type[UNPAIR_ARG]->info.pair.left);
   first_of_right = MET_data_index(arg_set[UNPAIR_ARG]->info.bare.data,
				   ind);
   if (pair_left) {
      result_set->info.bare.data =
	 MET_data_copy_upto(arg_set[UNPAIR_ARG]->info.bare.data,
			    first_of_right);
   } else {
      MET_DATA_COPY(result_set->info.bare.data, first_of_right);
   }
   MET_DATA_FREE(first_of_right);
   
   MET_TYPE_FREE(my_result_type);
   
 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(UNPAIR_ARG);
   
   return result_set;
}
