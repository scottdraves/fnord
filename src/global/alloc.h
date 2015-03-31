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
 *	alloc.h : defines for memory allocation and error checking
 *
 *	      A.K.A. wsd's memory leak tracker from hell
 *
 **********************************************************************/
#ifndef _ALLOC_H_
#define _ALLOC_H_

/* $Id: alloc.h,v 1.7 1992/06/22 17:03:51 mas Exp $ */

#include "port/mem.h"

#ifndef ALLOC__FREE_HOOK
#define  ALLOC__FREE_HOOK
#endif

#ifndef ALLOC__MALLOC_HOOK
#define  ALLOC__MALLOC_HOOK
#endif

/*
 * the allocation routines are altered when linting so they lint cleanly.
 */
#ifdef lint

#define FREE(PTR, SIZE) { \
   ALLOC__FREE_HOOK;	      \
   free((char*)(PTR));	       \
}
#define FREEN(PTR, TYPE, N) { \
   ALLOC__FREE_HOOK;	          \
   free((char*)(PTR));	           \
}
#define FREE_STR(PTR) { \
   ALLOC__FREE_HOOK;	    \
   free((char*)(PTR));	     \
}
#define ALLOC(PTR, TYPE, MSG) {      \
   ALLOC__MALLOC_HOOK;	                 \
   (PTR) = (TYPE *) NULL;		  \
   (void) malloc((unsigned) sizeof(TYPE)); \
   (void) printf(MSG);		            \
}
#define ALLOCM(PTR, TYPE) {          \
   ALLOC__MALLOC_HOOK;	                 \
   (PTR) = (TYPE *) NULL;		  \
   (void) malloc((unsigned) sizeof(TYPE)); \
}
#define ALLOCN(PTR, TYPE, N, MSG) {	    \
   ALLOC__MALLOC_HOOK;	                     \
   (PTR) = (TYPE *) NULL;		      \
   (void) malloc((unsigned) (N)*sizeof(TYPE)); \
   (void) printf(MSG); 			        \
}
#define ALLOCNM(PTR, TYPE, N) {	    \
   ALLOC__MALLOC_HOOK;	                     \
   (PTR) = (TYPE *) NULL;		      \
   (void) malloc((unsigned) (N)*sizeof(TYPE)); \
}
#define REALLOCN(PTR, TYPE, N, OLD_N, MSG) {	   \
   ALLOC__MALLOC_HOOK;	                     	    \
   ALLOC__FREE_HOOK;	                     	     \
   (PTR) = (TYPE *) NULL;		  	      \
   (void) realloc((char *)(PTR), (unsigned) (N)*sizeof(TYPE)); \
   (void) printf(MSG); 				        \
}
#define REALLOCNM(PTR, TYPE, N, OLD_N) {	   \
   ALLOC__MALLOC_HOOK;	                     	    \
   ALLOC__FREE_HOOK;	                     	     \
   (PTR) = (TYPE *) NULL;		  	      \
   (void) realloc((char *)(PTR), (unsigned) (N)*sizeof(TYPE)); \
}
#define STRDUP(P, S, MSG) { \
   ALLOC__MALLOC_HOOK;	        \
   (PTR) = (char *) NULL;	 \
   (void) printf(MSG);		  \
}
#define STRDUPM(P, S) { \
   ALLOC__MALLOC_HOOK;	    \
   (P) = (char *) NULL;      \
}

#define CAST(T, O)  ((T) ((O), NULL))

#else lint

#ifdef ALLOC__DEBUG_MEMORY
#define ALLOC__MALLOC(SIZE)			  \
   malloc((global_globals.mallocks++,	           \
	  global_globals.bytes_mallocked += (SIZE), \
	  global__stats_malloc(SIZE),	     \
	  (unsigned)(SIZE)))
