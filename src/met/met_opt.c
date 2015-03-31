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

#include "defs.h"
#include "ops/ops.h"

#ifndef lint
static char Version[] =
   "$Id: met_opt.c,v 1.12 1992/06/22 17:04:48 mas Exp $";
#endif

/*
 * replace every variable in sym (they should all be one true parms) 
 * with force_to.  the variable becomes force to and points to force_to's
 * children.
 */
static void
MET__opt_force_var_to(sym, force_to)
   METsym	*sym, *force_to;
{
   int		i;
   MME(MET__opt_force_var_to);

   if (YES == sym->seen.flags)
      return;

   for (i = 0; i < sym->num_kids; i++)
      MET__opt_force_var_to(sym->kids[i], force_to);

   if (MET_SYM_TYPE_VARIABLE == sym->type)
      MET_sym_force_to_sym(sym, force_to);

   sym->seen.flags = YES;
}

/*
 * left should evaluate to a function.
 * return a sym that is the symbolic description of the function, with its
 * one true parm replaced by right.
 * returns NULL on failure.
 */
METsym *
MET_opt_inline(left, right)
   METsym	*left, *right;
{
   METsym	*func_sym;
   METmap	*map;
   METset	*left_set;
   MME(MET_opt_inline);

   /* perhaps this should be deep copied ?? */
   left_set = MET_sym_be_set_ref(left, MET_type_generic_map,
				 (METtype **) NULL, INULL);
   if (MET_NULL_SET == left_set)
      return MET_NULL_SYM;

   if (MET_SET_TYPE_BARE != left_set->type ||
       1 != left_set->info.bare.how_many ||
       NO == MET_type_compatible(left_set->info.bare.type,
				 MET_type_generic_map, NO)) {
      MET_SET_FREE(left_set);
      return MET_NULL_SYM;
   }

   map = DATA_FIRST_VERTEX(left_set->info.bare.data, METmap);
   MET_SYM_COPY_DEEP(func_sym, map->sym);

   MET_SET_FREE(left_set);
   MET_sym_clear_seen_flags(func_sym);
   MET__opt_force_var_to(func_sym, right);

   return func_sym;
}

static METsym *
MET__opt_peephole(sym, id)
   METsym	*sym;
   int		id;
{
   METsym	*result = MET_NULL_SYM;
   METsym	*kids[MET__MAX_SYM_KIDS];
   METset	*t;
   int		i;
   METlist	*list;
   MME(MET__opt_peephole);

   LIST_HASH_FIND_SYM(list, sym, id);

   if (MET_NULL_LIST != list) {
      MET_SYM_COPY(result, list->object.sym);
      return result;
   }

   for (i = 0; i < sym->num_kids; i++)
      kids[i] = MET__opt_peephole(sym->kids[i], id);

   if (sym->type == MET_SYM_TYPE_OP) {
      if (sym->info.op.code == MET_SYM_OP_MULTIPLY) {
	 if (MET_SYM_TYPE_SET_REF == kids[0]->type &&
	     MET_set_zero == kids[0]->info.set_ref.set &&
	     MET_SYM_TYPE_SET_REF == kids[1]->type) {
	    t = MET_set_zeroed(kids[1]->info.set_ref.set);
	    if (MET_NULL_SET != t) {
	       result = METsym_new_set_ref(t, sym->origin);
	       MET_SET_FREE(t);
	    }
	 } else if (MET_SYM_TYPE_SET_REF == kids[1]->type &&
		    MET_SYM_TYPE_SET_REF == kids[0]->type &&
		    MET_set_zero == kids[1]->info.set_ref.set) {
	    t = MET_set_zeroed(kids[0]->info.set_ref.set);
	    if (MET_NULL_SET != t) {
	       result = METsym_new_set_ref(t, sym->origin);
	       MET_SET_FREE(t);
	    }
	 } else if (MET_SYM_TYPE_SET_REF == kids[0]->type &&
		    MET_set_one == kids[0]->info.set_ref.set) {
	    MET_SYM_COPY(result, kids[1]);
	 } else if (MET_SYM_TYPE_SET_REF == kids[1]->type &&
		    MET_set_one == kids[1]->info.set_ref.set) {
	    MET_SYM_COPY(result, kids[0]);
	 } 
      } else if (sym->info.op.code == MET_SYM_OP_ADD) {
	 if (MET_SYM_TYPE_SET_REF == kids[0]->type &&
	     MET_set_zero == kids[0]->info.set_ref.set) {
	    MET_SYM_COPY(result, kids[1]);
	 } else if (MET_SYM_TYPE_SET_REF == kids[1]->type &&
		    MET_set_zero == kids[1]->info.set_ref.set) {
	    MET_SYM_COPY(result, kids[0]);
	 } 
      }
   }

   if (MET_NULL_SYM == result)
      result = MET_sym_dup_with_kids(sym, kids);

   for (i = 0; i < sym->num_kids; i++)
      MET_SYM_FREE(kids[i]);

   LIST_HASH_ADD_SYM(list, sym, result, id);

   return result;
}

