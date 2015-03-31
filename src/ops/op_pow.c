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
   "$Id: op_pow.c,v 1.8 1992/06/22 17:06:51 mas Exp $";
#endif

/*
 * POW(r, 0.5 * s) * EXP(-t * u) should be replaced by
 * EXP(s * t * u * LOG(r) * -0.5);
 */

#include "defs.h"

#define POW_NUM_KIDS		2
#define POW_LEFT		0
#define POW_RIGHT		1

#define SIMPLE(F) (MET__op_pow_field_simple[F])
int MET__op_pow_field_simple[MET_FIELD_MAX] = {
   3, 0, 3, 1, 2, 3, 3, 3, 3};

#define RESULT_FIELD_REAL(F,G) \
   MET__op_pow_type_table_real[SIMPLE(F)][SIMPLE(G)]
#define RESULT_FIELD_COMPLEX(F,G) \
   MET__op_pow_type_table_complex[SIMPLE(F)][SIMPLE(G)]

int MET__op_pow_type_table_real[4][4] = {
{MET_FIELD_REAL,    MET_FIELD_REAL,    MET_FIELD_COMPLEX, MET_FIELD_UNKNOWN},
{MET_FIELD_REAL,    MET_FIELD_REAL,    MET_FIELD_COMPLEX, MET_FIELD_UNKNOWN},
{MET_FIELD_COMPLEX, MET_FIELD_COMPLEX, MET_FIELD_COMPLEX, MET_FIELD_UNKNOWN},
{MET_FIELD_UNKNOWN, MET_FIELD_UNKNOWN, MET_FIELD_UNKNOWN, MET_FIELD_UNKNOWN},
};

int MET__op_pow_type_table_complex[4][4] = {
{MET_FIELD_REAL,    MET_FIELD_COMPLEX, MET_FIELD_COMPLEX, MET_FIELD_UNKNOWN},
{MET_FIELD_REAL,    MET_FIELD_COMPLEX, MET_FIELD_COMPLEX, MET_FIELD_UNKNOWN},
{MET_FIELD_COMPLEX, MET_FIELD_COMPLEX, MET_FIELD_COMPLEX, MET_FIELD_UNKNOWN},
{MET_FIELD_UNKNOWN, MET_FIELD_UNKNOWN, MET_FIELD_UNKNOWN, MET_FIELD_UNKNOWN},
};

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
   int		src1_width, src2_width, dst_width; \
   TEMPS \
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
}

/*   u + vi
    b         = 

 u           u
b  cos(B) + b  sin(B) I	               for b > 0

A cos(pi u + B) + A sin(pi u + B) I    for b < 0

       u  -v pi
A = |b|  e

B = v ln |b|

*/
#define BODY				 \
   t = *src1;				  \
   u = src2->real;			   \
   v = src2->imaginary;			    \
   if (t > 0.0) {			     \
      r = v * LOG(t);			      \
      s = POW(t, u);			       \
   } else {					\
      t = FABS(t);				 \
      r = MET_CONSTANT_PI * u + v * LOG(t);       \
      s = POW(t, u) * EXP( - MET_CONSTANT_PI * v); \
   }						    \
   dst->real      = s * COS(r);			     \
   dst->imaginary = s * SIN(r);
#define TEMPS Real r, s, t, u, v;
TEMPLATE(MET__op_pow_guts_r_c, METreal,    METcomplex, METcomplex)
TEMPLATE(MET__op_pow_guts_i_c, METinteger, METcomplex, METcomplex)
#undef BODY
#undef TEMPS
/*
           b
   (u + vi)   =

A cos(b theta) + A sin(b theta) I

where
       2    2 (1/2 b)
A =  (u  + v )
and
theta = arctan(v, u)
*/
#define BODY 		    \
   u = src1->real;	     \
   v = src1->imaginary;	      \
   s = *src2;		       \
   r = POW(u*u + v*v, 0.5 * s); \
   t = s * ATAN2(v, u);		 \
   dst->real      = r * COS(t);   \
   dst->imaginary = r * SIN(t);
#define TEMPS Real u, v, s, r, t;
TEMPLATE(MET__op_pow_guts_c_r, METcomplex, METreal,    METcomplex)
TEMPLATE(MET__op_pow_guts_c_i, METcomplex, METinteger, METcomplex)
#undef BODY
#undef TEMPS

#define BODY *dst = (METreal) POW((Real)*src1, (Real)*src2);
#define TEMPS
TEMPLATE(MET__op_pow_guts_r_r_r, METreal,    METreal,    METreal)
TEMPLATE(MET__op_pow_guts_i_i,   METinteger, METinteger, METreal)
TEMPLATE(MET__op_pow_guts_r_i,   METreal,    METinteger, METreal)
TEMPLATE(MET__op_pow_guts_i_r_r, METinteger, METreal,    METreal)
#undef TEMPS
#undef BODY

