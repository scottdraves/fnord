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
   "$Id: met_data.c,v 1.19 1992/06/22 17:04:35 mas Exp $";
#endif

void
MET__data_free_deep(data)
   METdata	*data;
{
   MME(MET__data_free_deep);
   CHECK_DATA(data);

   MET_DATA_FREE_NULL(data->next);

   MET_VERTS_FREE(data->verts);
   
   data->mminfo.next = MET__data_free_list;
   MET__data_free_list = data;
#ifdef MET__DEBUG_MAGIC
   data->magic = MET__MAGIC_FREE;
#endif
}

/*
 * make d2 appear at the end of d1.
 * destructive to d1
 */
void
MET_data_append(d1, d2)
   METdata	*d1, *d2;
{
   MME(MET_data_append);

   while(MET_NULL_DATA != d1->next)
      d1 = d1->next;
   MET_DATA_COPY(d1->next, d2);
}

/*
 * deep copy only the part of data up until
 * (but not including) stop_at.
 * returns a new data.
 */
METdata *
MET_data_copy_upto(data, stop_at)
   METdata	*data;
   METdata	*stop_at;
{
   METdata	*baby_data;
   MME(MET_data_copy_upto);
   CHECK_DATA(data);

   MET_DATA_ALLOC(baby_data);

   if (MET_NULL_DATA != data->next && stop_at != data->next)
      baby_data->next = MET_data_copy_upto(data->next, stop_at);
   else
      baby_data->next = MET_NULL_DATA;
   baby_data->num_cols = data->num_cols;
   baby_data->first_col = data->first_col;
   MET_VERTS_COPY(baby_data->verts, data->verts);

   return baby_data;
}

/*
 * like MET_DATA_COPY_DEEP, but the vertices are copied
 * deep as well
 */
METdata *
MET_data_copy_very_deep(data)
   METdata	*data;
{
   METdata	*baby_data;
   MME(MET_data_copy_very_deep);
   
   MET_DATA_ALLOC(baby_data);

   if (MET_NULL_DATA != data->next)
      baby_data->next = MET_data_copy_very_deep(data->next);
   else
      baby_data->next = MET_NULL_DATA;
   baby_data->num_cols = data->num_cols;
   baby_data->first_col = data->first_col;
   MET_VERTS_COPY_DEEP(baby_data->verts, data->verts);

   return baby_data;
}

/*
 * return the index'th data in the passed data.
 * if index is greatere than the number of datas, return NULL.
 */
METdata *
MET_data_index(data, index)
   METdata	*data;
   int		index;
{
   METdata	*copy;
   MME(ME_data_index);
   CHECK_NULL_DATA(data);

   if (MET_NULL_DATA == data)
      return MET_NULL_DATA;

   if (0 == index) {
      MET_DATA_COPY(copy, data);
      return copy;
   }

   /* tail recursion should be fast */
   return MET_data_index(data->next, index - 1);
}

/*
 * really copy the data, shallow copying the verts
 */
METdata *
MET__data_copy_deep(data)
   METdata	*data;
{
   MME(MET__data_copy_deep);

   return MET_data_copy_upto(data, MET_NULL_DATA);
}

/*
 * create a new data that refers to the specified portion of the passed
 * verts. 
 */
METdata *
MET_data_new(verts, num_cols, first_col)
   METverts	*verts;
   int		num_cols, first_col;
{
   METdata	*baby_data;
   MME(MET_data_new);
   CHECK_VERTS(verts);

   MET_DATA_ALLOC(baby_data);
   baby_data->next = MET_NULL_DATA;
#ifdef MET__DEBUG
   if (num_cols + first_col > verts->total_cols)
      FATAL((ERR_ARB, "columns out of range: %d + %d > %d", num_cols,
	     first_col, verts->total_cols));
#endif
   baby_data->num_cols = num_cols;
   baby_data->first_col = first_col;
   MET_VERTS_COPY(baby_data->verts, verts);

   return baby_data;
}

/*
 * create a new METdata that has the i'th row of the passed METdata
 */