/*
 * perform peephole optimization on a dag.  right now, this only includes
 * removal of 1*, 0* and 0+ operations.  nevar fails.  There is a problem
 * with how 0* operations are handled; they can only be eliminated when we
 * have 0*c where c is a set ref.  this is because we need to know what
 * type of zero to use.  I should be able to get this info.
 */
METsym *
MET_opt_peephole(sym)
   METsym	*sym;
{
   int		id;
   METsym	*result;
   MME(MET_opt_peephole);

   if (TRUE == MET_defaults->optimizations.cse_elimination) {
      id = METlist_hash_new_id();
      result = MET__opt_peephole(sym, id);
      METlist_hash_free_id(id);
   } else {
      MET_SYM_COPY(result, sym);
   }

   return result;
}

/*
 * return YES if they are the same.  May be conservative; ie can return NO
 * if they are the same, but not the other way around.
 */
static Boolean
MET__opt_cse_compare_syms(one, two)
   METsym	*one, *two;
{
   int		i;
   MME(MET__opt_cse_compare_syms);

   if (one == two)
      return YES;

   if (one->type != two->type)
      return NO;

   switch (one->type) {
    case MET_SYM_TYPE_OP:
      if (one->info.op.code != two->info.op.code)
	 return NO;
      break;
    case MET_SYM_TYPE_CONTROL:
      if (one->info.control.data != two->info.control.data)
	 return NO;
      break;
    case MET_SYM_TYPE_VARIABLE:
      if (one->info.variable.id != two->info.variable.id)
	 return NO;
      break;
    case MET_SYM_TYPE_SET_REF:
      if (one->info.set_ref.set != two->info.set_ref.set)
	 return NO;
      break;
    default:
      DEFAULT_ERR(one->type);
   }

   for (i = 0; i < one->num_kids; i++)
      if (NO == MET__opt_cse_compare_syms(one->kids[i], two->kids[i]))
	 return NO;

   return YES;
}

static Boolean
MET__opt_cse_eliminate_foundp(list, arg)
   METlist	*list;
   char		*arg;
{
   MME(MET__opt_cse_eliminate_foundp);

   return MET__opt_cse_compare_syms(list->object.sym,
				    CAST(METsym *, arg));
}

/*
 *   Hash function for strings originally suggested by P.J. Weinstein:
 *     see Aho, Sethi, and Ullman, _Compilers:_Principles,_Techniques,
 *     _and_Tools, pp 435-437.  Loosely adapted.
 */
static METsym *
MET__opt_cse_eliminate(sym, id, key)
   METsym	*sym;
   int		id;
   long		*key;
{
   METsym	*kids[MET__MAX_SYM_KIDS];
   long		temp_key, my_key = 0;
   int		i;
   METsym	*result;
   METlist	*list;
   MME(MET__opt_cse_eliminate);

   LIST_HASH_FIND_SYM(list, sym, id);
   if (MET_NULL_LIST != list) {
      *key = list->data.l;
      MET_SYM_COPY(result, list->object.sym);
      return result;
   } 

   for (i = 0; i < sym->num_kids; i++) {
      kids[i] = MET__opt_cse_eliminate(sym->kids[i],
				       id, &temp_key);

      my_key = (my_key << 5) + temp_key;
      if (0 != (temp_key = my_key & 0xffc00000))
	 my_key ^= temp_key ^ (temp_key >> 22);
   }

   my_key = my_key << 5;
   switch (sym->type) {

    case MET_SYM_TYPE_OP:
      my_key += (sym->info.op.code << 3) + 2;
      break;

    case MET_SYM_TYPE_CONTROL:
      my_key += (((long) sym->info.control.data) & 0x7f8) + 3;
      break;

    case MET_SYM_TYPE_VARIABLE:
      my_key += (sym->info.variable.id << 3) + 4;
      break;

    case MET_SYM_TYPE_SET_REF:
      my_key += (((long) sym->info.set_ref.set) & 0x7f8) + 5;
      break;

    default:
      DEFAULT_ERR(sym->type);
   }
   if (0 != (temp_key = my_key & 0xffc00000))
      my_key ^= temp_key ^ (temp_key >> 22);


   list = MET_list_hash_find(my_key, id,
			     MET__opt_cse_eliminate_foundp,
			     (char *) sym);
   if (MET_NULL_LIST != list) {
      MET_SYM_COPY(result, list->object.sym);
   } else
      result = MET_sym_dup_with_kids(sym, kids);
   
   for (i = 0; i < sym->num_kids; i++)
      MET_SYM_FREE(kids[i]);

   MET_LIST_ALLOC(list);
   list->id = id;
   list->key = (long) sym;
   list->mm_type = MET__MM_TYPE_SYM;
   MET_SYM_COPY(list->object.sym, result);
   list->aux_mm_type = MET__MM_TYPE_SYM;
   MET_SYM_COPY(list->aux_object.sym, sym);
   list->data.l = my_key;
   MET_list_hash_add(list);

   MET_LIST_ALLOC(list);
   list->id = id;
   list->key = my_key;
   list->mm_type = MET__MM_TYPE_SYM;
   MET_SYM_COPY(list->object.sym, result);
   list->aux_mm_type = MET__MM_TYPE_NONE;
   MET_list_hash_add(list);

   *key = my_key;
   return result;
}

