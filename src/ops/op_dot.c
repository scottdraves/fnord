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
   "$Id: op_dot.c,v 1.10 1992/06/22 17:06:16 mas Exp $";
#endif

#include "defs.h"

#define DOT_NUM_KIDS		2
#define DOT_LEFT		0
#define DOT_RIGHT		1

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
   length = arg1->num_cols; \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   MAX_VERTS(i, arg1, arg2); \
   dst  = DATA_FIRST_VERTEX(result,  DST_TYPE); \
   UNROLL4(i, INIT_BODY;  dst += dst_width;); \
   for (j = 0; j < length; j++) { \
      src1 = DATA_FIRST_VERTEX(arg1, TYPE1) + j; \
      src2 = DATA_FIRST_VERTEX(arg2, TYPE2) + j; \
      dst  = DATA_FIRST_VERTEX(result, DST_TYPE); \
      MAX_VERTS(i, arg1, arg2); \
      UNROLL4(i, BODY; \
	 dst += dst_width; \
	 src1 += src1_width; \
	 src2 += src2_width;); \
   } \
} \

#define TEMPS
#define INIT_BODY   *dst = (METinteger) 0;
#define BODY *dst += (METinteger) ((int)*src1 * (int)*src2);
TEMPLATE(MET__op_dot_guts_i_i, METinteger, METinteger, METinteger)
#undef BODY
#undef INIT_BODY

#define INIT_BODY   *dst = (METreal) 0.0;
#define BODY *dst += (METreal) ((Real)*src1 * (Real)*src2);
TEMPLATE(MET__op_dot_guts_i_r, METinteger, METreal, METreal)
#undef BODY
#undef INIT_BODY

#define INIT_BODY   *dst = (METreal) 0.0;
#define BODY *dst += (METreal) ((Real)*src1 * (Real)*src2);
TEMPLATE(MET__op_dot_guts_r_r, METreal, METreal, METreal)
#undef BODY
#undef INIT_BODY

#define INIT_BODY dst->real = (METreal) 0.0; \
      dst->imaginary = (METreal) 0.0;
#define BODY dst->real += (METreal) ((Real)src1->real * (Real)*src2); \
   dst->imaginary += (METreal) ((Real)src1->imaginary * (Real)*src2);
TEMPLATE(MET__op_dot_guts_c_i, METcomplex, METinteger, METcomplex)
#undef BODY
#undef INIT_BODY

#define INIT_BODY dst->real = (METreal) 0.0; \
      dst->imaginary = (METreal) 0.0;
#define BODY dst->real += (METreal) ((Real)src2->real * (Real)*src1); \
   dst->imaginary -= (METreal) ((Real)src2->imaginary * (Real)*src1);
TEMPLATE(MET__op_dot_guts_i_c, METinteger, METcomplex, METcomplex)
#undef BODY
#undef INIT_BODY

#define INIT_BODY dst->real = (METreal) 0.0; \
      dst->imaginary = (METreal) 0.0;
#define BODY dst->real += (METreal) ((Real)src2->real * (Real)*src1); \
   dst->imaginary -= (METreal) ((Real)src2->imaginary * (Real)*src1);
TEMPLATE(MET__op_dot_guts_r_c, METreal, METcomplex, METcomplex)
#undef BODY
#undef INIT_BODY

#define INIT_BODY dst->real = (METreal) 0.0; \
      dst->imaginary = (METreal) 0.0;
#define BODY dst->real += (METreal) ((Real)src1->real * (Real)*src2); \
   dst->imaginary += (METreal) ((Real)src1->imaginary * (Real)*src2);
TEMPLATE(MET__op_dot_guts_c_r, METcomplex, METreal, METcomplex)
#undef BODY
#undef INIT_BODY

#define INIT_BODY dst->real = (METreal) 0.0; \
      dst->imaginary = (METreal) 0.0;
#define BODY \
   dst->real += (METreal) ((Real)src1->real * (Real)src2->real + \
		    (Real)src1->imaginary * (Real)src2->imaginary); \
  dst->imaginary += (METreal) ((Real)src1->imaginary * (Real)src2->real - \
		    (Real)src1->real * (Real)src2->imaginary);
TEMPLATE(MET__op_dot_guts_c_c, METcomplex, METcomplex, METcomplex)
#undef BODY
#undef INIT_BODY
#undef TEMPS

/* The dot product requires a unique iterative loop;  so dealing
   with vectors of maps is not as easy as the other operations are */

