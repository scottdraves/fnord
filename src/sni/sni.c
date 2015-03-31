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
   "$Id: sni.c,v 1.7 1992/06/22 17:08:48 mas Exp $";
#endif

/*
 * code to experiment with a new implicit surface algorithm
 */

#include "sni/defs.h"

#ifdef PROGRAM_IMPLICIT

/*
 * first arg should be {R^n}
 * second arg should be a fn (R^n, R^n) -> A
 * the third arg should be a scalar real R
 *
 * the result is {(A, R^n)}
 */

#define LOCAL_NUM_KIDS		4
#define LOCAL_PTS		0
#define LOCAL_FN		1
#define LOCAL_RADIUS		2
#define LOCAL_DIMS		3

/* try a grid size of radius */
#define RADIUS_GRID_FACTOR 1

static void
MET__op_local_find_extent(g)
   hash_globals *g;
{
   int		i, k;
   METreal	*src;
   MME(MET__op_local_find_extent);

   src = g->src_base;
   for (k = 0; k < g->dims; k++)
      g->max[k] = g->min[k] = src[k];

   for (i = 0; i < g->num_pts; i++) {
      for (k = 0; k < g->dims; k++) {
	 if (src[k] > g->max[k])
	    g->max[k] = src[k];
	 if (src[k] < g->min[k])
	    g->min[k] = src[k];
      }
      src += g->src_width;
   }
   /*
    * expand the bbox by a bit so that everything
    * is really inside
    */
   for (k = 0; k < g->dims; k++) {
      g->max[k] = 1.001*(g->max[k] - g->min[k]) + g->min[k];
      g->min[k] = g->max[k] - 1.001*(g->max[k] - g->min[k]);
   }
}

static void
MET__op_local_find_grid(g)
   hash_globals *g;
{
   int		k;
   int		by_radius, by_pow;
   MME(MET__op_local_find_grid);
   /*
    * our grid in each direction is the least of
    * RGF*(max - min)/g->radius and RGF*num_pts^(1/dims)
    */
   g->num_buckets = 1;
   for (k = 0; k < g->dims; k++) {
      g->grid_total[k] = g->num_buckets;
      by_radius = RADIUS_GRID_FACTOR * (g->max[k] - g->min[k]) / g->radius;
      g->grid[k] = by_radius;
      g->num_buckets *= g->grid[k];
   }
}

static void
MET__op_local_make_table(g)
   hash_globals *g;
{
   int		next_link, key, i, k;
   METreal	*src;
   MME(MET__op_local_make_table);
   /*
    * make room for the buckets of the hash table,
    * and for the linked lists.  each pt shows up
    * exactly once, and each link is a index and
    * a next field = 2 ints per pt.
    */
   ALLOCNM(g->buckets, int, g->num_buckets + 2 * g->num_pts);
   g->links = g->buckets + g->num_buckets;
   next_link = 0; /* indexed off of links */
   /*
    * initialize the hash table to -1s
    */
   for (i = 0; i < g->num_buckets; i++)
      g->buckets[i] = -1;

   /*
    * put the points into buckets
    */
   src = g->src_base;
   for (i = 0; i < g->num_pts; i++) {
      key = ((int) g->grid[0] *
	     (src[0] - g->min[0]) / (g->max[0] - g->min[0]));
      for (k = 1; k < g->dims; k++) {
	 key += g->grid_total[k] *
	    (int) (g->grid[k] *
		   (src[k] - g->min[k]) / (g->max[k] - g->min[k])); 
      }
      /* link it in */
      g->links[next_link] = i;
      g->links[next_link + 1] = g->buckets[key];
      g->buckets[key] = next_link;
      next_link += 2;

      src += g->src_width;
   }
}

static void
MET__op_local_chase_list(g, key, this_pt)
   hash_globals	*g;
   int		key;
   METreal	*this_pt;
{
   int		k;
   METreal	*src;
   METreal	dist, t;
   MME(MET__op_local_chase_list);

   key = g->buckets[key];
   while (-1 != key) {

      src = g->src_base + (g->links[key] * g->src_width);
      dist = 0.0;
      for (k = 0; k < g->dims; k++) {
	 t = this_pt[k] - src[k];
	 dist += t * t;
      }
      if (dist <= g->radius_squared) {
	 
	 g->num_pairs++;
	 for (k = 0; k < g->width; k++)
	    g->dst[k] = this_pt[k];
	 g->dst += g->width;
	 
	 for (k = 0; k < g->width; k++)
	    g->dst[k] = src[k];
	 g->dst += g->width;
	 
	 if (g->dst == g->last_pair) {
	    REALLOCNM(g->pairs, METreal,
		      2 * g->width * (g->num_pairs + 1000),
		      2 * g->width * (g->num_pairs));
	    g->last_pair = g->pairs + 2 * g->width * (g->num_pairs + 1000);
	    g->dst       = g->pairs + 2 * g->width * g->num_pairs;
	 }
      }

      key = g->links[key + 1];
   }
}

