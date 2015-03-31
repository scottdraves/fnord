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
   "$Id: met_verts.c,v 1.13 1992/06/22 17:05:07 mas Exp $";
#endif

void
MET__verts_free_deep(verts)
   METverts	*verts;
{
   int		i, total;
   METset	**set;
   METmap	*map;
   MME(MET__verts_free_deep);
   CHECK_VERTS(verts);

   total = verts->num_verts * verts->total_cols;

   switch(verts->type) {

    case MET_FIELD_ZTWO:
    case MET_FIELD_INTEGER:
    case MET_FIELD_ANGLE:
    case MET_FIELD_REAL:
    case MET_FIELD_COMPLEX:
    case MET_FIELD_QUATERNION:
      break;

    case MET_FIELD_MAP:
      map = (METmap *) verts->vertices;
      for (i = 0; i < total; i++) {
	 MET_SYM_FREE_NULL(map->sym);
	 MET_TYPE_FREE_NULL(map->loose_from);
	 MET_ENV_FREE_NULL(map->env);
	 map++;
      }
      break;

    case MET_FIELD_SET:
      set = (METset **) verts->vertices;
      for (i = 0; i < total; i++) {
	 MET_SET_FREE_NULL(*set);
	 set++;
      }
      break;

    case MET_FIELD_UNKNOWN:
    default:
      DEFAULT_ERR(verts->type);
   }

   FREE(verts->vertices, MET_field_size[verts->type] * total);

   verts->mminfo.next = MET__verts_free_list;
   MET__verts_free_list = verts;
#ifdef MET__DEBUG_MAGIC
   verts->magic = MET__MAGIC_FREE;
#endif
}


METverts *
MET__verts_copy_deep(verts)
   METverts	*verts;
{
   METverts	*baby;
   MME(MET__verts_copy_deep);
   CHECK_VERTS(verts);

   baby = MET_verts_new(verts->type,
			verts->num_verts,
			verts->total_cols);
   MET_verts_copy_vertices(baby, 0, 0,
			   verts, 0, 0,
			   verts->total_cols, verts->num_verts,
			   1, NO);

   return baby;
}

/*
 * create the specified verts table; the actual vertex values are
 * uninitialized
 */
METverts *
MET_verts_new(field, rows, columns)
   int		field;
   int		rows, columns;
{
   char		*vertices;
   METverts	*baby_verts;
   METmap	*map;
   METset	**set;
   int		i;
   MME(MET_verts_new);

   MET_VERTS_ALLOC(baby_verts);
   baby_verts->type = field;
   baby_verts->num_verts = rows;
   baby_verts->total_cols = columns;
   ALLOCNM(vertices, char, rows * columns * MET_field_size[field]);

   if (MET_FIELD_MAP == field) {
#ifndef lint
      map = (METmap *) vertices;
#else
      map = (METmap *) NULL;
#endif
      for (i = 0; i < rows * columns; i++) {
	 map->sym = MET_NULL_SYM;
	 map->loose_from = MET_NULL_TYPE;
	 map->env = MET_NULL_ENV;
	 map++;
      }
   } else if (MET_FIELD_SET == field) {
#ifndef lint
      set = (METset **) vertices;
#else
      set = (METset **) NULL;
#endif
      for (i = 0; i < rows * columns; i++)
	 *set++ = MET_NULL_SET;
   }

   /*
    * we know the pointer is aligned 'cause we got it from malloc().
    */
#ifndef lint
   baby_verts->vertices = (METnumber *) vertices;
#else
   baby_verts->vertices = (METnumber *) NULL;
   (void) printf("%#x\n", vertices);
#endif

   return baby_verts;
}

/*
 * fill a verts with zero values
 */