static void
MET__op_dot_guts_m (arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   int		i, j, length;
   METnumber	*src1;
   METmap	*src2;
   METmap	*dst;
   int		src1_width, src2_width, dst_width;
   METsym	*sym, *next;
   METtype	*type;
   METset	*nset;
   Boolean	arg1_is_map;
   MME(MET__op_dot_guts_m);

   length = arg1->num_cols;
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols;
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols;
   dst_width = result->verts->total_cols;

   src1_width *= MET_field_size [ arg1->verts->type ];
   arg1_is_map = (MET_FIELD_MAP == arg1->verts->type);

   MAX_VERTS(i, arg1, arg2);
   dst = DATA_FIRST_VERTEX(result, METmap);
   UNROLL4(i, dst->loose_from = MET_NULL_TYPE;
	      dst->env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV);
	      dst->sym = MET_NULL_SYM;
	      dst += dst_width;);
   for (j = 0; j < length; j++) {
      src1 = CAST(METnumber *, CAST(char *, arg1->verts->vertices) +
		  (arg1->first_col + j) *
		  MET_field_size [arg1->verts->type]);
      src2 = DATA_FIRST_VERTEX(arg2, METmap) + j;
      dst = DATA_FIRST_VERTEX(result, METmap);
      MAX_VERTS(i, arg1, arg2);
      for (; i; i--) {
	 if (MET_NULL_TYPE != dst->loose_from) {
	    (void) MET_type_cast(dst->loose_from,
				 src2->loose_from, &type,
				 TYPE_CAST_TAKE_SECOND);
	    if (type != dst->loose_from) {
	       MET_TYPE_FREE(dst->loose_from);
	       MET_TYPE_COPY(dst->loose_from, type);
	    }  
	    MET_TYPE_FREE(type);
	 } else 
	    MET_TYPE_COPY(dst->loose_from, src2->loose_from);
	 sym = METsym_new_op(MET_SYM_OP_MULTIPLY, src2->sym->origin);
	 if (arg1_is_map) {
	    METsym_add_kid(sym, src1->map.sym);
	 } else {
	    nset = METset_new_number(arg1->verts->type, src1);
	    next = METsym_new_set_ref(nset, src2->sym->origin);
	    MET_SET_FREE(nset);
	    METsym_add_kid(sym, next);
	    MET_SYM_FREE(next);
	 }
	 METsym_add_kid(sym, src2->sym);
	 if (MET_NULL_SYM != dst->sym) {
	    next = METsym_new_op(MET_SYM_OP_ADD, 	
				 src2->sym->origin);
	    METsym_add_kid(next, sym);
	    METsym_add_kid(next, dst->sym);
	    MET_SYM_FREE(dst->sym);			
	 } else
	    MET_SYM_COPY(next, sym);
	 MET_SYM_COPY(dst->sym, next);
	 MET_SYM_FREE(sym);	
	 MET_SYM_FREE(next);
	 dst += dst_width;
	 src1 = CAST(METnumber *, CAST(char *, src1) + src1_width);
	 src2 += src2_width;
      }
   }
}

