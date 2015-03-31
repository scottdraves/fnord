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
   "$Id: op_mult.c,v 1.10 1992/06/22 17:06:39 mas Exp $";
#endif

#include "defs.h"

#define MULT_NUM_KIDS		2
#define MULT_LEFT		0
#define MULT_RIGHT		1


/*
 * used by scalar-scalar and scalar-thing.  src1 must be scalar
 */
#define TEMPLATE(NAME, TYPE1, TYPE2, DST_TYPE) \
static Boolean \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		i, j, length; \
   TYPE1	*src1; \
   TYPE2	*src2; \
   DST_TYPE	*dst; \
   int		src1_width, src2_width, dst_width; \
   TEMPS \
   MME(NAME); \
 \
   length = result->num_cols; \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   for (j = 0; j < length; j++) { \
      src1 = DATA_FIRST_VERTEX(arg1, TYPE1); \
      src2 = DATA_FIRST_VERTEX(arg2, TYPE2) + j; \
      dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + j; \
      MAX_VERTS(i, arg1, arg2); \
      UNROLL4(i, BODY; \
	 dst += dst_width; \
	 src1 += src1_width; \
	 src2 += src2_width;); \
   } \
   return SUCCESS; \
}

#define TEMPS
#define BODY *dst = (METinteger) ((int)*src1 * (int)*src2)
TEMPLATE(MET__op_mult_guts_scalar_i_i, METinteger, METinteger, METinteger)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst = (METreal) ((Real)*src1 * (Real)*src2);
TEMPLATE(MET__op_mult_guts_scalar_i_r, METinteger, METreal, METreal)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst = (METreal) ((Real)*src1 * (Real)*src2);
TEMPLATE(MET__op_mult_guts_scalar_r_i, METreal, METinteger, METreal)
#undef BODY
#undef TEMPS

#define TEMPS Real t;
#define BODY dst->real = (METreal) ((t = (Real)*src1) * (Real)src2->real); \
	     dst->imaginary = (METreal) (t * (Real)src2->imaginary);
