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
   "$Id: op_interval.c,v 1.16 1992/06/22 17:06:32 mas Exp $";
#endif

#include "defs.h"

#define INTERVAL_LOWER		0
#define INTERVAL_UPPER		1
#define INTERVAL_SAMPLES	2

static METset *
MET__op_interval_set_vertices(set, lower, upper, samples)
   METset	*set;
   Real		lower, upper;
   int		samples;
{
   METreal	*scan;
   METdata	*data;
   int		width, i;
   Real		t;
   MME(MET_op_interval_set_vertices);

   data = set->info.block.block_list->data;
   scan = DATA_FIRST_VERTEX(data, METreal);
   width = data->verts->total_cols;
   if (1 == samples)
      t = 1.0;
   else
      t = (upper - lower) / (samples - 1);

   if (1 != samples)
      for (i = 0; i < DATA_NUM_VERTS(data); i++) {
	 *scan = (METreal) (lower + i * t);
	 scan += width;
      }
   else
      *scan = (METreal) (lower);
}

static METset *
MET__op_interval_flat(arg_set, arg_type, arg_set_type, has_third_arg,
		      type_guess, sym)
   METset	*arg_set[3];
   METtype	*arg_type[3];
   int		arg_set_type[3];
   Boolean	has_third_arg;
   METtype	*type_guess;
   METsym	*sym;
{
   METset	*result = MET_NULL_SET;
   METtype	*my_result_type, *t;
   Real		upper_real, lower_real;
   int		samples_int, square_mesh = MET_CONNEX_TYPE_MESH;
   METconnex	*connex;
   MME(MET_op_interval_flat);

   if (MET_NULL_TYPE == arg_type[INTERVAL_LOWER]) {
      goto bail;
   }
   if (MET_NULL_SET == arg_set[INTERVAL_LOWER]) {
      goto bail;
   }
   if (MET_TYPE_TYPE_FIELD != arg_type[INTERVAL_LOWER]->type ||
       MET_SET_TYPE_BLOCK == arg_set_type[INTERVAL_LOWER]) {
      goto bail;
   }
   if (MET_FIELD_INTEGER == arg_type[INTERVAL_LOWER]->info.field.type)
      lower_real =
	 (Real) *DATA_FIRST_VERTEX(arg_set[INTERVAL_LOWER]->info.bare.data,
				   METinteger);
   else if (MET_FIELD_REAL == arg_type[INTERVAL_LOWER]->info.field.type)
      lower_real =
	 (Real) *DATA_FIRST_VERTEX(arg_set[INTERVAL_LOWER]->info.bare.data,
				   METreal);
   else {
      ERROR(sym->origin, "lower bound of interval is bad", SEV_FATAL);
      goto bail;
   }
   
   if (MET_NULL_TYPE == arg_type[INTERVAL_UPPER]) {
      goto bail;
   }
   if (MET_NULL_SET == arg_set[INTERVAL_UPPER]) {
      goto bail;
   }
   if (MET_TYPE_TYPE_FIELD != arg_type[INTERVAL_UPPER]->type ||
       MET_SET_TYPE_BLOCK == arg_set_type[INTERVAL_UPPER]) {
      goto bail;
   }
   if (MET_FIELD_INTEGER == arg_type[INTERVAL_UPPER]->info.field.type)
      upper_real =
	 (Real) *DATA_FIRST_VERTEX(arg_set[INTERVAL_UPPER]->info.bare.data,
				   METinteger);
   else if (MET_FIELD_REAL == arg_type[INTERVAL_UPPER]->info.field.type)
      upper_real =
	 (Real) *DATA_FIRST_VERTEX(arg_set[INTERVAL_UPPER]->info.bare.data,
				   METreal);
   else {
      ERROR(sym->origin, "upper bound of interval is bad", SEV_FATAL);
      goto bail;
   }
   
   if (lower_real > upper_real)
      goto bail;
   
   if (has_third_arg) {
      if (MET_NULL_TYPE == arg_type[INTERVAL_SAMPLES]) {
	 goto bail;
      }
      if (MET_NULL_SET == arg_set[INTERVAL_SAMPLES]) {
	 goto bail;
      }
      if (MET_TYPE_TYPE_FIELD != arg_type[INTERVAL_SAMPLES]->type ||
	  MET_FIELD_INTEGER != arg_type[INTERVAL_SAMPLES]->info.field.type ||
	  MET_SET_TYPE_BLOCK == arg_set_type[INTERVAL_SAMPLES]) {
	 ERROR(sym->origin, "samples of interval is bad", SEV_FATAL);
	 goto bail;
      }
      samples_int =
	 (int) *DATA_FIRST_VERTEX(arg_set[INTERVAL_SAMPLES]->info.bare.data,
				  METinteger);
      /*
       * make sure the number of samples is 2 or greater
       */
      if (1 > samples_int) {
	 ERROR(sym->origin, "samples of interval is < 1", SEV_FATAL);
	 goto bail;
      }
   } else {
      samples_int = MET_defaults->interval_resolution.resolution;
   }
   samples_int = (int) (samples_int *
			MET_defaults->interval_resolution.scale);
   if (1 > samples_int)
      samples_int = 1;

   if (MET_SET_TYPE_BARE != arg_set_type[INTERVAL_LOWER] ||
       MET_SET_TYPE_BARE != arg_set_type[INTERVAL_UPPER] ||
       (has_third_arg && (MET_SET_TYPE_BARE !=
			  arg_set_type[INTERVAL_SAMPLES]))) {
      result = MET_NULL_SET;
   } else {

      if (MET_op_bare_only(arg_set, has_third_arg ? 3 : 2,
			   MET_op_interval, sym->info.op.code, type_guess)) {
	 MET_SET_COPY(result, arg_set[0]);
      } else {
	 if (lower_real == upper_real)
	    samples_int = 1;
	 
	 MET_TYPE_NEW_FIELD(t, MET_FIELD_REAL);
	 MET_TYPE_NEW_SET_OF(my_result_type, t);
	 MET_TYPE_FREE(t);
	 
	 connex = MET_connex_new(1, &samples_int, &square_mesh);
	 
	 result =
	    MET_type_to_block_set(my_result_type, samples_int, connex);
	 MET_TYPE_FREE(my_result_type);
	 MET_CONNEX_FREE(connex);
	 
	 MET__op_interval_set_vertices(result, lower_real, upper_real,
				       samples_int);
      }
   }

 bail:

   return result;
}
   

