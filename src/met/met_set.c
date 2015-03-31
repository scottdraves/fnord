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

#ifndef lint
static char Version[] =
   "$Id: met_set.c,v 1.46 1992/06/22 17:04:53 mas Exp $";
#endif

/* integers */
METset *MET_set_zero = MET_NULL_SET;
METset *MET_set_one  = MET_NULL_SET;
METset *MET_set_two  = MET_NULL_SET;
METset *MET_set_null = MET_NULL_SET;

static Boolean MET__set_constants_done;

/* forward reference */
static METdata *MET__set_data_merge();

void
MET_set_initialize()
{
   METnumber	num;
   MME(MET_set_initialize);

   MET__set_constants_done = NO;

   num.integer = 0;
   MET_set_zero = METset_new_number(MET_FIELD_INTEGER, &num);
   num.integer = 1;
   MET_set_one = METset_new_number(MET_FIELD_INTEGER, &num);
   num.integer = 2;
   MET_set_two = METset_new_number(MET_FIELD_INTEGER, &num);

   MET_set_null = MET_set_new(MET_SET_TYPE_BLOCK);
   MET_set_null->info.block.block_list = MET_NULL_BLOCK;

   MET__set_constants_done = YES;
}

void
MET_set_exit()
{
   MME(MET_set_exit);

   MET_SET_FREE(MET_set_zero);
   MET_SET_FREE(MET_set_one);
   MET_SET_FREE(MET_set_two);
   MET_SET_FREE(MET_set_null);
}

/*
 * free any reps that the set has.  the set is then of type unknown.
 */
static void
MET__set_free_reps(set)
   METset	*set;
{
   int		i;
   MME(MET__set_free_reps);
   CHECK_SET(set);

   switch (set->type) {
    case MET_SET_TYPE_UNKNOWN:
      break;

    case MET_SET_TYPE_BARE:
      MET_TYPE_FREE(set->info.bare.type);
      MET_DATA_FREE(set->info.bare.data);
      MET_tags_free(&set->info.bare.tags_bare);
      MET_tags_free(&set->info.bare.tags_in);
      break;

    case MET_SET_TYPE_BLOCK:
      MET_BLOCK_FREE_NULL(set->info.block.block_list);
      break;

    case MET_SET_TYPE_SYMBOL:
      break;

    case MET_SET_TYPE_SYM_CONST:
      break;
      
    case MET_SET_TYPE_TYPE:
      MET_TYPE_FREE(set->info.type.type);
      break;
      
    default:
      DEFAULT_ERR(set->type);
   }
   set->type = MET_SET_TYPE_UNKNOWN;
}

/*
 * free a set: free its pointers, put it back on the free list, give it bad
 * magic
 */
void
MET__set_free_deep(set)
   METset	*set;
{
   MME(MET__set_free_deep);
   CHECK_SET(set);

   /*
    * free my constituents
    */
   MET_SYM_FREE_NULL(set->definition);
   MET_LIST_FREE_NULL(set->dep_list);
   MET_ENV_FREE_NULL(set->env);
   MET__set_free_reps(set);

   /*
    * put it on the free list and cook up some bad magic
    */
   set->mminfo.next = MET__set_free_list;
   MET__set_free_list = set;
#ifdef MET__DEBUG_MAGIC
   set->magic = MET__MAGIC_FREE;
#endif
}

/*
 * copy a set and all its pointers
 */
METset *
MET__set_copy_deep(set)
   METset	*set;
{
   METset	*baby_set;
   MME(MET__set_copy_deep);
   CHECK_SET(set);

   baby_set = MET_set_new(set->type);
   MET_ENV_COPY_NULL(baby_set->env, set->env);
   switch (set->type) {
    case MET_SET_TYPE_UNKNOWN:
      break;

    case MET_SET_TYPE_BARE:
      MET_TYPE_COPY(baby_set->info.bare.type, set->info.bare.type);
      MET_DATA_COPY_DEEP(baby_set->info.bare.data, set->info.bare.data);
      baby_set->info.bare.how_many = set->info.bare.how_many;
      MET_tags_initialize(&baby_set->info.bare.tags_bare);
      MET_tags_initialize(&baby_set->info.bare.tags_in);
      MET_tags_copy(&baby_set->info.bare.tags_bare, 
		    &set->info.bare.tags_bare);
      MET_tags_copy(&baby_set->info.bare.tags_in,
		    &set->info.bare.tags_in);
      break;

    case MET_SET_TYPE_BLOCK:
      if (MET_NULL_BLOCK != set->info.block.block_list) {
	 MET_BLOCK_COPY_DEEP(baby_set->info.block.block_list,
			     set->info.block.block_list);
      } else {
	 baby_set->info.block.block_list = MET_NULL_BLOCK;
      }
      break;

    case MET_SET_TYPE_SYMBOL:
      baby_set->info.symbol.sym = set->info.symbol.sym;
      break;

    case MET_SET_TYPE_SYM_CONST:
      baby_set->info.sym_const.sym = set->info.sym_const.sym;
      break;

    case MET_SET_TYPE_TYPE:
      MET_TYPE_COPY_DEEP(baby_set->info.type.type, set->info.type.type);
      break;

    default:
      DEFAULT_ERR(set->type);
   }

   return baby_set;
}

/*
 * allocate more sets for the free list.
 */
#define FUNC MET__set_new_free_list
#define LIST MET__set_free_list
#define TYPE METset
#define MM_TYPE MET__MM_TYPE_SET
#include "new_free_list.c"
#undef FUNC
#undef LIST
#undef TYPE
#undef MM_TYPE


/*
 * create a set of the passed type.  The guts for that type are *not*
 * initialized.
 */
METset *
MET_set_new(type)
   int		type;
{
   METset	*baby_set;
   MME(MET_set_new);

   MET_SET_ALLOC(baby_set);

   baby_set->definition = MET_NULL_SYM;
   baby_set->dep_list = MET_NULL_LIST;
   baby_set->type = type;
   baby_set->env = MET_NULL_ENV;

   return baby_set;
}

/*
 * add the dependency in list to set
 * does not add things twice
 */
void
MET_set_add_dependency(set, list)
   METset	*set;
   METlist	*list;
{
   METlist	*chase;
   ME(MET_set_add_dependency);
   CHECK_SET(set);
   CHECK_LIST(list);

   /*
    * (linear) search for it
    */
   chase = set->dep_list;
   while (MET_NULL_LIST != chase) {
      if (chase->object.sym == list->object.sym)
	 return;
      chase = chase->next;
   }
   /*
    * not found -- add it to the list
    */
   MET_LIST_COPY_NULL(list->next, set->dep_list);
   MET_LIST_FREE_NULL(set->dep_list);
   MET_LIST_COPY(set->dep_list, list);
}
   

/*
 * free any dependencies a set has, and compute new ones based on its
 * definition.
 */
static void
MET__set_update_dependencies(set)
   METset	*set;
{
   MME(MET__set_update_dependencies);
   CHECK_SET(set);

   MET_LIST_FREE_NULL(set->dep_list);
   set->dep_list = MET_NULL_LIST;

   MET_sym_find_dependencies(set->definition, set);
}

/*
 * create a set with a definition and no reps
 * preprocess is YES/NO.  If it's yes, then the definition
 * is run through MET_sym_preprocess_definition() before being stuck into
 * the set.
 */
METset *
METset_new_with_definition(definition, env)
   METsym	*definition;
   METenv	*env;
{
   METset	*baby_set;
   METsym	*before_cse;
   MME(METset_new_with_definition);
   CHECK_SYM(definition);

   baby_set = MET_set_new(MET_SET_TYPE_UNKNOWN);

   if (YES == MET_defaults->preprocess.on)
      before_cse = MET_sym_preprocess_definition(definition);
   else
      MET_SYM_COPY_DEEP(before_cse, definition);

   baby_set->definition = MET_opt_whopper(before_cse);
   MET_SYM_FREE(before_cse);

   MET__set_update_dependencies(baby_set);

   return baby_set;
}

/*
 * takes a set, frees any previous symbolic definition, installs the new
 * definition (shallow copy), and frees any computed reps (which
 * were from the old definition).
 */
void
METset_redefine(set, definition)
   METset	*set;
   METsym	*definition;
{
   MME(METset_redefine);
   CHECK_SET(set);
   CHECK_SYM(definition);

   MET_SYM_FREE_NULL(set->definition);
   
   MET__set_free_reps(set);
   
   MET_SYM_COPY(set->definition, definition);
   MET__set_update_dependencies(set);
}

/*
 * a set which is a member of a base field. eg 1 or 3.4+8i
 */
METset *
METset_new_tensor(field, value, rank, dims)
   int		field;
   METnumber	*value;
   int		rank;
   int		*dims;
{
   METset	*baby_set;
   METdata	*data;
   METverts	*verts;
   METtype	*type;
   int		t;
   MME(METset_new_number);
   CHECK_PTR(value);

   if (rank > 2)
      return MET_NULL_SET;

   if (0 == rank &&
       MET_FIELD_INTEGER == field &&
       YES == MET__set_constants_done) {
      if (0 == value->integer) {
	 MET_SET_COPY(baby_set, MET_set_zero);
	 return baby_set;
      } else if (1 == value->integer) {
	 MET_SET_COPY(baby_set, MET_set_one);
	 return baby_set;
      } else if (2 == value->integer) {
	 MET_SET_COPY(baby_set, MET_set_two);
	 return baby_set;
      }
   }

   baby_set = MET_set_new(MET_SET_TYPE_BARE);
   switch (rank) {
    case 0:
      MET_TYPE_NEW_FIELD(baby_set->info.bare.type, field);
      t = 1;
      break;
    case 1:
      MET_TYPE_NEW_FIELD(type, field);
      MET_TYPE_NEW_VECTOR(baby_set->info.bare.type, type, dims[0], NO);
      MET_TYPE_FREE(type);
      t = dims[0];
      break;
    case 2:
      MET_TYPE_NEW_FIELD(baby_set->info.bare.type, field);
      MET_TYPE_NEW_VECTOR(type, baby_set->info.bare.type, dims[0], NO);
      MET_TYPE_FREE(baby_set->info.bare.type);
      MET_TYPE_NEW_VECTOR(baby_set->info.bare.type, type, dims[1], NO);
      MET_TYPE_FREE(type);
      t = dims[0] * dims[1];
      break;
    default:
      DEFAULT_ERR(rank);
   }
   baby_set->info.bare.how_many = 1;
   MET_tags_initialize(&baby_set->info.bare.tags_bare);
   MET_tags_initialize(&baby_set->info.bare.tags_in);
   verts = MET_verts_new(field, 1, t);
   data = baby_set->info.bare.data = MET_data_new(verts, t, 0);
   MET_VERTS_FREE(verts);

   bcopy((char *) value,
	 (char *) data->verts->vertices,
	 MET_field_size[field] * t);
   
   return baby_set;
}

