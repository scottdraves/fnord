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
   "$Id: op_cross.c,v 1.16 1992/06/22 17:06:05 mas Exp $";
#endif

#include "defs.h"

#define CROSS_NUM_KIDS		2
#define CROSS_LEFT		0
#define CROSS_RIGHT		1

/*
 * replicate the vertices of the data.  repl_factor sez how many times.
 * if repl_near is true, then abcd -> aaaaabbbbbcccccddddd, otherwise
 * abcd -> abcdabcdabcdabcdabcd (5 was arbitrary)
 * creates and returns a new data.
 */
static METdata *
MET__op_cross_repl_data(data, repl_near, repl_factor)
   METdata	*data;
   Boolean	repl_near;
   int		repl_factor;
{
   METdata	*baby_data;
   METverts	*verts, *src_verts = data->verts;
   int		i, j;
   MME(MET__op_cross_repl_data);

   verts = MET_verts_new(src_verts->type,
			 src_verts->num_verts * repl_factor,
			 data->num_cols);

   /*
    * probably a good idea to have a special case or two
    */
   if (repl_near) {
      for (i = j = 0; i < src_verts->num_verts; i++, j += repl_factor)
	 MET_verts_copy_vertices(verts, j, 0, src_verts, i,
				 data->first_col, data->num_cols,
				 repl_factor, 1, YES);
   } else {
      for (i = j = 0; i < repl_factor; i++, j += src_verts->num_verts)
	 MET_verts_copy_vertices(verts, j, 0, src_verts, 0,
				 data->first_col, data->num_cols,
				 src_verts->num_verts, 1, NO);
   }

   baby_data = MET_data_new(verts, data->num_cols, 0);
   MET_VERTS_FREE(verts);
   if (MET_NULL_DATA != data->next)
      baby_data->next = MET__op_cross_repl_data(data->next, repl_near,
						repl_factor);
   return baby_data;
}
   
/*
 * take the cartesian set cross product of two sets.
 * This is done by taking all possible combinations of blocks, and crossing
 * the blocks.
 */
static METset *
MET__op_cross_set(set1, set2)
   METset	*set1, *set2;
{
   METset	*result_set;
   METblock	*b1, *b2, *new_block;
   METdata	*d1, *d2;
   METtype	*type;
   MME(MET__op_cross_set);

   result_set = MET_set_new(MET_SET_TYPE_BLOCK);
   result_set->info.block.block_list = MET_NULL_BLOCK;

   b1 = set1->info.block.block_list;
   while (MET_NULL_BLOCK != b1) {
      b2 = set2->info.block.block_list;
      while (MET_NULL_BLOCK != b2) {
	 MET_BLOCK_ALLOC(new_block);
	 MET_tags_initialize(&new_block->tags_in);
	 MET_tags_initialize(&new_block->tags_part);
	 new_block->next = MET_NULL_BLOCK;
	 MET_TYPE_NEW_PAIR(type, b1->type->info.set_of.of,
			   b2->type->info.set_of.of);
	 MET_TYPE_NEW_SET_OF(new_block->type, type);
	 MET_TYPE_FREE(type);
	 new_block->connex = MET_connex_cross(b1->connex,
					      b2->connex);
	 new_block->data_connex = MET_connex_cross(b1->data_connex,
						   b2->data_connex);
	 /* Ack! Double copy overlapping hidden tags? */
	 d1 = MET__op_cross_repl_data(b1->data, YES,
				      DATA_NUM_VERTS(b2->data));
	 d2 = MET__op_cross_repl_data(b2->data, NO,
				      DATA_NUM_VERTS(b1->data));
	 MET_data_append(d1, d2);
	 MET_DATA_COPY(new_block->data, d1);
	 MET_DATA_FREE(d1);
	 MET_DATA_FREE(d2);
	 MET_set_add_block(result_set, new_block);
	 MET_BLOCK_FREE(new_block);

	 b2 = b2->next;
      }
      b1 = b1->next;
   }

   return result_set;
}