/*
  interval takes 2 real numbers and returns the closed interval of the real
  line bounded by those numbers.  An optional third parameter specifies how
  many points to generate along the interval.  It has some random default
  value, probably about 30.

  it can also take just one arg that is an ordered pair. (not yet)
*/

METset *
MET_op_interval(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[3], *result;
   METtype	*arg_type[3], *reals, *integers, *pair;
   int		arg_set_type[3];
   Boolean	has_third_arg;
   MME(MET_op_interval);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, ANY_NUMBER_OF_KIDS);
#endif

   has_third_arg = (sym->num_kids == 3);

   MET_TYPE_NEW_FIELD(reals, MET_FIELD_REAL);
   MET_TYPE_NEW_FIELD(integers, MET_FIELD_INTEGER);

   if (1 == sym->num_kids) {
      NYET;
      MET_TYPE_NEW_PAIR(pair, reals, reals);
      /*
       * just using INTERVAL_SAMPLES so I don't have to declare 3 more
       * variables
       */
      arg_set[INTERVAL_SAMPLES] =
	 MET_sym_be_set_ref(sym->kids[INTERVAL_SAMPLES], pair,
			    &arg_type[INTERVAL_SAMPLES],
			    &arg_set_type[INTERVAL_SAMPLES]);
      MET_TYPE_FREE(pair);
      if (MET_NULL_SET == arg_set[INTERVAL_SAMPLES]) {
	 arg_type[INTERVAL_LOWER] = arg_type[INTERVAL_UPPER] = MET_NULL_TYPE;
	 arg_set[INTERVAL_LOWER] = arg_set[INTERVAL_UPPER] = MET_NULL_SET;
      } else {
	 /*
	  * make sure we really got a pair
	  */
	 if (MET_TYPE_TYPE_PAIR != arg_type[INTERVAL_SAMPLES]->type) {
	    MET_SET_FREE(arg_set[INTERVAL_SAMPLES]);
	    arg_set[INTERVAL_SAMPLES] = MET_NULL_SET;

	    MET_TYPE_FREE(arg_type[INTERVAL_SAMPLES]);
	    arg_type[INTERVAL_SAMPLES] = MET_NULL_TYPE;
	    /*
	     * perhaps I should call the array op here?
	     */ /* Ack! */
	 } else {
	    /*
	     * pretend like we got two args
	     */
	    MET_TYPE_COPY(arg_type[INTERVAL_LOWER],
			  arg_type[INTERVAL_SAMPLES]->info.pair.left);
	    MET_TYPE_COPY(arg_type[INTERVAL_UPPER],
			  arg_type[INTERVAL_SAMPLES]->info.pair.right);
	    MET_TYPE_FREE(arg_type[INTERVAL_SAMPLES]);

	    NYET;
	 }
	    
	    
	 /*
	  * now split up the pair to look as if we had two arguments
	  */
	 
      }
   } else {
      arg_set[INTERVAL_LOWER] =
	 MET_sym_be_set_ref(sym->kids[INTERVAL_LOWER], reals,
			    &arg_type[INTERVAL_LOWER],
			    &arg_set_type[INTERVAL_LOWER]);
      
      arg_set[INTERVAL_UPPER] =
	 MET_sym_be_set_ref(sym->kids[INTERVAL_UPPER], reals,
			    &arg_type[INTERVAL_UPPER],
			    &arg_set_type[INTERVAL_UPPER]);
      if (has_third_arg)
	 arg_set[INTERVAL_SAMPLES] =
	    MET_sym_be_set_ref(sym->kids[INTERVAL_SAMPLES], integers,
			       &arg_type[INTERVAL_SAMPLES],
			       &arg_set_type[INTERVAL_SAMPLES]);
   }

   MET_TYPE_FREE(reals);
   MET_TYPE_FREE(integers);

   result = MET__op_interval_flat(arg_set, arg_type, arg_set_type,
				  has_third_arg, type, sym);

   MET_set_get_type(result, result_type, set_type);


   if (has_third_arg)
      COND_FREE(INTERVAL_SAMPLES);
   COND_FREE(INTERVAL_LOWER);
   COND_FREE(INTERVAL_UPPER);
   
   return result;
}