METdata *
MET_data_extract_row(data, row)
   METdata	*data;
   int		row;
{
   METdata	*baby_data;
   METverts	*verts;
   MME(MET_data_extract_row);

   verts = MET_verts_new(data->verts->type, 1, data->num_cols);
   MET_verts_copy_vertices(verts, 0, 0, data->verts, row, data->first_col,
			   data->num_cols, 1, 1, NO);
   baby_data = MET_data_new(verts, data->num_cols, 0);
   MET_VERTS_FREE(verts);

   if (MET_NULL_DATA != data->next)
      baby_data->next = MET_data_extract_row(data->next, row);

   return baby_data;
}

/*
 * create a new METdata that has the data from the passed METdata
 * with index ROW---assuming that things are stored as an array
 * where a block of AFTER consecutive elements have the same
 * value of the index and the index takes on SIZE values.
 */
METdata *
MET_data_extract_at(data, size, after, row)
   METdata	*data;
   int		size;
   int		after;
   int		row;
{
   METdata	*baby_data;
   METverts	*verts;
   int		i, j;
   MME(MET_data_extract_at);

   verts = MET_verts_new(data->verts->type, data->verts->num_verts / size,
			 data->num_cols);
   for (j = 0, i = row * after; i < data->verts->num_verts; 
	j += after, i += size * after)
      MET_verts_copy_vertices(verts, j, 0, data->verts, i, data->first_col,
			      data->num_cols, after, 1, NO);

   baby_data = MET_data_new(verts, data->num_cols, 0);
   MET_VERTS_FREE(verts);

   if (MET_NULL_DATA != data->next)
      baby_data->next = MET_data_extract_at(data->next, size, after, row);

   return baby_data;
}

/*
 * Create a new data by taking out those elements corresponding to
 * values of the index determined by SIZE and AFTER such that the
 * corresponding value in the array of Booleans is true or false.
 */

METdata *
MET_data_extract_subset(data, size, after, take, table, take_true)
   METdata	*data;
   int		size;
   int		after;
   int		take;
   FOboolean	*table;
   FOboolean	take_true;
{
   METdata	*baby_data;
   METverts	*verts;
   int		i, j, k, taken;
   MME(MET_data_extract_subset);

   verts = MET_verts_new(data->verts->type, 
			 data->verts->num_verts / size * take,
			 data->num_cols);

   for(i = 0, taken = 0; i < size; i++)
      if (take_true == table[i]) {
	 for (j = i * after, k = taken * after;
	      j < data->verts->num_verts;
	      j += size * after, k += take * after)
	    MET_verts_copy_vertices(verts, k, 0, 
				    data->verts, j, data->first_col,
				    data->num_cols, after, 1, NO);
	 taken++;
      }

   baby_data = MET_data_new(verts, data->num_cols, 0);
   MET_VERTS_FREE(verts);

   if (MET_NULL_DATA != data->next)
      baby_data->next = MET_data_extract_subset(data->next, size, after,
						take, table, take_true);

   return baby_data;
}

/*
 * create a new data with num_verts verts.  data's current verts
 * should only have one vertex.  A new verts is created in the process.
 */
METdata *
MET_data_replicate(data, num_verts)
   METdata	*data;
   int		num_verts;
{
   METdata	*baby_data;
   METverts	*verts;
   MME(ME_data_replicate);

   verts = MET_verts_new(data->verts->type, num_verts, data->num_cols);
   MET_verts_copy_vertices(verts, 0, 0, data->verts, 0, data->first_col,
			   data->num_cols, num_verts, 1, YES);

   baby_data = MET_data_new(verts, data->num_cols, 0);
   MET_VERTS_FREE(verts);

   if (MET_NULL_DATA != data->next)
      baby_data->next = MET_data_replicate(data->next, num_verts);

   return baby_data;
}

/*
 * create a block that refers to the data.  data should be bare
 */
METblock *
MET_data_to_block(data, type)
   METdata	*data;
   METtype	*type;
{
   METblock	*baby_block;
   MME(MET_data_to_block);
   CHECK_DATA(data);
   CHECK_TYPE(type);

   MET_BLOCK_ALLOC(baby_block);
   baby_block->next = MET_NULL_BLOCK;
   MET_TYPE_NEW_SET_OF(baby_block->type, type);
   baby_block->data = MET_data_copy_upto(data, data->next);
   baby_block->connex = MET_connex_new(0, INULL, INULL);
   MET_CONNEX_COPY(baby_block->data_connex, baby_block->connex);
   MET_tags_initialize(&baby_block->tags_in);
   MET_tags_initialize(&baby_block->tags_part);
#ifdef MET__DEBUG
   if (1 != DATA_NUM_VERTS(data))
      WARNING((ERR_SI, "data is not bare", DATA_NUM_VERTS(data)));
#endif

   return baby_block;
}

