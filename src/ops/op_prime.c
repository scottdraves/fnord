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
   "$Id: op_prime.c,v 1.17 1992/06/22 17:06:54 mas Exp $";
#endif

#include "defs.h"

#define PRIME_NUM_KIDS	1
#define PRIME_ARG	0

#define MET__OP_PRIME_MAX_PATH	100

/*
 * stuff we need to know as we traverse a type to a sym.
 * the path is the most important part of this.  it stores how we got to
 * part of the tree we are in.  each time we take a branch of the type,
 * we add to the path.
 */
typedef struct MET__op_prime_info MET__op_prime_info;

struct MET__op_prime_info {
   METtype	*root;		/* the root of the type tree for the domain */
				/* of the function */
   METsym	*sym;		/* the function itself */
   METsym	*(*handler)();	/* call this when we reach a leaf of the */
				/* type tree */
   int		path_len;
   int		path[MET__OP_PRIME_MAX_PATH];
   MET__op_prime_info *parent;
};

extern METsym *MET__op_prime_type_to_sym();
extern METsym *MET__op_prime_handler_code();
extern METsym *MET__op_prime_handler_partial();
extern METsym *MET__op_prime_partial();
extern METsym *MET__op_prime_product();
/*
 * generate code for the derivative of code.
 * f'(u) = f'(u)*u'.   CODE is f, SYM is u.
 * here we only create f'(u), the rest is done
 * at a higher level.
 */
static METsym *
MET__op_prime_chain(code, sym, origin)
   int		code;
   METsym	*sym;
   FILOCloc	*origin;
{
   METsym	*result = MET_NULL_SYM;
   METsym	*t1, *t2;
   MME(MET__op_prime_chain);

   switch (code) {

    case MET_SYM_OP_SIN:
      result = METsym_new_op(MET_SYM_OP_COS, origin);
      METsym_add_kid(result, sym);
      break;

    case MET_SYM_OP_COS:
      result = METsym_new_op(MET_SYM_OP_OPPOSITE, origin);
      t1 = METsym_new_op(MET_SYM_OP_SIN, origin);
      METsym_add_kid(t1, sym);
      METsym_add_kid(result, t1);
      MET_SYM_FREE(t1);
      break;

    case MET_SYM_OP_TAN:
      result = METsym_new_op(MET_SYM_OP_ADD, origin);
      t1 = METsym_new_set_ref(MET_set_one, origin);
      METsym_add_kid(result, t1);
      MET_SYM_FREE(t1);
      t1 = METsym_new_op(MET_SYM_OP_SQUARE, origin);
      METsym_add_kid(t1, sym);
      METsym_add_kid(result, t1);
      MET_SYM_FREE(t1);
      break;
      
    case MET_SYM_OP_ASIN:
    case MET_SYM_OP_ACOS:
    case MET_SYM_OP_ATAN:
      break;

    case MET_SYM_OP_SINH:
      result = METsym_new_op(MET_SYM_OP_COSH, origin);
      METsym_add_kid(result, sym);
      break;

    case MET_SYM_OP_COSH:
      result = METsym_new_op(MET_SYM_OP_SINH, origin);
      METsym_add_kid(result, sym);
      break;

    case MET_SYM_OP_TANH:
      result = METsym_new_op(MET_SYM_OP_SUBTRACT, origin);
      t1 = METsym_new_set_ref(MET_set_one, origin);
      METsym_add_kid(result, t1);
      MET_SYM_FREE(t1);
      t1 = METsym_new_op(MET_SYM_OP_SQUARE, origin);
      METsym_add_kid(t1, sym);
      METsym_add_kid(result, t1);
      MET_SYM_FREE(t1);
      break;
      
    case MET_SYM_OP_ASINH:
    case MET_SYM_OP_ACOSH:
    case MET_SYM_OP_ATANH:
      break;

    case MET_SYM_OP_LN:
      result = METsym_new_op(MET_SYM_OP_DIVIDE, origin);
      t1 = METsym_new_set_ref(MET_set_one, origin);
      METsym_add_kid(result, t1);
      MET_SYM_FREE(t1);
      METsym_add_kid(result, sym);
      break;

    case MET_SYM_OP_SQUARE:
      result = METsym_new_op(MET_SYM_OP_MULTIPLY, origin);
      t1 = METsym_new_set_ref(MET_set_two, origin);
      METsym_add_kid(result, t1);
      MET_SYM_FREE(t1);
      METsym_add_kid(result, sym);
      break;
      
    case MET_SYM_OP_SQRT:
      result = METsym_new_op(MET_SYM_OP_DIVIDE, origin);
      t1 = METsym_new_set_ref(MET_set_one, origin);
      METsym_add_kid(result, t1);
      MET_SYM_FREE(t1);
      t1 = METsym_new_op(MET_SYM_OP_MULTIPLY, origin);
      t2 = METsym_new_set_ref(MET_set_two, origin);
      METsym_add_kid(t1, t2);
      MET_SYM_FREE(t2);
      t2 = METsym_new_op(MET_SYM_OP_SQRT, origin);
      METsym_add_kid(t2, sym);
      METsym_add_kid(t1, t2);
      MET_SYM_FREE(t2);
      METsym_add_kid(result, t1);
      MET_SYM_FREE(t1);
      break;
      
    case MET_SYM_OP_EXP:
      result = METsym_new_op(MET_SYM_OP_EXP, origin);
      METsym_add_kid(result, sym);
      break;

    default:
      DEFAULT_ERR(code);
   }

   return result;
}