METset *
METset_new_number(field, value)
   int		field;
   METnumber	*value;
{
   MME(METset_new_number);

   return METset_new_tensor(field, value, 0, INULL);
}

/*
 * creates a vector of integers whose entries are the ascii values of the
 * characters of the string.  strings must have at least one character in
 * it.
 */
METset *
METset_new_string(string)
   char		*string;
{
   METset	*baby_set;
   METdata	*data;
   METverts	*verts;
   METtype	*type;
   int		i, length = strlen(string);
   int		*scan;
   MME(METset_new_string);

#ifdef MET__DEBUG
   if (0 == length) {
      WARNINGS("replacing zero length string with a space");
      string = " ";
      length = 1;
   }
#endif

   baby_set = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_NEW_FIELD(type, MET_FIELD_INTEGER);
   MET_TYPE_NEW_VECTOR(baby_set->info.bare.type, type, length, NO);
   MET_TYPE_FREE(type);
   baby_set->info.bare.how_many = 1;

   MET_tags_initialize(&baby_set->info.bare.tags_bare);
   MET_tags_initialize(&baby_set->info.bare.tags_in);

   verts = MET_verts_new(MET_FIELD_INTEGER, 1, length);
   data = baby_set->info.bare.data = MET_data_new(verts, length, 0);
   MET_VERTS_FREE(verts);

   scan = (int *) data->verts->vertices;
   for (i = 0; i < length; i++)
      *scan++ = (int) *string++;

   return baby_set;
}

/*
 * fill VALUE with the value of SET if it is a simple scalar of the
 * requested field.  if field is MET_FIELD_UNKNOWN, then any field
 * is ok and the field is returned in FIELD.  SET is evaluated if need be.
 * if CAST is YES, then the set will be cast to the requested type
 * if it needs to be.
 */
Boolean
METset_get_number(set, field, value, cast)
   METset	*set;
   int		*field;
   METnumber	*value;
   Boolean	cast;
{
   METdata	*data;
   METverts	*verts;
   METtype	*type;
   METset	*real_set;
   MME(METset_get_number);

   MET_TYPE_NEW_FIELD(type, *field);

   if (FAILURE == MET_set_have_rep(set, type, (METtype **) NULL,
				   INULL, BNULL, MET_NULL_SYM)) {
      MET_TYPE_FREE(type);
      return FAILURE;
   }

   if (YES == cast)
      real_set = MET_set_cast_to_type(set, type);
   else
      MET_SET_COPY(real_set, set);

   MET_TYPE_FREE(type);

   if (MET_NULL_SET == real_set)
      return FAILURE;

   if (MET_SET_TYPE_BARE != real_set->type ||
       1 != real_set->info.bare.how_many ||
       MET_TYPE_TYPE_FIELD != real_set->info.bare.type->type ||
       (*field != real_set->info.bare.type->info.field.type &&
	*field != MET_FIELD_UNKNOWN)) {
      MET_SET_FREE(real_set);
      return FAILURE;
   }


   verts = real_set->info.bare.data->verts;
   data = real_set->info.bare.data;

    /* Ack! */ /* Ack! */
   switch(verts->type) {
    case MET_FIELD_ZTWO:
      value->ztwo = * ((METztwo *) verts->vertices +
			       data->first_col);
      break;
    case MET_FIELD_INTEGER:
      value->integer = * ((METinteger *) verts->vertices +
				  data->first_col);
      break;
    case MET_FIELD_ANGLE:
      value->angle = * ((METangle *) verts->vertices +
				data->first_col);
      break;
    case MET_FIELD_REAL:
      value->real = * ((METreal *) verts->vertices +
			       data->first_col);
      break;
    case MET_FIELD_COMPLEX:
      value->complex = * ((METcomplex *) verts->vertices +
				  data->first_col);
      break;
    case MET_FIELD_QUATERNION:
      value->quaternion = * ((METquaternion *) verts->vertices +
				     data->first_col);
      break;
    case MET_FIELD_MAP:
      MET_SYM_COPY(value->map.sym,
		   ((METmap *) verts->vertices + data->first_col)->sym);
      MET_TYPE_COPY(value->map.loose_from, ((METmap *) verts->vertices +
		     data->first_col)->loose_from);
      MET_ENV_COPY(value->map.env,
		   ((METmap *) verts->vertices + data->first_col)->env);
      break;
    case MET_FIELD_SET:
      MET_SET_COPY(value->set,
 		   * ((METset **) verts->vertices + data->first_col));
      break;

    default:
      DEFAULT_ERR(verts->type);
   }

   MET_SET_FREE(real_set);
   return SUCCESS;
}

Boolean
METset_get_symbol(set, value)
   METset	*set;
   Sym		*value;
{
   MME(METset_get_symbol);
   CHECK_SET(set);

   if (FAILURE == MET_set_have_rep(set, MET_NULL_TYPE, (METtype **) NULL,
				   INULL, BNULL, MET_NULL_SYM))
      return FAILURE;

   if (MET_SET_TYPE_SYM_CONST == set->type)
   {
      *value = set->info.sym_const.sym;
      return SUCCESS;
   }
   return FAILURE;
}

Boolean
METset_get_string(set, value, length)
   METset	*set;
   char		*value;
   int		*length;
{
   METtype	*type, *t;
   int		actual_length, *scan;
   Boolean	result = FAILURE;
   MME(METset_get_symbol);
   CHECK_SET(set);

   MET_TYPE_NEW_FIELD(t, MET_FIELD_INTEGER);
   MET_TYPE_NEW_VECTOR(type, t, TYPE_DIMENSION_UNKNOWN, NO);
   MET_TYPE_FREE(t);
   if (FAILURE == MET_set_have_rep(set, type, (METtype **) NULL,
				   INULL, BNULL, MET_NULL_SYM))
      goto bail;
   if (MET_SET_TYPE_BARE != set->type ||
       1 != set->info.bare.how_many ||
       NO == MET_type_compatible(type, set->info.bare.type, NO))
      goto bail;
   result = SUCCESS;
   actual_length = set->info.bare.type->info.vector.dimension;
   if (actual_length + 1 > *length) {
      *length = -1;
      goto bail;
   }
   *length = actual_length;
   scan = (int *) DATA_FIRST_VERTEX(set->info.bare.data, METinteger);
   while (actual_length--)
      *value++ = (char) *scan++;
   *value++ = '\0';

 bail:
   MET_TYPE_FREE(type);
   return result;
}
/*
 * do the work of an assoc.  if SYMBOL is NULL, then just skip of DATA as
 * directed by TYPE.  otherwise, search for (symbol, value) pairs and
 * check them for matches.  SET is only so we have tags to create the
 * result.
 */
static METset *
MET__set_assoc(type, data, symbol, set)
   METtype	*type;
   METdata	**data;
   Sym		symbol;
   METset	*set;
{
   METset	*result = MET_NULL_SET, **symset;
   MME(MET__set_assoc);

   if ((Sym) NULL == symbol) {
      /*
       * skipping
       */
      switch (type->type) {

       case MET_TYPE_TYPE_PAIR:
	 (void) MET__set_assoc(type->info.pair.left,  data, symbol, set);
	 (void) MET__set_assoc(type->info.pair.right, data, symbol, set);
	 break;
	 
       case MET_TYPE_TYPE_MAP:
       case MET_TYPE_TYPE_VECTOR:
       case MET_TYPE_TYPE_SET:
       case MET_TYPE_TYPE_FIELD:
       case MET_TYPE_TYPE_SET_OF:
	 *data = (*data)->next;
	 break;

       default:
	 DEFAULT_ERR(type->type);
      }
   } else {
      /*
       * searching
       */
      switch (type->type) {

       case MET_TYPE_TYPE_MAP:
       case MET_TYPE_TYPE_VECTOR:
       case MET_TYPE_TYPE_FIELD:
       case MET_TYPE_TYPE_SET_OF:
       case MET_TYPE_TYPE_UNKNOWN:
       case MET_TYPE_TYPE_BOGUS:
       case MET_TYPE_TYPE_SET:
	 break;
	 
       case MET_TYPE_TYPE_PAIR:
	 if (MET_TYPE_TYPE_SET == type->info.pair.left->type) {
	    /*
	     * this is a (symbol, value) pair, see if it matches
	     */
	    symset = DATA_FIRST_VERTEX(*data, METset *);
	    *data = (*data)->next;
	    if (MET_SET_TYPE_SYM_CONST != (*symset)->type) {
	       ERROR(RNULL, "badly formed alist", SEV_WARNING);
	       (void) MET__set_assoc(type->info.pair.right,
				     data, (Sym) NULL, set);
	    } else {
	       if (SYM_EQ((*symset)->info.sym_const.sym, symbol))
		  result =
		     MET_set_from_data_and_type(*data,
						type->info.pair.right,
						set);
	       else
		  (void) MET__set_assoc(type->info.pair.right,
					data, (Sym) NULL, set);
	    }
	 } else {
	    /*
	     * this is internal to the tree, keep searching
	     */
	    result = MET__set_assoc(type->info.pair.left, data, symbol, set);
	    if (MET_NULL_SET == result)
	       result = MET__set_assoc(type->info.pair.right,
				       data, symbol, set);
	 }
	 break;
	 
       default:
	 DEFAULT_ERR(type->type);
      }
   }

   return result;
}   

/*
 * look SYMBOL up in SET, which should be an alist.  return the value
 * with which it is associated.  return NULL if it has no association.
 */
METset *
METset_assoc(set, symbol)
   METset	*set;
   Sym		symbol;
{
   METdata	*data;
   METset	*partial, *result;
   MME(METset_assoc);
   CHECK_SET(set);

   if (MET_SET_TYPE_BARE != set->type ||
       1 != set->info.bare.how_many)
      return MET_NULL_SET;

   data = set->info.bare.data;
   partial = MET__set_assoc(set->info.bare.type, &data, symbol, set);
   if (MET_NULL_SET != partial) {
      result = MET_set_canonize(partial);
      MET_SET_FREE(partial);
   } else {
      result = MET_NULL_SET;
   }
   return result;
}

