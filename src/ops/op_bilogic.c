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
   "$Id: op_bilogic.c,v 1.10 1992/06/22 17:05:54 mas Exp $";
#endif

#include "defs.h"

#define BILOGIC_NUM_KIDS	2
#define BILOGIC_LEFT		0
#define BILOGIC_RIGHT		1


/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME) \
static void \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		i; \
   METztwo	*src1, *src2; \
   METztwo	*dst; \
   int		src1_width, src2_width, dst_width; \
   MME(NAME); \
 \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   src1 = DATA_FIRST_VERTEX(arg1, METztwo); \
   src2 = DATA_FIRST_VERTEX(arg2, METztwo); \
   dst  = DATA_FIRST_VERTEX(result,  METztwo); \
   MAX_VERTS(i, arg1, arg2); \
    \
   while(i--) { \
      BODY; \
      dst += dst_width; \
      src1 += src1_width; \
      src2 += src2_width; \
   } \
} \



#define BODY *dst = (METztwo) (*src1 & *src2)
TEMPLATE(MET__op_bilogic_guts_and)
#undef BODY
#define BODY *dst = (METztwo) (*src1 | *src2)
TEMPLATE(MET__op_bilogic_guts_or)
#undef BODY

static Boolean
MET__op_bilogic_guts(opcode, arg1, arg2, result)
   int		opcode;
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_subtract_guts);

   switch (opcode) {
    case MET_SYM_OP_AND:
      MET__op_bilogic_guts_and(arg1, arg2, result);
      break;

    case MET_SYM_OP_OR:
      MET__op_bilogic_guts_or(arg1, arg2, result);
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
MET_op_bilogic(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[BILOGIC_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[BILOGIC_NUM_KIDS];
   METtype	*arg_type[BILOGIC_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many;
   MME(MET_op_bilogic);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, BILOGIC_NUM_KIDS);
#endif
   MET_TYPE_NEW_FIELD(guess, MET_FIELD_ZTWO);
   arg_set[BILOGIC_LEFT] =
      MET_sym_be_set_ref(sym->kids[BILOGIC_LEFT], guess,
			 &arg_type[BILOGIC_LEFT],
			 &arg_set_type[BILOGIC_LEFT]);
   
   arg_set[BILOGIC_RIGHT] =
      MET_sym_be_set_ref(sym->kids[BILOGIC_RIGHT], guess,
			 &arg_type[BILOGIC_RIGHT],
			 &arg_set_type[BILOGIC_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE != arg_type[BILOGIC_LEFT] &&
       MET_NULL_TYPE != arg_type[BILOGIC_RIGHT] &&
       MET_TYPE_TYPE_FIELD == arg_type[BILOGIC_RIGHT]->type &&
       MET_FIELD_ZTWO == arg_type[BILOGIC_RIGHT]->info.field.type &&
       MET_TYPE_TYPE_FIELD == arg_type[BILOGIC_LEFT]->type &&
       MET_FIELD_ZTWO == arg_type[BILOGIC_LEFT]->info.field.type) {

      MET_TYPE_NEW_FIELD(my_result_type, MET_FIELD_ZTWO);
   }
   
   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "bilogic, bad argument types", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, BILOGIC_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;
   MET_op_reconcile_sets(arg_set, BILOGIC_NUM_KIDS);
   
   MAX_VERTS(how_many, arg_set[BILOGIC_LEFT]->info.bare.data,
	     arg_set[BILOGIC_RIGHT]->info.bare.data);
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, BILOGIC_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);
   
   if (FAILURE ==
       MET__op_bilogic_guts(sym->info.op.code,
			  arg_set[BILOGIC_LEFT]->info.bare.data,
			  arg_set[BILOGIC_RIGHT]->info.bare.data,
			  result_set->info.bare.data)) {
      ERROR(sym->origin, "bilogic, bad argument fields", SEV_FATAL);
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(BILOGIC_LEFT);
   COND_FREE(BILOGIC_RIGHT);

   return result_set;
}