/*
 * eliminate common sub-expressions. never fails.
 */
METsym *
MET_opt_cse_eliminate(sym)
   METsym	*sym;
{
   int		id;
   long		key;
   METsym	*result;
   MME(MET_opt_cse_eliminate);

   if (TRUE == MET_defaults->optimizations.cse_elimination) {
      id = METlist_hash_new_id();
      result = MET__opt_cse_eliminate(sym, id, &key);
      METlist_hash_free_id(id);
   } else {
      MET_SYM_COPY(result, sym);
   }

   return result;
}

/*
 * do the work of MET_opt_inline_sym:  for each apply node, call
 * MET_opt_inline.  We don't have to worry about recursion, because the
 * name of the function itself hasn't been bound yet.
 */
METsym *
MET__opt_inline_sym(sym, id)
   METsym	*sym;
   int		id;
{
   METsym	*result;
   MME(MET__opt_inline_sym);

   MET_SYM_COPY(result, sym);

   return result;
}

/*
 * inline code, except recursive functions.
 */
METsym *
MET_opt_inline_sym(sym)
   METsym	*sym;
{
   int		id;
   METsym	*result;
   MME(MET_opt_inline_sym);

   id = METlist_hash_new_id();
   result = MET__opt_inline_sym(sym, id);
   METlist_hash_free_id(id);

   return result;
}

/*
 * evaluate as much as possible.  this includes inlining.
 */
Boolean met__opt_on = NO;

METsym *
MET_opt_remove_constants(sym)
   METsym	*sym;
{
   METsym	*result;
   METset	*dud;
   void		(*save_handler)();
   MME(MET_opt_remove_constants);

   /* suppress error messages */
   save_handler = MET_defaults->error_handler.handler;
   MET_defaults->error_handler.handler = MET_error_handler_ignore;

   /* evil hack so that lambda knows to to create closures */
   met__opt_on = YES;

   /* do the evaluation */

   dud = MET_sym_be_set_ref(sym, MET_NULL_TYPE, (METtype **) NULL, INULL);
   MET_SET_FREE_NULL(dud);


   met__opt_on = NO;

   /* restore errors */
   MET_defaults->error_handler.handler = save_handler;

   MET_SYM_COPY(result, sym);

   return result;
}

/*
 * optimization with everything (yea, right, ha ha ha)
 * return NULL on failure.  destructive, i think.
 */
METsym *
MET_opt_whopper(sym)
   METsym	*sym;
{
   METsym	*t1 /*, *t2 */;
   MME(MET_opt_whopper);

   /*
   t2 = MET_opt_remove_constants(sym);
   if (MET_NULL_SYM == t2)
      return MET_NULL_SYM;

   t1 = MET_opt_peephole(t2);
   MET_SYM_FREE(t2);
   if (MET_NULL_SYM == t1)
      return MET_NULL_SYM;

   t2 = MET_opt_cse_eliminate(t1);
   MET_SYM_FREE(t1);
   if (MET_NULL_SYM == t2)
      return MET_NULL_SYM;
      */

   /* temporary experimental thing */
   /*
   t2 = MET_opt_remove_constants(sym);
   if (MET_NULL_SYM == t2)
      return MET_NULL_SYM;

   t1 = MET_opt_cse_eliminate(t2);
   MET_SYM_FREE(t2);
   if (MET_NULL_SYM == t1)
      return MET_NULL_SYM;
      */
   t1 = MET_opt_cse_eliminate(sym);

   return t1;
}
