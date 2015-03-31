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
   "$Id: op_add.c,v 1.11 1992/06/22 17:05:45 mas Exp $";
#endif

#include "defs.h"

#define ADD_NUM_KIDS		2
#define ADD_LEFT		0
#define ADD_RIGHT		1

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
      src2 = DATA_FIRST_VERTEX(arg2, TYPE2) + j; \
      dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + j; \
      MAX_VERTS(i, arg1, arg2); \
      UNROLL4(i, BODY; \
	      dst += dst_width; src1 += src1_width; src2 += src2_width;);\
   } \
} \

/*
 * addition
 */
#define TEMPS
#define BODY *dst = (METinteger) ((int)*src1 + (int)*src2)
TEMPLATE(MET__op_add_guts_i_i, METinteger, METinteger, METinteger)
#undef BODY

#define BODY *dst = (METreal) ((Real)*src1 + (Real)*src2);
TEMPLATE(MET__op_add_guts_i_r, METinteger, METreal, METreal)
#undef BODY

#define BODY dst->real = (METreal) ((Real)*src1 + (Real)src2->real); \
	     dst->imaginary = src2->imaginary;
TEMPLATE(MET__op_add_guts_i_c, METinteger, METcomplex, METcomplex)
#undef BODY

#define BODY dst->real = (METreal) ((Real)*src1 + (Real)src2->real); \
	     dst->imaginary = src2->imaginary;
TEMPLATE(MET__op_add_guts_r_c, METreal, METcomplex, METcomplex)
#undef BODY

#define BODY *dst = (METreal) ((Real)*src1 + (Real)*src2);
TEMPLATE(MET__op_add_guts_r_r, METreal, METreal, METreal)
#undef BODY

#define BODY dst->real = (METreal) ((Real)src1->real + (Real)src2->real); \
	     dst->imaginary = (METreal) ((Real)src1->imaginary + \
				         (Real)src2->imaginary);
