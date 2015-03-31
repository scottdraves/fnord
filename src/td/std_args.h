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

/* $Id: std_args.h,v 1.9 1993/06/03 14:16:24 rch Exp $ */

#ifndef STD_ARGS_HAS_BEEN_INCLUDED
#define STD_ARGS_HAS_BEEN_INCLUDED

#include "sym/sym.h"

#define STD_ARG_TOP		0
#define STD_ARG_LEFT		1
#define STD_ARG_WIDTH		2
#define STD_ARG_HEIGHT		3
#define STD_ARG_DIVIDE		4
#define STD_ARG_SET		5
#define STD_ARG_MIN		6
#define STD_ARG_MAX		7
#define STD_ARG_INIT		8
#define STD_ARG_TYPE		9
#define STD_ARG_PARENT		10
#define STD_ARG_RIGHT		11
#define STD_ARG_BOTTOM		12
#define STD_ARG_START		13
#define STD_ARG_END		14
#define STD_ARG_ALL		15
#define STD_ARG_NAME		16
#define STD_ARG_WINDOW		17
#define STD_ARG_SLIDER		18
#define STD_ARG_VIEW		19
#define STD_ARG_OBJECT		20
#define STD_ARG_EDITOR		21
#define STD_ARG_DRAG		22
#define STD_ARG_COLOR		23
#define STD_ARG_PRINTER		24
#define STD_ARG_APPEND		25
#define STD_ARG_BITMAP		26
#define STD_ARG_ZOOM		27
#define STD_ARG_FIELD		28
#define STD_ARG_CHECKBOX	29
#define STD_ARG_TYPEIN		30
#define STD_ARG_TAPEDECK	31
#define STD_ARG_SPEED		32
#define STD_ARG_EVOLVE		33
#define STD_ARG_NEXT		34
#define STD_ARG_BUTTON		35
#define STD_ARG_OUTPUT		36
#define STD_ARG_POINT		37
#define STD_ARG_DIMS		38
#define STD_ARG_STROBE		39
#define STD_ARG_MODIFIERS	40
#define STD_ARG_READ            41
#define STD_ARG_WRITE           42
#define STD_ARG_PORT      	43
#define STD_ARG_HOST		44
#define STD_ARG_CAMERA		45
#define STD_ARG_VIDEO		46
#define STD_ARG_NUM		47

extern Sym std_args[];


#endif STD_ARGS_HAS_BEEN_INCLUDED

