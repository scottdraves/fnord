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

/* $Id: port.h,v 1.9 1992/06/22 17:08:31 mas Exp $ */

/**********************************************************************
 *
 *	port.h:	contains miscellaneous info for porting fnord to
 *		different architectures
 *
 **********************************************************************/

#ifndef _PORT_H_
#define _PORT_H_

/* Stellar uses SysV instead of SYSV for some bizarre reason */
#if defined(SysV) || defined(hpux) || defined(IBM6000) || defined(IRIX)
#define SYSV
#endif

/* Declare memory allocation routines */
#if defined(IBM6000)
#include <stdlib.h>
#else
#if defined(ultrix) || defined(SYSV) || defined(SunOS)
#include <malloc.h>
#else
extern char *malloc();
extern char *calloc();
extern char *realloc();
extern void free();
#endif /* undef(ultrix) && undef(SYSV) && undef(SunOS) */
#endif /* undef(IBM6000) */


/* Declare exit routine */
#if defined(SYSV) || defined(ultrix) || defined(IBM6000)
extern void exit();
#elif defined(lint)
/* lint insists on this exit */
extern void exit();
#else
extern int exit();
#endif

/* Declare sprintf routine */
#if defined(SYSV) || defined(IBM6000)
extern int sprintf();
#else
extern char *sprintf();
#endif

/* Declare perror routine */
#if defined(SYSV) || defined(ultrix) || defined(IBM6000)
extern void perror();
#else
extern int perror();
#endif

/* Declare getenv routine Ack! */
#if defined(SunOS)
extern char *getenv();
#endif

/* should do these the other way around: for once, sysV's is better */
/*
 * The macros below do not obey standards in the name of simplifying
 * porting to System V UNIX
 *
 */
#ifdef SYSV
#define bcopy(from, to, n)    memcpy(to, from, n)
#define bcmp(s1, s2, n)       memcmp(s1, s2, n)
#define bzero(addr, n)	      (void)memset(addr, 0, n)
#define index		      strchr
#define rindex		      strrchr
#endif

/* macros to put data transmitted between processes into a 
   standard form---network is Big-Endian */

#define ALIGN_FOR_NEXT(BUF, LEN, TEMP)  \
   ((TEMP) = ((int) (BUF) % 4 == 0) ? 0 : 4 - (int) (BUF) % 4, \
    (LEN) -= (TEMP),                                           \
    (BUF) += (TEMP),                                           \
    (LEN) > 0)
#define WRITE_CHAR(BUF, CHAR)    (*(BUF)++ = (CHAR))
#define WRITE_SHORT(BUF, INT)    (* CAST(short int *, BUF) = htons(INT), \
				  (BUF) += sizeof(short int))
#define WRITE_INTEGER(BUF, INT)  (* CAST(int *, BUF) = htonl(INT), \
				  (BUF) += sizeof(int))
#define WRITE_FLOAT(BUF, FLOAT)  (* CAST(float *, BUF) = (float) (FLOAT), \
                                  (BUF) += sizeof(float))
#define WRITE_DOUBLE(BUF, DOUB)  (* CAST(double *, BUF) = (double) (DOUB), \
				  (BUF) += sizeof(double))
#define READ_CHAR(BUF, CHAR)     ((CHAR) = *(BUF)++)
#define READ_SHORT(BUF, INT)	 ((INT) = ntohs(* CAST(short int *, BUF)), \
				  (BUF) += sizeof(short int))
#define READ_INTEGER(BUF, INT)	 ((INT) = ntohl(* CAST(int *, BUF)), \
				  (BUF) += sizeof(int))
#define READ_FLOAT(BUF, FLOAT)   ((FLOAT) = * CAST(float *, BUF), \
				  (BUF) += sizeof(float))
#define READ_DOUBLE(BUF, DOUB)   ((DOUB) = * CAST(double *, BUF), \
				  (BUF) += sizeof(double))
				  
#endif /* undef(_PORT_H_) */