/*
 * shallow copy the reps from source to dest, freeing any reps in dest
 * (whether or not they are overwritten).
 */
static void
MET__set_copy_reps(dest, source)
   METset	*dest, *source;
{
   MME(MET__set_copy_reps);
   
   CHECK_SET(dest);
   CHECK_SET(source);

   MET__set_free_reps(dest);
   dest->type = source->type;

   switch (source->type) {
    case MET_SET_TYPE_UNKNOWN:
      break;

    case MET_SET_TYPE_BARE:
      MET_TYPE_COPY(dest->info.bare.type, source->info.bare.type);
      MET_DATA_COPY(dest->info.bare.data, source->info.bare.data);
      dest->info.bare.how_many = source->info.bare.how_many;
      MET_tags_initialize(&dest->info.bare.tags_bare);
      MET_tags_initialize(&dest->info.bare.tags_in);
      MET_tags_copy(&dest->info.bare.tags_bare,
		    &source->info.bare.tags_bare);
      MET_tags_copy(&dest->info.bare.tags_in,
		    &source->info.bare.tags_in);
      break;

    case MET_SET_TYPE_BLOCK:
      MET_BLOCK_COPY_NULL(dest->info.block.block_list,
			  source->info.block.block_list);
      break;

    case MET_SET_TYPE_SYMBOL:
      dest->info.symbol.sym = source->info.symbol.sym;
      break;

    case MET_SET_TYPE_SYM_CONST:
      dest->info.sym_const.sym = source->info.sym_const.sym;
      break;

    case MET_SET_TYPE_TYPE:
      MET_TYPE_COPY(dest->info.type.type, source->info.type.type);
      break;

    default:
      DEFAULT_ERR(source->type);
   }
}


/*
 * take a set and construct a rep for it from its definition.  If we
 * have an idea about what type that rep is going to be, pass it in
 * type_guess.  Return FAILURE if the rep could not be computed.
 *
 * if no rep could be computed, but its type could be determined,
 * then *result_type is set to point to that type.
 *
 * CHANGED is YES if the set's value is potentially different from
 * previous rep (if any). CHANGED is undefined if FAILURE is returned.
 * CHANGED may be NULL.
 */
Boolean
MET_set_have_rep(set, type_guess, result_type, set_type, changed, sym)
   METset	*set;
   METtype	*type_guess, **result_type;
   int		*set_type;
   Boolean	*changed;
   METsym	*sym;
{
   METsym	*definition;
   METset	*result_set;
   MME(MET_set_have_rep);
   CHECK_SET(set);
   CHECK_NULL_TYPE(type_guess);
   CHECK_NULL_PTR(result_type);
   CHECK_NULL_PTR(set_type);
   
   if (MET_SET_TYPE_UNKNOWN != set->type) {
      MET_set_get_type(set, result_type, set_type);
      if (BNULL != changed)
	 *changed = NO;
      return SUCCESS;
   }

   if (MET_NULL_SYM == set->definition) {
      MET_set_get_type(MET_NULL_SET, result_type, set_type);
      ERROR((MET_NULL_SYM == sym) ? RNULL : sym->origin,
	    "set to be calculated has no definition", SEV_FATAL);
      return FAILURE;
   }

   MET_SYM_COPY_DEEP(definition, set->definition);
   result_set = MET_sym_be_set_ref(definition, type_guess,
				   result_type, set_type);
   MET_SYM_FREE(definition);

   if (MET_NULL_SET == result_set) {
      if (MET_NULL_SYM != sym) {
	 ERROR(sym->origin, 
	       "could not compute set representation", SEV_FATAL);
	 ERROR(set->definition->origin,
	       "definition of bad set", SEV_FATAL);
      } else {
	 ERROR(set->definition->origin,
	       "set defined here could not be calculated", SEV_FATAL);
      }      
      return FAILURE;
   }

   MET__set_copy_reps(set, result_set);
   MET_SET_FREE(result_set);

   if (BNULL != changed)
      *changed = YES;

   return SUCCESS;
}


/*
 * check all our dependencies and see if any of them is out of date, or if
 * we are out of date.  If so,  free any reps and return YES.  The dep list
 * is a list of syms which are either controls or set refs.  for those that
 * are controls, check if the control has changed.  for those that are
 * sets, recurse on that set and see if it is dirty.
 */
static int
MET__set_clear_rep(set, id)
   METset	*set;
   int		id;
{
   int		result = MET_CLEAR_NOT_AT_ALL;
   METlist	*list, *save;
   METsym	*sym;
   METset	*sym_set;
   METcontrol_request request;
   extern METenv *parse_root_environment;
   MME(MET__set_clear_rep);
   CHECK_SET(set);

   list = MET_list_hash_find((long)set, id,
			     MET_list_hash_find_object,
			     (char *) set);

   if (MET_NULL_LIST != list) {
      if (MET_CLEAR_IN_PROGRESS == list->data.i)
	 return MET_CLEAR_NOT_AT_ALL;
      else 
	 return list->data.i;
   }

   MET_LIST_ALLOC(list);
   list->key = (long)set;
   list->id  = id;
   list->data.i = MET_CLEAR_IN_PROGRESS;
   list->mm_type = MET__MM_TYPE_SET;
   MET_SET_COPY(list->object.set, set);
   list->aux_mm_type = MET__MM_TYPE_NONE;
   MET_LIST_COPY(save, list);
   MET_list_hash_add(list);

   list = set->dep_list;
   while (MET_NULL_LIST != list) {
      sym = list->object.sym;
      switch (sym->type) {

       case MET_SYM_TYPE_CONTROL:
	 request.code = MET_CONTROL_REQUEST_CHANGED;
	 request.common_data = MET_defaults->controls.common_data;
	 request.data = sym->info.control.data;
	 if (FAILURE == (MET_defaults->controls.callback)(&request))
	    result |= MET_CLEAR_ONE_OF_MANY;
	 else
	    result |= request.changed;
	 break;

       case MET_SYM_TYPE_SET_REF:
	 sym_set = sym->info.set_ref.set;
	 if (MET_SET_TYPE_SYMBOL == sym_set->type) {
	    sym_set = METenv_lookup(parse_root_environment, /* Ack! */
				    sym_set->info.symbol.sym);
	    if (MET_NULL_SET != sym_set) {
	       result |= MET__set_clear_rep(sym_set, id);
	       MET_SET_FREE(sym_set);
	    }
	 } else
	    result |= MET__set_clear_rep(sym_set, id);
	 break;

       default:
	 DEFAULT_ERR(sym->type);
      }
      
      list = list->next;
   }

   save->data.i = result;
   MET_LIST_FREE(save);

   if (MET_CLEAR_NOT_AT_ALL != result)
      MET__set_free_reps(set);

   return result;
}

int
METset_clear_rep(set)
   METset	*set;
{
   int		result;
   int		id;
   MME(METset_clear_rep);

   id = METlist_hash_new_id();
   result = MET__set_clear_rep(set, id);
   METlist_hash_free_id(id);
   return result;
}

Boolean
METset_have_rep(set, changed)
   METset	*set;
   Boolean	*changed;
{
   MME(METset_have_rep);

   return MET_set_have_rep(set, MET_NULL_TYPE,
			   (METtype **) NULL, INULL, changed,
			   MET_NULL_SYM);
}

/*
 * Handling cases for the next big function: MET_set_remove_tag
 */

static METset *
MET__set_remove_last_tag(set, full_connex, data_connex, pts)
   METset	*set;
   METconnex	*full_connex, *data_connex;
   int		pts;
{
   METset	*result_set;
   METblock	*block;
   int		i;
   ME(MET__set_remove_last_tag);
   CHECK_SET(set);

   if (MET_NULL_CONNEX != full_connex) {
      
      result_set = MET_set_new(MET_SET_TYPE_BLOCK);
      result_set->info.block.block_list = NULL;
      MET_BLOCK_ALLOC(block);
      block->next = MET_NULL_BLOCK;
      MET_TYPE_NEW_SET_OF(block->type, set->info.bare.type);
      MET_DATA_COPY(block->data, set->info.bare.data);
      MET_CONNEX_COPY(block->connex, full_connex);
      MET_CONNEX_COPY(block->data_connex, data_connex);
      MET_tags_initialize(&block->tags_in);
      MET_tags_copy(&block->tags_in, &set->info.bare.tags_in);
      
      MET_tags_initialize(&block->tags_part);
      for(i = 0; i < set->info.bare.tags_bare.num_tags; i++)
	 if (MET_TAG_TYPE_ELEMENT == set->info.bare.tags_bare.tags[i].type)
	    MET_tags_add(&block->tags_part,
			 set->info.bare.tags_bare.tags + i,
			 FALSE /* scalar--shouldn't matter */);

      MET_set_add_block(result_set, block);
      MET_BLOCK_FREE(block);

   } else {

      result_set = MET_set_new(MET_SET_TYPE_BARE);
      MET_TYPE_COPY(result_set->info.bare.type, set->info.bare.type);
      MET_DATA_COPY(result_set->info.bare.data, set->info.bare.data);
      result_set->info.bare.how_many = set->info.bare.how_many;
      MET_tags_initialize(&result_set->info.bare.tags_bare);
      MET_tags_initialize(&result_set->info.bare.tags_in);
      MET_tags_copy(&result_set->info.bare.tags_in, 
		    &set->info.bare.tags_in);
   }

   return result_set;
}

/*
 * We have a scalar tag in a result, probably as the result of applying
 * a constant function somewhere.  We must now recopy the data elements
 * several times in order to have the right number of elements appear
 * in the output.
 */
