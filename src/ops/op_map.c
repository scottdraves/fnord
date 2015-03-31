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
   "$Id: op_map.c,v 1.16 1992/06/22 17:06:36 mas Exp $";
#endif

#include "defs.h"

#define MAP_FROM		0
#define MAP_TO			1
#define MAP_NUM_KIDS		2

/*
 * take a parm declaration and non-destructively create code that inverts
 * it.  ie, functions take one parameter really.  In order for the user
 * to be able to write things like f : (R,R) -> R : (x,y) -> (x + y) ;
 * where there is more than one parameter, met replaces x and y with code
 * that extracts the appropriate part of the one true parameter.  This
 * routine generates those codes.  The code is really an inverted tree,
 * where the "leaves" of the tree are the nodes that evaluate to the
 * different local variables ("x", or "y"), and the root is the one true
 * parm.  For example, the DAG of the above expression is
 *
 *          +
 *         / \
 *        x   y
 *
 * and the generated code would be
 *
 *     left   right
 *         \ /
 *	    P
 *
 * where "P" is the one true parm, and left (resp right) is the op the
 * extracts the left (resp right) side of an ordered pair.
 *
 * SYM should be the code to invert.  ROOT should be the code to put at the
 * bottom of the generated code (P in the above example).  A hash table id
 * is returned.  The leaves of the inverted code are put in the hash table,
 * hashed on symbols.  They can be retrieved with METlist_hash_find_sym().
 * If an error is encountered, LIST_HASH_BAD_ID is returned.
 */
Boolean
MET__op_map_invert_parm_code(sym, type_guess, parm_type, root, id)
   METsym	*sym;
   METtype	*type_guess;
   METtype	**parm_type;
   METsym	*root;
   int		id;
{
   METset	*set;
   METsym	*s;
   METtype	*t, *t2;
   Boolean	result = SUCCESS;
   Boolean	build_type = ((METtype **) NULL != parm_type);
   char		buff[100];
   METlist	*list;
   int		set_type;
   extern Boolean MET_op_vector_from_pair();
   MME(MET__op_map_invert_parm_code);

   switch(sym->type) {
    case MET_SYM_TYPE_OP:
      switch(sym->info.op.code) {

       case MET_SYM_OP_TYPE_AND_CAST:
	 if (YES == build_type) {
	    MET_TYPE_NEW_UNKNOWN(t);
	    set = MET_sym_be_set_ref(sym->kids[0], t, parm_type, &set_type);
	    MET_TYPE_FREE(*parm_type);
	    MET_TYPE_FREE(t);
	    if (set_type != MET_SET_TYPE_TYPE) {
	       MET_SET_FREE_NULL(set);
	       result = FAILURE;
	    } else {
	       if (FAILURE == 
		   MET__op_map_invert_parm_code(sym->kids[1], 
						set->info.type.type, 
						&t, root, id)) {
		  result = FAILURE;
	       } else if (FAILURE == MET_type_cast(t, set->info.type.type,
						   &t2,
						   TYPE_CAST_MAKE_BOGUS)) {
		  MET_TYPE_FREE(t2);
		  MET_TYPE_FREE(t);
		  result = FAILURE;
	       } else {
		  MET_TYPE_COPY(*parm_type, t2);
		  MET_TYPE_FREE(t2);
		  MET_TYPE_FREE(t);
	       }
	       MET_SET_FREE(set);
	    }
	 }
	 break;

       case MET_SYM_OP_PAIR_BUILD:
	 if (YES == build_type) {
	    MET_TYPE_ALLOC (*parm_type);
	    (*parm_type)->type = MET_TYPE_TYPE_PAIR;
	    (*parm_type)->info.pair.left =
	       (*parm_type)->info.pair.right = MET_NULL_TYPE;
	 }
	 s = METsym_new_op(MET_SYM_OP_PAIR_LEFT, sym->origin);
	 METsym_add_kid(s, root);
	 if (FAILURE == 
	     MET__op_map_invert_parm_code(sym->kids[0], MET_NULL_TYPE,
					  (YES == build_type) ?
					  &(*parm_type)->info.pair.left
					  : (METtype **) NULL, s, id)) {
	    result = FAILURE;
	    if (build_type) MET_TYPE_FREE(*parm_type);
	 }
	 MET_SYM_FREE(s);
	    
	 s = METsym_new_op(MET_SYM_OP_PAIR_RIGHT, sym->origin);
	 METsym_add_kid(s, root);
	 if (FAILURE == 
	     MET__op_map_invert_parm_code(sym->kids[1], MET_NULL_TYPE, 
					  (YES == build_type) ?
					  &(*parm_type)->info.pair.right
					  : (METtype **) NULL, s, id)) {
	    result = FAILURE;
	    if (build_type) MET_TYPE_FREE(*parm_type);
	 }
	 MET_SYM_FREE(s);
	 break;

       case MET_SYM_OP_ROW_BUILD:
       case MET_SYM_OP_COLUMN_BUILD:
	 s = METsym_new_op(MET_SYM_OP_VECTOR_TO_PAIRS, sym->origin);
	 METsym_add_kid(s, root);
	 if (FAILURE == 
	     MET__op_map_invert_parm_code(sym->kids[0], MET_NULL_TYPE,
					  (YES == build_type) ? &t :
					  (METtype **) NULL, s, id))
	    result = FAILURE;
	 else if (build_type) {
	    if (FAILURE == MET_op_vector_from_pair (sym, t, parm_type))
	       result = FAILURE;
	    MET_TYPE_FREE (t);
	 }
	 MET_SYM_FREE(s);
	 break;

       default:
	 result = FAILURE;
      }
      break;

    case MET_SYM_TYPE_SET_REF:
      set = sym->info.set_ref.set;
      if (MET_SET_TYPE_SYMBOL == set->type) {
	 if (YES == METlist_hash_add_sym(id, set->info.symbol.sym, root)) {
	    sprintf(buff, "variable already bound: %.50s",
		    SYM_STRING(set->info.symbol.sym));
	    ERROR(sym->origin, buff, SEV_WARNING);
	 }
	 /*
	  * set the nesting level to 0.  each time we enter
	  * a new scope that defines this symbol, this is
	  * increased.
	  */
	 list = MET_list_hash_find((long) set->info.symbol.sym, id,
				   MET_list_hash_find_object,
				   (char *) set->info.symbol.sym);
	 list->data.i = 0;

	 if (build_type)
	    MET_TYPE_NEW_UNKNOWN(*parm_type);
	 
      } else {
	 ERROR(sym->origin, "cannot bind constant", SEV_FATAL);
	 result = FAILURE;
      }
      break;

    default:
      result = FAILURE;
   }

   return result;
}

