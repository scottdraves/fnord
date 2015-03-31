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

#include "ops/defs.h"
#include "sni/sni.h"

#ifndef lint
static char Version[] =
   "$Id: op_shared.c,v 1.33 1992/06/22 17:07:18 mas Exp $";
#endif

#ifdef OPS__CHECK_PARMS
void
MET_op_check_parms(sym, type, result_type, kids)
   METsym	*sym;
   METtype	*type, **result_type;
   int		kids;
{
   int i;
   MME(MET_op_check_parms);

   CHECK_SYM(sym);
   CHECK_NULL_PTR(result_type);
   CHECK_NULL_TYPE(type);

   if (sym->type != MET_SYM_TYPE_OP)
      FATAL((ERR_SI, "sym is not op", sym->type));

   if (kids != ANY_NUMBER_OF_KIDS &&
       sym->num_kids != kids) {
      SEVERE((ERR_ARB, "wrong number of kids, %d should be %d, op is",
	      sym->num_kids, kids));
      ERROR(sym->origin, OP_NAME(sym->info.op.code), SEV_FATAL);
      ERROR(sym->origin,
	    "substituting bogus ones. hack, chop...",
	    SEV_FATAL);
      for (i = sym->num_kids; i < kids; i++) {
	 sym->kids[i] = METsym_new_set_ref(MET_set_null, sym->origin);
      }
      sym->num_kids = kids;
   }
}
#endif

/*
 * call this after computing my_result_type (or leaving it NULL).
 * this checks the args, makes sure we have them.  if it returns true, then
 * the caller should bail out.
 */
Boolean
MET_op_type_only(args, num_args, result_type, known_type,
		 set_type, known_set_type)
   METset	**args;
   int		num_args;
   METtype	**result_type;
   METtype	*known_type;
   int		*set_type;
   int		known_set_type;
{
   int		i;
   MME(MET_op_type_only);

   for (i = 0; i < num_args; i++)
      if (MET_NULL_SET == args[i]) {

	 if ((METtype **) NULL != result_type) {
	    if (MET_NULL_TYPE != known_type) {
	       MET_TYPE_COPY(*result_type, known_type);
	    } else
	       MET_TYPE_NEW_UNKNOWN(*result_type);
	 }

	 if (INULL != set_type)
	    *set_type = known_set_type;

	 if (MET_NULL_TYPE != known_type)
	    MET_TYPE_FREE(known_type);

	 return TRUE;
      }
   
  return FALSE;
}

/*
 * This function takes care of the generating new symbolic definitions 
 * for a map using the passed op_code and argument sets, when there is
 * at most a single column of maps.
 */
static void
MET__op_guts_for_maps_1col(result_data, args, maps, num_args, op_code)
   METdata	*result_data;
   METset	**args;
   Boolean	*maps;
   int		num_args;
   int		op_code;
{
   METset	*nset;
   METsym	*ref;
   METtype	*t;
   METmap       *curr[MET__MAX_SYM_KIDS], *scan;
   int		i, j, i0;
   MME(MET__op_guts_map_1col);
   
   /* Set up information for big loop */
   for (i = 0, i0 = -1; i < num_args; i++)
      if (YES == maps[i]) {
	 if (i0 == -1) i0 = i;
	 curr[i] = DATA_FIRST_VERTEX(args[i]->info.bare.data, METmap);
      } else 
	 curr[i] = (METmap *) NULL;

   for (j = 0, scan = DATA_FIRST_VERTEX(result_data, METmap);
	j < DATA_NUM_VERTS(result_data);
	j++, scan++) {
      
      /* basic setup for new map element */
      scan->env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV);
      scan->sym = METsym_new_op(op_code, curr[i0]->sym->origin);
      scan->loose_from = MET_NULL_TYPE;

      for (i = 0; i < num_args; i++) 

	 /* if you have map data here, just add in the
	    sym of the map, and make sure the types given
	    in the data match */
	 if (YES == maps[i]) {
	    METsym_add_kid(scan->sym, curr[i]->sym);
	    if (MET_NULL_TYPE == scan->loose_from) {
	       MET_TYPE_COPY(scan->loose_from, curr[i]->loose_from);
	    } else {
	       (void) MET_type_cast(scan->loose_from, curr[i]->loose_from,
				    &t, TYPE_CAST_TAKE_FIRST);
	       MET_TYPE_FREE(scan->loose_from);
	       MET_TYPE_COPY(scan->loose_from, t);
	       MET_TYPE_FREE(t);
	    }
	    curr[i]++;
	 }

	 /* if you've got anything else, pull out the row, 
	    make a sym that refers to it, and plop that in
	    the new definition... */
	 else {
	    nset = MET_set_extract_row(args[i], j);
	    ref = METsym_new_set_ref(nset, scan->sym->origin);
	    MET_SET_FREE(nset);
	    METsym_add_kid(scan->sym, ref);
	    MET_SYM_FREE(ref);
	 }
   }
}

/*
 * Most general function for generating the data for new maps.  See
 * function that follows for more information.
 */
static void
MET__op_guts_for_maps(result_data, args, maps, num_args, op_code, nvrts)
   METdata	*result_data;
   METset	**args;
   Boolean	*maps;
   int 		num_args;
   int		op_code;
   int		nvrts;
{
   METtype	*type;
   METset	*set;
   METsym	*ref;
   METmap	*dst;
   METverts	*temp;
   METnumber	*cur[MET__MAX_SYM_KIDS];
   int		fields[MET__MAX_SYM_KIDS];
   int		src_step[MET__MAX_SYM_KIDS], dst_step;
   int		m0, i, j, k, length;
   MME(MET__op_guts_for_maps);

   length = result_data->num_cols;
   dst_step = result_data->verts->total_cols;

   /* initialization for our big loop:
      setup offsets for data in each arg, 
      remember type of each arg, 
      remember first location where map is found */

   for (i = 0, m0 = -1; i < num_args; i++) {
      temp = args[i]->info.bare.data->verts;
      src_step[i] = (1 == temp->num_verts) ?
	 0 : temp->total_cols;
      src_step[i] *= MET_field_size[temp->type];
      fields[i] = temp->type;
      if (m0 == -1 && MET_FIELD_MAP == fields[i])
	 m0 = i;
   }

   /* loop through data to set up new verts */
   
   for (j = 0; j < length; j++) {

      /* increment pointers to current vertex */
      for (i = 0; i < num_args; i++) {
	 cur[i] = args[i]->info.bare.data->verts->vertices;
	 cur[i] = CAST(METnumber *, CAST(char *, cur[i]) +
		       (j + args[i]->info.bare.data->first_col) *
		       MET_field_size[fields[i]]);
      }
      dst = DATA_FIRST_VERTEX(result_data, METmap) + j;
      for (k = 0; k < nvrts; k++, dst += dst_step) {

	 /* make the basic stuff for the new map */
	 dst->loose_from = MET_NULL_TYPE;
	 dst->env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV);
	 dst->sym = METsym_new_op(op_code, cur[m0]->map.sym->origin);
	 
	 /* add references to kids to the new sym */
	 for (i = 0; i < num_args; i++) {

	    /* for maps, just copy in the old symbol def & make sure
	       the bookkeeping works out right */
	    if (YES == maps[i]) {
	       if (MET_NULL_TYPE == dst->loose_from) {
		  MET_TYPE_COPY(dst->loose_from, 
				cur[i]->map.loose_from);
	       } else {
		  (void) MET_type_cast(dst->loose_from, 
				       cur[i]->map.loose_from,
				       &type, TYPE_CAST_TAKE_SECOND);
		  MET_TYPE_FREE(dst->loose_from);
		  MET_TYPE_COPY(dst->loose_from, type);
		  MET_TYPE_FREE(type);
	       }
	    } 

	    /* for numbers, make a new set & symbol that refers,
	       and plug that in */
	    else {
	       set = METset_new_number(fields[i], cur[i]);
	       MET_SET_FREE(set);
	       ref = METsym_new_set_ref(set, dst->sym->origin);
	       METsym_add_kid(dst->sym, ref);
	       MET_SYM_FREE(ref);
	    }
	    cur[i] = CAST(METnumber *, CAST(char *, cur[i]) + src_step[i]);
	 }		  
      }
   }
}

