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
   "$Id: op_apply.c,v 1.22 1992/06/22 17:05:48 mas Exp $";
#endif

#include "defs.h"

#define APPLY_NUM_KIDS  2

#define APPLY 		1
#define APPLY_SET	2
#define MAP		3
#define DOUBLE_MAP	4
#define O_P_MAP		5
#define MULTIPLY	6
#define COMPOSE		7
#define DIE		8

/* forward */
static METset *MET__op_apply_directly();

/* used to generate unique tag ids */
static int
MET__op_apply_get_id()
{
   static int id = MET_TAG_NO_ID + 1;
   MME(MET__op_apply_get_id);
   return id++;
}

/*
 * remove the tag ID from SET.  If this was the last
 * tag, then we are left with a block, otherwise we're
 * left with another polybare set.  As long as INTO is
 * of the same type as our result, we can add the components
 * of the decrossed set into it.  Otherwise, it is an
 * error.  We assume that INTO is ours to munge.
 */
static METset *
MET__op_apply_decross(set, into, connex, id)
   METset	*set, *into;
   METconnex	*connex;
   int		id;
{
   METset	*new, *stripped;
   int		i;
   MME(MET__op_apply_decross);
   CHECK_SET(set);
   CHECK_CONNEX(connex);

   /* Could be that the tag was lost because it was
      scalar: so we never built up the bogus polybare
      level.  So just use what we have. */

   if (set->type != MET_SET_TYPE_BARE) {

      MET_SET_COPY(stripped, set);

   } else {

      /* right now we assume that if we get a bare object
	 we have to find the tag on it.  This may be 
	 incorrect.  */
      
      for (i = 0; i < set->info.bare.tags_bare.num_tags; i++)
	 if (set->info.bare.tags_bare.tags[i].id == id)
	    break;
      
      if (i == set->info.bare.tags_bare.num_tags) {
	 for (i = 0; i < set->info.bare.tags_in.num_tags; i++)
	    if (set->info.bare.tags_in.tags[i].id == id)
	       break;

	 if (i == set->info.bare.tags_in.num_tags)
	    return MET_NULL_SET;

	 if (set->info.bare.tags_bare.num_tags == 0)
	    stripped = MET_set_remove_tag(set, id);

	 else 
	    MET_SET_COPY(stripped, set);
      } 
      else
      	 stripped = MET_set_remove_tag(set, id);
      
   }

   if (MET_NULL_SET == into)
      return stripped;

   if (stripped->type != into->type) {
      MET_SET_FREE(stripped);
      return MET_NULL_SET;
   }

   if (MET_SET_TYPE_BLOCK == stripped->type) {
      MET_set_add_block(into, stripped->info.block.block_list);
      MET_SET_COPY(new, into);
   } else {
      new = MET_set_polybare_merge(into, stripped);
   }

   MET_SET_FREE(stripped);
   return new;
}


/*
 * go through a sym dag and replace all parm variables (that is,
 * those with id SYM_VARIABLE_ID_PARM) with set-refs to the passed set.
 */
static void
MET__op_apply_replace_parms_with_set_refs(sym, set)
   METsym	*sym;
   METset	*set;
{
   int		i;
   MME(MET__op_apply_replace_parms_with_set_refs);
   CHECK_SYM(sym);
   CHECK_SET(set);

   for (i = 0; i < sym->num_kids; i++)
      MET__op_apply_replace_parms_with_set_refs(sym->kids[i], set);
   
   if (sym->type == MET_SYM_TYPE_VARIABLE &&
       sym->info.variable.id == SYM_VARIABLE_ID_PARM)
      MET_sym_force_set_ref(sym, set);
}

/*
 * the objects applied to each other are not functions, so multiply them
 */
static METset *
MET__op_apply_multiply(left, right, result_type, set_type, sym)
   METset	*left, *right;
   METtype	**result_type;
   int		*set_type;
   METsym	*sym;
{
   METset	*result_set;
   METsym	*sym2, *sym3;
   METtype	*type;
   MME(MET__op_apply_multiply);

   MET_TYPE_NEW_UNKNOWN(type);
   sym2 = METsym_new_op(MET_SYM_OP_MULTIPLY, sym->origin);
   sym3 = METsym_new_set_ref(left, RNULL);
   METsym_add_kid(sym2, sym3);
   MET_SYM_FREE(sym3);
   sym3 = METsym_new_set_ref(right, RNULL);
   METsym_add_kid(sym2, sym3);
   MET_SYM_FREE(sym3);

   result_set = MET_sym_be_set_ref(sym2, type, result_type, set_type);
   MET_TYPE_FREE(type);
   MET_SYM_FREE(sym2);

   return result_set;
}

