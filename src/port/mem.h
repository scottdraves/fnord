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
 *	mem.h : contains memory alignment constraint information
 *		for various architectures
 *
 **********************************************************************/

#ifndef _MEM_H_
#define _MEM_H_

/* $Id: mem.h,v 1.10 1992/06/22 17:08:28 mas Exp $ */
#include "port.h"

/* alignment constraints for various types.  May be more restrictive
 *  than actually necessary */
#if defined(sparc) || defined(mips)
#define MEM_LG_ALIGN_CHAR    0
#define MEM_LG_ALIGN_INT     2
#define MEM_LG_ALIGN_SHORT   1
#define MEM_LG_ALIGN_LONG    2
#define MEM_LG_ALIGN_FLOAT   2
#define MEM_LG_ALIGN_DOUBLE  2
#define MEM_LG_ALIGN_GENERIC 2

#else
#if defined(mc68020)
#define MEM_LG_ALIGN_CHAR    0
#define MEM_LG_ALIGN_INT     1
#define MEM_LG_ALIGN_SHORT   1
#define MEM_LG_ALIGN_LONG    1
#define MEM_LG_ALIGN_FLOAT   2
#define MEM_LG_ALIGN_DOUBLE  2
#define MEM_LG_ALIGN_GENERIC 2

#else
#if defined(stellar)
#define MEM_LG_ALIGN_CHAR    0
#define MEM_LG_ALIGN_INT     2
#define MEM_LG_ALIGN_SHORT   1
#define MEM_LG_ALIGN_LONG    2
#define MEM_LG_ALIGN_FLOAT   2
#define MEM_LG_ALIGN_DOUBLE  2
#define MEM_LG_ALIGN_GENERIC 2

#else
#if defined(hpux)
#define MEM_LG_ALIGN_CHAR    0
#define MEM_LG_ALIGN_INT     2
#define MEM_LG_ALIGN_SHORT   1
#define MEM_LG_ALIGN_LONG    2
#define MEM_LG_ALIGN_FLOAT   2
#define MEM_LG_ALIGN_DOUBLE  3
#define MEM_LG_ALIGN_GENERIC 3

#else
#if defined(IBM6000)
#define MEM_LG_ALIGN_CHAR    0
#define MEM_LG_ALIGN_INT     1
#define MEM_LG_ALIGN_SHORT   1
#define MEM_LG_ALIGN_LONG    1
#define MEM_LG_ALIGN_FLOAT   1
#define MEM_LG_ALIGN_DOUBLE  1
#define MEM_LG_ALIGN_GENERIC 1

#else
#if defined(vax) || defined(rt)
#define MEM_LG_ALIGN_CHAR    0
#define MEM_LG_ALIGN_INT     0
#define MEM_LG_ALIGN_SHORT   0
#define MEM_LG_ALIGN_LONG    0
#define MEM_LG_ALIGN_FLOAT   0
#define MEM_LG_ALIGN_DOUBLE  0
#define MEM_LG_ALIGN_GENERIC 0

#else
#define MEM_ALIGN_GUESSING	1
unsupported processor architecture
#endif /* undefined(vax) */
#endif /* undefined(IBM6000) */
#endif /* undefined(hpux) */
#endif /* undefined(stellar) */
#endif /* undefined(mc68020) */
#endif /* undefined(sparc) */


#define MEM_ALIGN_UP_LG(ptr,bits)	(	\
   ((ptr) & ((1 << (bits)) - 1)) ?		\
   ((ptr) & ~((1 << (bits)) -1)) +(1 << (bits)) :	\
   (ptr))

#define MEM_ALIGN_UP_VAL(ptr,align)	(	\
   ((ptr) % (align)) ?				\
   (((ptr) / (align) + 1) * (align)) :		\
   (ptr))

#ifndef MEM_ALIGN_GUESSING

#define MEM_ALIGN_CHAR    (1 << MEM_LG_ALIGN_CHAR)
#define MEM_ALIGN_INT     (1 << MEM_LG_ALIGN_INT)
#define MEM_ALIGN_SHORT   (1 << MEM_LG_ALIGN_SHORT)
#define MEM_ALIGN_LONG    (1 << MEM_LG_ALIGN_LONG)
#define MEM_ALIGN_FLOAT   (1 << MEM_LG_ALIGN_FLOAT)
#define MEM_ALIGN_DOUBLE  (1 << MEM_LG_ALIGN_DOUBLE)
#define MEM_ALIGN_GENERIC (1 << MEM_LG_ALIGN_GENERIC)

#define MEM_ALIGN_UP_CHAR(type, ptr) (type)	\
   MEM_ALIGN_UP_LG((long) ptr, MEM_LG_ALIGN_CHAR)
#define MEM_ALIGN_UP_INT(type, ptr) (type)	\
   MEM_ALIGN_UP_LG((long) ptr, MEM_LG_ALIGN_INT)
#define MEM_ALIGN_UP_SHORT(type, ptr) (type)	\
   MEM_ALIGN_UP_LG((long) ptr, MEM_LG_ALIGN_SHORT)
