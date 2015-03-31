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
   "$Id: op_cxbuild.c,v 1.9 1992/06/22 17:06:08 mas Exp $";
#endif

#include "defs.h"

#define CXBUILD_NUM_KIDS	2
#define CXBUILD_LEFT		0
#define CXBUILD_RIGHT		1

/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME, TYPE1, TYPE2, DST_TYPE) \
static void \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		i, j, length; \
   TYPE1	*src1; \
   TYPE2	*src2; \
   DST_TYPE	*dst; \
   int		src1_width, src2_width, dst_width; \
   MME(NAME); \
 \
   length = arg1->num_cols; \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   for (j = 0; j < length; j++) { \
      src1 = DATA_FIRST_VERTEX(arg1, TYPE1) + j; \
      src2 = DATA_FIRST_VERTEX(arg2, TYPE2) + j; \
      dst  = DATA_FIRST_VERTEX(result, DST_TYPE); \
      MAX_VERTS(i, arg1, arg2); \
       \
      while(i--) { \
	 BODY; \
	 dst += dst_width; \
	 src1 += src1_width; \
	 src2 += src2_width; \
      } \
   } \
} \


#define BODY dst->real = (METreal) *src1; \
        dst->imaginary = (METreal) *src2;
TEMPLATE(MET__op_cxbuild_guts_i_i, METinteger, METinteger, METcomplex)
TEMPLATE(MET__op_cxbuild_guts_r_i, METreal,    METinteger, METcomplex)
TEMPLATE(MET__op_cxbuild_guts_i_r, METinteger, METreal, METcomplex)
TEMPLATE(MET__op_cxbuild_guts_r_r, METreal,    METreal, METcomplex)
#undef BODY


static Boolean
MET__op_cxbuild_guts(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_cxbuild_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:       return FAILURE;
    case MET_FIELD_INTEGER:
      switch (arg2->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 MET__op_cxbuild_guts_i_i(arg1, arg2, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_cxbuild_guts_i_r(arg1, arg2, result);
	 break;
       case MET_FIELD_COMPLEX:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:        return FAILURE;
       default:
	 DEFAULT_ERR(arg2->verts->type);
      }
      break;
    case MET_FIELD_ANGLE:      return FAILURE;
    case MET_FIELD_REAL:
      switch (arg2->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 MET__op_cxbuild_guts_r_i(arg1, arg2, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_cxbuild_guts_r_r(arg1, arg2, result);
	 break;
       case MET_FIELD_COMPLEX:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:        return FAILURE;
       default:
	 DEFAULT_ERR(arg2->verts->type);
      }
      break;
    case MET_FIELD_COMPLEX:
    case MET_FIELD_QUATERNION:
    case MET_FIELD_MAP:
    case MET_FIELD_SET:        return FAILURE;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   return SUCCESS;
}

METset *
MET_op_complex_build(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[CXBUILD_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[CXBUILD_NUM_KIDS];
   METtype	*arg_type[CXBUILD_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many;
   MME(MET_op_complex_build);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, CXBUILD_NUM_KIDS);
#endif

   MET_TYPE_NEW_FIELD(guess, MET_FIELD_REAL);
   arg_set[CXBUILD_LEFT] =
      MET_sym_be_set_ref(sym->kids[CXBUILD_LEFT], guess,
			 &arg_type[CXBUILD_LEFT],
			 &arg_set_type[CXBUILD_LEFT]);
   
   arg_set[CXBUILD_RIGHT] =
      MET_sym_be_set_ref(sym->kids[CXBUILD_RIGHT], guess,
			 &arg_type[CXBUILD_RIGHT],
			 &arg_set_type[CXBUILD_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[CXBUILD_LEFT] ||
       MET_NULL_TYPE == arg_type[CXBUILD_RIGHT])
      goto bail1;
   
   /*
    * make sure they are both reals or integers:
    */
   if (MET_TYPE_TYPE_FIELD == arg_type[CXBUILD_LEFT]->type &&
       MET_TYPE_TYPE_FIELD == arg_type[CXBUILD_RIGHT]->type)
      MET_TYPE_NEW_FIELD(my_result_type, MET_FIELD_COMPLEX);

   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "cxbuild, bad argument types", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, CXBUILD_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
       goto bail2;
   MET_op_reconcile_sets(arg_set, CXBUILD_NUM_KIDS);

   MAX_VERTS(how_many, arg_set[CXBUILD_LEFT]->info.bare.data,
	     arg_set[CXBUILD_RIGHT]->info.bare.data);
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, CXBUILD_NUM_KIDS);
   
   MET_TYPE_FREE(my_result_type);
   
   if (FAILURE ==
       MET__op_cxbuild_guts(arg_set[CXBUILD_LEFT]->info.bare.data,
			    arg_set[CXBUILD_RIGHT]->info.bare.data,
			    result_set->info.bare.data)) {
      ERROR(sym->origin, "cxbuild, bad argument fields", SEV_FATAL);
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }
   
 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(CXBUILD_LEFT);
   COND_FREE(CXBUILD_RIGHT);
   
   return result_set;
}