/*
 * the object on the left is a set of functions.  apply each function in
 * the set, and collect the results.  this is exactly analagous to applying
 * a function to a set of objects
 */
static METset *
MET__op_apply_set_of_fns(left, right, range, result_type, set_type,
			 apply_op, sym)
   METset	*left, *right;
   METtype	*range;
   METtype	**result_type;
   int		*set_type;
   int		apply_op;
   METsym	*sym;
{
   METset	*result_set, *args[2], *bare_set;
   METset	*new_set;
   METblock	*scan_block;
   Boolean	split[2];
   int		id;
   int		tag_type, op_code;
   MME(MET__op_apply_set_of_fns);

   scan_block = left->info.block.block_list;
   result_set = MET_NULL_SET;
   
   /* default behavior here is really tricky.  watch carefully */
   if (MET_SYM_OP_DOUBLE_MAP == sym->info.op.code) {
      tag_type = MET_TAG_TYPE_DOUBLE_MAP;
      op_code = MET_SYM_OP_DOUBLE_MAP;
   } else {
      tag_type = MET_TAG_TYPE_PRODUCT_MAP;
      op_code = MET_SYM_OP_OUT_PROD_MAP;
   }
    
   while (MET_NULL_BLOCK != scan_block) {
      if (0 == scan_block->tags_in.num_tags) {
	 /*
	  * this is the case were we have a collection of
	  * functions with a different dag for each of them
	  * so we do them serially with polybare_to_bare. If
	  * necessary, we strip off a level when we're done.
	  */
	 id = MET__op_apply_get_id();
	 bare_set = MET_block_to_polybare(scan_block, id, tag_type,
					  DATA_NUM_VERTS(scan_block->data));
	 
	 MET_SET_COPY(args[0], bare_set);
	 MET_SET_COPY(args[1], right);
	 split[0] = YES;
	 split[1] = NO;
	 /* should use range type somehow */ /* Ack! */
	 new_set = MET_op_polybare_to_parts(args, split, 2, 
					    MET_op_apply_infer,
					    op_code, MET_NULL_TYPE);
	 MET_SET_FREE(args[0]);
	 MET_SET_FREE(args[1]);

      } else {
	 /*
	  * Here, there is just one dag, and it has polybare constants
	  * somewhere in it.  So we evaluate just once as if it were
	  * a normal function, but remember to take off an extra tag.
	  */
	 id = MET_TAG_NO_ID;
	 bare_set = 
	    MET_block_to_polybare(scan_block, id, tag_type,
				  DATA_NUM_VERTS(scan_block->data));
	 
	 switch(apply_op) {
	  case DOUBLE_MAP:
	  case O_P_MAP:
	    new_set = MET_op_apply_to_set(bare_set, right, range,
					  result_type, set_type, 
					  tag_type, sym);
	    break;
	  case APPLY_SET:
	    new_set = MET__op_apply_directly(bare_set, right, range,
					     result_type, set_type, sym);
	    break;
	  default:
	    DEFAULT_ERR(apply_op);
	 }

	 if (result_type != (METtype **) NULL &&
	     *result_type != MET_NULL_TYPE)
	    MET_TYPE_FREE(*result_type);

	 if (MET_SET_TYPE_BARE == new_set->type &&
	     0 != new_set->info.bare.tags_bare.num_tags)
	    id = new_set->info.bare.tags_bare.tags[0].id;
      }

      MET_SET_FREE(bare_set);

      if (MET_NULL_SET == new_set) {
	 MET_SET_FREE_NULL(result_set);
	 result_set = MET_NULL_SET;
	 break;
      }

      if (MET_TAG_NO_ID == id) {
	 /* this isn't really right! */
	 MET_SET_COPY(bare_set, new_set);
      } else {
	 bare_set = MET__op_apply_decross(new_set, result_set,
					  scan_block->connex, id);
      }

      MET_SET_FREE(new_set);
      if (MET_NULL_SET == bare_set) {
	 ERROR(sym->origin,
	       "weird error in apply set of functions",
	       SEV_FATAL);
	 MET_SET_FREE(bare_set);
	 MET_SET_FREE(result_set);
	 result_set = NULL;
	 break;
      }

      MET_SET_FREE_NULL(result_set);
      MET_SET_COPY(result_set, bare_set);
      MET_SET_FREE(bare_set);
      scan_block = scan_block->next;
   }
   MET_set_get_type(result_set, result_type, set_type);

   return result_set;
}