/*
 * deal with functions that don't fall into one of the broader categories
 */
static METsym *
MET__op_prime_special(sym, partial_alpha, id)
   METsym	*sym, *partial_alpha;
   int		id;
{
   METsym	 *result = MET_NULL_SYM;
   METsym	 *kid, *kid1, *kid2, *kid3, *kid4, *kid5;
   void		(*save_handler)();
   MME(MET__op_prime_special);

   switch (sym->info.op.code) {

    case MET_SYM_OP_ROW_EXTRACT:
    case MET_SYM_OP_COLUMN_EXTRACT:
    case MET_SYM_OP_IF:
      /* (f^g)' = (f')^g, ditto for _ and if */
      result = METsym_new_op(sym->info.op.code, sym->origin);
      kid = MET__op_prime_partial(sym->kids[0], partial_alpha, id);
      if (MET_NULL_SYM != kid) {
	 METsym_add_kid(result, kid);
	 MET_SYM_FREE(kid);
	 METsym_add_kid(result, sym->kids[1]);
	 break;
      }
      MET_SYM_FREE(result);
      result = MET_NULL_SYM;
      break;

    case MET_SYM_OP_DIVIDE:
      /* (f/g)' = (g f' - f g') / g**2 */
      kid2 = METsym_new_op(MET_SYM_OP_MULTIPLY, sym->origin);
      kid = MET__op_prime_partial(sym->kids[0], partial_alpha, id);
      if (MET_NULL_SYM == kid)
	 goto bail1;
      METsym_add_kid(kid2, kid);
      MET_SYM_FREE(kid);
      METsym_add_kid(kid2, sym->kids[1]);
      
      kid3 = METsym_new_op(MET_SYM_OP_MULTIPLY, sym->origin);
      METsym_add_kid(kid3, sym->kids[0]);
      kid = MET__op_prime_partial(sym->kids[1], partial_alpha, id);
      if (MET_NULL_SYM == kid)
	 goto bail2;
      METsym_add_kid(kid3, kid);
      MET_SYM_FREE(kid);
      
      kid4 = METsym_new_op(MET_SYM_OP_SUBTRACT, sym->origin);
      METsym_add_kid(kid4, kid2);
      METsym_add_kid(kid4, kid3);
      MET_SYM_FREE(kid2);
      MET_SYM_FREE(kid3);

      kid2 =  METsym_new_op(MET_SYM_OP_SQUARE, sym->origin);
      METsym_add_kid(kid2, sym->kids[1]);
      
      result = METsym_new_op(MET_SYM_OP_DIVIDE, sym->origin);
      METsym_add_kid(result, kid4);
      METsym_add_kid(result, kid2);
      MET_SYM_FREE(kid4);
      MET_SYM_FREE(kid2);
      break;

    bail2:
      MET_SYM_FREE(kid3);
    bail1:
      MET_SYM_FREE(kid2);
      result = MET_NULL_SYM;
      break;
      
    case MET_SYM_OP_ABS:
      break;

    case MET_SYM_OP_IF_ELSE:
      /* (f if g else h)'  = (f' if g else h') */
      result = METsym_new_op(sym->info.op.code, sym->origin);
      kid = MET__op_prime_partial(sym->kids[0], partial_alpha, id);
      if (MET_NULL_SYM == kid)
	 goto bail;
      METsym_add_kid(result, kid);
      MET_SYM_FREE(kid);
      METsym_add_kid(result, sym->kids[1]);
      kid = MET__op_prime_partial(sym->kids[2], partial_alpha, id);
      if (MET_NULL_SYM == kid)
	 goto bail;
      METsym_add_kid(result, kid);
      MET_SYM_FREE(kid);

      break;

    bail:
      MET_SYM_FREE(result);
      result = MET_NULL_SYM;
      break;

    case MET_SYM_OP_POW:
      /* (f ** g)' = (f ** (g - 1)) (g f' + f g' log f) */
      /* a problem with this is that (x -> x**3)'' -> (x -> 3*x**(3-1))'
	 then you end up with a log x term, which is NaN for x < 0.
	 It's coef is 0, but 0*NaN = NaN */
      kid1 = MET__op_prime_partial(sym->kids[0], partial_alpha, id);
      if (MET_NULL_SYM == kid1)
	 break;
      kid2 = MET__op_prime_partial(sym->kids[1], partial_alpha, id);
      if (MET_NULL_SYM == kid2)
	 goto bail3;

      kid = METsym_new_op(MET_SYM_OP_SUBTRACT, sym->origin);
      METsym_add_kid(kid, sym->kids[1]);
      kid3 = METsym_new_set_ref(MET_set_one, sym->origin);
      METsym_add_kid(kid, kid3);
      MET_SYM_FREE(kid3);
      kid3 = METsym_new_op(MET_SYM_OP_POW, sym->origin);
      METsym_add_kid(kid3, sym->kids[0]);
      METsym_add_kid(kid3, kid);
      MET_SYM_FREE(kid);
      kid = METsym_new_op(MET_SYM_OP_MULTIPLY, sym->origin);
      METsym_add_kid(kid, sym->kids[1]);
      METsym_add_kid(kid, kid1);

	 if (MET_SYM_TYPE_SET_REF != kid2->type ||
	     MET_set_zero != kid2->info.set_ref.set) {
	    /*
	     * After this if KID has either (g f') or
	     * or (f g' log f) or (g f' + f g' log f),
	     * g constant -> first
	     * f constant -> second
	     * neither -> third
	     */
	    kid5 = METsym_new_op(MET_SYM_OP_LN, sym->origin);
	    METsym_add_kid(kid5, sym->kids[0]);
	    kid4 = METsym_new_op(MET_SYM_OP_MULTIPLY, sym->origin);
	    METsym_add_kid(kid4, sym->kids[0]);
	    METsym_add_kid(kid4, kid5);
	    MET_SYM_FREE(kid5);
	    kid5 = METsym_new_op(MET_SYM_OP_MULTIPLY, sym->origin);
	    METsym_add_kid(kid5, kid4);
	    METsym_add_kid(kid5, kid2);
	    MET_SYM_FREE(kid4);
	    if (MET_SYM_TYPE_SET_REF == kid1->type &&
		MET_set_zero == kid1->info.set_ref.set) {
	       /* f is constant */
	       MET_SYM_FREE(kid);
	       MET_SYM_COPY(kid, kid5);
	       MET_SYM_FREE(kid5);
	    } else {
	       kid4 = METsym_new_op(MET_SYM_OP_ADD, sym->origin);
	       METsym_add_kid(kid4, kid5);
	       METsym_add_kid(kid4, kid);
	       MET_SYM_FREE(kid5);
	       MET_SYM_FREE(kid);
	       MET_SYM_COPY(kid, kid4);
	       MET_SYM_FREE(kid4);
	    }
	 }
	 result = METsym_new_op(MET_SYM_OP_MULTIPLY, sym->origin);
	 METsym_add_kid(result, kid3);
	 METsym_add_kid(result, kid);
	 MET_SYM_FREE(kid3);
	 MET_SYM_FREE(kid);

      MET_SYM_FREE(kid2);

    bail3:
      MET_SYM_FREE(kid1);
      break;

    case MET_SYM_OP_APPLY:
    case MET_SYM_OP_OUT_PROD_MAP:
    case MET_SYM_OP_DOUBLE_MAP:
      save_handler = MET_defaults->error_handler.handler;
      MET_defaults->error_handler.handler = MET_error_handler_ignore;
      kid = MET_opt_inline(sym->kids[0], sym->kids[1]);
      MET_defaults->error_handler.handler = save_handler;
      if (MET_NULL_SYM == kid) {
	 result = MET__op_prime_product(sym, MET_SYM_OP_MULTIPLY,
					partial_alpha, id);
      } else {
	 result = MET__op_prime_partial(kid, partial_alpha, id);
	 MET_SYM_FREE(kid);
      }
      break;
#if 0
      /* the apply used to be done with the chain rule, but
	 that didn't work unless we were in R^n */

      kid = METsym_new_op(MET_SYM_OP_PRIME, sym->origin);
      METsym_add_kid(kid, sym->kids[0]);

      MET_TYPE_NEW_UNKNOWN(unknown);
      left_prime = MET_op_prime(kid, unknown, (METtype **) NULL,
                                INULL); /* Ack! */
      MET_TYPE_FREE(unknown);

      MET_SYM_FREE(kid);
      if (MET_NULL_SET == left_prime) {
         result = MET_NULL_SYM;
         break;
      }
      kid = METsym_new_apply(sym->origin);
      kid2 = METsym_new_set_ref(left_prime, sym->kids[0]->origin);
      MET_SET_FREE(left_prime);
      METsym_add_kid(kid, kid2);
      MET_SYM_FREE(kid2);
      METsym_add_kid(kid, sym->kids[1]);
      result = METsym_new_op(MET_SYM_OP_MULTIPLY, sym->origin);
      METsym_add_kid(result, kid);
      MET_SYM_FREE(kid);
      kid = MET__op_prime_partial(sym->kids[1], partial_alpha, id);
      if (MET_NULL_SYM == kid) {
         MET_SYM_FREE(result);
         result = MET_NULL_SYM;
         break;
      }
      METsym_add_kid(result, kid);
      MET_SYM_FREE(kid);
      break;
#endif 0

    default:
      DEFAULT_ERR(sym->info.op.code);
   }

   return result;
}

