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

/* $Id: td_comm.h,v 1.2 1992/06/22 17:09:52 mas Exp $ */

#ifndef TD_COMM_H_INCLUDED
#define TD_COMM_H_INCLUDED

typedef struct fnw_read {
   int 		socket;
#ifdef X_TWO_D
   XtInputId 	x_input;
#endif
   unsigned long port;
   METset 	*set;
   char 	*buf;
   int 		size;
   int 		len;
   METlist 	*list;
   FOboolean 	connected;
} fnw_read;

typedef struct fnw_write {
   int 		socket;
#ifdef X_TWO_D
   XtInputId 	x_input;
   XtIntervalId timer;
#endif
   unsigned long port;
   Sym 		host;
   METset 	*set;
   FOboolean 	connected;
} fnw_write;

extern fnw_type fnw_type_read;
extern fnw_type fnw_type_write;

#endif /* TD_COMM_H_INCLUDED */
