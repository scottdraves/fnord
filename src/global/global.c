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
   "$Id: global.c,v 1.5 1992/06/22 17:03:56 mas Exp $";
#endif

#include "global.h"

Globals global_globals;

static int
global__log(n)
   int n;
{
   int base, i, j, result;
   ME(global__log);

   ALLOC__HISTOGRAM_SIZE;

   if (0 == n)
      return 0;

   base = 1;
   result = 1;

   for (i = 0; i < ALLOC__HISTOGRAM_SIZE; i++) {
      for (j = 1; j < ALLOC__HISTOGRAM_BASE; j++) {
	 if (n <= j * base)
	    return result;
	 result++;
      }
      base *= ALLOC__HISTOGRAM_BASE;
   }

   return result;
}


void
global_tron()
{
   ME(global_tron);
   
   global_globals.tracing = ON;
   ERR_ERROR(global_globals.error_id, ERR_INFO, (ERR_S, "tracing enabled"));
}

void
global_troff()
{
   ME(global_troff);

   global_globals.tracing = OFF;
}

FOboolean
global_trace(trace)
   FOboolean	trace;
{
   FOboolean	old_value = global_globals.tracing;
   ME(global_trace);

   if (trace == ON)
      global_tron();
   else
      global_troff();

   return old_value;
}

void
global_init()
{
   int		i;


   global_globals.error_id = ERRregister_package("global");
   global_globals.tracing = NO;
   global_globals.frees = global_globals.bytes_freed = 0;
   global_globals.mallocks = 0;
   global_globals.bytes_mallocked = 0;

   ERRset_core_dump_level(global_globals.error_id, ERR_FATAL);

   for (i = 0; i < ALLOC__HISTOGRAM_SIZE * ALLOC__HISTOGRAM_BASE + 1; i++)
      global_globals.histogram[i] = 0;
}

static void
global__print_bar(n, stream)
   int n;
   FILE *stream;
{
   int i;
   ME(global__print_bar);

   for (i = 0; i < n; i++)
      if (i % ALLOC__HISTOGRAM_BASE)
	 (void) putc('*', stream);
      else
	 (void) putc('|', stream);
}

void
global_print_memory_statistics(stream)
   FILE		*stream;
{
   int i, j, k, scan, size, base, last_entry;
   ME(global_print_memory_statistics);

   (void) fprintf(stream, "Memory statistics on %s\n",
		  ERR_PACKAGE_NAME(global_globals.error_id));

   (void) fprintf(stream, "             frees %8d\n",
		  global_globals.frees);
   (void) fprintf(stream, "          mallocks %8d\n",
		  global_globals.mallocks);
   (void) fprintf(stream, "outstanding blocks %8d\n",
		  global_globals.mallocks - global_globals.frees);
   (void) fprintf(stream, "       bytes freed %8d\n",
		  global_globals.bytes_freed);
   (void) fprintf(stream, "   bytes mallocked %8d\n",
		  global_globals.bytes_mallocked);
   (void) fprintf(stream, " outstanding bytes %8d\n",
		  global_globals.bytes_mallocked - global_globals.bytes_freed);

   (void) fprintf(stream, "%9s", "size");
   for (i = 0; i < ALLOC__HISTOGRAM_SIZE; i++) {
      (void) putc('|', stream);
      for (j = 1; j < ALLOC__HISTOGRAM_BASE; j++)
	 (void) putc(' ', stream);
   }
   (void) putc('\n', stream);

   last_entry = 1;
   for (i = ALLOC__HISTOGRAM_BASE * ALLOC__HISTOGRAM_SIZE - 1; i >= 0 ; i--)
      if (global_globals.histogram[i] > 0) {
	 last_entry = i;
	 break;
      }
   scan = 1;
   size = 1;
   base = 1;
   for (i = 0; i < ALLOC__HISTOGRAM_SIZE; i++) {
      for (j = 1; j < ALLOC__HISTOGRAM_BASE; j++) {
	 (void) fprintf(stream, "%9d", size);
	 size += base;
	 k = global__log(global_globals.histogram[scan++]);
	 if (scan > last_entry) {
	    i = ALLOC__HISTOGRAM_SIZE;
	    break;
	 }
	 global__print_bar(k, stream);
	 (void) putc('\n', stream);
      }
      base *= ALLOC__HISTOGRAM_BASE;
   }
}

#ifdef GLOBAL__DEBUG_TRACE
void
global__print_source(line, func, file)
   int	line;
   char	*func, *file;
{
   char buf[2000];
   ME(global__print_source);

   (void) sprintf(buf, "cat -n %s | head -%d | tail -10", file, line + 5);
   (void) system(buf);
   ERR_ERROR(global_globals.error_id, ERR_INFO,
	     (ERR_ARB, "in %s on line %d of %s",
	      func, line, file));
}

void
global__stats_malloc(size)
   int	size;
{
   ME(global__stats_malloc);

   global_globals.histogram[global__log(size)]++;
}
#endif GLOBAL__DEBUG_TRACE


int
global_syserror(code)
   int code;
{
   if (-1 == code)
      perror(CNULL);
   return code;
}
