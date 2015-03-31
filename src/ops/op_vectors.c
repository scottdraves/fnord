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
   "$Id: op_vectors.c,v 1.15 1992/06/22 17:07:39 mas Exp $";
#endif

#include "defs.h"

#define VECTOR_BUILD_PAIRS	0
#define VECTOR_BUILD_NUM_KIDS	1

/*
 * build  (((((of, of), of), of), of), of) where there are dimension of's
 * this is done recursively.  if the dimension is unknown, just return unknown.
 */
static METtype *
MET__op_vector_to_pair(dimension, of)
   int		dimension;
   METtype	*of;
{
   METtype	*guess, *left;
   MME(MET__op_vector_to_pair);
   CHECK_TYPE(of);

   switch (dimension) {

    case TYPE_DIMENSION_UNKNOWN:
      MET_TYPE_NEW_UNKNOWN(guess);
      break;

    case 1:
      MET_TYPE_COPY(guess, of);
      break;

    default:
      left = MET__op_vector_to_pair(dimension - 1, of);
      MET_TYPE_NEW_PAIR(guess, left, of);
      MET_TYPE_FREE(left);
      break;
   }

   return guess;
}

/*
 * take a type which should be a tree of pair nodes, with identical
 * types at the leaves.  leaf_type is set to be a copy of the type that
 * appears at all the leaves.  The number of leaves is returned in
 * dimension.  If the type doesn't fit the proper format, dimension
 * will be negative.  leaf_type will always contain a copy of something.
 */
static void
MET__op_vector_from_pair(type, leaf_type, dimension, sym, bitch)
   METtype	*type;
   METtype	**leaf_type;
   int		*dimension;
   METsym	*sym;
   Boolean	bitch;
{
   METtype	*result_of_cast;
   MME(MET__op_vector_from_pair);
   CHECK_TYPE(type);

   /*
    * if we have already failed, don't bother searching further
    */
   if (*dimension < 0 &&
       MET_NULL_TYPE != *leaf_type)
      return;

   if (MET_TYPE_TYPE_PAIR == type->type) {
      MET__op_vector_from_pair(type->info.pair.left, leaf_type, dimension,
			       sym, bitch);
      MET__op_vector_from_pair(type->info.pair.right, leaf_type, dimension,
			       sym, bitch);
   } else {
      if (MET_NULL_TYPE == *leaf_type) {
	 MET_TYPE_COPY(*leaf_type, type);
	 *dimension = 1;
      } else {
	 if (FAILURE == MET_type_cast(*leaf_type, type, &result_of_cast,
				      TYPE_CAST_MAKE_BOGUS)) {
	    MET_TYPE_FREE(result_of_cast);
	    *dimension = -1;
	    if (YES == bitch)
	       MET_op_cant_cast(sym, *leaf_type, type, CNULL);
	 } else {
	    *dimension += 1;
	    if (result_of_cast != *leaf_type) {
	       MET_TYPE_FREE(*leaf_type);
	       MET_TYPE_COPY(*leaf_type, result_of_cast);
	    }
	    MET_TYPE_FREE(result_of_cast);
	 }
      }
   }
}

Boolean
MET_op_vector_from_pair(sym, pair_type, vector_type)
   METsym 	*sym;
   METtype	*pair_type;
   METtype 	**vector_type;
{
   METtype 	*leaf_type = MET_NULL_TYPE;
   int		arg_dimension = 0;
   ME(MET_op_vector_from_pair);

   MET__op_vector_from_pair(pair_type, &leaf_type, &arg_dimension, sym, YES);
   if (arg_dimension > 0) {
      MET_TYPE_NEW_VECTOR(*vector_type, leaf_type,
			  arg_dimension, MET_SYM_OP_ROW_BUILD == sym->info.op.code);
   } else 
      *vector_type = MET_NULL_TYPE;

   MET_TYPE_FREE(leaf_type);
   return (MET_NULL_TYPE == *vector_type) ? FAILURE : SUCCESS;
}

/*
 * take a bunch of pairs and form them into a vector.  so (a, b) becomes
 * [a, b].  ((a, b), c) becomes [a, b, c]
 */
