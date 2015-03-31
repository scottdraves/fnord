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
   "$Id: op_reduce.c,v 1.6 1992/06/22 17:07:02 mas Exp $";
#endif

#include "defs.h"

#define REDUCE_NUM_KIDS		3
#define REDUCE_SET		0
#define REDUCE_FN		1
#define REDUCE_BASE		2

/*
 * takes a discrete set, a binary function, and a base case.
 * reduce({a, b, c}, f, i) -> f(f(f(i,a),b),c)
 */
/*
 * special cases:
 *    or/and (of boolean)
 *    sum (of anything)
 *    product (of scalars)
 *    union (of sets)
 *    list (of anything)
 * these are detected by munging the function.
 */

/* arg is the vector, result is the base case, which
   should be properly initialized on entry */
#define TEMPLATE(NAME, TYPE1, DST_TYPE) \
static Boolean \
NAME(arg, result) \
   METdata	*arg, *result; \
{ \
   int		i, j, length; \
   TYPE1	*src; \
   DST_TYPE	*dst; \
   int		src_width; \
   TEMPS \
   MME(NAME); \
 \
   length = result->num_cols; \
   src_width = (1 == DATA_NUM_VERTS(arg)) ? 0 : arg->verts->total_cols; \
 \
   for (j = 0; j < length; j++) { \
      src = DATA_FIRST_VERTEX(arg, TYPE1) + j; \
      dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + j; \
      i = arg->verts->num_verts; \
      UNROLL4(i, BODY; src += src_width;) \
   } \
   return SUCCESS; \
}

#define TEMPS
#define BODY *dst += *src
TEMPLATE(MET__op_reduce_guts_sum_i, METinteger, METinteger)
TEMPLATE(MET__op_reduce_guts_sum_r, METreal,    METreal)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY dst->real += src->real; dst->imaginary += src->imaginary
TEMPLATE(MET__op_reduce_guts_sum_c, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst *= *src
TEMPLATE(MET__op_reduce_guts_product_i, METinteger, METinteger)
TEMPLATE(MET__op_reduce_guts_product_r, METreal,    METreal)
#undef BODY
#undef TEMPS

#define TEMPS METreal t;
#define BODY t = (METreal) (src->real * dst->real -        \
			        src->imaginary * dst->imaginary) ; \
	dst->imaginary = (METreal) (src->real * dst->imaginary +   \
			        src->imaginary * dst->real) ; \
        dst->real = t;