/*
 * destructively make data be of type field.  this may be a no-op, or it
 * may involve replacing the verts.
 */
static void
MET__data_cast_to_field(data, field)
   METdata	*data;
   int		field;
{
   METverts	*verts;
   MME(MET__data_cast_to_field);
   CHECK_DATA(data);

   if (field == data->verts->type)
      return;

   verts = MET_verts_new(field, data->verts->num_verts, data->num_cols);
   MET_verts_copy_vertices(verts, 0, 0, data->verts, 0, data->first_col,
			   data->num_cols, data->verts->num_verts, 1, NO);
   MET_VERTS_FREE(data->verts);
   MET_VERTS_COPY(data->verts, verts);
   MET_VERTS_FREE(verts);
   data->first_col = 0;
}

/*
 * destructively make sets in DATA's verts be of type TYPE.
 */

static Boolean
MET__data_set_cast_to_type(data, type)
   METdata	*data;
   METtype	*type;
{
   METverts 	*verts;
   METset	*cast, **first;
   int		i;
   MME(MET__data_set_cast_to_type);
   CHECK_DATA(data);
   CHECK_TYPE(type);

   verts = MET_verts_new(MET_FIELD_SET, data->verts->num_verts, data->num_cols);
   MET_verts_copy_vertices(verts, 0, 0, data->verts, 0, data->first_col,
			   data->num_cols, data->verts->num_verts, 1, NO);
   
   MET_VERTS_FREE(data->verts);
   MET_VERTS_COPY(data->verts, verts);
   MET_VERTS_FREE(verts);
   data->first_col = 0;
   first = DATA_FIRST_VERTEX(data, METset *);
   for (i=0; i < data->num_cols * data->verts->num_verts; i++) {
      cast = MET_set_cast_to_type(first[i], type);
      if (MET_NULL_SET == cast) 
	 return FAILURE;

      MET_SET_FREE(first[i]);
      MET_SET_COPY(first[i], cast);
      MET_SET_FREE(cast);
   }
   return SUCCESS;
}

/*
 * see MET_data_cast_to_type()
 */
Boolean
MET__data_cast_to_type(data, type, new_type)
   METdata	**data;
   METtype	*type, *new_type;
{
   int		field;
   MME(MET__data_cast_to_type);
   CHECK_TYPE(type);
   CHECK_TYPE(new_type);

   if (new_type->type == MET_TYPE_TYPE_UNKNOWN) {
      switch (type->type) {
       case MET_TYPE_TYPE_FIELD:
       case MET_TYPE_TYPE_MAP:
       case MET_TYPE_TYPE_SET:
       case MET_TYPE_TYPE_SET_OF:
	 *data = (*data)->next;
	 break;

       default:
	 break;
      }
      return SUCCESS;
   }

   if (!((MET_TYPE_TYPE_SET == type->type && 
	  MET_TYPE_TYPE_SET_OF == new_type->type) ||
	 (MET_TYPE_TYPE_SET == new_type->type &&
	  MET_TYPE_TYPE_SET_OF == type->type)) &&
       type->type != new_type->type)
      return FAILURE;

   switch (new_type->type) {

    case MET_TYPE_TYPE_PAIR:
      if (FAILURE == MET__data_cast_to_type(data, type->info.pair.left,
					    new_type->info.pair.left) ||
	  FAILURE == MET__data_cast_to_type(data, type->info.pair.right,
					    new_type->info.pair.right))
	 return FAILURE;
      break;
	  
    case MET_TYPE_TYPE_VECTOR:
      /*
       * if we are casting to a type with a unknown vector length, then we
       * assume we match it.  this is so that  (R^? : [1]) works.
       * ACK! WRONG!
       */
      if (type->info.vector.dimension != new_type->info.vector.dimension &&
	  new_type->info.vector.dimension != TYPE_DIMENSION_UNKNOWN)
	 return FAILURE;

      return MET__data_cast_to_type(data, type->info.vector.of,
				    new_type->info.vector.of);
      
    case MET_TYPE_TYPE_FIELD:
      field = TYPE_CAST_FIELDS(type->info.field.type,
			       new_type->info.field.type);

      if (MET_FIELD_UNKNOWN == field || 
	  new_type->info.field.type != field)
	 return FAILURE;

      MET__data_cast_to_field(*data, field);
      *data = (*data)->next;
      break;

    case MET_TYPE_TYPE_MAP:
      /* let's hope that the type of the first vertex is the
	 type of the rest... */
      if (NO == 
	  MET_type_compatible(DATA_FIRST_VERTEX(*data, METmap)->loose_from,
			      new_type->info.map.from, YES)) 
	 return FAILURE;
      /* fall through */

    case MET_TYPE_TYPE_SET:
      *data = (*data)->next;
      break;

    case MET_TYPE_TYPE_SET_OF:
      if (FAILURE == 
	  MET__data_set_cast_to_type(*data, new_type))
	 return FAILURE;

      *data = (*data)->next;
      break;
      
    case MET_TYPE_TYPE_BOGUS:
      return FAILURE;

    default:
      DEFAULT_ERR(type->type);
   }

   return SUCCESS;
}

