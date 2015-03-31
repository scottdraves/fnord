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
   "$Id: met_sym.c,v 1.27 1992/06/22 17:04:58 mas Exp $";
#endif

#define COMMON_VARIBLE_COUNT 2
static int	 MET__sym_common_variables_count;
static METsym	*MET__sym_common_variables[COMMON_VARIBLE_COUNT];

static int	 MET__sym_symbol_set_id;
static int	 MET__sym_sym_const_set_id;

void
MET_sym_initialize()
{
   int		i;
   MME(MET_sym_initialize);

   /*
    * force creation of new syms
    */
   MET__sym_common_variables_count = 0;
   for (i = 0; i < COMMON_VARIBLE_COUNT; i++)
      MET__sym_common_variables[i] =
	 MET_sym_new_variable(i, (FILOCloc *) NULL);

   /*
    * back to normal
    */
   MET__sym_common_variables_count = COMMON_VARIBLE_COUNT;

   MET__sym_symbol_set_id = METlist_hash_new_id();
   MET__sym_sym_const_set_id = METlist_hash_new_id();
}

void
MET_sym_exit()
{
   int		i;
   MME(MET_sym_exit);

   for (i = 0; i < COMMON_VARIBLE_COUNT; i++)
      MET_SYM_FREE(MET__sym_common_variables[i]);

   METlist_hash_free_id(MET__sym_symbol_set_id);
   METlist_hash_free_id(MET__sym_sym_const_set_id);
}

/*
 * free the guts (children) of a sym, but don't actually free it
 */
static void
MET__sym_free_deep_guts(sym)
   METsym	*sym;
{
   int		i;
   MME(MET__sym_free_deep_guts);
   CHECK_SYM(sym);
   

   for (i = 0; i < sym->num_kids; i++)
      MET_SYM_FREE(sym->kids[i]);

   switch (sym->type) {

    case MET_SYM_TYPE_OP:
    case MET_SYM_TYPE_CONTROL:
    case MET_SYM_TYPE_VARIABLE:
      break;

    case MET_SYM_TYPE_SET_REF:
      MET_SET_FREE(sym->info.set_ref.set);
      break;

#if 0
    case MET_SYM_TYPE_DATA:
      FREE(sym->info.data.ptr, sym->info.data.size);
      break;
#endif

    default:
      DEFAULT_ERR(sym->type);
   }
}

void
MET__sym_free_deep(sym)
   METsym	*sym;
{
   MME(MET__sym_free_deep);
   CHECK_SYM(sym);

   MET__sym_free_deep_guts(sym);

   sym->mminfo.next = MET__sym_free_list;
   MET__sym_free_list = sym;
#ifdef MET__DEBUG_MAGIC
   sym->magic = MET__MAGIC_FREE;
#endif
}

/*
 * overwrite dst with src, if id is LIST_HASH_BAD_ID, then shallow copy
 * children, 
 * otherwise deep copy them with the given id.  deep copy includes hashing.
 */
static void
MET__sym_copy_into(dst, src, id)
   METsym	*dst, *src;
   int		id;
{
   int		i;
   METlist	*list;
   MME(MET__sym_copy_into);
   CHECK_SYM(dst);
   CHECK_SYM(src);

   dst->type = src->type;
   dst->num_kids = src->num_kids;
   dst->origin = src->origin;

   if (LIST_HASH_BAD_ID != id) {
      for (i = 0; i < src->num_kids; i++) {
	 LIST_HASH_FIND_SYM(list, src->kids[i], id);
	 if (MET_NULL_LIST == list) {
	    MET_SYM_ALLOC(dst->kids[i]);
	    MET__sym_copy_into(dst->kids[i], src->kids[i], id);
	    LIST_HASH_ADD_SYM(list, src->kids[i], dst->kids[i], id);
	 } else {
	    MET_SYM_COPY(dst->kids[i], list->object.sym);
	 }
      }
   } else {
      for (i = 0; i < src->num_kids; i++)
	 MET_SYM_COPY(dst->kids[i], src->kids[i]);
   }

   switch (src->type) {
      
    case MET_SYM_TYPE_OP:
      dst->info.op.code = src->info.op.code;
      break;
      
    case MET_SYM_TYPE_CONTROL:
      dst->info.control.data = src->info.control.data;
      break;
      
    case MET_SYM_TYPE_VARIABLE:
      dst->info.variable.id = src->info.variable.id;
      break;
      
    case MET_SYM_TYPE_SET_REF:
      MET_SET_COPY(dst->info.set_ref.set, src->info.set_ref.set);
      break;
      
#if 0
    case MET_SYM_TYPE_DATA:
      i = dst->info.data.size = src->info.data.size;
      ALLOCNM(dst->info.data.ptr, char, i);
      bcopy(src->info.data.ptr, dst->info.data.ptr, i);
      break;
#endif

    default:
      DEFAULT_ERR(src->type);
   }
}

