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

#include "td/defs.h"
#include "ops/ops.h"

#ifndef lint
static char Version[] =
   "$Id: td_shared.c,v 1.3 1993/06/04 13:14:23 rch Exp $";
#endif

	/********************************************************/
	/*                					*/
	/* Functions for accessing components of METsets 	*/
	/*                					*/
	/********************************************************/


/*
 *	Convenience function to return the symbol locked
 *	away inside the passed set.  Looks first for
 *	a symbol set, and then for the main symbol of
 *	a set-of-symbols set.  (Symbols tend to be hidden
 *	away in weird spots.)
 */

FOboolean td_find_symbol(set, symbol)
   METset *set;
   Sym *symbol;
{
   METset *next;

   if (MET_NULL_SET == set)
      return FAILURE;

   if (METset_get_symbol(set, symbol))
      return SUCCESS;

   else if (set->type != MET_SET_TYPE_BARE)
      return FAILURE;

   else if (set->info.bare.type->type != MET_TYPE_TYPE_SET)
      return FAILURE;
   
   else if (set->info.bare.data->verts->type != MET_FIELD_SET)
      return FAILURE;

   next = set->info.bare.data->verts->vertices->set;

   return(METset_get_symbol(next, symbol));
}
   
/*
 *	Convenience function to return the integer value 
 *	associated with the passed symbol in the passed
 *	alist.  The value passed in as default is returned
 *	as val if there is no association of the symbol in the
 *	alist, and if the association of the symbol in
 *	the alist is the special symbol special, then
 *	svalue is returned as val.  The function returns
 *	SUCCESS if there was an association in the alist.
 */

FOboolean td_lookup_int(alist, sym, val, def, special, svalue)
   METset *alist;
   Sym sym;
   int *val;
   int def;
   Sym special;
   int svalue;
{
   METset *assoc;
   int fld = MET_FIELD_INTEGER;
   Sym setsym;
   METnumber value;

   ME(td_lookup_int);

   if (MET_NULL_SET == alist)
   {
      *val = def;
      return(FAILURE);
   }

   assoc = METset_assoc(alist, sym);
   if (MET_NULL_SET == assoc)
   {
      *val = def;
      return(FAILURE);
   }

   if (FAILURE == METset_get_number(assoc, &fld, &value, YES))
   {
      if ((! td_find_symbol(assoc, &setsym)) ||
	  (! SYM_EQ(setsym, special)))
      {
         if( assoc->definition ) {
            FILOCloc_print(assoc->definition->origin, stderr, CNULL, 0); 
         }
	 TD_MSG(stderr, ": Bad argument in alist");
         TD_MSG(stderr, " (wanted integer for attr \"%s\")\n", SYM_STRING(sym) );
	 MET_SET_FREE(assoc);
	 *val = def;
	 return(SUCCESS);
      }
      else 
	 *val = svalue;
   }
   else
      *val = (int) value.integer;
   
   MET_SET_FREE(assoc);
   return(SUCCESS);
}


FOboolean td_lookup_boolean(alist, sym, val, def, special, svalue)
   METset *alist;
   Sym sym;
   FOboolean *val;
   FOboolean def;
   Sym special;
   FOboolean svalue;
{
   METset *assoc;
   int fld = MET_FIELD_ZTWO;
   Sym setsym;
   METnumber value;

   ME(td_lookup_boolean);

   if (MET_NULL_SET == alist)
   {
      *val = def;
      return(FAILURE);
   }

   assoc = METset_assoc(alist, sym);
   if (MET_NULL_SET == assoc)
   {
      *val = def;
      return(FAILURE);
   }

   if (FAILURE == METset_get_number(assoc, &fld, &value, YES))
   {
      if ((! td_find_symbol(assoc, &setsym)) ||
	  (! SYM_EQ(setsym, special)))
      {
         if( assoc->definition ) {
            FILOCloc_print(assoc->definition->origin, stderr, CNULL, 0); 
         }
	 TD_MSG(stderr, ": Bad argument in alist");
         TD_MSG(stderr, " (wanted boolean for attr \"%s\")\n", SYM_STRING(sym) );
	 MET_SET_FREE(assoc);
	 *val = def;
	 return(SUCCESS);
      }
      else 
	 *val = svalue;
   }
   else
      *val = (FOboolean) value.ztwo;
   
   MET_SET_FREE(assoc);
   return(SUCCESS);
}