void
MET_verts_zero(verts)
   METverts	*verts;
{
   int		i, n;
   METztwo	*ztwo; 
   METinteger	*integer;
   METreal	*real;
   METcomplex	*complex;
   METset	**set;
   METmap	*map;
   MME(MET_verts_zero);

   n = verts->num_verts * verts->total_cols;
      
   switch (verts->type) {
    case MET_FIELD_ZTWO:
      ztwo = (METztwo *) verts->vertices;
      for (i = 0; i < n; i++)
	 *ztwo++ = '\0';
      break;

    case MET_FIELD_INTEGER:
      integer = (METinteger *) verts->vertices;
      for (i = 0; i < n; i++)
	 *integer++ = 0;
      break;

    case MET_FIELD_ANGLE:
      NYET;
      break;
      
    case MET_FIELD_REAL:
      real = (METreal *) verts->vertices;
      for (i = 0; i < n; i++)
	 *real++ = 0.0;
      break;

    case MET_FIELD_COMPLEX:
      complex = (METcomplex *) verts->vertices;
      for (i = 0; i < n; i++) {
	 complex->real = 0.0;
	 complex->imaginary = 0.0;
	 complex++;
      }
      break;

    case MET_FIELD_MAP:
      map = (METmap *) verts->vertices;
      for (i = 0; i < n; i++) {
	 map->sym = METsym_new_zero(RNULL);
	 MET_TYPE_NEW_UNKNOWN(map->loose_from);
	 map->env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV);
	 map++;
      }
      break;

    case MET_FIELD_SET:
      set = (METset **) verts->vertices;
      for (i = 0; i < n; i++) {
	 MET_SET_COPY(*set, MET_set_null);
	 set++;
      }
      break;

    case MET_FIELD_QUATERNION:
      NYET;
      break;

    default:
      DEFAULT_ERR(verts->type);
   }
}

#define COPY_LOOP_I_TO_R(DST, SRC)		      \
   for (j = k = 0; k < width; j++, k += skip) {	       \
      SRC = src_col + src_verts->total_cols * src_row + \
	 (METinteger *) src_verts->vertices + k;	 \
      DST = dst_col + dst_verts->total_cols * dst_row +   \
	 (METreal *) dst_verts->vertices + j;		   \
      UNROLL4(height, *DST = (METreal) *SRC;		    \
	 DST += dst_skip;				     \
	 SRC += src_skip;);				      \
   }

#define COPY_LOOP_I_TO_C(DST, SRC)		      \
   for (j = k = 0; k < width; j++, k += skip) {	       \
      SRC = src_col + src_verts->total_cols * src_row + \
	 (METinteger *) src_verts->vertices + k;	 \
      DST = dst_col + dst_verts->total_cols * dst_row +   \
	 (METcomplex *) dst_verts->vertices + j;	   \
     UNROLL4(height, DST->real = (METreal) *SRC;	    \
	    DST->imaginary = (METreal) 0.0;	 	     \
	    DST += dst_skip;				      \
	    SRC += src_skip;);				       \
   }

#define COPY_LOOP_R_TO_C(DST, SRC)		      \
   for (j = k = 0; k < width; j++, k += skip) {	       \
      SRC = src_col + src_verts->total_cols * src_row + \
         (METreal *) src_verts->vertices + k;   	 \
      DST = dst_col + dst_verts->total_cols * dst_row +   \
         (METcomplex *) dst_verts->vertices + j;	   \
      UNROLL4(height,  DST->real = *SRC;		    \
	      DST->imaginary = (METreal) 0.0;		     \
	      DST += dst_skip;				      \
	      SRC += src_skip;);			       \
   }

#define COPY_LOOP(DST, SRC, TYPE) 		    	     \
   for (j = k = 0; k < width; j++, k += skip) {	    	      \
      SRC = src_col + src_verts->total_cols * src_row +        \
         (TYPE *) src_verts->vertices + k;		        \
      DST = dst_col + dst_verts->total_cols * dst_row +          \
         (TYPE *) dst_verts->vertices + j;			  \
      UNROLL4(height, *DST = *SRC;				   \
	      DST += dst_skip;					    \
	      SRC += src_skip;);				     \
   }