static METset *
MET__set_unscalar_tag(set, id)
   METset	*set;
   int		id;
{
   int		i, j;
   int 		unit_to_copy;
   int 		factor_to_copy;
   int 		number_of_units;
   METverts 	*verts;
   METdata	*old_data, *new_data;
   METset	*result_set;
   METtag_set	*tags;
   MME(MET__set_unscalar_tag);

   tags = &set->info.bare.tags_bare;

   for(i = tags->num_tags - 1, unit_to_copy = 1; 
       tags->tags[i].id != id; i--) 
      unit_to_copy *= tags->tags[i].size;

   number_of_units = DATA_NUM_VERTS(set->info.bare.data) / unit_to_copy;
   factor_to_copy = tags->tags[i].size;

   result_set = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(result_set->info.bare.type, set->info.bare.type);
   result_set->info.bare.how_many = 
      number_of_units * factor_to_copy * unit_to_copy;
   
   MET_tags_initialize(&result_set->info.bare.tags_bare);
   MET_tags_initialize(&result_set->info.bare.tags_in);
   MET_tags_copy(&result_set->info.bare.tags_bare,
		 &set->info.bare.tags_bare);
   MET_tags_copy(&result_set->info.bare.tags_in,
		 &set->info.bare.tags_in);
   result_set->info.bare.tags_bare.tags[i].scalar = NO;

   for (result_set->info.bare.data = MET_NULL_DATA,
	old_data = set->info.bare.data;
	old_data != MET_NULL_DATA; old_data = old_data->next) {

      verts = MET_verts_new(old_data->verts->type,
			    result_set->info.bare.how_many,
			    old_data->num_cols);

      for (i = 0; i < number_of_units; i++) 
	 for (j = 0; j < factor_to_copy; j++)
	    MET_verts_copy_vertices(verts, i * unit_to_copy *
				    factor_to_copy + j * unit_to_copy,
				    0, old_data->verts, i * unit_to_copy,
				    old_data->first_col, old_data->num_cols,
				    unit_to_copy, 1, NO);

      new_data = MET_data_new(verts, old_data->num_cols, 0);
      MET_VERTS_FREE(verts);

      if (result_set->info.bare.data == MET_NULL_DATA) {
	 MET_DATA_COPY(result_set->info.bare.data, new_data);
      } else {
	 MET_data_append(result_set->info.bare.data, new_data);
      }
      MET_DATA_FREE(new_data);
   }

   return result_set;
}

/*
 * Tag indicates that a single chunk of data represents elements of
 * a series of sets (rather than a single one).  Split up the data
 * so that appearance and reality mesh.
 */
static METset *
MET__set_split_for_tag(set, remove, full_connex, data_connex, pts)
   METset 	*set;
   int		remove;
   METconnex  	*full_connex, *data_connex;
   int		pts;
{
   int		i, j;
   int		num_rows, num_baby_rows, num_columns;
   METblock	*block;
   METdata	*data, *current_data;
   METverts	*verts;
   METset	*result_set, **scan_verts;
   MME(MET__set_split_for_tag);
   CHECK_SET(set);

   /*
    * find out how much stays at this level and how much is
    * demoted into the subsets. 
    */
   num_baby_rows = MET_connex_num_pts(data_connex);
   num_baby_rows = (num_baby_rows > 0) ?
      num_baby_rows : - num_baby_rows;
   num_rows = pts / num_baby_rows;
   
   /*
    * make our result 
    */
   result_set = MET_set_new(MET_SET_TYPE_BARE);
   
   verts = MET_verts_new(MET_FIELD_SET, num_rows, 1);
   result_set->info.bare.data = MET_data_new(verts, 1, 0);
   MET_VERTS_FREE(verts);
   result_set->info.bare.how_many = num_rows;
   
   MET_TYPE_NEW_SET_OF(result_set->info.bare.type,
		       set->info.bare.type);
   
   MET_tags_initialize(&result_set->info.bare.tags_bare);

   /* don't look while we trick copy tags. */
   i = set->info.bare.tags_bare.num_tags;
   set->info.bare.tags_bare.num_tags = remove;
   MET_tags_copy(&result_set->info.bare.tags_bare,
		 &set->info.bare.tags_bare);
   set->info.bare.tags_bare.num_tags = i;

   MET_tags_initialize(&result_set->info.bare.tags_in);
		 
   for (i = 0, scan_verts = DATA_FIRST_VERTEX(result_set->info.bare.data,
					      METset *);
	i < DATA_NUM_VERTS(result_set->info.bare.data);
	i++) {
      scan_verts[i] = MET_set_new(MET_SET_TYPE_BLOCK);
      scan_verts[i]->info.block.block_list = MET_NULL_BLOCK;
   }
   
   /*
    * copy out the old verts into baby sets at each of the higher
    * level vertices
    */
   for (current_data = set->info.bare.data;
	current_data != NULL;
	current_data = current_data->next) {
      for (j = i = 0; i < DATA_NUM_VERTS(current_data);
	   j++, i += num_baby_rows) {
	 verts = MET_verts_new(current_data->verts->type, 
			       num_baby_rows,
			       current_data->num_cols);
	 MET_verts_copy_vertices(verts, 0, 0, 
				 current_data->verts,
				 i, current_data->first_col, 
				 current_data->num_cols,
				 num_baby_rows,
				 1, NO);
	 data = MET_data_new(verts, current_data->num_cols, 0);
	 MET_VERTS_FREE(verts);
	 
	 if (scan_verts[j]->info.block.block_list == MET_NULL_BLOCK) {
	    MET_BLOCK_ALLOC(block);
	    block->next = MET_NULL_BLOCK;
	    MET_tags_initialize(&block->tags_in);
	    MET_tags_initialize(&block->tags_part);
	    MET_tags_copy(&block->tags_in, &set->info.bare.tags_in);
	    MET_CONNEX_COPY(block->connex, full_connex);
	    MET_CONNEX_COPY(block->data_connex, data_connex);
	    MET_TYPE_NEW_SET_OF(block->type, set->info.bare.type);
	    MET_DATA_COPY(block->data, data);
	    MET_DATA_FREE(data);
	    
	    MET_set_add_block(scan_verts[j], block);
	    MET_BLOCK_FREE(block);
	 } else {
	    MET_data_append(scan_verts[j]->info.block.block_list->data,
			    data);
	    MET_DATA_FREE(data);
	 }
      }
   }

   return result_set;
}

/*
 * (non-destructively) create a new set just like SET except that
 * TAG has been removed from it.  splits the data up
 * based on the passed tag, inserting another layer of
 * sets.  New set with funky data is returned.
 */

METset *
MET_set_remove_tag(tagged_set, id)
   METset 	*tagged_set;
   int		id;
{
   int 		i, j, remove;
   int		data_count;
   METset	*result_set, *temp, *set;
   METconnex	*full_connex, *data_connex, *temp_connex;
   METtag	*tag;
   METtag_set	*tags, *tags_in;
   FOboolean	take_in, last_tag;
   MME(MET_set_remove_tag);
   CHECK_SET(tagged_set);

   MET_SET_COPY(set, tagged_set);
   tags = &set->info.bare.tags_bare;
   
   /*
    * If lower levels are due to be split, split them first 
    */
   for (i = tags->num_tags - 1; 0 <= i && tags->tags[i].id > id; i--) 
      if (YES == tags->tags[i].scalar || 
	  MET_TAG_TYPE_DOUBLE_MAP == tags->tags[i].type) {
	 temp = MET_set_remove_tag(set, tags->tags[i].id);
	 MET_SET_FREE(set);
	 MET_SET_COPY(set, temp);
	 MET_SET_FREE(temp);
	 tags = &set->info.bare.tags_bare;
      }

   remove = i;
   tag = &tags->tags[remove];
   data_count = DATA_NUM_VERTS(set->info.bare.data);
   full_connex = MET_connex_new(0, INULL, INULL);
   data_connex = MET_connex_new(0, INULL, INULL);
   tags_in = &set->info.bare.tags_in;
   
   for (last_tag = YES, i = 0; i < remove; i++) 
      if (MET_TAG_TYPE_ELEMENT != tags->tags[i].type)
	 last_tag = NO;

   j = 0; 
   while (i != tags->num_tags || j != tags_in->num_tags) {

      if (i == tags->num_tags)
	 take_in = YES;
      else if (j == tags_in->num_tags) 
	 if (NO == tags->tags[i].scalar || i == remove) 
	    take_in = NO;
	 else {
	    i++; continue;
	 }
      else if (tags_in->tags[j].id < tags->tags[i].id)
	 take_in = YES;
      else if (tags_in->tags[j].id > tags->tags[i].id)
	 take_in = NO;
      else
	 j++, take_in = NO;

      if (YES == take_in) {
	 if (NO == tags_in->tags[j].scalar &&
	     MET_TAG_TYPE_ELEMENT != tags_in->tags[j].type) {
	    temp_connex = MET_connex_cross(full_connex,
					   tags_in->tags[j].connex);
	    MET_CONNEX_FREE(full_connex);
	    MET_CONNEX_COPY(full_connex, temp_connex);
	    MET_CONNEX_FREE(temp_connex);
	 }
	 j++;
      } else {
	 if ((NO == tags->tags[i].scalar || i == remove) &&
	     MET_TAG_TYPE_ELEMENT != tags->tags[i].type) {
	    temp_connex = MET_connex_cross(full_connex,
					   tags->tags[i].connex);
	    MET_CONNEX_FREE(full_connex);
	    MET_CONNEX_COPY(full_connex, temp_connex);
	    MET_CONNEX_FREE(temp_connex);
	    temp_connex = MET_connex_cross(data_connex,
					   tags->tags[i].connex);
	    MET_CONNEX_FREE(data_connex);
	    MET_CONNEX_COPY(data_connex, temp_connex);
	    MET_CONNEX_FREE(temp_connex);
	 }
	 i++;
      }
   }

   /* recopy the data if there's a mistake resulting from
      the application of a constant function. */
   
   if (YES == tag->scalar && MET_TAG_TYPE_ELEMENT != tag->type) {
      temp = MET__set_unscalar_tag(set, tag->id);
      MET_SET_FREE(set);
      MET_SET_COPY(set, temp);
      MET_SET_FREE(temp);
   }
   
   /*
    * If this is the last real tag, there is no need to munge data;
    * just convert to a block set, and return the answer 
    */

   if (YES == last_tag) 
   {
      result_set = MET__set_remove_last_tag(set, full_connex,
					    data_connex, data_count);
   }
   /* 
    * if this tag is just an instruction to make an outer
    * product map with a higher level apply, then there is
    * nothing to do, except possibly to recopy the data as
    * needed if you have a scalar tag you're expanding.
    */
   else if (MET_TAG_TYPE_PRODUCT_MAP == tag->type ||
	    MET_TAG_TYPE_ELEMENT     == tag->type) {
      MET_SET_COPY(result_set, set);
   }
   /*
    * if it's a false scalar---keep the tags on the
    * higher level for now; when we untagify the
    * function after an apply, we'll want them there... 
    */
   else if (set->info.bare.how_many != data_count) {
      result_set = MET_set_new(MET_SET_TYPE_BARE);
      result_set->info.bare.data = MET_set_to_data(set);
      result_set->info.bare.how_many = 1;

      MET_TYPE_NEW_SET_OF(result_set->info.bare.type,
			  set->info.bare.type);
      MET_tags_initialize(&result_set->info.bare.tags_bare);
      MET_tags_initialize(&result_set->info.bare.tags_in);
      MET_tags_copy(&result_set->info.bare.tags_bare,
		    &set->info.bare.tags_bare);
      MET_tags_copy(&result_set->info.bare.tags_in,
		    &set->info.bare.tags_in);

   }
   /*
    * it's a regular old polybare set that we want to split
    * into a set of sets. 
    */
   else {
      result_set = MET__set_split_for_tag(set, remove, full_connex,
					  data_connex, data_count);
   }

   MET_CONNEX_FREE(full_connex);
   MET_CONNEX_FREE(data_connex);
   MET_SET_FREE(set);
   return result_set;
}