/*
 * create and return a new data that is like DATA, but has type TYPE.
 * returns a shallow copy if it is already of the correct type.
 */
METdata *
MET_data_cast_to_type(data, type, new_type)
   METdata	*data;
   METtype	*type, *new_type;
{
   METdata	*result = MET_NULL_DATA, *scan;
   MME(MET_data_cast_to_type);
   CHECK_DATA(data);
   CHECK_TYPE(type);

   if (YES == MET_type_compatible(type, new_type, NO)) {
      MET_DATA_COPY(result, data);
      return result;
   }

   MET_DATA_COPY_DEEP(result, data);
   scan = result;
   if (SUCCESS == MET__data_cast_to_type(&scan, type, new_type))
      return result;

   MET_DATA_FREE(result);
   return MET_NULL_DATA;
}


#define FUNC MET__data_new_free_list
#define LIST MET__data_free_list
#define TYPE METdata
#define MM_TYPE MET__MM_TYPE_DATA
#include "new_free_list.c"
#undef FUNC
#undef LIST
#undef TYPE
#undef MM_TYPE

#define MAX_PRINT_DIMS 3

static void
MET__data_pretty_map(buf, num, verts)
   char		*buf;
   METnumber	*num;
   METverts	*verts;
{
   MME(MET__data_pretty_map);
   (void) strcat(buf, "fn");
}

static void
MET__data_pretty_field(buf, num, verts)
   char		*buf;
   METnumber	*num;
   METverts	*verts;
{
   char		temp[100];
   MME(MET__data_pretty_field);
   switch(verts->type) {
    case MET_FIELD_ZTWO:
      if (0 == num->ztwo)
	 (void) strcat(buf, "False");
      else
	 (void) strcat(buf, "True ");
      return;
    case MET_FIELD_INTEGER:
      (void) sprintf(temp, "%8d", (int) num->integer);
      break;
    case MET_FIELD_REAL:
      (void) sprintf(temp, "%8g", (double) num->real);
      break;
    case MET_FIELD_COMPLEX:
      (void) sprintf(temp, "%8g + ",
		     (double) num->complex.real);
      (void) strcat(buf, temp);
      (void) sprintf(temp, "%8gi",
		     (double) num->complex.imaginary);
      (void) strcat(buf, temp);
      return;
    case MET_FIELD_MAP:
      (void) strcat(buf, "fn");
      return;
    case MET_FIELD_SET:
      METset_print_pretty(FNULL, num->set, buf, 0);
      return;
    default:
      DEFAULT_ERR(verts->type);
   }
   (void) strcat(buf, temp);
}

static void
MET__data_pretty_set(buf, num, verts)
   char		*buf;
   METnumber	*num;
   METverts	*verts;
{
   MME(MET__data_pretty_set);
   METset_print_pretty(FNULL, num->set, buf, 0);
}