/*
 * make a real copy of a sym tree, this includes *shallow* copying set_refs
 * in it
 */
METsym *
MET__sym_copy_deep(sym)
   METsym	*sym;
{
   METsym	*baby_sym;
   int		id;
   MME(MET_copy_deep_sym);
   CHECK_SYM(sym);

   MET_SYM_ALLOC(baby_sym);

   id = METlist_hash_new_id();
   MET__sym_copy_into(baby_sym, sym, id);
   METlist_hash_free_id(id);

   return baby_sym;
}

/*
 * copy sym, but use the given children rather than the ones it has.
 * if the children *are* the ones it has, then just copy sym.
 */
METsym *
MET_sym_dup_with_kids(sym, kids)
   METsym	*sym, *kids[MET__MAX_SYM_KIDS];
{
   METsym	*result;
   int		i;
   Boolean	kid_changed = NO;
   MME(MET_sym_dup_with_kids);

   /* this is a slow + gross way to do this */ /* Ack! */
   result = METsym_new_op(MET_SYM_OP_APPLY, RNULL);
   MET_sym_force_to_sym(result, sym);
   for (i = 0; i < sym->num_kids; i++)
      if (result->kids[i] != kids[i]) {
	 kid_changed = YES;
	 MET_SYM_FREE(result->kids[i]);
	 MET_SYM_COPY(result->kids[i], kids[i]);
      }

   if (NO == kid_changed) {
      MET_SYM_FREE(result);
      MET_SYM_COPY(result, sym);
   }

   return result;
}

/*
 * routine to allocate more syms for the free list
 */
#define FUNC MET__sym_new_free_list
#define LIST MET__sym_free_list
#define TYPE METsym
#define MM_TYPE MET__MM_TYPE_SYM
#include "new_free_list.c"
#undef FUNC
#undef LIST
#undef TYPE
#undef MM_TYPE

/*
 * child is added as the next child to sym.  child_sym should have been
 * created with a METnew_sym_xxx() routine.  You can only add children to
 * apply and op syms.
 */
void
METsym_add_kid(parent, child)
   METsym	*parent, *child;
{
   MME(METsym_add_kid);
   
   CHECK_SYM(parent);
   CHECK_SYM(child);
   
#ifdef MET__DEBUG
   if (parent->type != MET_SYM_TYPE_OP)
      FATAL((ERR_SI, "not an op sym:", parent->type));
   if (parent->num_kids >= MET__MAX_SYM_KIDS)
      FATAL((ERR_S, "sym is full"));
#endif
      MET_SYM_COPY(parent->kids[parent->num_kids++], child);
}

METsym *
METsym_new_set_ref(set, origin)
   METset	*set;
   FILOCloc	*origin;
{
   int		i;
   METsym	*baby_sym;
   MME(METsym_new_set_ref);
   CHECK_SET(set);

   MET_SYM_ALLOC(baby_sym);
   baby_sym->type = MET_SYM_TYPE_SET_REF;
   baby_sym->origin = origin;
   baby_sym->num_kids = 0;
   for (i = 0; i < MET__MAX_SYM_KIDS; i++)
      baby_sym->kids[i] == MET_NULL_SYM;

   MET_SET_COPY(baby_sym->info.set_ref.set, set);

   return baby_sym;
}

/*
 * the client should start numbering its variables with
 * MET_SYM_VARIABLE_ID_FIRST, and use #s greater than that, but as low as
 * possible.
 */
