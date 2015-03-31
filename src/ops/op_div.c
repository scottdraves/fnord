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
   "$Id: op_div.c,v 1.10 1992/06/22 17:06:13 mas Exp $";
#endif

#include "defs.h"

#define DIV_NUM_KIDS		2
#define DIV_LEFT		0
#define DIV_RIGHT		1


#define TEMPLATE(NAME, TYPE1, TYPE2, DST_TYPE) \
static Boolean \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		i, j, length; \
   TYPE1	*src1; \
   TYPE2	*src2; \
   DST_TYPE	*dst; \
   int		src1_width, src2_width, dst_width; \
   TEMPS \
   MME(NAME); \
 \
   length = result->num_cols; \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   for (j = 0; j < length; j++) { \
      src1 = DATA_FIRST_VERTEX(arg1, TYPE1) + j; \
      src2 = DATA_FIRST_VERTEX(arg2, TYPE2); \
      dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + j; \
      MAX_VERTS(i, arg1, arg2); \
      UNROLL4(i, BODY; \
	 dst += dst_width; \
	 src1 += src1_width; \
	 src2 += src2_width;); \
   } \
   return SUCCESS; \
}

#define TEMPS
#define BODY *dst = (METreal) ((Real)*src1 / (Real)*src2)
TEMPLATE(MET__op_div_guts_scalar_i_i, METinteger, METinteger, METreal)
TEMPLATE(MET__op_div_guts_scalar_i_r, METinteger, METreal,    METreal)
TEMPLATE(MET__op_div_guts_scalar_r_i, METreal,    METinteger, METreal)
TEMPLATE(MET__op_div_guts_scalar_r_r, METreal,    METreal,    METreal)
#undef BODY
#undef TEMPS


#define TEMPS Real t;
#define BODY t = 1.0 / ((Real) src2->real * src2->real + \
                 (Real) src2->imaginary * src2->imaginary); \
      dst->real = (METreal) ((Real)*src1 * src2->real * t); \
      dst->imaginary = (METreal)((Real)*src1 * src2->imaginary * -t);
