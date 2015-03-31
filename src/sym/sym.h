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


/* $Id: sym.h,v 1.7 1992/06/22 17:09:08 mas Exp $ */

#ifndef _SYM_H_
#define _SYM_H_

#include "port/mem.h"

typedef struct Sym_struct {
   int hash;			/* hash value for this string */
   struct Sym_struct *next;	/* next Sym in this hash bucket */
   char s[1];			/* variable-length string */
} *Sym;

#define SYM_STRING(sym)		((sym)->s)
#define SYM_KEY(sym)    	((sym)->hash)
#define SYM_EQ(sym1,sym2)	((sym1)==(sym2))

/* initialize Sym package.  Hashsize should be a prime. */
extern void sym_init(/* int hashsize */);

/* Turn a string into a Sym.  The string passed in is copied, so it
 *  should be freed by the user and SYM_STRING(sym) used instead */
extern Sym sym_from_string(/* char *s */);

/* Use this instead of strdup if you don't want to be bothered with
 *  freeing the storage afterwards */
#define SYM_STRDUP(s)	SYM_STRING(sym_from_string(s))


/* default size of a sym area */
#define SYM_AREA_SIZE_DEFAULT	4096

#endif /* _SYM_H_ */
