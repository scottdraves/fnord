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
   "$Id: op_relop.c,v 1.13 1992/06/22 17:07:05 mas Exp $";
#endif

#include "defs.h"

#define RELOP_NUM_KIDS		2
#define RELOP_LEFT		0
#define RELOP_RIGHT		1

/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME, TYPE1, TYPE2, DST_TYPE) \
static Boolean \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		i; \
   TYPE1	*src1; \
   TYPE2	*src2; \
   DST_TYPE	*dst; \
   Boolean	error = SUCCESS; \
   TEMPS \
   int		src1_width, src2_width, dst_width; \
   MME(NAME); \
 \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   src1 = DATA_FIRST_VERTEX(arg1, TYPE1); \
   src2 = DATA_FIRST_VERTEX(arg2, TYPE2); \
   dst  = DATA_FIRST_VERTEX(result,  DST_TYPE); \
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

/*
 * eeq = epsilon equals
 * xeq = exact equals
 *  gt = greater than
 *  lt = less than
 * gte = greater or equal
 * lte = less or equal
 */
#define TEMPS
#define BODY *dst = (METztwo)(*src1 == *src2)
TEMPLATE(MET__op_relop_xeq_z_z, METztwo,    METztwo,    METztwo)
TEMPLATE(MET__op_relop_xeq_i_i, METinteger, METinteger, METztwo)
TEMPLATE(MET__op_relop_xeq_i_r, METinteger, METreal,    METztwo)
TEMPLATE(MET__op_relop_xeq_r_r, METreal   , METreal,    METztwo)
#undef BODY
#undef TEMPS
#define TEMPS
#define BODY *dst = (METztwo)(*src1 != *src2)
TEMPLATE(MET__op_relop_neq_z_z, METztwo,    METztwo,    METztwo)
TEMPLATE(MET__op_relop_neq_i_i, METinteger, METinteger, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS
#define BODY *dst = (METztwo)(*src1 > *src2)
TEMPLATE(MET__op_relop_gt_i_i, METinteger, METinteger, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS
#define BODY *dst = (METztwo)(*src1 < *src2)
TEMPLATE(MET__op_relop_lt_i_i, METinteger, METinteger, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS
#define BODY *dst = (METztwo)(*src1 >= *src2)
TEMPLATE(MET__op_relop_gte_i_i, METinteger, METinteger, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS
#define BODY *dst = (METztwo)(*src1 <= *src2)
TEMPLATE(MET__op_relop_lte_i_i, METinteger, METinteger, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS Real eps = (MET_defaults->relop_epsilon.epsilon) ;
#define BODY *dst = (METztwo)(FABS(*src1 - *src2) <= eps)
TEMPLATE(MET__op_relop_eeq_i_r, METinteger, METreal, METztwo)
TEMPLATE(MET__op_relop_eeq_r_r, METreal   , METreal, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS Real eps = (MET_defaults->relop_epsilon.epsilon) ;
#define BODY *dst = (METztwo)(FABS(*src1 - *src2) > eps)
TEMPLATE(MET__op_relop_neq_i_r, METinteger, METreal, METztwo)
TEMPLATE(MET__op_relop_neq_r_r, METreal   , METreal, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS 
#define BODY *dst = (METztwo)(*src1 > *src2)
TEMPLATE(MET__op_relop_gt_i_r, METinteger, METreal, METztwo)
TEMPLATE(MET__op_relop_gt_r_r, METreal   , METreal, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS
#define BODY *dst = (METztwo)(*src1 < *src2)
TEMPLATE(MET__op_relop_lt_i_r, METinteger, METreal, METztwo)
TEMPLATE(MET__op_relop_lt_r_r, METreal   , METreal, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS Real eps = (MET_defaults->relop_epsilon.epsilon) ;
#define BODY *dst = (METztwo)((*src2 - *src1) < eps)
TEMPLATE(MET__op_relop_gte_i_r, METinteger, METreal, METztwo)
TEMPLATE(MET__op_relop_gte_r_r, METreal   , METreal, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS Real eps = (MET_defaults->relop_epsilon.epsilon) ;
#define BODY *dst = (METztwo)((*src1 - *src2) < eps)
TEMPLATE(MET__op_relop_lte_i_r, METinteger, METreal, METztwo)
TEMPLATE(MET__op_relop_lte_r_r, METreal   , METreal, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS
#define BODY *dst = (METztwo)(*src1 == src2->real && \
                                0.0 == src2->imaginary)
TEMPLATE(MET__op_relop_xeq_i_c, METinteger, METcomplex, METztwo)
TEMPLATE(MET__op_relop_xeq_r_c, METreal   , METcomplex, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS Real eps = (MET_defaults->relop_epsilon.epsilon) ;
#define BODY *dst = (METztwo)(FABS(*src1 - src2->real) <= eps && \
                                 FABS(src2->imaginary) <= eps)
TEMPLATE(MET__op_relop_eeq_i_c, METinteger, METcomplex, METztwo)
TEMPLATE(MET__op_relop_eeq_r_c, METreal   , METcomplex, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS Real eps = (MET_defaults->relop_epsilon.epsilon) ;
#define BODY *dst = (METztwo)(FABS(*src1 - src2->real) > eps && \
                                 FABS(src2->imaginary) > eps)
TEMPLATE(MET__op_relop_neq_i_c, METinteger, METcomplex, METztwo)
TEMPLATE(MET__op_relop_neq_r_c, METreal   , METcomplex, METztwo)
#undef BODY
#undef TEMPS


#define TEMPS
#define BODY *dst = (METztwo)(src1->real == src2->real && \
                         src1->imaginary == src2->imaginary)
TEMPLATE(MET__op_relop_xeq_c_c, METcomplex, METcomplex, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS Real eps = (MET_defaults->relop_epsilon.epsilon) ;
#define BODY *dst = (METztwo)(FABS(src1->real - src2->real) <= eps && \
                    FABS(src1->imaginary - src2->imaginary) <= eps)
TEMPLATE(MET__op_relop_eeq_c_c, METcomplex, METcomplex, METztwo)
#undef BODY
#undef TEMPS
#define TEMPS Real eps = (MET_defaults->relop_epsilon.epsilon) ;
#define BODY *dst = (METztwo)(FABS(src1->real - src2->real) > eps && \
                    FABS(src1->imaginary - src2->imaginary) > eps)
TEMPLATE(MET__op_relop_neq_c_c, METcomplex, METcomplex, METztwo)
#undef BODY
#undef TEMPS


static Boolean
MET__op_relop_guts(opcode, arg1, arg2, result)
   int		opcode;
   METdata	*arg1, *arg2, *result;
{
   Boolean	is_equals;
   Boolean	rev = NO; /* YES -> call f with args reversed */
   int		a2t = arg2->verts->type;
   Boolean	(*f)() = MET_op_guts_failure;
   MME(MET__op_relop_guts);

   switch (opcode) {
    case MET_SYM_OP_EPSILON_EQUAL:
    case MET_SYM_OP_EXACT_EQUAL:
    case MET_SYM_OP_NOTEQUAL:
      is_equals = YES;
      break;
      
    case MET_SYM_OP_GREATER:
    case MET_SYM_OP_LESS:
    case MET_SYM_OP_GREATEROREQUAL:
    case MET_SYM_OP_LESSOREQUAL:
      is_equals = NO;
      break;
      
    default:
      DEFAULT_ERR(opcode);
   }

   switch (arg1->verts->type) {

      /* top level */
    case MET_FIELD_ZTWO:
      if (NO == is_equals || MET_FIELD_ZTWO != arg2->verts->type)
	 break;
      switch (opcode) {
       case MET_SYM_OP_EPSILON_EQUAL:
       case MET_SYM_OP_EXACT_EQUAL:
	 f = MET__op_relop_xeq_z_z; break;
       case MET_SYM_OP_NOTEQUAL:
	 f = MET__op_relop_neq_z_z; break;
      }
      break;

      /* top level */
    case MET_FIELD_INTEGER:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 switch (opcode) {
	  case MET_SYM_OP_EPSILON_EQUAL:
	  case MET_SYM_OP_EXACT_EQUAL:
	    f = MET__op_relop_xeq_i_i; break;
	  case MET_SYM_OP_NOTEQUAL:
	    f = MET__op_relop_neq_i_i; break;
	  case MET_SYM_OP_GREATER:
	    f = MET__op_relop_gt_i_i; break;
	  case MET_SYM_OP_LESS:
	    f = MET__op_relop_lt_i_i; break;
	  case MET_SYM_OP_GREATEROREQUAL:
	    f = MET__op_relop_gte_i_i; break;
	  case MET_SYM_OP_LESSOREQUAL:
	    f = MET__op_relop_lte_i_i; break;
	 }
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 switch (opcode) {
	  case MET_SYM_OP_EPSILON_EQUAL:
	    f = MET__op_relop_eeq_i_r; break;
	  case MET_SYM_OP_EXACT_EQUAL:
	    f = MET__op_relop_xeq_i_r; break;
	  case MET_SYM_OP_NOTEQUAL:
	    f = MET__op_relop_neq_i_r; break;
	  case MET_SYM_OP_GREATER:
	    f = MET__op_relop_gt_i_r; break;
	  case MET_SYM_OP_LESS:
	    f = MET__op_relop_lt_i_r; break;
	  case MET_SYM_OP_GREATEROREQUAL:
	    f = MET__op_relop_gte_i_r; break;
	  case MET_SYM_OP_LESSOREQUAL:
	    f = MET__op_relop_lte_i_r; break;
	 }
	 break;
       case MET_FIELD_COMPLEX:
	 if (NO == is_equals)
	    break;
	 switch (opcode) {
	  case MET_SYM_OP_EPSILON_EQUAL:
	    f = MET__op_relop_eeq_i_c; break;
	  case MET_SYM_OP_EXACT_EQUAL:
	    f = MET__op_relop_xeq_i_c; break;
	  case MET_SYM_OP_NOTEQUAL:
	    f = MET__op_relop_neq_i_c; break;
	 }
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;

      /* top level */
    case MET_FIELD_ANGLE:
      break;

      /* top level */
    case MET_FIELD_REAL:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 switch (opcode) {
	  case MET_SYM_OP_EPSILON_EQUAL:
	    rev = YES ; f = MET__op_relop_eeq_i_r; break;
	  case MET_SYM_OP_EXACT_EQUAL:
	    rev = YES ; f = MET__op_relop_xeq_i_r; break;
	  case MET_SYM_OP_NOTEQUAL:
	    rev = YES ; f = MET__op_relop_neq_i_r; break;
	  case MET_SYM_OP_GREATER:
	    rev = YES ; f = MET__op_relop_lt_i_r; break;
	  case MET_SYM_OP_LESS:
	    rev = YES ; f = MET__op_relop_gt_i_r; break;
	  case MET_SYM_OP_GREATEROREQUAL:
	    rev = YES ; f = MET__op_relop_lte_i_r; break;
	  case MET_SYM_OP_LESSOREQUAL:
	    rev = YES ; f = MET__op_relop_gte_i_r; break;
	 }
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 switch (opcode) {
	  case MET_SYM_OP_EPSILON_EQUAL:
	    f = MET__op_relop_eeq_r_r; break;
	  case MET_SYM_OP_EXACT_EQUAL:
	    f = MET__op_relop_xeq_r_r; break;
	  case MET_SYM_OP_NOTEQUAL:
	    f = MET__op_relop_neq_r_r; break;
	  case MET_SYM_OP_GREATER:
	    f = MET__op_relop_gt_r_r; break;
	  case MET_SYM_OP_LESS:
	    f = MET__op_relop_lt_r_r; break;
	  case MET_SYM_OP_GREATEROREQUAL:
	    f = MET__op_relop_gte_r_r; break;
	  case MET_SYM_OP_LESSOREQUAL:
	    f = MET__op_relop_lte_r_r; break;
	 }
	 break;
       case MET_FIELD_COMPLEX:
	 if (NO == is_equals)
	    break;
	 switch (opcode) {
	  case MET_SYM_OP_EPSILON_EQUAL:
	    f = MET__op_relop_eeq_r_c; break;
	  case MET_SYM_OP_EXACT_EQUAL:
	    f = MET__op_relop_xeq_r_c; break;
	  case MET_SYM_OP_NOTEQUAL:
	    f = MET__op_relop_neq_r_c; break;
	 }
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;

      /* top level */
    case MET_FIELD_COMPLEX:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 if (NO == is_equals)
	    break;
	 switch (opcode) {
	  case MET_SYM_OP_EPSILON_EQUAL:
	    rev = YES ; f = MET__op_relop_eeq_i_c; break;
	  case MET_SYM_OP_EXACT_EQUAL:
	    rev = YES ; f = MET__op_relop_xeq_i_c; break;
	  case MET_SYM_OP_NOTEQUAL:
	    rev = YES ; f = MET__op_relop_neq_i_c; break;
	 }
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 if (NO == is_equals)
	    break;
	 switch (opcode) {
	  case MET_SYM_OP_EPSILON_EQUAL:
	    rev = YES ; f = MET__op_relop_eeq_r_c; break;
	  case MET_SYM_OP_EXACT_EQUAL:
	    rev = YES ; f = MET__op_relop_xeq_r_c; break;
	  case MET_SYM_OP_NOTEQUAL:
	    rev = YES ; f = MET__op_relop_neq_r_c; break;
	 }
	 break;
       case MET_FIELD_COMPLEX:
	 if (NO == is_equals)
	    break;
	 switch (opcode) {
	  case MET_SYM_OP_EPSILON_EQUAL:
	    f = MET__op_relop_eeq_c_c; break;
	  case MET_SYM_OP_EXACT_EQUAL:
	    f = MET__op_relop_xeq_c_c; break;
	  case MET_SYM_OP_NOTEQUAL:
	    f = MET__op_relop_neq_c_c; break;
	 }
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;

      /* top level */
    case MET_FIELD_QUATERNION:
    case MET_FIELD_MAP:
    case MET_FIELD_SET:
      break;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   if (rev)
      return (f)(arg2, arg1, result);

   return (f)(arg1, arg2, result);
}

METset *
MET_op_relop(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[RELOP_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[RELOP_NUM_KIDS];
   METtype	*arg_type[RELOP_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many;
   MME(MET_op_relop);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, RELOP_NUM_KIDS);
#endif
   MET_TYPE_NEW_FIELD(guess, MET_FIELD_REAL);
   arg_set[RELOP_LEFT] =
      MET_sym_be_set_ref(sym->kids[RELOP_LEFT], guess,
			 &arg_type[RELOP_LEFT],
			 &arg_set_type[RELOP_LEFT]);
   
   arg_set[RELOP_RIGHT] =
      MET_sym_be_set_ref(sym->kids[RELOP_RIGHT], guess,
			 &arg_type[RELOP_RIGHT],
			 &arg_set_type[RELOP_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE != arg_type[RELOP_LEFT] &&
       MET_NULL_TYPE != arg_type[RELOP_RIGHT] &&
       ((MET_TYPE_TYPE_FIELD == arg_type[RELOP_RIGHT]->type &&
	 MET_TYPE_TYPE_FIELD == arg_type[RELOP_LEFT]->type) ||
	(MET_TYPE_TYPE_SET == arg_type[RELOP_RIGHT]->type &&
	 MET_TYPE_TYPE_SET == arg_type[RELOP_LEFT]->type))) {
      MET_TYPE_NEW_FIELD(my_result_type, MET_FIELD_ZTWO);
   }
   
   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "arguments to relop of of wrong type", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, RELOP_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;
   MET_op_reconcile_sets(arg_set, RELOP_NUM_KIDS);


   if (MET_SET_TYPE_SYM_CONST == arg_set[RELOP_LEFT]->type &&
       MET_SET_TYPE_SYM_CONST == arg_set[RELOP_RIGHT]->type) {
      METztwo t =
	 (METztwo) SYM_EQ(arg_set[RELOP_LEFT]->info.sym_const.sym,
			  arg_set[RELOP_RIGHT]->info.sym_const.sym);
      switch (sym->info.op.code) {
       case MET_SYM_OP_EXACT_EQUAL:
       case MET_SYM_OP_EPSILON_EQUAL:
	 break;
       case MET_SYM_OP_NOTEQUAL:
	 t = !t;
	 break;
       default:
	 ERROR(sym->origin, "only equal and nequal apply to symbols",
	       SEV_FATAL);
	 MET_TYPE_FREE(my_result_type);
	 goto bail1;
      }
      result_set = MET_type_to_polybare_set(my_result_type, 1,
					    &MET_set_zero, 1);
      *DATA_FIRST_VERTEX(result_set->info.bare.data, METztwo) = t;
      MET_TYPE_FREE(my_result_type);
   } else if (MET_SET_TYPE_BARE == arg_set[RELOP_LEFT]->type &&
	      MET_SET_TYPE_BARE == arg_set[RELOP_RIGHT]->type) {
      
      MAX_VERTS(how_many, arg_set[RELOP_LEFT]->info.bare.data,
		arg_set[RELOP_RIGHT]->info.bare.data);
      
      result_set = MET_type_to_polybare_set(my_result_type, how_many,
					    arg_set, RELOP_NUM_KIDS);
      MET_TYPE_FREE(my_result_type);
      
      if (FAILURE ==
	  MET__op_relop_guts(sym->info.op.code,
			     arg_set[RELOP_LEFT]->info.bare.data,
			     arg_set[RELOP_RIGHT]->info.bare.data,
			     result_set->info.bare.data)) {
	 MET_SET_FREE(result_set);
	 result_set = MET_NULL_SET;
      }
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(RELOP_LEFT);
   COND_FREE(RELOP_RIGHT);

   return result_set;
}
