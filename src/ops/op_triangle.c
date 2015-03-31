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
   "$Id: op_triangle.c,v 1.6 1992/06/22 17:07:23 mas Exp $";
#endif

#include "defs.h"

#define TRIANGLE_NUM_KIDS 2
#define TRIANGLE_MATRIX 0
#define TRIANGLE_SAMPLES 1

static void
MET__op_triangle_set_vertices(set, vertices, vert_dims, samples)
   METset *set;
   METreal *vertices;
   int vert_dims;
   int samples;
{
   METreal *scan;
   METdata *data;
   int width, i, j, k;
   METreal *first, *second, *third;
   Real *d1, *d2;
   Real stat_d1[MET_CONNEX_STATIC_SIZE];
   Real stat_d2[MET_CONNEX_STATIC_SIZE];

   MME(MET__op_triangle_set_vertices);

   first = vertices;
   second = first + vert_dims;
   third = second + vert_dims;

   if (vert_dims > MET_CONNEX_STATIC_SIZE)
   {
      ALLOCNM(d1, Real, vert_dims);
      ALLOCNM(d2, Real, vert_dims);
   }
   else
   {
      d1 = stat_d1;
      d2 = stat_d2;
   }

   data = set->info.block.block_list->data;
   scan = DATA_FIRST_VERTEX(data, METreal);
   width = data->verts->total_cols;

   for(i=0; i<vert_dims; i++)
   {
      d1[i] = (Real)(second[i] - first[i])/(samples - 1);
      d2[i] = (Real)(third[i] - first[i])/(samples - 1);
   }
   
   if (1 != samples )
      for (i=0; i<samples; i++)
	 for (j=0; j<samples-i; j++)
	 {
	    for (k=0; k<vert_dims; k++)
	       scan[k] = (METreal) 
		  ( (METreal) i * d1[k] + (METreal) j * d2[k] + first[k] );
	    scan += width;
	 }
   else
      for (k=0; k<vert_dims; k++)
	 scan[k] = (METreal) (first[k]);
   
   if (MET_CONNEX_STATIC_SIZE < vert_dims)
   {
      FREEN(d1, Real, vert_dims);
      FREEN(d2, Real, vert_dims);
   }
}

static METset *
MET__op_triangle(data, samples, my_result_type, are_reals)
   METdata	*data;
   METdata 	*samples;
   METtype	*my_result_type;
   Boolean	are_reals;

{
   METset	*result;
   METconnex	*connex;
   Boolean	has_second_arg = (samples != MET_NULL_DATA);
   int		samples_int, tri_mesh = MET_CONNEX_TYPE_TRIANGLE;
   int		vert_dims;
   int 		i;
   METreal	*vertices;
   MME(MET__op_triangle);

   if (are_reals)
   {
      vertices  = DATA_FIRST_VERTEX(data, METreal);
   }
   else
   {
      ALLOCNM(vertices, METreal, data->num_cols);
      for(i=0; i<data->num_cols; i++)
	 vertices[i] = (METreal) (DATA_FIRST_VERTEX(data, METinteger))[i];
   }

   vert_dims = data->num_cols / 3;

   if (has_second_arg) {
      samples_int =
	 (int) *DATA_FIRST_VERTEX(samples, METinteger);
      if (samples_int < 2) {
	 ERROR(RNULL, "must have at least 2 samples in triangle",
	       SEV_WARNING);
      }
   } else {
      samples_int = 2;
   }

   samples_int = (int) (samples_int *
			MET_defaults->interval_resolution.scale);
   if (2 > samples_int)
      samples_int = 2;

   connex = MET_connex_new(1, &samples_int, &tri_mesh);
   result = MET_type_to_block_set(my_result_type, 
				  ( samples_int * (samples_int + 1) / 2 ),
				  connex);
   MET_CONNEX_FREE(connex);
   MET__op_triangle_set_vertices(result, vertices, vert_dims, samples_int);

   if (!are_reals)
      FREEN(vertices, METreal, data->num_cols);

   return result;
}


/*
  Triangle takes a 3><3 matrix and an integer and returns a triangular
  mesh whose vertices are the rows of the 3><3 matrix, which is subdivided
  into the correct number of segments along each edge.  If no integer
  is passed, the default makes the edges unsubdivided.
*/

