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
   "$Id: op_comm.c,v 1.4 1992/06/22 17:06:02 mas Exp $";
#endif

#include "defs.h"

#define SHIFT_NUM_KIDS		2
#define SHIFT_LATTICE		0
#define SHIFT_OFFSET		1

static Boolean
MET__op_shift_guts(dst, src, ncoords, coords, offsets)
     METdata	*dst, *src;
     int	ncoords;
     int	*coords;
     METinteger	*offsets;
{
  int 		i, total_offset, t;
  MME(MET__op_shift_guts);

  /*
   * for now, don't deal with bdy conditions.  that means, we just
   * convert our offset vector into a 1-dimensional offset, and then
   * treat the lattice as a 1-dimensional torus (a ring).
   */
  total_offset = 0;
  t = 1;
  for (i = 0; i < ncoords; i++) {
    total_offset += offsets[i] * t;
    t *= coords[i];
  }

  if (total_offset > 0) {
    MET_verts_copy_vertices(dst->verts, total_offset, dst->first_col,
			    src->verts, 0, src->first_col,
			    src->num_cols, src->verts->num_verts - total_offset,
			    1, NO);
    MET_verts_copy_vertices(dst->verts, 0, dst->first_col,
			    src->verts, src->verts->num_verts -
			    total_offset, src->first_col,
			    src->num_cols, total_offset, 1, NO);
  } else {
    MET_verts_copy_vertices(dst->verts, 0, dst->first_col,
			    src->verts, -total_offset, src->first_col,
			    src->num_cols, src->verts->num_verts + total_offset,
			    1, NO);
    MET_verts_copy_vertices(dst->verts, src->verts->num_verts + total_offset,
			    dst->first_col,
			    src->verts, 0, src->first_col,
			    1, src->num_cols, -total_offset, NO);
  }


  if (MET_NULL_DATA != dst->next)
    return MET__op_shift_guts(dst->next, src->next,
			      ncoords, coords, offsets);
  else
    return SUCCESS;
}
  

METset *
MET_op_shift(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[SHIFT_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[SHIFT_NUM_KIDS];
   METtype	*arg_type[SHIFT_NUM_KIDS], *t;
   int		vlen, i;
   int		dims[20];
   METinteger	*offsets;
   METtag	*tags;
   Boolean	failed;
   MME(MET_op_shift);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, SHIFT_NUM_KIDS);
#endif
   arg_set[SHIFT_LATTICE] = MET_sym_be_set_ref(sym->kids[SHIFT_LATTICE], type,
					  &arg_type[SHIFT_LATTICE],
					  &arg_set_type[SHIFT_LATTICE]);

   arg_set[SHIFT_OFFSET] = MET_sym_be_set_ref(sym->kids[SHIFT_OFFSET], type,
					   &arg_type[SHIFT_OFFSET],
					   &arg_set_type[SHIFT_OFFSET]);

   if (MET_NULL_TYPE == arg_type[SHIFT_LATTICE] ||
       MET_NULL_TYPE == arg_type[SHIFT_OFFSET])
      goto bail1;
   
   if (MET_op_type_only(arg_set, SHIFT_NUM_KIDS,
			result_type, arg_type[SHIFT_LATTICE],
			set_type, MET_SET_TYPE_BARE))
      goto bail2;

   if (MET_TYPE_TYPE_VECTOR != (t = arg_type[SHIFT_OFFSET])->type ||
       MET_TYPE_TYPE_FIELD != (t = t->info.vector.of)->type ||
       MET_FIELD_INTEGER != t->info.field.type) {
     MET_op_error_bad_arg_type(2, arg_type[SHIFT_OFFSET], "Z^?", sym);
     goto bail1;
   }

   /* I don't think it makes sense for SHIFT_OFFSET to be anything but */
   /* scalar, but i feel like i should do this anyway */
   MET_op_reconcile_sets(arg_set, SHIFT_NUM_KIDS);

   if (MET_SET_TYPE_BARE != arg_set[SHIFT_LATTICE]->type ||
       0 == arg_set[SHIFT_LATTICE]->info.bare.tags_bare.num_tags) {
     ERROR(sym->origin, "shift, first argument must be mapping over a set",
	   SEV_FATAL);
     goto bail1;
   }

   if (MET_SET_TYPE_BARE != arg_set[SHIFT_OFFSET]->type ||
       1 != arg_set[SHIFT_OFFSET]->info.bare.how_many) {
     ERROR(sym->origin,
	   "shift, second argument must not be mapping over a set",
	   SEV_FATAL);
     goto bail1;
   }

   vlen = 0;
   tags = arg_set[SHIFT_LATTICE]->info.bare.tags_bare.tags;
   for (i = 0; i < arg_set[SHIFT_LATTICE]->info.bare.tags_bare.num_tags; i++) {
     int j;
     for (j = 0; j < tags[i].connex->ndims; j++) {
       dims[vlen + j] = tags[i].connex->dims[j];
       if (tags[i].connex->types[j] != MET_CONNEX_TYPE_MESH) {
	 ERROR(sym->origin,
	       "shift, can't handle non-mesh connectivity",
	       SEV_FATAL);
	 goto bail1;
       }
     }
     vlen += tags[i].connex->ndims;
   }
   if (vlen != arg_type[SHIFT_OFFSET]->info.vector.dimension) {
     char buf[200];
     sprintf(buf,
	     "shift, offset vector is wrong length.  need Z^%d, got Z^%d.",
	     vlen, arg_type[SHIFT_OFFSET]->info.vector.dimension);
     ERROR(sym->origin, buf, SEV_FATAL);
     goto bail1;
   }

   offsets = DATA_FIRST_VERTEX(arg_set[SHIFT_OFFSET]->info.bare.data,
			       METinteger);

   /* check for all-zero offset vector -- then we are identity */
   for (i = 0; i < vlen; i++)
     if ((int) offsets[i] != 0)
       break;

   if (i == vlen) {
     MET_SET_COPY(result_set, arg_set[SHIFT_LATTICE]);
   } else {
     result_set =
       MET_type_to_polybare_set(arg_type[SHIFT_LATTICE],
				arg_set[SHIFT_LATTICE]->info.bare.how_many,
				arg_set, SHIFT_NUM_KIDS);
     
     failed = MET__op_shift_guts(result_set->info.bare.data,
				 arg_set[SHIFT_LATTICE]->info.bare.data,
				 vlen, dims, offsets);
     
     if (FAILURE == failed) {
       ERROR(sym->origin, "shift, failure??", SEV_FATAL);
       MET_SET_FREE(result_set);
       result_set = MET_NULL_SET;
     }
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(SHIFT_LATTICE);
   COND_FREE(SHIFT_OFFSET);

   return result_set;
}