/*
 * product rule: (f op g)' = (f' op g) + (f op g')
 */
static METsym *
MET__op_prime_product(sym, op_code, partial_alpha, id)
   METsym	*sym, *partial_alpha;
   int		id, op_code;
{
   METsym	*result, *kid, *kid2, *kid3;
   MME(MET__op_product);
   
#ifdef OP__CHECK_PARMS
   if (2 != sym->num_kids) {
      SERVER((ERR_SI, "product rule sym doesn't have 2 kids:",
	      sym->num_kids));
      ERROR(sym->origin, "product without 2 kids, munging", SEV_FATAL);
      MET_SYM_COPY(result, sym);
      return result;
   }
   
#endif
   kid2 = METsym_new_op(op_code, sym->origin);
   kid = MET__op_prime_partial(sym->kids[0], partial_alpha, id);
   if (MET_NULL_SYM == kid)
      goto bail1;
   METsym_add_kid(kid2, kid);
   MET_SYM_FREE(kid);
   METsym_add_kid(kid2, sym->kids[1]);
   
   kid3 = METsym_new_op(op_code, sym->origin);
   METsym_add_kid(kid3, sym->kids[0]);
   kid = MET__op_prime_partial(sym->kids[1], partial_alpha, id);
   if (MET_NULL_SYM == kid)
      goto bail2;
   METsym_add_kid(kid3, kid);
   MET_SYM_FREE(kid);
   
   result = METsym_new_op(MET_SYM_OP_ADD, sym->origin);
   METsym_add_kid(result, kid2);
   METsym_add_kid(result, kid3);
   MET_SYM_FREE(kid2);
   MET_SYM_FREE(kid3);
   
   return result;

 bail2:
   MET_SYM_FREE(kid3);
 bail1:
   MET_SYM_FREE(kid2);
   result = MET_NULL_SYM;
   
   return result;
}