/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME, TYPE1, TYPE2, DST_TYPE) \
static void \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		i; \
   TYPE1	*src1; \
   TYPE2	*src2; \
   DST_TYPE	*dst; \
   int		src1_width, src2_width, dst_width; \
   MME(NAME); \
 \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   src1 = DATA_FIRST_VERTEX(arg1, TYPE1); \
   src2 = DATA_FIRST_VERTEX(arg2, TYPE2); \
   dst  = DATA_FIRST_VERTEX(result, DST_TYPE); \
   MAX_VERTS(i, arg1, arg2); \
   UNROLL4(i, BODY; \
      dst += dst_width; \
      src1 += src1_width; \
      src2 += src2_width;); \
}

#define BODY \
dst[0] = (src1[1] * src2[2] - src1[2] * src2[1]); \
dst[1] = (src1[2] * src2[0] - src1[0] * src2[2]); \
dst[2] = (src1[0] * src2[1] - src1[1] * src2[0]);
TEMPLATE(MET__op_cross_guts_i_i, METinteger, METinteger, METinteger)
TEMPLATE(MET__op_cross_guts_i_r, METinteger, METreal, METreal)
TEMPLATE(MET__op_cross_guts_r_i, METreal, METinteger, METreal)
TEMPLATE(MET__op_cross_guts_r_r, METreal, METreal, METreal)
#undef BODY