static Boolean
MET__op_dot_guts(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_dot_guts);

   switch (arg1->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 switch (arg2->verts->type) {
	  case MET_FIELD_ZTWO:       return FAILURE;
	  case MET_FIELD_INTEGER:
	    MET__op_dot_guts_i_i(arg1, arg2, result);
	    break;
	  case MET_FIELD_ANGLE:      return FAILURE;
	  case MET_FIELD_REAL:
	    MET__op_dot_guts_i_r(arg1, arg2, result);
	    break;
	  case MET_FIELD_COMPLEX:
	    MET__op_dot_guts_i_c(arg1, arg2, result);
	    break;
	  case MET_FIELD_MAP:
	    MET__op_dot_guts_m(arg1, arg2, result);
	    break;
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
	    MET__op_dot_guts_i_r(arg2, arg1, result);
	    break;
	  case MET_FIELD_ANGLE:      return FAILURE;
	  case MET_FIELD_REAL:
	    MET__op_dot_guts_r_r(arg1, arg2, result);
	    break;
	  case MET_FIELD_COMPLEX:
	    MET__op_dot_guts_r_c(arg1, arg2, result);
	    break;
	  case MET_FIELD_MAP:
	    MET__op_dot_guts_m(arg1, arg2, result);
	    break;
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
	    MET__op_dot_guts_c_i(arg1, arg2, result);
	    break;
	  case MET_FIELD_ANGLE:      return FAILURE;
	  case MET_FIELD_REAL:
	    MET__op_dot_guts_c_r(arg1, arg2, result);
	    break;
	  case MET_FIELD_COMPLEX:
	    MET__op_dot_guts_c_c(arg1, arg2, result);
	    break;
	  case MET_FIELD_MAP:
	    MET__op_dot_guts_m(arg1, arg2, result);
	    break;
	  case MET_FIELD_QUATERNION:
	  case MET_FIELD_SET:        return FAILURE;
	  default:
	    DEFAULT_ERR(arg2->verts->type);
	 }
	 break;
       case MET_FIELD_MAP:
	 switch (arg2->verts->type) {
	  case MET_FIELD_INTEGER:
	  case MET_FIELD_REAL:
	  case MET_FIELD_COMPLEX:
	  case MET_FIELD_MAP:
	    MET__op_dot_guts_m(arg2, arg1, result);
	    break;

	  case MET_FIELD_ZTWO:
	  case MET_FIELD_ANGLE:
	  case MET_FIELD_QUATERNION:
	  case MET_FIELD_SET:        
	    return FAILURE;
	  default:
	    DEFAULT_ERR(arg2->verts->type);
	 }
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:        return FAILURE;
       default:
	 DEFAULT_ERR(arg1->verts->type);
   }

   return SUCCESS;
}

/*
 * with all these fancy operations on functions, it now takes
 * a whole procedure to figure out whether good types have
 * been passed to dot and what the output type should be.
 * NULL means bad argument types.
 */

static METtype *
MET__op_dot_type(arg_types, map)
   METtype	*arg_types[DOT_NUM_KIDS];
   Boolean	*map;
{
   METtype	*my_result_type = MET_NULL_TYPE, *left_vec, *right_vec;
   METtype	**temp, **old;
   Boolean	check;
   MME(MET__op_dot_type);

   left_vec = arg_types[DOT_LEFT];
   right_vec = arg_types[DOT_RIGHT];
   temp = &my_result_type;
   *map = FALSE;
   /*
    * MAP . MAP :
    * 1) all argument types must be castible
    * 2) final result must meet product conditions
    */
   if (left_vec->type == MET_TYPE_TYPE_MAP &&
       right_vec->type == MET_TYPE_TYPE_MAP) {
      check = TRUE;
      *map = TRUE;
      do {
	 MET_TYPE_ALLOC(*temp);
	 (*temp)->type = MET_TYPE_TYPE_MAP;
	 (*temp)->info.map.to = MET_NULL_TYPE;
	 if (FAILURE == MET_type_cast(left_vec->info.map.from,
				      right_vec->info.map.from,
				      &(*temp)->info.map.from,
				      TYPE_CAST_MAKE_BOGUS))
	    check = FALSE;
	 else {
	    temp = &(*temp)->info.map.to;
	    left_vec = left_vec->info.map.to;
	    right_vec = right_vec->info.map.to;
	    if (left_vec->type != right_vec->type)
	       check = FALSE;
	 }
      } while (left_vec->type == MET_TYPE_TYPE_MAP && TRUE == check);

      if (FALSE == check) {
	 MET_TYPE_FREE(my_result_type);
	 return MET_NULL_TYPE;
      }
   }
   /*
    * VEC . MAP
    * 1) final result of map must meet product conditions
    *    with vector.
    */
   else if (left_vec->type == MET_TYPE_TYPE_MAP ||
	    right_vec->type == MET_TYPE_TYPE_MAP) {
      *map = TRUE;
      old = (left_vec->type == MET_TYPE_TYPE_MAP) ? &left_vec : &right_vec;
      do {
	 MET_TYPE_ALLOC(*temp);
	 (*temp)->type = MET_TYPE_TYPE_MAP;
	 (*temp)->info.map.to = MET_NULL_TYPE;
	 MET_TYPE_COPY((*temp)->info.map.from, (*old)->info.map.from);
	 *old = (*old)->info.map.to;
	 temp = &(*temp)->info.map.to;
      } while ((*old)->type == MET_TYPE_TYPE_MAP);
   }
   
   /*
    * vectors must have the same dimension
    */

   if (left_vec->type != MET_TYPE_TYPE_VECTOR ||
       right_vec->type != MET_TYPE_TYPE_VECTOR ||
       left_vec->info.vector.dimension != right_vec->info.vector.dimension) {
      MET_TYPE_FREE_NULL(my_result_type);
      return MET_NULL_TYPE;
   }

   left_vec = left_vec->info.vector.of;
   right_vec = right_vec->info.vector.of;

   /* 
    * tight condition on elements:
    *    both must be castible maps or fields
    */

   if (left_vec->type == right_vec->type) {
      if ((left_vec->type != MET_TYPE_TYPE_MAP &&
	   left_vec->type != MET_TYPE_TYPE_FIELD) ||
	  FAILURE == MET_type_cast(left_vec, right_vec, temp,
				   TYPE_CAST_MAKE_BOGUS)) {
	 MET_TYPE_FREE_NULL(my_result_type);
	 return MET_NULL_TYPE;
      } else {
	 return my_result_type;
      }
   }
   
   if (YES == *map) {
      MET_TYPE_FREE(my_result_type);
      return MET_NULL_TYPE;
   }

   /* 
    * loose condition:
    *    one may be a map, the other a field
    */

   if ((left_vec->type != MET_TYPE_TYPE_MAP &&
	left_vec->type != MET_TYPE_TYPE_FIELD) ||
       (right_vec->type != MET_TYPE_TYPE_MAP &&
	right_vec->type != MET_TYPE_TYPE_FIELD)) 
      return MET_NULL_TYPE;
   
   if (right_vec->type == MET_TYPE_TYPE_FIELD) 
      *temp = MET_type_cast_to_field(left_vec, right_vec->info.field.type);
   else
      *temp = MET_type_cast_to_field(right_vec, left_vec->info.field.type);

   return my_result_type;
}

