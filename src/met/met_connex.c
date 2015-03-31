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
   "$Id: met_connex.c,v 1.14 1992/06/22 17:04:33 mas Exp $";
#endif

void
MET__connex_free_deep(connex)
   METconnex	*connex;
{
   MME(MET__connex_free_deep);
   CHECK_CONNEX(connex);

   if (connex->ndims > MET_CONNEX_STATIC_SIZE)
   {
      FREEN(connex->dims, int, connex->ndims);
      FREEN(connex->types, int, connex->ndims);
   }

   connex->mminfo.next = MET__connex_free_list;
   MET__connex_free_list = connex;
#ifdef MET__DEBUG_MAGIC
   connex->magic = MET__MAGIC_FREE;
#endif
}


METconnex *
MET__connex_copy_deep(connex)
   METconnex	*connex;
{
   METconnex	*baby;
   MME(MET__copy_deep);
   CHECK_CONNEX(connex);

   baby = MET_connex_new(connex->ndims,
			 connex->dims,
			 connex->types);

   return baby;
}

METconnex *
MET_connex_new(ndims, dims, types)
   int		ndims;		/* number of dimensions */
   int		*dims;		/* array of dimensions */
   int		*types;		/* whether each dimension is part
				   of a square or triangular mesh */
{
   METconnex *connex;
   int i;
   int *dim;
   int *type;

   MME(METconnex_new);

   MET_CONNEX_ALLOC(connex);
   connex->ndims = ndims;

   if (ndims > MET_CONNEX_STATIC_SIZE) {
      ALLOCNM(connex->dims, int, ndims);
      ALLOCNM(connex->types, int, ndims);
   } else {
      connex->dims = connex->static_dims;
      connex->types = connex->static_types;
   }

   for(i = ndims, dim = connex->dims, type=connex->types;
       i > 0;
       i--)
   {
      *dim++ = *dims++;
      *type++ = *types++;
   }

   return connex;
}


/*
 * Create a connex for the cross product of two connexes
 */
METconnex *
MET_connex_cross(connex1, connex2)
   METconnex	*connex1, *connex2;
{
   METconnex *connex;
   int i;
   int *dims, *new_dim, *types, *new_type;

   ME(MET_connex_cross);
   CHECK_CONNEX(connex1);
   CHECK_CONNEX(connex2);

   MET_CONNEX_ALLOC(connex);
   connex->ndims = connex1->ndims + connex2->ndims;

   if (connex->ndims > MET_CONNEX_STATIC_SIZE) {
      ALLOCNM(connex->dims, int, connex->ndims);
      ALLOCNM(connex->types, int, connex->ndims);
   } else {
      connex->types = connex->static_types;
      connex->dims = connex->static_dims;
   }

   /* copy the dims and types arrays, first from connex1 and then from */
   /* connex2  */

   i = connex1->ndims;
   new_dim = connex->dims;
   new_type = connex->types;
   dims = connex1->dims;
   types = connex1->types;
   while (i--)
   {
      *new_dim++ = *dims++;
      *new_type++ = *types++;
   }
   i = connex2->ndims;
   dims = connex2->dims;
   types = connex2->types;
   while (i--)
   {
      *new_dim++ = *dims++;
      *new_type++ = *types++;
   }

   return connex;
}

/* 
 * Create a connex for the union of two sets with the passed connexes.
 * (Returns MET_NULL_CONNEX if that can't be done in a nice way).
 */

METconnex *
MET_connex_union(connex1, connex2)
   METconnex	*connex1, *connex2;
{
   int		i1, i2, j, num1, num2;
   METconnex 	*result, *temp1, *temp2;
   Boolean	OK = YES;
   MME(MET_connex_union);

   for (num1 = 1, i1 = 0; i1 < connex1->ndims; i1++)
      if (connex1->dims[i1] < 0) {
	 if (MET_CONNEX_TYPE_MESH == connex1->types[i1])
	    num1 *= -connex1->dims[i1];
	 else
	    num1 *= (connex1->dims[i1] - 1) * (connex1->dims[i1]) / 2;
      } else 
	 break;

   for (num2 = 1, i2 = 0; i2 < connex2->ndims; i2++)
      if (connex2->dims[i2] < 0) {
	 if (MET_CONNEX_TYPE_MESH == connex2->types[i2])
	    num2 *= -connex2->dims[i2];
	 else
	    num2 *= (connex2->dims[i2] - 1) * (connex2->dims[i2]) / 2;
      } else 
	 break;

   /* connected parts are different, so give up */
   if (connex1->ndims - i1 != connex2->ndims - i2)
      return MET_NULL_CONNEX;

   num1 = - (num1 + num2);
   num2 = MET_CONNEX_TYPE_MESH;
   
   /* both are point sets so connectivity doesn't matter */
   if (i1 == connex1->ndims && i2 == connex2->ndims) 
      return MET_connex_new(1, &num1, &num2);
   
   for (j = 0; j < connex1->ndims - i1; j++)
      if (connex1->dims[i1 + j] != connex2->dims[i2 + j] ||
	  connex1->types[i1 + j] != connex2->types[i2 + j]) {
	 OK = NO;
	 break;
      }

   if (NO == OK)
      return MET_NULL_CONNEX;

   /* connected parts are the same */
   temp1 = MET_connex_new(1, &num1, &num2);
   temp2 = MET_connex_new(connex1->ndims - i1, connex1->dims + i1,
			  connex1->types + i1);
   result = MET_connex_cross(temp1, temp2);
   MET_CONNEX_FREE(temp1);
   MET_CONNEX_FREE(temp2);
   
   return result;
}