/*
 * take a partial derivative of sym by partial alpha.
 * recurse using the table for direction, when reach alpha,
 * substitute partial_alpha.
 */
static METsym *
MET__op_prime_partial(sym, partial_alpha, id)
   METsym	*sym, *partial_alpha;
   int		id;
{
   METsym	*result, *kid;
   METset	*tset, *set;
   METtype	*unknown;
   int		i;
   METlist	*list;
   MME(MET__op_prime_partial);

   LIST_HASH_FIND_SYM(list, sym, id);
   if (MET_NULL_LIST != list) {
      MET_SYM_COPY(result, list->object.sym);
      return result;
   }

   switch (sym->type) {

    case MET_SYM_TYPE_OP:
      switch (OP_PRIME_RULE(sym->info.op.code)) {

       case PRIME_RULE_NONE:
	 result = MET_NULL_SYM;
	 break;

       case PRIME_RULE_LINEAR:
	 result = METsym_new_op(sym->info.op.code, sym->origin);
	 for (i = 0; i < sym->num_kids; i++) {
	    kid = MET__op_prime_partial(sym->kids[i], partial_alpha, id);
	    if (MET_NULL_SYM == kid) {
	       MET_SYM_FREE(result);
	       result = MET_NULL_SYM;
	       break;
	    }
	    METsym_add_kid(result, kid);
	    MET_SYM_FREE(kid);
	 }
	 break;
	    
       case PRIME_RULE_PRODUCT:
	 result = MET__op_prime_product(sym, sym->info.op.code,
					partial_alpha, id);
	 break;

       case PRIME_RULE_CHAIN:
	 result = METsym_new_op(MET_SYM_OP_MULTIPLY, sym->origin);

	 kid = MET__op_prime_chain(sym->info.op.code, sym->kids[0],
				   sym->origin);
	 if (MET_NULL_SYM == kid)
	    goto bail3;
	 METsym_add_kid(result, kid);
	 MET_SYM_FREE(kid);

	 kid = MET__op_prime_partial(sym->kids[0], partial_alpha, id);
	 if (MET_NULL_SYM == kid)
	    goto bail3;
	 METsym_add_kid(result, kid);
	 MET_SYM_FREE(kid);
	 
	 break;
       bail3:
	 MET_SYM_FREE(result);
	 break;
	 
       case PRIME_RULE_SPECIAL:
	 result = MET__op_prime_special(sym, partial_alpha, id);
	 break;

       default:
	 DEFAULT_ERR(OP_PRIME_RULE(sym->info.op.code));
      }
      break;

    case MET_SYM_TYPE_CONTROL:
      result = METsym_new_set_ref(MET_set_zero, sym->origin);
      /* should check the type, build a set from the type, then build a */
      /* zero set from that */  /* Ack! */
      break;

    case MET_SYM_TYPE_VARIABLE:
      MET_SYM_COPY(result, partial_alpha);
      break;

    case MET_SYM_TYPE_SET_REF:
      MET_TYPE_NEW_UNKNOWN(unknown);
      set = MET_sym_eval_set_ref(sym, unknown, (METtype **) NULL, INULL,
				 MET_NULL_ENV);
      if (MET_NULL_SET != set) {
	 tset = MET_set_zeroed(set);
	 MET_SET_FREE(set);
	 result = METsym_new_set_ref(tset, sym->origin);
	 MET_SET_FREE(tset);
      } else
	 result = MET_NULL_SYM;
      MET_TYPE_FREE(unknown);
      
      break;

    case MET_SYM_TYPE_DATA:
      result = MET_NULL_SYM;
      break;
      
    default:
      DEFAULT_ERR(sym->type);
   }

   if (MET_NULL_SYM != result)
      LIST_HASH_ADD_SYM(list, sym, result, id);

   return result;
}

