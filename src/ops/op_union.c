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
   "$Id: op_union.c,v 1.12 1992/06/22 17:07:31 mas Exp $";
#endif

#include "defs.h"

#define UNION_NUM_KIDS		2
#define UNION_LEFT		0
#define UNION_RIGHT		1

METset *
MET_op_union(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[UNION_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[UNION_NUM_KIDS];
   METtype	*arg_type[UNION_NUM_KIDS];
   METblock	*block, *tblock;
   MME(MET_op_union);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, UNION_NUM_KIDS);
#endif
   arg_set[UNION_LEFT] =
      MET_sym_be_set_ref(sym->kids[UNION_LEFT], type,
			 &arg_type[UNION_LEFT],
			 &arg_set_type[UNION_LEFT]);
   
   arg_set[UNION_RIGHT] =
      MET_sym_be_set_ref(sym->kids[UNION_RIGHT], type,
			 &arg_type[UNION_RIGHT],
			 &arg_set_type[UNION_RIGHT]);

   if (MET_op_type_only(arg_set, 2, result_type, MET_NULL_TYPE,
			arg_set_type, MET_SET_TYPE_BLOCK))
      goto bail;

   if (MET_op_bare_only(arg_set, 2, MET_op_union, MET_SYM_OP_UNION, type)) {
      MET_SET_COPY(result_set, arg_set[0]);
   } else {
      if (MET_SET_TYPE_BLOCK != arg_set_type[UNION_RIGHT]) {
	 ERROR(sym->origin, "right argument to op union is not a set",
	       SEV_FATAL);
      } else if (MET_SET_TYPE_BLOCK != arg_set_type[UNION_LEFT]) {
	 ERROR(sym->origin, "left argument to op union is not a set",
	       SEV_FATAL);
      } else {
	 result_set = MET_set_new(MET_SET_TYPE_BLOCK);
	 result_set->info.block.block_list = MET_NULL_BLOCK;
	 
	 block = arg_set[UNION_LEFT]->info.block.block_list;
	 if (MET_NULL_BLOCK != block) {
	    MET_BLOCK_COPY_DEEP(tblock, block);
	    MET_set_add_block(result_set, tblock);
	    MET_BLOCK_FREE(tblock);
	 }
	 block = arg_set[UNION_RIGHT]->info.block.block_list;
	 MET_set_add_block(result_set, block);
      }
   }
   MET_set_get_type(result_set, result_type, set_type);

 bail:
   COND_FREE(UNION_LEFT);
   COND_FREE(UNION_RIGHT);
   return result_set;
}