METset *
MET_op_triangle(sym, type, result_type, set_type)
   METsym 	*sym;
   METtype 	*type, **result_type;
   int 		*set_type;
{
   METset	*arg_set[TRIANGLE_NUM_KIDS], *result = MET_NULL_SET;
   METtype	*arg_type[TRIANGLE_NUM_KIDS], *matrix, *integers;
   METtype	*t, *my_result_type;
   int		arg_set_type[TRIANGLE_NUM_KIDS];
   Boolean	has_second_arg;
   Boolean 	are_reals;
   MME(MET_op_triangle);
#ifdef MET__DEBUG
   MET_op_check_parms(sym, type, result_type, ANY_NUMBER_OF_KIDS);
#endif

   has_second_arg = (sym->num_kids == 2);

   /*
    * generate a type for a matrix, and a scalar integer.
    */
   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_SET_OF == type->type) {
      MET_TYPE_NEW_VECTOR(matrix, type->info.set_of.of, 3, NO);
   } else {
      MET_TYPE_NEW_UNKNOWN(matrix);
   }

   MET_TYPE_NEW_FIELD(integers, MET_FIELD_INTEGER);

   arg_set[TRIANGLE_MATRIX] =
      MET_sym_be_set_ref(sym->kids[TRIANGLE_MATRIX], matrix,
			 &arg_type[TRIANGLE_MATRIX],
			 &arg_set_type[TRIANGLE_MATRIX]);
   

   if (has_second_arg)
      arg_set[TRIANGLE_SAMPLES] =
	 MET_sym_be_set_ref(sym->kids[TRIANGLE_SAMPLES], integers,
			    &arg_type[TRIANGLE_SAMPLES],
			    &arg_set_type[TRIANGLE_SAMPLES]);

   MET_TYPE_FREE(matrix);
   MET_TYPE_FREE(integers);

   if (MET_NULL_TYPE == arg_type[TRIANGLE_MATRIX] ||
       has_second_arg && (MET_NULL_TYPE == arg_type[TRIANGLE_SAMPLES]))
      goto bail;

   if ((MET_TYPE_TYPE_VECTOR != (t = arg_type[TRIANGLE_MATRIX])->type) ||
       (3 != t->info.vector.dimension) ||
       (MET_TYPE_TYPE_VECTOR != (t = t->info.vector.of)->type) ||
       (MET_TYPE_TYPE_FIELD != t->info.vector.of->type))
   {
      ERROR(sym->origin,
	    "first argument to triangle must be a 3 by n matrix",
	    SEV_FATAL);
      goto bail;
   }

   t = arg_type[TRIANGLE_MATRIX]->info.vector.of->info.vector.of;
   if ((MET_FIELD_REAL != t->info.field.type) &&
       (MET_FIELD_INTEGER != t->info.field.type))
   {
      ERROR(sym->origin, 
	    "triangle can only operate on matrices of reals or integers",
	    SEV_FATAL);
      goto bail;
   }
   else
      are_reals = (MET_FIELD_REAL == t->info.field.type) ?
	 YES : NO;

   if (has_second_arg)
      if (MET_TYPE_TYPE_FIELD != (t = arg_type[TRIANGLE_SAMPLES])->type ||
	  MET_FIELD_INTEGER != t->info.field.type) {
	 ERROR(sym->origin, "second argument to triangle must be an integer",
	       SEV_FATAL);
	 goto bail;
      }
   
   if (MET_SET_TYPE_BARE != arg_set_type[TRIANGLE_MATRIX] ||
       (has_second_arg && 
	MET_SET_TYPE_BARE != arg_set_type[TRIANGLE_SAMPLES]))
   {
      result = MET_NULL_SET;
      goto bail;
   }
      
   /* 
    * build the type of our answer.
    */

   MET_TYPE_NEW_FIELD(t, MET_FIELD_REAL);
   MET_TYPE_NEW_VECTOR(matrix, t, 
		       arg_type[TRIANGLE_MATRIX]->
		       info.vector.of->info.vector.dimension, NO);
   MET_TYPE_FREE(t);
   MET_TYPE_NEW_SET_OF(my_result_type, matrix);
   MET_TYPE_FREE(matrix);

   if (MET_op_type_only(arg_set, TRIANGLE_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BLOCK))
      goto bail2;
   MET_op_reconcile_sets(arg_set, TRIANGLE_NUM_KIDS);

   if (MET_op_bare_only(arg_set, has_second_arg ? 2 : 1,
			MET_op_triangle, MET_SYM_OP_TRIANGLE, type))
   {
      MET_SET_COPY(result, arg_set[0]);
   }
   else if (has_second_arg)
      result = MET__op_triangle(arg_set[TRIANGLE_MATRIX]->info.bare.data,
				arg_set[TRIANGLE_SAMPLES]->info.bare.data,
				my_result_type, are_reals);
   else
      result = MET__op_triangle(arg_set[TRIANGLE_MATRIX]->info.bare.data,
				MET_NULL_DATA, my_result_type, are_reals);
   
   MET_set_get_type(result, result_type, set_type);

 bail2:
   MET_TYPE_FREE(my_result_type);
   
 bail:
   if (has_second_arg)
      COND_FREE(TRIANGLE_SAMPLES);
   COND_FREE(TRIANGLE_MATRIX);

   return result;
}
   
