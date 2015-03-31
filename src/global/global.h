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

/**********************************************************************
 *
 *	global include file defining many convenience routines
 *		for packages
 *
 **********************************************************************/
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/* $Id: global.h,v 1.6 1992/06/22 17:03:58 mas Exp $ */

#include "port/port.h"

#include <stdio.h>
#include <sys/types.h>

#include "err/err.h"

/* If you include X11/Intrinsic.h, you must make sure that this
   matches the type defined for Booleans there! */
typedef char         FOboolean;

#define ALLOC__HISTOGRAM_SIZE	6
#define ALLOC__HISTOGRAM_BASE	10

typedef struct {
   ERRid	*error_id;
   int		tracing;
   int		frees;
   int		bytes_freed;
   int		mallocks;
   int		bytes_mallocked;
   int		histogram[ALLOC__HISTOGRAM_SIZE * ALLOC__HISTOGRAM_BASE + 1];
} Globals;

extern Globals global_globals;


/*
 * values for booleans
 */
#define TRUE	1
#define FALSE	0
#define SUCCESS	1
#define FAILURE	0
#define YES	1
#define NO	0
#define ON	1
#define OFF	0

/*
 * sixteen different flavors of NULL
 */
#define CNULL      ((char   *) NULL)
#define INULL      ((int    *) NULL)
#define DNULL      ((double *) NULL)
#define FNULL	   ((FILE   *) NULL)

/* debugging stuff */
#ifdef DEBUG
#define GLOBAL__DEBUG_TRACE
#define ALLOC__DEBUG_MAGIC
#define ALLOC__DEBUG_MEMORY
#endif /* defined(DEBUG) */

#ifndef GLOBAL__ME_HOOK
#define GLOBAL__ME_HOOK
#endif /* undef(GLOBAL__ME_HOOK) */

#ifndef GLOBAL__WATCH_HOOK
#define GLOBAL__WATCH_HOOK
#endif /* undef(GLOBAL__WATCH_HOOK) */

#ifndef GLOBAL__ERROR_HOOK
#define GLOBAL__ERROR_HOOK
#endif /* undef(GLOBAL__ERROR_HOOK) */

#ifdef GLOBAL__DEBUG_TRACE
#define ME(NAME)			      \
static char *global_routine_name = "NAME";         \
GLOBAL__ME_HOOK;					\
if (global_globals.tracing == ON)	         \
      ERR_ERROR(global_globals.error_id, ERR_INFO, \
		(ERR_SS, "entering", global_routine_name))

#define WATCH(VALUE)	{		       \
if (VALUE) {					\
   GLOBAL__WATCH_HOOK;				 \
   WARNING((ERR_SSI, "watched value is true:", \
	       "VALUE", (VALUE)));		   \
}}

#define DEFAULT_ERR(VALUE)	 		       \
   FATAL((ERR_ARB, "switch fell through: %s was %d", \
		   "VALUE", (VALUE)))

#define GLOBAL__ERR(SEVERITY, TEXT) {                       \
   GLOBAL__ERROR_HOOK;				        \
   ERR_ERROR(global_globals.error_id, SEVERITY, TEXT);    \
}

#else
#define ME(NAME) GLOBAL__ME_HOOK
#define WATCH(VALUE)
#define DEFAULT_ERR(VALUE)

#define GLOBAL__ERR(SEVERITY, TEXT) {    \
   GLOBAL__ERROR_HOOK;		     \
   ERR_ERROR(global_globals.error_id, \
	     SEVERITY, TEXT);	       \
}
#endif


/*
 * these must be used with double ()s,
 * eg FATAL((ERR_ALLOC1, "out of memory"));
 */
#define INFO(TEXT)    GLOBAL__ERR(ERR_INFO, TEXT)
#define WARNING(TEXT) GLOBAL__ERR(ERR_WARNING, TEXT)
#define SEVERE(TEXT)  GLOBAL__ERR(ERR_SEVERE, TEXT)
#ifdef GLOBAL__DEBUG_TRACE
#define FATAL(TEXT)   { \
   GLOBAL__ERR(ERR_FATAL, TEXT); \
   global__print_source(__LINE__, global_routine_name, __FILE__); \
}
#else
#define FATAL(TEXT) GLOBAL__ERR(ERR_FATAL, TEXT);
#endif GLOBAL__DEBUG_TRACE

/*
 * these take an ordinary string
 * eg FATALS("time to die");
 */
#define INFOS(STRING)    GLOBAL__ERR(ERR_INFO, (ERR_S, STRING))
#define WARNINGS(STRING) GLOBAL__ERR(ERR_WARNING, (ERR_S, STRING))
#define SEVERES(STRING)  GLOBAL__ERR(ERR_SEVERE, (ERR_S, STRING))
#define FATALS(STRING)   FATAL((ERR_S, STRING))

#define NYET	         SEVERE((ERR_S, "not yet implemented"))

#include "alloc.h"

#include "float.h"

extern void       global_init();

extern void       global_tron();
extern void       global_troff();
extern FOboolean  global_trace();

extern void	  global_print_memory_statistics();
extern int	  global_syserror();
#ifdef GLOBAL__DEBUG_TRACE
extern void	  global__print_source();
extern void	  global__stats_malloc();
#endif GLOBAL__DEBUG_TRACE

#endif /* undef(_GLOBAL_H_) */
