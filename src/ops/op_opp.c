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
   "$Id: op_opp.c,v 1.9 1992/06/22 17:06:42 mas Exp $";
#endif

#include "defs.h"

#define OPP_NUM_KIDS	1
#define OPP_ARG		0


#define TEMPLATE(NAME, TYPE1, DST_TYPE) \
static Boolean \
NAME(arg1, result) \
   METdata	*arg1, *result; \
{ \
   int		i, j, length; \
   TYPE1	*src1; \
   DST_TYPE	*dst; \
   int		src1_width, dst_width; \
   MME(NAME); \
 \
   length = result->num_cols; \
   src1_width = (1 == DATA_NUM_VERTS(arg1)) ? 0 : arg1->verts->total_cols; \
   dst_width =  result->verts->total_cols; \
 \
   for (j = 0; j < length; j++) { \
      src1 = DATA_FIRST_VERTEX(arg1, TYPE1) + j; \
      dst  = DATA_FIRST_VERTEX(result,  DST_TYPE) + j; \
      i = DATA_NUM_VERTS(arg1); \
       \
      while(i--) { \
	 BODY; \
	 dst += dst_width; \
	 src1 += src1_width; \
      } \
   } \
   return SUCCESS; \
}

#define BODY *dst = - *src1;
TEMPLATE(MET__op_opp_guts_i, METinteger, METinteger)
TEMPLATE(MET__op_opp_guts_r, METreal,    METreal)
#undef BODY

#define BODY dst->real = - src1->real; \
             dst->imaginary = - src1->imaginary;
TEMPLATE(MET__op_opp_guts_c, METcomplex, METcomplex)
#undef BODY

#define foobar MET_SYM_OP_OPPOSITE
#define BODY MET_TYPE_COPY(dst->loose_from, src1->loose_from);      \
   dst->env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV); 	     \
   dst->sym = METsym_new_op(foobar, src1->sym->origin);  \
   METsym_add_kid(dst->sym, src1->sym);				       
TEMPLATE(MET__op_opp_guts_m, METmap, METmap)
#undef BODY

#define BODY *dst = *src1 * *src1;
TEMPLATE(MET__op_opp_guts_square_i, METinteger, METinteger)
TEMPLATE(MET__op_opp_guts_square_r, METreal,    METreal)
#undef BODY

#define BODY dst->real = (src1->real * src1->real) - \
                         (src1->imaginary * src1->imaginary); \
             dst->imaginary = 2 * src1->imaginary * src1->real;
TEMPLATE(MET__op_opp_guts_square_c, METcomplex, METcomplex)
#undef BODY

#define BODY MET_TYPE_COPY(dst->loose_from, src1->loose_from);      \
   dst->env = METenv_new(MET_ENV_TYPE_LITTLE, MET_NULL_ENV); 	     \
   dst->sym = METsym_new_op(MET_SYM_OP_SQUARE, src1->sym->origin);    \
   METsym_add_kid(dst->sym, src1->sym);				      
TEMPLATE(MET__op_opp_guts_square_m, METmap, METmap)
#undef BODY