METset *
MET_op_dot(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[DOT_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[DOT_NUM_KIDS];
   METtype	*arg_type[DOT_NUM_KIDS], *guess;
   METtype	*my_result_type;
   int		how_many;
   Boolean	map;
   MME(MET_op_dot);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, DOT_NUM_KIDS);
#endif

   MET_TYPE_NEW_UNKNOWN(guess);
   if (MET_NULL_TYPE != type) {
      MET_TYPE_FREE(guess);
      MET_TYPE_NEW_VECTOR(guess, type, TYPE_DIMENSION_UNKNOWN, YES);
   }
   arg_set[DOT_LEFT] = MET_sym_be_set_ref(sym->kids[DOT_LEFT], guess,
					  &arg_type[DOT_LEFT],
					  &arg_set_type[DOT_LEFT]);

   arg_set[DOT_RIGHT] = MET_sym_be_set_ref(sym->kids[DOT_RIGHT], guess,
					   &arg_type[DOT_RIGHT],
					   &arg_set_type[DOT_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[DOT_LEFT] ||
       MET_NULL_TYPE == arg_type[DOT_RIGHT])
      goto bail2;

   my_result_type = MET__op_dot_type(arg_type, &map);

   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "dot, bad argument types", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, DOT_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;

   /* Take care of situations like f . [1,2,3] using the easy system;
      things like [f,g,h] . [g,h,f] will have to be done the hard way. */
   
   if (YES == map) {
      Boolean maps[DOT_NUM_KIDS];

      maps[DOT_LEFT] = MET_FIELD_MAP ==
	 arg_set[DOT_LEFT]->info.bare.data->verts->type;
      maps[DOT_RIGHT] = MET_FIELD_MAP == 
	 arg_set[DOT_RIGHT]->info.bare.data->verts->type;

      result_set = MET_op_guts_for_maps(arg_set, maps, DOT_NUM_KIDS,
					sym->info.op.code, my_result_type);
      MET_TYPE_FREE(my_result_type);
      goto bail1;
   }

   MET_op_reconcile_sets(arg_set, DOT_NUM_KIDS);

   MAX_VERTS(how_many, arg_set[DOT_LEFT]->info.bare.data,
	     arg_set[DOT_RIGHT]->info.bare.data);
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, DOT_NUM_KIDS);
   
   MET_TYPE_FREE(my_result_type);
   
   if (FAILURE == MET__op_dot_guts(arg_set[DOT_LEFT]->info.bare.data,
				   arg_set[DOT_RIGHT]->info.bare.data,
				   result_set->info.bare.data)) {
      ERROR(sym->origin, "dot, bad argument fields", SEV_FATAL);
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }
   

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(DOT_LEFT);
   COND_FREE(DOT_RIGHT);

   return result_set;
}
