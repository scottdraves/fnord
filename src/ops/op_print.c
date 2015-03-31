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
   "$Id: op_print.c,v 1.9 1992/06/22 17:06:57 mas Exp $";
#endif

#include "defs.h"

#define PRINT_NUM_KIDS		1
#define PRINT_ARG		0

#define TRACE_NUM_KIDS		1
#define TRACE_ARG		0

METset *
MET_op_print(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[PRINT_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[PRINT_NUM_KIDS];
   METtype	*arg_type[PRINT_NUM_KIDS];
   MME(MET_op_print);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, PRINT_NUM_KIDS);
#endif
   arg_set[PRINT_ARG] =
      MET_sym_be_set_ref(sym->kids[PRINT_ARG], type,
			 &arg_type[PRINT_ARG],
			 &arg_set_type[PRINT_ARG]);

   if (MET_NULL_SET == arg_set[PRINT_ARG])
      goto bail1;

   MET_op_reconcile_sets(arg_set, PRINT_NUM_KIDS);

   MET_SET_COPY(result_set, arg_set[PRINT_ARG]);

   if (MET_SYM_OP_PRINT == sym->info.op.code) {
      METset_print_pretty(stdout, result_set, CNULL, 0);
      (void) printf("\n");
   }

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

   COND_FREE(PRINT_ARG);

   return result_set;
}

METset *
MET_op_trace(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[TRACE_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[TRACE_NUM_KIDS];
   METtype	*arg_type[TRACE_NUM_KIDS];
   Boolean      old_trace;
   MME(MET_op_trace);

#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, TRACE_NUM_KIDS);
#endif

   old_trace = MET_defaults->trace.on;
   MET_defaults->trace.on = (sym->info.op.code == MET_SYM_OP_TRACE_ON);

   arg_set[TRACE_ARG] =
      MET_sym_be_set_ref(sym->kids[TRACE_ARG], type,
			 &arg_type[TRACE_ARG],
			 &arg_set_type[TRACE_ARG]);

   MET_defaults->trace.on = old_trace;

   if (MET_NULL_SET == arg_set[PRINT_ARG])
      goto bail1;

   MET_op_reconcile_sets(arg_set, TRACE_NUM_KIDS);

   MET_SET_COPY(result_set, arg_set[TRACE_ARG]);

 bail1:
   MET_set_get_type(result_set, result_type, set_type);

   COND_FREE(TRACE_ARG);

   return result_set;
}
