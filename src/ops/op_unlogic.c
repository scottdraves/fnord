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
   "$Id: op_unlogic.c,v 1.8 1992/06/22 17:07:34 mas Exp $";
#endif

#include "defs.h"

#define UNLOGIC_NUM_KIDS	1
#define UNLOGIC_ARG		0


/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME) \
static void \
NAME(arg1, result) \
   METdata	*arg1, *result; \
{ \
   int		i; \
   METztwo	*src1; \
   METztwo	*dst; \
   int		src1_width, dst_width; \
   MME(NAME); \
 \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   src1 = DATA_FIRST_VERTEX(arg1, METztwo); \
   dst  = DATA_FIRST_VERTEX(result,  METztwo); \
   i = DATA_NUM_VERTS(arg1); \
    \
   while(i--) { \
      BODY; \
      dst += dst_width; \
      src1 += src1_width; \
   } \
} \



#define BODY *dst = (METztwo) ! (*src1)
TEMPLATE(MET__op_unlogic_guts_not)
#undef BODY

static Boolean
MET__op_unlogic_guts(opcode, arg1, result)
   int		opcode;
   METdata	*arg1, *result;
{
   MME(MET__op_subtract_guts);

   switch (opcode) {
    case MET_SYM_OP_NOT:
      MET__op_unlogic_guts_not(arg1, result);
      break;

    default:
      DEFAULT_ERR(opcode);
   }
   
   return SUCCESS;
}
/*
 * deals with "and" and "or"
 */
METset *
MET_op_unlogic(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[UNLOGIC_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[UNLOGIC_NUM_KIDS];
   METtype	*arg_type[UNLOGIC_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many;
   MME(MET_op_unlogic);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, UNLOGIC_NUM_KIDS);
#endif
   MET_TYPE_NEW_FIELD(guess, MET_FIELD_ZTWO);
   arg_set[UNLOGIC_ARG] =
      MET_sym_be_set_ref(sym->kids[UNLOGIC_ARG], guess,
			 &arg_type[UNLOGIC_ARG],
			 &arg_set_type[UNLOGIC_ARG]);
   
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE != arg_type[UNLOGIC_ARG] &&
       MET_TYPE_TYPE_FIELD == arg_type[UNLOGIC_ARG]->type &&
       MET_FIELD_ZTWO == arg_type[UNLOGIC_ARG]->info.field.type) {

      MET_TYPE_NEW_FIELD(my_result_type, MET_FIELD_ZTWO);
   }
   
   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "argument to unary logic op is not a boolean",
	    SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, UNLOGIC_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;
   MET_op_reconcile_sets(arg_set, UNLOGIC_NUM_KIDS);
   
   how_many = DATA_NUM_VERTS(arg_set[UNLOGIC_ARG]->info.bare.data);
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, UNLOGIC_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);
   
   if (FAILURE ==
       MET__op_unlogic_guts(sym->info.op.code,
			  arg_set[UNLOGIC_ARG]->info.bare.data,
			  result_set->info.bare.data)) {
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(UNLOGIC_ARG);

   return result_set;
}
