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
   "$Id: op_setof.c,v 1.13 1992/06/22 17:07:14 mas Exp $";
#endif

#include "defs.h"

#define SETOF_NUM_KIDS		1
#define SETOF_ARG		0

static int
MET__op_setof_process_data(type, data, ptr)
   METtype	*type;
   METdata	*data;
   char		*ptr;
{
   METset	*set = CAST(METset *, ptr);
   METblock	*block;
   MME(MET__op_setof_process_data);

   block = MET_data_to_block(data, type);
   MET_set_add_block(set, block);
   MET_BLOCK_FREE(block);

   return 0; /* this is ignored */
}

METset *
MET_op_setof(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[SETOF_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[SETOF_NUM_KIDS];
   METtype	*arg_type[SETOF_NUM_KIDS], *guess;
   METdata	*data, *save_data;
   MME(MET_op_setof);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, SETOF_NUM_KIDS);
#endif


   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_SET == type->type) {
      MET_TYPE_COPY(guess, type);
   }  else
      MET_TYPE_NEW_UNKNOWN(guess);

   arg_set[SETOF_ARG] =
      MET_sym_be_set_ref(sym->kids[SETOF_ARG], guess,
			 &arg_type[SETOF_ARG],
			 &arg_set_type[SETOF_ARG]);
   
   MET_TYPE_FREE(guess);
   
   if (MET_NULL_SET == arg_set[SETOF_ARG])
      goto bail1;
   MET_op_reconcile_sets(arg_set, SETOF_NUM_KIDS);

   if (MET_SET_TYPE_TYPE == arg_set[SETOF_ARG]->type) {
      COND_FREE(SETOF_ARG);
      return MET_op_type_expr(sym, type, result_type, set_type);
   }

   if (MET_SET_TYPE_BARE != arg_set_type[SETOF_ARG]) {
      result_set = MET_set_new(MET_SET_TYPE_BLOCK);
      result_set->info.block.block_list = MET_NULL_BLOCK;
      save_data = data = MET_set_to_data(arg_set[SETOF_ARG]);
      (void) MET_type_traverse_data(arg_type[SETOF_ARG],
				    &data, MET__op_setof_process_data,
				    (char *) result_set);
      MET_DATA_FREE(save_data);
   } else {
            
      if (MET_op_bare_only(arg_set, SETOF_NUM_KIDS, MET_op_setof, 
			   MET_SYM_OP_SET_OF, type)) {
	 MET_SET_COPY(result_set, arg_set[0]);
      } else {
	 /*
	  * take each data of the arg and convert it into a block and place it in
	  * the result set
	  */
	 result_set = MET_set_new(MET_SET_TYPE_BLOCK);
	 result_set->info.block.block_list = MET_NULL_BLOCK;
	 data = arg_set[SETOF_ARG]->info.bare.data;
	 (void) MET_type_traverse_data(arg_set[SETOF_ARG]->info.bare.type,
				       &data, MET__op_setof_process_data,
				       (char *) result_set);
      }
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);
   COND_FREE(SETOF_ARG);

   return result_set;
}
