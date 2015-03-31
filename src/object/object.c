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
   "$Id: object.c,v 1.3 1992/06/22 17:05:21 mas Exp $";
#endif

#include "object.h"

int object__count_copies[object_type_max];
int object__count_deep_copies[object_type_max];
int object__count_allocs[object_type_max];
int object__count_frees[object_type_max];
int object__count_deep_frees[object_type_max];
int object__count_chunks[object_type_max];
long object__magic[object_type_max] = {
   0x82a048f2L,
   0x7a010d33L,
   0x486abb71L,
   0x226cf08aL,
   0xb9089348L,
   0x9936a185L,
   0x8d32b425L,
   0xd6717e72L,
   0xca5dbfecL,
};
object__chunk_header *object__chunk_list[object_type_max];

void
object_initialize()
{
   int		i;
   ME(object_initialize);
   
   for (i = 0; i < object_type_max; i++) {
      object__count_copies[i] =
	 object__count_deep_copies[i] =
	    object__count_allocs[i] =
	       object__count_frees[i] =
		  object__count_deep_frees[i] =
		     object__count_chunks[i] = 0;
      object__chunk_list[i] = object__null_chunk;
   }
}

/*
 * round num_objects up for malloc efficiency
 */
int
object__round_up(num_objects, object_size)
   int		num_objects;
   int		object_size;
{
   int		size, bits_used = 0;
   ME(object__round_up);
   
   size = num_objects * object_size;
   /*
    * count # of bits in size.  this is so we can round it
    * up to the nearest power of two.
    */
   while (size) {
      bits_used++;
      size >>= 1;
   }
   size = 1 << bits_used;
   return (size - (MEM_MALLOC_HEADER +
		   sizeof(object__chunk_header))) /
      object_size;
}