METsym *
MET_sym_new_variable(variable_id, origin)
   int		variable_id;
   FILOCloc	*origin;
{
   int		i;
   METsym	*baby_sym;
   MME(MET_sym_new_variable);

   /*
    * if it's in our pre-allocated array and it has no region
    * then we can just copy a pointer
    */
   if (variable_id < MET__sym_common_variables_count && RNULL == origin) {
      MET_SYM_COPY(baby_sym, MET__sym_common_variables[variable_id]);
      return baby_sym;
   }

   /*
    * otherwise, create a new one
    */
   MET_SYM_ALLOC(baby_sym);
   baby_sym->type = MET_SYM_TYPE_VARIABLE;
   baby_sym->origin = origin;
   baby_sym->num_kids = 0;
   for (i = 0; i < MET__MAX_SYM_KIDS; i++)
      baby_sym->kids[i] == MET_NULL_SYM;
   baby_sym->info.variable.id = variable_id;

   return baby_sym;
}

METsym *
METsym_new_sym(sym, origin)
   Sym		sym;
   FILOCloc	*origin;
{
   METset	*set;
   METsym	*baby_sym;
   int		id = MET__sym_symbol_set_id;
   MME(METsym_new_sym);

   set = METlist_hash_find_set(id, sym);
   if (MET_NULL_SET == set) {
      set = MET_set_new(MET_SET_TYPE_SYMBOL);
      set->info.symbol.sym = sym;
      (void) METlist_hash_add_set(id, sym, set);
   }

   baby_sym = METsym_new_set_ref(set, origin);
   MET_SET_FREE(set);

   return baby_sym;
}

METset *
METsym_new_sym_const_set(sym)
   Sym 		sym;
{
   METset	*set;
   int		id = MET__sym_sym_const_set_id;
   MME(METsym_new_sym_const_set);

   set = METlist_hash_find_set(id, sym);
   if (MET_NULL_SET == set) {
      set = MET_set_new(MET_SET_TYPE_SYM_CONST);
      set->info.sym_const.sym = sym;
      (void) METlist_hash_add_set(id, sym, set);
   }

   return set;
}

METsym *
METsym_new_op(op_code, origin)
   int		op_code;
   FILOCloc	*origin;
{
   int		i;
   METsym	*baby_sym;
   MME(METsym_new_op);

   MET_SYM_ALLOC(baby_sym);
   baby_sym->type = MET_SYM_TYPE_OP;
   baby_sym->origin = origin;
   baby_sym->num_kids = 0;
   for (i = 0; i < MET__MAX_SYM_KIDS; i++)
      baby_sym->kids[i] == MET_NULL_SYM;
   baby_sym->info.op.code = op_code;

   return baby_sym;
}

METsym *
METsym_new_zero(origin)
   FILOCloc 	*origin;
{
   MME(METsym_new_zero);

   return METsym_new_op(MET_SYM_OP_ZERO, origin);
}

METsym *
METsym_new_control(data, origin)
   char		*data;
   FILOCloc	*origin;
{
   int		i;
   METsym	*baby_sym;
   MME(METsym_new_control);

   MET_SYM_ALLOC(baby_sym);
   baby_sym->type = MET_SYM_TYPE_CONTROL;
   baby_sym->origin = origin;
   baby_sym->info.control.data = data;
   baby_sym->num_kids = 0;
   for (i = 0; i < MET__MAX_SYM_KIDS; i++)
      baby_sym->kids[i] == MET_NULL_SYM;

   return baby_sym;
}

void
MET_sym_force_set_ref(sym, set)
   METsym	*sym;
   METset	*set;
{
   MME(MET_sym_force_set_ref);
   CHECK_SYM(sym);
   CHECK_SET(set);
   
   MET__sym_free_deep_guts(sym);

   sym->type = MET_SYM_TYPE_SET_REF;
   sym->num_kids = 0;
   MET_SET_COPY(sym->info.set_ref.set, set);
}

/*
 * make sym identical to force_to
 */
void
MET_sym_force_to_sym(sym, force_to)
   METsym	*sym, *force_to;
{
   MME(MET_sym_force_to_sym);
   CHECK_SYM(sym);
   CHECK_SYM(force_to);

   MET__sym_free_deep_guts(sym);
   MET__sym_copy_into(sym, force_to, LIST_HASH_BAD_ID);
}