/*
 * takes a block set and returns a list of polybare sets containing all the
 * elements of the block set.  Each block is converted to a polybare.
 * if set is not block, null is returned.  Each element of the list has
 * object set to a bare set, and aux_object set to the associated connex.
 * this may return NULL!
 *
 * all polybares created have tag ID; such a tag will be removed through
 * the tag removal operation indicated by TYPE.
 */
METlist *
MET_set_block_to_polybare(set, id, type)
   METset	*set;
   int		id;
   int 		type;
{
   METlist	*list, *result_list = MET_NULL_LIST;
   METblock	*block;
   METset	*bare_set;
   MME(MET_set_block_to_polybare);
   CHECK_SET(set);

   if (MET_SET_TYPE_BLOCK != set->type)
      return MET_NULL_LIST;

   block = set->info.block.block_list;
   while (MET_NULL_BLOCK != block) {
      /*
       * create the bare set
       */
      bare_set = MET_block_to_polybare(block, id, type,
				       DATA_NUM_VERTS(block->data));
      /*
       * create a list refering to it
       */
      MET_LIST_ALLOC(list);
      list->mm_type = MET__MM_TYPE_SET;
      MET_SET_COPY(list->object.set, bare_set);
      MET_SET_FREE(bare_set);
      list->aux_mm_type = MET__MM_TYPE_CONNEX;
      MET_CONNEX_COPY(list->aux_object.connex, block->data_connex);
      /*
       * link it in
       */
      MET_LIST_COPY_NULL(list->next, result_list);
      MET_LIST_FREE_NULL(result_list);
      MET_LIST_COPY(result_list, list);
      MET_LIST_FREE(list);

      block = block->next;
   }

   return result_list;
}

/*
 * return a type describing a set.  it may include unknown nodes if we are
 * asked for the type of a block set that is too complicated.  It can deal
 * with a NULL set pointer (in which case it returns that we don't know
 * anything).  Either type or set_type may be NULL.  If type is not NULL, it
 * is guaranteed to point to a real object after this call.
 */
void
MET_set_get_type(set, type, set_type)
   METset	*set;
   METtype	**type;
   int		*set_type;
{
   METtype	*t;
   METblock	*block;
   MME(MET_set_get_type);

   if (MET_NULL_SET == set) {
      if ((METtype **) NULL != type)
	 MET_TYPE_NEW_UNKNOWN(*type);
      if (INULL != set_type)
	 *set_type = MET_SET_TYPE_UNKNOWN;
      return;
   }
      
   CHECK_SET(set);
   if (INULL != set_type)
      *set_type = set->type;
   
   if ((METtype **) NULL != type) {
      switch (set->type) {
	 
       case MET_SET_TYPE_BARE:
	 MET_TYPE_COPY(*type, set->info.bare.type);
	 break;
	 
       case MET_SET_TYPE_BLOCK:
	 /*
	  * return most specific template type all blocks could be cast to.
	  */
	 block = set->info.block.block_list;
	 if (MET_NULL_BLOCK == block) {
	    MET_TYPE_NEW_UNKNOWN(t);
	    MET_TYPE_NEW_SET_OF(*type, t);
	    MET_TYPE_FREE(t);
	 } else {
	    MET_TYPE_COPY(*type, block->type);
	    block = block->next;
	    while (MET_NULL_BLOCK != block) {
	       (void) MET_type_cast(*type, block->type, &t, 
				    TYPE_CAST_MAKE_BOGUS);
	       MET_TYPE_FREE(*type);
	       MET_TYPE_COPY(*type, t);
	       MET_TYPE_FREE(t);
	       block = block->next;
	    }
	 }
	 break;
	 
       case MET_SET_TYPE_SYMBOL:
       case MET_SET_TYPE_SYM_CONST:
       case MET_SET_TYPE_TYPE:
	 MET_TYPE_NEW_SET(*type);
	 break;
	 
       default:
	 DEFAULT_ERR(set->type);
      }
   }
}

METtype *
METset_get_type(set)
   METset	*set;
{
   METtype	*type;
   MME(METset_get_type);

   MET_TYPE_NEW_UNKNOWN(type);
   (void) MET_set_have_rep(set, type, (METtype **) NULL,
			   INULL, BNULL, MET_NULL_SYM);
   MET_TYPE_FREE(type);

   if (MET_SET_TYPE_TYPE == set->type) {
      MET_TYPE_COPY(type, set->info.type.type);
   } else
      type = MET_NULL_TYPE;

   return type;
}

/*
 * create and return a new set as much like SET as possible, but having
 * type TYPE.  if it has the correct type, this is just a shallow copy.
 * the real work is done by MET_data_cast_to_type.  if it can't be done,
 * then MET_NULL_SET is returned.
 */
METset *
MET_set_cast_to_type(set, type)
   METset	*set;
   METtype	*type;
{
   METdata	*data;
   METset	*result;
   METblock	*block, *new_block;
   METtype 	*new_type;
   Boolean	changed = NO;
   MME(MET_set_cast_to_type);
   CHECK_SET(set);
   CHECK_TYPE(type);
   
   switch (set->type) {
      /*
       * this should never happen
       *
       case MET_SET_TYPE_UNKNOWN:
       */

    case MET_SET_TYPE_BARE:
      data = MET_data_cast_to_type(set->info.bare.data,
				   set->info.bare.type, type);

      if (MET_NULL_DATA == data)
	 return MET_NULL_SET;

      (void) MET_type_cast(set->info.bare.type, type, &new_type,
			   TYPE_CAST_TAKE_SECOND);
      
      if (data == set->info.bare.data &&
	  new_type == set->info.bare.type) {
	 MET_SET_COPY(result, set);
      }
      else {
	 result = MET_set_new(MET_SET_TYPE_BARE);
	 MET_tags_initialize(&result->info.bare.tags_bare);
	 MET_tags_initialize(&result->info.bare.tags_in);
	 MET_tags_copy(&result->info.bare.tags_bare,
		       &set->info.bare.tags_bare);
	 MET_tags_copy(&result->info.bare.tags_in,
		       &set->info.bare.tags_in);
	 
	 MET_TYPE_COPY(result->info.bare.type, new_type);
	 MET_DATA_COPY(result->info.bare.data, data);
	 result->info.bare.how_many = set->info.bare.how_many;
      }

      MET_DATA_FREE(data);
      MET_TYPE_FREE(new_type);
      break;

    case MET_SET_TYPE_BLOCK:
      if (MET_TYPE_TYPE_UNKNOWN == type->type) {
	 MET_SET_COPY(result, set);
	 break;
      }
      if (MET_TYPE_TYPE_SET_OF != type->type)
	 return MET_NULL_SET;
      
      result = MET_set_new(MET_SET_TYPE_BLOCK);
      result->info.block.block_list = MET_NULL_BLOCK;
      
      block = set->info.block.block_list;
      while (MET_NULL_BLOCK != block) {
	 data = MET_data_cast_to_type(block->data,
				      block->type->info.set_of.of,
				      type->info.set_of.of);
	 if (MET_NULL_DATA == data) {
	    MET_SET_FREE(result);
	    return MET_NULL_SET;
	 }
	 
	 MET_BLOCK_ALLOC(new_block);
	 new_block->next = MET_NULL_BLOCK;
	  
	 (void) MET_type_cast(block->type, type, &new_block->type,
			      TYPE_CAST_TAKE_SECOND);

	 MET_DATA_COPY(new_block->data, data);
	 MET_CONNEX_COPY(new_block->connex, block->connex);
	 MET_CONNEX_COPY(new_block->data_connex, block->data_connex);
	 MET_tags_initialize(&new_block->tags_in);
	 MET_tags_copy(&new_block->tags_in, &block->tags_in);
	 MET_tags_initialize(&new_block->tags_part);
	 MET_tags_copy(&new_block->tags_part, &block->tags_part);

	 if (data != block->data || new_block->type != block->type)
	    changed = YES;

	 MET_DATA_FREE(data);
	 MET_set_add_block(result, new_block);
	 MET_BLOCK_FREE(new_block);
	 
	 block = block->next;
      }
      if (NO == changed) {
	 MET_SET_FREE(result);
	 MET_SET_COPY(result, set);
      }
      break;

    case MET_SET_TYPE_SYMBOL:
    case MET_SET_TYPE_TYPE:
      switch (type->type) {
       case MET_TYPE_TYPE_BOGUS:
       case MET_TYPE_TYPE_MAP:
       case MET_TYPE_TYPE_PAIR:
       case MET_TYPE_TYPE_VECTOR:
       case MET_TYPE_TYPE_FIELD:
       case MET_TYPE_TYPE_SET_OF:
	 result = MET_NULL_SET;
	 break;
       case MET_TYPE_TYPE_SET:
       case MET_TYPE_TYPE_UNKNOWN:
	 MET_SET_COPY(result, set);
	 break;
       default:
	 DEFAULT_ERR(type->type);
      }
      break;
      
    default:
      DEFAULT_ERR(set->type);
   }

   return result;
}

/*
 * extract one row of a polybare set and put it into a new set.
 * have it look like it has been referenced by all of the tags
 * (id-wise) that the parent has, but don't make it a false scalar.
 */
METset *
MET_set_extract_row(set, row)
   METset	*set;
   int		row;
{
   METset	*result_set;
   MME(MET_set_extract_row);

   result_set = MET_set_new(MET_SET_TYPE_BARE);
   result_set->info.bare.how_many = 1;

   MET_tags_initialize(&result_set->info.bare.tags_bare);
   MET_tags_copy(&result_set->info.bare.tags_bare,
		 &set->info.bare.tags_bare);
   MET_tags_partialize(&result_set->info.bare.tags_bare, row);

   MET_tags_initialize(&result_set->info.bare.tags_in);
   MET_tags_copy(&result_set->info.bare.tags_in,
		 &set->info.bare.tags_in);
   
   MET_TYPE_COPY(result_set->info.bare.type, set->info.bare.type);
   result_set->info.bare.data =
      MET_data_extract_row(set->info.bare.data, row);

   return result_set;
}