/*
 * when traversing the type and generating code for the derivative,
 * at the leaves we want to generate partial alpha, and then take
 * a partial of the main body of the code.
 */
static METsym *
MET__op_prime_handler_code(type, info, path_len)
   METtype	*type;
   MET__op_prime_info *info;
   int		path_len;
{
   METsym	*partial_alpha, *result;
   int		id;
   MET__op_prime_info my_info;
   MME(MET__op_prime_handler_code);

   info->path_len = path_len;
   my_info.root = MET_NULL_TYPE;
   my_info.sym  = MET_NULL_SYM;
   my_info.handler = MET__op_prime_handler_partial;
   my_info.parent = info;
   partial_alpha = MET__op_prime_type_to_sym(info->root, &my_info, 0);
   if (MET_NULL_SYM == partial_alpha)
      return MET_NULL_SYM;

   id = METlist_hash_new_id();
   result = MET__op_prime_partial(info->sym, partial_alpha, id);
   METlist_hash_free_id(id);
   MET_SYM_FREE(partial_alpha);

   return result;
}

/*
 * when traversing the type to generate partial alpha, the leaves are
 * either zero or one, depending if this is the leaf that we are taking the
 * partial by
 */
static METsym *
MET__op_prime_handler_partial(type, info, path_len)
   METtype	*type;
   MET__op_prime_info *info;
   int		path_len;
{
   MET__op_prime_info *parent = info->parent;
   Boolean	is_one = YES;
   int		i;
   MME(MET__op_prime_handler_partial);

   if (path_len == parent->path_len) {
      for (i = 0; i < path_len; i++)
	 if (info->path[i] != parent->path[i]) {
	    is_one = NO;
	    break;
	 }
   } else
      is_one = NO;

   return METsym_new_set_ref(is_one ? MET_set_one : MET_set_zero, RNULL);
}