static METset *
MET__sym_be_set_ref_control(sym, type_guess, result_type, set_type)
   METsym	*sym;
   METtype	*type_guess;
   METtype	**result_type;
   int		*set_type;
{
   METset	*result_set;
   METcontrol_request request;
   extern Boolean met__opt_on;
   MME(MET__sym_be_set_ref_control);

   if (YES == met__opt_on)
      goto bail;
   
   request.code = MET_CONTROL_REQUEST_VALUE;
   request.common_data = MET_defaults->controls.common_data;
   request.data = sym->info.control.data;
   if (FAILURE == (MET_defaults->controls.callback)(&request) ||
       MET_NULL_SET == request.value) {
      ERROR(sym->origin, "could not evaluate control", SEV_FATAL);
      goto bail;
   }
   MET_SET_COPY(result_set, request.value);
   MET_SET_FREE(request.value);
   request.code = MET_CONTROL_REQUEST_TYPE;
   if (FAILURE == (MET_defaults->controls.callback)(&request) ||
       MET_NULL_TYPE == request.type)
   {
      MET_set_get_type(result_set, result_type, set_type);
   }
   else
   {
      if ((METtype **) NULL != result_type)
	 MET_TYPE_COPY (*result_type, request.type);
      MET_TYPE_FREE (request.type);
      if (INULL != set_type)
	 *set_type = result_set->type;
   }
   
   return result_set;
   
 bail:
   MET_set_get_type(MET_NULL_SET, result_type, set_type);
   return MET_NULL_SET;
   
}

/*
 * convert a symbolic definition into a set, that is, make it into a set ref.
 * If anything about the type of the set we will get is known, pass it in
 * type_guess.  return NULL if unable to reduce to a set, otherwise return the
 * set. This is possible if there are controls or variables, and we are not
 * ready to resolve them yet.  Not only is the set returned, but the sym
 * becomes a set-ref to it.
 */
METset *
MET_sym_be_set_ref(sym, type_guess, result_type, set_type)
   METsym	*sym;
   METtype	*type_guess;
   METtype	**result_type;
   int		*set_type;
{
   METset	*result_set, *t;
   MME(MET_sym_be_set_ref);
   CHECK_SYM(sym);

   switch (sym->type) {

    case MET_SYM_TYPE_OP:
      result_set = OP_FUNCTION(sym->info.op.code)
	 (sym, type_guess, result_type, set_type);
      break;
   
    case MET_SYM_TYPE_CONTROL:
      result_set = MET__sym_be_set_ref_control(sym, type_guess,
					       result_type, set_type);
      break;
   
    case MET_SYM_TYPE_VARIABLE:
      /* this happens when you differentiate code with an apply in it */
      /* sigh. */
      ERROR(sym->origin, "unbound alpha (you should never see this)",
	    SEV_FATAL);
      MET_set_get_type(MET_NULL_SET, result_type, set_type);
      result_set = MET_NULL_SET;
      break;
   
    case MET_SYM_TYPE_SET_REF:
      result_set = MET_sym_eval_set_ref(sym, type_guess, result_type,
					set_type, MET_NULL_ENV);
      break;

    default:
      DEFAULT_ERR(sym->type);
   }


   /*
    * if we got something, put it into a std form and
    * xform ourselves into a set-ref
    */
   if (MET_NULL_SET != result_set) {
      t = MET_set_canonize(result_set);
      if (t != result_set) {
	 if ((METtype **) NULL != result_type)
	    MET_TYPE_FREE(*result_type);
	 (void) MET_set_have_rep(t, type_guess, result_type,
				 set_type, BNULL, sym);
      }
      MET_SET_FREE(result_set);
      MET_sym_force_set_ref(sym, t);
      return t;
   }

   /*
    * this is a good place to set a breakpoint.  The code only gets here
    * after an op fails.  So if you are failing deep down somewhere, try
    * this. 
    */
   return result_set;
}

/*
 * SYM should be a set ref, evaluate it.  This includes looking it up if it
 * is a symbol.  It includes forcing it to have a value.  Returns null for
 * failure. 
 */
