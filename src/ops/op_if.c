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
   "$Id: op_if.c,v 1.11 1992/06/22 17:06:25 mas Exp $";
#endif

#include "defs.h"

#define IF_NUM_KIDS	3
#define IF_TRUE		0	/* take this arg if clause is true */
#define IF_CLAUSE	1
#define IF_FALSE	2

static void
MET__op_if_up_blocks(args)
   METset	**args;
{
   METset	*new_set;
   MME(MET__op_if_up_blocks);

   if (MET_SET_TYPE_BLOCK == args[IF_TRUE]->type) {
      new_set = MET_set_de_canonize(args[IF_TRUE]);
      MET_SET_FREE(args[IF_TRUE]);
      MET_SET_COPY(args[IF_TRUE], new_set);
      MET_SET_FREE(new_set);
   }

   if (MET_SET_TYPE_BLOCK == args[IF_FALSE]->type) {
      new_set = MET_set_de_canonize(args[IF_FALSE]);
      MET_SET_FREE(args[IF_FALSE]);
      MET_SET_COPY(args[IF_FALSE], new_set);
      MET_SET_FREE(new_set);
   }
}

static Boolean
MET__op_if_else_guts(is_if_else, arg1, arg2, arg3, result)
   Boolean	is_if_else;
   METdata	*arg1, *arg2, *arg3, *result;
{
   int		dst, src1, src3;
   int		width1, width2, width3;
   METztwo	*src2;
   int		max_verts = DATA_NUM_VERTS(result);
   METdata	*chosen;
   Boolean	error = SUCCESS;
   MME(MET__op_if_else_guts);

   width1 = (1 == DATA_NUM_VERTS(arg1)) ? 0 : 1;
   if (is_if_else)
      width3 = (1 == DATA_NUM_VERTS(arg3)) ? 0 : 1;
   width2 = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols;
   src2 = DATA_FIRST_VERTEX(arg2, METztwo);
   src1 = src3 = 0;

   dst = 0;
   if (is_if_else) {
      while (dst < max_verts) {
	 chosen = *src2 ? arg1 : arg3;
	 MET_verts_copy_vertices(result->verts, dst, result->first_col,
				 chosen->verts, *src2 ? src1 : src3,
				 chosen->first_col,
				 chosen->num_cols, 1, 1, NO);
	 src1 += width1;
	 src2 += width2;
	 src3 += width3;
	 dst++;
      }
   } else {
      /* yes, this is really stupid */ /* Ack! */
      chosen = arg1;
      while (dst < max_verts) {
	 error &= *src2;
	 MET_verts_copy_vertices(result->verts, dst, result->first_col,
				 chosen->verts, src1, chosen->first_col,
				 chosen->num_cols, 1, 1, NO);
	 src1 += width1;
	 src2 += width2;
	 dst++;
      }
   }
   
   if (SUCCESS == error) {
      if (MET_NULL_DATA != result->next)
	 return MET__op_if_else_guts(is_if_else, arg1->next, arg2,
				     is_if_else ? arg3->next :
				     MET_NULL_DATA, result->next);
      else
	 return SUCCESS;
   }

   return FAILURE;
}

/*
 * if the clause is non-poly, then we are just choosing between our
 * operands.  more importantly, we only have to eval the operand that is
 * needed.  this makes recursion useful...
 */
static METset *
MET__op_if_simple(clause, sym, type, result_type, set_type)
   METset	*clause;
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*result_set = MET_NULL_SET;
   METtype	*clause_type;
   Boolean	result, is_if_else;
   MME(MET__op_if_simple);

   is_if_else = (sym->info.op.code == MET_SYM_OP_IF_ELSE);
   clause_type = clause->info.bare.type;

   if (MET_TYPE_TYPE_FIELD == clause_type->type &&
       MET_FIELD_ZTWO == clause_type->info.field.type) {
      result = *DATA_FIRST_VERTEX(clause->info.bare.data, METztwo);
      if (result) {
	 result_set =
	    MET_sym_be_set_ref(sym->kids[IF_TRUE], type,
			       result_type, set_type);
      } else {
	 if (is_if_else) {
	    result_set =
	       MET_sym_be_set_ref(sym->kids[IF_FALSE], type,
				  result_type, set_type);
	 } else {
	    MET_set_get_type(MET_NULL_SET, result_type, set_type);
	    ERROR(sym->origin, "if failed", SEV_FATAL);
	 }
      }
   } else {
      MET_set_get_type(MET_NULL_SET, result_type, set_type);
      MET_op_error_bad_arg_type(2, clause_type, "B", sym);
   }
   MET_SET_FREE(clause);

   return result_set;
}