/*
 * This complicated function is required to correctly account
 * for a whole bunch of things.  It must deal correctly with
 * symbolic manipulations, which may not leave enough tags
 * on their results.  It must deal correctly with constant 
 * functions, which have the same deficiency.  When the 
 * cardinality of a tag is 1, we may end up with a block
 * set where we don't want one.  As it is, this function
 * will still fail for the extremely unlikely situation 
 * (maybe impossible) where we have bare tags that should
 * be added to a polybarized block set and in tags that 
 * should be added to the block set itself.  Ack.
 */

static METset *
MET__op_apply_restore_tags(tagged_set, my_type, left, right)
   METset	*tagged_set;
   METtype	*my_type;
   METset	*left, *right;
{
   METset	*tag_me;
   METset	*temp;
   METblock	*block;
   METtag_set	tags;
   METtag_set	*tag_me_tags_bare, *tag_me_tags_in;
   int		i;
   int		mapping_tags;
   MME(MET__op_apply_restore_tags);

   MET_SET_COPY(tag_me, tagged_set);

   /* Find out what tags survived the application process. */
   switch(tag_me->type) {
    case MET_SET_TYPE_BLOCK:
      if (MET_NULL_BLOCK == (block = tag_me->info.block.block_list)) {
	 tag_me_tags_bare = (METtag_set *) NULL;
	 tag_me_tags_in = (METtag_set *) NULL;
      } else {
	 tag_me_tags_bare = &block->tags_part;
	 tag_me_tags_in = &block->tags_in;
      }
      break;

    case MET_SET_TYPE_BARE:
      tag_me_tags_bare = &tag_me->info.bare.tags_bare;
      tag_me_tags_in = &tag_me->info.bare.tags_in;
      break;

    default:
      tag_me_tags_bare = (METtag_set *) NULL;
      tag_me_tags_in = (METtag_set *) NULL;
      break;
   }

   /* Create a list of all of the tags we haven't accounted for. */
   MET_tags_initialize(&tags);
   if ((METtag_set *) NULL == tag_me_tags_bare) {
      MET_tags_copy(&tags, &left->info.bare.tags_in);
      switch (right->type) {
       case MET_SET_TYPE_BARE:
	 MET_tags_copy(&tags, &right->info.bare.tags_bare);
	 MET_tags_copy(&tags, &right->info.bare.tags_in);
	 break;
       case MET_SET_TYPE_BLOCK:
	 if (MET_NULL_BLOCK != (block = right->info.block.block_list)) {
	    MET_tags_copy(&tags, &block->tags_part);
#if 0
	    /* Ack!  We suspect strongly that the internal tags were
	       eliminated due to successful application.  But I wish
	       I could PROVE that this is right. */
	       MET_tags_copy(&tags, &block->tags_in);
#endif
	 }
	 break;
       default:
	 break;
      }
   } else {
      MET_tags_copy_new(&tags, &left->info.bare.tags_in, 
			tag_me_tags_bare);
      switch (right->type) {
       case MET_SET_TYPE_BARE:
	 MET_tags_copy_new(&tags, &right->info.bare.tags_bare,
			   tag_me_tags_bare);
	 MET_tags_copy_new(&tags, &right->info.bare.tags_in,
			   tag_me_tags_bare);
	 break;
       case MET_SET_TYPE_BLOCK:
	 if (MET_NULL_BLOCK != (block = right->info.block.block_list)) {
	    MET_tags_copy_new(&tags, &block->tags_part, tag_me_tags_bare);
#if 0
	    /* Once again we'll make the semi-educated guess that
	       we don't have these cuze we don't want them. */
	    MET_tags_copy_new(&tags, &block->tags_in, tag_me_tags_bare);
#endif
	 }
	 break;
       default:
	 break;
      }
   }

   /* Add in these tags in the appropriate way. */
   switch (tag_me->type) {
    
    case MET_SET_TYPE_BLOCK:
      /* Two things must be true if we're to leave a block set
	 as a block set.  There must have been no plural tags on
	 the original function, and either there are no other tags 
	 to deal with, or the block set can absorb those tags
	 itself. */

      if (0 != left->info.bare.tags_bare.num_tags) {
	 mapping_tags = NO;
	 for (i = 0; i < left->info.bare.tags_bare.num_tags; i++)
	    if (MET_TAG_TYPE_ELEMENT !=
		left->info.bare.tags_bare.tags[i].type)
	       mapping_tags = YES;
      } else 
	 mapping_tags = NO;
      
      if (YES == mapping_tags)
	 goto polybarize;

      if (0 != tags.num_tags) {
	 for (i = 0; i < tags.num_tags; i++)
	    if (MET_TAG_TYPE_ELEMENT != tags.tags[i].type)
	       mapping_tags = YES;
      }

      if (YES == mapping_tags && 
	  MET_FIELD_MAP != 
	  MET_type_base_field(my_type->info.set_of.of, NO))
	 goto polybarize;

      for (block = tag_me->info.block.block_list;
	   block != MET_NULL_BLOCK;  block = block->next) {
	 MET_tags_copy(&block->tags_part, 
		       &left->info.bare.tags_bare);
	 for (i = 0; i < tags.num_tags; i++)
	    if (MET_TAG_TYPE_ELEMENT == tags.tags[i].type)
	       MET_tags_add(&block->tags_part, tags.tags + i,
			    tags.tags[i].scalar);
	    else
	       MET_tags_add(&block->tags_in, tags.tags + i,
			    tags.tags[i].scalar);
      }

      MET_tags_free(&tags);
      return tag_me;

    polybarize:
      /* If we have to add additional tags, we'll need to add an extra
	 layer onto our set. */
      temp = MET_set_de_canonize(tag_me);
      MET_SET_FREE(tag_me);
      MET_SET_COPY(tag_me, temp);
      MET_SET_FREE(temp);

      /* Fall through */

    case MET_SET_TYPE_BARE:
      MET_tags_copy(&tag_me->info.bare.tags_bare,
		    &left->info.bare.tags_bare);
      
      if (MET_FIELD_MAP ==
	  MET_type_base_field(my_type, NO)) {
	 MET_tags_copy(&tag_me->info.bare.tags_in, &tags);
      } else {
	 for (i = 0; i < tags.num_tags; i++)
	    tags.tags[i].scalar = YES;
	 MET_tags_copy(&tag_me->info.bare.tags_bare, &tags);
      }
      break;

    default:
      break;
   }

   MET_tags_free(&tags);
   return tag_me;
}

