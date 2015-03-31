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
   "$Id: op_scalar.c,v 1.13 1992/06/22 17:07:11 mas Exp $";
#endif

#include "defs.h"

#define SCALAR_OPERAND		0
#define SCALAR_NUM_KIDS		1

typedef Boolean (*MET_op_guts)();

#define NULL_GUTS               ((MET_op_guts) NULL)


/*
the scalar ops are

done: sin cos tan ln sqrt exp

not done: asin acos atan sinh cosh tanh
asinh acosh atanh log lg erf gamma

or, those functions defined by power series.  right now we define most of
them on Z, R, and C.  later on, most of them can also be defined on
matrices, and maybe Quats.  the trig ones should be defined on A as soon as
we figure out what we're going to do with that.  They all take Z and R to R
and C to C.

*/

#define VARS_INTEGER             \
   int		i;		  \
   METinteger	*src_ptr;	   \
   METreal	*dst_ptr;	    \
   int		src_width, dst_width
#define VARS_REAL                \
   int		i;		  \
   METreal	*src_ptr;	   \
   METreal	*dst_ptr;	    \
   int		src_width, dst_width
#define VARS_COMPLEX             \
   int		i;		  \
   METcomplex	*src_ptr;	   \
   METcomplex	*dst_ptr;	    \
   int		src_width, dst_width

#define PREP_INTEGER {				\
   src_ptr = DATA_FIRST_VERTEX(src, METinteger); \
   dst_ptr = DATA_FIRST_VERTEX(dst, METreal);	  \
   src_width = src->verts->total_cols;		   \
   dst_width = dst->verts->total_cols;		    \
   i = DATA_NUM_VERTS(src);			     \
}
#define PREP_REAL {				\
   src_ptr = DATA_FIRST_VERTEX(src, METreal);    \
   dst_ptr = DATA_FIRST_VERTEX(dst, METreal);	  \
   src_width = src->verts->total_cols;		   \
   dst_width = dst->verts->total_cols;		    \
   i = DATA_NUM_VERTS(src);			     \
}
#define PREP_COMPLEX {				\
   src_ptr = DATA_FIRST_VERTEX(src, METcomplex); \
   dst_ptr = DATA_FIRST_VERTEX(dst, METcomplex);  \
   src_width = src->verts->total_cols;		   \
   dst_width = dst->verts->total_cols;		    \
   i = DATA_NUM_VERTS(src);			     \
}

#define NEXT_INTEGER {  \
   src_ptr += src_width; \
   dst_ptr += dst_width;  \
}
#define NEXT_REAL    NEXT_INTEGER
#define NEXT_COMPLEX NEXT_INTEGER

#define SIMPLE_INTEGER(NAME, CALL)	\
static Boolean			         \
NAME(src, dst)				  \
   METdata	*src, *dst;		   \
{					    \
   VARS_INTEGER;			     \
   MME(NAME);				      \
   PREP_INTEGER;			       \
   UNROLL4(i, *dst_ptr = (METreal)(CALL((Real)*src_ptr));\
      NEXT_INTEGER;);				  \
   return SUCCESS;				    \
}
#define SIMPLE_REAL(NAME, CALL)		 \
static Boolean				  \
NAME(src, dst)				   \
   METdata	*src, *dst;		    \
{					     \
   VARS_REAL;				      \
   MME(MET__op_sin_real);		       \
   PREP_REAL;					\
   UNROLL4(i, *dst_ptr = (METreal)(CALL((Real)*src_ptr)); \
      NEXT_REAL;);				   \
   return SUCCESS;				     \
}

SIMPLE_INTEGER(MET__op_sin_integer, SIN)
SIMPLE_REAL(MET__op_sin_real, SIN)
/*
 * should probably compute exp(t2) and exp(-t2), then add/subtract
 * them myself.
 */ /* Ack! */

static Boolean
MET__op_sin_complex(src, dst)
   METdata	*src, *dst;
{
   VARS_COMPLEX;
   Real		t1, t2;
   MME(MET__op_sin_complex);
   PREP_COMPLEX;

   UNROLL4(i, t1 = (Real) src_ptr->real;
      t2 = (Real) src_ptr->imaginary;
      dst_ptr->real      = (METreal) (SIN(t1) * COSH(t2));
      dst_ptr->imaginary = (METreal) (COS(t1) * SINH(t2));
      NEXT_COMPLEX;);

   return SUCCESS;
}

