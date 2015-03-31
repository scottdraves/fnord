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
   "$Id: time.c,v 1.9 1992/06/22 17:08:33 mas Exp $";
#endif

#include "global/global.h"
#include "port/time.h"

/* Declare Time routines */
#if defined(SYSV) && !defined(stellar) && !defined(IBM6000)
#include <time.h>		/* stellar sys/time.h includes time.h */
#else
#include <sys/time.h>
#endif
#include <sys/resource.h>

#define RESOURCE_MAX 10
typedef struct RUsage RUsage;
struct RUsage {
   int    faults;      /* how many */
   double user_time;   /* seconds */
   double system_time; /* seconds */
   double memory;      /* Mb */
};

static RUsage resource_stack[RESOURCE_MAX];
static int    resource_index = -1;

static void
time_sample(r)
   RUsage *r;
{
   struct rusage ru;

#ifndef SYSV
   (void) getrusage(RUSAGE_SELF, &ru);
   r->faults = ru.ru_majflt;
#ifdef SunOS
   r->memory = ru.ru_maxrss * (double)getpagesize() / 1e6;
#else
   r->memory = ru.ru_maxrss * 1000.0 / 1e6;
#endif
   r->user_time =
      ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6;
   r->system_time =
      ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6;
#else
   r->faults = 0;
   r->user_time = r->system_time = r->memory = 0.0;
#endif SYV
}


void
time_begin()
{
   if (-1 == resource_index) {
      resource_index = 0;
      resource_stack[0].faults      = 0;
      resource_stack[0].user_time   = 0.0;
      resource_stack[0].system_time = 0.0;
      resource_stack[0].memory      = 0.0;
   } else {
      time_sample(resource_stack + resource_index);
   }
   resource_index++;
}

void
time_end(stream, name)
   FILE *stream;
   char *name;
{
   RUsage *begin = resource_stack + resource_index - 1;
   RUsage *end   = resource_stack + resource_index;

   time_sample(end);

   if (FNULL == stream)
      stream = stdout;

   if (0.02 < end->user_time - begin->user_time)
      fprintf(stream, "%s %5g %5g %5d %5g\n", name,
	      end->user_time - begin->user_time,
	      end->system_time - begin->system_time,
	      end->faults - begin->faults,
	      end->memory - begin->memory);

   resource_index--;
}
