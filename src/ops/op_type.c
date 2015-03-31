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
   "$Id: op_type.c,v 1.13 1992/06/22 17:07:26 mas Exp $";
#endif

#include "defs.h"

#define TYPE_TYPE		0
#define TYPE_EXPR		1
#define TYPE_NUM_KIDS		2

METtype *
MET__op_type_eval_type(sym, natural)
   METsym	*sym;
   Boolean	*natural;
{
   METtype	*guess, *result, *explicit_type;
   METset	*partial_result;
   MME(MET__op_type_eval_type);
   
   MET_TYPE_NEW_SET(guess);
   partial_result = MET_sym_be_set_ref(sym, guess, &explicit_type, INULL);
   MET_TYPE_FREE(guess);

   if (MET_NULL_SET == partial_result) {
      MET_TYPE_FREE(explicit_type);
      return MET_NULL_TYPE;
   }

   if (MET_SET_TYPE_TYPE == partial_result->type) {
      MET_TYPE_COPY(result, partial_result->info.type.type);
      *natural = YES;
   } else {
      MET_TYPE_COPY(result, explicit_type);
      *natural = NO;
   }
   MET_SET_FREE(partial_result);
   MET_TYPE_FREE(explicit_type);

   return result;
}

METset *
MET_op_type(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METtype	*cast_to;
   METset	*result;
   Boolean	dummy;
   MME(MET_op_type);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, 1);
#endif

   cast_to = MET__op_type_eval_type(sym->kids[TYPE_TYPE], &dummy);

   if (MET_NULL_TYPE != cast_to) {
      result = MET_set_new(MET_SET_TYPE_TYPE);
      MET_TYPE_COPY(result->info.type.type, cast_to);
      MET_TYPE_FREE(cast_to);
   } else {
      result = MET_NULL_SET;
   }

 bail:
   MET_set_get_type(result, result_type, set_type);
   return result;
}

METset *
MET_op_type_and_cast(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METtype	*explicit_type, *t;
   METset	*result = MET_NULL_SET, *partial_result;
   Boolean	natural;
   MME(MET_op_type_and_cast);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, TYPE_NUM_KIDS);
#endif

   t = MET__op_type_eval_type(sym->kids[TYPE_TYPE], &natural);
   if (MET_NULL_TYPE == t)
      goto bail;
      
   if (NO == natural) {
      /*
       * strip a {} of a type-by-example arg
       */
      if (MET_TYPE_TYPE_SET_OF != t->type) {
	 ERROR(sym->origin,
	       "non-type objects in type exprs must be sets",
	       SEV_FATAL);
	 MET_TYPE_FREE(t);
	 goto bail;
      }
      MET_TYPE_COPY(explicit_type, t->info.set_of.of);
   } else {
      MET_TYPE_COPY(explicit_type, t);
   }
   MET_TYPE_FREE(t);

   partial_result = MET_sym_be_set_ref(sym->kids[TYPE_EXPR],
				       explicit_type,
				       result_type, set_type);
   if (partial_result == MET_NULL_SET) {
      MET_TYPE_FREE(explicit_type);
      goto bail;
   }
   
   result = MET_set_cast_to_type(partial_result, explicit_type);
   MET_TYPE_FREE(explicit_type);

   if (MET_NULL_SET == result) {
      ERROR(sym->origin,
	    "in type-cast op, object could not be cast to requested type",
	    SEV_FATAL);
      MET_SET_FREE(partial_result);
      goto bail;
   }
   
   if (result != partial_result &&
       (METtype **) NULL != result_type) {
      MET_TYPE_FREE(*result_type);
      MET_set_get_type(result, result_type, set_type);
   }
   MET_SET_FREE(partial_result);
   return result;

 bail:
   MET_set_get_type(result, result_type, set_type);
   return result;
}

/*
 * return true if the right type can be cast up to the left.
 */
METset *
MET_op_type_match(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METtype	*left_type, *right_type;
   METset	*result;
   METnumber	num;
   Boolean	dummy;
   MME(MET_op_type_and_cast);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, TYPE_NUM_KIDS);
#endif

   left_type =  MET__op_type_eval_type(sym->kids[0], &dummy);
   right_type = MET__op_type_eval_type(sym->kids[1], &dummy);

   if (MET_NULL_TYPE == left_type ||
       MET_NULL_TYPE == right_type) {
      MET_set_get_type(MET_NULL_SET, result_type, set_type);
      result = MET_NULL_SET;
   } else {

      num.ztwo = MET_type_compatible(right_type, left_type,
				     sym->info.op.code !=
				     MET_SYM_OP_TYPE_MATCH_X);

      MET_TYPE_FREE(left_type);
      MET_TYPE_FREE(right_type);

      result = METset_new_number(MET_FIELD_ZTWO, &num);
      MET_set_get_type(result, result_type, set_type);
   }
   return result;
}

METset *
MET_op_type_unknown(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*result;
   MME(MET_op_type_unknown);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, 0);
#endif

   result = MET_set_new(MET_SET_TYPE_TYPE);
   MET_TYPE_NEW_UNKNOWN(result->info.type.type);
   MET_set_get_type(result, result_type, set_type);
   return result;
}