#ifdef MET__DEBUG
#define MATCH_TYPES \
   if (src_verts->type != dst_verts->type) \
      WARNINGS("can't copy between verts of different types");
#else
#define MATCH_TYPES
#endif

/*
 * copy a rectangle of vertices from one METverts to another.
 * if replicate is YES, then only one row of the src (the first) is copied
 * to all rows in the destination.
 *
 * This will cast if the types of the vertices do not match.
 */
void
MET_verts_copy_vertices(dst_verts, dst_row, dst_col,
			src_verts, src_row, src_col,
			width, height, skip, replicate)
   METverts	*dst_verts;
   int		dst_row, dst_col;
   METverts	*src_verts;
   int		src_row, src_col, width, height, skip;
   Boolean	replicate;
{
   int		i, j, k, src_skip, dst_skip;
   METztwo	*src_ztwo, *dst_ztwo; 
   METinteger	*src_integer, *dst_integer;
   METangle	*src_angle, *dst_angle;
   METreal	*src_real, *dst_real;
   METcomplex	*src_complex, *dst_complex;
   METquaternion*src_quaternion, *dst_quaternion;
   METmap	*src_map, *dst_map;
   METset	**src_set, **dst_set;
   MME(MET_verts_copy_vertices);


   dst_skip = dst_verts->total_cols;
   if (YES == replicate)
      src_skip = 0;
   else
      src_skip = src_verts->total_cols;

   switch (dst_verts->type) {

    case MET_FIELD_ZTWO:
      MATCH_TYPES;
      COPY_LOOP(dst_ztwo, src_ztwo, METztwo);
      break;

    case MET_FIELD_INTEGER:
      COPY_LOOP(dst_integer, src_integer, METinteger);
      break;

    case MET_FIELD_ANGLE:
      MATCH_TYPES;
      COPY_LOOP(dst_angle, src_angle, METangle);
      break;

    case MET_FIELD_REAL:
      switch (src_verts->type) {
       case MET_FIELD_ZTWO:
	 MATCH_TYPES;
	 break;
       case MET_FIELD_INTEGER:
	 COPY_LOOP_I_TO_R(dst_real, src_integer);
	 break;
       case MET_FIELD_ANGLE:
	 MATCH_TYPES;
	 break;
       case MET_FIELD_REAL:
	 COPY_LOOP(dst_real, src_real, METreal);
	 break;
       case MET_FIELD_COMPLEX:
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:
	 MATCH_TYPES;
       default:
	 DEFAULT_ERR(dst_verts->type);
      }
      break;

    case MET_FIELD_COMPLEX:
      switch (src_verts->type) {
       case MET_FIELD_ZTWO:
	 MATCH_TYPES;
	 break;
       case MET_FIELD_INTEGER:
	 COPY_LOOP_I_TO_C(dst_complex, src_integer);
	 break;
       case MET_FIELD_ANGLE:
	 MATCH_TYPES;
	 break;
       case MET_FIELD_REAL:
	 COPY_LOOP_R_TO_C(dst_complex, src_real);
	 break;
       case MET_FIELD_COMPLEX:
	 COPY_LOOP(dst_complex, src_complex, METcomplex);
	 break;
       case MET_FIELD_QUATERNION:
       case MET_FIELD_MAP:
       case MET_FIELD_SET:
	 MATCH_TYPES;
       default:
	 DEFAULT_ERR(dst_verts->type);
      }
      break;

    case MET_FIELD_QUATERNION:
      MATCH_TYPES;
      COPY_LOOP(dst_quaternion, src_quaternion, METquaternion);
      break;

    case MET_FIELD_MAP:
      MATCH_TYPES;
      for (j = 0; j * skip < width; j++) {
	 src_map = src_col + src_verts->total_cols * src_row +
	    (METmap *) src_verts->vertices + j * skip;
	 dst_map = dst_col + dst_verts->total_cols * dst_row +
	    (METmap *) dst_verts->vertices + j;
	 
	 for (i = 0; i < height; i++) {
	    MET_SYM_FREE_NULL(dst_map->sym);
	    MET_TYPE_FREE_NULL(dst_map->loose_from);
	    MET_ENV_FREE_NULL(dst_map->env);
	    MET_SYM_COPY(dst_map->sym, src_map->sym);
	    MET_TYPE_COPY(dst_map->loose_from, src_map->loose_from);
	    MET_ENV_COPY(dst_map->env, src_map->env);
	    dst_map += dst_skip;
	    src_map += src_skip;
	 }
      }
      break;
      
    case MET_FIELD_SET:
      MATCH_TYPES;
      for (j = 0; j * skip < width; j++) {
	 src_set = src_col + src_verts->total_cols * src_row +
	    (METset **) src_verts->vertices + j * skip;
	 dst_set = dst_col + dst_verts->total_cols * dst_row +
	    (METset **) dst_verts->vertices + j;
	 
	 for (i = 0; i < height; i++) {
	    MET_SET_FREE_NULL(*dst_set);
	    MET_SET_COPY(*dst_set, *src_set);
	    dst_set += dst_skip;
	    src_set += src_skip;
	 }
      }
      break;

    default:
      DEFAULT_ERR(dst_verts->type);
   }
}
			


