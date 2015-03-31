/* Copyright 1990, Brown University.  All Rights Reserved. *//*
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
 $Id: object_new_list.c,v 1.3 1992/06/22 17:05:26 mas Exp $ */

/*
 * This is included by client .c files.  It is called if we try
 * to allocate an object and there are none in the free list.  This adds new
 * objects to the free list.
 */

/*
 * this calls malloc and grabs a big chunk of memory, and treats it as an
 * array of objects.  It then goes through array and links each object to
 * the next, making a linked list.  All of them are added to the free list
 * by setting the free list to point to the first one.
 *
 * the big array is called a chunk.  it has a header of type
 * object__chunk_header, which puts all chunks for a given
 * MM_TYPE in a linked list.
 */

/*
 * FUNC - name of this function
 * LIST - head of free list
 * C_TYP - C type of objects in that list
 * MM_TYPE - object_type_pkg_xxx constant
 * CHUNK_SIZE - how many objects in the chunk
 */

void
FUNC()
{
   int		i, n;
   char		*c;
   object__chunk_header *header;
   ME(object_new_free_list);

   n = object__round_up(CHUNK_SIZE, sizeof(C_TYP));
   ALLOCNM(c, char, (n * sizeof(C_TYP) +
	   sizeof(object__chunk_header)));

#ifndef lint
   header = (object__chunk_header *) c;
#else
   header = NULL_CHUNK_HEADER;
#endif
   header->next = object__chunk_list[MM_TYPE];
   object__chunk_list[MM_TYPE] = header;

#ifndef lint
   LIST = (C_TYP *) (c + sizeof(object__chunk_header));
#else
   (void) printf("%#x\n", c);
   LIST = ((C_TYP *) NULL);
#endif
   header->first_of_chunk = (char *) LIST;

   for (i = 0; i < n - 1; i++)
      LIST[i].mminfo.next = LIST + i + 1;
   LIST[n - 1].mminfo.next = (C_TYP *) NULL;

#ifdef object__debug_magic
   for (i = 0; i < n; i++)
      LIST[i].magic = object__magic_free;

   object__count_chunks[MM_TYPE]++;
#endif
}
