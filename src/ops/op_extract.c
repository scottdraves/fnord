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
   "$Id: op_extract.c,v 1.14 1992/06/22 17:06:22 mas Exp $";
#endif

#include "defs.h"

#define EXTRACT_NUM_KIDS	2
#define EXTRACT_VEC		0
#define EXTRACT_INDEX		1

static int extract_poison;

/* BODY must be defined when this macro is invoked */
#define TEMPLATE(NAME, TYPE) \
static Boolean \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		i, j, max_index, length; \
   TYPE		*src1; \
   METinteger	*src2; \
   TYPE		*dst; \
   int		src1_width, src2_width, dst_width; \
   MME(NAME); \
   \
   length = result->num_cols; \
   max_index = arg1->num_cols / length; \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   /* be sure indices are all good */ \
   MAX_VERTS(i, arg1, arg2); \
   src2 = DATA_FIRST_VERTEX(arg2, METinteger); \
   while(i--) { \
      if (((int) *src2) > max_index || ((int) *src2) < 1) { \
	 extract_poison = (int) *src2; \
	 return FAILURE; \
	 } \
      src2 += src2_width; \
   } \
 \
   for (j = 0; j < length; j++) { \
      src1 = DATA_FIRST_VERTEX(arg1, TYPE) + j; \
      src2 = DATA_FIRST_VERTEX(arg2, METinteger); \
      dst  = DATA_FIRST_VERTEX(result, TYPE) + j; \
      MAX_VERTS(i, arg1, arg2); \
       \
      while(i--) { \
	 BODY; \
	 dst += dst_width; \
	 src1 += src1_width; \
	 src2 += src2_width; \
      } \
   } \
   return SUCCESS ; \
}

#define BODY *dst = *(src1 + (((int)*src2 - 1) * length))
TEMPLATE(MET__op_extract_guts_z2, METztwo)
TEMPLATE(MET__op_extract_guts_i, METinteger)
TEMPLATE(MET__op_extract_guts_r, METreal)
TEMPLATE(MET__op_extract_guts_c, METcomplex)
#undef BODY

#define BODY MET_SYM_COPY(*dst, *(src1 + (((int)*src2 - 1) * length)))
TEMPLATE(MET__op_extract_guts_m, METsym*)
#undef BODY

#define BODY MET_SET_COPY(*dst, *(src1 + (((int)*src2 - 1) * length)))
TEMPLATE(MET__op_extract_guts_s, METset*)
#undef BODY