#define MEM_ALIGN_UP_LONG(type, ptr) (type)	\
   MEM_ALIGN_UP_LG((long) ptr, MEM_LG_ALIGN_LONG)
#define MEM_ALIGN_UP_FLOAT(type, ptr) (type)	\
   MEM_ALIGN_UP_LG((long) ptr, MEM_LG_ALIGN_FLOAT)
#define MEM_ALIGN_UP_DOUBLE(type, ptr) (type)	\
   MEM_ALIGN_UP_LG((long) ptr, MEM_LG_ALIGN_DOUBLE)
#define MEM_ALIGN_UP_GENERIC(type, ptr) (type)	\
   MEM_ALIGN_UP_LG((long) ptr, MEM_LG_ALIGN_GENERIC)


#if 0
/* use token pasting if ANSI C, null comment otherwise */
#ifdef _STDC_
#define MEM_ALIGN_UP(type, ptr, align)	(type)	\
   MEM_ALIGN_UP_LG((long) ptr, MEM_LG_ALIGN_##align)
#else /* undef(_STDC_) */
#define MEM_ALIGN_UP(type, ptr, align)	(type)	\
   MEM_ALIGN_UP_LG((long) ptr, MEM_LG_ALIGN_/* */align)
#endif /* undef(_STDC_) */
#endif /* 0 */

#else /* defined(MEM_ALIGN_GUESSING) */

#define MEM_ALIGN_UP_CHAR(type, ptr) (type)	\
   MEM_ALIGN_UP_VAL((long) ptr, MEM_ALIGN_CHAR)
#define MEM_ALIGN_UP_INT(type, ptr) (type)	\
   MEM_ALIGN_UP_VAL((long) ptr, MEM_ALIGN_INT)
#define MEM_ALIGN_UP_SHORT(type, ptr) (type)	\
   MEM_ALIGN_UP_VAL((long) ptr, MEM_ALIGN_SHORT)
#define MEM_ALIGN_UP_LONG(type, ptr) (type)	\
   MEM_ALIGN_UP_VAL((long) ptr, MEM_ALIGN_LONG)
#define MEM_ALIGN_UP_FLOAT(type, ptr) (type)	\
   MEM_ALIGN_UP_VAL((long) ptr, MEM_ALIGN_FLOAT)
#define MEM_ALIGN_UP_DOUBLE(type, ptr) (type)	\
   MEM_ALIGN_UP_VAL((long) ptr, MEM_ALIGN_DOUBLE)
#define MEM_ALIGN_UP_GENERIC(type, ptr) (type)	\
   MEM_ALIGN_UP_VAL((long) ptr, MEM_ALIGN_GENERIC)

#if 0

/* use token pasting if ANSI C, null comment otherwise */
#ifdef _STDC_
#define MEM_ALIGN_UP(type, ptr, align)	(type)	\
   MEM_ALIGN_UP_VAL((long) ptr, MEM_ALIGN_##align)
#else /* undef(_STDC_) */
#define MEM_ALIGN_UP(type, ptr, align)	(type)	\
   MEM_ALIGN_UP_VAL((long) ptr, MEM_ALIGN_/* */align)
#endif /* undef(_STDC_) */
#endif 0

#endif /* defined(MEM_ALIGN_GUESSING) */

typedef union {
   char c;
   short s;
   int i;
   long	l;
   float f;
   double d;
   char *p;
} MEM_ALIGN_TYPE;

/* for any ptr returned by malloc, ptr & MEM_MALLOC_MASK is guaranteed
 *  to be 0 */
/* MEM_MALLOC_HEADER is the number of bytes stolen from a malloc chunk */
#if defined (sun4)
#define MEM_MALLOC_MASK 	0x7
#define MEM_MALLOC_HEADER	12
#else
#if defined (sun3)
#define MEM_MALLOC_MASK 	0x3
#define MEM_MALLOC_HEADER	12
#else
#if defined (stellar)
#define MEM_MALLOC_MASK 	0x3
#define MEM_MALLOC_HEADER	12
#else
#if defined (hpux)
#define MEM_MALLOC_MASK 	0x3
#define MEM_MALLOC_HEADER	12
#else
#if defined (IBM6000)
#define MEM_MALLOC_MASK 	0x3
#define MEM_MALLOC_HEADER	12
#else
#if defined (mips)
#define MEM_MALLOC_MASK 	0x3
#define MEM_MALLOC_HEADER	12
#else
#if defined (vax)
#define MEM_MALLOC_MASK 	0x7
#define MEM_MALLOC_HEADER	12
#else
#if defined (rt)
#define MEM_MALLOC_MASK 	0x7
#define MEM_MALLOC_HEADER	12
#else
unsupported operating system
#endif /* undefined(rt) */
#endif /* undefined(vax) */
#endif /* undefined(mips) */
#endif /* undefined(IBM6000) */
#endif /* undefined(hpux) */
#endif /* undefined(stellar) */
#endif /* undefined(sun3) */
#endif /* undefined(sun4) */

#endif /* undef(_MEM_H_) */
