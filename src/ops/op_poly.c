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
   "$Id: op_poly.c,v 1.9 1992/06/22 17:06:48 mas Exp $";
#endif

#include "defs.h"

#define POLY_NUM_KIDS		2
#define POLY_LEFT		0
#define POLY_RIGHT		1

/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME, TYPE1, TYPE2, DST_TYPE) \
static Boolean \
NAME(arg1, arg2, result, bottom_dim) \
   METdata	*arg1, *arg2, *result; \
   int		bottom_dim; \
{ \
   int		i, j, k; \
   TYPE1	*src1; \
   TYPE2	*src2; \
   DST_TYPE	*dst; \
   int		src1_width, src2_width, dst_width; \
   TEMPS \
   MME(NAME); \
 \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   for (k = 0; k < arg2->num_cols; k += bottom_dim) { \
      j = bottom_dim - 1; \
      MAX_VERTS(i, arg1, arg2); \
      dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + k/bottom_dim; \
      src2 = DATA_FIRST_VERTEX(arg2, TYPE2) + k + j; \
       \
      while(i--) { \
	 INIT_BODY; \
	 dst += dst_width; \
	 src2 += src2_width; \
      } \
      j--; \
      while (j >= 0) { \
	 src1 = DATA_FIRST_VERTEX(arg1, TYPE1); \
	 src2 = DATA_FIRST_VERTEX(arg2, TYPE2) + k + j; \
	 dst  = DATA_FIRST_VERTEX(result, DST_TYPE) + k/bottom_dim; \
	 MAX_VERTS(i, arg1, arg2); \
	  \
	 while(i--) { \
	    BODY; \
	    dst += dst_width; \
	    src1 += src1_width; \
	    src2 += src2_width; \
	 } \
	 j--; \
      } \
   } \
   return SUCCESS; \
} \


#define TEMPS
#define INIT_BODY   *dst = *src2;
#define BODY *dst *= *src1; *dst += *src2;
TEMPLATE(MET__op_poly_guts_i_i, METinteger, METinteger, METinteger)
TEMPLATE(MET__op_poly_guts_i_r, METinteger, METreal,    METreal)
TEMPLATE(MET__op_poly_guts_r_i, METreal,    METinteger, METreal)
TEMPLATE(MET__op_poly_guts_r_r, METreal,    METreal,    METreal)
#undef BODY
#undef INIT_BODY
#undef TEMPS

#define TEMPS Real t;
#define INIT_BODY dst->real = (METreal) *src2; \
                  dst->imaginary = (METreal) 0.0;
#define BODY \
   t = dst->real; \
   dst->real = t * src1->real - dst->imaginary * src1->imaginary + *src2; \
   dst->imaginary = t * src1->imaginary + dst->imaginary * src1->real;
TEMPLATE(MET__op_poly_guts_c_i, METcomplex, METinteger, METcomplex)
TEMPLATE(MET__op_poly_guts_c_r, METcomplex, METreal,    METcomplex)
#undef BODY
#undef INIT_BODY
#undef TEMPS

#define TEMPS
#define INIT_BODY *dst = *src2;
#define BODY dst->real = (dst->real * *src1) + src2->real; \
             dst->imaginary = (dst->imaginary * *src1) + src2->imaginary;
TEMPLATE(MET__op_poly_guts_i_c, METinteger, METcomplex, METcomplex)
TEMPLATE(MET__op_poly_guts_r_c, METreal,    METcomplex, METcomplex)
#undef BODY
#undef INIT_BODY
#undef TEMPS

#define TEMPS Real t;
#define INIT_BODY *dst = *src2;
#define BODY \
   t = dst->real; \
   dst->real = t * src1->real - dst->imaginary * src1->imaginary + src2->real; \
   dst->imaginary = t * src1->imaginary + \
                    dst->imaginary * src1->real + src2->imaginary;
TEMPLATE(MET__op_poly_guts_c_c, METcomplex, METcomplex, METcomplex)
#undef BODY
#undef INIT_BODY
#undef TEMPS

/*
 * arg1 is scalar, arg2 is vector
 */