static Boolean
MET__op_extract_guts(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   MME(MET__op_extract_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:
      return MET__op_extract_guts_z2(arg1, arg2, result);
    case MET_FIELD_INTEGER:
      return MET__op_extract_guts_i(arg1, arg2, result);
    case MET_FIELD_ANGLE: return FAILURE;
    case MET_FIELD_REAL:
      return MET__op_extract_guts_r(arg1, arg2, result);
    case MET_FIELD_COMPLEX:
      return MET__op_extract_guts_c(arg1, arg2, result);
    case MET_FIELD_QUATERNION: return FAILURE;
    case MET_FIELD_MAP:
      return MET__op_extract_guts_m(arg1, arg2, result);
    case MET_FIELD_SET:
      return MET__op_extract_guts_s(arg1, arg2, result);
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   /* make lint happy */
   return SUCCESS;
}

METset *
MET_op_extract(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[EXTRACT_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[EXTRACT_NUM_KIDS];
   METtype	*arg_type[EXTRACT_NUM_KIDS], *guess;
   METtype	*my_result_type = MET_NULL_TYPE;
   Boolean	is_row_extract;
   int		how_many, i, length, max_index;
   METdata	*ldata, *rdata;
   MME(MET_op_extract);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, EXTRACT_NUM_KIDS);
#endif
   is_row_extract = (sym->info.op.code == MET_SYM_OP_ROW_EXTRACT);

   MET_TYPE_NEW_UNKNOWN(guess);
   if (MET_NULL_TYPE != type) {
      MET_TYPE_FREE(guess);
      MET_TYPE_NEW_VECTOR(guess, type,
			  TYPE_DIMENSION_UNKNOWN,
			  is_row_extract);
   } else if (MET_NULL_TYPE != type &&
	      MET_TYPE_TYPE_SET == type->type) {
      MET_TYPE_FREE(guess);
      MET_TYPE_COPY(guess, type);
   } 
   arg_set[EXTRACT_VEC] =
      MET_sym_be_set_ref(sym->kids[EXTRACT_VEC], type,
			 &arg_type[EXTRACT_VEC],
			 &arg_set_type[EXTRACT_VEC]);
   MET_TYPE_FREE(guess);

   MET_TYPE_NEW_FIELD(guess, MET_FIELD_INTEGER);
   arg_set[EXTRACT_INDEX] =
      MET_sym_be_set_ref(sym->kids[EXTRACT_INDEX], type,
			 &arg_type[EXTRACT_INDEX],
			 &arg_set_type[EXTRACT_INDEX]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[EXTRACT_VEC] ||
       MET_NULL_TYPE == arg_type[EXTRACT_INDEX])
      goto bail1;
   
   if ((MET_NULL_SET != arg_set[EXTRACT_VEC] &&
	MET_SET_TYPE_TYPE == arg_set[EXTRACT_VEC]->type)) {
      COND_FREE(EXTRACT_VEC);
      COND_FREE(EXTRACT_INDEX);
      return MET_op_type_expr(sym, type, result_type, set_type);
   }

   if (MET_TYPE_TYPE_FIELD == arg_type[EXTRACT_INDEX]->type &&
       MET_TYPE_TYPE_FIELD == arg_type[EXTRACT_VEC]->type) {
      /*
       * if we have scalar ^ scalar, then do pow
       */
      COND_FREE(EXTRACT_VEC);
      COND_FREE(EXTRACT_INDEX);
      return MET_op_pow(sym, type, result_type, set_type);
   }

   /*
    * make sure left is a vector and right is an integer
    */
   if (MET_TYPE_TYPE_FIELD != arg_type[EXTRACT_INDEX]->type ||
       MET_FIELD_INTEGER != arg_type[EXTRACT_INDEX]->info.field.type) {
      MET_op_error_bad_arg_type(2, arg_type[EXTRACT_INDEX],
				"Z", sym);
      goto bail1;
   }
   if (MET_TYPE_TYPE_VECTOR != arg_type[EXTRACT_VEC]->type ||
       is_row_extract != arg_type[EXTRACT_VEC]->info.vector.is_row) {
      MET_op_error_bad_arg_type(1, arg_type[EXTRACT_VEC],
				is_row_extract ? "?_?" : "?^?", sym);
      goto bail1;
   }

   MET_TYPE_COPY(my_result_type, arg_type[EXTRACT_VEC]->info.vector.of);
   
   if (MET_op_type_only(arg_set, EXTRACT_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE))
       goto bail2;
   
   MET_op_reconcile_sets(arg_set, EXTRACT_NUM_KIDS);

   MET_DATA_COPY(ldata, arg_set[EXTRACT_VEC]->info.bare.data);
   MET_DATA_COPY(rdata, arg_set[EXTRACT_INDEX]->info.bare.data);
   /*
    * we can special case when we are extracting the same entry from all
    * the vectors and just create a new data that points to part of the
    * same verts.
    */
   if (1 == DATA_NUM_VERTS(rdata)) {
      result_set = MET_set_new(MET_SET_TYPE_BARE);
      result_set->info.bare.how_many = DATA_NUM_VERTS(ldata);
      MET_tags_initialize(&result_set->info.bare.tags_bare);
      MET_tags_initialize(&result_set->info.bare.tags_in);
      MET_tags_copy(&result_set->info.bare.tags_bare, 
		    &arg_set[EXTRACT_VEC]->info.bare.tags_bare);
      MET_tags_copy(&result_set->info.bare.tags_in, 
		    &arg_set[EXTRACT_VEC]->info.bare.tags_in);
      MET_TYPE_COPY(result_set->info.bare.type, my_result_type);
      i = (int) *DATA_FIRST_VERTEX(rdata, METinteger);
      max_index = arg_type[EXTRACT_VEC]->info.vector.dimension;
      length = ldata->num_cols / max_index;
      if (i > max_index || i < 1) {
	 char buf[100];
	 sprintf(buf, "%s, index out of range: %d",
		 OP_NAME(sym->info.op.code), i);
	 ERROR(sym->origin, buf, SEV_FATAL);

	 /* created to be killed */
	 result_set->info.bare.data = MET_data_new(ldata->verts, 1, 0);
	 MET_SET_FREE(result_set);
	 result_set = MET_NULL_SET;
      } else {
	 result_set->info.bare.data =
	    MET_data_new(ldata->verts,
			 length,
			 ldata->first_col + ((i - 1) * length));
      }
   } else {
      /*
       * otherwise do this like a generic op
       */
      MAX_VERTS(how_many, ldata, rdata);
      result_set = MET_type_to_polybare_set(my_result_type, how_many,
					    arg_set, EXTRACT_NUM_KIDS);
      
      if (FAILURE ==
	  MET__op_extract_guts(ldata, rdata,
			       result_set->info.bare.data)) {
	 char buf[100];
	 sprintf(buf, "%s, index out of range: %d",
		 OP_NAME(sym->info.op.code),
		 extract_poison);
	 ERROR(sym->origin, buf, SEV_FATAL);
	 MET_SET_FREE(result_set);
	 result_set = MET_NULL_SET;
      }
   }
   MET_TYPE_FREE(my_result_type);
   MET_DATA_FREE(ldata);
   MET_DATA_FREE(rdata);

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(EXTRACT_VEC);
   COND_FREE(EXTRACT_INDEX);
   
   return result_set;
}