static METset *
MET__op_apply_directly(left, right, range_type, result_type, set_type,sym)
   METset	*left, *right;
   METtype	*range_type, **result_type;
   int		*set_type;
   METsym	*sym;
{
   METsym	*deep_sym;
   METset	*result_set, *partial_result, *tagged_result;
   METdata	*data;
   METmap	*map;
   METtype	*my_result_type;
   METtag_set	tags;
   METblock	*block;
   MME(MET__op_apply_directly);
   CHECK_SET(left);
   CHECK_SET(right);
   CHECK_TYPE(range_type);

   /* Gotta be something more efficient */
   if (DATA_NUM_VERTS(left->info.bare.data) > 1) {
      Boolean 	split[2];
      METset	*args[2];

      split[0] = YES;
      split[1] = NO;
      MET_SET_COPY(args[0], left);
      MET_SET_COPY(args[1], right);
      result_set =
	 MET_op_polybare_to_parts(args, split, 2,
				  MET_op_apply_infer, sym->info.op.code,
				  left->info.bare.type->info.map.to);
      MET_SET_FREE(args[0]);
      MET_SET_FREE(args[1]);

      MET_set_get_type(result_set, result_type, set_type);
      return result_set;
   }

   data = left->info.bare.data;
   map = DATA_FIRST_VERTEX(data, METmap);
   MET_SYM_COPY_DEEP(deep_sym, map->sym);
   MET__op_apply_replace_parms_with_set_refs(deep_sym, right);
   partial_result = MET_sym_be_set_ref(deep_sym, range_type,
				       &my_result_type, set_type);
   MET_SYM_FREE(deep_sym);
   
   if (MET_NULL_SET == partial_result) 
      return MET_NULL_SET;
   
   MET_SET_COPY_DEEP(tagged_result, partial_result);
   MET_SET_FREE(partial_result);
   
   /* Tags will not have been done correctly if the
      return type is a functional one, or if we've
      applied a constant function.... */
   partial_result = MET__op_apply_restore_tags(tagged_result,
					       my_result_type, 
					       left, right);
      
   MET_SET_FREE(tagged_result);
   result_set = MET_set_cast_to_type(partial_result, range_type);

   if (MET_NULL_SET == result_set) {
      MET_op_cant_cast(sym, my_result_type, range_type,
		       " (result doesn't match codomain)");
      result_set = MET_NULL_SET;
   }
   
   if ((METtype **) NULL != result_type) {
      if (result_set != tagged_result) {
	 MET_set_get_type(result_set, result_type, set_type);
      } else {
	 MET_TYPE_COPY(*result_type, my_result_type);
      }
   }   
   
   MET_SET_FREE(partial_result);
   MET_TYPE_FREE(my_result_type);
   
   return result_set;
}