static METdata *
MET__op_local_wrap_pairs(g)
   hash_globals *g;
{
   METverts	*verts;
   METdata	*data;
   MME(MET__op_local_wrap_pairs);
   
   MET_VERTS_ALLOC(verts);
   verts->type = MET_FIELD_REAL;
   verts->num_verts = g->num_pairs;
   verts->total_cols = g->width * 2;
   verts->vertices = (METnumber *) g->pairs;

   data = MET_data_new(verts, g->width, 0);
   data->next = MET_data_new(verts, g->width, g->width);
   MET_VERTS_FREE(verts);

   return data;
}

static METdata *
MET__op_local_find_pairs(pts, radius, dims, indices)
   METblock	*pts;
   METreal	radius;
   int		dims;
   int		*indices;
{
   hash_globals g;
   METdata	*data = pts->data;
   
   METreal	this_pt[MAX_DIMS];
   METreal	*src;
   int		center[MAX_DIMS];
   int		i, k;
   int		key;
   MME(MET__op_local_find_pairs);

   g.src_base = (METreal *) data->verts->vertices;
   g.width = data->num_cols;
   g.dims = dims;
   g.src_width = data->verts->total_cols;
   g.num_pts = data->verts->num_verts;
   g.radius = radius;
   g.radius_squared = radius * radius;

   MET__op_local_find_extent(&g);
   MET__op_local_find_grid(&g);
   MET__op_local_make_table(&g);

   /*
    * PAIRS points to beginning
    * of block of pairs.  DST points to current place in
    * it.  LAST_PAIR points off end of pairs array.
    */
   ALLOCNM(g.pairs, METreal, 2 * g.width * 5 * g.num_pts);
   g.last_pair = g.pairs + 2 * g.width * 5 * g.num_pts;
   g.dst = g.pairs;

#define DUMP1  \
   if (center[0] >= 0 && center[0] < g.grid[0]) { \
      key = center[0]; \
      MET__op_local_chase_list(&g, key, this_pt); \
   }

#define DUMP2  \
   if (center[0] >= 0 && center[0] < g.grid[0] && \
       center[1] >= 0 && center[1] < g.grid[1]) { \
      key = center[0] + g.grid_total[1] * center[1]; \
      MET__op_local_chase_list(&g, key, this_pt); \
   }

#define DUMP3  \
   if (center[0] >= 0 && center[0] < g.grid[0] && \
       center[1] >= 0 && center[1] < g.grid[1] && \
       center[2] >= 0 && center[2] < g.grid[2]) { \
      key = center[0] + g.grid_total[1] * center[1] + \
	 g.grid_total[2] * center[2]; \
      MET__op_local_chase_list(&g, key, this_pt); \
   }

   g.num_pairs = 0;
   src = g.src_base;
   for (i = 0; i < g.num_pts; i++) {
      for (k = 0; k < g.width; k++)
	 this_pt[k] = src[k];
      for (k = 0; k < g.dims; k++)
	 center[k] = (int) g.grid[k] *
	    (src[k] - g.min[k]) / (g.max[k] - g.min[k]);
      switch (g.dims) {
       case 1:
	 DUMP1;
	 center[0]++;
	 DUMP1;
	 center[0]--;
	 center[0]--;
	 DUMP1;
	 break;
       case 2:
	 DUMP2;
	 center[0]++;
	 DUMP2;
	 center[1]++;
	 DUMP2;
	 center[0]--;
	 DUMP2;
	 center[0]--;
	 DUMP2;
	 center[1]--;
	 DUMP2;
	 center[1]--;
	 DUMP2;
	 center[0]++;
	 DUMP2;
	 center[0]++;
	 DUMP2;
	 break;
       case 3:
	 center[2]--;
	 for (k = 0; k < 3; k++) {
	    DUMP3;
	    center[0]++;
	    DUMP3;
	    center[1]++;
	    DUMP3;
	    center[0]--;
	    DUMP3;
	    center[0]--;
	    DUMP3;
	    center[1]--;
	    DUMP3;
	    center[1]--;
	    DUMP3;
	    center[0]++;
	    DUMP3;
	    center[0]++;
	    DUMP3;
	    center[0]--;
	    center[1]++;
	    center[2]++;
	 }
	 break;
       default:
	 NYET;
      }
      indices[i] = g.num_pairs;
      src += g.src_width;
   }
   FREEN(g.buckets, int, g.num_buckets + 2 * g.num_pts);

   return MET__op_local_wrap_pairs(&g);
}

