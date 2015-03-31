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
   "$Id: op_cxfrag.c,v 1.9 1992/06/22 17:06:11 mas Exp $";
#endif

#include "defs.h"

#define CXFRAG_NUM_KIDS	1
#define CXFRAG_ARG	0

/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME, SRC1, DST1, DST2) \
static void \
NAME(arg1, res1, res2) \
   METdata	*arg1, *res1, *res2; \
{ \
   int		i; \
   SRC1		*src1; \
   DST1		*dst1; \
   DST2		*dst2; \
   int		src1_width, dst1_width, dst2_width; \
   MME(NAME); \
 \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   dst1_width = (1 == DATA_NUM_VERTS(res1)) ? 0 : res1->verts->total_cols; \
   dst2_width = (1 == DATA_NUM_VERTS(res2)) ? 0 : res2->verts->total_cols; \
 \
   src1 = DATA_FIRST_VERTEX(arg1, SRC1); \
   dst1 = DATA_FIRST_VERTEX(res1, DST1); \
   dst2 = DATA_FIRST_VERTEX(res2, DST2); \
   i = DATA_NUM_VERTS(arg1); \
    \
   while(i--) { \
      BODY; \
      dst1 += dst1_width; \
      dst2 += dst2_width; \
      src1 += src1_width; \
      } \
} \


#define BODY *dst1 = src1->real; *dst2 = src1->imaginary;
TEMPLATE(MET__op_cxfrag_guts_c, METcomplex, METreal, METreal)
#undef BODY


static Boolean
MET__op_cxfrag_guts(arg1, result)
   METdata	*arg1, *result;
{
   MME(MET__op_cxfrag_guts);

   MET__op_cxfrag_guts_c(arg1, result, result->next);

   return SUCCESS;
}

METset *
MET_op_complex_frag(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[CXFRAG_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[CXFRAG_NUM_KIDS];
   METtype	*arg_type[CXFRAG_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE, *t1, *t2;
   int		how_many;
   MME(MET_op_complex_frag);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, CXFRAG_NUM_KIDS);
#endif

   MET_TYPE_NEW_FIELD(guess, MET_FIELD_COMPLEX);
   arg_set[CXFRAG_ARG] =
      MET_sym_be_set_ref(sym->kids[CXFRAG_ARG], guess,
			 &arg_type[CXFRAG_ARG],
			 &arg_set_type[CXFRAG_ARG]);
   
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[CXFRAG_ARG])
      goto bail1;
   
   /*
    * make sure it is complex
    */
   if (MET_TYPE_TYPE_FIELD == arg_type[CXFRAG_ARG]->type &&
       MET_FIELD_COMPLEX == arg_type[CXFRAG_ARG]->info.field.type) {
      MET_TYPE_NEW_FIELD(t1, MET_FIELD_REAL);
      MET_TYPE_COPY(t2, t1);
      MET_TYPE_NEW_PAIR(my_result_type, t1, t2);
      MET_TYPE_FREE(t1);
      MET_TYPE_FREE(t2);
   }

   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "cxfrag, bad argument types", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, CXFRAG_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
       goto bail2;
   MET_op_reconcile_sets(arg_set, CXFRAG_NUM_KIDS);

   how_many = DATA_NUM_VERTS(arg_set[CXFRAG_ARG]->info.bare.data);
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, CXFRAG_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);
   
   if (FAILURE ==
       MET__op_cxfrag_guts(arg_set[CXFRAG_ARG]->info.bare.data,
			    result_set->info.bare.data)) {
      ERROR(sym->origin, "cxfrag, bad argument fields", SEV_FATAL);
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }
   
 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(CXFRAG_ARG);
   
   return result_set;
}
