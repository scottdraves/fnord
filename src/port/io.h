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

/**********************************************************************
 *
 *	io.h :	contains I/O related definitions, e.g. to make
 *		select() run on different architectures
 *
 **********************************************************************/

#ifndef _IO_H_
#define _IO_H_

/* $Id: io.h,v 1.4 1992/06/22 17:08:25 mas Exp $ */

#include "port.h"

/* get fcntl.h */
#if defined(IBM6000)
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif

#ifdef SYSV
extern char *getcwd();
#define getwd(string) getcwd(string,200)
#else
extern char *getwd();
#endif


#endif /* undef(_IO_H_) */