METset *
MET_sym_eval_set_ref(sym, type_guess, result_type, set_type, env)
   METsym	*sym;
   METtype	*type_guess;
   METtype	**result_type;
   int		*set_type;
   METenv	*env;
{
   METset	*result_set, *set_ref;
   char		buff[100];
   extern METenv *parse_root_environment;
   MME(MET_sym_eval_set_ref);
   CHECK_SYM(sym);
   CHECK_NULL_ENV(env);

#ifdef MET__DEBUG
   if (MET_SYM_TYPE_SET_REF != sym->type)
      FATALS("sym must be set ref");
#endif

   if (FAILURE != MET_set_have_rep(sym->info.set_ref.set,
				   type_guess, result_type,
				   set_type, BNULL, sym)) {
      set_ref = sym->info.set_ref.set;
      if (MET_SET_TYPE_SYMBOL == set_ref->type) {
	 /*
	  * look up the value of the symbol in the current environment
	  */
	 result_set =
	    METenv_lookup(parse_root_environment, /* Ack! */ /* Ack! */
			  set_ref->info.symbol.sym);
	 if (MET_NULL_SET == result_set) {
	    sprintf(buff, "unbound symbol: %.50s",
		    SYM_STRING(set_ref->info.symbol.sym));
	    ERROR(sym->origin, buff, SEV_FATAL);
	    result_set = MET_NULL_SET;
	 } else {
	    /*
	     * evaluate the thing that the symbol refers to.
	     * we must free result_type, if non-null.
	     */
	    if ((METtype **) NULL != result_type)
	       MET_TYPE_FREE(*result_type);
	    if (FAILURE == MET_set_have_rep(result_set, type_guess,
					    result_type, set_type,
					    BNULL, sym)) {
	       sprintf(buff, "could not evaluate symbol ref: %.50s",
		       SYM_STRING(set_ref->info.symbol.sym));
	       ERROR(sym->origin, buff, SEV_FATAL);
	       MET_SET_FREE(result_set);
	       result_set = MET_NULL_SET;
	    }
	 }
      } else {
	 MET_SET_COPY(result_set, set_ref);
      }
   } else {
      ERROR(sym->origin, "could not evaluate set ref", SEV_FATAL);
      result_set = MET_NULL_SET;
   }

   return result_set;
}

/*
 * traverse SYM, add any dependencies we find there to SET
 * dependencies are any references to other sets, or to controls
 */
void
MET_sym_find_dependencies(sym, set)
   METsym	*sym;
   METset	*set;
{
   METlist	*list;
   int		i;
   MME(MET_sym_find_dependencies);
   CHECK_SYM(sym);
   CHECK_SET(set);

   if (MET_SYM_TYPE_SET_REF == sym->type ||
       MET_SYM_TYPE_CONTROL == sym->type) {
      MET_LIST_ALLOC(list);
      list->next = MET_NULL_LIST;
      list->mm_type = MET__MM_TYPE_SYM;
      MET_SYM_COPY(list->object.sym, sym);
      list->aux_mm_type = MET__MM_TYPE_NONE;
      MET_set_add_dependency(set, list);
      MET_LIST_FREE(list);
   }
   else if (MET_SYM_TYPE_OP == sym->type &&
	    MET_SYM_OP_QUOTE == sym->info.op.code)
      return;

   for (i = 0; i < sym->num_kids; i++)
      MET_sym_find_dependencies(sym->kids[i], set);
}
   

/*
 * non-destructively expands definitions generated by the parser.
 * The parser generates ops on the left side of applies, these are moved
 * into op nodes.  Also, comma expressions are expanded into ops with
 * more than one child.  two examples:
 *     o       sin            o           atan2
 *    / \  ->   |    and     / \  ->       / \
 *  sin  x      x         atan2 ,         x   y
 *                             / \
 *                            x   y
 *
 * The code for this whole function sucks.  rewrite it.  Ack!!
 */