/*
 * return true if the connexes have the same number of elements
 * in the same dimensions.
 */

Boolean
MET_connex_equal(connex1, connex2)
   METconnex 	*connex1, *connex2;
{
   int i;
   MME(MET_connex_equal);

   if (connex1 == connex2)
      return YES;

   if (connex1->ndims != connex2->ndims)
      return NO;

   for (i = 0; i < connex1->ndims; i++)
      if (connex1->dims[i] != connex2->dims[i] ||
	  connex1->types[i] != connex2->types[i])
	 return NO;

   return YES;
}

/*
 * return the number of points in a connex,
 * or -1 times that if it is not discrete
 */
int
MET_connex_num_pts(connex)
   METconnex	*connex;
{
   int		i;
   int		total = 1;
   Boolean	discrete = YES;
   MME(MET_connex_num_pts);

   for (i = 0; i < connex->ndims; i++)
      if (connex->dims[i] > 0) {
	 if (MET_CONNEX_TYPE_MESH == connex->types[i])
	    total *= connex->dims[i];
	 else
	    total *= connex->dims[i]*(connex->dims[i]+1)/2;
	 discrete = NO;
      } else
	 if (MET_CONNEX_TYPE_MESH == connex->types[i])
	    total *= -connex->dims[i];
	 else 
	    total *= connex->dims[i]*(connex->dims[i]-1)/2;

   if (NO == discrete)
      return -total;

   return total;
}
   

#define FUNC MET__connex_new_free_list
#define LIST MET__connex_free_list
#define TYPE METconnex
#define MM_TYPE MET__MM_TYPE_CONNEX
#include "new_free_list.c"
#undef FUNC
#undef LIST
#undef TYPE
#undef MM_TYPE

Boolean
METconnex_write(connex, buf, len)
   METconnex		*connex;
   unsigned char	**buf;
   int			*len;
{
   int			i;
   MME(METconnex_write);
   
   if (2 * 4 * connex->ndims + 4 > *len)
      return FAILURE;

   WRITE_INTEGER(*buf, connex->ndims);
   for (i = 0; i < connex->ndims; i++)
      WRITE_INTEGER(*buf, connex->dims[i]);

   for (i = 0; i < connex->ndims; i++)
      WRITE_INTEGER(*buf, connex->types[i]);

   *len -= 2 * 4 * connex->ndims + 4;
   return SUCCESS;
}

METconnex *
METconnex_read(buf, len, used)
   unsigned char	*buf;
   int			len;
   int			*used;
{
   int			ndims;
   int                  i;
   unsigned char	*temp;
   MME(METconnex_read);
   
   if (4 > len)
      return MET_NULL_CONNEX;
   
   READ_INTEGER(buf, ndims);
   len -= sizeof(int);

   if (2 * 4 * ndims > len)
      return MET_NULL_CONNEX;

   *used += 2 * 4 * ndims + 4;
   
   for (i = 0, temp = buf; i < 2 * ndims; i++)
      READ_INTEGER(temp, *(int *) temp);

   return MET_connex_new(ndims, CAST(int *, buf), 
			 CAST(int *, buf) + ndims);
}

#ifdef MET__DEBUG_PRINT
void
METconnex_print(stream, connex, indent_level)
   FILE		*stream;
   METconnex	*connex;
   int		indent_level;
{
   int i;

   MME(METconnex_print);
   CHECK_CONNEX(connex);

   if (FNULL == stream)
      stream = stdout;

   MET_indent(stream, indent_level);
   PRINT(stream, "connex at %#x: ref_count = %d, ndims = %d, dims = ",
	 connex, connex->mminfo.ref_count, connex->ndims);
   for (i = 0; i < connex->ndims; i++)
      PRINT(stream, "(%d %d)", connex->dims[i], connex->types[i]);
   PRINT(stream, "\n");
}

/*ARGSUSED*/
Boolean
MET__connex_has_ref(connex, mm_type, object)
   METconnex	*connex;
   int		mm_type;
   MET_object	*object;
{
   MME(MET__connex_has_ref);

   return NO;
}
#endif MET__DEBUG_PRINT