/*
 * take a point set and generate a data with
 * all pairs of pts that are within radius.
 * indices[i] will contain the first index
 * into pairs that came from pts[i].
 *
 * for now, just do the N^2 thang.
 */
#if 0
  The old code
static METdata *
MET__op_local_find_pairs(pts, radius, dims, indices)
   METblock	*pts;
   METreal	radius;
   int		dims;
   int		*indices;
{
   METdata	*pt_data = pts->data;
   METdata	*result;
   METverts	*verts;
   int		num_pts = DATA_NUM_VERTS(pt_data);
   int		src_width = pt_data->verts->total_cols;
   int		i, j, k, width = pt_data->num_cols;
   METreal	*src1, *src2, *dst;
   MME(MET__op_local_find_pairs);

   verts = MET_verts_new(MET_FIELD_REAL, num_pts * num_pts, width * 2);

   src1 = (METreal *) pt_data->verts->vertices;
   dst = (METreal *) verts->vertices;
   for (i = 0; i < num_pts; i++) {
      src2 = (METreal *) pt_data->verts->vertices;
      for (j = 0; j < num_pts; j++) {
	 for (k = 0; k < width; k++)
	    *dst++ = *src1++;
	 src1 -= width;
	 for (k = 0; k < width; k++)
	    *dst++ = *src2++;
	 src2 += src_width - width;
      }
      src1 += src_width;
   }

   result = MET_data_new(verts, width, 0);
   result->next = MET_data_new(verts, width, width);
   MET_VERTS_FREE(verts);

   for (i = 0; i < num_pts; i++)
      indices[i] = (i + 1) * num_pts;

   return result;
}
#endif

/*
 * create a new data where each row is the sum
 * of sum contiguous group of the rows in DATA.
 * how many is given by indices
 */
static METdata *
MET__op_local_reduce(data, indices, num_pts)
   METdata	*data;
   int		*indices;
   int		num_pts;
{
   METdata	*result;
   METverts	*verts;
   int		i, last, j, k, width;
   METreal	*src, *dst;
   MME(MET__op_local_reduce);

   if (MET_NULL_DATA == data)
      return MET_NULL_DATA;

   width = data->num_cols;
   verts = MET_verts_new(data->verts->type, num_pts, width);
   last = data->verts->num_verts;
   src = (METreal *) data->verts->vertices;
   dst = (METreal *) verts->vertices;
   j = 0;
   for (i = 0; i < last;) {
      for (k = 0; k < width; k++)
	 dst[k] = 0.0;
      while (i < indices[j]) {
	 for (k = 0; k < width; k++)
	    dst[k] += src[k];
	 src += data->verts->total_cols;
	 i++;
      }
      j++;
      dst += width;
   }
#ifdef MET__DEBUG
   if (j != num_pts)
      FATALS("j should be num_pts");
#endif

   result = MET_data_new(verts, width, 0);
   MET_VERTS_FREE(verts);
   result->next = MET__op_local_reduce(data->next, indices, num_pts);
   return result;
}