static METnumber *
MET__data_number_add(scan, field, n)
   METnumber	*scan;
   int		field;
   int		n;
{
   MME(MET__data_number_add);
   
   switch(field) {
    case MET_FIELD_ZTWO:
      scan = CAST(METnumber *, CAST(METztwo *, scan) + n);
      break;
    case MET_FIELD_INTEGER:
      scan = CAST(METnumber *, CAST(METinteger *, scan) + n);
      break;
    case MET_FIELD_REAL:
      scan = CAST(METnumber *, CAST(METreal *, scan) + n);
      break;
    case MET_FIELD_COMPLEX:
      scan = CAST(METnumber *, CAST(METcomplex *, scan) + n);
      break;
    case MET_FIELD_MAP:
      scan = CAST(METnumber *, CAST(METmap *, scan) + n);
      break;
    case MET_FIELD_SET:
      scan = CAST(METnumber *, CAST(METset **, scan) + n);
      break;
    default:
      DEFAULT_ERR(field);
   }
   return scan;
}

static void
MET__data_pretty_array(type, data, buf, dims, ndims, fn, row)
   METtype	*type;
   METdata	*data;
   char		*buf;
   int		*dims;
   int		ndims;
   void		(*fn)();
   int		row;
{
   int		c[MAX_PRINT_DIMS];
   METnumber	*scan;
   MME(MET__data_pretty_array);

   switch(data->verts->type) {
    case MET_FIELD_ZTWO:
      scan = CAST(METnumber *, 
		  row * data->verts->total_cols +
		  DATA_FIRST_VERTEX(data, METztwo));
      break;
    case MET_FIELD_INTEGER:
      scan = CAST(METnumber *,
		  row * data->verts->total_cols +
		  DATA_FIRST_VERTEX(data, METinteger));
      break;
    case MET_FIELD_REAL:
      scan = CAST(METnumber *,
		  row * data->verts->total_cols +
		  DATA_FIRST_VERTEX(data, METreal));
      break;
    case MET_FIELD_COMPLEX:
      scan = CAST(METnumber *,
		  row * data->verts->total_cols +
		  DATA_FIRST_VERTEX(data, METcomplex));
      break;
    case MET_FIELD_MAP:
      scan = CAST(METnumber *,
		  row * data->verts->total_cols +
		  DATA_FIRST_VERTEX(data, METmap));
      break;
    case MET_FIELD_SET:
      scan = CAST(METnumber *,
		  row * data->verts->total_cols +
		  DATA_FIRST_VERTEX(data, METset *));
      break;
    default:
      DEFAULT_ERR(data->verts->type);
   }
   
   switch (ndims) {

    case 0:
      (fn)(buf, scan, data->verts);
      break;

    case 1:
      (void) strcat(buf, "[");
      for (c[0] = 0; c[0] < dims[0]; c[0]++) {
	 (fn)(buf, scan, data->verts);
	 scan = MET__data_number_add(scan, data->verts->type, 1);
	 if (c[0] != dims[0] - 1)
	    (void) strcat(buf, ", ");
      }
      (void) strcat(buf, "]");
      break;

    case 2:
      (void) strcat(buf, "\n[");
      for (c[0] = 0; c[0] < dims[0]; c[0]++) {
	 (void) strcat(buf, "[");
	 for (c[1] = 0; c[1] < dims[1]; c[1]++) {
	    (fn)(buf, scan, data->verts);
	    scan = MET__data_number_add(scan, data->verts->type, 1);
	    if (c[1] != dims[1] - 1)
	       (void) strcat(buf, ", ");
	 }
	 (void) strcat(buf, "]");
	 if (c[0] != dims[0] - 1)
	    (void) strcat(buf, ",\n ");
      }
      (void) strcat(buf, "]");
      break;

    case 3:
      (void) strcat(buf, "\n[");
      for (c[0] = 0; c[0] < dims[0]; c[0]++) {
	 (void) strcat(buf, "[");
	 for (c[1] = 0; c[1] < dims[1]; c[1]++) {
	    (void) strcat(buf, "[");
	    for (c[2] = 0; c[2] < dims[2]; c[2]++) {
	       (fn)(buf, scan, data->verts);
	       scan = MET__data_number_add(scan, data->verts->type, 1);
	       if (c[2] != dims[2] - 1)
		  (void) strcat(buf, ", ");
	    }
	    (void) strcat(buf, "]");
	    if (c[1] != dims[1] - 1)
	       (void) strcat(buf, ", ");
	 }
	 (void) strcat(buf, "]");
	 if (c[0] != dims[0] - 1)
	    (void) strcat(buf, ",\n ");
      }
      (void) strcat(buf, "]");
      break;

    default:
      (void) strcat(buf, "<<too many indices>>");
      break;
   }
}

