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

/* $Id: defs.h,v 1.4 1992/06/22 17:08:46 mas Exp $ */

#ifndef SNIDEF_HAS_BEEN_INCLUDED
#define SNIDEF_HAS_BEEN_INCLUDED


#include "sni/sni.h"
#include "ops/defs.h"

/* the algirithm works in n dimensions, this is the max for n */
#define MAX_DIMS 10

typedef struct {
   /* input data */
   METreal	*src_base;	/* beginning of array of source points */
   int		width;		/* how many dimensions we have */
   int		dims;		/* how many of them we hash on */
   int		src_width;	/* vector stride of source points */
   int		num_pts;	/* lenght of vector of source points */
   METreal	radius;		/* max dist of interaction */
   METreal	radius_squared;	/* that squared */

   /* data structure */
   METreal	max[MAX_DIMS], min[MAX_DIMS]; /* bounding box */
   int		grid[MAX_DIMS];       /* size of hash table in each coord */
   int		grid_total[MAX_DIMS]; /* partial products of grid */
   int		num_buckets;	      /* total number of buckets */
   int		*buckets, *links;     /* first bucket and first link */

   /* output data */
   METreal	*pairs;		/* start of array of pairs of pts */
   METreal	*last_pair;	/* beyond last allocated pair */
   METreal	*dst;		/* write output here */
   int		num_pairs;	/* (dst - pairs)/(2width) */

   
} hash_globals;
   

#endif SNIDEF_HAS_BEEN_INCLUDED