/*
 * extract a set element of the passed set at the passed row.
 */
METset *
MET_set_extract_set_row(set, row)
   METset	*set;
   int		row;
{
   METset	*result_set;
   METblock	*block;
   METtag_set	tags;
   MME(MET_set_extract_set_row);

   MET_tags_initialize(&tags);
   MET_tags_copy(&tags, &set->info.bare.tags_bare);
   MET_tags_partialize(&tags, row);

   MET_SET_COPY_DEEP(result_set, 
		     DATA_FIRST_VERTEX(set->info.bare.data, METset *)
		     [row * set->info.bare.data->verts->total_cols]);
   for (block = result_set->info.block.block_list;
	block != MET_NULL_BLOCK; block = block->next) {
      MET_tags_copy(&block->tags_part, &tags);
   }

   MET_tags_free(&tags);
   return result_set;
}

/*
 * extract a subset of a polybare set corresponding to all of
 * the rows with a given index on a polybare tag.  Note, this would
 * work a lot faster if we could know in advance to pick out
 * all of the partial tag indices at once....
 */
METset *
MET_set_extract_tagged_row(set, id, row)
   METset	*set;
   int		id;
   int		row;
{
   METset	*result_set;
   int		i, j, before, after, size;
   MME(MET_set_extract_tagged_row);

   for (i = 0, before = 1;
	id != set->info.bare.tags_bare.tags[i].id; i++) 
      if (NO == set->info.bare.tags_bare.tags[i].scalar &&
	  MET_TAG_TYPE_ELEMENT != set->info.bare.tags_bare.tags[i].type)
	 before *= set->info.bare.tags_bare.tags[i].size;

   for (j = i+1, after = 1;
	j < set->info.bare.tags_bare.num_tags; j++) 
      if (NO == set->info.bare.tags_bare.tags[j].scalar &&
	  MET_TAG_TYPE_ELEMENT != set->info.bare.tags_bare.tags[i].type)
	 after *= set->info.bare.tags_bare.tags[j].size;

   size = (YES == set->info.bare.tags_bare.tags[i].scalar) ?
      1 : set->info.bare.tags_bare.tags[i].size;

   if (MET_TAG_TYPE_ELEMENT == set->info.bare.tags_bare.tags[i].type) {
      MET_SET_COPY(result_set, set);
      return result_set;
   }

   result_set = MET_set_new(MET_SET_TYPE_BARE);
   result_set->info.bare.how_many = set->info.bare.how_many / size;
   
   MET_tags_initialize(&result_set->info.bare.tags_bare);
   MET_tags_copy(&result_set->info.bare.tags_bare,
		 &set->info.bare.tags_bare);

   MET_tags_initialize(&result_set->info.bare.tags_in);
   MET_tags_copy(&result_set->info.bare.tags_in,
		 &set->info.bare.tags_in);

   MET_TYPE_COPY(result_set->info.bare.type, set->info.bare.type);
   
   if (YES == set->info.bare.tags_bare.tags[i].scalar) {
      MET_DATA_COPY(result_set->info.bare.data, 
		    set->info.bare.data);
   } else {
      result_set->info.bare.data = 
	 MET_data_extract_at(set->info.bare.data, size, after, row);
   }

   result_set->info.bare.tags_bare.tags[i].type = MET_TAG_TYPE_ELEMENT;
   result_set->info.bare.tags_bare.tags[i].index = row;

   return result_set;
}

/*
 * give outside packages access to the vertices of a set.
 * This operates on the ind'th block of set, which should be a block set.
 * It returns a copy of the  connex for that block. If ind is too high,
 * then MET_NULL_CONNEX is returned.  vertices, width, and field point to
 * arrays that are filled in.  vertices will be an array of pointers
 * METnumbers.  to get the next METnumber in each of those arrays, add
 * width to the (already cast) pointer.  The type of that METnumber is in
 * the field array.  num returns the size of the arrays, and should be
 * passed in as the maximum size of the arrays.  SPECIAL is one of the
 * MET_ACCESS_VERTICES_SPECIAL_XXX constants.  PT_PART  returns the size of
 * the first data.  for text and markers, this many coordinates is used for
 * the location, the rest is the text (or the marker number).
 *
 * this function has far too many parameters.  gack.
 */
METconnex *
METset_access_vertices(set, ind, vertices, width, field,
		       length, num, special, pt_part)
   METset	*set;
   int		ind;
   METnumber	**vertices;
   int		*width, *field, *length, *num, *special, *pt_part;
{
   METblock	*block;
   METdata	*data;
   int		col = 0, i;
   MME(METset_access_vertices);
   CHECK_SET(set);
   CHECK_PTR(vertices);
   CHECK_PTR(width);
   CHECK_PTR(field);
   CHECK_PTR(length);
   CHECK_PTR(num);

   if (MET_SET_TYPE_BLOCK != set->type)
      return MET_NULL_CONNEX;

   block = set->info.block.block_list;
   while (ind--) {
      if (MET_NULL_BLOCK == block)
	 return MET_NULL_CONNEX;
      block = block->next;
   }
   if (MET_NULL_BLOCK == block)
      return MET_NULL_CONNEX;


   if (MET_type_compatible(block->type->info.set_of.of,
			   MET_type_access_text, NO))
      *special = MET_ACCESS_VERTICES_SPECIAL_TEXT;
   else if (MET_type_compatible(block->type->info.set_of.of,
				MET_type_access_marker, NO))
      *special = MET_ACCESS_VERTICES_SPECIAL_MARKER;
   else
      *special = MET_ACCESS_VERTICES_SPECIAL_NONE;

   i = 0;
   data = block->data;
   *pt_part = data->num_cols;
   while (MET_NULL_DATA != data) {
      i += data->num_cols;
      data = data->next;
   }
   if (*num < i) {
      *num = i;
      return MET_NULL_CONNEX;
   }

   data = block->data;
   while (MET_NULL_DATA != data) {
      for (i = 0; i < data->num_cols; i++) {
	 width[col] = data->verts->total_cols;
	 *length = DATA_NUM_VERTS(data); /* this will be overoverwritten */
	 switch(field[col] = data->verts->type) {
	  case MET_FIELD_ZTWO:
	    vertices[col] =
	       CAST(METnumber *, DATA_FIRST_VERTEX(data, METztwo) + i);
	    break;
	  case MET_FIELD_INTEGER:
	    vertices[col] =
	       CAST(METnumber *, DATA_FIRST_VERTEX(data, METinteger) + i);
	    break;
	  case MET_FIELD_REAL:
	    vertices[col] =
	       CAST(METnumber *, DATA_FIRST_VERTEX(data, METreal) + i);
	    break;
	  case MET_FIELD_COMPLEX:
	    vertices[col] =
	       CAST(METnumber *, DATA_FIRST_VERTEX(data, METcomplex) + i);
	    break;
	  case MET_FIELD_MAP:
	  case MET_FIELD_SET:
	    /*
	     * skip such columns
	     */
	    col--;
	    break;
	  default:
	    DEFAULT_ERR(field[col]);
	 }
	 col++;
      }

      data = data->next;
   }

   *num = col;
   return block->data_connex;
}


static METdata *
MET__set_data_merge(d1, d2)
   METdata	*d1;
   METdata	*d2;
{
   METdata	*baby;
   METverts	*verts;
   MME(MET__set_data_merge);

   verts = MET_verts_new(d1->verts->type,
			 d1->verts->num_verts + d2->verts->num_verts,
			 d1->num_cols);

   
   MET_verts_copy_vertices(verts, 0, 0,
			   d2->verts, 0, d2->first_col,
			   d2->num_cols, d2->verts->num_verts, 1, NO);
   
   MET_verts_copy_vertices(verts, d2->verts->num_verts, 0,
			   d1->verts, 0, d1->first_col,
			   d1->num_cols, d1->verts->num_verts, 1, NO);
   
   baby = MET_data_new(verts, d1->num_cols, 0);
   MET_VERTS_FREE(verts);

   if (MET_NULL_DATA != d1->next)
      baby->next = MET__set_data_merge(d1->next, d2->next);

   return baby;
}

/*
 * adds a copy of the passed block to the block list of the
 * passed set.  this is destructive to SET but not to BLOCK.
 */
void
MET_set_add_block(set, block)
   METset	*set;
   METblock	*block;
{
   METblock	*t, *baby;
   METconnex	*connex;
   MME(MET_set_add_block);


   if (MET_NULL_BLOCK == block)
      return;

   t = set->info.block.block_list;
   if (MET_NULL_BLOCK != t) {

      if (YES == MET_type_compatible(block->type, t->type, NO) &&
	  NO == MET_type_unknown(block->type) &&
	  NO == MET_type_unknown(t->type) &&
	  YES == MET_tags_equal(&t->tags_in, &block->tags_in) &&
	  YES == MET_tags_equal(&t->tags_part, &block->tags_part) &&
	  MET_NULL_CONNEX != 
	  (connex = MET_connex_union(t->connex, block->connex))) {
	 /*
	  * both are the same type, and we can merge connexes, so we can
	  * combine them into one block.  this is only working on the first
	  * blocks, but that's good most of the time.  it's only an
	  * optimization, so it doesn't matter.
	  *
	  * they must also both be not false scalar, or false with the same
	  * origin. 
	  */
	 MET_BLOCK_ALLOC(baby);
	 MET_tags_initialize(&baby->tags_in);
	 MET_tags_initialize(&baby->tags_part);
	 MET_tags_copy(&baby->tags_in, &t->tags_in);
	 MET_tags_copy(&baby->tags_part, &t->tags_part);
	 MET_BLOCK_COPY_NULL(baby->next, block->next);
	 MET_TYPE_COPY(baby->type, block->type);
	 MET_CONNEX_COPY(baby->connex, connex);
	 baby->data_connex = MET_connex_union(t->data_connex,
					      block->data_connex);
	 MET_CONNEX_FREE(connex);

	 baby->data = MET__set_data_merge(block->data, t->data);

	 MET_BLOCK_COPY_NULL(t, set->info.block.block_list->next);
	 MET_BLOCK_FREE(set->info.block.block_list);
	 set->info.block.block_list = t;

	 if (MET_NULL_BLOCK != t) {
	    while (MET_NULL_BLOCK != t->next)
	       t = t->next;
	    MET_BLOCK_COPY(t->next, baby);
	 } else {
	    MET_BLOCK_COPY(set->info.block.block_list, baby);
	 }
	 MET_BLOCK_FREE(baby);
      } else {
	 while (MET_NULL_BLOCK != t->next)
	    t = t->next;
	 MET_BLOCK_COPY(t->next, block);
      }
   } else {
      MET_BLOCK_COPY(set->info.block.block_list, block);
   }
}

