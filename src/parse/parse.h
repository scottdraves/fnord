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

/* $Id: parse.h,v 1.7 1992/06/22 17:07:56 mas Exp $ */

#ifndef _PARSE_H_
#define _PARSE_H_

#include "met/met.h"
#include "filoc/filoc.h"

#define PARSE_NO_FLAGS            0x00
#define PARSE_USE_WIDGETS_FLAG    0x01
#define PARSE_DEFINE_SYMBOLS_FLAG 0x02
#define PARSE_EXEC_COMMANDS_FLAG  0x04
#define PARSE_PRINT_EXPR_FLAG     0x08
#define PARSE_ALL_FLAGS		  0x0f

extern FILOCloc *parse_new_filoc();
extern void parse_init();
extern void parse_exit();
extern void parse_fix();
extern void parse_clear();
extern METset *parse_last_expr();
extern int fnord_parse();

extern METenv *parse_root_environment;

#endif /* ndef _PARSE_H_ */