/*
 * generate code for a vector.  this is recurses here for a while,
 * then jumps back to the main recursion.
 * this is a linear recursion Ack!
 */
static METsym *
MET__op_prime_expand_vector(dim, leaf, info, path_len)
   int		dim;
   METtype	*leaf;
   MET__op_prime_info *info;
{
   METsym	*new_sym, *kid;
   MME(MET__op_prime_expand_vector);

   if (1 == dim) {
      info->path[path_len] = dim;
      return MET__op_prime_type_to_sym(leaf, info, path_len + 1);
   }

   new_sym = METsym_new_op(MET_SYM_OP_PAIR_BUILD, RNULL);

   kid = MET__op_prime_expand_vector(dim - 1, leaf, info, path_len);
   if (MET_NULL_SYM == kid)
      goto bail;
   METsym_add_kid(new_sym, kid);
   MET_SYM_FREE(kid);

   info->path[path_len] = dim;
   kid = MET__op_prime_type_to_sym(leaf, info, path_len + 1);
   if (MET_NULL_SYM == kid)
      goto bail;
   METsym_add_kid(new_sym, kid);
   MET_SYM_FREE(kid);

   return new_sym;

 bail:
   MET_SYM_FREE(new_sym);
   return MET_NULL_SYM;
}

/*
 * generate derivative code from type code.  a lexical translation.
 * at leaves, generate a partial from sym;
 */
static METsym *
MET__op_prime_type_to_sym(type, info, path_len)
   METtype	*type;
   MET__op_prime_info *info;
   int		path_len;
{
   METsym	*new_sym, *kid;
   MME(MET__op_prime_type_to_sym);

   switch (type->type) {
      
    case MET_TYPE_TYPE_MAP:
      return MET_NULL_SYM;
      
    case MET_TYPE_TYPE_PAIR:
      new_sym = METsym_new_op(MET_SYM_OP_PAIR_BUILD, RNULL);

      info->path[path_len] = -1;
      kid = MET__op_prime_type_to_sym(type->info.pair.left, info,
				      path_len + 1);
      if (MET_NULL_SYM == kid)
	 break;
      METsym_add_kid(new_sym, kid);
      MET_SYM_FREE(kid);

      info->path[path_len] = -2;
      kid = MET__op_prime_type_to_sym(type->info.pair.right, info,
				      path_len + 1);
      if (MET_NULL_SYM == kid)
	 break;
      METsym_add_kid(new_sym, kid);
      MET_SYM_FREE(kid);

      return new_sym;

    case MET_TYPE_TYPE_VECTOR:
      new_sym =
	 METsym_new_op(type->info.vector.is_row ?
		       MET_SYM_OP_ROW_BUILD :
		       MET_SYM_OP_COLUMN_BUILD, RNULL);
      kid = MET__op_prime_expand_vector(type->info.vector.dimension,
					type->info.vector.of, info,
					path_len);
      if (MET_NULL_SYM == kid)
	 break;
      METsym_add_kid(new_sym, kid);
      MET_SYM_FREE(kid);
      return new_sym;

    case MET_TYPE_TYPE_FIELD:
      return (info->handler)(type, info, path_len);
      
    case MET_TYPE_TYPE_SET_OF:
    case MET_TYPE_TYPE_UNKNOWN:
      return MET_NULL_SYM;

    default:
      DEFAULT_ERR(type->type);
   }

   MET_SYM_FREE(new_sym);
   return MET_NULL_SYM;
}