/*
 * Fits two polybare sets together (as we might need to do if the
 * result of an apply does not come out block when complicated
 * tagged input is being used).  Set1 and set2 should have the
 * same tags.
 */

METset *
MET_set_polybare_merge(set1, set2)
   METset	*set1, *set2;
{
   METset	*result;
   MME(MET_set_polybare_merge);
   CHECK_SET(set1);
   CHECK_SET(set2);

   if (MET_SET_TYPE_BARE != set1->type ||
       MET_SET_TYPE_BARE != set2->type)
      return MET_NULL_SET;

   if (NO == MET_type_compatible(set1->info.bare.type,
				 set2->info.bare.type, NO)) 
      return MET_NULL_SET;

   result = MET_set_new(MET_SET_TYPE_BARE);
   
   MET_TYPE_COPY(result->info.bare.type, set1->info.bare.type);
   result->info.bare.how_many = set1->info.bare.how_many +
      set2->info.bare.how_many;
   MET_tags_initialize(&result->info.bare.tags_bare);
   MET_tags_initialize(&result->info.bare.tags_in);
   MET_tags_copy(&result->info.bare.tags_bare,
		 &set1->info.bare.tags_bare);
   MET_tags_copy(&result->info.bare.tags_in,
		 &set1->info.bare.tags_in);

   result->info.bare.data = MET__set_data_merge(set1->info.bare.data,
						set2->info.bare.data);
   
   return result;
}

/*
 * take a set, return a set that has the same structure, but is all zeroes.
 * only works if it's bare, otherwise return NULL.
 */
METset *
MET_set_zeroed(set)
   METset	*set;
{
   METset	*result;
   METdata	*data;
   METverts	*verts;
   MME(MET_set_zeroed);
   CHECK_SET(set);

   if (MET_SET_TYPE_BARE != set->type)
      return MET_NULL_SET;

   if (set->info.bare.type->type == MET_TYPE_TYPE_FIELD &&
       (set->info.bare.type->info.field.type == MET_FIELD_REAL ||
	set->info.bare.type->info.field.type == MET_FIELD_INTEGER)) {
      MET_SET_COPY(result, MET_set_zero);
      return result;
   }

   /* deep copies up to the verts */
   MET_SET_COPY_DEEP(result, set);

   data = result->info.bare.data;
   while (MET_NULL_DATA != data) {
      verts = MET_verts_new(data->verts->type, data->verts->num_verts,
			    data->num_cols);
      MET_verts_zero(verts);
      MET_VERTS_FREE(data->verts);
      MET_VERTS_COPY(data->verts, verts);
      MET_VERTS_FREE(verts);
      data = data->next;
   }
   
   return result;
}

/*
 * create and return a bare set with the given type and data.
 * a deep copy of the needed portion of data is made.  tags
 * are stolen from SET.
 */
METset *
MET_set_from_data_and_type(data, type, set)
   METdata	*data;
   METtype	*type;
   METset	*set;
{
   METdata	*new_data, *t;
   METset	*result;
   MME(MET_set_from_data_and_type);
   CHECK_DATA(data);
   CHECK_TYPE(type);

   t = MET_data_index(data, MET_type_count_data(type));
   new_data = MET_data_copy_upto(data, t);
   MET_DATA_FREE_NULL(t);
   result = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(result->info.bare.type, type);
   MET_DATA_COPY(result->info.bare.data, new_data);
   result->info.bare.how_many = DATA_NUM_VERTS(new_data);
   MET_DATA_FREE(new_data);
   MET_tags_initialize(&result->info.bare.tags_bare);
   MET_tags_initialize(&result->info.bare.tags_in);
   MET_tags_copy(&result->info.bare.tags_bare,
		 &set->info.bare.tags_bare);
   MET_tags_copy(&result->info.bare.tags_in,
		 &set->info.bare.tags_in);

   return result;
}

/*
 * put a set into a standard form.  right now this means popping off a
 * MET_FIELD_SET layer, if there is one---this may be destructive to set
 * in that it changes the data, but it should only make any operations
 * on the set faster and better.
 */
METset *
MET_set_canonize(set)
   METset	*set;
{
   METset	*result, **in_data;
   MME(MET_set_canonize);
   CHECK_SET(set);

   if (MET_SET_TYPE_BARE != set->type) {
      MET_SET_COPY(result, set);
      return result;
   } else if (!(1 == set->info.bare.how_many &&
		0 == set->info.bare.tags_bare.num_tags &&
		0 == set->info.bare.tags_in.num_tags &&
		(MET_TYPE_TYPE_SET_OF == set->info.bare.type->type ||
		 MET_TYPE_TYPE_SET == set->info.bare.type->type))) {
      MET_SET_COPY(result, set);
      return result;
   }

   in_data = DATA_FIRST_VERTEX(set->info.bare.data, METset *);
   MET_SET_COPY(result, *in_data);
   return result;
}

/*
 * put a set into a data using MET_FIELD_SET.  this is sort of the inverse
 * of MET_set_canonize() (although in the future canonize may do more than
 * this).
 */
METdata *
MET_set_to_data(set)
   METset	*set;
{
   METdata	*data;
   METverts	*verts;
   METset	**dst_set;
   MME(MET_set_to_data);
   
   verts = MET_verts_new(MET_FIELD_SET, 1, 1);
   data = MET_data_new(verts, 1, 0);
   MET_VERTS_FREE(verts);
   dst_set = DATA_FIRST_VERTEX(data, METset *);
   MET_SET_COPY(*dst_set, set);

   return data;
}

METset *
MET_set_de_canonize(set)
   METset	*set;
{
   METset	*result_set;
   METtype	*type;
   MME(MET_set_de_canonize);

   result_set = MET_set_new(MET_SET_TYPE_BARE);
   MET_set_get_type(set, &type, INULL);
   MET_TYPE_COPY(result_set->info.bare.type, type);
   MET_TYPE_FREE(type);
   MET_tags_initialize(&result_set->info.bare.tags_bare);
   MET_tags_initialize(&result_set->info.bare.tags_in);
   result_set->info.bare.data = MET_set_to_data(set);
   result_set->info.bare.how_many = 1;
   return result_set;
}

void
METset_print_pretty(stream, set, s, slen)
   FILE		*stream;
   METset	*set;
   char		*s;
   int		slen;
{
   METblock 	*block;
   char		buf[8192];
   MME(METset_print_pretty);

   buf[0] = '\0';
   switch (set->type) {
    case MET_SET_TYPE_UNKNOWN:
      (void) strcpy(buf, "unknown");
      break;

    case MET_SET_TYPE_BARE:
      METtype_print_pretty(FNULL, set->info.bare.type, buf, 0);
      (void) strcat(buf, " : ");
      METdata_print_pretty(FNULL,
			   set->info.bare.data,
			   set->info.bare.type, buf, 0);
      break;

    case MET_SET_TYPE_BLOCK:
      block = set->info.block.block_list;
      if (MET_NULL_BLOCK == block) {
	 (void) strcat(buf, "{}");
      } else {
	 while (MET_NULL_BLOCK != block) {
	    METblock_print_pretty(FNULL, block, buf, 0);
	    block = block->next;
	    if (MET_NULL_BLOCK != block)
	       (void) strcat(buf, " union\n");
	 }
      }
      break;

    case MET_SET_TYPE_SYMBOL:
      (void) sprintf(buf, "*%s",
		     SYM_STRING(set->info.symbol.sym));
      break;

    case MET_SET_TYPE_SYM_CONST:
      (void) sprintf(buf, "`%s",
		     SYM_STRING(set->info.sym_const.sym));
      break;

    case MET_SET_TYPE_TYPE:
      (void) strcpy(buf, "type ");
      METtype_print_pretty(FNULL, set->info.type.type, buf, 0);
      break;

    default:
      DEFAULT_ERR(set->type);
   }

   if (FNULL != stream)
      (void) fprintf(stream, "%s", buf);
   if (CNULL != s)
      (void) strcat(s, buf);
}

#define NULL_BLOCK  0x01
#define LAST_BLOCK  0x02
#define MORE_BLOCK  0x03
#define GET_STATE   0x04

Boolean
METset_write(set, buf, len)
   METset		*set;
   unsigned char	**buf;
   int			*len;
{
   METblock	*block;
   MME(METset_write);
   
   if (sizeof(int) > *len)
      return FAILURE;

   WRITE_INTEGER(*buf, set->type);
   *len -= sizeof(int);

   switch (set->type) {
    case MET_SET_TYPE_UNKNOWN:
      break;
      
    case MET_SET_TYPE_BARE:
      return (METtype_write(set->info.bare.type, buf, len) &&
	      METdata_write(set->info.bare.data, buf, len));

    case MET_SET_TYPE_BLOCK:
      block = set->info.block.block_list;
      if (MET_NULL_BLOCK == block) 
	 if (sizeof(int) > *len)
	    return FAILURE;
	 else {
	    WRITE_INTEGER(*buf, NULL_BLOCK);
	    *len -= sizeof(int);
	 }
      else do {
	 if (sizeof(int) > *len)
	    return FAILURE;
	 WRITE_INTEGER(*buf, ((MET_NULL_BLOCK == block->next) ?
			      LAST_BLOCK : MORE_BLOCK));
	 *len -= sizeof(int);
	 if (FAILURE == METblock_write(block, buf, len))
	    return FAILURE;
	 block = block->next;
      } while (MET_NULL_BLOCK != block);
      break;

    case MET_SET_TYPE_SYMBOL:
      NYET;
      break;

    case MET_SET_TYPE_SYM_CONST:
      NYET;
      break;

    case MET_SET_TYPE_TYPE:
      return METtype_write(set->info.type.type, buf, len);

    default:
      DEFAULT_ERR(set->type);
   }

   return SUCCESS;
}