SIMPLE_INTEGER(MET__op_cos_integer, COS)
SIMPLE_REAL(MET__op_cos_real, COS)

/*
 * should probably compute exp(t2) and exp(-t2), then add/subtract
 * them myself.
 */ /* Ack! */
static Boolean
MET__op_cos_complex(src, dst)
   METdata	*src, *dst;
{
   VARS_COMPLEX;
   Real		t1, t2;
   MME(MET__op_cos_complex);
   PREP_COMPLEX;

   UNROLL4(i, t1 = (Real) src_ptr->real;
      t2 = (Real) src_ptr->imaginary;
      dst_ptr->real      = (METreal) ( COS(t1) * COSH(t2));
      dst_ptr->imaginary = (METreal) (-SIN(t1) * SINH(t2));
      NEXT_COMPLEX;);

   return SUCCESS;
}

SIMPLE_INTEGER(MET__op_tan_integer, TAN)
SIMPLE_REAL(MET__op_tan_real, TAN)

/*
  tan(u + iv) = 

                       sin(2 u)                sinh(2 v) I
                ---------------------- + ----------------------
                 cos(2 u) + cosh(2 v)     cos(2 u) + cosh(2 v)
*/
/*
 * should probably compute exp(t2) and exp(-t2), then add/subtract
 * them myself.
 */ /* Ack! */
static Boolean
MET__op_tan_complex(src, dst)
   METdata	*src, *dst;
{
   VARS_COMPLEX;
   Real		t1, t2, t3;
   MME(MET__op_tan_complex);
   PREP_COMPLEX;

   while (i--) {
      t1 = 2.0 * (Real) src_ptr->real;
      t2 = 2.0 * (Real) src_ptr->imaginary;
      t3 = COS(t1) + COSH(t2);
      dst_ptr->real      = (METreal) (SIN(t1)  / t3); 
      dst_ptr->imaginary = (METreal) (SINH(t2) / t3);
      NEXT_COMPLEX;
   }

   return SUCCESS;
}
/*
  ln(u + iv) =
                                2    2
                        1/2 ln(u  + v ) + arctan(v, u) I
*/

SIMPLE_INTEGER(MET__op_ln_integer, LOG)
SIMPLE_REAL(MET__op_ln_real, LOG)

static Boolean
MET__op_ln_complex(src, dst)
   METdata	*src, *dst;
{
   VARS_COMPLEX;
   Real		t1, t2;
   MME(MET__op_ln_complex);
   PREP_COMPLEX;

   while (i--) {
      t1 = (Real) src_ptr->real;
      t2 = (Real) src_ptr->imaginary;
      dst_ptr->real      = (METreal) (0.5 * LOG(t1 * t1 + t2 * t2));
      dst_ptr->imaginary = (METreal) ATAN2(t2, t1);
      NEXT_COMPLEX;
   }

   return SUCCESS;
}

/*
  exp(u + I * v) =

                        exp(u) cos(v) + exp(u) sin(v) I
*/

SIMPLE_INTEGER(MET__op_exp_integer, EXP)
SIMPLE_REAL(MET__op_exp_real, EXP)

static Boolean
MET__op_exp_complex(src, dst)
   METdata	*src, *dst;
{
   VARS_COMPLEX;
   Real		t1, t2;
   MME(MET__op_exp_complex);
   PREP_COMPLEX;

   UNROLL4(i, t1 = EXP((Real) src_ptr->real);
      t2 = (Real) src_ptr->imaginary;
      dst_ptr->real      = (METreal) (COS(t2) * t1);
      dst_ptr->imaginary = (METreal) (SIN(t2) * t1);
      NEXT_COMPLEX;);

   return SUCCESS;
}

/*
sqrt(u + I * v) =
          2    2 1/2         1/2                    2    2 1/2         1/2
   (1/2 (u  + v )    + 1/2 u)    + signum(v) (1/2 (u  + v )    - 1/2 u)    I
*/
SIMPLE_INTEGER(MET__op_sqrt_integer, SQRT)
SIMPLE_REAL(MET__op_sqrt_real, SQRT)

