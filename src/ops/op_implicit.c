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
   "$Id: op_implicit.c,v 1.7 1992/06/22 17:06:28 mas Exp $";
#endif

#include "defs.h"

#define IMPLICIT_NUM_KIDS	2
#define IMPLICIT_SET		0
#define IMPLICIT_FN		1

/*
 * takes a discrete set, a boolean-valued function, and
 * returns those elements that pass the test.
 *
 * eventually this will be extended to handle continuous sets.  but that's
 * a bit harder :)
 */


static METblock *
MET__op_implicit_choose(block, pred_block)
   METblock *block, *pred_block;
{
   METblock	*result;
   METverts	*verts;
   int		i, j, width, count;
   METztwo	*scan;
   METdata	*data, *pred_data, **last;
   MME(MET__op_implicit_choose);


   pred_data = pred_block->data;
   scan = DATA_FIRST_VERTEX(pred_data, METztwo);
   width = (1 == DATA_NUM_VERTS(pred_data)) ? 0 :
      pred_data->verts->total_cols;

   /* assumes 0/1 for false/true */
   count = 0;
   for (i = 0; i < pred_data->verts->num_verts; i++) {
      count += (int) *scan;
      scan += width;
   }

   /* noone passes the pred */
   if (0 == count)
      return MET_NULL_BLOCK;

   MET_BLOCK_ALLOC(result);
   last = &result->data;

   data = block->data;
   while (MET_NULL_DATA != data) {

      verts = MET_verts_new(data->verts->type,
			    count, data->num_cols);

      j = 0;
      scan = DATA_FIRST_VERTEX(pred_data, METztwo);
      for (i = 0; i < pred_data->verts->num_verts; i++) {
	 if (*scan) {
	    MET_verts_copy_vertices(verts, j, 0,
				    data->verts, i, data->first_col,
				    data->num_cols, 1, 1, NO);
	    j++;
	 }
	 scan += width;
      }

      *last = MET_data_new(verts, data->num_cols, 0);
      MET_VERTS_FREE(verts);
      last = &(*last)->next;
      data = data->next;
   }

   *last = MET_NULL_DATA;
   
   result->next = MET_NULL_BLOCK;
   MET_TYPE_COPY(result->type, block->type);
   count = -count;
   i = MET_CONNEX_TYPE_MESH;
   result->connex = MET_connex_new(1, &count, &i);
   MET_CONNEX_COPY(result->data_connex, result->connex);
   if (0 != block->tags_in.num_tags) {
      printf("false scalar in implicit\n");
      NYET;
   }
   MET_tags_initialize(&result->tags_in);
   MET_tags_initialize(&result->tags_part);
   return result;
}

static METset *
MET__op_implicit_guts(arg_set, arg_type, loc_sym)
   METset	**arg_set;
   METtype	**arg_type;
   METsym	*loc_sym;
{
   METset	*result, *preds, **vert, **pred;
   METblock	*block, *pred_block, *new_block;
   METtype	*boolean, *answer;
   int		i;
   MME(MET__op_implicit_guts);

   MET_TYPE_NEW_FIELD(boolean, MET_FIELD_ZTWO);
   MET_TYPE_NEW_SET_OF(answer, boolean);
   MET_TYPE_FREE(boolean);

   preds = MET_op_apply_to_set(arg_set[IMPLICIT_FN],
			       arg_set[IMPLICIT_SET],
			       answer, (METtype **) NULL,
			       INULL, MET_TAG_TYPE_DOUBLE_MAP,
			       loc_sym);
   MET_TYPE_FREE(answer);

   if (MET_NULL_SET == preds)
      return MET_NULL_SET;

   block = arg_set[IMPLICIT_SET]->info.block.block_list;

   if (MET_NULL_BLOCK != block) {

      if (MET_NULL_BLOCK != block->next) {
	 ERROR(loc_sym->origin,
	       "implicit can only handle homogeneous sets",
	       SEV_FATAL);
	 result = MET_NULL_SET;
	 goto bail;
      }
      
      if (MET_SET_TYPE_BLOCK == preds->type) {
	 result = MET_set_new(MET_SET_TYPE_BLOCK);
	 result->info.block.block_list = MET_NULL_BLOCK;
	 
	 pred_block = preds->info.block.block_list;
	 new_block = MET__op_implicit_choose(block, pred_block);
	 
	 if (MET_NULL_BLOCK != new_block) {
	    MET_set_add_block(result, new_block);
	    MET_BLOCK_FREE(new_block);
	 }
      } else {
	 result = 
	    MET_type_to_polybare_set(block->type,
				     DATA_NUM_VERTS(preds->info.bare.data),
				     &preds, 1); 
	 vert = DATA_FIRST_VERTEX(result->info.bare.data, METset *);
	 pred = DATA_FIRST_VERTEX(preds->info.bare.data, METset *);
	 for (i = 0; i < DATA_NUM_VERTS(preds->info.bare.data); i++) {
	    vert[i] = MET_set_new(MET_SET_TYPE_BLOCK);
	    vert[i]->info.block.block_list = MET_NULL_BLOCK;
	    pred_block = pred[i]->info.block.block_list;
	    new_block = MET__op_implicit_choose(block, pred_block);
	    
	    if (MET_NULL_BLOCK != new_block) {
	       MET_set_add_block(vert[i], new_block);
	       MET_BLOCK_FREE(new_block);
	    }
	 }
      }
   }

 bail:
   MET_SET_FREE(preds);
   return result;
}