static void
MET__data_pretty_traverse(type, data, buf, is_right, dims, ndims, row)
   METtype	*type;
   METdata	**data;
   char		*buf;
   Boolean	is_right;
   int		*dims;
   int		ndims;
   int		row;
{
   MME(MET__data_pretty_traverse);

   switch (type->type) {
    case MET_TYPE_TYPE_MAP:
      MET__data_pretty_array(type, *data, buf, dims, ndims,
			     MET__data_pretty_map, row);
      *data = (*data)->next;
      break;

    case MET_TYPE_TYPE_PAIR:
      if (0 != ndims) {
	 (void) strcat(buf, "(cannot print vector of pairs)");
      } else {
	 if (YES == is_right)
	    (void) strcat(buf, "(");
	 MET__data_pretty_traverse(type->info.pair.left, data, buf, NO,
				   dims, ndims, row);
	 (void) strcat(buf, ", ");
	 MET__data_pretty_traverse(type->info.pair.right, data, buf, YES,
				   dims, ndims, row);
	 if (YES == is_right)
	    (void) strcat(buf, ")");
      }
      break;

    case MET_TYPE_TYPE_VECTOR:
      if (ndims >= MAX_PRINT_DIMS) {
	 (void) strcat(buf, "(cannot print tensor of that rank)");
      } else {
	 dims[ndims] = type->info.vector.dimension;
	 MET__data_pretty_traverse(type->info.vector.of, data, buf, NO,
				   dims, ndims + 1, row);
      }
      break;

    case MET_TYPE_TYPE_SET_OF:
    case MET_TYPE_TYPE_SET:
      MET__data_pretty_array(type, *data, buf, dims, ndims,
			     MET__data_pretty_set, row); 
      *data = (*data)->next;
      break;
    case MET_TYPE_TYPE_FIELD:
      MET__data_pretty_array(type, *data, buf, dims, ndims,
			     MET__data_pretty_field, row);
      *data = (*data)->next;
      break;

    case MET_TYPE_TYPE_UNKNOWN:
    case MET_TYPE_TYPE_BOGUS:
    default:
      DEFAULT_ERR(type->type);
   }
}

void
METdata_print_pretty(stream, data, type, s, slen)
   FILE		*stream;
   METdata	*data;
   METtype	*type;
   char		*s;
   int		slen;
{
   char		buf[8192];
   int		dims[MAX_PRINT_DIMS];
   int		i;
   METdata	*tdata;
   MME(METdata_print_pretty);

   for (i = 0; i < DATA_NUM_VERTS(data); i++) {
      buf[0] = '\0';
      tdata = data;
      MET__data_pretty_traverse(type, &tdata, buf, NO, dims, 0, i);

      if (i != DATA_NUM_VERTS(data) - 1)
	 (void) strcat(buf, ", ");
      
      if (FNULL != stream)
	 (void) fprintf(stream, "%s", buf);
      if (CNULL != s)
	 (void) strcat(s, buf);
   }
}

static Boolean
MET__data_write_guts(data, buf, len)
   METdata		*data;
   unsigned char	**buf;
   int			*len;
{
   int			i, j, size, type, width;
   METnumber		*scan;
   MME(MET__data_write_guts);

   type = data->verts->type;
   size = MET_field_size[type];
   width = size * data->verts->total_cols;
   for (i = data->first_col; i < data->first_col + data->num_cols; i++) {
      scan = CAST(METnumber *, i * size + CAST(char *, data->verts->vertices));
      for (j = 0; j < data->verts->num_verts; j++) {
	 switch(type) {
	  case MET_FIELD_ZTWO:
	    WRITE_CHAR(*buf, scan->ztwo);
	    break;
	  case MET_FIELD_INTEGER:
	    WRITE_INTEGER(*buf, scan->integer);
	    break;
	  case MET_FIELD_REAL:
	    WRITE_DOUBLE(*buf, scan->real);
	    break;
	  case MET_FIELD_COMPLEX:
	    WRITE_DOUBLE(*buf, scan->complex.real);
	    WRITE_DOUBLE(*buf, scan->complex.imaginary);
	    break;
	 }
	 scan = CAST(METnumber *, width + CAST(char *, scan));
      }
   }

   *len -= size * data->num_cols * data->verts->num_verts;
   return ALIGN_FOR_NEXT(*buf, len, type);
}