static Boolean
MET__op_sqrt_complex(src, dst)
   METdata	*src, *dst;
{
   VARS_COMPLEX;
   Real		t1, t2, t3;
   MME(MET__op_sqrt_complex);
   PREP_COMPLEX;

   while (i--) {
      t1 = (Real) src_ptr->real;
      t2 = (Real) src_ptr->imaginary;
      if (t2 > 0.0) {
	 t3 = HYPOT(t1, t2);
	 dst_ptr->real      = (METreal) (SQRT(t3+t1) * MET_CONSTANT_SQRT1_2);
	 dst_ptr->imaginary = (METreal) (SQRT(t3-t1) * MET_CONSTANT_SQRT1_2);
      } else if (t2 < 0.0) {
	 t3 = HYPOT(t1, t2);
	 dst_ptr->real      = (METreal) (SQRT(t3+t1) *   MET_CONSTANT_SQRT1_2);
	 dst_ptr->imaginary = (METreal) (SQRT(t3-t1) * - MET_CONSTANT_SQRT1_2);
      } else if (t1 < 0.0) {
	 dst_ptr->real      = (METreal) (0.0);
	 dst_ptr->imaginary = (METreal) SQRT(-t1);
      } else {
	 dst_ptr->real      = (METreal) SQRT(t1);
	 dst_ptr->imaginary = (METreal) (0.0);
      }
      NEXT_COMPLEX;
   }

   return SUCCESS;
}

SIMPLE_INTEGER(MET__op_asin_integer, ASIN)
SIMPLE_REAL(MET__op_asin_real, ASIN)
SIMPLE_INTEGER(MET__op_acos_integer, ACOS)
SIMPLE_REAL(MET__op_acos_real, ACOS)
SIMPLE_INTEGER(MET__op_atan_integer, ATAN)
SIMPLE_REAL(MET__op_atan_real, ATAN)

SIMPLE_INTEGER(MET__op_asinh_integer, ASINH)
SIMPLE_REAL(MET__op_asinh_real, ASINH)
SIMPLE_INTEGER(MET__op_acosh_integer, ACOSH)
SIMPLE_REAL(MET__op_acosh_real, ACOSH)
SIMPLE_INTEGER(MET__op_atanh_integer, ATANH)
SIMPLE_REAL(MET__op_atanh_real, ATANH)

SIMPLE_INTEGER(MET__op_sinh_integer, SINH)
SIMPLE_REAL(MET__op_sinh_real, SINH)
SIMPLE_INTEGER(MET__op_cosh_integer, COSH)
SIMPLE_REAL(MET__op_cosh_real, COSH)
SIMPLE_INTEGER(MET__op_tanh_integer, TANH)
SIMPLE_REAL(MET__op_tanh_real, TANH)


