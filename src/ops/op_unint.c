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
   "$Id: op_unint.c,v 1.10 1992/06/22 17:07:29 mas Exp $";
#endif

#include "defs.h"

#define UNINT_NUM_KIDS	1
#define UNINT_ARG	0

static METinteger factorial_lut[] = {
   1, 1, 2, 6, 24, 120, 720, 5040,
   40320, 362880, 3628800, 39916800,
   479001600,
};

#define FACTORIAL_TABLE_SIZE (sizeof(factorial_lut)/sizeof(METinteger))

/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME, TYPE1) \
static Boolean \
NAME(arg1, result) \
   METdata	*arg1, *result; \
{ \
   int		i; \
   TYPE1	*src1; \
   METinteger	*dst; \
   Boolean	error = SUCCESS; \
   int		src1_width, dst_width; \
   TEMPS \
   MME(NAME); \
 \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   src1 = DATA_FIRST_VERTEX(arg1, TYPE1); \
   dst  = DATA_FIRST_VERTEX(result,  METinteger); \
   i = DATA_NUM_VERTS(arg1); \
    \
   while(i--) { \
      BODY; \
      dst += dst_width; \
      src1 += src1_width; \
   } \
   return error; \
} \


#define TEMPS int t;
#define BODY \
   t = (int) *src1; \
if (t >= 0 && t < FACTORIAL_TABLE_SIZE) \
   *dst = (METinteger) factorial_lut[t]; \
else error = FAILURE;
TEMPLATE(MET__op_unint_guts_factorial, METinteger)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst = (METinteger) ~ (*src1)
TEMPLATE(MET__op_unint_guts_bitnot, METinteger)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst = (METinteger) (*src1)
TEMPLATE(MET__op_unint_guts_rint, METreal)
#undef BODY
#undef TEMPS

static Boolean
MET__op_unint_guts(opcode, arg1, result)
   int		opcode;
   METdata	*arg1, *result;
{
   MME(MET__op_subtract_guts);

   switch (opcode) {
    case MET_SYM_OP_FACTORIAL:
      return MET__op_unint_guts_factorial(arg1, result);

    case MET_SYM_OP_BITNOT:
      return MET__op_unint_guts_bitnot(arg1, result);

    case MET_SYM_OP_REAL_TO_INTEGER:
      return MET__op_unint_guts_rint(arg1, result);

    default:
      DEFAULT_ERR(opcode);
   }
   
   return SUCCESS;
}

METset *
MET_op_unint(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[UNINT_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[UNINT_NUM_KIDS];
   METtype	*arg_type[UNINT_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many;
   Boolean	is_rint;
   MME(MET_op_unint);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, UNINT_NUM_KIDS);
#endif
   is_rint = sym->info.op.code == MET_SYM_OP_REAL_TO_INTEGER;
   if (is_rint) {
      MET_TYPE_NEW_FIELD(guess, MET_FIELD_REAL);
   } else {
      MET_TYPE_NEW_FIELD(guess, MET_FIELD_INTEGER);
   }
   arg_set[UNINT_ARG] =
      MET_sym_be_set_ref(sym->kids[UNINT_ARG], guess,
			 &arg_type[UNINT_ARG],
			 &arg_set_type[UNINT_ARG]);
   
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE != arg_type[UNINT_ARG] &&
       MET_TYPE_TYPE_FIELD == arg_type[UNINT_ARG]->type &&
       (MET_FIELD_INTEGER == arg_type[UNINT_ARG]->info.field.type ||
	(is_rint &&
	 MET_FIELD_REAL == arg_type[UNINT_ARG]->info.field.type))) {
      MET_TYPE_NEW_FIELD(my_result_type, MET_FIELD_INTEGER);
   }
   
   if (MET_NULL_TYPE == my_result_type) {
      if (is_rint) {
	 ERROR(sym->origin,
	       "argument to op real to integer should be real or integer",
	       SEV_FATAL);
      } else {
	 ERROR(sym->origin,
	       "argument to op unint should be an integer",
	       SEV_FATAL);
      }
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, UNINT_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;
   MET_op_reconcile_sets(arg_set, UNINT_NUM_KIDS);
   
   how_many = DATA_NUM_VERTS(arg_set[UNINT_ARG]->info.bare.data);

   if (is_rint && MET_FIELD_INTEGER ==
       arg_type[UNINT_ARG]->info.field.type) {
      MET_SET_COPY(result_set, arg_set[UNINT_ARG]);
      MET_TYPE_FREE(my_result_type);
   } else {
      result_set = MET_type_to_polybare_set(my_result_type, how_many,
					    arg_set, UNINT_NUM_KIDS);
      MET_TYPE_FREE(my_result_type);
      
      if (FAILURE ==
	  MET__op_unint_guts(sym->info.op.code,
			     arg_set[UNINT_ARG]->info.bare.data,
			     result_set->info.bare.data)) {
	 ERROR(sym->origin, "op unint failed", SEV_FATAL);
	 MET_SET_FREE(result_set);
	 result_set = MET_NULL_SET;
      }
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(UNINT_ARG);

   return result_set;
}