METset *
MET_op_implicit(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[IMPLICIT_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[IMPLICIT_NUM_KIDS];
   METtype	*arg_type[IMPLICIT_NUM_KIDS], *t, *guess;
   MME(MET_op_implicit);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, IMPLICIT_NUM_KIDS);
#endif

   /* the type of our result is the same as the type of our first arg */
   arg_set[IMPLICIT_SET] =
      MET_sym_be_set_ref(sym->kids[IMPLICIT_SET], type,
			 &arg_type[IMPLICIT_SET],
			 &arg_set_type[IMPLICIT_SET]);

   /* type of second arg should be A -> Bool, where {A} is tp of first arg */

   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_SET_OF == type->type) {
      MET_TYPE_NEW_FIELD(t, MET_FIELD_ZTWO);
      MET_TYPE_NEW_MAP(guess, type->info.set_of.of, t);
      MET_TYPE_FREE(t);
   } else {
      MET_TYPE_NEW_UNKNOWN(guess);
   }
   arg_set[IMPLICIT_FN] =
      MET_sym_be_set_ref(sym->kids[IMPLICIT_FN], guess,
			 &arg_type[IMPLICIT_FN],
			 &arg_set_type[IMPLICIT_FN]);
   MET_TYPE_FREE(guess);


   if (MET_NULL_TYPE == arg_type[IMPLICIT_SET] ||
       MET_NULL_TYPE == arg_type[IMPLICIT_FN])
      goto bail1;

   if (arg_type[IMPLICIT_SET]->type != MET_TYPE_TYPE_SET_OF) {
      MET_op_error_bad_arg_type(1, arg_type[IMPLICIT_SET], "{?}", sym);
      goto bail1;
   }
   if ((t = arg_type[IMPLICIT_FN])->type != MET_TYPE_TYPE_MAP ||
       t->info.map.to->type != MET_TYPE_TYPE_FIELD ||
       t->info.map.to->info.field.type != MET_FIELD_ZTWO) {
      MET_op_error_bad_arg_type(2, arg_type[IMPLICIT_FN], "? -> B", sym);
      goto bail1;
   }

   if (MET_op_type_only(arg_set, IMPLICIT_NUM_KIDS,
			result_type, arg_type[IMPLICIT_SET],
			set_type, MET_SET_TYPE_BLOCK)) {
      goto bail2;
   }
   MET_op_reconcile_sets(arg_set, IMPLICIT_NUM_KIDS);

   if (MET_NULL_SET == arg_set[IMPLICIT_SET])
      goto bail1;

   if (MET_op_block_only(arg_set, IMPLICIT_NUM_KIDS, 
			 MET_op_implicit, sym->info.op.code,
			 arg_type[IMPLICIT_SET])) {
      MET_SET_COPY(result_set, arg_set[0]);
      goto bail1;
   }

   result_set = MET__op_implicit_guts(arg_set, arg_type, sym);

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(IMPLICIT_FN);
   COND_FREE(IMPLICIT_SET);

   return result_set;
}