#define FREE(PTR, SIZE) {	    \
   ALLOC__FREE_HOOK;	             \
   free((char*)(PTR));		      \
   (PTR) = NULL;	               \
   global_globals.frees++;	        \
   global_globals.bytes_freed += (SIZE); \
}
#define FREEN(PTR, TYPE, N) {    \
   ALLOC__FREE_HOOK;	             \
   free((char*)(PTR));		      \
   (PTR) = NULL;	               \
   global_globals.frees++;	        \
   global_globals.bytes_freed += (N) * sizeof(TYPE); \
}
#define FREE_STR(PTR) {	   \
   int ALLOC__free_temp = strlen(PTR); \
   ALLOC__FREE_HOOK;	             \
   free((char*)(PTR));		     \
   (PTR) = NULL;	              \
   global_globals.frees++;	       \
   global_globals.bytes_freed +=	\
      ALLOC__free_temp + 1;		 \
}
#define ALLOC__REALLOC_BODY(PTR, TYPE, N, OLD_N)			\
   global_globals.bytes_freed += (OLD_N) * sizeof(TYPE);	 \
   global_globals.bytes_mallocked += (N) * sizeof(TYPE);	  \
   global_globals.frees++;					   \
   global_globals.mallocks++;					    \
   if (((PTR) = (TYPE *) realloc((PTR),				     \
				 (unsigned) (N)*sizeof(TYPE))) == NULL)
#define STRDUP(P, S, MESG) {    \
   global_globals.mallocks++;	    \
   global_globals.bytes_mallocked += \
      strlen(S) + 1;		      \
   ALLOC__MALLOC_HOOK;	               \
   (P) = strdup(S);	     	        \
   if ((P) == NULL)	      		 \
      FATAL((ERR_ALLOC1, MSG));		  \
}
#define STRDUPM(P, S) { 	   \
   global_globals.mallocks++;	    \
   global_globals.bytes_mallocked += \
      strlen(S) + 1;		      \
   ALLOC__MALLOC_HOOK;	              \
   (P) = strdup(S);	     	       \
   if ((P) == NULL)	      		\
      FATAL((ERR_ARB, \
	     "Unable to duplicate %d length string from %s into %s", \
	     strlen(S), "S", "P"));		 \
}
#else ALLOC__DEBUG_MEMORY
#define ALLOC__MALLOC(SIZE) malloc((unsigned)(SIZE))
#ifdef ALLOC__DEBUG
#define FREE(PTR, SIZE) { \
   ALLOC__FREE_HOOK;	      \
   free((char*)(PTR));	       \
   (PTR) = NULL;	        \
}
#define FREEN(PTR, TYPE, N) { \
   ALLOC__FREE_HOOK;	          \
   free((char*)(PTR));	           \
   (PTR) = NULL;	            \
}
#else
#define FREE(PTR, SIZE) { \
   ALLOC__FREE_HOOK;	      \
   free((char*)(PTR));	       \
}
#define FREEN(PTR, TYPE, N) { \
   ALLOC__FREE_HOOK;	          \
   free((char*)(PTR));	           \
}
#endif
#define FREE_STR(PTR) FREE(PTR, 0)
#define ALLOC__REALLOC_BODY(PTR, TYPE, N, OLD_N)			  \
   if (((PTR) = (TYPE *) realloc((char*)(PTR),				   \
				 (unsigned) (N)*sizeof(TYPE))) == NULL)
#define STRDUP(P, S, MSG) { \
   ALLOC__MALLOC_HOOK;             \
   (P) = strdup(S);	         \
   if ((P) == NULL)	          \
      FATAL((ERR_ALLOC1, MSG));    \
}
#define STRDUPM(P, S) { \
   ALLOC__MALLOC_HOOK;	    \
   (P) = strdup(S);	     \
   if ((P) == NULL)	      \
      FATAL((ERR_ARB,          \
	     "Unable to duplicate %d length string from %s into %s", \
	     strlen(S), "S", "P"));		 \
}
	
#endif ALLOC__DEBUG_MEMORY