/*
 * for operations that hop across the lambda operator, this function
 * generates the symbolic description for the result, in as friendly a
 * way as possible.  All of the sets should be polybare or bare.
 */

METset *
MET_op_guts_for_maps(args, maps, num_args, op_code, expect_type)
   METset	**args;
   Boolean	*maps;
   int		num_args;
   int		op_code;
   METtype	*expect_type;
{
   METmap	*scan, *curr;
   METset	*result_set;
   METtype	*t;
   METsym	*ref;
   int		i, most, most_maps, one_map, most_map_cols;
   MME(MET_op_apply_to_maps);

   for (i = most = most_maps = most_map_cols = 0; i < num_args; i++) {
      if (YES == maps[i])
	 one_map = i,
	 most_map_cols =
	    MAX(most_map_cols, args[i]->info.bare.data->num_cols),
	 most_maps = 
	    MAX(most_maps, DATA_NUM_VERTS(args[i]->info.bare.data));
      most = MAX(most, DATA_NUM_VERTS(args[i]->info.bare.data));
   }
   
   /* this case is easy---just make a "false scalar" style map */

   if (most_maps == 1) {
      result_set = MET_type_to_polybare_set(expect_type, 1, args, num_args);
      scan = DATA_FIRST_VERTEX(result_set->info.bare.data, METmap);
      scan->env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV);
      scan->sym = 
	 METsym_new_op(op_code, 
		       DATA_FIRST_VERTEX(args[one_map]->info.bare.data, 
					 METmap)->sym->origin);
      for (i = 0; i < num_args; i++) {
	 if (YES == maps[i]) {
	    curr = DATA_FIRST_VERTEX(args[i]->info.bare.data, METmap);
	    METsym_add_kid(scan->sym, curr->sym);
	    if (scan->loose_from == MET_NULL_TYPE) {
	       MET_TYPE_COPY(scan->loose_from, curr->loose_from);
	    } else {
	       (void) MET_type_cast(scan->loose_from, curr->loose_from,
				    &t, TYPE_CAST_TAKE_FIRST);
	       MET_TYPE_FREE(scan->loose_from);
	       MET_TYPE_COPY(scan->loose_from, t);
	       MET_TYPE_FREE(t);
	    }
	 } else {
	    ref = METsym_new_set_ref(args[i], scan->sym->origin);
	    METsym_add_kid(scan->sym, ref);
	    MET_SYM_FREE(ref);
	 }
      }

      return result_set;
   }
	    
   /* this case is somewhat easy, because we can use METset_extract_row---
      it covers situations like  f + [ 1, 2, 3 ] (where f is a function
      that returns a vector).... */
   if (most_map_cols == 1) {
      result_set = MET_type_to_polybare_set(expect_type, most, args, num_args);
      MET__op_guts_for_maps_1col(result_set->info.bare.data, args, 
				 maps, num_args, op_code);
      
      return result_set;
   }

   /* this is the really hard case---it takes care of everything else */
   MET__op_guts_for_maps(result_set->info.bare.data, args, maps, 
			 num_args, op_code, most);
   
   return result_set;
}

/*
 * for functions which strictly deal with simple bare args.
 * returns YES/NO if it was evaluated.  result is returned in args[0].
 */
Boolean
MET_op_bare_only(args, num_args, op_func, op_code, type_guess)
   METset	**args;
   int		num_args;
   METset	*(*op_func)();
   int		op_code;
   METtype	*type_guess;
{
   int		i;
   Boolean	split[MET__MAX_SYM_KIDS];
   Boolean	i_deal = NO;
   METset	*result;
   MME(MET_op_bare_only);

   for (i = 0; i < num_args; i++) {
      if (MET_SET_TYPE_BARE == args[i]->type &&
	  DATA_NUM_VERTS(args[i]->info.bare.data) > 1) {
	 split[i] = YES;
	 i_deal = YES;
      } else
	 split[i] = NO;
   }
   if (YES == i_deal) {
      result = MET_op_polybare_to_parts(args, split, num_args,
					op_func, op_code, type_guess);
      MET_SET_FREE(args[0]);
      if (MET_NULL_SET != result) {
	 MET_SET_COPY(args[0], result);
	 MET_SET_FREE(result);
      } else {
	 args[0] = MET_NULL_SET;
      }
   }

   return i_deal;
}

/*
 * for functions which strictly deal with simple block args, and don't
 * know what to do when they get a polybare block of them.  Returns
 * returns YES/NO if it was evaluated.  result is returned in args[0].
 */
