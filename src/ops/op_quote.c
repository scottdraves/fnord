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
static char Version[] =
   "$Id: op_quote.c,v 1.8 1992/06/22 17:06:59 mas Exp $";
#endif

#define QUOTE_ARG	0
#define QUOTE_NUM_KIDS	1

#include "defs.h"


METset *
MET_op_quote(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METsym	*kid;
   METset	*result = MET_NULL_SET;
   MME(MET_op_quote);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, QUOTE_NUM_KIDS);
#endif

   kid = sym->kids[QUOTE_ARG];

   if (MET_SYM_TYPE_SET_REF == kid->type &&
       MET_SET_TYPE_SYMBOL == kid->info.set_ref.set->type) {
      result =
	 METsym_new_sym_const_set(kid->info.set_ref.set->info.symbol.sym);
   } else {
      ERROR(sym->origin, "quote only takes symbols", SEV_FATAL);
   }

   MET_set_get_type(result, result_type, set_type);
   return result;
}

#define ASSOC_SYMBOL	0
#define ASSOC_VALUE	1
#define ASSOC_NUM_KIDS	2

METset *
MET_op_assoc(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[ASSOC_NUM_KIDS], *result_set = MET_NULL_SET;
   METsym	*quote_kid;
   Sym		symbol;
   char		buft[80];
   char		buf[80];
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, ASSOC_NUM_KIDS);
#endif

   quote_kid = sym->kids[ASSOC_SYMBOL];

   if (MET_SYM_TYPE_SET_REF == quote_kid->type &&
       MET_SET_TYPE_SYMBOL == quote_kid->info.set_ref.set->type) {
      symbol = quote_kid->info.set_ref.set->info.symbol.sym;
      arg_set[ASSOC_VALUE] =
	 MET_sym_be_set_ref(sym->kids[ASSOC_VALUE],
			    MET_NULL_TYPE,
			    (METtype **) NULL, INULL);
      if (MET_NULL_SET != arg_set[ASSOC_VALUE]) {
	 result_set = METset_assoc(arg_set[ASSOC_VALUE], symbol);
	 if (MET_NULL_SET == result_set) {
	    strncpy(buft, SYM_STRING(symbol), 50);
	    sprintf(buf, "association for \"%s\" not found", buft);
	    ERROR(sym->origin, buf, SEV_FATAL);
	 } else
	    MET_set_get_type(result_set, result_type, set_type);
	 MET_SET_FREE(arg_set[ASSOC_VALUE]);
      }
   } else {
      ERROR(sym->origin, "assoc first arg must be symbol", SEV_FATAL);
   }
   return result_set;
}