static Boolean
MET__set_init(set)
   METset	*set;
{
   MME(MET__set_init);

   switch(set->type) {
    case MET_SET_TYPE_UNKNOWN:
      return FAILURE;
    case MET_SET_TYPE_BARE:
      set->info.bare.type = MET_NULL_TYPE;
      set->info.bare.data = MET_NULL_DATA;
      MET_tags_initialize(&set->info.bare.tags_bare);
      MET_tags_initialize(&set->info.bare.tags_in);
      return SUCCESS;
    case MET_SET_TYPE_BLOCK:
      set->info.block.block_list = MET_NULL_BLOCK;
      return SUCCESS;
    case MET_SET_TYPE_SYMBOL:
      NYET;
      return FAILURE;
    case MET_SET_TYPE_SYM_CONST:
      NYET;
      return FAILURE;
    case MET_SET_TYPE_TYPE:
      set->info.type.type = MET_NULL_TYPE;
      return SUCCESS;
    default:
      return FAILURE;
   }

}

METset *
METset_read(buf, len, used, list, last, cont)
   unsigned char	*buf;
   int			len;
   int			*used;
   METlist		**list;
   MET_object_p		last;
   Boolean		cont;
{
   METset		*set;
   METblock		*block;
   METlist		*temp;
   MET_object_p		ob;
   int			type;
   int			count;
   MME(METset_read);

   if (NO == cont) {
      if (sizeof(int) > len)
	 return MET_NULL_SET;
      
      if (MET_NULL_LIST != *list) {
	 MET_LIST_COPY(temp, *list);
	 MET_LIST_FREE(*list);
      } else 
	 temp = MET_NULL_LIST;

      MET_LIST_ALLOC(*list);
      
      if (MET_NULL_LIST != temp) {
	 MET_LIST_COPY((*list)->next, temp);
	 MET_LIST_FREE(temp);
      } else
	 (*list)->next = MET_NULL_LIST;

      (*list)->mm_type = MET__MM_TYPE_SET;
      (*list)->aux_mm_type = MET__MM_TYPE_NONE;
      (*list)->data.i = GET_STATE;

      READ_INTEGER(buf, type);
      len -= sizeof(int);
      *used += sizeof(int);
      (*list)->object.set = MET_set_new(type);
      if (FAILURE == MET__set_init((*list)->object.set)) {
	 MET_LIST_FREE(*list);
	 *list = MET_NULL_LIST;
	 return MET_NULL_SET;
      }
   } 

   MET_SET_COPY(set, (*list)->object.set);

   switch(set->type) {
    case MET_SET_TYPE_UNKNOWN:
      break;

    case MET_SET_TYPE_BARE:
      if (MET_NULL_TYPE == set->info.bare.type) {
	 if (YES == cont && MET_NULL_TYPE != last.type) {
	    MET_TYPE_COPY(set->info.bare.type, last.type);
	    last.type = MET_NULL_TYPE;
	 } else {
	    count = 0;
	    ob.type = MET_NULL_TYPE;
	    set->info.bare.type = METtype_read(buf, len, &count, 
					       list, ob, FALSE);
	    buf += count;
	    *used += count;
	    len -= count;
	    if (MET_NULL_TYPE == set->info.bare.type) {
	       MET_SET_FREE(set);
	       return MET_NULL_SET;
	    }
	 }
      }
      if (MET_NULL_DATA == set->info.bare.data) {
	 if (YES == cont && MET_NULL_DATA != last.data) {
	    MET_DATA_COPY(set->info.bare.data, last.data);
	    last.data = MET_NULL_DATA;
	 } else {
	    count = 0;
	    ob.data = MET_NULL_DATA;
	    set->info.bare.data = METdata_read(buf, len, &count, 
					       list, ob, FALSE);
	    buf += count;
	    *used += count;
	    len -= count;
	    if (MET_NULL_DATA == set->info.bare.data) {
	       MET_SET_FREE(set);
	       return MET_NULL_SET;
	    }
	 }
      }
      break;

    case MET_SET_TYPE_BLOCK:
      if (YES == cont && MET_NULL_BLOCK != last.block) {
	 MET_BLOCK_COPY(block, last.block);
	 goto add_block;
      }
      if (YES == cont && GET_STATE != (*list)->data.i) {
	 goto read_block;
      }
      
      do {
	 if (sizeof(int) > len) {
	    (*list)->data.i = GET_STATE;
	    MET_SET_FREE(set);
	    return MET_NULL_SET;
	 } else {
	    READ_INTEGER(buf, (*list)->data.i);
	    *used += sizeof(int);
	    len -= sizeof(int);
	 }

	 if (NULL_BLOCK == (*list)->data.i)
	    break;
	 
       read_block:
	 count = 0;
	 ob.type = MET_NULL_TYPE;
	 block = METblock_read(buf, len, &count, list, ob, FALSE);
	 buf += count;
	 *used += count;
	 len -= count;
	 if (MET_NULL_BLOCK == block) {
	    MET_SET_FREE(set);
	    return MET_NULL_SET;
	 }

       add_block:
	 MET_set_add_block(set, block);
	 MET_BLOCK_FREE(block);
      } while (MORE_BLOCK == (*list)->data.i);
      break;

    case MET_SET_TYPE_TYPE:
      if (MET_NULL_TYPE == set->info.type.type) {
	 if (YES == cont && MET_NULL_TYPE != last.type) {
	    MET_TYPE_COPY(set->info.type.type, last.type);
	    last.type = MET_NULL_TYPE;
	 } else {
	    count = 0;
	    ob.type = MET_NULL_TYPE;
	    set->info.type.type = METtype_read(buf, len, &count, 
					       list, ob, FALSE);
	    buf += count;
	    *used += count;
	    len -= count;
	    if (MET_NULL_TYPE == set->info.type.type) {
	       MET_SET_FREE(set);
	       return MET_NULL_SET;
	    }
	 }
      }
      break;

    default:
      MET_SET_FREE(set);
      MET_LIST_FREE(*list);
      *list = MET_NULL_LIST;
      return MET_NULL_SET;
   }

   if (MET_NULL_LIST != (*list)->next) {
      MET_LIST_COPY(temp, (*list)->next);
      MET_LIST_FREE(*list);
      MET_LIST_COPY(*list, temp);
      MET_LIST_FREE(temp);
   } else {
      MET_LIST_FREE(*list);
      *list = MET_NULL_LIST;
   }

   return set;
}

#ifdef MET__DEBUG_PRINT

/*
 * print an ascii description of set to stream
 */
void
METset_print(stream, set, indent_level)
   FILE		*stream;
   METset	*set;
   int		indent_level;
{
   int		i;
   MME(METset_print);
   CHECK_NULL_SET(set);

   if (FNULL == stream)
      stream = stdout;

   MET_indent(stream, indent_level);
   if (MET_NULL_SET == set) {
      PRINT(stream, "set is null\n");
      return;
   }
   PRINT(stream, "set at %#x: ref_count = %d,",
	 set, set->mminfo.ref_count);
   
   if (MET_NULL_SYM == set->definition)
      PRINT(stream, " no definition,");
   if (MET_NULL_LIST == set->dep_list)
      PRINT(stream, " no dependencies,");

   PRINT(stream, " type = %s,", MET_names_set_type[set->type]);
   
   PRINT(stream, "\n");
   
   if (MET_NULL_SYM != set->definition) {
      MET_indent(stream, indent_level + 1);
      PRINT(stream, "definition:\n");
      METsym_print(stream, set->definition, indent_level + 2);
   }
   
   if (MET_NULL_LIST != set->dep_list) {
      MET_indent(stream, indent_level + 1);
      PRINT(stream, "dependencies:\n");
      METlist_print(stream, set->dep_list, indent_level + 2);
   }

   MET_indent(stream, indent_level + 1);

   switch (set->type) {
    case MET_SET_TYPE_UNKNOWN:
      break;

    case MET_SET_TYPE_BARE:
      PRINT(stream, "bare rep how_many = %d, tags =",
	    set->info.bare.how_many);
      for (i = 0; i < set->info.bare.tags_bare.num_tags; i++)
	 PRINT(stream, " (%d %d)",
	       set->info.bare.tags_bare.tags[i].id,
	       set->info.bare.tags_bare.tags[i].scalar);
      PRINT(stream, "\n");

      MET_indent(stream, indent_level + 1);
      PRINT(stream, "bare rep type:\n");
      METtype_print(stream, set->info.bare.type, indent_level + 2);

      MET_indent(stream, indent_level + 1);
      PRINT(stream, "bare rep data:\n");
      METdata_print(stream, set->info.bare.data, indent_level + 2);

      MET_indent(stream, indent_level + 1);
      break;

    case MET_SET_TYPE_BLOCK:
      PRINT(stream, "block rep block_list:\n");
      METblock_print(stream, set->info.block.block_list, indent_level + 2);
      break;

    case MET_SET_TYPE_SYMBOL:
      PRINT(stream, "symbol rep symbol: %#x, %s\n",
	    (int) set->info.symbol.sym, SYM_STRING(set->info.symbol.sym));
      break;

    case MET_SET_TYPE_SYM_CONST:
      PRINT(stream, "symbolic constant rep symbol: %#x, %s\n",
	    (int) set->info.sym_const.sym, SYM_STRING(set->info.sym_const.sym));
      break;

    case MET_SET_TYPE_TYPE:
      PRINT(stream, "type rep:\n");
      METtype_print(stream, set->info.type.type, indent_level + 2);
      break;

    default:
      DEFAULT_ERR(set->type);
   }
}

Boolean
MET__set_has_ref(set, mm_type, object)
   METset	*set;
   int		mm_type;
   MET_object	*object;
{
   MME(MET__set_has_ref);

   switch(mm_type) {
    case MET__MM_TYPE_SET:
      break;

    case MET__MM_TYPE_TYPE:
      if (MET_SET_TYPE_BARE == set->type &&
	  set->info.bare.type == (METtype *) object)
	 return YES;
      break;

    case MET__MM_TYPE_DATA:
      if (MET_SET_TYPE_BARE == set->type &&
	  set->info.bare.data == (METdata *) object)
	 return YES;
      break;

    case MET__MM_TYPE_VERTS:
      break;

    case MET__MM_TYPE_SYM:
      if (set->definition == (METsym *) object)
	 return YES;
      break;
	  
    case MET__MM_TYPE_BLOCK:
      if (MET_SET_TYPE_BLOCK == set->type &&
	  set->info.block.block_list == (METblock *) object)
	 return YES;
      break;

    case MET__MM_TYPE_CONNEX:
      break;

    case MET__MM_TYPE_LIST:
      if (set->dep_list == (METlist *) object)
	 return YES;
      break;
      
    default:
      DEFAULT_ERR(mm_type);
   }
   return NO;
}

#endif MET__DEBUG_PRINT

