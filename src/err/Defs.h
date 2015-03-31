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

/* Internal ERR defs */

#define MAX_LEN		2048		/* Max len of an error message */

#define MAGIC		2137

#ifdef lint
#define MALLOC(P, T, N, M1, M2) { 	    \
   (P) = (T *) NULL;			     \
   (void) malloc((unsigned) (N) * sizeof(T)); \
}
#define REALLOC(P, T, N, M1, M2) {			  \
   (P) = (T *) NULL;					   \
   (void) realloc((char *)(P), (unsigned) (N) * sizeof(T)); \
}
   
#else
#define MALLOC(P, T, N, M1, M2) { \
   if (((P) = (T *) malloc((unsigned) (N) * sizeof(T))) == NULL) 		\
      FPRINTF(stderr, "ERR: Couldn't allocate space for %s in %s.\n", M1, M2); \
}

#define REALLOC(P, T, N, M1, M2) {	 	       \
   if (((P) = (T *) realloc((char *)(P), (unsigned) (N) * sizeof(T))) == NULL)   \
      FPRINTF(stderr, "ERR: Couldn't allocate space for %s in %s.\n", M1, M2); \
}
#endif

#define CHECK_MAGIC(ID, RTN)						\
  if ((ID) == NULL || MAGIC_NUM(ID) != MAGIC) {				\
    FPRINTF(stderr, "ERR fatal: Invalid ERRid passed to %s.\n", (RTN)); \
    exit(1);								\
  }

#define MAGIC_NUM(ID)	    ((ID)->magic)
#define NUMS(ID) 	    ((ID)->err_nums)
#define COUNTS(ID)	    ((ID)->err_cnts)
#define NUM_COUNTS(ID)	    ((ID)->num_cnts)
#define CORE_DUMP_LEVEL(ID) ((ID)->core_level)

#define FPRINTF		(void) fprintf

#define TRUE  1
#define FALSE 0
/* $Id: defs.h,v 1.5 1992/06/22 17:03:16 mas Exp $ */