static Boolean
MET__op_poly_guts(arg1, arg2, result, bottom_dim)
   METdata	*arg1, *arg2, *result;
   int		bottom_dim;
{
   int		a2t = arg2->verts->type;
   Boolean	(*f)() = MET_op_guts_failure;
   MME(MET__op_poly_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:
      break;
      
    case MET_FIELD_INTEGER:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_poly_guts_i_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_poly_guts_i_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_poly_guts_i_c;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_ANGLE:
      break;
    case MET_FIELD_REAL:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_poly_guts_r_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_poly_guts_r_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_poly_guts_r_c;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_COMPLEX:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_poly_guts_c_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_poly_guts_c_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_poly_guts_c_c;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_QUATERNION:
    case MET_FIELD_MAP:
    case MET_FIELD_SET:
      break;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   return (f)(arg1, arg2, result, bottom_dim);
}

static int
MET__op_poly_field(type)
   METtype	*type;
{
   int		t;
   MME(MET__op_poly_field);

   if (MET_TYPE_TYPE_FIELD == type->type)
      return -2;
   if (MET_TYPE_TYPE_VECTOR == type->type) {
      t = MET__op_poly_field(type->info.vector.of);
      if (-2 == t)
	 return type->info.vector.dimension;
      else
	 return t;
   }
   return -1;
}

static METtype *
MET__op_poly_type(type, base_type)
   METtype	*type, *base_type;
{
   METtype	*s, *t;
   MME(MET__op_poly_type);

   if (MET_TYPE_TYPE_VECTOR == type->type) {
      if (MET_TYPE_TYPE_FIELD == (t = type->info.vector.of)->type) {
	 if (SUCCESS == MET_type_cast(t, base_type, &s,
				      TYPE_CAST_MAKE_BOGUS)) 
	    return s;
	 MET_TYPE_FREE(s);
	 return MET_NULL_TYPE;
      } else {
	 s = MET__op_poly_type(t, base_type);
	 if (MET_NULL_TYPE != s) {
	    MET_TYPE_NEW_VECTOR(t, s,
				type->info.vector.dimension,
				type->info.vector.is_row);
	    MET_TYPE_FREE(s);
	    return t;
	 }
      }
   }
   return MET_NULL_TYPE;
}

/*
 * evaluate polynomials with horner's (sp?) rule:
 *
 *    3      2      1      0           2      1      0
 * a x  + b x  + c x  + d x   = x (a x   + b x  + c x  ) + d
 *
 * 			      = x ( x (a x + b) + c ) + d
 *
 * poly(x, v) = sum (v_i * x**i) or poly(x, [a, b]) = a + xb
 */

METset *
MET_op_poly(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[POLY_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[POLY_NUM_KIDS];
   METtype	*arg_type[POLY_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many, bottom_dim;
   MME(MET_op_poly);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, POLY_NUM_KIDS);
#endif

   MET_TYPE_NEW_UNKNOWN(guess);
   if (MET_NULL_TYPE != type) {
      MET_TYPE_FREE(guess);
      MET_TYPE_COPY(guess, type);
   }
   arg_set[POLY_LEFT] = MET_sym_be_set_ref(sym->kids[POLY_LEFT], guess,
					  &arg_type[POLY_LEFT],
					  &arg_set_type[POLY_LEFT]);

   if (MET_NULL_TYPE != type) {
      MET_TYPE_FREE(guess);
      MET_TYPE_NEW_VECTOR(guess, type, TYPE_DIMENSION_UNKNOWN, YES);
   }
   arg_set[POLY_RIGHT] = MET_sym_be_set_ref(sym->kids[POLY_RIGHT], guess,
					   &arg_type[POLY_RIGHT],
					   &arg_set_type[POLY_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[POLY_LEFT] ||
       MET_NULL_TYPE == arg_type[POLY_RIGHT])
      goto bail2;
   
   /*
    * make sure left is scalar and right is vector
    */
   if (arg_type[POLY_LEFT]->type == MET_TYPE_TYPE_FIELD &&
       0 < (bottom_dim = MET__op_poly_field(arg_type[POLY_RIGHT]))) {
      my_result_type = MET__op_poly_type(arg_type[POLY_RIGHT],
					 arg_type[POLY_LEFT]);
   }

   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "poly, bad argument types", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, POLY_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;
   MET_op_reconcile_sets(arg_set, POLY_NUM_KIDS);

   MAX_VERTS(how_many, arg_set[POLY_LEFT]->info.bare.data,
	     arg_set[POLY_RIGHT]->info.bare.data);
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, POLY_NUM_KIDS);
   
   MET_TYPE_FREE(my_result_type);
   
   if (FAILURE == MET__op_poly_guts(arg_set[POLY_LEFT]->info.bare.data,
				    arg_set[POLY_RIGHT]->info.bare.data,
				    result_set->info.bare.data,
				    bottom_dim)) {
      ERROR(sym->origin, "poly, bad argument fields", SEV_FATAL);
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }
   

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(POLY_LEFT);
   COND_FREE(POLY_RIGHT);

   return result_set;
}
