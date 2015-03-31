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
   "$Id: op_pairs.c,v 1.16 1992/06/22 17:06:45 mas Exp $";
#endif

#define PAIR_BUILD_LEFT		0
#define PAIR_BUILD_RIGHT	1
#define PAIR_BUILD_NUM_KIDS	2

#include "defs.h"

static METdata *
MET_op_pair_build_compute_data(set, set_type, num_verts)
   METset	*set;
   int		set_type, *num_verts;
{
   METdata	*data;
   MME(MET_op_pair_build_compute_data);

   switch (set_type) {

    case MET_SET_TYPE_BARE:
      MET_DATA_COPY(data, set->info.bare.data);
      *num_verts = DATA_NUM_VERTS(data);
      break;

    case MET_SET_TYPE_BLOCK:
    case MET_SET_TYPE_SYMBOL:
    case MET_SET_TYPE_SYM_CONST:
    case MET_SET_TYPE_TYPE:
      data = MET_set_to_data(set);
      *num_verts = 1;
      break;

    default:
      DEFAULT_ERR(set_type);
   }

   return data;
}

   

METset *
MET_op_pair_build(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[2], *result_set = MET_NULL_SET;
   METtype	*left_type, *right_type, *guess, *my_result_type;
   int		left_set_type, right_set_type;
   METdata	*left_data, *right_data, *left_copy, *temp_data;
   int		left_num_verts, right_num_verts, final_verts;
   MME(MET_op_pair_build);
   left_type = right_type = MET_NULL_TYPE;
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, PAIR_BUILD_NUM_KIDS);
#endif

   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_SET == type->type)
      return MET_op_type_expr(sym, type, result_type, set_type);

   /*
    * evaluate arguments
    */
   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_PAIR == type->type) {
      MET_TYPE_COPY(guess, type->info.pair.left);
   } else if (MET_NULL_TYPE != type &&
	      MET_TYPE_TYPE_SET == type->type) {
      MET_TYPE_COPY(guess, type);
   } else {
      MET_TYPE_NEW_UNKNOWN(guess);
   }
   arg_set[0] = MET_sym_be_set_ref(sym->kids[PAIR_BUILD_LEFT], guess,
				 &left_type, &left_set_type);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_PAIR == type->type) {
      MET_TYPE_COPY(guess, type->info.pair.right);
   } else if (MET_NULL_TYPE != type &&
	      MET_TYPE_TYPE_SET == type->type) {
      MET_TYPE_COPY(guess, type);
   } else {
      MET_TYPE_NEW_UNKNOWN(guess);
   }
   arg_set[1] = MET_sym_be_set_ref(sym->kids[PAIR_BUILD_RIGHT], guess,
				  &right_type, &right_set_type);
   MET_TYPE_FREE(guess);

   /*
    * determine type of result, if we can
    */
   if (MET_NULL_TYPE == left_type ||
       MET_NULL_TYPE == right_type)
      goto bail1;

   MET_TYPE_NEW_PAIR(my_result_type, left_type, right_type);
   if ((METtype **) NULL != result_type)
      MET_TYPE_COPY(*result_type, my_result_type);
   if (INULL != set_type)
      *set_type = MET_SET_TYPE_BARE;
	 

   /*
    * create the result, if we can
    */
   if (MET_NULL_SET == arg_set[0] ||
       MET_NULL_SET == arg_set[1])
      goto bail2;
   
   MET_op_reconcile_sets(arg_set, PAIR_BUILD_NUM_KIDS);

   result_set = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(result_set->info.bare.type, my_result_type);

   left_data = MET_op_pair_build_compute_data(arg_set[0], left_set_type,
					      &left_num_verts);
   right_data = MET_op_pair_build_compute_data(arg_set[1], right_set_type,
					       &right_num_verts);
#ifdef MET__DEBUG
   if (left_num_verts != right_num_verts &&
       left_num_verts != 1 &&
       right_num_verts != 1)
      FATAL((ERR_ARB, "different length datas: %d %d",
	     left_num_verts, right_num_verts));
#endif
   final_verts = left_num_verts; /* assume they are the same */
   if (left_num_verts != right_num_verts) {
      if (left_num_verts != 1) {
	 temp_data = MET_data_replicate(right_data, left_num_verts);
	 MET_DATA_FREE(right_data);
	 right_data = temp_data;
	 final_verts = left_num_verts;
      } else if (right_num_verts != 1) {
	 temp_data = MET_data_replicate(left_data, right_num_verts);
	 MET_DATA_FREE(left_data);
	 left_data = temp_data;
	 final_verts = right_num_verts;
      }
   }
   result_set->info.bare.how_many = final_verts;
   MET_tags_initialize(&result_set->info.bare.tags_bare);
   MET_tags_initialize(&result_set->info.bare.tags_in);
   if (MET_SET_TYPE_BARE == arg_set[0]->type) {
      MET_tags_copy(&result_set->info.bare.tags_bare, 
		    &arg_set[0]->info.bare.tags_bare);
      MET_tags_copy(&result_set->info.bare.tags_in, 
		    &arg_set[0]->info.bare.tags_in);
   } if (MET_SET_TYPE_BARE == arg_set[1]->type) {
      MET_tags_copy(&result_set->info.bare.tags_bare, 
		    &arg_set[1]->info.bare.tags_bare);
      MET_tags_copy(&result_set->info.bare.tags_in, 
		    &arg_set[1]->info.bare.tags_in);
   }

   MET_DATA_COPY_DEEP(left_copy, left_data);
   MET_DATA_FREE(left_data);
   left_data = left_copy;
   while (MET_NULL_DATA != left_data->next)
      left_data = left_data->next;
   MET_DATA_COPY(left_data->next, right_data);
   MET_DATA_FREE(right_data);
   MET_DATA_COPY(result_set->info.bare.data, left_copy);
   MET_DATA_FREE(left_copy);

 bail2:
   MET_TYPE_FREE(my_result_type);
 bail1:
   if (MET_NULL_SET != arg_set[0])
      MET_SET_FREE(arg_set[0]);

   if (MET_NULL_SET != arg_set[1])
      MET_SET_FREE(arg_set[1]);

   if (MET_NULL_TYPE != left_type)
      MET_TYPE_FREE(left_type);

   if (MET_NULL_TYPE != right_type)
      MET_TYPE_FREE(right_type);

   return result_set;
}