METsym *
MET_sym_preprocess_definition(sym)
   METsym	*sym;
{
   METsym	*kids[MET__MAX_SYM_KIDS];
   METsym	*baby;
   int		i, op;
   MME(MET_sym_preprocess_definition);
   CHECK_SYM(sym);

   if (MET_SYM_TYPE_OP == sym->type &&
       MET_SYM_OP_APPLY == sym->info.op.code) {
      /*
       * num_kids must be 2 unless it's a pair_build, then it is 3
       */
      for (i = 0; i < sym->num_kids; i++)
	 kids[i] = MET_sym_preprocess_definition(sym->kids[i]);
      
      if (MET_SYM_TYPE_OP == kids[0]->type &&
	  kids[0]->num_kids == 0) {
	 baby = METsym_new_op(op = kids[0]->info.op.code, kids[0]->origin);
	 if (MET_SYM_OP_PAIR_BUILD == op) {
#ifdef MET__DEBUG
	    if (3 != sym->num_kids)
	       FATALS("pair build has wrong number of kids");
#endif
	    METsym_add_kid(baby, kids[1]);
	    METsym_add_kid(baby, kids[2]);
	 } else {
	    /*
	     * this could be nicer
	     */
	    if (OP_NUM_ARGS(op) & OP_FOUR_ARG &&
		
		MET_SYM_TYPE_OP == kids[1]->type &&
		MET_SYM_OP_PAIR_BUILD == kids[1]->info.op.code &&
		
		MET_SYM_TYPE_OP == kids[1]->kids[0]->type &&
		MET_SYM_OP_PAIR_BUILD == kids[1]->kids[0]->info.op.code &&
		
		MET_SYM_TYPE_OP == kids[1]->kids[0]->kids[0]->type &&
		MET_SYM_OP_PAIR_BUILD ==
		kids[1]->kids[0]->kids[0]->info.op.code) {
	       METsym_add_kid(baby, kids[1]->kids[0]->kids[0]->kids[0]);
	       METsym_add_kid(baby, kids[1]->kids[0]->kids[0]->kids[1]);
	       METsym_add_kid(baby, kids[1]->kids[0]->kids[1]);
	       METsym_add_kid(baby, kids[1]->kids[1]);
	    } else if (OP_NUM_ARGS(op) & OP_THREE_ARG &&
		       
		       MET_SYM_TYPE_OP == kids[1]->type &&
		       MET_SYM_OP_PAIR_BUILD == kids[1]->info.op.code &&
		       
		       MET_SYM_TYPE_OP == kids[1]->kids[0]->type &&
		       MET_SYM_OP_PAIR_BUILD ==
		       kids[1]->kids[0]->info.op.code) {
	       METsym_add_kid(baby, kids[1]->kids[0]->kids[0]);
	       METsym_add_kid(baby, kids[1]->kids[0]->kids[1]);
	       METsym_add_kid(baby, kids[1]->kids[1]);
	    } else if (OP_NUM_ARGS(op) & OP_TWO_ARG &&
		       
		       MET_SYM_TYPE_OP == kids[1]->type &&
		       MET_SYM_OP_PAIR_BUILD == kids[1]->info.op.code) {
	       METsym_add_kid(baby, kids[1]->kids[0]);
	       METsym_add_kid(baby, kids[1]->kids[1]);
	    } else if (OP_NUM_ARGS(op) & OP_ONE_ARG) {
	       METsym_add_kid(baby, kids[1]);
	    } else {
	       /* error */
	       MET_SYM_FREE(baby);
	       baby = METsym_new_op(MET_SYM_OP_BAD_ARGS, kids[0]->origin);
	       METsym_add_kid(baby, kids[1]);

	    }
	 }
      } else {
	 baby = METsym_new_op(MET_SYM_OP_APPLY, sym->origin);
	 METsym_add_kid(baby, kids[0]);
	 METsym_add_kid(baby, kids[1]);
      }
      for (i = 0; i < sym->num_kids; i++)
	 MET_SYM_FREE(kids[i]);
   } else
      MET_SYM_COPY(baby, sym);

   return baby;
}

void
MET_sym_clear_seen_flags(sym)
   METsym	*sym;
{
   int		i;
   MME(MET_sym_clear_seen_flags);
   CHECK_SYM(sym);

   sym->seen.flags = 0;
   for (i = 0; i < sym->num_kids; i++)
      MET_sym_clear_seen_flags(sym->kids[i]);
}



/*
 * icky thing for external code.  this is here so it can access the
 * MET_SYM_OP defintions, even though it is set code... ooh well.
 */
METset *
METset_form_pair(l, r)
   METset	*l, *r;
{
   METset	*result;
   METsym	*s[3];
   METtype	*guess;
   MME(METset_form_pair);
   
   s[0] = METsym_new_set_ref(l, RNULL);
   s[1] = METsym_new_set_ref(r, RNULL);
   s[2] = METsym_new_op(MET_SYM_OP_PAIR_BUILD, RNULL);
   METsym_add_kid(s[2], s[0]);
   METsym_add_kid(s[2], s[1]);
   MET_SYM_FREE(s[0]);
   MET_SYM_FREE(s[1]);
   MET_TYPE_NEW_UNKNOWN(guess);
   result = MET_sym_be_set_ref(s[2], guess, (METtype **) NULL, INULL);
   MET_TYPE_FREE(guess);
#ifdef MET__DEBUG
   if (s[2]->type != MET_SYM_TYPE_SET_REF) {
      WARNINGS("sym should be set ref");
   }
#endif
   MET_SYM_FREE(s[2]);

   return result;
}

   

