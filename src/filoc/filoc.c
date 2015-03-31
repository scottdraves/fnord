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
static char Version[]=
   "$Id: filoc.c,v 1.6 1992/06/22 17:03:31 mas Exp $";
#endif

#include "filoc.h"
#include "err/err.h"

void
FILOCinitialize()
{
}

FILOCpool *
FILOCpool_new()
{
   FILOCpool *pool;
   ME(FILOCpool_new);

   ALLOCM(pool, FILOCpool);

   pool->free_list = NULL;
   pool->pool = NULL;
   pool->next = NULL;

   return pool;
}

void
FILOC_pool_expand(pool)
   FILOCpool *pool;
{
   FILOCpool *new_pool;
   int i;
   ME(FILOC_pool_expand);

   /* create a new pool and link it into the pool list */
   ALLOCM(new_pool, FILOCpool);
   new_pool->free_list = NULL;
   new_pool->next = pool->next;
   pool->next = new_pool;

   /* allocate a block of FILOClocs for the pool, and link them into
    * the pool free list */
   ALLOCNM(new_pool->pool, FILOCloc, FILOC_POOL_SIZE_DEFAULT);
   for (i = FILOC_POOL_SIZE_DEFAULT; i--;)
   {
      new_pool->pool[i].next = pool->free_list;
      pool->free_list = &(new_pool->pool[i]);
   }
}

void
FILOCpool_free(pool)
   FILOCpool *pool;
{
   FILOCpool *next;
   ME(FILOCpool_free);

   while (pool != NULL)
   {
      next = pool->next;

      if (pool->pool != NULL)
	 FREEN(pool->pool, FILOCloc, FILOC_POOL_SIZE_DEFAULT);

      FREE(pool, sizeof(FILOCpool));

      pool = next;
   }
}

FILOCloc *
FILOC_loc_alloc(pool)
   FILOCpool *pool;
{
   FILOCloc *loc;
   ME(FILOC_loc_alloc);

   while (pool->free_list == NULL)
      FILOC_pool_expand(pool);

   loc = pool->free_list;

   pool->free_list = pool->free_list->next;

   return loc;
}

FILOCloc *
FILOCloc_new(pool, file, begin_line, begin_col, end_line, end_col)
   FILOCpool *pool;
   char *file;
   int begin_line, begin_col, end_line, end_col;
{
   FILOCloc *loc;
   ME(FILOCloc_new);

   loc = FILOC_loc_alloc(pool);

   loc->filoc.filename = file;
   loc->filoc.data.region[FILOC_LOC_LINE_BEGIN] = begin_line;
   loc->filoc.data.region[FILOC_LOC_COL_BEGIN] = begin_col;
   loc->filoc.data.region[FILOC_LOC_LINE_END] = end_line;
   loc->filoc.data.region[FILOC_LOC_COL_END] = end_col;

   return loc;
}

FILOCloc *
FILOCloc_union(pool, loc1, loc2)
   FILOCpool *pool;
   FILOCloc *loc1, *loc2;
{
   FILOCloc *loc;
   ME(FILOCloc_union);

   loc = FILOC_loc_alloc(pool);

   loc->filoc.filename = NULL;
   loc->filoc.data.kids[0] = loc1;
   loc->filoc.data.kids[1] = loc2;

   return loc;
}

char *
FILOCloc_file_begin(loc)
   FILOCloc *loc;
{
   ME(FILOCloc_file_begin);

   while (loc->filoc.filename == NULL)
      loc = loc->filoc.data.kids[0];

   return loc->filoc.filename;
}

int
FILOCloc_line_begin(loc)
   FILOCloc *loc;
{
   ME(FILOCloc_line_begin);

   while (loc->filoc.filename == NULL)
      loc = loc->filoc.data.kids[0];

   return loc->filoc.data.region[FILOC_LOC_LINE_BEGIN];
}

int
FILOCloc_col_begin(loc)
   FILOCloc *loc;
{
   ME(FILOCloc_col_begin);

   while (loc->filoc.filename == NULL)
      loc = loc->filoc.data.kids[0];

   return loc->filoc.data.region[FILOC_LOC_COL_BEGIN];
}

char *
FILOCloc_file_end(loc)
   FILOCloc *loc;
{
   ME(FILOCloc_file_end);

   while (loc->filoc.filename == NULL)
      loc = loc->filoc.data.kids[1];

   return loc->filoc.filename;
}

int
FILOCloc_line_end(loc)
   FILOCloc *loc;
{
   ME(FILOCloc_line_end);

   while (loc->filoc.filename == NULL)
      loc = loc->filoc.data.kids[1];

   return loc->filoc.data.region[FILOC_LOC_LINE_END];
}

int
FILOCloc_col_end(loc)
   FILOCloc *loc;
{
   ME(FILOCloc_col_end);

   while (loc->filoc.filename == NULL)
      loc = loc->filoc.data.kids[1];

   return loc->filoc.data.region[FILOC_LOC_COL_END];
}

void
FILOCloc_print(loc, file, s, slen)
   FILOCloc *loc;
   FILE *file;
   char *s;
   int slen;
{
   static int last_begin, last_end;
   static char *last_file;
   int begin, end;
   char *filename;
   char buf[50];
   ME(FILOCloc_print);

   if (loc == NULL) 
   {
      last_begin = last_end = 0;
      last_file = (char *) NULL;
      if (file == NULL) {
	 (void) sprintf(s, "%.*s", slen - 1, "unknown location");
      } else 
	 fprintf(file, "unknown location");
   }
   else 
   {
      begin = FILOCloc_line_begin(loc);
      end = FILOCloc_line_end(loc);
      filename = FILOCloc_file_begin(loc);

      if (begin == last_begin && end == last_end &&
	  last_file == filename) 
      {
	 if (file == NULL)
	    (void) sprintf(s, "%.*s", slen - 1, "\t(same)\t");
	 else 
	    fprintf(file, "\t(same)\t");
      }
      else 
      {
	 last_begin = begin;
	 last_end = end;
	 last_file = filename;
	 if (file == NULL) {
	    if (begin == end)
	       (void) sprintf(buf, "line %d of input ", begin);
	    else
	       (void) sprintf(buf, "line %d to line %d of input ",
			      begin, end);
	    (void) sprintf(s, "%.*s", slen - 1, buf);
	    begin = strlen(s);
	    s += strlen(s);
	    slen -= strlen(s);
	    strncat(s, filename, slen - 1);
	 }
	 else 
	 {
	    if (begin == end)
	       fprintf(file, "line %d of input \"%s\"",
		       begin, FILOCloc_file_begin(loc));
	    else
	       fprintf(file, "line %d to line %d of input \"%s\"",
		       begin, end, FILOCloc_file_begin(loc));
	 }
      }
   }      
}
