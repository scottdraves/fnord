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
   "$Id: op_biint.c,v 1.9 1992/06/22 17:05:51 mas Exp $";
#endif

#include "defs.h"

#define BIINT_NUM_KIDS		2
#define BIINT_LEFT		0
#define BIINT_RIGHT		1


/*
 * compute combinatorial choose function
 * negative result signals error
 */
static int
MET__op_biint_choose(a, b, error)
   int a, b;
   Boolean *error;
{
   unsigned long x = 1, y = 1, t;
   MME(MET__op_biint_choose);

   if (a < 0 || b < 0 || b > a) {
      *error = FAILURE;
      return -1;
   }

   if (a == 0 || b == 0)
      return 1;

   t = a;
   while(a > b) {
      x *= a;
      a--;
   }
   b = t - b;
   while (b > 1) {
      y *= b;
      b--;
   }

   return x / y;
}

/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME) \
static Boolean \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		i; \
   METinteger	*src1, *src2; \
   METinteger	*dst; \
   Boolean	error = SUCCESS; \
   int		src1_width, src2_width, dst_width; \
   MME(NAME); \
 \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   src1 = DATA_FIRST_VERTEX(arg1, METinteger); \
   src2 = DATA_FIRST_VERTEX(arg2, METinteger); \
   dst  = DATA_FIRST_VERTEX(result,  METinteger); \
   MAX_VERTS(i, arg1, arg2); \
    \
   while(i--) { \
      BODY; \
      dst += dst_width; \
      src1 += src1_width; \
      src2 += src2_width; \
   } \
   return error; \
} \


#define BODY *dst = (METinteger) \
   MET__op_biint_choose((int)*src1, (int)*src2, &error)
TEMPLATE(MET__op_biint_guts_choose)
#undef BODY

#define BODY *dst = (METinteger) ((int)*src1 % (int)*src2)
TEMPLATE(MET__op_biint_guts_modulo)
#undef BODY
#define BODY *dst = (METinteger) ((int)*src1 & (int)*src2)
TEMPLATE(MET__op_biint_guts_bitand)
#undef BODY
#define BODY *dst = (METinteger) ((int)*src1 | (int)*src2)
TEMPLATE(MET__op_biint_guts_bitor)
#undef BODY
#define BODY *dst = (METinteger) ((int)*src1 ^ (int)*src2)
TEMPLATE(MET__op_biint_guts_bitxor)
#undef BODY

static Boolean
MET__op_biint_guts(opcode, arg1, arg2, result)
   int		opcode;
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_subtract_guts);

   switch (opcode) {
    case MET_SYM_OP_CHOOSE:
      return MET__op_biint_guts_choose(arg1, arg2, result);

    case MET_SYM_OP_MODULO:
      return MET__op_biint_guts_modulo(arg1, arg2, result);

    case MET_SYM_OP_BITAND:
      return MET__op_biint_guts_bitand(arg1, arg2, result);

    case MET_SYM_OP_BITOR:
      return MET__op_biint_guts_bitor(arg1, arg2, result);

    case MET_SYM_OP_BITXOR:
      return MET__op_biint_guts_bitxor(arg1, arg2, result);

    default:
      DEFAULT_ERR(opcode);
   }
   /* make lint happy */
   return SUCCESS;
}
/*
 * deals with choose, modulo, bitand, bitor, and bitxor
 */
METset *
MET_op_biint(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[BIINT_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[BIINT_NUM_KIDS];
   METtype	*arg_type[BIINT_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many;
   MME(MET_op_biint);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, BIINT_NUM_KIDS);
#endif
   MET_TYPE_NEW_FIELD(guess, MET_FIELD_INTEGER);
   arg_set[BIINT_LEFT] =
      MET_sym_be_set_ref(sym->kids[BIINT_LEFT], guess,
			 &arg_type[BIINT_LEFT],
			 &arg_set_type[BIINT_LEFT]);
   
   arg_set[BIINT_RIGHT] =
      MET_sym_be_set_ref(sym->kids[BIINT_RIGHT], guess,
			 &arg_type[BIINT_RIGHT],
			 &arg_set_type[BIINT_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE != arg_type[BIINT_LEFT] &&
       MET_NULL_TYPE != arg_type[BIINT_RIGHT] &&
       MET_TYPE_TYPE_FIELD == arg_type[BIINT_RIGHT]->type &&
       MET_FIELD_INTEGER == arg_type[BIINT_RIGHT]->info.field.type &&
       MET_TYPE_TYPE_FIELD == arg_type[BIINT_LEFT]->type &&
       MET_FIELD_INTEGER == arg_type[BIINT_LEFT]->info.field.type) {

      MET_TYPE_NEW_FIELD(my_result_type, MET_FIELD_INTEGER);
   }
   
   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "biint, bad argument types", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, BIINT_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;
   MET_op_reconcile_sets(arg_set, BIINT_NUM_KIDS);
   
   MAX_VERTS(how_many, arg_set[BIINT_LEFT]->info.bare.data,
	     arg_set[BIINT_RIGHT]->info.bare.data);
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, BIINT_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);
   
   if (FAILURE ==
       MET__op_biint_guts(sym->info.op.code,
			  arg_set[BIINT_LEFT]->info.bare.data,
			  arg_set[BIINT_RIGHT]->info.bare.data,
			  result_set->info.bare.data)) {
      ERROR(sym->origin, "biint, bad argument fields", SEV_FATAL);
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(BIINT_LEFT);
   COND_FREE(BIINT_RIGHT);

   return result_set;
}