Boolean
MET_op_block_only(args, num_args, op_func, op_code, type_guess)
   METset	**args;
   int		num_args;
   METset	*(*op_func)();
   int		op_code;
   METtype	*type_guess;
{
   int		i;
   Boolean	split[MET__MAX_SYM_KIDS];
   Boolean	i_deal = NO;
   METset	*result;
   METdata 	*data;
   MME(MET_op_bare_only);

   for (i = 0; i < num_args; i++) {
      if (MET_SET_TYPE_BARE == args[i]->type) {
	 data = args[i]->info.bare.data;

	 /* This duplicates the condition for using
	    EXTRACT_SET_D in the function below---
	    only in this case would the type of set
	    passed into the recursive call of the op
	    be any different: */
	 if (MET_FIELD_SET == data->verts->type &&
	     1 == data->num_cols &&
	     MET_NULL_DATA == data->next) {

	    split[i] = YES;
	    i_deal = YES;
	 } else
	    split[i] = NO;
      } else
	 split[i] = NO;
   }
   if (YES == i_deal) {
      result = MET_op_polybare_to_parts(args, split, num_args,
					op_func, op_code, type_guess);
      MET_SET_FREE(args[0]);
      if (MET_NULL_SET != result) {
	 MET_SET_COPY(args[0], result);
	 MET_SET_FREE(result);
      } else {
	 args[0] = MET_NULL_SET;
      }
   }

   return i_deal;
}
   
/*
 * this is called by ops that want a bare arg and return a block set when
 * they get a polybare arg.  This the requested args into bare sets,
 * calls op_func for each one, and fits the results together into a
 * polybare set with type {something}.  can only split bare sets.
 * Modifies split for internal use, so be warned.
 */
#define PASS_IN		0
#define EXTRACT_ROW  	1	
#define EXTRACT_SET_D 	2

METset *
MET_op_polybare_to_parts(args, split, num_args, op_func, 
			 op_code, type_guess)
   METset	**args;
   Boolean	*split;
   int		num_args;
   METset	*(*op_func)();
   int		op_code;
   METtype	*type_guess;
{
   int		i, j, k, num_verts = 0, save_splitter;
   METsym	*sym;
   METset	*temp, *bare_arg, *one_vert, *result_set, **scan_verts, **arg_vert;
   METsym	*arg_ref;
   METverts	*verts;
   METdata	*data;
   METtype	*my_result_type = MET_NULL_TYPE;
   METtype	*one_result_type;
   int		set_type, one_set_type, num_subverts, subfield, ncols;
   METblock	*block;
   MME(MET_op_polybare_to_parts);

   for (i = 0; i < num_args; i++) {
      if (YES == split[i] &&
	  (j = DATA_NUM_VERTS(args[i]->info.bare.data)) > num_verts) {
	 num_verts = j;
	 save_splitter = i;
      }
      if (YES == split[i])
	 split[i] =
	    (MET_FIELD_SET == args[i]->info.bare.data->verts->type &&
	     1 == args[i]->info.bare.data->num_cols &&
	     MET_NULL_DATA == args[i]->info.bare.data->next) ? 
		EXTRACT_SET_D : EXTRACT_ROW;
   }

   /* must reconcile splitting parms */
   result_set = MET_set_new(MET_SET_TYPE_BARE);
   result_set->info.bare.how_many = num_verts;
   MET_tags_initialize(&result_set->info.bare.tags_bare);
   MET_tags_initialize(&result_set->info.bare.tags_in);
   MET_tags_copy(&result_set->info.bare.tags_bare, 
		 &args[save_splitter]->info.bare.tags_bare);
   MET_tags_copy(&result_set->info.bare.tags_in, 
		 &args[save_splitter]->info.bare.tags_in);

   verts = MET_verts_new(MET_FIELD_SET, num_verts, 1);
   data = MET_data_new(verts, 1, 0);
   scan_verts = (METset **) verts->vertices;
   MET_VERTS_FREE(verts);
   MET_DATA_COPY(result_set->info.bare.data, data);
   MET_DATA_FREE(data);

   /*
    * we need to give it a type now, but we don't know what it is for sure.
    * so guess {?}.  If, later on, it turns out to be something more
    * specific, then we replace this.
    */
   MET_TYPE_NEW_UNKNOWN(one_result_type);
   MET_TYPE_NEW_SET_OF(result_set->info.bare.type, one_result_type);
   MET_TYPE_FREE(one_result_type);

   for (i = 0; i < num_verts; i++) {
      /*
       * build sym
       */
      sym = METsym_new_op(op_code, RNULL);
      for (j = 0; j < num_args; j++) {
	 switch (split[j]) {
	  case PASS_IN:
	    MET_SET_COPY(bare_arg, args[j]);
	    break;
	  case EXTRACT_ROW:
	    bare_arg = MET_set_extract_row(args[j], i);
	    break;
	  case EXTRACT_SET_D:
	    bare_arg = MET_set_extract_set_row(args[j], i);
	    break;
	  default:
	    DEFAULT_ERR(split);
	 }
	 arg_ref = METsym_new_set_ref(bare_arg, RNULL);
	 MET_SET_FREE(bare_arg);
	 METsym_add_kid(sym, arg_ref);
	 MET_SYM_FREE(arg_ref);
      }
      /*
       * apply the passed op
       */
      one_vert = (op_func)(sym, type_guess, 
			   &one_result_type, &one_set_type);
      MET_SYM_FREE(sym);

      if (MET_NULL_SET == one_vert)
	 goto bail;
      
      /*
       * first time through remember the type of the result.  after that,
       * see if they all match that first type
       */
      if (0 == i) {
	 MET_TYPE_COPY(my_result_type, one_result_type);
	 if (MET_SET_TYPE_BARE == (set_type = one_set_type)) {
	    num_subverts = DATA_NUM_VERTS(one_vert->info.bare.data);
	    ncols = one_vert->info.bare.data->num_cols;
	    subfield = one_vert->info.bare.data->verts->type;
	 }
      } else {
	 if (MET_SET_TYPE_BARE == one_set_type &&
	     (set_type != one_set_type || 
	      subfield != one_vert->info.bare.data->verts->type ||
	      ncols != one_vert->info.bare.data->num_cols))
	    goto bail;
	 if (NO == MET_type_compatible(one_result_type,
				       my_result_type, NO)) {
	    if (MET_SET_TYPE_BARE == set_type)
	       goto bail;
	    else {
	       MET_TYPE_FREE(my_result_type);
	       my_result_type == MET_NULL_TYPE;
	    }
	 }
	 if (MET_SET_TYPE_BARE == set_type && 
	     num_subverts < DATA_NUM_VERTS(one_vert->info.bare.data))
	    num_subverts = DATA_NUM_VERTS(one_vert->info.bare.data);
      }

      /* if we have a block set, we may have stupid partial tags to 
	 get rid of */
      if (MET_SET_TYPE_BLOCK == one_vert->type){
	 temp = MET_set_new(MET_SET_TYPE_BLOCK);
	 temp->info.block.block_list =
	    MET_block_lose_tags(one_vert->info.block.block_list,
				&result_set->info.bare.tags_bare);
	 MET_SET_FREE(one_vert);
	 MET_SET_COPY(one_vert, temp);
	 MET_SET_FREE(temp);
      }
	    
      MET_TYPE_FREE(one_result_type);
      
      /*
       * add it to our result
       */
      MET_SET_COPY(*scan_verts, one_vert);
      scan_verts++;

      MET_SET_FREE(one_vert);
   }

   if (my_result_type != MET_NULL_TYPE) {
      /*
       * they are all the same type, so we can use that rather than what we
       * guessed when we created the set.
       */
      MET_TYPE_FREE(result_set->info.bare.type);
      MET_TYPE_COPY(result_set->info.bare.type, my_result_type);
      /*
       * collapse the representation if everything has not gone
       * right.
       */
      if (MET_SET_TYPE_BARE == set_type) {
	 MET_SET_COPY(bare_arg, result_set);
	 MET_SET_FREE(result_set);

	 result_set = MET_set_new(MET_SET_TYPE_BARE);
	 result_set->info.bare.how_many = num_verts * num_subverts;
	 MET_tags_initialize(&result_set->info.bare.tags_bare);
	 MET_tags_initialize(&result_set->info.bare.tags_in);
	 MET_tags_copy(&result_set->info.bare.tags_bare, 
		       &bare_arg->info.bare.tags_bare);
	 MET_tags_copy(&result_set->info.bare.tags_in, 
		       &bare_arg->info.bare.tags_in);
	 MET_TYPE_COPY(result_set->info.bare.type, my_result_type);

	 for (j = 0; 1; j++) {
	    verts = MET_verts_new(subfield, num_verts * num_subverts, 
				  ncols);
	    data = MET_data_new(verts, ncols, 0);
	    
	    if (j == 0) {
	       MET_DATA_COPY(result_set->info.bare.data, data);
	    } else
	       MET_data_append(result_set->info.bare.data, data);
	    
	    MET_DATA_FREE(data);
	    
	    scan_verts = 
	       DATA_FIRST_VERTEX(bare_arg->info.bare.data, METset *);
	    MET_tags_copy(&result_set->info.bare.tags_bare, 
			  &(*scan_verts)->info.bare.tags_bare);
	    MET_tags_copy(&result_set->info.bare.tags_in, 
			  &(*scan_verts)->info.bare.tags_in);

	    for (i = 0; i < num_verts; i++, scan_verts++) {
	       data = (*scan_verts)->info.bare.data;
	       for (k = 0; k < j; k++)
		  data = data->next;
	       
	       MET_verts_copy_vertices(verts, i * num_subverts, 0, 
				       data->verts, 0, data->first_col,
				       data->num_cols,
				       data->verts->num_verts, 1,
				       (data->verts->num_verts == 1));
	    }
	    
	    MET_VERTS_FREE(verts);
	    if (data->next == MET_NULL_DATA)
	       break;
	 }

	 MET_SET_FREE(bare_arg);
      }

      MET_TYPE_FREE(my_result_type);
   }
   
   return result_set;

 bail:
   for (; j < num_verts; j++)
      *scan_verts++ = MET_NULL_SET;
   
   MET_TYPE_FREE_NULL(one_result_type);
   MET_TYPE_FREE_NULL(my_result_type);
   MET_SET_FREE(result_set);
   
   return MET_NULL_SET;
}