/*
 * take a type (guaranteed ? -> ?) and a symbolic description of a function,
 * and return a set which is its derivative.
 *
 * algorithm: traverse type of domain, generating equivalent code.  when we
 * reach a leaf, we go back to the top (we could be passing the top node down)
 * and construct a partial alpha.  this has analagous structure, but the
 * leaves are 0 or 1 (0 for all but the one we are on).  Now, take a partial
 * of the function body by following the table rules.  The partial of alpha is
 * passed down and substituted when it is encountered.
 */
static METset *
MET__op_prime(type, sym, from, tagger)
   METtype	*type;
   METsym	*sym;
   METtype 	*from;
   METset	*tagger;
{
   METset	*result;
   METsym	*map, *pre_map;
   METmap	*vert_map;
   METenv	*new_env;
   METtype	*result_type, *domain_type, *codomain_type;
   MET__op_prime_info info;
   MME(MET__op_prime);

   info.root = type->info.map.from;
   info.sym = sym;
   info.handler = MET__op_prime_handler_code;
   pre_map = MET__op_prime_type_to_sym(type->info.map.from, &info, 0);
   if (MET_NULL_SYM == pre_map)
      return MET_NULL_SET;
   map = MET_opt_peephole(pre_map);
   MET_SYM_FREE(pre_map);
   pre_map = MET_opt_cse_eliminate(map);
   MET_SYM_FREE(map);

   MET_TYPE_COPY(domain_type, type->info.map.from);
   MET_TYPE_NEW_UNKNOWN(codomain_type); /* Ack! */
   MET_TYPE_NEW_MAP(result_type, domain_type, codomain_type);
   MET_TYPE_FREE(domain_type);
   MET_TYPE_FREE(codomain_type);
   result = MET_type_to_polybare_set(result_type, 1, &tagger, 1);
   MET_TYPE_FREE(result_type);
   vert_map = DATA_FIRST_VERTEX(result->info.bare.data, METmap);
   MET_SYM_COPY(vert_map->sym, pre_map);
   MET_TYPE_COPY(vert_map->loose_from, from);
   MET_SYM_FREE(pre_map);
   new_env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV); /* Ack! */
   MET_ENV_COPY(vert_map->env, new_env);
   MET_ENV_FREE(new_env);

   return result;
}


METset *
MET_op_prime(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[PRIME_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[PRIME_NUM_KIDS];
   METtype	*arg_type[PRIME_NUM_KIDS];
   METtype	*guess, *from;
   METsym	*map;
   MME(MET_op_prime);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, PRIME_NUM_KIDS);
#endif
   MET_TYPE_NEW_UNKNOWN(guess); /* Ack! */
   arg_set[PRIME_ARG] =
      MET_sym_be_set_ref(sym->kids[PRIME_ARG], guess,
			 &arg_type[PRIME_ARG],
			 &arg_set_type[PRIME_ARG]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[PRIME_ARG] ||
       MET_NULL_SET == arg_set[PRIME_ARG])
      goto bail1;

   if (MET_TYPE_TYPE_MAP != arg_type[PRIME_ARG]->type)
      goto bail1;

   if (1 != DATA_NUM_VERTS(arg_set[PRIME_ARG]->info.bare.data))
      goto bail1;

   MET_SYM_COPY(map,
	        DATA_FIRST_VERTEX(arg_set[PRIME_ARG]->info.bare.data,
				  METmap)->sym);
   MET_TYPE_COPY(from,
		 DATA_FIRST_VERTEX(arg_set[PRIME_ARG]->info.bare.data,
				   METmap)->loose_from);
   /*
    * ok, now we have a type and a map.  take the derivative. 
    * the set is just for tags.
    */
   result_set = MET__op_prime(arg_type[PRIME_ARG], map, 
			      from, arg_set[PRIME_ARG]);
   
   MET_SYM_FREE(map);
   MET_TYPE_FREE(from);

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

   COND_FREE(PRIME_ARG);


   return result_set;
}