#define BODY                \
   t = *src1;                \
   s = *src2;                 \
   if (t < 0.0) {              \
      t = POW(-t, s);           \
      s *= MET_CONSTANT_PI;      \
      dst->real = t * COS(s);     \
      dst->imaginary = t * SIN(s); \
   } else {                         \
      dst->real = POW(t, s);         \
      dst->imaginary = 0.0;           \
   }
#define TEMPS Real s, t;
TEMPLATE(MET__op_pow_guts_r_r_c, METreal,    METreal, METcomplex)
TEMPLATE(MET__op_pow_guts_i_r_c, METinteger, METreal, METcomplex)
#undef BODY
#undef TEMPS

/*
 (u+v*I)^(a+b*I) =

C cos(B) + C sin(B) I

where

       2    2
rr = (u  + v )
theta = arctan(v, u)

and
      (1/2 a)
A = rr
B = a theta + 1/2 b ln(rr)
C = A exp(- b theta)
*/
#define BODY			\
   u = src1->real;		 \
   v = src1->imaginary;		  \
   s = src2->real;		   \
   t = src2->imaginary;		    \
   r = u*u + v*v;		     \
   u = ATAN2(v, u);		      \
   v = POW(r, 0.5 * s) * EXP(-t * u);  \
   s = s * u + 0.5 * t * LOG(r);        \
   dst->real      = v * COS(s);		 \
   dst->imaginary = v * SIN(s);
#define TEMPS Real u, v, s, t, r;
TEMPLATE(MET__op_pow_guts_c_c, METcomplex, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

static Boolean
MET__op_pow_guts(complex_mode, arg1, arg2, result)
   Boolean	complex_mode;
   METdata	*arg1, *arg2, *result;
{
   int		a2t = arg2->verts->type;
   Boolean	(*f)() = MET_op_guts_failure;
   MME(MET__op_pow_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:
      break;
    case MET_FIELD_INTEGER:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_pow_guts_i_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = (complex_mode == ON) ?
	    MET__op_pow_guts_i_r_c : MET__op_pow_guts_i_r_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_pow_guts_i_c;
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
	 f = MET__op_pow_guts_r_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = (complex_mode == ON) ?
	    MET__op_pow_guts_r_r_c : MET__op_pow_guts_r_r_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_pow_guts_r_c;
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
	 f = MET__op_pow_guts_c_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_pow_guts_c_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_pow_guts_c_c;
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

   return (f)(arg1, arg2, result);
}



METset *
MET_op_pow(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[POW_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[POW_NUM_KIDS];
   METtype	*arg_type[POW_NUM_KIDS];
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many, result_field;
   Boolean	result;
   MME(MET_op_pow);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, POW_NUM_KIDS);
#endif
   arg_set[POW_LEFT] =
      MET_sym_be_set_ref(sym->kids[POW_LEFT], type,
			 &arg_type[POW_LEFT],
			 &arg_set_type[POW_LEFT]);

   arg_set[POW_RIGHT] =
      MET_sym_be_set_ref(sym->kids[POW_RIGHT], type,
			 &arg_type[POW_RIGHT],
			 &arg_set_type[POW_RIGHT]);

   if (MET_NULL_TYPE == arg_type[POW_LEFT] ||
       MET_NULL_TYPE == arg_type[POW_RIGHT])
      goto bail1;
   
   if (arg_type[POW_LEFT]->type == MET_TYPE_TYPE_FIELD &&
       arg_type[POW_RIGHT]->type == MET_TYPE_TYPE_FIELD) {
      result_field = (YES == MET_defaults->complex_flags.pow) ?
	 RESULT_FIELD_COMPLEX(arg_type[POW_LEFT]->info.field.type,
			      arg_type[POW_RIGHT]->info.field.type) :
	 RESULT_FIELD_REAL(arg_type[POW_LEFT]->info.field.type,
			   arg_type[POW_RIGHT]->info.field.type);
      if (MET_FIELD_UNKNOWN != result_field)
	 MET_TYPE_NEW_FIELD(my_result_type, result_field);
   }
   
   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "op pow, arguments of wrong type", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, POW_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE)) {
      goto bail2;
   }
   MET_op_reconcile_sets(arg_set, POW_NUM_KIDS);
   
   MAX_VERTS(how_many, arg_set[POW_LEFT]->info.bare.data,
	     arg_set[POW_RIGHT]->info.bare.data);
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, POW_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);

   result =
      MET__op_pow_guts(MET_defaults->complex_flags.pow,
		       arg_set[POW_LEFT]->info.bare.data,
		       arg_set[POW_RIGHT]->info.bare.data,
		       result_set->info.bare.data);

   if (FAILURE == result) {
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(POW_LEFT);
   COND_FREE(POW_RIGHT);

   return result_set;
}