static MET_op_guts
MET_op_scalar_choose_guts(op_code, operand_type, operand_set_type,
			  result_type, result_set_type)
   int		op_code;
   METtype	*operand_type, **result_type;
   int		operand_set_type, *result_set_type;
{
   int		field;
   MME(MET_op_scalar_choose_guts);

   /*
    * make sure it's a scalar
    */
   if (MET_TYPE_TYPE_FIELD != operand_type->type ||
       MET_SET_TYPE_BLOCK == operand_set_type) {
      if ((METtype **) NULL != result_type)
	 MET_TYPE_NEW_UNKNOWN(*result_type);
      if (INULL != result_set_type)
	 *result_set_type = MET_SET_TYPE_UNKNOWN;
      return NULL_GUTS;
   }

   if (INULL != result_set_type)
      *result_set_type = operand_set_type;

   field = operand_type->info.field.type;

   if ((METtype **) NULL != result_type)
      switch (field) {
	 
       case MET_FIELD_INTEGER:
	 MET_TYPE_NEW_FIELD(*result_type, MET_FIELD_REAL);
	 break;
	 
       case MET_FIELD_REAL:
	 MET_TYPE_NEW_FIELD(*result_type, MET_FIELD_REAL);
	 break;
	 
       case MET_FIELD_COMPLEX:
	 MET_TYPE_NEW_FIELD(*result_type, MET_FIELD_COMPLEX);
	 break;
      }

   switch (op_code) {

    case MET_SYM_OP_SIN:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_sin_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_sin_real;
       case MET_FIELD_COMPLEX:    return MET__op_sin_complex;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_COS:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_cos_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_cos_real;
       case MET_FIELD_COMPLEX:    return MET__op_cos_complex;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_TAN:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_tan_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_tan_real;
       case MET_FIELD_COMPLEX:    return MET__op_tan_complex;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_ASIN:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_asin_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_asin_real;
       case MET_FIELD_COMPLEX:    return NULL_GUTS;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_ACOS:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_acos_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_acos_real;
       case MET_FIELD_COMPLEX:    return NULL_GUTS;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_ATAN:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_atan_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_atan_real;
       case MET_FIELD_COMPLEX:    return NULL_GUTS;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_SINH:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_sinh_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_sinh_real;
       case MET_FIELD_COMPLEX:    return NULL_GUTS;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_COSH:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_cosh_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_cosh_real;
       case MET_FIELD_COMPLEX:    return NULL_GUTS;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_TANH:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_tanh_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_tanh_real;
       case MET_FIELD_COMPLEX:    return NULL_GUTS;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_ASINH:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_asinh_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_asinh_real;
       case MET_FIELD_COMPLEX:    return NULL_GUTS;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_ACOSH:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_acosh_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_acosh_real;
       case MET_FIELD_COMPLEX:    return NULL_GUTS;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_ATANH:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_atanh_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_atanh_real;
       case MET_FIELD_COMPLEX:    return NULL_GUTS;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_LN:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_ln_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_ln_real;
       case MET_FIELD_COMPLEX:    return MET__op_ln_complex;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_LOG:
    case MET_SYM_OP_LG:
      break;
    case MET_SYM_OP_SQRT:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_sqrt_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_sqrt_real;
       case MET_FIELD_COMPLEX:    return MET__op_sqrt_complex;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_EXP:
      switch (field) {
       case MET_FIELD_ZTWO:       return NULL_GUTS;
       case MET_FIELD_INTEGER:    return MET__op_exp_integer;
       case MET_FIELD_ANGLE:      return NULL_GUTS;
       case MET_FIELD_REAL:       return MET__op_exp_real;
       case MET_FIELD_COMPLEX:    return MET__op_exp_complex;
       case MET_FIELD_QUATERNION: return NULL_GUTS;
       case MET_FIELD_MAP:        return NULL_GUTS;
       case MET_FIELD_SET:        return NULL_GUTS;
       default:
	 DEFAULT_ERR(field);
      }
      break;

    case MET_SYM_OP_ERF:
    case MET_SYM_OP_GAMMA:
      break;
    default:
      DEFAULT_ERR(op_code);
   }

   return NULL_GUTS;
}

METset *
MET_op_scalar(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*operand, *result;
   METtype	*operand_type, *my_result_type;
   int		operand_set_type;
   MET_op_guts	guts;
   MME(MET_op_scalar);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, SCALAR_NUM_KIDS);
#endif

   operand = MET_sym_be_set_ref(sym->kids[SCALAR_OPERAND], type,
				&operand_type, &operand_set_type);

   if (MET_NULL_TYPE == operand_type)
      result = MET_NULL_SET;
   else {
      
      guts = MET_op_scalar_choose_guts(sym->info.op.code,
				       operand_type, operand_set_type,
				       &my_result_type, set_type);
      MET_TYPE_FREE(operand_type);
      
      if (MET_NULL_SET == operand) {
	 result = MET_NULL_SET;
      } else {
	 MET_op_reconcile_sets(&operand, 1);

	 if (NULL_GUTS == guts) {
	    result = MET_op_apply_to_elements(&operand, 1,
					      MET_op_scalar,
					      sym->info.op.code,
					      type, sym);
	    if (MET_NULL_SET == result) {
	       ERROR(sym->origin, "argument is of wrong type, op is scalar",
		     SEV_FATAL);
	    }
	 } else {
	    result =
	       MET_type_to_polybare_set(my_result_type,
					DATA_NUM_VERTS(operand->info.bare.data),
					&operand, 1);
	    guts(operand->info.bare.data, result->info.bare.data);
	 }
	 MET_SET_FREE(operand);
      }
   }
   MET_TYPE_FREE(my_result_type);
   MET_set_get_type(result, result_type, set_type);
   return result;
}