/*
 * apply a function (set really) to a set, and return the result.  if it could
 * not be done, return NULL.
 *
 * If the function apearing in left is of type A -> B, and right is block,
 * and all of the blocks are of type A, then apply the map without
 * converting blocks to polybare sets.  otherwise use the below algorithm.
 *
 * Algorithm: If right is bare, just call MET__op_apply_directly().  If
 * right is a block set, for each of its blocks create a polybare set by
 * shallow copying the block's data and the block's type below the set_of type
 * node.  Call MET_set_apply_to_bare_set().  Now, convert the polybare result
 * back to a block by shallow copying the original connex and the data of the
 * polybare just created.  The type should be set_of, and a shallow copy of
 * the type of the polybare just created.  Free the polybare.
 */
METset *
MET_op_apply_to_set(left, right, range_type, result_type, 
		    set_type, tag_type, sym)
   METset	*left;
   METset	*right;
   METtype	*range_type;
   METtype	**result_type;
   int		*set_type;
   int		tag_type;
   METsym	*sym;
{
   METlist	*list, *save_list;
   METset	*uncrossed, *new_set, *result_set;
   METset	*args[2];
   Boolean	split[2];
   METtype	*domain_type, *codomain_type;
   int		my_id;
   MME(MET_op_apply_to_set);
   CHECK_SET(left);
   CHECK_SET(right);
   CHECK_NULL_PTR(result_type);
   CHECK_NULL_PTR(set_type);
   
   /*
    * these will probably get reset later on
    */
   if ((METtype **) NULL != result_type)
      *result_type = MET_NULL_TYPE;
   
   if (INULL != set_type)
      *set_type = MET_SET_TYPE_UNKNOWN;
   
   if (MET_SET_TYPE_BARE != left->type) {
      ERROR(sym->origin,
	    "left side of apply is not a valid function (is not bare)",
	    SEV_FATAL);
      return MET_NULL_SET;
   }

   if (MET_SET_TYPE_BLOCK != right->type) {
      ERROR(sym->origin,
	    "set argument in map is not block", SEV_FATAL);
      return MET_NULL_SET;
   }

   if (DATA_NUM_VERTS(left->info.bare.data) > 1) {
      split[0] = YES;
      split[1] = NO;
      MET_SET_COPY(args[0], left);
      MET_SET_COPY(args[1], right);
      result_set =
	 MET_op_polybare_to_parts(args, split, 2,
				  MET_op_apply_infer, sym->info.op.code,
				  left->info.bare.type->info.map.to);
      MET_SET_FREE(args[0]);
      MET_SET_FREE(args[1]);
      
      return result_set;
   }
   
#ifdef MET__DEBUG
   if (MET_SET_TYPE_UNKNOWN == right->type) {
      MET_TYPE_FREE(domain_type);
      MET_TYPE_FREE(codomain_type);
      ERROR(sym->origin, "cannot apply to yukky set", SEV_FATAL);
      
      return MET_NULL_SET;
   }
#endif
   
   /*
    * We'll have to special case the NULL set:  any function mapped
    * over {} will give back {}.
    */

   if (MET_NULL_BLOCK == right->info.block.block_list) {
      MET_SET_COPY(result_set, MET_set_null);
      MET_set_get_type(result_set, result_type, set_type);
      return result_set;
   }

   /*
    * now we know that the function should be applied to each element of
    * right.   So convert right's blocks into polybare sets, apply to
    * each of them, and convert them back to blocks.
    *
    * we also generate a new unique apply id, and mark the polybares with
    * it.
    */

   MET_TYPE_COPY(domain_type, left->info.bare.type->info.map.from);
   MET_TYPE_COPY(codomain_type, left->info.bare.type->info.map.to);
   
   my_id = MET__op_apply_get_id();
   list = MET_set_block_to_polybare(right, my_id, tag_type);
   
   result_set = MET_NULL_SET;
   
   save_list = list;
   while (list != MET_NULL_LIST) {
      new_set = MET__op_apply_directly(left, list->object.set, codomain_type,
				       (METtype **) NULL, INULL, sym);
      if (MET_NULL_SET == new_set) {
	 MET_SET_FREE_NULL(result_set);
	 result_set = MET_NULL_SET;
	 break;
      }
      uncrossed = MET__op_apply_decross(new_set, result_set,
					list->aux_object.connex,
					my_id);
      MET_SET_FREE(new_set);
      if (MET_NULL_SET == uncrossed) {
	 ERROR(sym->origin,
	       "weird error in apply set of functions",
	       SEV_FATAL);
	 MET_SET_FREE_NULL(result_set);
	 result_set = NULL;
	 break;
      }
      
      MET_SET_FREE_NULL(result_set);
      MET_SET_COPY(result_set, uncrossed);
      MET_SET_FREE(uncrossed);
      list = list->next;
   }
   MET_LIST_FREE_NULL(save_list);
   MET_set_get_type(result_set, result_type, set_type);
   
   MET_TYPE_FREE(domain_type);
   MET_TYPE_FREE(codomain_type);
   
   return result_set;
}
/*
 * Decide what the general apply should do, based on the types of the
 * arguments.
 */