TEMPLATE(MET__op_div_guts_scalar_i_c, METinteger, METcomplex, METcomplex)
TEMPLATE(MET__op_div_guts_scalar_r_c, METreal, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY dst->real = (METreal) ((Real)src1->real / (Real)*src2); \
	     dst->imaginary = (METreal) ((Real)src1->imaginary / (Real)*src2);
TEMPLATE(MET__op_div_guts_scalar_c_i, METcomplex, METinteger, METcomplex)
TEMPLATE(MET__op_div_guts_scalar_c_r, METcomplex, METreal,    METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS Real t;
#define BODY t = 1.0 / ((Real) src2->real * src2->real + \
                 (Real) src2->imaginary * src2->imaginary); \
        dst->real = t * (METreal) (src1->real * src2->real + \
			       src1->imaginary * src2->imaginary) ;  \
	dst->imaginary = t * (METreal) (src1->imaginary * src2->real - \
					src1->real * src2->imaginary) ;
TEMPLATE(MET__op_div_guts_scalar_c_c, METcomplex, METcomplex, METcomplex)
#undef BODY
#undef TEMPS
				      
static Boolean
MET__op_div_guts_scalar(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   int		a2t = arg2->verts->type;
   Boolean	(*f)() = MET_op_guts_failure;
   MME(MET__op_div_guts_scalar);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:
      break;
    case MET_FIELD_INTEGER:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_div_guts_scalar_i_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_div_guts_scalar_i_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_div_guts_scalar_i_c;
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
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
	 f = MET__op_div_guts_scalar_r_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_div_guts_scalar_r_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_div_guts_scalar_r_c;
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
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
	 f = MET__op_div_guts_scalar_c_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_div_guts_scalar_c_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_div_guts_scalar_c_c;
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_MAP:
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:
      break;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   if (SUCCESS == (f)(arg1, arg2, result)) {
      if (MET_NULL_DATA != result->next)
	 return MET__op_div_guts_scalar(arg1->next, arg2, result->next);
      else
	 return SUCCESS;
   } else
      return FAILURE;
}



METset *
MET_op_div(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[DIV_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[DIV_NUM_KIDS];
   METtype	*arg_type[DIV_NUM_KIDS];
   METtype	*check_type, *my_result_type = MET_NULL_TYPE, *guess;
   int		how_many, denom_field;
   Boolean	result, is_map = NO;
   MME(MET_op_div);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, DIV_NUM_KIDS);
#endif
   arg_set[DIV_LEFT] =
      MET_sym_be_set_ref(sym->kids[DIV_LEFT], type,
			 &arg_type[DIV_LEFT],
			 &arg_set_type[DIV_LEFT]);

   MET_TYPE_NEW_FIELD(guess, MET_FIELD_UNKNOWN);
   arg_set[DIV_RIGHT] =
      MET_sym_be_set_ref(sym->kids[DIV_RIGHT], guess,
			 &arg_type[DIV_RIGHT],
			 &arg_set_type[DIV_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[DIV_LEFT] ||
       MET_NULL_TYPE == arg_type[DIV_RIGHT])
      goto bail1;
   
   if (arg_type[DIV_RIGHT]->type == MET_TYPE_TYPE_MAP) {
      if (arg_type[DIV_LEFT]->type == MET_TYPE_TYPE_FIELD) {
	 is_map = YES;
	 check_type = arg_type[DIV_RIGHT]->info.map.to;
      }
      else if (MET_TYPE_TYPE_MAP == arg_type[DIV_LEFT]->type)
	 if (SUCCESS == MET_type_cast(arg_type[DIV_LEFT]->info.map.from, 
				      arg_type[DIV_RIGHT]->info.map.from,
				      &check_type, TYPE_CAST_MAKE_BOGUS)) {
	    MET_TYPE_FREE(check_type);
	    check_type = arg_type[DIV_RIGHT]->info.map.to;
	    while (MET_TYPE_TYPE_MAP == check_type->type)
	       check_type = check_type->info.map.to;
	 } else {
	    MET_TYPE_FREE(check_type);
	    check_type = arg_type[DIV_RIGHT];
	 }
   }
   else
      check_type = arg_type[DIV_RIGHT];

   if (check_type->type == MET_TYPE_TYPE_FIELD) {
      denom_field = check_type->info.field.type;
      my_result_type =
	 MET_type_cast_to_field(arg_type[DIV_LEFT],
				denom_field == MET_FIELD_INTEGER ?
				MET_FIELD_REAL : denom_field);
      if (YES == is_map) {
	 MET_TYPE_NEW_MAP(check_type, arg_type[DIV_RIGHT]->info.map.from,
			  my_result_type);
	 MET_TYPE_FREE(my_result_type);
	 MET_TYPE_COPY(my_result_type, check_type);
	 MET_TYPE_FREE(check_type);
      } 
   }

   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "div, bad argument types", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, DIV_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE)) {
      goto bail2;
   }

   /* If you have block sets, operate on each of the elements. */

   if (MET_SET_TYPE_BLOCK == arg_set[DIV_LEFT]->type ||
       MET_SET_TYPE_BLOCK == arg_set[DIV_RIGHT]->type) {
      result_set = MET_op_apply_to_elements(arg_set, DIV_NUM_KIDS,
					    MET_op_div, sym->info.op.code,
					    my_result_type, sym);
      MET_TYPE_FREE(my_result_type);
      goto bail1;
   }

   if (MET_SET_TYPE_BARE != arg_set[DIV_LEFT]->type ||
       MET_SET_TYPE_BARE != arg_set[DIV_RIGHT]->type) {
      ERROR(sym->origin, "no division of types, or symbols", SEV_FATAL);
      MET_TYPE_FREE(my_result_type);
      goto bail1;
   }

   MET_op_reconcile_sets(arg_set, DIV_NUM_KIDS);
   
   /* Map data is a special case: we'll take care of it separately. */

   if (MET_FIELD_MAP == arg_set[DIV_LEFT]->info.bare.data->verts->type ||
       MET_FIELD_MAP == arg_set[DIV_RIGHT]->info.bare.data->verts->type) {
      Boolean maps[DIV_NUM_KIDS];

      maps[DIV_LEFT] = MET_FIELD_MAP ==
	 arg_set[DIV_LEFT]->info.bare.data->verts->type;
      maps[DIV_RIGHT] = MET_FIELD_MAP == 
	 arg_set[DIV_RIGHT]->info.bare.data->verts->type;

      result_set = MET_op_guts_for_maps(arg_set, maps, DIV_NUM_KIDS,
					sym->info.op.code, my_result_type);
      MET_TYPE_FREE(my_result_type);
      goto bail1;
   }

   MAX_VERTS(how_many, arg_set[DIV_LEFT]->info.bare.data,
	     arg_set[DIV_RIGHT]->info.bare.data);
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, DIV_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);
   
   result =
      MET__op_div_guts_scalar(arg_set[DIV_LEFT]->info.bare.data,
			      arg_set[DIV_RIGHT]->info.bare.data,
			      result_set->info.bare.data);
   
   if (FAILURE == result) {
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(DIV_LEFT);
   COND_FREE(DIV_RIGHT);

   return result_set;
}