static void
MET__data_read_guts(data, buf, len, used)
   METdata		*data;
   unsigned char	*buf;
   int			len;
   int			*used;
{
   int			i, j, size, width, type;
   int			startlen;
   METnumber		*scan;
   MME(MET__data_write_guts);

   startlen = len;
   type = data->verts->type;
   size = MET_field_size[type];
   width = size * data->verts->total_cols;
   for (i = data->first_col; i < data->first_col + data->num_cols; i++) {
      scan = CAST(METnumber *, i * size + CAST(char *, data->verts->vertices));
      for (j = 0; j < data->verts->num_verts; j++) {
	 switch(type) {
	  case MET_FIELD_ZTWO:
	    READ_CHAR(buf, scan->ztwo);
	    break;
	  case MET_FIELD_INTEGER:
	    READ_INTEGER(buf, scan->integer);
	    break;
	  case MET_FIELD_REAL:
	    READ_DOUBLE(buf, scan->real);
	    break;
	  case MET_FIELD_COMPLEX:
	    READ_DOUBLE(buf, scan->complex.real);
	    READ_DOUBLE(buf, scan->complex.imaginary);
	    break;
	 }
	 scan = CAST(METnumber *, width + CAST(char *, scan));
      }
   }

   len -= size * data->num_cols * data->verts->num_verts;
   ALIGN_FOR_NEXT(buf, len, type);
   *used += startlen - len;
}

#define LAST_IN_LIST 0x01
#define MORE_FOLLOW  0x02
#define READ_VERTS   0x01
#define READ_VERTS_AND_NEXT 0x02
#define READ_NEXT 0x03

Boolean
METdata_write(data, buf, len)
   METdata		*data;
   unsigned char	**buf;
   int			*len;
{
   int		need_in_buffer;
   MME(METdata_write);

   if (4 * sizeof(int) > *len)
      return FAILURE;

   WRITE_INTEGER(*buf, (MET_NULL_DATA == data->next ? 
			LAST_IN_LIST : MORE_FOLLOW));
   WRITE_INTEGER(*buf, data->num_cols);
   WRITE_INTEGER(*buf, data->verts->num_verts);
   WRITE_INTEGER(*buf, data->verts->type);
   *len -= 4 * sizeof(int);

   switch(data->verts->type) {
    case MET_FIELD_ZTWO:
      need_in_buffer = sizeof(METztwo);
      break;
    case MET_FIELD_INTEGER:
      need_in_buffer = sizeof(METinteger);
      break;
    case MET_FIELD_REAL:
      need_in_buffer = sizeof(METreal);
      break;
    case MET_FIELD_COMPLEX:
      need_in_buffer = sizeof(METcomplex);
      break;
    case MET_FIELD_MAP:
      NYET;
      return FAILURE;
      break;
    case MET_FIELD_SET:
      NYET;
      return FAILURE;
      break;
    default:
      DEFAULT_ERR(data->verts->type);
   }

   need_in_buffer *= data->num_cols * data->verts->num_verts;

   if (need_in_buffer + 1 > *len)
      return FAILURE;

   if (FAILURE == MET__data_write_guts(data, buf, len))
      return FAILURE;

   if (MET_NULL_DATA == data->next)
      return SUCCESS;

   return METdata_write(data->next, buf, len);
}