TEMPLATE(MET__op_mult_guts_scalar_i_c, METinteger, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS Real t;
#define BODY dst->real = (METreal) ((t = (Real)*src2) * (Real)src1->real); \
	     dst->imaginary = (METreal) (t * (Real)src1->imaginary);
TEMPLATE(MET__op_mult_guts_scalar_c_i, METcomplex, METinteger, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS Real t;
#define BODY dst->real = (METreal) ((t = (Real)*src1) * (Real)src2->real); \
	     dst->imaginary = (METreal) (t * (Real)src2->imaginary);
TEMPLATE(MET__op_mult_guts_scalar_r_c, METreal, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS Real t;
#define BODY dst->real = (METreal) ((t = (Real)*src2) * (Real)src1->real); \
	     dst->imaginary = (METreal) (t * (Real)src1->imaginary);
TEMPLATE(MET__op_mult_guts_scalar_c_r, METcomplex, METreal, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst = (METreal) ((Real)*src1 * (Real)*src2);
TEMPLATE(MET__op_mult_guts_scalar_r_r, METreal, METreal, METreal)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY dst->real = (METreal) (src1->real * src2->real -        \
			       src1->imaginary * src2->imaginary) ;  \
	dst->imaginary = (METreal) (src1->real * src2->imaginary +   \
			       src1->imaginary * src2->real) ;
TEMPLATE(MET__op_mult_guts_scalar_c_c, METcomplex, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

/* arg1 is scalar */
static Boolean
MET__op_mult_guts_scalar(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   int		a2t = arg2->verts->type;
   Boolean	(*f)() = MET_op_guts_failure;
   MME(MET__op_mult_guts_scalar);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:
      break;
    case MET_FIELD_INTEGER:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_mult_guts_scalar_i_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_mult_guts_scalar_i_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_mult_guts_scalar_i_c;
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_ANGLE:
      break;
    case MET_FIELD_REAL:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_mult_guts_scalar_r_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_mult_guts_scalar_r_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_mult_guts_scalar_r_c;
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_COMPLEX:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_mult_guts_scalar_c_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_mult_guts_scalar_c_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_mult_guts_scalar_c_c;
	 break;
       case MET_FIELD_MAP:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_MAP:
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:
      break;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }
   
   if (SUCCESS == (f)(arg1, arg2, result)) {
      if (MET_NULL_DATA != result->next)
	 return MET__op_mult_guts_scalar(arg1, arg2->next, result->next);
      else
	 return SUCCESS;
   } else
      return FAILURE;
}


/*
 * used by matrix_vector.  arg1 is matrix, arg2 is vector
 */
#undef  TEMPLATE
#define MV_TEMPLATE(NAME, TYPE1, TYPE2, DST_TYPE) \
static Boolean \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		i, j, k; \
   int		max_verts, num_rows, num_cols; \
   TYPE1	*src1; \
   TYPE2	*src2; \
   DST_TYPE	*dst; \
   int		src1_width, src2_width, dst_width; \
   TEMPS \
   MME(NAME); \
 \
   num_cols = arg2->num_cols; \
   num_rows = result->num_cols; \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
   MAX_VERTS(max_verts, arg1, arg2); \
 \
   for (k = 0; k < num_rows; k++) { \
      dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + k; \
      UNROLL4(max_verts, INIT;  dst += dst_width;);\
      for (j = 0; j < num_cols; j++) { \
	 src1 = DATA_FIRST_VERTEX(arg1, TYPE1) + (k * num_cols) + j; \
	 src2 = DATA_FIRST_VERTEX(arg2, TYPE2) + j; \
	 dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + k; \
	 i = max_verts; \
	 UNROLL4(i, BODY; \
	    dst += dst_width; \
	    src1 += src1_width; \
	    src2 += src2_width;); \
      } \
   } \
   return SUCCESS; \
}


/*
 * used by matrix_matrix.
 */
#undef  TEMPLATE
#define MM_TEMPLATE(NAME, TYPE1, TYPE2, DST_TYPE) \
static Boolean \
NAME(arg1, arg2, result) \
   METdata	*arg1, *arg2, *result; \
{ \
   int		j, k, l; \
   int		max_verts, lrows, rcols, shared_size; \
   TYPE1	*src1; \
   TYPE2	*src2; \
   DST_TYPE	*dst; \
   int		src1_width, src2_width, dst_width; \
   TEMPS \
   MME(NAME); \
 \
   shared_size = (int) ANINT(SQRT((arg1->num_cols * arg2->num_cols) / \
			           result->num_cols)); \
   lrows = arg1->num_cols / shared_size; \
   rcols = arg2->num_cols / shared_size; \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
   MAX_VERTS(max_verts, arg1, arg2); \
 \
   for (l = 0; l < lrows; l++) { \
      for (k = 0; k < rcols; k++) { \
	 dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + (l * rcols) + k; \
	 UNROLL4(max_verts, INIT; dst += dst_width;); \
	 for (j = 0; j < shared_size; j++) { \
	    src1 = DATA_FIRST_VERTEX(arg1, TYPE1) + (l * shared_size) + j; \
	    src2 = DATA_FIRST_VERTEX(arg2, TYPE2) + (j * rcols) + k; \
	    dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + (l * rcols) + k; \
	    UNROLL4(max_verts, BODY; \
	       dst += dst_width; \
	       src1 += src1_width; \
	       src2 += src2_width;); \
	 } \
      } \
   } \
   return SUCCESS; \
}

/*
 * matrix-matrix and matrix-vector both use the multiply-accumulate
 * BODY, so share the definitions
 */
#define INIT *dst = (int) 0;
#define TEMPS
#define BODY *dst += (METinteger) ((int)*src1 * (int)*src2)
MV_TEMPLATE(MET__op_mult_guts_matrix_vector_i_i, METinteger, METinteger, METinteger)
MM_TEMPLATE(MET__op_mult_guts_matrix_matrix_i_i, METinteger, METinteger, METinteger)
#undef BODY
#undef TEMPS
#undef INIT

#define INIT *dst = (Real) 0.0;
#define TEMPS
#define BODY *dst += (METreal) ((Real)*src1 * (Real)*src2);
MV_TEMPLATE(MET__op_mult_guts_matrix_vector_i_r, METinteger, METreal, METreal)
MM_TEMPLATE(MET__op_mult_guts_matrix_matrix_i_r, METinteger, METreal, METreal)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst += (METreal) ((Real)*src1 * (Real)*src2);
MV_TEMPLATE(MET__op_mult_guts_matrix_vector_r_i, METreal, METinteger, METreal)
MM_TEMPLATE(MET__op_mult_guts_matrix_matrix_r_i, METreal, METinteger, METreal)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY *dst += (METreal) ((Real)*src1 * (Real)*src2);
MV_TEMPLATE(MET__op_mult_guts_matrix_vector_r_r, METreal, METreal, METreal)
MM_TEMPLATE(MET__op_mult_guts_matrix_matrix_r_r, METreal, METreal, METreal)
#undef BODY
#undef TEMPS
#undef INIT

#define INIT dst->real = (Real) 0.0; dst->imaginary = (Real) 0.0;
#define TEMPS Real t;
#define BODY dst->real += (METreal) ((t = (Real)*src1) * (Real)src2->real); \
	     dst->imaginary += (METreal) (t * (Real)src2->imaginary);
MV_TEMPLATE(MET__op_mult_guts_matrix_vector_i_c, METinteger, METcomplex, METcomplex)
MM_TEMPLATE(MET__op_mult_guts_matrix_matrix_i_c, METinteger, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS Real t;
#define BODY dst->real += (METreal) ((t = (Real)*src2) * (Real)src1->real); \
	     dst->imaginary += (METreal) (t * (Real)src1->imaginary);
MV_TEMPLATE(MET__op_mult_guts_matrix_vector_c_i, METcomplex, METinteger, METcomplex)
MM_TEMPLATE(MET__op_mult_guts_matrix_matrix_c_i, METcomplex, METinteger, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS Real t;
#define BODY dst->real += (METreal) ((t = (Real)*src1) * (Real)src2->real); \
	     dst->imaginary += (METreal) (t * (Real)src2->imaginary);
MV_TEMPLATE(MET__op_mult_guts_matrix_vector_r_c, METreal, METcomplex, METcomplex)
MM_TEMPLATE(MET__op_mult_guts_matrix_matrix_r_c, METreal, METcomplex, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS Real t;
#define BODY dst->real += (METreal) ((t = (Real)*src2) * (Real)src1->real); \
	     dst->imaginary += (METreal) (t * (Real)src1->imaginary);
MV_TEMPLATE(MET__op_mult_guts_matrix_vector_c_r, METcomplex, METreal, METcomplex)
MM_TEMPLATE(MET__op_mult_guts_matrix_matrix_c_r, METcomplex, METreal, METcomplex)
#undef BODY
#undef TEMPS

#define TEMPS
#define BODY dst->real += (METreal) (src1->real * src2->real -        \
			        src1->imaginary * src2->imaginary) ;  \
	dst->imaginary += (METreal) (src1->real * src2->imaginary +   \
			        src1->imaginary * src2->real) ;
MV_TEMPLATE(MET__op_mult_guts_matrix_vector_c_c, METcomplex, METcomplex, METcomplex)
MM_TEMPLATE(MET__op_mult_guts_matrix_matrix_c_c, METcomplex, METcomplex, METcomplex)
#undef BODY
#undef TEMPS
#undef INIT

static void 
MET__op_mult_guts_map_acc(src1, field, src2, dst)
   METnumber	*src1;
   int		field;
   METmap	*src2;
   METmap	**dst;
{
   METset	*nset;
   METtype	*type;
   METsym	*sym, *next;
   MME(MET__op_mult_map_guts_acc);

   if (MET_NULL_TYPE != (*dst)->loose_from) {
      (void) MET_type_cast((*dst)->loose_from, src2->loose_from,
			   &type, TYPE_CAST_TAKE_SECOND);
      if (type != (*dst)->loose_from) {
	 MET_TYPE_FREE((*dst)->loose_from);
	 MET_TYPE_COPY((*dst)->loose_from, type);
      } 
      MET_TYPE_FREE(type);
   } else
      MET_TYPE_COPY((*dst)->loose_from, src2->loose_from);
   sym = METsym_new_op(MET_SYM_OP_MULTIPLY, src2->sym->origin);
   METsym_add_kid(sym, src2->sym);
   if (field == MET_FIELD_MAP) {
      METsym_add_kid(sym, src1->map.sym);
   } else {
      nset = METset_new_number(field, src1);
      next = METsym_new_set_ref(nset, src2->sym->origin);
      MET_SET_FREE(nset);
      METsym_add_kid(sym, next);
      MET_SYM_FREE(next);
   }
   if (MET_NULL_SYM != (*dst)->sym) {
      next = METsym_new_op(MET_SYM_OP_ADD,
			   src2->sym->origin);
      METsym_add_kid(next, sym);
      METsym_add_kid(next, (*dst)->sym);
      MET_SYM_FREE((*dst)->sym);
   } else
      MET_SYM_COPY(next, sym);
   MET_SYM_COPY((*dst)->sym, next);
   MET_SYM_FREE(sym);
   MET_SYM_FREE(next);
}

static Boolean
MET__op_mult_guts_mv_map(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   int 		i, j, k;
   int 		max_verts, num_rows, num_cols;
   METnumber	*src1;
   METnumber 	*src2;
   METmap	*dst;
   int		src1_width, src2_width, dst_width;
   int		type1, type2;
   Boolean      first_is_map;
   MME(MET__op_mult_guts_mv_map);

   num_cols = arg2->num_cols;
   num_rows = result->num_cols;

   type1 = arg1->verts->type;
   type2 = arg2->verts->type;
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols;
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols;
   src1_width *= MET_field_size[type1];
   src2_width *= MET_field_size[type2];

   dst_width = result->verts->total_cols;
   MAX_VERTS(max_verts, arg1, arg2);
   first_is_map = (MET_FIELD_MAP == arg1->verts->type);

   for (k = 0; k < num_rows; k++) {
      dst = DATA_FIRST_VERTEX(result, METmap) + k;
      i = max_verts;
      do {
	 dst->loose_from = MET_NULL_TYPE;
	 dst->env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV);
	 dst->sym = MET_NULL_SYM;
	 dst += dst_width;
      } while (--i);

      for (j = 0; j < num_cols; j++) {
	 src1 = CAST(METnumber *, CAST(char *, arg1->verts->vertices) +
		     MET_field_size[type1] *
		     ((k * num_cols) + j + arg1->first_col));
	 src2 = CAST(METnumber *, CAST(char *, arg2->verts->vertices) +
		     MET_field_size[type2] * (j + arg2->first_col));
	 dst = DATA_FIRST_VERTEX(result, METmap) + k;
	 i = max_verts;
	 if (first_is_map) do {
	    MET__op_mult_guts_map_acc(src2, type2, (METmap *) src1, &dst);
	    dst += dst_width;
	    src1 = CAST(METnumber *, CAST(char *, src1) + src1_width);
	    src2 = CAST(METnumber *, CAST(char *, src2) + src2_width);
	 } while (--i);
	 else do {
	    MET__op_mult_guts_map_acc(src1, type1, (METmap *) src2, &dst);
	    dst += dst_width;
	    src1 = CAST(METnumber *, CAST(char *, src1) + src1_width);
	    src2 = CAST(METnumber *, CAST(char *, src2) + src2_width);
	 } while (--i);
      }
   }
   return SUCCESS;
}
	    
static Boolean
MET__op_mult_guts_mm_map(arg1, arg2, result)
   METdata 	*arg1, *arg2, *result;
{
   int 		i, j, k, l;
   int		max_verts, lrows, rcols, shared_size;
   METnumber	*src1;
   METnumber	*src2;
   METmap	*dst;
   int 		src1_width, src2_width, dst_width;
   int		type1, type2;
   Boolean	first_is_map;
   MME(MET__op_mult_guts_mm_map);

   shared_size = (int) ANINT(SQRT((arg1->num_cols * arg2->num_cols) / 
				  result->num_cols));
   lrows = arg1->num_cols / shared_size;
   rcols = arg2->num_cols / shared_size;

   type1 = arg1->verts->type;
   type2 = arg2->verts->type;
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols;
   src2_width = (1 == DATA_NUM_VERTS(arg2)) ? 0 : arg2->verts->total_cols;
   src1_width *= MET_field_size[type1];
   src2_width *= MET_field_size[type2];

   dst_width = result->verts->total_cols;
   MAX_VERTS(max_verts, arg1, arg2);
   first_is_map = (MET_FIELD_MAP == arg1->verts->type);

   for (l = 0; l < lrows; l++) {
      for (k = 0; k < rcols; k++) {
	 i = max_verts;
	 dst = DATA_FIRST_VERTEX(result, METmap) + (l * rcols) + k;
	 do {
	    dst->loose_from = MET_NULL_TYPE;
	    dst->env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV);
	    dst->sym = MET_NULL_SYM;
	    dst += dst_width;
	 } while (--i);
	 for (j = 0; j < shared_size; j++) {
	    dst = DATA_FIRST_VERTEX(result, METmap) + l * rcols + k;
	    src1 = CAST(METnumber *, CAST(char *, arg1->verts->vertices) + 
			MET_field_size[type1] *
			((l * shared_size) + j + arg1->first_col));
	    src2 = CAST(METnumber *, CAST(char *, arg2->verts->vertices) + 
			MET_field_size[type2] *
			((j * rcols) + k + arg2->first_col));
	    i = max_verts;
	    if (first_is_map) do {
	       MET__op_mult_guts_map_acc(src2, type2, (METmap *) src1, &dst);
	       dst += dst_width;
	       src1 = CAST(METnumber *, CAST(char *, src1) + src1_width);
	       src2 = CAST(METnumber *, CAST(char *, src2) + src2_width);
	    } while (--i);
	    else do {
	       MET__op_mult_guts_map_acc(src1, type1, (METmap *) src2, &dst);
	       dst += dst_width;
	       src1 = CAST(METnumber *, CAST(char *, src1) + src1_width);
	       src2 = CAST(METnumber *, CAST(char *, src2) + src2_width);
	    } while (--i);
	 }
      }
   }
   return SUCCESS;
}

/* arg1 is matrix, arg2 is vector */
static Boolean
MET__op_mult_guts_matrix_vector(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   int		a2t = arg2->verts->type;
   Boolean	(*f)() = MET_op_guts_failure;
   MME(MET__op_mult_guts_matrix_vector);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:
      break;
    case MET_FIELD_INTEGER:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_mult_guts_matrix_vector_i_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_mult_guts_matrix_vector_i_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_mult_guts_matrix_vector_i_c;
	 break;
       case MET_FIELD_MAP:
	 f = MET__op_mult_guts_mv_map;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_ANGLE:
      break;
    case MET_FIELD_REAL:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_mult_guts_matrix_vector_r_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_mult_guts_matrix_vector_r_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_mult_guts_matrix_vector_r_c;
	 break;
       case MET_FIELD_MAP:
	 f = MET__op_mult_guts_mv_map;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_COMPLEX:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_mult_guts_matrix_vector_c_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_mult_guts_matrix_vector_c_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_mult_guts_matrix_vector_c_c;
	 break;
       case MET_FIELD_MAP:
	 f = MET__op_mult_guts_mv_map;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_MAP:
      switch (a2t) {
       case MET_FIELD_ZTWO:
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_INTEGER:
       case MET_FIELD_REAL:
       case MET_FIELD_COMPLEX:
       case MET_FIELD_MAP:
	 f = MET__op_mult_guts_mv_map;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:
      break;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   return (f)(arg1, arg2, result);
}


/* arg1 is matrix, arg2 is vector */
static Boolean
MET__op_mult_guts_matrix_matrix(arg1, arg2, result)
   METdata	*arg1, *arg2, *result;
{
   int		a2t = arg2->verts->type;
   Boolean	(*f)() = MET_op_guts_failure;
   MME(MET__op_mult_guts_matrix_matrix);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:
      break;
    case MET_FIELD_INTEGER:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_mult_guts_matrix_matrix_i_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_mult_guts_matrix_matrix_i_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_mult_guts_matrix_matrix_i_c;
	 break;
       case MET_FIELD_MAP:
	 f = MET__op_mult_guts_mm_map;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_ANGLE:
      break;
    case MET_FIELD_REAL:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_mult_guts_matrix_matrix_r_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_mult_guts_matrix_matrix_r_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_mult_guts_matrix_matrix_r_c;
	 break;
       case MET_FIELD_MAP:
	 f = MET__op_mult_guts_mm_map;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_COMPLEX:
      switch (a2t) {
       case MET_FIELD_ZTWO:
	 break;
       case MET_FIELD_INTEGER:
	 f = MET__op_mult_guts_matrix_matrix_c_i;
	 break;
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_REAL:
	 f = MET__op_mult_guts_matrix_matrix_c_r;
	 break;
       case MET_FIELD_COMPLEX:
	 f = MET__op_mult_guts_matrix_matrix_c_c;
	 break;
       case MET_FIELD_MAP:
	 f = MET__op_mult_guts_mm_map;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_MAP:
      switch (a2t) {
       case MET_FIELD_ZTWO:
       case MET_FIELD_ANGLE:
	 break;
       case MET_FIELD_INTEGER:
       case MET_FIELD_REAL:
       case MET_FIELD_COMPLEX:
       case MET_FIELD_MAP:
	 f = MET__op_mult_guts_mm_map;
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_SET:
	 break;
       default:
	 DEFAULT_ERR(a2t);
      }
      break;
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:
      break;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   return (f)(arg1, arg2, result);
}

#define THING_SCALAR	0
#define SCALAR_THING	1
#define MATRIX_VECTOR	2
#define MATRIX_MATRIX	3

METset *
MET_op_mult(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[MULT_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[MULT_NUM_KIDS];
   METtype	*arg_type[MULT_NUM_KIDS];
   METtype	*my_result_type = MET_NULL_TYPE, *guess, *t1, *t2, *s;
   int		how_many, sub_op;
   Boolean	result;
   Boolean	over_maps = NO;
   Boolean	over_sets = NO;
   MME(MET_op_mult);

#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, MULT_NUM_KIDS);
#endif

   MET_TYPE_NEW_UNKNOWN(guess);
   arg_set[MULT_LEFT] =
      MET_sym_be_set_ref(sym->kids[MULT_LEFT], guess,
			 &arg_type[MULT_LEFT],
			 &arg_set_type[MULT_LEFT]);

   arg_set[MULT_RIGHT] =
      MET_sym_be_set_ref(sym->kids[MULT_RIGHT], guess,
			 &arg_type[MULT_RIGHT],
			 &arg_set_type[MULT_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE == arg_type[MULT_LEFT] ||
       MET_NULL_TYPE == arg_type[MULT_RIGHT])
      goto bail1;
   
   /* get rid of a layer of sets, if necessary */
   
   if (arg_type[MULT_LEFT]->type == MET_TYPE_TYPE_SET_OF) {
      MET_TYPE_COPY(guess, arg_type[MULT_LEFT]->info.set_of.of);
      MET_TYPE_FREE(arg_type[MULT_LEFT]);
      MET_TYPE_COPY(arg_type[MULT_LEFT], guess);
      MET_TYPE_FREE(guess);
      over_sets = YES;
   }
   if (arg_type[MULT_RIGHT]->type == MET_TYPE_TYPE_SET_OF) {
      MET_TYPE_COPY(guess, arg_type[MULT_RIGHT]->info.set_of.of);
      MET_TYPE_FREE(arg_type[MULT_RIGHT]);
      MET_TYPE_COPY(arg_type[MULT_RIGHT], guess);
      MET_TYPE_FREE(guess);
      over_sets = YES;
   }

   if (arg_type[MULT_LEFT]->type == MET_TYPE_TYPE_FIELD) {
      sub_op = SCALAR_THING;
      my_result_type =
	 MET_type_cast_to_field(arg_type[MULT_RIGHT],
				arg_type[MULT_LEFT]->info.field.type);
   } else if (arg_type[MULT_RIGHT]->type == MET_TYPE_TYPE_FIELD) {
      sub_op = THING_SCALAR;
      my_result_type =
	 MET_type_cast_to_field(arg_type[MULT_LEFT],
				arg_type[MULT_RIGHT]->info.field.type);
   } else if (arg_type[MULT_LEFT]->type == MET_TYPE_TYPE_MAP) {
      sub_op = SCALAR_THING;
      over_maps = YES;
      my_result_type = 
	 MET_type_cast_maps(arg_type[MULT_RIGHT], arg_type[MULT_LEFT]);
   } else if (arg_type[MULT_RIGHT]->type == MET_TYPE_TYPE_MAP) {
      sub_op = THING_SCALAR;
      over_maps = YES;
      my_result_type =
	 MET_type_cast_maps(arg_type[MULT_LEFT], arg_type[MULT_RIGHT]);
   } else if ((t1 = arg_type[MULT_LEFT])->type == MET_TYPE_TYPE_VECTOR &&
	      (t1 = t1->info.vector.of)->type == MET_TYPE_TYPE_VECTOR &&
	      ((t1 = t1->info.vector.of)->type == MET_TYPE_TYPE_FIELD ||
	       t1->type == MET_TYPE_TYPE_MAP)  &&
	      (t2 = arg_type[MULT_RIGHT])->type == MET_TYPE_TYPE_VECTOR &&
	      ((t2 = t2->info.vector.of)->type == MET_TYPE_TYPE_FIELD ||
	       t2->type == MET_TYPE_TYPE_MAP) &&
	      arg_type[MULT_LEFT]->info.vector.of->info.vector.dimension ==
	      arg_type[MULT_RIGHT]->info.vector.dimension) {
      sub_op = MATRIX_VECTOR;
      if (t1->type == MET_TYPE_TYPE_MAP || t2->type == MET_TYPE_TYPE_MAP) 
	 over_maps = YES;
      if (t1->type == MET_TYPE_TYPE_FIELD && YES == over_maps)
	 s = MET_type_cast_to_field(t2, t1->info.field.type);
      else if (t2->type == MET_TYPE_TYPE_FIELD && YES == over_maps)
	 s = MET_type_cast_to_field(t1, t2->info.field.type);
      else if (FAILURE == MET_type_cast(t1, t2, &s, TYPE_CAST_MAKE_BOGUS)){
	 MET_TYPE_FREE(s);
	 s = MET_NULL_TYPE;
      }
      if (MET_NULL_TYPE == s)
	 goto escape;
      MET_TYPE_NEW_VECTOR(my_result_type, s,
			  arg_type[MULT_LEFT]->info.vector.dimension, 
			  arg_type[MULT_RIGHT]->info.vector.is_row);
      MET_TYPE_FREE(s);
   } else if ((t1 = arg_type[MULT_LEFT])->type == MET_TYPE_TYPE_VECTOR &&
	      (t1 = t1->info.vector.of)->type == MET_TYPE_TYPE_VECTOR &&
	      ((t1 = t1->info.vector.of)->type == MET_TYPE_TYPE_FIELD ||
	       t1->type == MET_TYPE_TYPE_MAP) &&
	      (t2 = arg_type[MULT_RIGHT])->type == MET_TYPE_TYPE_VECTOR &&
	      (t2 = t2->info.vector.of)->type == MET_TYPE_TYPE_VECTOR &&
	      ((t2 = t2->info.vector.of)->type == MET_TYPE_TYPE_FIELD ||
	       t2->type == MET_TYPE_TYPE_MAP) &&
	      arg_type[MULT_LEFT]->info.vector.of->info.vector.dimension ==
	      arg_type[MULT_RIGHT]->info.vector.dimension) {
      sub_op = MATRIX_MATRIX;
      if (t1->type == MET_TYPE_TYPE_MAP || t2->type == MET_TYPE_TYPE_MAP) 
	 over_maps = YES;
      if (t1->type == MET_TYPE_TYPE_FIELD && YES == over_maps)
	 s = MET_type_cast_to_field(t2, t1->info.field.type);
      else if (t2->type == MET_TYPE_TYPE_FIELD && YES == over_maps)
	 s = MET_type_cast_to_field(t1, t2->info.field.type);
      else if (FAILURE == MET_type_cast(t1, t2, &s, TYPE_CAST_MAKE_BOGUS)){
	 MET_TYPE_FREE(s);
	 s = MET_NULL_TYPE;
      }
      if (MET_NULL_TYPE == s)
	 goto escape;
      MET_TYPE_NEW_VECTOR(t1, s, arg_type[MULT_RIGHT]->
			  info.vector.of->info.vector.dimension, NO);
      MET_TYPE_FREE(s);
      MET_TYPE_NEW_VECTOR(my_result_type, t1,
			  arg_type[MULT_LEFT]->info.vector.dimension, NO);
      MET_TYPE_FREE(t1);
   } 
	      
 escape:
   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "arguments of * are of wrong type", SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, MULT_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE)) {
      goto bail2;
   }

   MET_op_reconcile_sets(arg_set, MULT_NUM_KIDS);

   if (YES == over_sets) {
      MET_TYPE_NEW_SET_OF(guess, my_result_type);
      MET_TYPE_FREE(my_result_type);

      if (YES == MET_op_block_only(arg_set, MULT_NUM_KIDS, MET_op_mult,
				   sym->info.op.code, guess)) {
	 MET_SET_COPY(result_set, arg_set[0]);
      } else if (MET_SET_TYPE_BLOCK == arg_set[MULT_LEFT]->type ||
		 MET_SET_TYPE_BLOCK == arg_set[MULT_RIGHT]->type) {
	 result_set = MET_op_apply_to_elements(arg_set, MULT_NUM_KIDS,
					       MET_op_mult, sym->info.op.code,
					       guess, sym);
      }
      
      MET_TYPE_FREE(guess);
      goto bail1;
   }

   if (MET_SET_TYPE_BARE != arg_set[MULT_LEFT]->type ||
       MET_SET_TYPE_BARE != arg_set[MULT_RIGHT]->type) {
      ERROR(sym->origin, "arguments of * cannot be types or symbols", 
	    SEV_FATAL);
      MET_TYPE_FREE(my_result_type);
      goto bail1;
   }
   
   if ((arg_set[MULT_LEFT]->info.bare.data->verts->type == MET_FIELD_MAP ||
	arg_set[MULT_RIGHT]->info.bare.data->verts->type == MET_FIELD_MAP)
       && (sub_op == THING_SCALAR || sub_op == SCALAR_THING)) {
      Boolean maps[MULT_NUM_KIDS];

      maps[MULT_LEFT] = MET_FIELD_MAP ==
	 arg_set[MULT_LEFT]->info.bare.data->verts->type;
      maps[MULT_RIGHT] = MET_FIELD_MAP ==
	 arg_set[MULT_RIGHT]->info.bare.data->verts->type;
      
      result_set = MET_op_guts_for_maps(arg_set, maps, MULT_NUM_KIDS,
					sym->info.op.code, my_result_type);

      MET_TYPE_FREE(my_result_type);
      goto bail1;
   }

   MAX_VERTS(how_many, arg_set[MULT_LEFT]->info.bare.data,
	     arg_set[MULT_RIGHT]->info.bare.data);
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, MULT_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);

   switch(sub_op) {
    case THING_SCALAR:
      result =
	 MET__op_mult_guts_scalar(arg_set[MULT_RIGHT]->info.bare.data,
				  arg_set[MULT_LEFT]->info.bare.data,
				  result_set->info.bare.data);
      break;

    case SCALAR_THING:
      result =
	 MET__op_mult_guts_scalar(arg_set[MULT_LEFT]->info.bare.data,
				  arg_set[MULT_RIGHT]->info.bare.data,
				  result_set->info.bare.data);

      break;

    case MATRIX_VECTOR:
      result =
	 MET__op_mult_guts_matrix_vector(arg_set[MULT_LEFT]->info.bare.data,
					 arg_set[MULT_RIGHT]->info.bare.data,
					 result_set->info.bare.data);
      break;

    case MATRIX_MATRIX:
      result =
	 MET__op_mult_guts_matrix_matrix(arg_set[MULT_LEFT]->info.bare.data,
					 arg_set[MULT_RIGHT]->info.bare.data,
					 result_set->info.bare.data);
      break;

    default:
      DEFAULT_ERR(sub_op);
   }

   if (FAILURE == result) {
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(MULT_LEFT);
   COND_FREE(MULT_RIGHT);


   return result_set;
}