#define ALLOC(PTR, TYPE, MSG) {			   \
   ALLOC__MALLOC_HOOK;	          \
   if (((PTR) = (TYPE *) ALLOC__MALLOC(sizeof(TYPE))) == NULL) \
      FATAL((ERR_ALLOC1, MSG));			     \
}
#define ALLOCM(PTR, TYPE) {			             \
   ALLOC__MALLOC_HOOK;	          \
   if (((PTR) = (TYPE *) ALLOC__MALLOC(sizeof(TYPE))) == NULL)   \
      FATAL((ERR_ARB, "Unable to allocate type %s into %s", \
		"TYPE", "PTR"));			        \
}
#define ALLOCN(PTR, TYPE, N, MSG) {			       \
   ALLOC__MALLOC_HOOK;	          \
   if (((PTR) = (TYPE *) ALLOC__MALLOC((N)*sizeof(TYPE))) == NULL) \
      FATAL((ERR_ALLOC1, MSG));				 \
}
#define ALLOCNM(PTR, TYPE, N) {			         \
   ALLOC__MALLOC_HOOK;	          \
   if (((PTR) = (TYPE *) ALLOC__MALLOC((N)*sizeof(TYPE))) == NULL)   \
      FATAL((ERR_ARB,					   \
		"Unable to allocate %d length array of %s into %s", \
		(N), "TYPE", "PTR"));				     \
}
#define REALLOCN(PTR, TYPE, N, OLD_N, MSG) { \
   ALLOC__MALLOC_HOOK;	          \
   ALLOC__FREE_HOOK;	          \
   ALLOC__REALLOC_BODY(PTR, TYPE, N, OLD_N)	 \
      FATAL((ERR_REALLOC1, MSG));		  \
}
#define REALLOCNM(PTR, TYPE, N, OLD_N) {			  \
   ALLOC__MALLOC_HOOK;	          \
   ALLOC__FREE_HOOK;	          \
   ALLOC__REALLOC_BODY(PTR, TYPE, N, OLD_N)	 		   \
      FATAL((ERR_ARB,					    \
"Unable to reallocate %d length array of %s into %s from length %d", \
		(N), "TYPE", "PTR", (OLD_N)));			      \
}

#define CAST(T, O)	((T) (O))

#endif lint

#ifdef ALLOC__DEBUG_MAGIC

#define ALLOC__MAGIC_FIELD long magic;
#define ALLOC__MAGIC_FREE		0x62390d68L

#define ALLOC_FREE_MAGIC(PTR, SIZE) { \
   FREE(PTR, SIZE);		    \
   (PTR)->magic = ALLOC__MAGIC_FREE;    \
}
#define ALLOC_ADD_MAGIC(PTR, MAGIC) { \
   (PTR)->magic = MAGIC;       	    \
}
#define ALLOC_CHECK_PTR(PTR) {		          \
   if ((PTR) == NULL)			           \
      FATAL((ERR_SS, "PTR", "pointer is null")); \
}
#define ALLOC_CHECK_MAGIC(PTR, MAGIC) {           \
   ALLOC_CHECK_PTR(PTR);				\
   if ((int)(PTR) & (MEM_ALIGN_LONG-1)) {	 \
      FATAL((ERR_SS, "PTR", "is misaligned"))  \
   }						   \
   if ((PTR)->magic != (MAGIC)) {		    \
      if ((PTR)->magic != ALLOC__MAGIC_FREE) {	     \
	 FATAL((ERR_SS, "PTR", "has bad magic"));  \
      } else {					       \
	 FATAL((ERR_SS, "PTR", "has free magic"));   \
      }						         \
   }						          \
}
#define ALLOC_CHECK_NULL_PTR(PTR)
#define ALLOC_CHECK_NULL_MAGIC(PTR, MAGIC) { \
   if (NULL != (PTR)) {			   \
      ALLOC_CHECK_MAGIC(PTR, MAGIC);	    \
   }					     \
}

#else ALLOC__DEBUG_MAGIC
#define ALLOC__MAGIC_FIELD
#define ALLOC_FREE_MAGIC(PTR, SIZE)
#define ALLOC_ADD_MAGIC(PTR, MAGIC)
#define ALLOC_CHECK_PTR(PTR)
#define ALLOC_CHECK_MAGIC(PTR, MAGIC)
#define ALLOC_CHECK_NULL_PTR(PTR)
#define ALLOC_CHECK_NULL_MAGIC(PTR, MAGIC)
#endif ALLOC__DEBUG_MAGIC

#endif /* undef(_ALLOC_H_) */
