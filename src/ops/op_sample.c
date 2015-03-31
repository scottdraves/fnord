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
   "$Id: op_sample.c,v 1.8 1992/06/22 17:07:08 mas Exp $";
#endif

#include "defs.h"

#define SAMPLE_ARG 0

/*
  sample takes a set and removes its connectivity.
*/

METset *
MET_op_sample(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[1], *result = MET_NULL_SET;
   METtype	*arg_type[1], *my_result_type = MET_NULL_TYPE;
   int		arg_set_type[1];
   METconnex	*connex;
   METblock	*block;
   int		i;
   MME(MET_op_sample);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, ANY_NUMBER_OF_KIDS);
#endif

   arg_set[SAMPLE_ARG] =
      MET_sym_be_set_ref(sym->kids[SAMPLE_ARG], type,
			 &arg_type[SAMPLE_ARG],
			 &arg_set_type[SAMPLE_ARG]);

   if (MET_NULL_TYPE == arg_type[SAMPLE_ARG])
      goto bail1;
   
   if (MET_TYPE_TYPE_SET_OF == arg_type[SAMPLE_ARG]->type) {
      MET_TYPE_COPY(my_result_type, arg_type[SAMPLE_ARG]);
   }

   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "argument to op sample is not a set", SEV_FATAL);
      goto bail1;
   }

   if (MET_op_type_only(arg_set, 1, result_type, my_result_type,
			set_type, MET_SET_TYPE_UNKNOWN))
       goto bail2;
   MET_op_reconcile_sets(arg_set, 1);

   MET_TYPE_FREE(my_result_type);
   
   if (MET_SET_TYPE_BLOCK == arg_set[SAMPLE_ARG]->type) {
      MET_SET_COPY_DEEP(result, arg_set[SAMPLE_ARG]);
      block = result->info.block.block_list;
      while (MET_NULL_BLOCK != block) {
	 MET_CONNEX_COPY_DEEP(connex, block->connex);
	
	 for (i = 0; i < connex->ndims; i++)
	    if (connex->dims[i] > 0)
	       connex->dims[i] *= -1;
	 
	 MET_CONNEX_FREE(block->connex);
	 MET_CONNEX_COPY(block->connex, connex);
	 MET_CONNEX_FREE(connex);
	 
	 MET_CONNEX_COPY_DEEP(connex, block->data_connex);
	
	 for (i = 0; i < connex->ndims; i++)
	    if (connex->dims[i] > 0)
	       connex->dims[i] *= -1;
	 
	 MET_CONNEX_FREE(block->data_connex);
	 MET_CONNEX_COPY(block->data_connex, connex);
	 MET_CONNEX_FREE(connex);

	 block = block->next;
      }
   } else {
      ERROR(sym->origin, "cannot apply sample to bare sets (yet)",
	    SEV_WARNING);
      MET_SET_COPY(result, arg_set[SAMPLE_ARG]);
   }

 bail1:
   MET_set_get_type(result, result_type, set_type);

 bail2:
   COND_FREE(SAMPLE_ARG);
   
   return result;
}