TEMPLATE(MET__op_add_guts_c_c, METcomplex, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

/*
 * subtraction (I need more of these because it doesn't commute)
 */
#define TEMPS
#define BODY *dst = (METinteger) ((int)*src1 - (int)*src2)
TEMPLATE(MET__op_subtract_guts_i_i, METinteger, METinteger, METinteger)
#undef BODY

#define BODY *dst = (METreal) ((Real)*src1 - (Real)*src2);
TEMPLATE(MET__op_subtract_guts_i_r, METinteger, METreal, METreal)
#undef BODY

#define BODY *dst = (METreal) ((Real)*src1 - (Real)*src2);
TEMPLATE(MET__op_subtract_guts_r_i, METreal, METinteger, METreal)
#undef BODY

#define BODY dst->real = (METreal) ((Real)src1->real - (Real)*src2); \
	     dst->imaginary = src1->imaginary;
TEMPLATE(MET__op_subtract_guts_c_i, METcomplex, METinteger, METcomplex)
#undef BODY

#define BODY dst->real = (METreal) ((Real)*src1 - (Real)src2->real); \
	     dst->imaginary = -src2->imaginary;
TEMPLATE(MET__op_subtract_guts_i_c, METinteger, METcomplex, METcomplex)
#undef BODY

#define BODY dst->real = (METreal) ((Real)*src1 - (Real)src2->real); \
	     dst->imaginary = -src2->imaginary;
TEMPLATE(MET__op_subtract_guts_r_c, METreal, METcomplex, METcomplex)
#undef BODY

#define BODY dst->real = (METreal) ((Real)src1->real - (Real)*src2); \
	     dst->imaginary = src1->imaginary;
TEMPLATE(MET__op_subtract_guts_c_r, METcomplex, METreal, METcomplex)
#undef BODY

#define BODY *dst = (METreal) ((Real)*src1 - (Real)*src2);
TEMPLATE(MET__op_subtract_guts_r_r, METreal, METreal, METreal)
#undef BODY

#define BODY dst->real = (METreal) ((Real)src1->real - (Real)src2->real); \
	     dst->imaginary = (METreal) ((Real)src1->imaginary - \
				         (Real)src2->imaginary);
TEMPLATE(MET__op_subtract_guts_c_c, METcomplex, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

/*
 * min
 */
#define TEMPS
#define BODY *dst = (METinteger) \
   (((int)*src1 < (int)*src2) ? (int)*src1 : (int)*src2)
TEMPLATE(MET__op_min_guts_i_i, METinteger, METinteger, METinteger)
#undef BODY

#define BODY *dst = (METreal) \
   (((Real)*src1 < (Real)*src2) ? (Real)*src1 : (Real)*src2)
TEMPLATE(MET__op_min_guts_i_r, METinteger, METreal, METreal)
TEMPLATE(MET__op_min_guts_r_r, METreal, METreal, METreal)
#undef BODY
#undef TEMPS

/*
 * max
 */

#define TEMPS
#define BODY *dst = (METinteger) \
   (((int)*src1 > (int)*src2) ? (int)*src1 : (int)*src2)
TEMPLATE(MET__op_max_guts_i_i, METinteger, METinteger, METinteger)
#undef BODY

#define BODY *dst = (METreal) \
   (((Real)*src1 > (Real)*src2) ? (Real)*src1 : (Real)*src2)
TEMPLATE(MET__op_max_guts_i_r, METinteger, METreal, METreal)
TEMPLATE(MET__op_max_guts_r_r, METreal, METreal, METreal)
#undef BODY
#undef TEMPS

/*
 * scan arg1 and arg2, adding everything pairwise.
 * the actual addition is done by a function tuned for the particular cast
 * involved. 
 */
static Boolean
MET__op_add_guts(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_add_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:       return FAILURE;
    case MET_FIELD_INTEGER:
      switch (arg2->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 MET__op_add_guts_i_i(arg1, arg2, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_add_guts_i_r(arg1, arg2, result);
	 break;
       case MET_FIELD_COMPLEX:
	 MET__op_add_guts_i_c(arg1, arg2, result);
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
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
	 MET__op_add_guts_i_r(arg2, arg1, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_add_guts_r_r(arg1, arg2, result);
	 break;
       case MET_FIELD_COMPLEX:
	 MET__op_add_guts_r_c(arg1, arg2, result);
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:        return FAILURE;
       default:
	 DEFAULT_ERR(arg2->verts->type);
      }
      break;
    case MET_FIELD_COMPLEX:
      switch (arg2->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 MET__op_add_guts_i_c(arg2, arg1, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_add_guts_r_c(arg2, arg1, result);
	 break;
       case MET_FIELD_COMPLEX:
	 MET__op_add_guts_c_c(arg1, arg2, result);
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:        return FAILURE;
       default:
	 DEFAULT_ERR(arg2->verts->type);
      }
      break;
    case MET_FIELD_MAP:
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:        return FAILURE;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }
   
   if (MET_NULL_DATA != result->next)
      return MET__op_add_guts(arg1->next, arg2->next, result->next);
   else
      return SUCCESS;
}

static Boolean
MET__op_subtract_guts(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_subtract_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:       return FAILURE;
    case MET_FIELD_INTEGER:
      switch (arg2->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 MET__op_subtract_guts_i_i(arg1, arg2, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_subtract_guts_i_r(arg1, arg2, result);
	 break;
       case MET_FIELD_COMPLEX:
	 MET__op_subtract_guts_i_c(arg1, arg2, result);
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
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
	 MET__op_subtract_guts_r_i(arg1, arg2, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_subtract_guts_r_r(arg1, arg2, result);
	 break;
       case MET_FIELD_COMPLEX:
	 MET__op_subtract_guts_r_c(arg1, arg2, result);
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:        return FAILURE;
       default:
	 DEFAULT_ERR(arg2->verts->type);
      }
      break;
    case MET_FIELD_COMPLEX:
      switch (arg2->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 MET__op_subtract_guts_c_i(arg1, arg2, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_subtract_guts_c_r(arg1, arg2, result);
	 break;
       case MET_FIELD_COMPLEX:
	 MET__op_subtract_guts_c_c(arg1, arg2, result);
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:        return FAILURE;
       default:
	 DEFAULT_ERR(arg2->verts->type);
      }
      break;
    case MET_FIELD_MAP:
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:        return FAILURE;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   if (MET_NULL_DATA != result->next)
      return MET__op_subtract_guts(arg1->next, arg2->next, result->next);
   else
      return SUCCESS;
}

static Boolean
MET__op_min_guts(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_min_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:       return FAILURE;
    case MET_FIELD_INTEGER:
      switch (arg2->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 MET__op_min_guts_i_i(arg1, arg2, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_min_guts_i_r(arg1, arg2, result);
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
	 MET__op_min_guts_i_r(arg2, arg1, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_min_guts_r_r(arg1, arg2, result);
	 break;
       case MET_FIELD_COMPLEX:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:        return FAILURE;
       default:
	 DEFAULT_ERR(arg2->verts->type);
      }
      break;
    case MET_FIELD_MAP:
    case MET_FIELD_COMPLEX:
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:        return FAILURE;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }
   
   if (MET_NULL_DATA != result->next)
      return MET__op_min_guts(arg1->next, arg2->next, result->next);
   else
      return SUCCESS;
}


static Boolean
MET__op_max_guts(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_max_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:       return FAILURE;
    case MET_FIELD_INTEGER:
      switch (arg2->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 MET__op_max_guts_i_i(arg1, arg2, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_max_guts_i_r(arg1, arg2, result);
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
	 MET__op_max_guts_i_r(arg2, arg1, result);
	 break;
       case MET_FIELD_ANGLE:      return FAILURE;
       case MET_FIELD_REAL:
	 MET__op_max_guts_r_r(arg1, arg2, result);
	 break;
       case MET_FIELD_COMPLEX:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:        return FAILURE;
       default:
	 DEFAULT_ERR(arg2->verts->type);
      }
      break;
    case MET_FIELD_MAP:
    case MET_FIELD_COMPLEX:
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:        return FAILURE;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }
   
   if (MET_NULL_DATA != result->next)
      return MET__op_max_guts(arg1->next, arg2->next, result->next);
   else
      return SUCCESS;
}

/*
 * limitation (not to say bug): certain cases that the math guts
 * could handle will not be accepted by this function because of 
 * unfortunate restrictions in type checking.  Really there should
 * be a recursive defintion of what types match what for addition
 * so that you could add vectors of functions to vectors of integers
 * for example....
 */

METset *
MET_op_add(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[ADD_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[ADD_NUM_KIDS];
   METtype	*arg_type[ADD_NUM_KIDS], *made_of[ADD_NUM_KIDS];
   METtype	*my_result_type, **temp;
   int		how_many;
   Boolean	failed;
   MME(MET_op_add);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, ADD_NUM_KIDS);
#endif
   arg_set[ADD_LEFT] = MET_sym_be_set_ref(sym->kids[ADD_LEFT], type,
					  &arg_type[ADD_LEFT],
					  &arg_set_type[ADD_LEFT]);

   arg_set[ADD_RIGHT] = MET_sym_be_set_ref(sym->kids[ADD_RIGHT], type,
					   &arg_type[ADD_RIGHT],
					   &arg_set_type[ADD_RIGHT]);

   if (MET_NULL_TYPE == arg_type[ADD_LEFT] ||
       MET_NULL_TYPE == arg_type[ADD_RIGHT])
      goto bail1;
   
   made_of[ADD_LEFT] = arg_type[ADD_LEFT];
   made_of[ADD_RIGHT] = arg_type[ADD_RIGHT];

   my_result_type = MET_NULL_TYPE;
   temp = &my_result_type;

   /* A more flexible addition requires more work to make 
      sure that types match---at the same time, we'll be 
      removing information from our two comparison types 
      and building up our result type.  First, we'll strip
      off a layer of sets if there is one.  Note that if
      we are not doing addition or subtraction, then we'll
      bail if we have set types. */

   if (made_of[ADD_LEFT]->type == MET_TYPE_TYPE_SET_OF) {
      if (MET_SYM_OP_ADD == sym->info.op.code ||
	  MET_SYM_OP_SUBTRACT == sym->info.op.code) {
	 MET_TYPE_ALLOC(my_result_type);
	 my_result_type->type = MET_TYPE_TYPE_SET_OF;
	 my_result_type->info.set_of.of = MET_NULL_TYPE;
	 temp = &my_result_type->info.set_of.of;
	 made_of[ADD_LEFT] = made_of[ADD_LEFT]->info.set_of.of;
      } else {
	 goto bail1;
      }
   }

   if (made_of[ADD_RIGHT]->type == MET_TYPE_TYPE_SET_OF) {
       if (MET_SYM_OP_ADD == sym->info.op.code ||
	   MET_SYM_OP_SUBTRACT == sym->info.op.code) {
	  if (MET_NULL_TYPE == my_result_type) {
	     MET_TYPE_ALLOC(my_result_type);
	     my_result_type->type = MET_TYPE_TYPE_SET_OF;
	     my_result_type->info.set_of.of = NULL;
	     temp = &my_result_type->info.set_of.of;
	  }
	  made_of[ADD_RIGHT] = made_of[ADD_RIGHT]->info.set_of.of;
       } else {
	  goto bail1;
       }
    }

   /* Then (no matter what the op is), we'll get rid of any
      lopsided maps (those not echoed on the other type). 
      This because `f + 5' makes sense. */

   if ((made_of[ADD_LEFT]->type == MET_TYPE_TYPE_MAP ||
	made_of[ADD_RIGHT]->type == MET_TYPE_TYPE_MAP) &&
       made_of[ADD_RIGHT]->type != made_of[ADD_LEFT]->type) {
      while (made_of[ADD_RIGHT]->type == MET_TYPE_TYPE_MAP) {
	 MET_TYPE_ALLOC(*temp);
	 (*temp)->type = MET_TYPE_TYPE_MAP;
	 MET_TYPE_COPY((*temp)->info.map.from,
		       made_of[ADD_RIGHT]->info.map.from);
	 (*temp)->info.map.to = MET_NULL_TYPE;
	 temp = &(*temp)->info.map.to;
	 made_of[ADD_RIGHT] = made_of[ADD_RIGHT]->info.map.to;
      }
      while (made_of[ADD_LEFT]->type == MET_TYPE_TYPE_MAP) {
	 MET_TYPE_ALLOC(*temp);
	 (*temp)->type = MET_TYPE_TYPE_MAP;
	 MET_TYPE_COPY((*temp)->info.map.from,
		       made_of[ADD_LEFT]->info.map.from);
	 (*temp)->info.map.to = MET_NULL_TYPE;
	 temp = &(*temp)->info.map.to;
	 made_of[ADD_LEFT] = made_of[ADD_LEFT]->info.map.to;
      }
   }

   /* now make sure that we've got the same things for our
      two operands. */

   if (NO == MET_type_cast(made_of[ADD_LEFT], made_of[ADD_RIGHT],
			   temp, TYPE_CAST_MAKE_BOGUS)) {
      MET_TYPE_FREE(my_result_type);
      my_result_type = MET_NULL_TYPE;
   }
   
   if (MET_op_type_only(arg_set, ADD_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;

   if (MET_NULL_TYPE == my_result_type) {
      MET_op_cant_cast(sym, arg_type[ADD_LEFT], arg_type[ADD_RIGHT], CNULL);
      goto bail1;
   }

   /* If you have block sets, operate on each of the elements.
      (Only if you are doing addition; this does not make much sense
      for min and max---but they will have already bailed on type.) */

   if (MET_SET_TYPE_BLOCK == arg_set[ADD_LEFT]->type ||
       MET_SET_TYPE_BLOCK == arg_set[ADD_RIGHT]->type) {
      result_set = MET_op_apply_to_elements(arg_set, ADD_NUM_KIDS,
					    MET_op_add, sym->info.op.code,
					    my_result_type, sym);
      MET_TYPE_FREE(my_result_type);
      goto bail1;
   }

   if (MET_SET_TYPE_BARE != arg_set[ADD_LEFT]->type ||
       MET_SET_TYPE_BARE != arg_set[ADD_RIGHT]->type) {
      ERROR(sym->origin, "no addition of types, or symbols", SEV_FATAL);
      MET_TYPE_FREE(my_result_type);
      goto bail1;
   }

   MET_op_reconcile_sets(arg_set, ADD_NUM_KIDS);

   if (YES == MET_op_block_only(arg_set, ADD_NUM_KIDS, MET_op_add,
				MET_SYM_OP_ADD, my_result_type)) {
      MET_TYPE_FREE(my_result_type);
      MET_SET_COPY(result_set, arg_set[0]);
      goto bail1;
   }

   /* Map data is a special case: we'll take care of it separately. */

   if (MET_FIELD_MAP == arg_set[ADD_LEFT]->info.bare.data->verts->type ||
       MET_FIELD_MAP == arg_set[ADD_RIGHT]->info.bare.data->verts->type) {
      Boolean maps[ADD_NUM_KIDS];

      maps[ADD_LEFT] = MET_FIELD_MAP ==
	 arg_set[ADD_LEFT]->info.bare.data->verts->type;
      maps[ADD_RIGHT] = MET_FIELD_MAP == 
	 arg_set[ADD_RIGHT]->info.bare.data->verts->type;

      result_set = MET_op_guts_for_maps(arg_set, maps, ADD_NUM_KIDS,
					sym->info.op.code, my_result_type);
      MET_TYPE_FREE(my_result_type);
      goto bail1;
   }

   MAX_VERTS(how_many, arg_set[ADD_LEFT]->info.bare.data,
	     arg_set[ADD_RIGHT]->info.bare.data);
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, ADD_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);

   switch (sym->info.op.code) {
    case MET_SYM_OP_ADD:
      	failed = MET__op_add_guts(arg_set[ADD_LEFT]->info.bare.data,
				  arg_set[ADD_RIGHT]->info.bare.data,
				  result_set->info.bare.data);
	break;
    case MET_SYM_OP_SUBTRACT:
      	failed = MET__op_subtract_guts(arg_set[ADD_LEFT]->info.bare.data,
				       arg_set[ADD_RIGHT]->info.bare.data,
				       result_set->info.bare.data);
	break;
    case MET_SYM_OP_MIN:
      	failed = MET__op_min_guts(arg_set[ADD_LEFT]->info.bare.data,
				  arg_set[ADD_RIGHT]->info.bare.data,
				  result_set->info.bare.data);
	break;
    case MET_SYM_OP_MAX:
      	failed = MET__op_max_guts(arg_set[ADD_LEFT]->info.bare.data,
				  arg_set[ADD_RIGHT]->info.bare.data,
				  result_set->info.bare.data);
	break;
   }
   if (FAILURE == failed) {
      ERROR(sym->origin, "add, bad argument fields", SEV_FATAL);
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }
   
 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(ADD_LEFT);
   COND_FREE(ADD_RIGHT);

   return result_set;
}