static int
MET__op_apply_choose_op(left, right, code, range)
   METtype 	*left, *right;
   int		code;
   METtype	**range;
{
   Boolean	sets;
   METtype	*map, *rmap;
   ME(MET__op_apply_choose_op);
   
   switch (left->type) {
    case MET_TYPE_TYPE_SET:
    case MET_TYPE_TYPE_UNKNOWN:
    case MET_TYPE_TYPE_VECTOR:
    case MET_TYPE_TYPE_FIELD:
    case MET_TYPE_TYPE_PAIR:
      *range = MET_NULL_TYPE;
      return MULTIPLY;
      
    case MET_TYPE_TYPE_SET_OF:
      if (MET_TYPE_TYPE_MAP == left->info.set_of.of->type) {
	 map = left->info.set_of.of;
	 sets = YES;
	 break;
      } else {
	 *range = MET_NULL_TYPE;
	 return MULTIPLY;
      }

    case MET_TYPE_TYPE_MAP:
      sets = NO;
      map = left;
      break;
      
    default:
      DEFAULT_ERR(left->type);
   }
   
   if (YES == MET_type_compatible(right, map->info.map.from, YES)) 
      if (YES == sets) {
	 MET_TYPE_COPY(*range, map->info.map.to);
         return APPLY_SET;
      } else {
	 MET_TYPE_COPY(*range, map->info.map.to);
	 return APPLY;
      }
   if (MET_TYPE_TYPE_SET_OF == right->type &&
       YES == MET_type_compatible(right->info.set_of.of,
				  map->info.map.from, YES)) 
      if (YES == sets) {
	 MET_TYPE_NEW_SET_OF(*range, map->info.map.to);
         return (MET_SYM_OP_DOUBLE_MAP == code) ? DOUBLE_MAP : O_P_MAP;
      } else {
	 MET_TYPE_NEW_SET_OF(*range, map->info.map.to);
	 return MAP;
      }
   
   *range = MET_NULL_TYPE;
   if (MET_TYPE_TYPE_SET_OF == right->type)
      rmap = right->info.set_of.of;
   else
      rmap = right;
   
   if (MET_TYPE_TYPE_MAP == rmap->type) 
      if (YES == MET_type_compatible(map->info.map.from, 
				     rmap->info.map.to, YES))
	 return COMPOSE;
      else 
	 return MULTIPLY;
   
   return DIE;
}