METset *
MET_op_if(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[IF_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[IF_NUM_KIDS];
   METtype	*arg_type[IF_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many;
   Boolean	result, is_if_else;
   MME(MET_op_if);

   is_if_else = (sym->info.op.code == MET_SYM_OP_IF_ELSE);

   MET_TYPE_NEW_FIELD(guess, MET_FIELD_ZTWO);
   arg_set[IF_CLAUSE] =
      MET_sym_be_set_ref(sym->kids[IF_CLAUSE], guess,
			 &arg_type[IF_CLAUSE],
			 &arg_set_type[IF_CLAUSE]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_SET != arg_set[IF_CLAUSE] &&
       MET_SET_TYPE_BARE == arg_set[IF_CLAUSE]->type &&
       1 == arg_set[IF_CLAUSE]->info.bare.how_many) {
      MET_TYPE_FREE(arg_type[IF_CLAUSE]);
      return MET__op_if_simple(arg_set[IF_CLAUSE], sym, type, result_type,
			       set_type);
   }


   arg_set[IF_TRUE] =
      MET_sym_be_set_ref(sym->kids[IF_TRUE], type,
			 &arg_type[IF_TRUE],
			 &arg_set_type[IF_TRUE]);
   
   if (YES == is_if_else) {
      arg_set[IF_FALSE] =
	 MET_sym_be_set_ref(sym->kids[IF_FALSE], type,
			    &arg_type[IF_FALSE],
			    &arg_set_type[IF_FALSE]);
   }

   if (MET_NULL_TYPE == arg_type[IF_TRUE] ||
       MET_NULL_TYPE == arg_type[IF_CLAUSE] ||
       (MET_NULL_TYPE == arg_type[IF_FALSE] && is_if_else))
      goto bail1;

   if (MET_TYPE_TYPE_FIELD != arg_type[IF_CLAUSE]->type ||
       MET_FIELD_ZTWO != arg_type[IF_CLAUSE]->info.field.type) {
      MET_op_error_bad_arg_type(2, arg_type[IF_CLAUSE], "B", sym);
      goto bail1;
   }

   if (YES == is_if_else) {
      if (FAILURE == MET_type_cast(arg_type[IF_TRUE], arg_type[IF_FALSE],
				   &my_result_type, TYPE_CAST_MAKE_BOGUS)){
	 MET_TYPE_FREE(my_result_type);
	 my_result_type = MET_NULL_TYPE;
      }
   } else	
      MET_TYPE_COPY(my_result_type, arg_type[IF_TRUE]);
   
   if (MET_NULL_TYPE == my_result_type) {
      MET_op_cant_cast(sym, arg_type[IF_TRUE], arg_type[IF_FALSE], CNULL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, is_if_else ? 3 : 2,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;

   MET_op_reconcile_sets(arg_set, IF_NUM_KIDS);

   MET__op_if_up_blocks(arg_set);

   MAX_VERTS(how_many,
	     arg_set[IF_TRUE]->info.bare.data,
	     arg_set[IF_CLAUSE]->info.bare.data);
   if (YES == is_if_else)
      how_many = MAX(how_many,
		     DATA_NUM_VERTS(arg_set[IF_FALSE]->info.bare.data));
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, IF_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);
   
   result = MET__op_if_else_guts(is_if_else,
				 arg_set[IF_TRUE]->info.bare.data,
				 arg_set[IF_CLAUSE]->info.bare.data,
				 is_if_else ? 
				 arg_set[IF_FALSE]->info.bare.data :
				 MET_NULL_DATA,
				 result_set->info.bare.data);
   
   if (FAILURE == result) {
      ERROR(sym->origin, "if, bad argument field", SEV_FATAL);
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(IF_TRUE);
   COND_FREE(IF_CLAUSE);
   if (is_if_else)
      COND_FREE(IF_FALSE);

   return result_set;
}