int
MET_op_map_invert_parm_code(sym, type_guess, root, parm_type)
   METsym	*sym;
   METtype	*type_guess;
   METsym	*root;
   METtype	**parm_type;
{
   int		id;
   MME(MET_sym_invert_parm_code);
   CHECK_SYM(sym);
   CHECK_NULL_TYPE(type_guess);
   CHECK_SYM(root);

   id = METlist_hash_new_id();

   if (FAILURE == 
       MET__op_map_invert_parm_code(sym, type_guess, parm_type, 
				    root, id)) {
      METlist_hash_free_id(id);
      return LIST_HASH_BAD_ID;
   }

   return id;
}
      
#define DECL_NO		0
#define DECL_PUSH	1
#define DECL_POP	2

/*
 * search the code in sym for symbols.  see if those symbols are bound to
 * something in virtual table ID, if so replace them with the code they are
 * bound to.  HACK is true if we are the left child of a cast op.  in this
 * case, we know that map ops really aren't map ops, but are type ops.
 *
 * the hack exists for the following situation:  
 *  F := x ->  (x -> R : v -> v . v);
 *  U := {[1,2,3]};
 *  print F(U);
 *
 * without the hack, the (x -> R) arrow is interpreted as a lambda, thus it
 * thinks that x is a local, and it is not replaced.  the hack detects most
 * such cases.
 */

static void
MET__op_map_replace_parms_with_parm_code(sym, id, in_decl, hack)
   METsym	**sym;
   int		id;
   int		in_decl;
   Boolean	hack;
{
   int		i;
   METsym	*kid;
   METlist	*list;
   Sym		symbol;
   MME(MET__op_map_replace_parms_with_parm_code);
   CHECK_SYM(*sym);

   kid = *sym;

   if (MET_SYM_TYPE_OP == kid->type &&
       MET_SYM_OP_MAP == kid->info.op.code &&
       NO == hack) {
      MET__op_map_replace_parms_with_parm_code(kid->kids + 0, id,
					       DECL_PUSH, NO); 
      MET__op_map_replace_parms_with_parm_code(kid->kids + 1, id,
					       DECL_NO, NO); 
      MET__op_map_replace_parms_with_parm_code(kid->kids + 0, id,
					       DECL_POP, NO); 
      return;
   }

   if (MET_SYM_TYPE_SET_REF == kid->type &&
       MET_SET_TYPE_SYMBOL == kid->info.set_ref.set->type) {
      symbol = kid->info.set_ref.set->info.symbol.sym;
      list = MET_list_hash_find((long) symbol, id,
				MET_list_hash_find_object,
				(char *) symbol);
      
      if (MET_NULL_LIST != list &&
	  MET_NULL_SYM != list->object.sym) {
	 if (DECL_PUSH == in_decl) {
	    list->data.i++;
	 } else if (DECL_POP == in_decl) {
	    list->data.i--;
	 } else {
	    if (0 == list->data.i) {
	       /*
		* replace it
		*/
	       MET_SYM_FREE(*sym);
	       MET_SYM_COPY(*sym, list->object.sym);
	    }
	 }
      }
   }
   for (i = 0; i < kid->num_kids; i++)
      MET__op_map_replace_parms_with_parm_code
	 (&kid->kids[i], id, in_decl,
	  i == 0 && MET_SYM_TYPE_OP == kid->type &&
	  MET_SYM_OP_TYPE_AND_CAST == kid->info.op.code);
}