METblock *
MET__op_local_block(block, fn, radius, dims, loc_sym)
   METblock	*block;
   METset	*fn;
   METreal	radius;
   int		dims;
   METsym	*loc_sym;
{
   METdata	*pair_data, *post_reduce, *pts;
   METset	*pair_set, *pre_reduce;
   METtype	*pair_type;
   METblock	*result;
   int		*start_indices, num_pts, num_pairs;
   Boolean	dummy_bool;
   MME(MET__op_local_block);

   pts = block->data;

   if (pts->num_cols > MAX_DIMS) {
      ERROR(RNULL, "too many dims for local effect", SEV_FATAL);
      return MET_NULL_BLOCK;
   }
   
   num_pts = DATA_NUM_VERTS(pts);
   ALLOCNM(start_indices, int, num_pts);
   pair_data = MET__op_local_find_pairs(block, radius, dims,
					start_indices);
   num_pairs = DATA_NUM_VERTS(pair_data);

   if (getenv("LOCAL_EFFECT"))
      printf("local: %d objs and %g inter/obj\n",
	     num_pts,
	     num_pairs / (double) num_pts);

   pair_set = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_NEW_PAIR(pair_type, block->type->info.set_of.of,
		     block->type->info.set_of.of);
   MET_TYPE_COPY(pair_set->info.bare.type, pair_type);
   MET_TYPE_FREE(pair_type);
   MET_DATA_COPY(pair_set->info.bare.data, pair_data);
   pair_set->info.bare.how_many = num_pairs;
   MET_tags_initialize(&pair_set->info.bare.tags_bare);
   MET_tags_initialize(&pair_set->info.bare.tags_in);
   MET_DATA_FREE(pair_data);
   pre_reduce = MET_op_apply_to_set(fn, pair_set,
				    (METtype *) NULL, INULL,
				    loc_sym, &dummy_bool);
   MET_SET_FREE(pair_set);
   if (MET_NULL_SET == pre_reduce) {
      FREEN(start_indices, int, num_pts);
      return MET_NULL_BLOCK;
   }
#ifdef MET__DEBUG
   if (MET_SET_TYPE_BARE != pre_reduce->type) {
      FATALS("pre_reduce should be bare in local effect");
   }
#endif
   post_reduce = MET__op_local_reduce(pre_reduce->info.bare.data,
				      start_indices, num_pts);
   FREEN(start_indices, int, num_pts);
   MET_BLOCK_ALLOC(result);
   if (block->false_scalar) {
      printf("false scalar in localeffect\n");
      NYET;
   }
   result->false_scalar = NO;
   MET_data_append(post_reduce, pts);
   MET_DATA_COPY(result->data, post_reduce);
   MET_DATA_FREE(post_reduce);
   MET_TYPE_NEW_PAIR(pair_type, pre_reduce->info.bare.type,
		     block->type->info.set_of.of);
   MET_TYPE_NEW_SET_OF(result->type, pair_type);
   MET_TYPE_FREE(pair_type);
   MET_SET_FREE(pre_reduce);
   MET_CONNEX_COPY(result->connex, block->connex);
   MET_CONNEX_COOY(result->data_connex, block->data_connex);
   result->next = MET_NULL_BLOCK;

   return result;
}

static Boolean
MET__op_le_is_tensor(type)
   METtype	*type;
{
   MME(MET__op_le_is_tensor);

   if ((MET_TYPE_TYPE_FIELD == type->type &&
	MET_FIELD_REAL == type->info.field.type) ||
       (MET_TYPE_TYPE_VECTOR == type->type &&
	YES == MET__op_le_is_tensor(type->info.vector.of)))
      return YES;
   return NO;
}

