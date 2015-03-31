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
   "$Id: op_card.c,v 1.3 1992/06/22 17:05:56 mas Exp $";
#endif

#include "defs.h"

#define CARD_NUM_KIDS	1
#define CARD_SET	0


static int
MET__op_card_connex(connex)
   METconnex	*connex;
{
   int		i;
   int		pts = 1;
   int		dims = 0;
   MME(MET_connex_card);

   for (i = 0; i < connex->ndims; i++)

      if (connex->dims[i] < 0) {
	 pts *= -connex->dims[i];
      } else
	 switch (connex->types[i]) {

	  case MET_CONNEX_TYPE_MESH:
	    dims += 1;
	    break;

	  case MET_CONNEX_TYPE_TRIANGLE:
	    dims += 2;
	    break;

	  default:
	    DEFAULT_ERR(connex->types[i]);
	 }

   if (dims > 0)
      return -dims;
   else
      return pts;
}
/*
 * takes a  set and returns how many elements are in it.  If it is
 * continuous, then it returns a negative number that is the dimension, eg
 * line -> -1, surface -> -2 etc.  Should this have syntax '#x'?
 */

METset *
MET_op_card(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[CARD_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[CARD_NUM_KIDS], count;
   METtype	*arg_type[CARD_NUM_KIDS], *t, *guess, *my_result_type;
   METblock	*block;
   METnumber	num;
   MME(MET_op_card);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, CARD_NUM_KIDS);
#endif

   MET_TYPE_NEW_UNKNOWN(t);
   MET_TYPE_NEW_SET_OF(guess, t);
   MET_TYPE_FREE(t);

   arg_set[CARD_SET] =
      MET_sym_be_set_ref(sym->kids[CARD_SET], type,
			 &arg_type[CARD_SET],
			 &arg_set_type[CARD_SET]);

   MET_TYPE_FREE(guess);


   if (MET_NULL_TYPE == arg_type[CARD_SET])
      goto bail1;

   if (arg_type[CARD_SET]->type != MET_TYPE_TYPE_SET_OF) {
      MET_op_error_bad_arg_type(1, arg_type[CARD_SET], "{?}", sym);
      goto bail1;
   }

   MET_TYPE_NEW_FIELD(my_result_type, MET_FIELD_INTEGER);

   if (MET_op_type_only(arg_set, CARD_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BLOCK))
      goto bail2;

   if (MET_op_block_only(arg_set, CARD_NUM_KIDS, MET_op_card,
			 MET_SYM_OP_CARDINALITY, my_result_type)) {
      MET_SET_COPY(result_set, arg_set[0]);
      MET_TYPE_FREE(my_result_type);
      MET_set_get_type(result_set, result_type, set_type);
      goto bail2;
   }

   MET_TYPE_FREE(my_result_type);

   if (MET_NULL_SET == arg_set[CARD_SET])
      goto bail1;

   count = 0;
   block = arg_set[CARD_SET]->info.block.block_list;

   while (MET_NULL_BLOCK != block) {
      int t;

      t = MET__op_card_connex(block->connex);
      if (t < 0) {
	 if (t < count)
	    count = t;
      } else {
	 if (count >= 0)
	    count += t;
      }

      block = block->next;
   }

   num.integer = count;
   result_set = METset_new_number(MET_FIELD_INTEGER, &num);

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(CARD_SET);

   return result_set;
}
