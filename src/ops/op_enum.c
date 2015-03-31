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
   "$Id: op_enum.c,v 1.10 1992/06/22 17:06:19 mas Exp $";
#endif

#include "defs.h"

#define ENUM_NUM_KIDS		2
#define ENUM_LEFT		0
#define ENUM_RIGHT		1

/*
 * create a type that is (int, int, int, ... int)
 */
static METtype *
MET__op_enum_result_type(size)
   int		size;
{
   METtype	*result, *t, *s;
   MME(MET__op_enum_result_type);

   MET_TYPE_NEW_FIELD(t, MET_FIELD_INTEGER);
   if (1 == size)
      return t;

   s = MET__op_enum_result_type(size - 1);
   MET_TYPE_NEW_PAIR(result, s, t);
   MET_TYPE_FREE(s);
   MET_TYPE_FREE(t);
   
   return result;
}
      
/*
 * deals with choose, modulo, bitand, bitor, and bitxor
 */
METset *
MET_op_enum(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[ENUM_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[ENUM_NUM_KIDS];
   METtype	*arg_type[ENUM_NUM_KIDS], *guess;
   METtype	*my_result_type;
   METdata	*tdata;
   METverts	*verts;
   int		how_many, lower_bound, upper_bound, width, i;
   MME(MET_op_enum);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, ENUM_NUM_KIDS);
#endif

   MET_TYPE_NEW_FIELD(guess, MET_FIELD_INTEGER);
   arg_set[ENUM_LEFT] =
      MET_sym_be_set_ref(sym->kids[ENUM_LEFT], guess,
			 &arg_type[ENUM_LEFT],
			 &arg_set_type[ENUM_LEFT]);
   
   arg_set[ENUM_RIGHT] =
      MET_sym_be_set_ref(sym->kids[ENUM_RIGHT], guess,
			 &arg_type[ENUM_RIGHT],
			 &arg_set_type[ENUM_RIGHT]);
   MET_TYPE_FREE(guess);

   if (MET_NULL_TYPE != arg_type[ENUM_LEFT] &&
       MET_NULL_TYPE != arg_type[ENUM_RIGHT]) {
      if (! (MET_TYPE_TYPE_FIELD == arg_type[ENUM_RIGHT]->type &&
	     MET_FIELD_INTEGER == arg_type[ENUM_RIGHT]->info.field.type &&
	     MET_TYPE_TYPE_FIELD == arg_type[ENUM_LEFT]->type &&
	     MET_FIELD_INTEGER == arg_type[ENUM_LEFT]->info.field.type)) {
	 ERROR(sym->origin, "enum, bad argument types", SEV_FATAL);
      } else {
	 MET_op_reconcile_sets(arg_set, ENUM_NUM_KIDS);
	 
	 MAX_VERTS(how_many, arg_set[ENUM_LEFT]->info.bare.data,
		   arg_set[ENUM_RIGHT]->info.bare.data);
	 /*
	  * this doesn't work on polybare objects because it would yield a
	  * different type for each one
	  */
	 if (1 == how_many) {
	    
	    lower_bound = (int) *
	       DATA_FIRST_VERTEX(arg_set[ENUM_LEFT]->info.bare.data,
				 METinteger);
	    upper_bound = (int) *
	       DATA_FIRST_VERTEX(arg_set[ENUM_RIGHT]->info.bare.data,
				 METinteger);
	    
	    if (upper_bound >= lower_bound) {
	       width = upper_bound - lower_bound + 1;
	       my_result_type = MET__op_enum_result_type(width);
	       result_set = MET_set_new(MET_SET_TYPE_BARE);
	       result_set->info.bare.how_many = 1;
	       MET_tags_initialize(&result_set->info.bare.tags_bare);
	       MET_tags_initialize(&result_set->info.bare.tags_in);
	       MET_TYPE_COPY(result_set->info.bare.type, my_result_type);
	       MET_TYPE_FREE(my_result_type);
	       verts = MET_verts_new(MET_FIELD_INTEGER, 1, width);
	       result_set->info.bare.data = MET_NULL_DATA;
	       for (i = width - 1; i >= 0; i--) {
		  tdata = MET_data_new(verts, 1, i);
		  *DATA_FIRST_VERTEX(tdata, METinteger) =
		     (METinteger)(i + lower_bound);
		  
		  if (MET_NULL_DATA != result_set->info.bare.data) {
		     MET_DATA_COPY(tdata->next, result_set->info.bare.data);
		     MET_DATA_FREE(result_set->info.bare.data);
		  }
		  MET_DATA_COPY(result_set->info.bare.data, tdata);
		  MET_DATA_FREE(tdata);
	       }
	       MET_VERTS_FREE(verts);
	    } else {
	       char buf[100];
	       sprintf(buf, "enumerate: bad bounds: %d..%d",
		       lower_bound, upper_bound);
	       ERROR(sym->origin, buf, SEV_FATAL);
	    }
	 } else {
	    ERROR(sym->origin, "enumerate: cannot handle poly arguments",
		  SEV_FATAL);
	 }
      }
   }

   MET_set_get_type(result_set, result_type, set_type);

   COND_FREE(ENUM_LEFT);
   COND_FREE(ENUM_RIGHT);

   return result_set;
}
