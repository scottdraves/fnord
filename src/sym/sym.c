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

#include <stdio.h>
#include "global/global.h"
#include "port/mem.h"
#include "sym/sym.h"
#include "err/err.h"
#include "global/alloc.h"

#ifndef lint
static char Version[] =
   "$Id: sym.c,v 1.9 1992/06/22 17:09:05 mas Exp $";
#endif /* undef lint */


static Sym *sym_table;
static int sym_table_size;
static char *sym_area;
static int sym_room;

/********************************************************************
 *
 *   Hash function for strings originally suggested by P.J. Weinstein:
 *     see Aho, Sethi, and Ullman, _Compilers:_Principles,_Techniques,
 *     _and_Tools, pp 435-437
 *
 ********************************************************************/
static unsigned int
sym_hashpjw(s)
   char *s;
{
   unsigned int h = 0;
   unsigned int g;
   ME(sym_hashpjw);
   
   while (*s)
   {
      h = (h << 4) + *s;

      if (g = h & 0xf0000000)
	  h ^= g ^ (g >> 24);
      s++;
   }

   return h;
}


/********************************************************************
 *
 *   Initialize the sym package with a hash table size of tablesize.
 * If tablesize is not a prime you will regret it.  At some point we
 * should take the least prime greater than or equal to the given
 * tablesize to be the real tablesize.
 *
 ********************************************************************/
void
sym_init(tablesize)
   int tablesize;
{
   int i;
   Sym *table;
   ME(sym_init);

   sym_table_size = tablesize;
   ALLOCNM(sym_table, Sym, tablesize);

   /* initialize the table to NULLs */
   table = sym_table;
   i = sym_table_size;
   while (i--)
   {
      *table = NULL;
      table++;
   }

   sym_area = NULL;
   sym_room = 0;
}

Sym
sym_from_string(s)
   char *s;
{
   int hash = sym_hashpjw(s);
   Sym *entry = &sym_table[hash % sym_table_size];
   int size;
   ME(sym_from_string);

   while (*entry != NULL)
   {
      if ((*entry)->hash == hash)
	 if (!strcmp((*entry)->s, s))
	    return *entry;
      entry = &(*entry)->next;
   }
   /* if we've gotten to this point, there is no entry in the symbol
    * table, so we insert one. */
   /* note this means there are strlen(s) + 1 characters: 1 extra for
    *  the '\0' */
   size = sizeof(struct Sym_struct) + strlen(s);
   size = MEM_ALIGN_UP_INT(int, size);

   if (size > sym_room)
   {
      sym_room = (SYM_AREA_SIZE_DEFAULT > size
		  ? SYM_AREA_SIZE_DEFAULT
		  : size);
      ALLOCNM(sym_area, char, sym_room);
   }

   *entry = CAST(Sym, sym_area);
   sym_room -= size;
   sym_area += size;

   (*entry)->hash = hash;
   (*entry)->next = NULL;
   (void) strcpy((*entry)->s, s);

   return *entry;
}

void
sym_table_print(stream)
   FILE *stream;
{
   Sym *bucket = sym_table;
   Sym entry;
   int i = sym_table_size;
   ME(sym_table_print);

   if (stream == NULL)
      stream = stdout;

   while (i--)
   {
      entry = *bucket;

      while (entry != NULL)
      {
	 fprintf(stream, "\"%s\":\tSym 0x%x\t hash 0x%x\n",
		 SYM_STRING(entry), (int) entry, entry->hash);

	 entry = entry->next;
      }
      bucket++;
   }
}

void
sym_stats_print(stream)
   FILE *stream;
{
   Sym *bucket = sym_table;
   Sym entry;
   int i = sym_table_size;
   int hist[80];
   int j;
   int nentries = 0, maxdepth = 0;
   double avgdepth = 0.0;
   ME(sym_stats_print);

   if (stream == NULL)
      stream = stdout;

   while (i--)
   {
      entry = *bucket;

      j = 1;

      while (entry != NULL)
      {
	 hist[j]++;

	 nentries++;

	 if (j > maxdepth)
	    maxdepth = j;

	 avgdepth += (double) j;

	 j++;
	 entry = entry->next;
      }
      entry++;
   }

   avgdepth /= (double) nentries;

   fprintf(stream, "Sym storage:\n");
   fprintf(stream, "table size = %d\n", sym_table_size);
   fprintf(stream, "# of syms = %d\n", nentries);
   fprintf(stream, "maximum depth = %d, \taverage depth = %d\n",
	   maxdepth, avgdepth);
   fprintf(stream, "efficiency ratio = %g\n", (double) (nentries) /
	   (double) (avgdepth * sym_table_size));
   fprintf(stream, "histogram:\n");

   for (i = 0; i < maxdepth; i++)
   {
      /* scale histogram? */

      j = 79;		/* maximum width */
      while (hist[i] > 0 && --j > 0)
	 putc(' ', stream);

      if (j > 0)
	 putc('*', stream);
      else
	 putc('>', stream);
      putc('\n', stream);
   }
}

