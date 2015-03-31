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

/* $Id: filoc.h,v 1.5 1992/06/22 17:03:34 mas Exp $ */

#ifndef _FILOC_H_
#define _FILOC_H_

#include "global/global.h"

/* -----------------------------  Constants  ------------------------------ */


/* offsets into the loc.filoc.data.region array */
#define FILOC_LOC_LINE_BEGIN	0
#define FILOC_LOC_COL_BEGIN	1
#define FILOC_LOC_LINE_END	2
#define FILOC_LOC_COL_END	3

/* default size of a FILOCpool */
#define FILOC_POOL_SIZE_DEFAULT 256


/* ------------------------------  Types  --------------------------------- */

typedef union filoc_union {
   union filoc_union *next;		/* for linked free list */
   struct {
      char *filename;			/* NULL means a union-node */
      union {
	 int region[4];		/* begin and end line and column */
	 union filoc_union *kids[2];	/* children for union */
      } data;
   } filoc;
} FILOCloc;

typedef struct filoc_pool {
   FILOCloc *free_list;			/* linked list of free filocs */
   FILOCloc *pool;			/* arena for later freeing */
   struct filoc_pool *next;		/* pointer to another pool
					   with another arena to free */
} FILOCpool;

/* ------------------------------  Macros  -------------------------------- */

/* ------------------------------  Entries  ------------------------------- */

/* initialize the package */
extern void		FILOCinitialize();

/* create a new pool */
extern FILOCpool *	FILOCpool_new();
/* destroy a pool, including all the FILOCloc's within it */
extern void		FILOCpool_free(/* FILOCpool *pool */);

/* create a new FILOCloc from a FILOCpool and some location info */
extern FILOCloc *	FILOCloc_new(/* FILOCpool *pool, char *file,
					int begin_line, int begin_col,
					int end_line, int end_col
					*/);
/* create a new FILOCloc from a FILOCpool and two other FILOClocs */
extern FILOCloc *	FILOCloc_union(/* FILOCpool *pool,
					  FILOCloc *loc1, FILOCloc *loc2 */);

/* print a FILOCloc to stream FILE, or into string s if FILE == NULL */
extern void		FILOCloc_print(/* FILOC *loc, FILE *file, char *s,
				      int slen */);

/* get begin and end lines and columns from a filoc */
extern char *		FILOCloc_file_begin(/* FILOCloc *loc */);
extern int		FILOCloc_line_begin(/* FILOCloc *loc */);
extern int		FILOCloc_col_begin(/* FILOCloc *loc */);
extern char *		FILOCloc_file_end(/* FILOCloc *loc */);
extern int		FILOCloc_line_end(/* FILOCloc *loc */);
extern int		FILOCloc_col_end(/* FILOCloc *loc */);


#endif /* undef(_FILOC_H_) */