METdata *
METdata_read(buf, len, used, list, last, cont)
   unsigned char	*buf;
   int			len;
   int			*used;
   METlist		**list;
   MET_object_p		last;
   Boolean		cont;
{
   int		code;
   int		num_cols;
   int		num_verts;
   int		type;
   METverts	*verts;
   METdata	*data;
   MET_object_p op;
   METlist      *oldlist;
   int		need_in_buffer;
   MME(METdata_read);
   
   if (NO == cont) {
      if (4 * sizeof(int) > len)
	 return MET_NULL_DATA;
      READ_INTEGER(buf, code);
      READ_INTEGER(buf, num_cols);
      READ_INTEGER(buf, num_verts);
      READ_INTEGER(buf, type);
      len -= 4 * sizeof(int);
      *used += 4 * sizeof(int);
      verts = MET_verts_new(type, num_verts, num_cols);
      data = MET_data_new(verts, num_cols, 0);
      MET_VERTS_FREE(verts);

      if (*list != MET_NULL_LIST) {
	 MET_LIST_COPY(oldlist, *list);
	 MET_LIST_FREE(*list);
      } else 
	 oldlist = MET_NULL_LIST;

      MET_LIST_ALLOC(*list);
      MET_DATA_COPY((*list)->object.data, data);
      (*list)->mm_type = MET__MM_TYPE_DATA;
      (*list)->aux_mm_type = MET__MM_TYPE_NONE;
      (*list)->data.i = (code == MORE_FOLLOW) ? 
	 READ_VERTS_AND_NEXT : READ_VERTS;

      if (MET_NULL_LIST != oldlist) {
	 MET_LIST_COPY((*list)->next, oldlist);
	 MET_LIST_FREE(oldlist);
      } else 
	 (*list)->next = MET_NULL_LIST;
   } else if (MET_NULL_LIST == *list)
      return last.data;
   else {
      MET_DATA_COPY(data, (*list)->object.data);
      if (READ_NEXT == (*list)->data.i) {
	 if (MET_NULL_DATA == last.data) 
	    goto read_next;
	 MET_DATA_COPY(data->next, last.data);
	 goto finish;
      }
   }
      
   switch(data->verts->type) {
    case MET_FIELD_ZTWO:
      need_in_buffer = sizeof(METztwo);
      break;
    case MET_FIELD_INTEGER:
      need_in_buffer = sizeof(METinteger);
      break;
    case MET_FIELD_REAL:
      need_in_buffer = sizeof(METreal);
      break;
    case MET_FIELD_COMPLEX:
      need_in_buffer = sizeof(METcomplex);
      break;
    case MET_FIELD_MAP:
    case MET_FIELD_SET:
      NYET;
    default:
      MET_LIST_FREE_NULL(*list);
      *list = MET_NULL_LIST;
      return MET_NULL_DATA;
   }

   need_in_buffer *= data->num_cols * data->verts->num_verts;

   if (need_in_buffer > len) {
      MET_DATA_FREE(data);
      return MET_NULL_DATA;
   }

   num_verts = 0;
   MET__data_read_guts(data, buf, len, &num_verts);
   *used += num_verts;
   len -= num_verts;
   buf += num_verts;
   
   if (READ_VERTS == (*list)->data.i)
      goto finish;

 read_next:
   (*list)->data.i = READ_NEXT;
   op.data = MET_NULL_DATA;
   data->next = METdata_read(buf, len, used, list, op, FALSE);
   if (MET_NULL_DATA == data->next) {
      MET_DATA_FREE(data);
      return MET_NULL_DATA;
   }

 finish:
   if (MET_NULL_LIST == (*list)->next) {
      MET_LIST_FREE(*list);
      *list = MET_NULL_LIST;
   } else {
      MET_LIST_COPY(oldlist, (*list)->next);
      MET_LIST_FREE(*list);
      MET_LIST_COPY(*list, oldlist);
      MET_LIST_FREE(oldlist);
   }

   return data;
}

#ifdef MET__DEBUG_PRINT
void
METdata_print(stream, data, indent_level)
   FILE		*stream;
   METdata	*data;
   int		indent_level;
{
   MME(METdata_print);
   CHECK_NULL_DATA(data);

   if (FNULL == stream)
      stream = stdout;

   MET_indent(stream, indent_level);
   if (MET_NULL_DATA == data) {
      PRINT(stream, "data is null\n");
      return;
   }
   PRINT(stream,
	 "data at %#x: ref_count = %d, num_cols = %d, first_col = %d\n",
	 data, data->mminfo.ref_count, data->num_cols, data->first_col);
   METverts_print(stream, data->verts, indent_level + 1);
   if (MET_NULL_DATA != data->next)
      METdata_print(stream, data->next, indent_level);
}
Boolean
MET__data_has_ref(data, mm_type, object)
   METdata	*data;
   int		mm_type;
   MET_object	*object;
{
   MME(MET__data_has_ref);

   switch(mm_type) {
    case MET__MM_TYPE_SET:
    case MET__MM_TYPE_TYPE:
      break;

    case MET__MM_TYPE_DATA:
      if (data->next == (METdata *) object)
	 return YES;

    case MET__MM_TYPE_VERTS:
      if (data->verts == (METverts *) object)
	 return YES;

    case MET__MM_TYPE_SYM:
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