/*
 *	Convenience function to return the double value 
 *	associated with the passed symbol in the passed
 *	alist.  The value passed in as default is returned
 *	as val if there is no association of the symbol in the
 *	alist, and if the association of the symbol in
 *	the alist is the special symbol special, then
 *	svalue is returned as val.  The function returns
 *	SUCCESS if there was an association in the alist.
 */

FOboolean td_lookup_real(alist, sym, val, def, special, svalue)
   METset *alist;
   Sym sym;
   Real *val;
   Real def;
   Sym special;
   Real svalue;
{
   METset *assoc;
   int fld = MET_FIELD_REAL;
   METnumber met_val;
   Sym setsym;

   if (MET_NULL_SET == alist)
   {
      *val = def;
      return(FAILURE);
   }

   assoc = METset_assoc(alist, sym);
   if (MET_NULL_SET == assoc)
   {
      *val = def;
      return(FAILURE);
   }

   if (!METset_get_number(assoc, &fld, &met_val, YES))
   {
      if (( ! td_find_symbol(assoc, &setsym)) ||
	  (! SYM_EQ(setsym, special)))
      {
         if( assoc->definition ) {
            FILOCloc_print(assoc->definition->origin, stderr, CNULL, 0); 
         }
	 TD_MSG(stderr, ": Bad argument in alist");
         TD_MSG(stderr, " (wanted real for attr \"%s\")\n", SYM_STRING(sym) );
	 MET_SET_FREE(assoc);
	 *val = def;
	 return(SUCCESS);
      }
      else
	 *val = svalue;
   }
   else
      *val = (Real) met_val.real;

   MET_SET_FREE(assoc);
   return(SUCCESS);
}

FOboolean td_lookup_field(alist, sym, val, def, special, svalue)
   METset *alist;
   Sym sym;
   int *val;
   int def;
   Sym special;
   int svalue;
{
   METtype *type;
   METset *assoc;
   Sym setsym;

   if (MET_NULL_SET == alist)
   {
      *val = def;
      return FAILURE;
   }

   assoc = METset_assoc(alist, sym);
   if (MET_NULL_SET == assoc)
   {
      *val = def;
      return(FAILURE);
   }

   type = METset_get_type(assoc);
   if (MET_NULL_TYPE == type)
   {
      if (( ! td_find_symbol(assoc, &setsym)) ||
	  (! SYM_EQ(setsym, special)))
      {
         if( assoc->definition ) {
            FILOCloc_print(assoc->definition->origin, stderr, CNULL, 0); 
         }
	 TD_MSG(stderr, ": Bad argument in alist");
         TD_MSG(stderr, " (wanted field for attr \"%s\")\n", SYM_STRING(sym) );
	 MET_SET_FREE(assoc);
	 *val = def;
	 return(SUCCESS);
      }
      else
	 *val = svalue;
   }
   else {
      if (MET_TYPE_TYPE_FIELD == type->type)
	 *val = type->info.field.type;
      else {
	 fprintf(stderr, "type must be a field");
	 *val = def;
      }
      MET_TYPE_FREE(type);
   }

   MET_SET_FREE(assoc);
   return(SUCCESS);
}

FOboolean td_lookup_type(alist, sym, val, def, special, svalue)
   METset *alist;
   Sym sym;
   METtype **val;
   METtype *def;
   Sym special;
   METtype *svalue;
{
   METtype *type;
   METset *assoc;
   Sym setsym;

   if (MET_NULL_SET == alist)
   {
      MET_TYPE_COPY (*val, def);
      return FAILURE;
   }

   assoc = METset_assoc(alist, sym);
   if (MET_NULL_SET == assoc)
   {
      MET_TYPE_COPY (*val, def);
      return(FAILURE);
   }

   type = METset_get_type(assoc);
   if (MET_NULL_TYPE == type)
   {
      if (( ! td_find_symbol(assoc, &setsym)) ||
	  (! SYM_EQ(setsym, special)))
      {
         if( assoc->definition ) {
            FILOCloc_print(assoc->definition->origin, stderr, CNULL, 0); 
         }
	 TD_MSG(stderr, ": Bad argument in alist");
         TD_MSG(stderr, " (wanted field for type \"%s\")\n", SYM_STRING(sym) );
	 MET_SET_FREE(assoc);
	 MET_TYPE_COPY (*val, def);
	 return(SUCCESS);
      }
      else
	 MET_TYPE_COPY (*val, svalue);
   }
   else
      *val = type;

   MET_SET_FREE(assoc);
   return(SUCCESS);
}

	/********************************************************/
	/*                					*/
	/* Functions for building recalculable METsets from 	*/
	/* alist descriptions.                                  */
	/*                					*/
	/********************************************************/


