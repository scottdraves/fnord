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

/* $Id: ops.h,v 1.18 1992/06/22 17:07:42 mas Exp $ */

#ifndef OPS_H_HAS_BEEN_INCLUDED
#define OPS_H_HAS_BEEN_INCLUDED

#include "met/defs.h"

typedef METset	   *(*MET_pfr_set)();

#define OP_NAME(OP)		(MET_op_info_table[OP].name)
#define OP_FUNCTION(OP)		(MET_op_info_table[OP].function)
#define OP_NUM_ARGS(OP)		(MET_op_info_table[OP].num_args)

typedef struct MET_op_info MET_op_info;

struct MET_op_info {
   char		*name;
   MET_pfr_set	function;   /* function to evaluate this op */
   int		prime_rule; /* one of PRIME_RULE_XXX */
   int		num_args;   /* (bit i = 1) => (i is a valid # of args) */
};

#define OP_NO_ARG	0x01
#define OP_ONE_ARG	0x02
#define OP_TWO_ARG	0x04
#define OP_THREE_ARG	0x08
#define OP_FOUR_ARG	0x10

extern MET_op_info MET_op_info_table[];


#define MET_SYM_OP_NONE		        (-1)
#define MET_SYM_OP_NOP			0	/* ORGANIZATIONAL */
#define MET_SYM_OP_PRINT		1 	/* stdout */
#define MET_SYM_OP_PAIR_BUILD		2 	/* a, b -> (a,b) */
#define MET_SYM_OP_PAIR_LEFT		3 	/* (a,b) -> a */
#define MET_SYM_OP_PAIR_RIGHT		4	/* (a,b) -> b */
#define MET_SYM_OP_ROW_BUILD		5	/* (a,b) -> [a,b] */
#define MET_SYM_OP_COLUMN_BUILD		6 	/* (a,b) -> [a,b]t */
#define MET_SYM_OP_ROW_EXTRACT		7 	/* [x,y,z],2 -> y */
#define MET_SYM_OP_COLUMN_EXTRACT	8	/* [x,y,z]t,3 -> z */
#define MET_SYM_OP_CONCATENATE		9       /* [x,y],[z,w] -> [x,y,z,w] */
#define MET_SYM_OP_VECTOR_TO_PAIRS	10      /* [x,y,z] -> (x, y, z) */
#define MET_SYM_OP_PAIR_BUILD_3		11	/* a,b,c -> (a,b,c) */
#define MET_SYM_OP_PAIR_BUILD_4		12
#define MET_SYM_OP_FLATTEN		13	/* (a,b),(c,d) -> (a,b,c,d) */
#define MET_SYM_OP_SET_TO_PAIRS		14	/* {a, b} -> (a, b) */
#define MET_SYM_OP_BAD_ARGS		15	/* always an error */
#define MET_SYM_OP_TRACE_ON		16
#define MET_SYM_OP_TRACE_OFF		17
#define MET_SYM_OP_UNASSIGNED_8		18
#define MET_SYM_OP_OPPOSITE		19 	/* NUMERICAL - real */
#define MET_SYM_OP_ADD			20      /* (most also apply to  */
#define MET_SYM_OP_SUBTRACT		21 	/* complex numbers) */
#define MET_SYM_OP_MULTIPLY		22 
#define MET_SYM_OP_DIVIDE		23 
#define MET_SYM_OP_ABS			24 
#define MET_SYM_OP_SIN			25 
#define MET_SYM_OP_COS			26 
#define MET_SYM_OP_TAN			27 
#define MET_SYM_OP_ASIN			28 
#define MET_SYM_OP_ACOS			29 
#define MET_SYM_OP_ATAN			30 
#define MET_SYM_OP_ATAN2		31 
#define MET_SYM_OP_SINH			32 
#define MET_SYM_OP_COSH			33 
#define MET_SYM_OP_TANH			34 
#define MET_SYM_OP_ASINH		35 
#define MET_SYM_OP_ACOSH		36 
#define MET_SYM_OP_ATANH		37 
#define MET_SYM_OP_LN			38      /* base e */
#define MET_SYM_OP_LOG			39      /* base 10 */
#define MET_SYM_OP_LG			40      /* base 2 */
#define MET_SYM_OP_POW			41 
#define MET_SYM_OP_SQUARE		42 
#define MET_SYM_OP_SQRT			43 
#define MET_SYM_OP_EXP			44 
#define MET_SYM_OP_ERF			45 
#define MET_SYM_OP_GAMMA		46 
#define MET_SYM_OP_ADD_3		47	/* a + b + c */
#define MET_SYM_OP_ADD_4		48	/* a + b + c + d */
#define MET_SYM_OP_MULTIPLY_3		49	/* abc */
#define MET_SYM_OP_MULTIPLY_4		50	/* abcd */
#define MET_SYM_OP_SIN_COS		51	/* x -> (sin x, cos x) */
#define MET_SYM_OP_EXP_EXP		52	/* x -> (exp x, exp -x) */
#define MET_SYM_OP_POLYNOMIAL		53	/* x,[a,b] -> ax^0 + bx^1 */
#define MET_SYM_OP_MIN			54
#define MET_SYM_OP_MAX			55
#define MET_SYM_OP_UNASSIGNED_18	56
#define MET_SYM_OP_UNASSIGNED_19	57
#define MET_SYM_OP_UNASSIGNED_20	58
#define MET_SYM_OP_UNASSIGNED_21	59
#define MET_SYM_OP_CONJUGATE		60      /* NUMERICAL - complex */
#define MET_SYM_OP_RE			61 
#define MET_SYM_OP_IM			62 
#define MET_SYM_OP_COMPLEX_BUILD	63	/* (a, b) -> a + bi */
#define MET_SYM_OP_COMPLEX_FRAG		64 	/* a + bi -> (a, b) */
#define MET_SYM_OP_UNASSIGNED_24	65 
#define MET_SYM_OP_UNASSIGNED_25	66 
#define MET_SYM_OP_UNASSIGNED_26	67 
#define MET_SYM_OP_UNASSIGNED_27	68 
#define MET_SYM_OP_UNASSIGNED_28	69 
#define MET_SYM_OP_UNASSIGNED_29	70 
#define MET_SYM_OP_VECTOR_CROSS		71       /* NUMERICAL - vector */
#define MET_SYM_OP_DOT			72  
#define MET_SYM_OP_NORM			73  	 /* F^n -> R */
#define MET_SYM_OP_NORMALIZE		74  
#define MET_SYM_OP_SUM_OF_ENTRIES	75  	 /* F^n -> F */
#define MET_SYM_OP_UNASSIGNED_30	76
#define MET_SYM_OP_UNASSIGNED_31	77
#define MET_SYM_OP_UNASSIGNED_32	78
#define MET_SYM_OP_UNASSIGNED_33	79
#define MET_SYM_OP_UNASSIGNED_34	80
#define MET_SYM_OP_UNASSIGNED_35	81
#define MET_SYM_OP_UNASSIGNED_36	82
#define MET_SYM_OP_TRACE		83      /* NUMERICAL - matrix */
#define MET_SYM_OP_DET			84 
#define MET_SYM_OP_TRANSPOSE		85 
#define MET_SYM_OP_INVERSE		86 
#define MET_SYM_OP_ADJOINT		87 
#define MET_SYM_OP_EVALUES		88       /* matrix -> vector */
#define MET_SYM_OP_EVECTORS		89       /* matrix -> matrix */
#define MET_SYM_OP_ROTATE		90       /* axis, angle -> matrix */
#define MET_SYM_OP_ORIENT		91       /* from, at, up -> matrix */
#define MET_SYM_OP_PERMUTE		92	 /* COMMUNICATIONAL */
#define MET_SYM_OP_SHIFT		93 
#define MET_SYM_OP_UNASSIGNED_39	94 
#define MET_SYM_OP_UNASSIGNED_40	95 
#define MET_SYM_OP_UNASSIGNED_41	96 
#define MET_SYM_OP_UNASSIGNED_42	97 
#define MET_SYM_OP_UNASSIGNED_43	98 
#define MET_SYM_OP_UNASSIGNED_44	99 
#define MET_SYM_OP_CEILING		100     /* NUMERICAL - integer */
#define MET_SYM_OP_FLOOR		101	/* associated with, not */
#define MET_SYM_OP_SGN			102	/* necc. yielding or */
#define MET_SYM_OP_MODULO		103	/* acting on */
#define MET_SYM_OP_CHOOSE		104	/* binomial coef */
#define MET_SYM_OP_FACTORIAL		105
#define MET_SYM_OP_ENUMERATE		106	/* a, b -> (next line) */
#define MET_SYM_OP_UNASSIGNED_46	107	/* a,a+1,a+2,...,b-1,b */
#define MET_SYM_OP_UNASSIGNED_47	108
#define MET_SYM_OP_UNASSIGNED_48	109
#define MET_SYM_OP_UNASSIGNED_49	110
#define MET_SYM_OP_UNASSIGNED_50	111
#define MET_SYM_OP_UNASSIGNED_51	112
#define MET_SYM_OP_ZTWO_TO_INTEGER	113  /* NUMERICAL - conversion */
#define MET_SYM_OP_INTEGER_TO_REAL	114
#define MET_SYM_OP_ANGLE_TO_REAL	115
#define MET_SYM_OP_REAL_TO_COMPLEX	116
#define MET_SYM_OP_REAL_TO_ANGLE	117
#define MET_SYM_OP_REAL_TO_INTEGER	118
#define MET_SYM_OP_INTEGER_TO_ZTWO	119
#define MET_SYM_OP_UNASSIGNED_52	120
#define MET_SYM_OP_UNASSIGNED_53	121
#define MET_SYM_OP_UNASSIGNED_54	122
#define MET_SYM_OP_UNASSIGNED_55	123
#define MET_SYM_OP_UNASSIGNED_56	124
#define MET_SYM_OP_REDUCE		125
#define MET_SYM_OP_SET_CROSS		126	/* SETICAL */
#define MET_SYM_OP_IN			127
#define MET_SYM_OP_UNION		128
#define MET_SYM_OP_INTERSECTION		129
#define MET_SYM_OP_EXCLUSION		130
#define MET_SYM_OP_CARDINALITY		131
#define MET_SYM_OP_SET_OF		132	/* (a,b,c) -> {a,b,c} */
#define MET_SYM_OP_INTERVAL		133	/* (a, b) -> [a, b] */
#define MET_SYM_OP_CYLINDER		134
#define MET_SYM_OP_SPHERE		135
#define MET_SYM_OP_TRIANGLE		136
#define MET_SYM_OP_TORUS		137
#define MET_SYM_OP_MEASURE		138	/* area, volume, etc. */
#define MET_SYM_OP_SET_ZTWO		139	/* create canonical */
#define MET_SYM_OP_SET_INTEGERS		140	/* sets, eg, set of all */
#define MET_SYM_OP_SET_ANGLES		141	/* the reals */
#define MET_SYM_OP_SET_REALS		142
#define MET_SYM_OP_SET_COMPLEXES	143
#define MET_SYM_OP_SAMPLE		144	/* interval of pts */
#define MET_SYM_OP_NOT			145	/* LOGICAL */
#define MET_SYM_OP_AND			146
#define MET_SYM_OP_OR			147
#define MET_SYM_OP_BITNOT		148
#define MET_SYM_OP_BITAND		149
#define MET_SYM_OP_BITOR		150
#define MET_SYM_OP_BITXOR		151
#define MET_SYM_OP_EPSILON_EQUAL	152
#define MET_SYM_OP_EXACT_EQUAL		153
#define MET_SYM_OP_NOTEQUAL		154
#define MET_SYM_OP_GREATER		155
#define MET_SYM_OP_LESS			156
#define MET_SYM_OP_GREATEROREQUAL	157
#define MET_SYM_OP_LESSOREQUAL		158
#define MET_SYM_OP_SET_NULL		159
#define MET_SYM_OP_ZERO			160	/* polymorphic zero */
#define MET_SYM_OP_IDENTITY		161	/* polymorphic one */
#define MET_SYM_OP_UNASSIGNED_69	162
#define MET_SYM_OP_UNASSIGNED_70	163
#define MET_SYM_OP_UNASSIGNED_71	164
#define MET_SYM_OP_TYPE			165     /* SYMBOLICAL */
#define MET_SYM_OP_IMPLICIT		166
#define MET_SYM_OP_MAP			167
#define MET_SYM_OP_PRIME		168
#define MET_SYM_OP_DERIVATIVE		169
#define MET_SYM_OP_IF			170	/* error on else */
#define MET_SYM_OP_IF_ELSE		171
#define MET_SYM_OP_ROW_LOOP		172
#define MET_SYM_OP_COLUMN_LOOP		173
#define MET_SYM_OP_TYPE_UNKNOWN		174	/* an unknown node for */
#define MET_SYM_OP_COMPILED_CODE	175	/* type expressions */
#define MET_SYM_OP_FOR_EACH		176
#define MET_SYM_OP_TYPE_AND_CAST	177
#define MET_SYM_OP_TYPE_MATCH		178
#define MET_SYM_OP_TYPE_MATCH_X		179
#define MET_SYM_OP_UNASSIGNED_78        180
#define MET_SYM_OP_UNASSIGNED_79	181
#define MET_SYM_OP_CROSS		182	/* vector or set */
#define MET_SYM_OP_BARS			183	/* norm or abs */
#define MET_SYM_OP_APPLY		184
#define MET_SYM_OP_OUT_PROD_MAP		185
#define MET_SYM_OP_DOUBLE_MAP		186
#define MET_SYM_OP_QUOTE		187	/* `symbol */
#define MET_SYM_OP_ALIST		188	/* symbol<-expr */
#define MET_SYM_OP_ASSOC		189	/* find symbol in alist */
#define MET_SYM_OP_LOCAL_EFFECT		190
#define MET_SYM_OP_UNASSIGNED_89	191
#define MET_SYM_OP_UNASSIGNED_90	192
#define MET_SYM_OP_UNASSIGNED_91	193


#endif OPS_H_HAS_BEEN_INCLUDED
