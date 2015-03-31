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

/* $Id: new_free_list.c,v 1.4 1992/06/22 17:05:10 mas Exp $ */

/*
 * This is included by each of the main .c files.  It is called if we try
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
 * MET_chunk_header, which puts all chunks for a given MM_TYPE in a linked
 * list.
 */

/*
 * FUNC - name of this function
 * LIST - head of free list
 * TYPE - C type of objects in that list
 * MM_TYPE - MET__MM_TYPE_XXX constant
 */

void
FUNC()
{
   int		i, n;
   char		*c;
   MET_chunk_header *header;
   MME(MET_xxx_new_free_list);

   n = MET_chunk_size[MM_TYPE];
   ALLOCNM(c, char, (n * MET_obj_size[MM_TYPE]) +
	   sizeof(MET_chunk_header));

#ifndef lint
   header = (MET_chunk_header *) c;
#else
   header = NULL_CHUNK_HEADER;
#endif
   header->next = MET_chunk_list[MM_TYPE];
   MET_chunk_list[MM_TYPE] = header;

#ifndef lint
   LIST = (TYPE *) (c + sizeof(MET_chunk_header));
#else
   (void) printf("%#x\n", c);
   LIST = ((TYPE *) NULL);
#endif
   header->first_of_chunk = (MET_object *) LIST;

   for (i = 0; i < n - 1; i++)
      LIST[i].mminfo.next = LIST + i + 1;
   LIST[n - 1].mminfo.next = (TYPE *) NULL;

#ifdef MET__DEBUG_MAGIC
   for (i = 0; i < n; i++)
      LIST[i].magic = MET__MAGIC_FREE;

   MET_count_chunks[MM_TYPE]++;
#endif MET__DEBUG_MAGIC
}