METset *
MET_op_local_effect(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[LOCAL_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[LOCAL_NUM_KIDS];
   METtype	*arg_type[LOCAL_NUM_KIDS], *guess, *t1, *t2;
   METtype	*my_result_type = MET_NULL_TYPE;
   METblock	*block, *new_block;
   METreal	radius;
   int		dims;
   char		*buf;
   MME(MET_op_LOCAL);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, LOCAL_NUM_KIDS);
#endif

   MET_TYPE_NEW_FIELD(guess, MET_FIELD_REAL);
   MET_TYPE_NEW_VECTOR(t1, guess, TYPE_DIMENSION_UNKNOWN, NO);
   MET_TYPE_FREE(guess);
   MET_TYPE_NEW_SET_OF(guess, t1);
   /* keep t1 = R^? */

   arg_set[LOCAL_PTS] = MET_sym_be_set_ref(sym->kids[LOCAL_PTS], guess,
					   &arg_type[LOCAL_PTS],
					   &arg_set_type[LOCAL_PTS]);
   MET_TYPE_FREE(guess);

   MET_TYPE_NEW_PAIR(t2, t1, t1);
   MET_TYPE_FREE(t1);
   MET_TYPE_NEW_UNKNOWN(t1);
   MET_TYPE_NEW_MAP(guess, t2, t1);
   MET_TYPE_FREE(t1);
   MET_TYPE_FREE(t2);

   arg_set[LOCAL_FN] = MET_sym_be_set_ref(sym->kids[LOCAL_FN], guess,
					  &arg_type[LOCAL_FN],
					  &arg_set_type[LOCAL_FN]);
   MET_TYPE_FREE(guess);

   MET_TYPE_NEW_FIELD(guess, MET_FIELD_REAL);
   arg_set[LOCAL_RADIUS] = MET_sym_be_set_ref(sym->kids[LOCAL_RADIUS],
					      guess,
					      &arg_type[LOCAL_RADIUS],
					      &arg_set_type[LOCAL_RADIUS]);
   MET_TYPE_FREE(guess);

   MET_TYPE_NEW_FIELD(guess, MET_FIELD_INTEGER);
   arg_set[LOCAL_DIMS] = MET_sym_be_set_ref(sym->kids[LOCAL_DIMS],
					    guess,
					    &arg_type[LOCAL_DIMS],
					    &arg_set_type[LOCAL_DIMS]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[LOCAL_PTS] ||
       MET_NULL_TYPE == arg_type[LOCAL_FN] ||
       MET_NULL_TYPE == arg_type[LOCAL_DIMS] ||
       MET_NULL_TYPE == arg_type[LOCAL_RADIUS])
      goto bail2;
   
   if ((arg_type[LOCAL_PTS]->type != MET_TYPE_TYPE_SET_OF) ||
       NO == MET__op_le_is_tensor(arg_type[LOCAL_PTS]->info.set_of.of)) {
      MET_op_error_bad_arg_type(1, arg_type[LOCAL_PTS], "{R^?}", sym);
      goto bail1;
   }
   if (arg_type[LOCAL_FN]->type != MET_TYPE_TYPE_MAP) {
      MET_op_error_bad_arg_type(2, arg_type[LOCAL_FN], "? -> ?", sym);
      goto bail1;
   }
   if ((t1 = arg_type[LOCAL_RADIUS])->type != MET_TYPE_TYPE_FIELD ||
       t1->info.field.type != MET_FIELD_REAL) {
      MET_op_error_bad_arg_type(3, arg_type[LOCAL_RADIUS], "R", sym);
      goto bail1;
   }
   if ((t1 = arg_type[LOCAL_DIMS])->type != MET_TYPE_TYPE_FIELD ||
       t1->info.field.type != MET_FIELD_INTEGER) {
      MET_op_error_bad_arg_type(4, arg_type[LOCAL_DIMS], "Z", sym);
      goto bail1;
   }

   MET_TYPE_NEW_PAIR(t1, arg_type[LOCAL_PTS]->info.set_of.of,
		     arg_type[LOCAL_FN]->info.map.to);
   MET_TYPE_NEW_SET_OF(my_result_type, t1);
   MET_TYPE_FREE(t1);

   if (MET_op_type_only(arg_set, LOCAL_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
      goto bail2;

   radius = *DATA_FIRST_VERTEX(arg_set[LOCAL_RADIUS]->info.bare.data,
			       METreal);

   dims = (int) *DATA_FIRST_VERTEX(arg_set[LOCAL_DIMS]->info.bare.data,
				   METinteger);

   result_set = MET_set_new(MET_SET_TYPE_BLOCK);
   result_set->info.block.block_list = MET_NULL_BLOCK;
   block = arg_set[LOCAL_PTS]->info.block.block_list;
   while (MET_NULL_BLOCK != block) {
      new_block = MET__op_local_block(block, arg_set[LOCAL_FN],
				      radius, dims, sym);
      if (MET_NULL_BLOCK != new_block) {
	 MET_set_add_block(result_set, new_block);
	 MET_BLOCK_FREE(new_block);
      } else {
	 ERROR(sym->origin, "local effect, reduce failed", SEV_FATAL);
      }
      block = block->next;
   }
   MET_TYPE_FREE(my_result_type);

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(LOCAL_PTS);
   COND_FREE(LOCAL_FN);
   COND_FREE(LOCAL_RADIUS);
   COND_FREE(LOCAL_DIMS);

   return result_set;
}

#else PROGRAM_IMPLICIT

METset *
MET_op_local_effect(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   MME(MET_op_local_effect);
   CHECK_SYM(sym);

   if ((METtype **) NULL != result_type)
      MET_TYPE_NEW_UNKNOWN(*result_type);

   ERROR(sym->origin,
	 "local effect op not in this version",
	 SEV_FATAL);

   return MET_NULL_SET;
}

#endif PROGRAM_IMPLICIT