static Boolean
MET__op_cross_guts(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_cross_guts);

   switch (arg1->verts->type) {
       case MET_FIELD_ZTWO:       return FAILURE;
       case MET_FIELD_INTEGER:
	 switch (arg2->verts->type) {
	  case MET_FIELD_ZTWO:       return FAILURE;
	  case MET_FIELD_INTEGER:
	    MET__op_cross_guts_i_i(arg1, arg2, result);
	    break;
	  case MET_FIELD_ANGLE:      return FAILURE;
	  case MET_FIELD_REAL:
	    MET__op_cross_guts_i_r(arg1, arg2, result);
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
	    MET__op_cross_guts_r_i(arg1, arg2, result);
	    break;
	  case MET_FIELD_ANGLE:      return FAILURE;
	  case MET_FIELD_REAL:
	    MET__op_cross_guts_r_r(arg1, arg2, result);
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
MET_op_cross(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[CROSS_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[CROSS_NUM_KIDS];
   METtype	*arg_type[CROSS_NUM_KIDS], *lguess, *rguess;
   METtype	*my_result_type = MET_NULL_TYPE;
   int		how_many;
   Boolean	is_set_cross = NO;
   MME(MET_op_cross);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, CROSS_NUM_KIDS);
#endif

   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_PAIR == type->type) {
      MET_TYPE_COPY(lguess, type->info.pair.left);
      MET_TYPE_COPY(rguess, type->info.pair.right);
   } else if (MET_NULL_TYPE != type &&
	      MET_TYPE_TYPE_VECTOR == type->type) {
      MET_TYPE_COPY(lguess, type);
      MET_TYPE_COPY(rguess, type);
   } else if (MET_NULL_TYPE != type &&
	      MET_TYPE_TYPE_SET == type->type) {
      MET_TYPE_COPY(lguess, type);
      MET_TYPE_COPY(rguess, type);
   } else {
      MET_TYPE_NEW_UNKNOWN(lguess);
      MET_TYPE_NEW_UNKNOWN(rguess);
   }

   arg_set[CROSS_LEFT] =
      MET_sym_be_set_ref(sym->kids[CROSS_LEFT], type,
			 &arg_type[CROSS_LEFT],
			 &arg_set_type[CROSS_LEFT]);
   
   arg_set[CROSS_RIGHT] =
      MET_sym_be_set_ref(sym->kids[CROSS_RIGHT], type,
			 &arg_type[CROSS_RIGHT],
			 &arg_set_type[CROSS_RIGHT]);

   MET_TYPE_FREE(lguess);
   MET_TYPE_FREE(rguess);
   
   if (MET_NULL_TYPE == arg_type[CROSS_LEFT] ||
       MET_NULL_TYPE == arg_type[CROSS_RIGHT])
      goto bail1;
   
   if ((MET_NULL_SET != arg_set[CROSS_LEFT] &&
	MET_SET_TYPE_TYPE == arg_set[CROSS_LEFT]->type) ||
       (MET_NULL_SET != arg_set[CROSS_RIGHT] &&
	MET_SET_TYPE_TYPE == arg_set[CROSS_RIGHT]->type)) {
      COND_FREE(CROSS_LEFT);
      COND_FREE(CROSS_RIGHT);
      return MET_op_type_expr(sym, type, result_type, set_type);
   }
      

   /*
    * make sure they are both vectors of length 3
    */
   if (arg_type[CROSS_LEFT]->type == MET_TYPE_TYPE_VECTOR &&
       arg_type[CROSS_RIGHT]->type == MET_TYPE_TYPE_VECTOR &&
       arg_type[CROSS_LEFT]->info.vector.of->type == MET_TYPE_TYPE_FIELD &&
       arg_type[CROSS_RIGHT]->info.vector.of->type == MET_TYPE_TYPE_FIELD &&
       arg_type[CROSS_LEFT]->info.vector.dimension == 3 &&
       arg_type[CROSS_RIGHT]->info.vector.dimension == 3 &&
       arg_type[CROSS_LEFT]->info.vector.is_row ==
       arg_type[CROSS_RIGHT]->info.vector.is_row) {
      if (FAILURE == MET_type_cast(arg_type[CROSS_LEFT], 
				   arg_type[CROSS_RIGHT],
				   &my_result_type, TYPE_CAST_MAKE_BOGUS)){
	 MET_TYPE_FREE(my_result_type);
	 my_result_type = MET_NULL_TYPE;
      }
   } else
      if (MET_TYPE_TYPE_SET_OF == arg_type[CROSS_LEFT]->type &&
	  MET_TYPE_TYPE_SET_OF == arg_type[CROSS_RIGHT]->type) {
	 is_set_cross = YES;
	 MET_TYPE_NEW_UNKNOWN(my_result_type);
      }
   
   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "cross, bad argument types", SEV_FATAL);
      goto bail1;
   }

   if (MET_op_type_only(arg_set, CROSS_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_UNKNOWN))
      goto bail2;
   MET_op_reconcile_sets(arg_set, CROSS_NUM_KIDS);

   if (YES == is_set_cross) {
      if (YES == MET_op_block_only(arg_set, CROSS_NUM_KIDS, 
				   MET_op_cross, MET_SYM_OP_CROSS,
				   my_result_type)) {
	 MET_SET_COPY(result_set, arg_set[0]);
      } else {
	 result_set = MET__op_cross_set(arg_set[CROSS_LEFT],
					arg_set[CROSS_RIGHT]);
      }
   } else {
      MAX_VERTS(how_many, arg_set[CROSS_LEFT]->info.bare.data,
		arg_set[CROSS_RIGHT]->info.bare.data);
      result_set = MET_type_to_polybare_set(my_result_type, how_many,
					    arg_set, CROSS_NUM_KIDS);
      
      if (FAILURE ==
	  MET__op_cross_guts(arg_set[CROSS_LEFT]->info.bare.data,
			     arg_set[CROSS_RIGHT]->info.bare.data,
			     result_set->info.bare.data)) {
	 ERROR(sym->origin, "cross, bad argument fields", SEV_FATAL);
	 MET_SET_FREE(result_set);
	 result_set = MET_NULL_SET;
      }
   }
   MET_TYPE_FREE(my_result_type);
      
 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(CROSS_LEFT);
   COND_FREE(CROSS_RIGHT);

   return result_set;
}