static Boolean
MET__op_type_eval_arg(sym, arg_type, n, dim)
   METsym	*sym;
   METtype	**arg_type;
   int		n;
   int		*dim;
{
   METtype	*my_guess, *its_type;
   int		field;
   METnumber	number;
   METset	*arg;
   MME(MET__op_type_eval_arg);

   if (INULL == dim) {
      MET_TYPE_NEW_SET(my_guess);
   } else {
      MET_TYPE_NEW_FIELD(my_guess, MET_FIELD_INTEGER);
   }
   arg = MET_sym_be_set_ref(sym->kids[n], my_guess,
			    &its_type, INULL);
   MET_TYPE_FREE(my_guess);

   /*
    * make sure we got something
    */
   if (MET_NULL_SET == arg) {
      /*
       * stick something in to be freed
       */
      MET_TYPE_NEW_UNKNOWN(arg_type[n]);
      goto bail;
   }

   if (INULL == dim) {
      /*
       * make sure it is a type set, if not then steal
       * its type.
       */
      if (MET_SET_TYPE_TYPE != arg->type) {
	 if (MET_TYPE_TYPE_SET_OF != its_type->type) {
	    ERROR(sym->origin,
		  "non-type objects in type exprs must be sets",
		  SEV_FATAL);
	    MET_TYPE_NEW_UNKNOWN(arg_type[n]);
	    goto bail;
	 }
	 MET_TYPE_COPY(arg_type[n],
		       its_type->info.set_of.of);
      } else {
	 MET_TYPE_COPY(arg_type[n],
		       arg->info.type.type);
      }
   } else {
      /*
       * make sure it's an integer or a MET_TYPE_TYPE_UNKNOWN.
       */
      MET_TYPE_NEW_UNKNOWN(arg_type[n]);
      if (MET_SET_TYPE_TYPE == arg->type &&
	  MET_TYPE_TYPE_UNKNOWN == arg->info.type.type->type)
	 *dim = TYPE_DIMENSION_UNKNOWN;
      else {
	 field = MET_FIELD_INTEGER;
	 if (FAILURE == METset_get_number(arg, &field,
					  &number, YES)) {
	    ERROR(sym->origin, "exponent in type expr was not integer",
		  SEV_FATAL);
	    goto bail;
	 }
	 *dim = number.integer;
      }
   }

   MET_SET_FREE(arg);
   MET_TYPE_FREE(its_type);
   return SUCCESS;

 bail:
   MET_TYPE_FREE(its_type);
   return FAILURE;
}

#if 0
/*
 * this is called by ops that want block args, and return another
 * block set.  If they get called with a polybare argument 
 * consisting of a set of block sets, this function calls op_func
 * on each one, and fits the results together into a polybare
 * set with type {{something}}.
 */