METset *
td_make_set_recalc(alist, std_arg, loc)
   METset	*alist;
   int		std_arg;
   FILOCloc	*loc;
{
   METset	*good;
   METsym	*set_def, *set_sym, *pair_op, *pair;
   ME(td_make_set_recalc);
   
   set_def = METsym_new_op(MET_SYM_OP_APPLY, loc);
   set_sym = METsym_new_op(MET_SYM_OP_ASSOC, loc);
   METsym_add_kid(set_def, set_sym);
   MET_SYM_FREE(set_sym);
   set_sym = METsym_new_sym(std_args[std_arg], loc);
   pair = METsym_new_op(MET_SYM_OP_APPLY, loc);
   pair_op = METsym_new_op(MET_SYM_OP_PAIR_BUILD, loc);
   METsym_add_kid(pair, pair_op);
   MET_SYM_FREE(pair_op);
   METsym_add_kid(pair, set_sym);
   MET_SYM_FREE(set_sym);
   set_sym = METsym_new_set_ref(alist, loc);
   METsym_add_kid(pair, set_sym);
   MET_SYM_FREE(set_sym);
   METsym_add_kid(set_def, pair);
   MET_SYM_FREE(pair);
   good = METset_new_with_definition( set_def, MET_NULL_ENV );
   MET_SYM_FREE(set_def);
   METset_have_rep(good, (FOboolean *) NULL);
   
   return good;
}   

METset *
td_make_type_recalc(alist, type_arg, loc)
   METset 	*alist;
   int 		type_arg;
   FILOCloc	*loc;
{
   METset *temp, *type;
   METsym *type_def;
   ME(td_make_type_recalc);

   temp = METset_assoc(alist, std_args[type_arg]);
   if (MET_NULL_SET == temp)
   {
      type_def = METsym_new_op (MET_SYM_OP_TYPE_UNKNOWN, 
				loc);
      type = METset_new_with_definition(type_def, MET_NULL_ENV);
      MET_SYM_FREE(type_def);
   }
   else
   {
      type = td_make_set_recalc(alist, type_arg, loc);
   }
   MET_SET_FREE_NULL(temp);

   return type;
}

METset *
td_make_set_cast(set, type, success)
   METset *set;
   METset *type;
   FOboolean *success;
{
   METset *value;
   METsym *type_def, *set_def, *result_def;
   ME(td_make_set_cast);

   type_def = METsym_new_set_ref(type, (FILOCloc *) NULL);
   set_def = METsym_new_set_ref(set, (FILOCloc *) NULL);
   result_def = METsym_new_op(MET_SYM_OP_TYPE_AND_CAST, (FILOCloc *) NULL);
   METsym_add_kid(result_def, type_def);
   MET_SYM_FREE(type_def);
   METsym_add_kid(result_def, set_def);
   MET_SYM_FREE(set_def);
   value = METset_new_with_definition(result_def, MET_NULL_ENV);
   MET_SYM_FREE(result_def);
   *success = METset_have_rep(value, (FOboolean *) NULL);

   return value;
}

/************************************************************************/
/*									*/
/*		GOOD DEFAULTS FOR UNSUPPORTED OBJECTS			*/
/*									*/
/************************************************************************/

METsym *td_cant_create(parent, name, alist, sym)
   fnord_widget		*parent;
   char			*name;
   METset		*alist;
   METsym		*sym;
{
   ME(td_cant_create);

   FILOCloc_print(sym->origin, stdout, CNULL, 0);
   TD_MSG(stdout, ": type of widget unsupported.\n");
   return MET_NULL_SYM;
}

FOboolean td_cant_request(widget, request)
   fnord_widget		*widget;
   METcontrol_request	*request;
{
   ME(td_cant_request);

   return FAILURE;
}