/*
 * first, generate code to extract the named parameters from the one true
 * parm.  Then replace the named parms with this code.  Also replace the
 * variable code with the one true parm.  Evaluate kid[MAP_TO].  If we got
 * a result, that is our value.  If we didn't, then a bare set and put the
 * syms of MAP_TO into it, and return that set as our value
 */
METset *
MET_op_map(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METsym	*from, *one_true_parm;
   METmap	*vert_map;
   int		id;
   METtype	*my_guess, *function_type, *temp;
   METset	*result = MET_NULL_SET;
   METenv	*new_env;
   MME(MET_op_map);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, MAP_NUM_KIDS);
#endif

   if (MET_NULL_TYPE != type &&
       MET_TYPE_TYPE_SET == type->type)
      return MET_op_type_expr(sym, type, result_type, set_type);

   from = sym->kids[MAP_FROM];
   function_type = MET_NULL_TYPE;
   my_guess = MET_NULL_TYPE;

   /*
    * if we've already been converted over to the ONE TRUE PARM, don't need
    * to do any of this.
    */
   if (MET_SYM_TYPE_VARIABLE != from->type ||
       SYM_VARIABLE_ID_PARM != from->info.variable.id) {
      METset	*dud;
      void	(*save_handler)();

      one_true_parm = MET_sym_new_variable(SYM_VARIABLE_ID_PARM,
					   sym->origin);
      id = MET_op_map_invert_parm_code(from, type, one_true_parm, &my_guess);
      if (LIST_HASH_BAD_ID == id) {
	 ERROR(sym->origin, "can't invert parm code", SEV_FATAL);
	 MET_SYM_FREE(one_true_parm);
	 MET_set_get_type(MET_NULL_SET, result_type, set_type);
	 return MET_NULL_SET;
      }
      
      if (MET_NULL_TYPE != type && MET_TYPE_TYPE_MAP == type->type)
      {
	 if (FAILURE == MET_type_compatible(type->info.map.from,
					    my_guess, NO)) {
	    ERROR(sym->origin, 
		  "Function's parameter list incompatible with type",
		  SEV_FATAL);
	    MET_SYM_FREE(one_true_parm);
	    MET_TYPE_FREE(my_guess);
	    MET_set_get_type(MET_NULL_SET, result_type, set_type);
	    METlist_hash_free_id (id);
	    return MET_NULL_SET;
	 } else {
	    (void) MET_type_cast(type->info.map.from, my_guess, &temp,
				 TYPE_CAST_TAKE_FIRST);
	    if (temp != type->info.map.from) {
	       MET_TYPE_NEW_MAP(function_type, temp, type->info.map.to);
	    } else 
	       MET_TYPE_COPY(function_type, type);
	    MET_TYPE_FREE(temp);
	 }
      }
      else {
	 MET_TYPE_NEW_UNKNOWN(temp);
	 MET_TYPE_NEW_MAP(function_type, my_guess, temp);
	 MET_TYPE_FREE(temp);
      }

      if (1) {
	 extern Boolean met__opt_on;
	 if (YES == met__opt_on) {
	    /* constant prop */
	    save_handler = MET_defaults->error_handler.handler;
	    MET_defaults->error_handler.handler = MET_error_handler_ignore;
	    dud = MET_sym_be_set_ref(sym->kids[MAP_TO], type,
				     (METtype **) NULL, INULL);
	    MET_defaults->error_handler.handler = save_handler;
	    MET_SET_FREE_NULL(dud);
	    MET_SYM_FREE(one_true_parm);
	    MET_TYPE_FREE(my_guess);
	    METlist_hash_free_id(id);
	    goto bail;
	 }
      }

      MET_SYM_FREE(from);
      MET_SYM_COPY(sym->kids[MAP_FROM], one_true_parm);
      MET_SYM_FREE(one_true_parm);

      MET__op_map_replace_parms_with_parm_code(&sym, id, DECL_NO, NO);
      METlist_hash_free_id(id);
   }

   if (MET_NULL_TYPE == function_type)
      MET_TYPE_COPY(function_type, MET_type_generic_map);


   result = MET_type_to_polybare_set(function_type, 1, &MET_set_zero, 1);
   vert_map = DATA_FIRST_VERTEX(result->info.bare.data, METmap);
   MET_SYM_COPY(vert_map->sym, sym->kids[MAP_TO]);
   new_env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV); /* Ack! */
   MET_ENV_COPY(vert_map->env, new_env);
   MET_ENV_FREE(new_env);
   if (MET_NULL_TYPE == my_guess) {
      MET_TYPE_COPY(vert_map->loose_from, function_type->info.map.from);
   } else {
      MET_TYPE_COPY(vert_map->loose_from, my_guess);
      MET_TYPE_FREE(my_guess);
   }
   MET_TYPE_FREE(function_type);

 bail:
   MET_set_get_type(result, result_type, set_type);

   return result;
}