METset *
MET_op_apply_infer(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   int		operation;
   int		tag_type;
   METset	*result_set, *arg_set[2];
   METtype	*new_guess, *unknown, *arg_type[2], *range;

   static int call_level = 0;
   int  t = call_level;
   char *name = CNULL;

   MME(MET_op_apply_infer);

#ifdef MET__DEBUG
   if (sym->num_kids != 2)
      FATALS("apply sym doesn't have 2 kids");
#endif

   MET_TYPE_NEW_UNKNOWN(unknown);
   if (MET_NULL_TYPE == type) {
      MET_TYPE_NEW_MAP(new_guess, unknown, unknown);
   } else {
      MET_TYPE_NEW_MAP(new_guess, unknown, type);
   }

   if (YES == MET_defaults->trace.on) 
      if (MET_SYM_TYPE_SET_REF == sym->kids[0]->type &&
	  MET_SET_TYPE_SYMBOL == sym->kids[0]->info.set_ref.set->type) {
	 MET_indent(stdout, t);
	 call_level++;
	 name = SYM_STRING(sym->kids[0]->info.set_ref.set->info.symbol.sym);
	 fprintf(stdout, "%s arg", name);
	 fflush(stdout);
      } 

   MET_TYPE_FREE(unknown);
   arg_set[0] = MET_sym_be_set_ref(sym->kids[0], new_guess,
				   &arg_type[0], INULL);
   MET_TYPE_FREE(new_guess);
   if (MET_NULL_SET == arg_set[0])
      goto bail;

   /* Special case: applying the NULL set (of functions)
      to anything returns the NULL set. */
   if (MET_SET_TYPE_BLOCK == arg_set[0]->type &&
       MET_NULL_BLOCK == arg_set[0]->info.block.block_list) {
      MET_SET_COPY(result_set, arg_set[0]);
      MET_SET_FREE(arg_set[0]);
      MET_set_get_type(result_set, result_type, set_type);
      goto win;
   }

   arg_set[1] = MET_sym_be_set_ref(sym->kids[1],  /* Ack! */
				   MET_NULL_TYPE,
				   &arg_type[1],
				   INULL);

   if (CNULL != name) {
      char print_buf[8192];

      print_buf[0] = 0;
      METset_print_pretty(FNULL, arg_set[1], print_buf, 0);
      fprintf(stdout, "is %s\n", name, print_buf);
   }

   if (MET_NULL_SET == arg_set[1]) {
      MET_SET_FREE(arg_set[0]);
      MET_TYPE_FREE(arg_type[1]);
      goto bail;
   }

   if (MET_SET_TYPE_TYPE == arg_set[1]->type) {
      MET_TYPE_FREE(arg_type[1]);
      arg_type[1] = MET_type_of_type(arg_set[1]->info.type.type);
   }

   operation = MET__op_apply_choose_op(arg_type[0], arg_type[1], 
				       sym->info.op.code, &range);

   switch (operation) {
    case APPLY:
      result_set = MET__op_apply_directly(arg_set[0], arg_set[1], range,
					  result_type, set_type, sym);
      break;
    case APPLY_SET:
      result_set = MET__op_apply_set_of_fns(arg_set[0], arg_set[1], range,
					    result_type, set_type,
					    APPLY_SET, sym);
      break;
    case MAP:
      tag_type = (MET_SYM_OP_OUT_PROD_MAP == sym->info.op.code) ?
	 MET_TAG_TYPE_PRODUCT_MAP : MET_TAG_TYPE_DOUBLE_MAP;
      if (MET_op_block_only(arg_set, APPLY_NUM_KIDS, MET_op_apply_infer,
			    sym->info.op.code, range)) {
	 if (MET_NULL_SET != arg_set[0]) {
	    MET_SET_COPY(result_set, arg_set[0]);
	 } else 
	    result_set = MET_NULL_SET;
	 MET_set_get_type(result_set, result_type, set_type);
      } else {
	 result_set = MET_op_apply_to_set(arg_set[0], arg_set[1], range,
					  result_type, set_type, 
					  tag_type, sym);
      }
      break;
    case DOUBLE_MAP:
    case O_P_MAP:
      if (MET_op_block_only(arg_set, APPLY_NUM_KIDS, MET_op_apply_infer,
			    sym->info.op.code, range)) {
	 if (MET_NULL_SET != arg_set[0]) {
	    MET_SET_COPY(result_set, arg_set[0]);
	 } else 
	    result_set = MET_NULL_SET;
	 MET_set_get_type(result_set, result_type, set_type);
      } else {
	 result_set = MET__op_apply_set_of_fns(arg_set[0], arg_set[1], range,
					       result_type, set_type,
					       operation, sym);
      }
      break;
    case MULTIPLY:
    case COMPOSE:
      if (MET_SYM_OP_APPLY == sym->info.op.code) {
	 result_set = MET__op_apply_multiply(arg_set[0], arg_set[1], 
					     result_type, set_type, sym);
	 break;
      }
      /* fall through */
    case DIE:
      ERROR(sym->origin, "apply: argument and function types don't match",
	    SEV_FATAL);
      result_set = MET_NULL_SET;
      break;
    default:
      DEFAULT_ERR(operation);
   }

   MET_TYPE_FREE_NULL(range);
   MET_TYPE_FREE_NULL(arg_type[0]);
   MET_TYPE_FREE_NULL(arg_type[1]);
   MET_SET_FREE(arg_set[0]);
   MET_SET_FREE(arg_set[1]);

 win:
 
   if (CNULL != name && MET_NULL_SET != result_set) {
      char print_buf[8192];

      print_buf[0] = 0;
      MET_indent(stdout, t);
      METset_print_pretty(FNULL, result_set, print_buf, 0);
      fprintf(stdout, "%s result is %s\n", name, print_buf);
      call_level--;
   }
   
   if (MET_NULL_SET != result_set)
      return result_set;
   
 bail:

   if (CNULL != name) {
      fprintf(stdout, "\n");
      MET_indent(stdout, t);
      fprintf(stdout, "%s failed\n", name);
      call_level--;
   }

   MET_set_get_type(MET_NULL_SET, result_type, set_type);
   return MET_NULL_SET;
}