static Boolean
MET__op_opp_guts(arg1, result)
   METdata	*arg1, *result;
{
   Boolean	(*f)() = MET_op_guts_failure;
   MME(MET__op_opp_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:
      break;
    case MET_FIELD_INTEGER:
      f = MET__op_opp_guts_i;
      break;
    case MET_FIELD_ANGLE:
      break;
    case MET_FIELD_REAL:
      f = MET__op_opp_guts_r;
      break;
    case MET_FIELD_COMPLEX:
      f = MET__op_opp_guts_c;
      break;
    case MET_FIELD_MAP:
      f = MET__op_opp_guts_m;
      break;
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:
      break;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   if (SUCCESS == (f)(arg1, result)) {
      if (MET_NULL_DATA != result->next)
	 return MET__op_opp_guts(arg1->next, result->next);
      else
	 return SUCCESS;
   } else
      return FAILURE;
}

static Boolean
MET__op_opp_guts_square(arg1, result)
   METdata	*arg1, *result;
{
   Boolean	(*f)() = MET_op_guts_failure;
   MME(MET__op_opp_guts);

   switch (arg1->verts->type) {
    case MET_FIELD_ZTWO:
      break;
    case MET_FIELD_INTEGER:
      f = MET__op_opp_guts_square_i;
      break;
    case MET_FIELD_ANGLE:
      break;
    case MET_FIELD_REAL:
      f = MET__op_opp_guts_square_r;
      break;
    case MET_FIELD_COMPLEX:
      f = MET__op_opp_guts_square_c;
      break;
    case MET_FIELD_MAP:
      f = MET__op_opp_guts_square_m;
      break;
    case MET_FIELD_QUATERNION:
    case MET_FIELD_SET:
      break;
    default:
      DEFAULT_ERR(arg1->verts->type);
   }

   return (f)(arg1, result);
}

/*
 * opp is opposite, "-",  unary minus.  Also deals with "square"
 */
METset *
MET_op_opp(sym, type, result_type, set_type)
   METsym	*sym;
   METtype	*type, **result_type;
   int		*set_type;
{
   METset	*arg_set[OPP_NUM_KIDS], *result_set = MET_NULL_SET;
   int		arg_set_type[OPP_NUM_KIDS];
   METtype	*arg_type[OPP_NUM_KIDS];
   METtype	*my_result_type = MET_NULL_TYPE, *temp;
   int		how_many;
   Boolean	result, is_square;
   MME(MET_op_opp);
#ifdef OPS__CHECK_PARMS
   MET_op_check_parms(sym, type, result_type, OPP_NUM_KIDS);
#endif
   is_square = (sym->info.op.code == MET_SYM_OP_SQUARE);

   arg_set[OPP_ARG] =
      MET_sym_be_set_ref(sym->kids[OPP_ARG], type,
			 &arg_type[OPP_ARG],
			 &arg_set_type[OPP_ARG]);


   if (MET_NULL_TYPE == arg_type[OPP_ARG])
      goto bail1;

   if (MET_TYPE_TYPE_MAP == arg_type[OPP_ARG]->type) {
      temp = arg_type[OPP_ARG];
      do {
	 temp = temp->info.map.to;
      } while (MET_TYPE_TYPE_MAP == temp->type);
   } else
      temp = arg_type[OPP_ARG];

   if ((YES == is_square &&
	MET_TYPE_TYPE_FIELD == temp->type) ||
       !(YES == is_square))
      MET_TYPE_COPY(my_result_type, arg_type[OPP_ARG]);
   
   if (MET_NULL_TYPE == my_result_type) {
      ERROR(sym->origin, "opp or square argument of wrong type",
	    SEV_FATAL);
      goto bail1;
   }
   
   if (MET_op_type_only(arg_set, OPP_NUM_KIDS,
			result_type, my_result_type,
			set_type, MET_SET_TYPE_BARE)) {
      goto bail2;
   }

   if (MET_SET_TYPE_BARE != arg_set[OPP_ARG]->type) {
      ERROR(sym->origin, "opp or square argument of wrong type",
	    SEV_FATAL);
      goto bail1;
   }

   MET_op_reconcile_sets(arg_set, OPP_NUM_KIDS);

   how_many = DATA_NUM_VERTS(arg_set[OPP_ARG]->info.bare.data);
   
   result_set = MET_type_to_polybare_set(my_result_type, how_many,
					 arg_set, OPP_NUM_KIDS);
   MET_TYPE_FREE(my_result_type);

   if (YES == is_square)
      result = MET__op_opp_guts_square(arg_set[OPP_ARG]->info.bare.data,
				       result_set->info.bare.data);
   else
      result = MET__op_opp_guts(arg_set[OPP_ARG]->info.bare.data,
				result_set->info.bare.data);
   
   if (FAILURE == result) {
      MET_SET_FREE(result_set);
      result_set = MET_NULL_SET;
   }
   
 bail1:
   MET_set_get_type(result_set, result_type, set_type);

 bail2:
   COND_FREE(OPP_ARG);


   return result_set;
}