METset *
MET_op_polybare_to_blocks(args, split, num_args, op_func, type_guess)
   METset 	**args;
   Boolean	*split;
   int 		num_args;
   METset	*(*op_func)();
   METtype	*type_guess;
{
   int		i, num_verts = 0, save_splitter;
   METsym	*sym;
   METtype	*my_result_type = MET_NULL_TYPE, *one_result_type;
   METverts	*verts;
   METset	*block_arg, *result_set, *scan_verts;
   MME(MET_op_bare_to_blocks);

   for (i = 0; i < num_args; i++) {
      if (YES == split[i]) {
	 if ((j = args[i]->info.bare.how_many) > num_verts) {
	    num_verts = j;
	    save_splitter = i;
	 }
	 if (MET_FIELD_SET != args[i]->info.bare.data->verts->type)
	    return MET_NULL_SET;
      }
   }

   result_set = MET_set_new(MET_SET_TYPE_BARE);
   result_set->info.bare.how_many = num_verts;
   MET_tags_initialize(result_set->info.bare.tags_bare);
   MET_tags_initialize(result_set->info.bare.tags_in);
   MET_tags_copy(&result_set->info.bare.tags_bare,
		 &args[save_splitter]->info.bare.tags_bare);
   MET_tags_copy(&result_set->info.bare.tags_in,
		 &args[save_splitter]->info.bare.tags_in);
   verts = MET_verts_new(MET_FIELD_SET, num_verts, 1);
   data = MET_data_new(verts, 1, 0);
   scan_verts = (METset **) verts->vertices;
   MET_VERTS_FREE(verts);
   MET_DATA_COPY(result_set->info.bare.data, data);
   MET_DATA_FREE(data);

   /*
    * we need to give it a type now, but we don't know what it is for sure.
    * so guess {?}.  If, later on, it turns out to be something more
    * specific, then we replace this.
    */
   MET_TYPE_NEW_UNKNOWN(one_result_type);
   MET_TYPE_NEW_SET_OF(result_set->info.bare.type, one_result_type);
   MET_TYPE_FREE(one_result_type);

   for (i = 0; i < num_verts; i++) {
      sym = METsym_new_op(MET_SYM_OP_NONE, RNULL);
      if (YES == split[i] &&
	  args[j]->info.bare.how_many > 1){
	 block_arg = 

}
#endif

METset *
MET_op_type_expr(sym, type_guess, result_type, set_type)
   METsym	*sym;
   METtype	*type_guess, **result_type;
   int		*set_type;
{
   METset	*result_set = MET_NULL_SET;
   METtype	*arg_type[2], *result;
   METnumber	num;
   int		dim, num_args, i;
   Boolean	failed = SUCCESS;
   MME(MET_op_type_expr);


   /*
    * evaluate args
    */
   switch (sym->info.op.code) {
    case MET_SYM_OP_PAIR_BUILD:
    case MET_SYM_OP_MAP:
    case MET_SYM_OP_CROSS:
      failed &= MET__op_type_eval_arg(sym, arg_type, 0, INULL);
      failed &= MET__op_type_eval_arg(sym, arg_type, 1, INULL);
      num_args = 2;
      break;

    case MET_SYM_OP_PAIR_LEFT:
    case MET_SYM_OP_PAIR_RIGHT:
    case MET_SYM_OP_SET_OF:
    case MET_SYM_OP_ROW_BUILD:
    case MET_SYM_OP_COLUMN_BUILD:
      failed &= MET__op_type_eval_arg(sym, arg_type, 0, INULL);
      num_args = 1;
      break;

    case MET_SYM_OP_ROW_EXTRACT:
    case MET_SYM_OP_COLUMN_EXTRACT:
    case MET_SYM_OP_POW:
      failed &= MET__op_type_eval_arg(sym, arg_type, 0, INULL);
      failed &= MET__op_type_eval_arg(sym, arg_type, 1, &dim);
      num_args = 2;
      break;
   }

   if (FAILURE == failed)
      goto bail;

   /*
    * apply the op
    */
   switch (sym->info.op.code) {
    case MET_SYM_OP_PAIR_BUILD:
    case MET_SYM_OP_CROSS:
      MET_TYPE_NEW_PAIR(result, arg_type[0], arg_type[1]);
      break;
    case MET_SYM_OP_PAIR_LEFT:
      switch (arg_type[0]->type) {
       case MET_TYPE_TYPE_MAP:
	 MET_TYPE_COPY(result, arg_type[0]->info.map.from);
	 break;
       case MET_TYPE_TYPE_PAIR:
	 MET_TYPE_COPY(result, arg_type[0]->info.pair.left);
	 break;
       case MET_TYPE_TYPE_VECTOR:
	 MET_TYPE_COPY(result, arg_type[0]->info.vector.of);
	 break;
       case MET_TYPE_TYPE_SET:
	 ERROR(sym->origin, "type SET has no left", SEV_FATAL);
	 goto bail;
       case MET_TYPE_TYPE_FIELD:
	 ERROR(sym->origin, "type FIELD has no left", SEV_FATAL);
	 goto bail;
       case MET_TYPE_TYPE_SET_OF:
	 MET_TYPE_COPY(result, arg_type[0]->info.set_of.of);
	 break;
       case MET_TYPE_TYPE_UNKNOWN:
	 ERROR(sym->origin, "type ? has no left", SEV_FATAL);
	 goto bail;
       case MET_TYPE_TYPE_BOGUS:
	 ERROR(sym->origin, "type $ has no left", SEV_FATAL);
	 goto bail;
       default:
	 DEFAULT_ERR(arg_type[0]->type);
      }
      break;
    case MET_SYM_OP_PAIR_RIGHT:
      switch (arg_type[0]->type) {
       case MET_TYPE_TYPE_MAP:
	 MET_TYPE_COPY(result, arg_type[0]->info.map.to);
	 break;
       case MET_TYPE_TYPE_PAIR:
	 MET_TYPE_COPY(result, arg_type[0]->info.pair.right);
	 break;
       case MET_TYPE_TYPE_VECTOR:
	 num.integer = arg_type[0]->info.vector.dimension;
	 result_set = METset_new_number(MET_FIELD_INTEGER, &num);
	 goto bail;
       case MET_TYPE_TYPE_SET:
	 ERROR(sym->origin, "type SET has no right", SEV_FATAL);
	 goto bail;
       case MET_TYPE_TYPE_FIELD:
	 ERROR(sym->origin, "type FIELD has no right", SEV_FATAL);
	 goto bail;
       case MET_TYPE_TYPE_SET_OF:
	 ERROR(sym->origin, "type {} has no right", SEV_FATAL);
	 goto bail;
       case MET_TYPE_TYPE_UNKNOWN:
	 ERROR(sym->origin, "type ? has no right", SEV_FATAL);
	 goto bail;
       case MET_TYPE_TYPE_BOGUS:
	 ERROR(sym->origin, "type $ has no left", SEV_FATAL);
	 goto bail;

       default:
	 DEFAULT_ERR(arg_type[0]->type);
      }
      break;
    case MET_SYM_OP_ROW_BUILD:
    case MET_SYM_OP_COLUMN_BUILD:
      NYET;
      MET_TYPE_COPY(result, arg_type[0]);
      break;
    case MET_SYM_OP_ROW_EXTRACT:
      MET_TYPE_NEW_VECTOR(result, arg_type[0], dim, YES);
      break;
    case MET_SYM_OP_POW:
    case MET_SYM_OP_COLUMN_EXTRACT:
      MET_TYPE_NEW_VECTOR(result, arg_type[0], dim, NO);
      break;
    case MET_SYM_OP_SET_OF:
      MET_TYPE_NEW_SET_OF(result, arg_type[0]);
      break;
    case MET_SYM_OP_MAP:
      MET_TYPE_NEW_MAP(result, arg_type[0], arg_type[1]);
      break;
    default:
      DEFAULT_ERR(sym->info.op.code);
   }
   result_set = MET_set_new(MET_SET_TYPE_TYPE);
   MET_TYPE_COPY(result_set->info.type.type, result);
   MET_TYPE_FREE(result);
   
 bail:
   MET_set_get_type(result_set, result_type, set_type);
   for (i = 0; i < num_args; i++)
      MET_TYPE_FREE(arg_type[i]);

   return result_set;
}


static void
MET__op_shared_extend_by_tag(change_me, tag)
   METset	**change_me;
   METtag	*tag;
{
   int		i, j, k, sizes[2];
   int		num_real, found;
   METverts	*verts;
   METdata	*data, *old_data;
   METset	*set;
   METtag_set	*tag_set;
   MME(MET__op_shared_extend_by_tag);

   /* 
    * if the tag is not an extender type, don't worry
    */
   if (YES == tag->scalar)
      return;

   /*
    * if we already have extendend by the tag,
    * we don't have to do anything.
    */
   tag_set = &(*change_me)->info.bare.tags_bare;
   num_real = DATA_NUM_VERTS((*change_me)->info.bare.data);

   for (found = NO, i = 0; i < tag_set->num_tags; i++)
      if (tag->id == tag_set->tags[i].id) 
	 if (MET_TAG_TYPE_ELEMENT == tag->type) {
	    set = MET_set_extract_tagged_row(*change_me, tag->id, tag->index);
	    MET_SET_FREE(*change_me);
	    MET_SET_COPY(*change_me, set);
	    MET_SET_FREE(set);
	    return;
	 } else if (NO == tag->scalar && 
		    YES == tag_set->tags[i].scalar &&
		    1 != num_real) {
	    found = YES;
	    break;
	 } else {
	    return;
	 }	
   
   /*
    * if we have just one element (scalars expand to fit),
    * just add the tag, without expanding the data....
    */
   if (1 == num_real) {
      MET_SET_COPY_DEEP(set, *change_me);
      MET_tags_add(&set->info.bare.tags_bare, tag, TRUE /* scalar tag */);
      MET_SET_FREE(*change_me);
      MET_SET_COPY(*change_me, set);
      MET_SET_FREE(set);
      return;
   }

   /*
    * make sure that the tag that we've added is stored
    * on the set (avoiding duplication of tags).
    */
   MET_SET_COPY_DEEP(set, *change_me);
   if (YES == found)
      set->info.bare.tags_bare.tags[i].scalar = FALSE;
   else {
      MET_tags_add(&set->info.bare.tags_bare, 
		   tag, FALSE /* we've altered the data */);
      set->info.bare.how_many *= tag->size;
   }

   /*
    * ok, now replicate our vertices.
    */
   if (tag->size > 1) {
      for (sizes[0] = 1, i = 0; 
	   set->info.bare.tags_bare.tags[i].id < tag->id && 
	   i < set->info.bare.tags_bare.num_tags;
	   i++)
	 if (NO == set->info.bare.tags_bare.tags[i].scalar)
	    sizes[0] *= set->info.bare.tags_bare.tags[i].size;

      old_data = set->info.bare.data;
      sizes[1] = DATA_NUM_VERTS(old_data) / sizes[0];
      verts = MET_verts_new(old_data->verts->type,
			    tag->size * DATA_NUM_VERTS(old_data),
			    old_data->num_cols);
      data = MET_data_new(verts, old_data->num_cols, 0);
      MET_VERTS_FREE(verts);
      k = 0;
      for (i = 0; i < sizes[0]; i++)
	 for (j = 0; j < tag->size; j++) {
	    MET_verts_copy_vertices(verts,
				    k * sizes[1], 0,
				    old_data->verts,
				    i * sizes[1], old_data->first_col,
				    old_data->num_cols, sizes[1], 1, NO);
	    k++;
	 }
      MET_DATA_FREE(set->info.bare.data);
      MET_DATA_COPY(set->info.bare.data, data);
      MET_DATA_FREE(data);
   }

   MET_SET_FREE(*change_me);
   MET_SET_COPY(*change_me, set);
   MET_SET_FREE(set);
}

/*
 * take two bare sets.  change CHANGE_ME so that it has all the tags
 * present in extender.  This involves replicating the vertices if
 * there are any new tags in EXTENDER.  CHANGE_ME is replaced if need be.
 * extender is unaffected.
 */
static void
MET__op_shared_extend_set(change_me, extender)
   METset	**change_me, *extender;
{
   int		i;
   METset	*set;
   METtag	*tags;
   int 		num_tags;
   MME(MET__op_shared_extend_set);

   switch (extender->type) {
    case MET_SET_TYPE_BLOCK:
      if (MET_NULL_BLOCK == extender->info.block.block_list)
	 return;
      tags = extender->info.block.block_list->tags_part.tags;
      num_tags = extender->info.block.block_list->tags_part.num_tags;
      break;

    case MET_SET_TYPE_BARE:
      tags = extender->info.bare.tags_bare.tags;
      num_tags = extender->info.bare.tags_bare.num_tags;
      break;

    default:
      return;
   }

   for (i = 0; i < num_tags; i++)
      MET__op_shared_extend_by_tag(change_me, tags + i);
}

/*
 * make a group of sets all have the same tags in the same order.
 * extend all possible pairs in each direction.
 */
void
MET_op_reconcile_sets(sets, num_sets)
   METset	**sets;
   int		num_sets;
{
   int		i, j;
   MME(MET_op_reconcile_sets);

   for (i = 0; i < num_sets; i++)
      for (j = 0; j < num_sets; j++)
	 if (i != j) {
	    if (MET_SET_TYPE_BARE == sets[i]->type)
	       MET__op_shared_extend_set(sets + i, sets[j]);
	    if (MET_SET_TYPE_BARE == sets[j]->type)
	       MET__op_shared_extend_set(sets + j, sets[i]);
	 }
}
   
void
MET_op_error_bad_arg_type(narg, type, expected, sym)
   int		narg;
   METtype	*type;
   char		*expected;
   METsym	*sym;
{
   char		buf[8000];
   MME(MET_op_error_bad_arg_type);

   sprintf(buf, "%s, arg %d had bad type.  wanted %s, got ",
	   OP_NAME(sym->info.op.code), narg, expected);
   METtype_print_pretty(FNULL, type, buf, 0);
   (void) strcat(buf, "\n");
   ERROR(sym->origin, buf, SEV_FATAL);
}

void
MET_op_cant_cast(sym, a, b, info)
   METsym	*sym;
   METtype	*a, *b;
   char		*info;
{
   char		buf[8000];
   MME(MET_op_cant_cast);

   sprintf(buf, "%s, couldn't cast ", OP_NAME(sym->info.op.code));
   METtype_print_pretty(FNULL, a, buf, 0);
   (void) strcat(buf, " to ");
   METtype_print_pretty(FNULL, b, buf, 0);
   if (CNULL != info)
      (void) strcat(buf, info);
   (void) strcat(buf, "\n");
   ERROR(sym->origin, buf, SEV_FATAL);
}

Boolean
MET_op_guts_failure()
{
   MME(MET_op_guts_failure);

   ERROR(RNULL, "op guts failure, probably bad fields", SEV_FATAL);

   return FAILURE;
}

/*ARGSUSED*/
METset *
MET_op_unimplemented(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   char		buf[200];
   MME(MET_op_unimplemented);
   CHECK_SYM(sym);

   /* why is this here??? */

   if ((METtype **) NULL != result_type)
      MET_TYPE_NEW_UNKNOWN(*result_type);

   (void) sprintf(buf, "op %s is unimplemented",
		  OP_NAME(sym->info.op.code));
   ERROR(sym->origin, buf, SEV_FATAL);

   return MET_NULL_SET;
}

/*ARGSUSED*/
METset *
MET_op_bad_args(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   MME(MET_op_bad_args);
   CHECK_SYM(sym);

   /* why is this here??? */
   if ((METtype **) NULL != result_type)
      MET_TYPE_NEW_UNKNOWN(*result_type);

   ERROR(sym->origin, "op had the wrong number of args", SEV_FATAL);

   return MET_NULL_SET;
}

MET_op_info MET_op_info_table[] = {
/*  0   */ {"nop",             MET_op_print        , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  1   */ {"print",           MET_op_print        , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  2   */ {"pair_build",      MET_op_pair_build   , PRIME_RULE_LINEAR  ,OP_TWO_ARG },	
/*  3   */ {"pair_left",       MET_op_unpair       , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  4   */ {"pair_right",      MET_op_unpair       , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  5   */ {"row_build",       MET_op_vector_build , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  6   */ {"column_build",    MET_op_vector_build , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  7   */ {"row_extract",     MET_op_extract      , PRIME_RULE_SPECIAL ,OP_TWO_ARG },	
/*  8   */ {"column_extract",  MET_op_extract      , PRIME_RULE_SPECIAL ,OP_TWO_ARG },	
/*  9   */ {"concatenate",     MET_op_cat          , PRIME_RULE_LINEAR  ,OP_TWO_ARG },
/*  10  */ {"vector_to_pairs", MET_op_frag         , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  11  */ {"pair_build_3",    MET_op_unimplemented, PRIME_RULE_NONE    ,OP_THREE_ARG },
/*  12  */ {"pair_build_4",    MET_op_unimplemented, PRIME_RULE_NONE    ,OP_FOUR_ARG },
/*  13  */ {"flatten",         MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  14  */ {"set_to_pairs",    MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  15  */ {"bad_args",        MET_op_bad_args     , PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  16  */ {"trace_on",        MET_op_trace        , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  17  */ {"trace_off",       MET_op_trace        , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  18  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  19  */ {"opposite",        MET_op_opp          , PRIME_RULE_LINEAR  ,OP_ONE_ARG },	
/*  20  */ {"add",             MET_op_add          , PRIME_RULE_LINEAR  ,OP_TWO_ARG },	
/*  21  */ {"subtract",        MET_op_add          , PRIME_RULE_LINEAR  ,OP_TWO_ARG },	
/*  22  */ {"multiply",        MET_op_mult         , PRIME_RULE_PRODUCT ,OP_TWO_ARG },	
/*  23  */ {"divide",          MET_op_div          , PRIME_RULE_SPECIAL ,OP_TWO_ARG },	
/*  24  */ {"abs",             MET_op_unimplemented, PRIME_RULE_SPECIAL ,OP_ONE_ARG },	
/*  25  */ {"sin",             MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  26  */ {"cos",             MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  27  */ {"tan",             MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  28  */ {"asin",            MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  29  */ {"acos",            MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  30  */ {"atan",            MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  31  */ {"atan2",           MET_op_unimplemented, PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  32  */ {"sinh",            MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  33  */ {"cosh",            MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  34  */ {"tanh",            MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  35  */ {"asinh",           MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  36  */ {"acosh",           MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  37  */ {"atanh",           MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  38  */ {"ln",              MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  39  */ {"log",             MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  40  */ {"lg",              MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  41  */ {"pow",             MET_op_pow          , PRIME_RULE_SPECIAL ,OP_TWO_ARG },	
/*  42  */ {"square",          MET_op_opp          , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  43  */ {"sqrt",            MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  44  */ {"exp",             MET_op_scalar       , PRIME_RULE_CHAIN   ,OP_ONE_ARG },	
/*  45  */ {"erf",             MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  46  */ {"gamma",           MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  47  */ {"add_3",           MET_op_unimplemented, PRIME_RULE_NONE    ,OP_THREE_ARG },
/*  48  */ {"add_4",           MET_op_unimplemented, PRIME_RULE_NONE    ,OP_FOUR_ARG },
/*  49  */ {"multiply_3",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_THREE_ARG },
/*  50  */ {"multiply_4",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_FOUR_ARG },
/*  51  */ {"sin_cos",         MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  52  */ {"exp_exp",         MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  53  */ {"polynomial",      MET_op_poly         , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  54  */ {"min",             MET_op_add          , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  55  */ {"max",             MET_op_add          , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  56  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  57  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  58  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  59  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  60  */ {"conjugate",       MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  61  */ {"re",              MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  62  */ {"im",              MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  63  */ {"complex_build",   MET_op_complex_build, PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  64  */ {"complex_frag",    MET_op_complex_frag , PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  65  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  66  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  67  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  68  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  69  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  70  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  71  */ {"vector_cross",    MET_op_unimplemented, PRIME_RULE_PRODUCT ,OP_TWO_ARG },	
/*  72  */ {"dot",             MET_op_dot          , PRIME_RULE_PRODUCT ,OP_TWO_ARG },	
/*  73  */ {"norm",            MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  74  */ {"normalize",       MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  75  */ {"sum_of_entries",  MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  76  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  77  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  78  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  79  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  80  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  81  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  82  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  83  */ {"trace",           MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  84  */ {"det",             MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  85  */ {"transpose",       MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  86  */ {"inverse",         MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  87  */ {"adjoint",         MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  88  */ {"evalues",         MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  89  */ {"evectors",        MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  90  */ {"rotate",          MET_op_unimplemented, PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  91  */ {"orient",          MET_op_unimplemented, PRIME_RULE_NONE    ,OP_THREE_ARG },
/*  92  */ {"permute",         MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  93  */ {"shift",           MET_op_shift        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  94  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  95  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  96  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  97  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  98  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  99  */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  100 */ {"ceiling",         MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  101 */ {"floor",           MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  102 */ {"sgn",             MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  103 */ {"modulo",          MET_op_biint        , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  104 */ {"choose",          MET_op_biint        , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  105 */ {"factorial",       MET_op_unint        , PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  106 */ {"enumerate",       MET_op_enum         , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  107 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  108 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  109 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  110 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  111 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  112 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  113 */ {"ztwo_to_integer", MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  114 */ {"integer_to_real", MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  115 */ {"angle_to_real",   MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  116 */ {"real_to_complex", MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  117 */ {"real_to_angle",   MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  118 */ {"real_to_integer", MET_op_unint        , PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  119 */ {"integer_to_ztwo", MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  120 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  121 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  122 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  123 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  124 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  125 */ {"reduce",          MET_op_reduce       , PRIME_RULE_NONE    ,OP_THREE_ARG },	
/*  126 */ {"set_cross",       MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  127 */ {"in",              MET_op_unimplemented, PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  128 */ {"union",           MET_op_union        , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  129 */ {"intersection",    MET_op_unimplemented, PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  130 */ {"exclusion",       MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  131 */ {"cardinality",     MET_op_card         , PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  132 */ {"set_of",          MET_op_setof        , PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  133 */ {"interval",        MET_op_interval     , PRIME_RULE_NONE    ,OP_TWO_ARG|OP_THREE_ARG},
/*  134 */ {"cylinder",        MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  135 */ {"sphere",          MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  136 */ {"triangle",        MET_op_triangle,      PRIME_RULE_NONE    ,OP_ONE_ARG|OP_TWO_ARG },
/*  137 */ {"torus",           MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  138 */ {"measure",         MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  139 */ {"set_ztwo",        MET_op_unimplemented, PRIME_RULE_NONE    ,OP_NO_ARG },	
/*  140 */ {"set_integers",    MET_op_unimplemented, PRIME_RULE_NONE    ,OP_NO_ARG },	
/*  141 */ {"set_angles",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_NO_ARG },	
/*  142 */ {"set_reals",       MET_op_unimplemented, PRIME_RULE_NONE    ,OP_NO_ARG },	
/*  143 */ {"set_complexes",   MET_op_unimplemented, PRIME_RULE_NONE    ,OP_NO_ARG },	
/*  144 */ {"sample",          MET_op_sample       , PRIME_RULE_NONE    ,OP_ONE_ARG },
/*  145 */ {"not",             MET_op_unlogic      , PRIME_RULE_NONE    ,OP_ONE_ARG },
/*  146 */ {"and",             MET_op_bilogic      , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  147 */ {"or",              MET_op_bilogic      , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  148 */ {"bitnot",          MET_op_unint        , PRIME_RULE_NONE    ,OP_ONE_ARG },
/*  149 */ {"bitand",          MET_op_biint        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  150 */ {"bitor",           MET_op_biint        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  151 */ {"bitxor",          MET_op_biint        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  152 */ {"epsilon_equal",   MET_op_relop        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  153 */ {"exact_equal",     MET_op_relop        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  154 */ {"notequal",        MET_op_relop        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  155 */ {"greater",         MET_op_relop        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  156 */ {"less",            MET_op_relop        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  157 */ {"greaterorequal",  MET_op_relop        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  158 */ {"lessorequal",     MET_op_relop        , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  159 */ {"set_null",        MET_op_set_null     , PRIME_RULE_NONE    ,OP_NO_ARG },
/*  160 */ {"zero",            MET_op_set_null     , PRIME_RULE_NONE    ,OP_NO_ARG },	
/*  161 */ {"identity",        MET_op_set_null     , PRIME_RULE_NONE    ,OP_NO_ARG },	
/*  162 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  163 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  164 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  165 */ {"type",            MET_op_type         , PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  166 */ {"implicit",        MET_op_implicit     , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  167 */ {"map",             MET_op_map          , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  168 */ {"prime",           MET_op_prime        , PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  169 */ {"derivative",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  170 */ {"if",              MET_op_if           , PRIME_RULE_SPECIAL ,OP_TWO_ARG },	
/*  171 */ {"if_else",         MET_op_if           , PRIME_RULE_SPECIAL ,OP_THREE_ARG },
/*  172 */ {"row_loop",        MET_op_unimplemented, PRIME_RULE_NONE    ,OP_THREE_ARG },
/*  173 */ {"column_loop",     MET_op_unimplemented, PRIME_RULE_NONE    ,OP_THREE_ARG },
/*  174 */ {"type_unknown",    MET_op_type_unknown , PRIME_RULE_NONE    ,OP_NO_ARG },
/*  175 */ {"compiled_code",   MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },
/*  176 */ {"for_each",        MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },
/*  177 */ {"type_and_cast",   MET_op_type_and_cast, PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  178 */ {"type_match",      MET_op_type_match   , PRIME_RULE_NONE    ,OP_TWO_ARG },
/*  179 */ {"type_matchx",     MET_op_type_match   , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  180 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  181 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  182 */ {"cross",           MET_op_cross        , PRIME_RULE_PRODUCT ,OP_TWO_ARG },	
/*  183 */ {"bars",            MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  184 */ {"apply",           MET_op_apply_infer  , PRIME_RULE_SPECIAL ,OP_TWO_ARG },	
/*  185 */ {"o.p. map",        MET_op_apply_infer,   PRIME_RULE_SPECIAL, OP_TWO_ARG },	
/*  186 */ {"double map",      MET_op_apply_infer,   PRIME_RULE_SPECIAL ,OP_TWO_ARG },	
/*  187 */ {"quote",           MET_op_quote        , PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  188 */ {"alist",           MET_op_unimplemented, PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  189 */ {"assoc",           MET_op_assoc        , PRIME_RULE_NONE    ,OP_TWO_ARG },	
/*  190 */ {"local_effect",    MET_op_local_effect , PRIME_RULE_NONE    ,OP_FOUR_ARG },	
/*  191 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  192 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
/*  193 */ {"unassigned",      MET_op_unimplemented, PRIME_RULE_NONE    ,OP_ONE_ARG },	
};