static METset *
MET__op_id_matrix(type, sym)
   METtype	*type;
   METsym	*sym;
{
   METset	*result;
   int		size, i, j;
   METinteger	*integer;
   METreal	*real;
   METcomplex	*complex;
   METdata	*data;
   MME(MET__op_id_matrix);

   size = type->info.vector.dimension;

   if (size != type->info.vector.of->info.vector.dimension) {
      ERROR(sym->origin, "can't form non-square identity matrix", SEV_FATAL);
      return MET_NULL_SET;
   }

   result = MET_type_to_polybare_set(type, 1, &MET_set_zero, 1);
   data = result->info.bare.data;
   switch (data->verts->type) {
    case MET_FIELD_ZTWO:
      MET_SET_FREE(result);
      result = MET_NULL_SET;
      break;
    case MET_FIELD_INTEGER:
      integer = DATA_FIRST_VERTEX(data, METinteger);
      for (i = 0; i < size; i++)
	 for (j = 0; j < size; j++)
	    *integer++ = (i == j) ? 1 : 0;
      break;
    case MET_FIELD_REAL:
      real = DATA_FIRST_VERTEX(data, METreal);
      for (i = 0; i < size; i++)
	 for (j = 0; j < size; j++)
	    *real++ = (i == j) ? 1.0 : 0.0;
      break;
    case MET_FIELD_COMPLEX:
      complex = DATA_FIRST_VERTEX(data, METcomplex);
      for (i = 0; i < size; i++)
	 for (j = 0; j < size; j++) {
	    complex->real = (i == j) ? 1.0 : 0.0;
	    complex->imaginary = 0.0;
	    complex++;
	 }
      break;
    case MET_FIELD_MAP:
      MET_SET_FREE(result);
      result = MET_NULL_SET;
      break;
    case MET_FIELD_SET:
      MET_SET_FREE(result);
      result = MET_NULL_SET;
      break;
    default:
      DEFAULT_ERR(data->verts->type);
   }
   return result;
}

METset *
MET_op_set_null(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   int		code = sym->info.op.code;
   METsym	*tsym;
   METset	*result = MET_NULL_SET, *tset;
   MME(MET_op_set_null);

   if (MET_SYM_OP_SET_NULL == code) {
      MET_SET_COPY(result, MET_set_null);
   } else if (MET_SYM_OP_IDENTITY == code &&
       MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_MAP == type->type) {
      tsym = MET_sym_new_variable(SYM_VARIABLE_ID_PARM, RNULL);
      sym = METsym_new_op(MET_SYM_OP_MAP, RNULL);
      METsym_add_kid(sym, tsym);
      METsym_add_kid(sym, tsym);
      MET_SYM_FREE(tsym);
      result = METset_new_with_definition(sym, MET_NULL_ENV);
      MET_SYM_FREE(sym);
      (void) METset_have_rep(result, BNULL);
   } else if (MET_NULL_TYPE == type ||
	 YES == MET_type_unknown(type)) {
      if (MET_SYM_OP_ZERO == code) {
	 MET_SET_COPY(result, MET_set_zero);
      } else {
	 MET_SET_COPY(result, MET_set_one);
      }
   } else if (MET_SYM_OP_ZERO == code) {
      tset = MET_type_to_polybare_set(type, 1, &MET_set_zero, 1);
      result = MET_set_zeroed(tset);
      MET_SET_FREE(tset);
   } else if (MET_TYPE_TYPE_FIELD == type->type) {
      MET_SET_COPY(result, MET_set_one);
   } else if (MET_TYPE_TYPE_VECTOR == type->type &&
	      MET_TYPE_TYPE_VECTOR == type->info.vector.of->type &&
	      (MET_TYPE_TYPE_FIELD ==
	       type->info.vector.of->info.vector.of->type)) {
      result = MET__op_id_matrix(type, sym);
   }
      
      
      
      

   MET_set_get_type(result, result_type, set_type);
   return result;
}
