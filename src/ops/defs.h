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

/* $Id: defs.h,v 1.26 1992/06/22 17:05:42 mas Exp $ */

#include "ops/ops.h"

#define ANY_NUMBER_OF_KIDS (-1)

#define OP_PRIME_RULE(OP)	(MET_op_info_table[OP].prime_rule)
#define PRIME_RULE_NONE		0	/* can't do it */
#define PRIME_RULE_LINEAR	1	/* (f,g)' = (f',g') */
#define PRIME_RULE_PRODUCT	2	/* (f,g)' = (f',g) + (f,g') */
#define PRIME_RULE_CHAIN	3	/* (cos f)' = (-sin f) f' */
#define PRIME_RULE_SPECIAL	4	/* other */

/*
 * assumes standard locals arg_type and arg_set
 *
 * frees them if they are not NULL.
 */
#define COND_FREE(I) {		      \
   if (MET_NULL_TYPE != arg_type[I]) { \
      MET_TYPE_FREE(arg_type[I]);	\
      if (MET_NULL_SET != arg_set[I])	 \
	 MET_SET_FREE(arg_set[I]);	  \
   }					   \
}

#define MAX_VERTS(I,D1,D2) {         \
   (I) = DATA_NUM_VERTS(D1);          \
   (I) = MAX((I), DATA_NUM_VERTS(D2)); \
}
#define MAX_VERTS3(I,D1,D2,D3) {     \
   (I) = DATA_NUM_VERTS(D1);          \
   (I) = MAX((I), DATA_NUM_VERTS(D2)); \
   (I) = MAX((I), DATA_NUM_VERTS(D3));  \
}

/*
 * this is a hack
 */ /* Ack! */
#define OPS__CHECK_PARMS

#ifdef OPS__CHECK_PARMS
extern void MET_op_check_parms();
#endif
extern Boolean   MET_op_bare_only();
extern Boolean	 MET_op_block_only();
extern METset	*MET_op_polybare_to_parts();
extern METset	*MET_op_apply_to_elements();
extern METset	*MET_op_apply_to_set();
extern METset	*MET_op_type_expr();
extern Boolean   MET_op_type_only();
extern void	 MET_op_error_bad_arg_type();
extern Boolean   MET_op_guts_failure();
extern void      MET_op_reconcile_sets();
extern METset	*MET_op_guts_for_maps();
extern void	 MET_op_cant_cast();

extern METset
   *MET_op_if(),
   *MET_op_add(),
   *MET_op_cat(),
   *MET_op_opp(),
   *MET_op_div(),
   *MET_op_dot(),
   *MET_op_map(),
   *MET_op_pow(),
   *MET_op_card(),
   *MET_op_frag(),
   *MET_op_mult(),
   *MET_op_type(),
   *MET_op_enum(),
   *MET_op_poly(),
   *MET_op_svec(),
   *MET_op_assoc(),
   *MET_op_biint(),
   *MET_op_unint(),
   *MET_op_cross(),
   *MET_op_prime(),
   *MET_op_print(),
   *MET_op_quote(),
   *MET_op_relop(),
   *MET_op_setof(),
   *MET_op_shift(),
   *MET_op_trace(),
   *MET_op_union(),
   *MET_op_sample(),
   *MET_op_scalar(),
   *MET_op_unpair(),
   *MET_op_reduce(),
   *MET_op_bilogic(),
   *MET_op_extract(),
   *MET_op_unlogic(),
   *MET_op_implicit(),
   *MET_op_interval(),
   *MET_op_set_null(),
   *MET_op_triangle(),
   *MET_op_pair_build(),
   *MET_op_type_match(),
   *MET_op_apply_infer(),
   *MET_op_vector_build(),
   *MET_op_complex_frag(),
   *MET_op_complex_build(),
   *MET_op_type_and_cast(),
   *MET_op_type_unknown();