#define FUNC MET__verts_new_free_list
#define LIST MET__verts_free_list
#define TYPE METverts
#define MM_TYPE MET__MM_TYPE_VERTS
#include "new_free_list.c"
#undef FUNC
#undef LIST
#undef TYPE
#undef MM_TYPE


#ifdef MET__DEBUG_PRINT
void
METverts_print(stream, verts, indent_level)
   FILE		*stream;
   METverts	*verts;
   int		indent_level;
{
   int		row, col, max_row, max_col;
   double	t;
   METnumber	*scan;
   Boolean	suppress = NO, trunc_row = NO, trunc_col = NO;
   MME(METverts_print);
   CHECK_NULL_VERTS(verts);

   if (FNULL == stream)
      stream = stdout;

   MET_indent(stream, indent_level);
   if (MET_NULL_VERTS == verts) {
      PRINT(stream, "verts is null\n");
      return;
   }
   PRINT(stream,
	 "verts at %#x: type = %s, ref_count = %d, ",
	 verts, MET_names_field_type[verts->type], verts->mminfo.ref_count);
   PRINT(stream,
	 "num_verts = %d, total_cols = %d, vertices = %#x\n",
	 verts->num_verts, verts->total_cols, verts->vertices);

   scan = verts->vertices;
   max_row = verts->num_verts;
   if (max_row > 100) {
      max_row = 100;
      trunc_row = YES;
   }
   max_col = verts->total_cols;
   if (max_col > 20) {
      max_col = 20;
      trunc_col = YES;
   }
   for (row = 0; row < max_row; row++) {
      MET_indent(stream, indent_level);
      for (col = 0; col < max_col; col++) {
	 switch (verts->type) {
	
	  case MET_FIELD_ZTWO:
	    switch (* (METztwo *) scan) {
	       case 0  : PUTC('0', stream); break;
	       case 1  : PUTC('1', stream); break;
	       default : PUTC('x', stream); break;
	    }
	    scan = CAST(METnumber *, CAST(METztwo *, scan) + 1);
	    break;

	  case MET_FIELD_INTEGER:
	    PRINT(stream, "%8d", (int) * (METinteger *) scan);
	    scan = CAST(METnumber *, CAST(METinteger *, scan) + 1);
	    break;

	  case MET_FIELD_ANGLE:
	    PRINT(stream, "%8d", (int) * (METangle *) scan);
	    scan = CAST(METnumber *, CAST(METangle *, scan) + 1);
	    break;

	  case MET_FIELD_REAL:
	    PRINT(stream, "%8g", (double) * (METreal *) scan);
	    scan = CAST(METnumber *, CAST(METreal *, scan) + 1);
	    break;

	  case MET_FIELD_COMPLEX:
	    if ((t = (double)((METcomplex *) scan)->imaginary) >= 0.0) {
	       PRINT(stream, "%8g + %8gi",
		     (double)((METcomplex *) scan)->real, t);
	    } else {
	       PRINT(stream, "%8g - %8gi",
		     (double)((METcomplex *) scan)->real, -t);
	    } 
	    scan = CAST(METnumber *, CAST(METcomplex *, scan) + 1);
	    break;

	  case MET_FIELD_QUATERNION:
	    PRINT(stream, "%8g + %8gi + %8gj + %8gk",
		  (double)((METquaternion *) scan)->real,
		  (double)((METquaternion *) scan)->i,
		  (double)((METquaternion *) scan)->j,
		  (double)((METquaternion *) scan)->k);
	    scan = CAST(METnumber *, CAST(METquaternion *, scan) + 1);
	    break;

	  case MET_FIELD_MAP:
	    PRINT(stream, "%#8x %#8x %#8x",
		  (int)((METmap *) scan)->sym,
		  (int)((METmap *) scan)->loose_from,
		  (int)((METmap *) scan)->env);
	    if (5 > verts->num_verts &&
		1 == verts->total_cols) {
	       PUTC('\n', stream);
	       METsym_print(stream, ((METmap *) scan)->sym, indent_level);
	       METtype_print(stream, ((METmap *) scan)->loose_from,
			     indent_level);
	       METenv_print(stream, ((METmap *) scan)->env, indent_level);
	       suppress = YES;
	    }
	    scan = CAST(METnumber *, CAST(METmap *, scan) + 1);
	    break;

	  case MET_FIELD_SET:
	    PRINT(stream, "%#8x", (int) * (METset **) scan);
	    if (5 > verts->num_verts &&
		1 == verts->total_cols) {
	       PUTC('\n', stream);
	       METset_print(stream, * (METset **) scan, indent_level);
	       suppress = YES;
	    }
	    scan = CAST(METnumber *, CAST(METset **, scan) + 1);
	    break;

	  default:
	    DEFAULT_ERR(verts->type);
	 }
	 if (NO == suppress) {
	    PUTC(',', stream);
	    PUTC(' ', stream);
	 }
      }
      if (YES == trunc_col)
	 PRINT(stream, "%8s", "...");
      if (NO == suppress)
	 PUTC('\n', stream);
   }
   if (YES == trunc_row) {
      MET_indent(stream, indent_level);
      for (col = 0; col < max_col; col++)
	 PRINT(stream, "%8s, ", "...");
      if (YES == trunc_col)
	 PRINT(stream, "...");
      PUTC('\n', stream);
   }
}