#ifdef MET__DEBUG_PRINT

#define BUF_SIZE 4000
static char MET__filoc_buf[BUF_SIZE];
void
METsym_print(stream, sym, indent_level)
   FILE		*stream;
   METsym	*sym;
   int		indent_level;
{
   static Boolean top_level_call = YES;
   static Boolean elide_cses = NO;
   Boolean	my_top_level;
   int		i;
   MME(METsym_print);
   CHECK_NULL_SYM(sym);

   if (FNULL == stream)
      stream = stdout;

   MET_indent(stream, indent_level);
   if (MET_NULL_SYM == sym) {
      PRINT(stream, "sym is null\n");
      return;
   }
   if (YES == elide_cses &&
       YES == top_level_call) {
      top_level_call = NO;
      my_top_level = YES;
      MET_sym_clear_seen_flags(sym);
   }
   if (YES == elide_cses &&
       YES == sym->seen.flags) {
      PRINT(stream, "...\n");
   } else {
      sym->seen.flags = YES;
      PRINT(stream, "sym at %#x: type = %s, ref_count = %d, ",
	    sym,
	    MET_names_sym_type[sym->type],
	    sym->mminfo.ref_count);
      PRINT(stream, "kids =");
      if (0 == sym->num_kids)
	 PRINT(stream, " <none>");
      else
	 for (i = 0; i < sym->num_kids; i++)
	    PRINT(stream, " %#x", sym->kids[i]);
      
      switch (sym->type) {
	 
       case MET_SYM_TYPE_OP:
	 PRINT(stream, ", code = %s", OP_NAME(sym->info.op.code));
	 break;
	 
       case MET_SYM_TYPE_CONTROL:
	 PRINT(stream, ", data = %#x",
	       sym->info.control.data);
	 break;
	 
       case MET_SYM_TYPE_VARIABLE:
	 PRINT(stream, ", id = %d", sym->info.variable.id);
	 break;
	 
       case MET_SYM_TYPE_SET_REF:
	 PRINT(stream, ", set_ref = %#x", sym->info.set_ref.set);
	 break;
	 
#if 0      
       case MET_SYM_TYPE_DATA:
	 PRINT(stream, ", ptr = %#x, size = %d",
	       sym->info.data.ptr, sym->info.data.size);
	 break;
#endif
	 
       default:
	 DEFAULT_ERR(sym->type);
      }
      PRINT(stream, "\n");
      
#if 0
      MET__filoc_buf[0] = '\0';
      FILOCloc_print(sym->origin, FNULL, MET__filoc_buf, BUF_SIZE);
      MET_indent(stream, indent_level);
      PRINT(stream, "origin = %s\n", MET__filoc_buf);
#endif
      
      for (i = 0; i < sym->num_kids; i++)
	 METsym_print(stream, sym->kids[i], indent_level + 1);
   }
   if (YES == my_top_level) {
      top_level_call = YES;
   }
      
   
}
Boolean
MET__sym_has_ref(sym, mm_type, object)
   METsym	*sym;
   int		mm_type;
   MET_object	*object;
{
   int		i;
   MME(MET__sym_has_ref);

   switch(mm_type) {
    case MET__MM_TYPE_SET:
      if (MET_SYM_TYPE_SET_REF == sym->type &&
	  sym->info.set_ref.set == (METset *) object)
	 return YES;
      break;

    case MET__MM_TYPE_TYPE:
    case MET__MM_TYPE_DATA:
    case MET__MM_TYPE_VERTS:
      break;

    case MET__MM_TYPE_SYM:
      for (i = 0; i < sym->num_kids; i++)
	 if (sym->kids[i] == (METsym *) object)
	    return YES;
      break;

    case MET__MM_TYPE_BLOCK:
    case MET__MM_TYPE_CONNEX:
    case MET__MM_TYPE_LIST:
      break;
      
    default:
      DEFAULT_ERR(mm_type);
   }

   return NO;
}

#endif MET__DEBUG_PRINT