TEMPLATE(MET__op_reduce_guts_product_c, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst |= *src
TEMPLATE(MET__op_reduce_guts_or_b, METztwo, METztwo)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst &= *src
TEMPLATE(MET__op_reduce_guts_and_b, METztwo, METztwo)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst = (*src < *dst) ? *src : *dst
TEMPLATE(MET__op_reduce_guts_min_i, METinteger, METinteger)
TEMPLATE(MET__op_reduce_guts_min_r, METreal,    METreal)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst = (*src > *dst) ? *src : *dst
TEMPLATE(MET__op_reduce_guts_max_i, METinteger, METinteger)
TEMPLATE(MET__op_reduce_guts_max_r, METreal,    METreal)
#undef BODY
#undef TEMPS


static void
MET__op_reduce_bad_base(sym, base, type)
   METsym	*sym;
   METset	*base;
   METtype	*type;
{
   char		buf[8000];
   MME(MET__op_reduce_bad_base);

   sprintf(buf, "%s, couldn't cast ", OP_NAME(sym->info.op.code));
   METset_print_pretty(FNULL, base, buf, 0);
   (void) strcat(buf, " to ");
   METtype_print_pretty(FNULL, type, buf, 0);
   (void) strcat(buf, " (bad base case)\n");
   ERROR(sym->origin, buf, SEV_FATAL);
}

static METset *
MET__op_reduce_sum(base, block, fn, sym)
   METset	*base;
   METblock	*block;
   METset	*fn;
   METsym	*sym;
{
   METset	*result, *cast;
   METdata	*src, *dst;
   MME(MET__op_reduce_sum);

   cast = MET_set_cast_to_type(base, block->type->info.set_of.of);
   if (MET_NULL_SET == cast) {
      MET__op_reduce_bad_base(sym, base, block->type->info.set_of.of);
      return MET_NULL_SET;
   }

   if (MET_SET_TYPE_BARE != cast->type) {
      ERROR(sym->origin, 
	    "reduce (sum) only handles sets with bare elements",
	    SEV_FATAL);
      MET_SET_FREE(cast);
      return MET_NULL_SET;
   }

   dst = MET_data_copy_very_deep(cast->info.bare.data);

   result = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(result->info.bare.type,
		 block->type->info.set_of.of);
   result->info.bare.data = dst;
   result->info.bare.how_many = 1;
   MET_tags_initialize(&result->info.bare.tags_bare);
   MET_tags_initialize(&result->info.bare.tags_in);

   MET_SET_FREE(cast);

   src = block->data;
   while (MET_NULL_DATA != src) {
      switch (src->verts->type) {
       case MET_FIELD_INTEGER:
	 (void) MET__op_reduce_guts_sum_i(src, dst);
	 break;
       case MET_FIELD_REAL:
	 (void) MET__op_reduce_guts_sum_r(src, dst);
	 break;
       case MET_FIELD_COMPLEX:
	 (void) MET__op_reduce_guts_sum_c(src, dst);
	 break;
       default:
	 MET_SET_FREE(result);
	 result = MET_NULL_SET;
	 NYET;
      }
      src = src->next;
      dst = dst->next;
   }


   return result;
}


static METset *
MET__op_reduce_min(base, block, fn, sym)
   METset	*base;
   METblock	*block;
   METset	*fn;
   METsym	*sym;
{
   METset	*result, *cast;
   METdata	*src, *dst;
   MME(MET__op_reduce_min);

   cast = MET_set_cast_to_type(base, block->type->info.set_of.of);
   if (MET_NULL_SET == cast) {
      MET__op_reduce_bad_base(sym, base, block->type->info.set_of.of);
      return MET_NULL_SET;
   }

   if (MET_SET_TYPE_BARE != cast->type) {
      ERROR(sym->origin, 
	    "reduce (min) only handles sets with bare elements",
	    SEV_FATAL);
      MET_SET_FREE(cast);
      return MET_NULL_SET;
   }

   dst = MET_data_copy_very_deep(cast->info.bare.data);

   result = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(result->info.bare.type,
		 block->type->info.set_of.of);
   result->info.bare.data = dst;
   result->info.bare.how_many = 1;
   MET_tags_initialize(&result->info.bare.tags_bare);
   MET_tags_initialize(&result->info.bare.tags_in);

   MET_SET_FREE(cast);

   src = block->data;
   while (MET_NULL_DATA != src) {
      switch (src->verts->type) {
       case MET_FIELD_INTEGER:
	 (void) MET__op_reduce_guts_min_i(src, dst);
	 break;
       case MET_FIELD_REAL:
	 (void) MET__op_reduce_guts_min_r(src, dst);
	 break;
       default:
	 MET_SET_FREE(result);
	 result = MET_NULL_SET;
	 NYET;
      }
      src = src->next;
      dst = dst->next;
   }


   return result;
}

static METset *
MET__op_reduce_max(base, block, fn, sym)
   METset	*base;
   METblock	*block;
   METset	*fn;
   METsym	*sym;
{
   METset	*result, *cast;
   METdata	*src, *dst;
   MME(MET__op_reduce_max);

   cast = MET_set_cast_to_type(base, block->type->info.set_of.of);
   if (MET_NULL_SET == cast) {
      MET__op_reduce_bad_base(sym, base, block->type->info.set_of.of);
      return MET_NULL_SET;
   }
   
   if (MET_SET_TYPE_BARE != cast->type) {
      ERROR(sym->origin, 
	    "reduce (max) only handles sets with bare elements",
	    SEV_FATAL);
      MET_SET_FREE(cast);
      return MET_NULL_SET;
   }

   dst = MET_data_copy_very_deep(cast->info.bare.data);

   result = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(result->info.bare.type,
		 block->type->info.set_of.of);
   result->info.bare.data = dst;
   result->info.bare.how_many = 1;
   MET_tags_initialize(&result->info.bare.tags_bare);
   MET_tags_initialize(&result->info.bare.tags_in);

   MET_SET_FREE(cast);

   src = block->data;
   while (MET_NULL_DATA != src) {
      switch (src->verts->type) {
       case MET_FIELD_INTEGER:
	 (void) MET__op_reduce_guts_max_i(src, dst);
	 break;
       case MET_FIELD_REAL:
	 (void) MET__op_reduce_guts_max_r(src, dst);
	 break;
       default:
	 MET_SET_FREE(result);
	 result = MET_NULL_SET;
	 NYET;
      }
      src = src->next;
      dst = dst->next;
   }


   return result;
}

static METset *
MET__op_reduce_product(base, block, fn, sym)
   METset	*base;
   METblock	*block;
   METset	*fn;
   METsym	*sym;
{
   METset	*result, *cast;
   METdata	*src, *dst;
   MME(MET__op_reduce_product);

   cast = MET_set_cast_to_type(base, block->type->info.set_of.of);
   if (MET_NULL_SET == cast) {
      MET__op_reduce_bad_base(sym, base, block->type->info.set_of.of);
      return MET_NULL_SET;
   }

   if (MET_SET_TYPE_BARE != cast->type) {
      ERROR(sym->origin, 
	    "reduce (product) only handles sets with bare elements",
	    SEV_FATAL);
      MET_SET_FREE(cast);
      return MET_NULL_SET;
   }

   dst = MET_data_copy_very_deep(cast->info.bare.data);
   result = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(result->info.bare.type,
		 block->type->info.set_of.of);
   result->info.bare.data = dst;
   result->info.bare.how_many = 1;
   MET_tags_initialize(&result->info.bare.tags_bare);
   MET_tags_initialize(&result->info.bare.tags_in);

   MET_SET_FREE(cast);

   src = block->data;
   switch (src->verts->type) {
    case MET_FIELD_INTEGER:
      (void) MET__op_reduce_guts_product_i(src, dst);
      break;
    case MET_FIELD_REAL:
      (void) MET__op_reduce_guts_product_r(src, dst);
      break;
    case MET_FIELD_COMPLEX:
      (void) MET__op_reduce_guts_product_c(src, dst);
      break;
    default:
      MET_SET_FREE(result);
      result = MET_NULL_SET;
      NYET;
   }

   return result;
}

static METset *
MET__op_reduce_union(base, block, fn, sym)
   METset	*base;
   METblock	*block;
   METset	*fn;
   METsym	*sym;
{
   int		i, num;
   METset	*result, **scan;
   MME(MET__op_reduce_union);

   if (MET_SET_TYPE_BLOCK != base->type) {
      MET__op_reduce_bad_base(sym, base, block->type->info.set_of.of);
      return MET_NULL_SET;
   }

   if (MET_TYPE_TYPE_SET_OF != block->type->type)
      return MET_NULL_SET;

   result = MET_set_new(MET_SET_TYPE_BLOCK);
   result->info.block.block_list = MET_NULL_BLOCK;
   MET_set_add_block(result, base->info.block.block_list);

   scan = DATA_FIRST_VERTEX(block->data, METset*);
   num = block->data->verts->num_verts;
   for (i = 0; i < num; i++) {
      MET_set_add_block(result, (*scan)->info.block.block_list);
      scan += block->data->verts->total_cols;
   }

   return result;
}

static METset *
MET__op_reduce_list(base, block, fn, sym)
   METset	*base;
   METblock	*block;
   METset	*fn;
   METsym	*sym;
{
   MME(MET__op_reduce_list);
   NYET;
   return MET_NULL_SET;
}

static METset *
MET__op_reduce_or(base, block, fn, sym)
   METset	*base;
   METblock	*block;
   METset	*fn;
   METsym	*sym;
{
   METset	*result, *cast;
   METdata	*src, *dst;
   MME(MET__op_reduce_or);

   cast = MET_set_cast_to_type(base, block->type->info.set_of.of);
   if (MET_NULL_SET == cast) {
      MET__op_reduce_bad_base(sym, base, block->type->info.set_of.of);
      return MET_NULL_SET;
   }

   if (MET_SET_TYPE_BARE != cast->type) {
      ERROR(sym->origin, 
	    "reduce (or) only handles sets with bare elements",
	    SEV_FATAL);
      MET_SET_FREE(cast);
      return MET_NULL_SET;
   }

   dst = MET_data_copy_very_deep(cast->info.bare.data);
   result = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(result->info.bare.type,
		 block->type->info.set_of.of);
   result->info.bare.data = dst;
   result->info.bare.how_many = 1;
   MET_tags_initialize(&result->info.bare.tags_bare);
   MET_tags_initialize(&result->info.bare.tags_in);

   MET_SET_FREE(cast);

   src = block->data;
   switch (src->verts->type) {
    case MET_FIELD_ZTWO:
      (void) MET__op_reduce_guts_or_b(src, dst);
      break;
    default:
      MET_SET_FREE(result);
      result = MET_NULL_SET;
      NYET;
   }

   return result;
}

static METset *
MET__op_reduce_and(base, block, fn, sym)
   METset	*base;
   METblock	*block;
   METset	*fn;
   METsym	*sym;
{
   METset	*result, *cast;
   METdata	*src, *dst;
   MME(MET__op_reduce_and);

   cast = MET_set_cast_to_type(base, block->type->info.set_of.of);
   if (MET_NULL_SET == cast) {
      MET__op_reduce_bad_base(sym, base, block->type->info.set_of.of);
      return MET_NULL_SET;
   }

   if (MET_SET_TYPE_BARE != cast->type) {
      ERROR(sym->origin, 
	    "reduce (and) only handles sets with bare elements",
	    SEV_FATAL);
      MET_SET_FREE(cast);
      return MET_NULL_SET;
   }

   dst = MET_data_copy_very_deep(cast->info.bare.data);
   result = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(result->info.bare.type,
		 block->type->info.set_of.of);
   result->info.bare.data = dst;
   result->info.bare.how_many = 1;
   MET_tags_initialize(&result->info.bare.tags_bare);
   MET_tags_initialize(&result->info.bare.tags_in);

   MET_SET_FREE(cast);

   src = block->data;
   switch (src->verts->type) {
    case MET_FIELD_ZTWO:
      (void) MET__op_reduce_guts_and_b(src, dst);
      break;
    default:
      MET_SET_FREE(result);
      result = MET_NULL_SET;
      NYET;
   }

   return result;
}

static METset *
MET__op_reduce_general(base, block, fn, sym)
   METset	*base;
   METblock	*block;
   METset	*fn;
   METsym	*sym;
{
   MME(MET__op_reduce_general);
   NYET;
   return MET_NULL_SET;
}

static METset *
MET__op_reduce_guts(arg_set, arg_type, sym)
   METset	**arg_set;
   METtype	**arg_type;
   METsym	*sym;
{
   METmap	*fn;
   METsym	*t;
   METset	*(*doit)(), *result, *next_result;
   METblock	*block;
   MME(MET__op_reduce_guts);

   fn = DATA_FIRST_VERTEX(arg_set[REDUCE_FN]->info.bare.data, METmap);
   
   if (MET_SYM_TYPE_VARIABLE == fn->sym->type) {
      doit = MET__op_reduce_list;
   } else if (MET_SYM_TYPE_OP == fn->sym->type &&
	      2 == fn->sym->num_kids &&
	      MET_SYM_TYPE_OP == (t = fn->sym->kids[0])->type &&
	      MET_SYM_OP_PAIR_LEFT == t->info.op.code &&
	      MET_SYM_TYPE_VARIABLE == t->kids[0]->type &&
	      MET_SYM_TYPE_OP == (t = fn->sym->kids[1])->type &&
	      MET_SYM_OP_PAIR_RIGHT == t->info.op.code &&
	      MET_SYM_TYPE_VARIABLE == t->kids[0]->type) {
      switch (fn->sym->info.op.code) {
       case MET_SYM_OP_PAIR_BUILD:
	 doit = MET__op_reduce_list;
	 break;
       case MET_SYM_OP_ADD:
	 doit = MET__op_reduce_sum;
	 break;
       case MET_SYM_OP_MULTIPLY:
	 doit = MET__op_reduce_product;
	 break;
       case MET_SYM_OP_UNION:
	 doit = MET__op_reduce_union;
	 break;
       case MET_SYM_OP_OR:
	 doit = MET__op_reduce_or;
	 break;
       case MET_SYM_OP_AND:
	 doit = MET__op_reduce_and;
	 break;
       case MET_SYM_OP_MIN:
	 doit = MET__op_reduce_min;
	 break;
       case MET_SYM_OP_MAX:
	 doit = MET__op_reduce_max;
	 break;
       default:
	 doit = MET__op_reduce_general;
      }
   } else
      doit = MET__op_reduce_general;


   MET_SET_COPY(result, arg_set[REDUCE_BASE]);
   block = arg_set[REDUCE_SET]->info.block.block_list;
   while (MET_NULL_BLOCK != block) {
      next_result = (doit)(result, block, arg_set[REDUCE_FN], sym);
      MET_SET_FREE(result);
      if (MET_NULL_SET == next_result)
	 return MET_NULL_SET;
      MET_SET_COPY(result, next_result);
      MET_SET_FREE(next_result);
      block = block->next;
   }

   return result;
}

METset *
MET_op_reduce(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[REDUCE_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[REDUCE_NUM_KIDS];
   METtype	*arg_type[REDUCE_NUM_KIDS];
   METtype	*my_result_type = MET_NULL_TYPE, *guess, *t;
   MME(MET_op_reduce);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, REDUCE_NUM_KIDS);
#endif

   if (MET_NULL_TYPE != type) {
      MET_TYPE_NEW_SET_OF(guess, type);
   } else {
      MET_TYPE_NEW_UNKNOWN(guess);
   }
   arg_set[REDUCE_SET] =
      MET_sym_be_set_ref(sym->kids[REDUCE_SET], guess,
			 &arg_type[REDUCE_SET],
			 &arg_set_type[REDUCE_SET]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE != type) {
      MET_TYPE_NEW_PAIR(t, type, type);
      MET_TYPE_NEW_MAP(guess, t, type);
      MET_TYPE_FREE(t);
   } else {
      MET_TYPE_COPY(guess, MET_type_generic_map);
   }
   arg_set[REDUCE_FN] =
      MET_sym_be_set_ref(sym->kids[REDUCE_FN], guess,
			 &arg_type[REDUCE_FN],
			 &arg_set_type[REDUCE_FN]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE != arg_type[REDUCE_SET] &&
       MET_TYPE_TYPE_SET_OF == arg_type[REDUCE_SET]->type) {
      MET_TYPE_COPY(guess, arg_type[REDUCE_SET]->info.set_of.of);
   } else {
      MET_TYPE_NEW_UNKNOWN(guess);
   }
   arg_set[REDUCE_BASE] =
      MET_sym_be_set_ref(sym->kids[REDUCE_BASE], guess,
			 &arg_type[REDUCE_BASE],
			 &arg_set_type[REDUCE_BASE]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[REDUCE_SET] ||
       MET_NULL_TYPE == arg_type[REDUCE_FN] ||
       MET_NULL_TYPE == arg_type[REDUCE_BASE])
      goto bail1;

   if (arg_type[REDUCE_SET]->type != MET_TYPE_TYPE_SET_OF) {
      MET_op_error_bad_arg_type(1, arg_type[REDUCE_SET], "{?}", sym);
      goto bail1;
   }
   if (arg_type[REDUCE_FN]->type != MET_TYPE_TYPE_MAP) {
      MET_op_error_bad_arg_type(2, arg_type[REDUCE_FN], "? -> ?", sym);
      goto bail1;
   }

   if (MET_SET_TYPE_BARE != arg_set[REDUCE_FN]->type) {
      ERROR(sym->origin, "first arg must be set, second must be fn", 
	    SEV_FATAL);
      goto bail1;
   }
   
   MET_TYPE_COPY(my_result_type, arg_type[REDUCE_FN]->info.map.to);
   
   if (MET_op_type_only(arg_set, REDUCE_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE)) {
      goto bail2;
   }

   MET_op_reconcile_sets(arg_set, REDUCE_NUM_KIDS);

   if (YES == MET_op_block_only(arg_set, REDUCE_NUM_KIDS, MET_op_reduce,
				MET_SYM_OP_REDUCE, my_result_type)) {
      MET_SET_COPY(result_set, arg_set[0]);
      MET_TYPE_FREE(my_result_type);
      goto bail1;
   } 

   MET_TYPE_FREE(my_result_type);

   result_set = MET__op_reduce_guts(arg_set, arg_type, sym);

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(REDUCE_SET);
   COND_FREE(REDUCE_FN);
   COND_FREE(REDUCE_BASE);

   return result_set;
}