Boolean
MET__verts_has_ref(verts, mm_type, object)
   METverts	*verts;
   int		mm_type;
   MET_object	*object;
{
   int		i, total;
   METset	**set;
   METmap	*map;
   MME(MET__verts_has_ref);

   switch(mm_type) {
    case MET__MM_TYPE_SET:
      if (MET_FIELD_SET == verts->type) {
	 set = (METset **) verts->vertices;
	 total = verts->num_verts * verts->total_cols;
	 for (i = 0; i < total ; i++)
	    if (*set++ == (METset *) object)
	       return YES;
      }
      break;

    case MET__MM_TYPE_TYPE:
    case MET__MM_TYPE_DATA:
    case MET__MM_TYPE_VERTS:
      break;

    case MET__MM_TYPE_SYM:
      if (MET_FIELD_MAP == verts->type) {
	 map = (METmap *) verts->vertices;
	 total = verts->num_verts * verts->total_cols;
	 for (i = 0; i < total; i++) {
	    if (map->sym == (METsym *) object)
	       return YES;
	    if (map->loose_from == (METtype *) object)
	       return YES;
	    map++;
	 }
      }
      break;

    case MET__MM_TYPE_BLOCK:
    case MET__MM_TYPE_CONNEX:
    case MET__MM_TYPE_LIST:
      break;

    default:
      DEFAULT_ERR(mm_type);
   }

   return NO;
}
#endif MET__DEBUG_PRINT