/*
 * apply an op to each of the elements of the passed arg.
 * if no arg is block, null is returned. 
 */
METset *
MET_op_apply_to_elements(arg, nargs, op_func, op_code, type_guess, sym)
   METset	**arg;
   int		nargs;
   METset	*(*op_func)();
   int		op_code;
   METtype	*type_guess;
   METsym	*sym;
{
   METlist	*list[MET__MAX_SYM_KIDS], *save_list[MET__MAX_SYM_KIDS];
   METset	*temp, *new_set, *result_set;
   METtype	*guess;
   METsym	*tsym, *kid;
   int		id[MET__MAX_SYM_KIDS];
   int		i, j;
   MME(MET_op_apply_to_elements);

   for (j = 0; j < nargs; j++)
      if (MET_SET_TYPE_BLOCK == arg[j]->type) 
	 break;

   if (nargs == j)
      return MET_NULL_SET;

   if (MET_NULL_TYPE != type_guess &&
       MET_TYPE_TYPE_SET_OF == type_guess->type) {
      /* not for long.... */
      MET_TYPE_COPY(guess, type_guess->info.set_of.of);
   } else {
      MET_TYPE_NEW_UNKNOWN(guess);
   }

   for (j = 0; j < nargs; j++) {
      if (MET_SET_TYPE_BLOCK == arg[j]->type) {
	 id[j] = MET__op_apply_get_id();
	 list[j] = save_list[j] = 
	    MET_set_block_to_polybare(arg[j], id[j], MET_TAG_TYPE_PRODUCT_MAP);
      } else {
	 id[j] = -1;	/* is that bogus enough? */
	 list[j] = save_list[j] = MET_NULL_LIST;
      }
   }

   result_set = MET_NULL_SET;
   
   for (i = 0; MET_NULL_LIST == list[i]; i++);

   while (1) {   
      tsym = METsym_new_op(op_code, sym->origin);
      for (j = 0; j < nargs; j++) {
	 kid = METsym_new_set_ref((MET_NULL_LIST == list[j]) ? arg[j] :
				  list[j]->object.set, sym->origin);
	 METsym_add_kid(tsym, kid);
	 MET_SYM_FREE(kid);
      }
      new_set = (op_func)(tsym, guess, (METtype **) NULL, INULL);
      MET_SYM_FREE(tsym);
      if (MET_NULL_SET == new_set) {
	 MET_SET_FREE_NULL(result_set);
	 result_set = MET_NULL_SET;
	 break;
      }
      
      for (j = nargs - 1; j > -1; j-- ) 
	 if (id[j] != -1) {
	    temp = MET__op_apply_decross(new_set, MET_NULL_SET,
					 list[j]->aux_object.connex,
					 id[j]); 
	    MET_SET_FREE(new_set);
	    MET_SET_COPY(new_set, temp);
	    MET_SET_FREE(temp);
	 }
      
      if (MET_NULL_SET == result_set) {
	 MET_SET_COPY(result_set, new_set);
      } else if (result_set->type != new_set->type) {
	 ERROR(sym->origin,
	       "weird error in apply to elements",
	       SEV_FATAL);
	 MET_SET_FREE(new_set);
	 MET_SET_FREE(result_set);
	 result_set = NULL;
	 break;
      } else if (MET_SET_TYPE_BARE == result_set->type) {
	 temp = MET_set_polybare_merge(result_set, new_set);
	 MET_SET_FREE(result_set);
	 MET_SET_COPY(result_set, temp);
	 MET_SET_FREE(temp);
      } else {
	 MET_set_add_block(result_set, new_set->info.block.block_list);
      }

      MET_SET_FREE(new_set);
      
      list[i] = list[i]->next;
      while (list[i] == MET_NULL_LIST && i > 0) {
	 i--;
	 if (list[i] != MET_NULL_LIST)
	    list[i] = list[i]->next;
      }

      if (MET_NULL_LIST == list[i]) 
	 break;

      for (j = i + 1; j < nargs; j++)
	 list[j] = save_list[j];

      i = nargs - 1;
   }

   for (i = 0; i < nargs; i++)
      MET_LIST_FREE_NULL(save_list[i]);
   MET_TYPE_FREE(guess);

   return result_set;
}