METset *
MET_op_vector_build(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set, *result_set = MET_NULL_SET;
   METtype	*arg_type = MET_NULL_TYPE, *guess, *my_result_type;
   METtype	*leaf_type;
   int		arg_set_type, op_is_row, col;
   int		arg_dimension, result_dimension;
   METdata	*arg_data, *result_data, *scan_data;
   METverts	*result_verts;
   MME(MET_op_vector_build);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, VECTOR_BUILD_NUM_KIDS);
#endif

   /*
    * if the expected type is an interval, then create one of those instead
    * of a vector
    */
   if (MET_NULL_TYPE != type &&
       ! MET_type_unknown(type) &&
       MET_type_compatible(type, MET_type_interval, NO))
      return MET_op_interval(sym, type, result_type, set_type);

   /*
    * make a guess about our argument's type
    */
   op_is_row = MET_SYM_OP_ROW_BUILD == sym->info.op.code;
   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_VECTOR == type->type) {
      guess = MET__op_vector_to_pair(type->info.vector.dimension,
				     type->info.vector.of);
   } else {
      MET_TYPE_NEW_UNKNOWN(guess);
   }

   /*
    * evaluate our argument
    */
   arg_set = MET_sym_be_set_ref(sym->kids[VECTOR_BUILD_PAIRS], guess,
				&arg_type, &arg_set_type);
   MET_TYPE_FREE(guess);

   /*
    * determine type of result, if we can
    */
   if (MET_NULL_TYPE == arg_type)
      goto bail1;

   leaf_type = MET_NULL_TYPE;
   MET__op_vector_from_pair(arg_type, &leaf_type, &arg_dimension, sym, YES);
   if (arg_dimension > 0 &&
       MET_TYPE_TYPE_SET_OF != arg_type->type) {
      MET_TYPE_NEW_VECTOR(my_result_type, leaf_type,
			  arg_dimension, op_is_row);
   } else {
      if (MET_NULL_SET != arg_set) {
	 if (YES == MET_op_block_only(&arg_set, 1, MET_op_vector_build,
				      sym->info.op.code, type)) {
	    MET_SET_COPY(result_set, arg_set);
	 } else {
	    result_set = MET_op_apply_to_elements(&arg_set, 1,
						  MET_op_vector_build,
						  sym->info.op.code,
						  type, sym);
	 }
      }
      MET_TYPE_FREE(leaf_type);
      goto bail1;
   }

   MET_TYPE_FREE(leaf_type);
   
   /*
    * create the result, if we can.  First create the result set with data
    * arg_dimension times bigger than the first data of arg. (they are all the
    * same, so its ok to use the first) Then fill the data in by chasing the
    * linked list of datas from the arg.  for each of these, copy its verts to
    * the result set.
    */

   if (MET_NULL_TYPE == my_result_type)
      goto bail1;
   if (MET_NULL_SET == arg_set)
      goto bail2;

   MET_op_reconcile_sets(&arg_set, VECTOR_BUILD_NUM_KIDS);

   arg_data = arg_set->info.bare.data;
   result_dimension = arg_dimension * arg_data->num_cols;

   /*
    * special case one dimensional vectors
    */
   if (1 == arg_dimension) {
      MET_DATA_COPY(result_data, arg_data);
   } else {
      result_verts = MET_verts_new(MET_type_base_field(my_result_type, NO),
				   DATA_NUM_VERTS(arg_data),
				   result_dimension);
      result_data = MET_data_new(result_verts, result_dimension, 0);

      col = 0;
      scan_data = arg_data;
      while (MET_NULL_DATA != scan_data) {
	 MET_verts_copy_vertices(result_verts, 0, col, scan_data->verts,
				 0, scan_data->first_col,
				 scan_data->num_cols, result_verts->num_verts,
				 1, 1 == DATA_NUM_VERTS(scan_data));
	 col += scan_data->num_cols;
	 scan_data = scan_data->next;
      }

      MET_VERTS_FREE(result_verts);
   }

   result_set = MET_set_new(MET_SET_TYPE_BARE);
   MET_DATA_COPY(result_set->info.bare.data, result_data);
   MET_DATA_FREE(result_data);
   MET_TYPE_COPY(result_set->info.bare.type, my_result_type);
   result_set->info.bare.how_many = DATA_NUM_VERTS(arg_data);
   MET_tags_initialize(&result_set->info.bare.tags_bare);
   MET_tags_initialize(&result_set->info.bare.tags_in);
   MET_tags_copy(&result_set->info.bare.tags_bare, 
		 &arg_set->info.bare.tags_bare);
   MET_tags_copy(&result_set->info.bare.tags_in, 
		 &arg_set->info.bare.tags_in);

 bail2:
   MET_TYPE_FREE(my_result_type);
 bail1:
   MET_set_get_type(result_set, result_type, set_type);
   if (MET_NULL_SET != arg_set)
      MET_SET_FREE(arg_set);

   if (MET_NULL_TYPE != arg_type)
      MET_TYPE_FREE(arg_type);

   return result_set;
}


#define FRAG_NUM_KIDS	1
#define FRAG_VEC	0


METset *
MET_op_frag(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[FRAG_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[FRAG_NUM_KIDS], dim, i, length;
   METtype	*arg_type[FRAG_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE, *leaf_type;
   METdata	*vdata, *tdata, *td;
   MME(MET_op_frag);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, FRAG_NUM_KIDS);
#endif
   MET_TYPE_NEW_UNKNOWN(guess);
   if (MET_NULL_TYPE != type) {
      leaf_type = MET_NULL_TYPE;
      MET__op_vector_from_pair(type, &leaf_type, &dim, sym, NO);
      if (dim > 0) {
	 MET_TYPE_FREE(guess);
	 MET_TYPE_NEW_VECTOR(guess, leaf_type, dim, NO);
      }
      MET_TYPE_FREE(leaf_type);
   }

   arg_set[FRAG_VEC] =
      MET_sym_be_set_ref(sym->kids[FRAG_VEC], guess,
			 &arg_type[FRAG_VEC],
			 &arg_set_type[FRAG_VEC]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[FRAG_VEC])
      goto bail1;
   
   /*
    * make sure it is a vector
    */
   if (MET_TYPE_TYPE_VECTOR == arg_type[FRAG_VEC]->type) {
      my_result_type =
	 MET__op_vector_to_pair(dim =
				arg_type[FRAG_VEC]->info.vector.dimension,
				arg_type[FRAG_VEC]->info.vector.of);
   } 

   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "argument to op frag is not a vector", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, FRAG_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_UNKNOWN))
       goto bail2;

   MET_op_reconcile_sets(arg_set, FRAG_NUM_KIDS);

   MET_DATA_COPY(vdata, arg_set[FRAG_VEC]->info.bare.data);
   result_set = MET_set_new(MET_SET_TYPE_BARE);
   result_set->info.bare.how_many = arg_set[FRAG_VEC]->info.bare.how_many;
   MET_tags_initialize(&result_set->info.bare.tags_bare);
   MET_tags_initialize(&result_set->info.bare.tags_in);
   MET_tags_copy(&result_set->info.bare.tags_bare, 
		 &arg_set[FRAG_VEC]->info.bare.tags_bare);
   MET_tags_copy(&result_set->info.bare.tags_in, 
		 &arg_set[FRAG_VEC]->info.bare.tags_in);
   result_set->info.bare.data = MET_NULL_DATA;
   MET_TYPE_COPY(result_set->info.bare.type, my_result_type);
   length = vdata->num_cols / dim;
   for (i = dim - 1; i >= 0; i--) {
      tdata = MET_data_new(vdata->verts, length,
			   vdata->first_col + (i * length));
      if (MET_NULL_DATA != (td = result_set->info.bare.data)) {
	 MET_DATA_COPY(tdata->next, td);
	 MET_DATA_FREE(td);
      }
      MET_DATA_COPY(result_set->info.bare.data, tdata);
      MET_DATA_FREE(tdata);
   }
   MET_DATA_FREE(vdata);
   
   MET_TYPE_FREE(my_result_type);
   
 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(FRAG_VEC);

   return result_set;
}
