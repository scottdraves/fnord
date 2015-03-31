/* $Id: err.h,v 1.4 1992/06/22 17:03:21 mas Exp $ */

/*
 * Copyright 1986, Brown University, Providence, RI
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

#ifndef ERR_INCLUDED
#define ERR_INCLUDED

#include "port/port.h"
#include <stdio.h>
#include <string.h>

typedef struct ERRid ERRid;

/* Message severity */
#define ERR_INFO	 0
#define ERR_WARNING	 1
#define ERR_SEVERE	 2
#define ERR_FATAL	 3
#define ERR_NUM_SEV	 4
#define ERR_NO_CORE_DUMP 4		/* Must be > all severities */
#define ERR_ALL 	 10

#define ERR_GLOBAL_FILE  ((FILE *)0)
#define ERR_NO_FILE	 ((FILE *)1)

/* No limit for ERR_ERROR_AND_COUNT */
#define ERR_NO_MAX	(-1)

struct ERRid {
  char *pname;			/* Package name				*/
  int  totals[ERR_NUM_SEV];	/* Counts by severity level		*/
  int  magic;			/* Make sure this is an ERRid 		*/
  int  *err_nums, *err_cnts;	/* Realloc'ed arrays for counting errors */
  int  num_cnts;		/* Size of above arrays			*/
  int  core_level;		/* Severity required for core dump	*/
  FILE *error_stream;		/* place to write errors to, if this is */
				/* ERR_GLOBAL_FILE then use the global */
				/* default file (err_fp).  if it's */
				/* ERR_NO_FILE, then it is left untouched */
				/* in ERRbuffer */
};

#define ERR_PACKAGE_NAME(id) (id->pname)

#define ERR_TOTAL_ERRORS(id, sev) (id->totals[sev])

#define ERR_ERROR(id, severity, msg) \
   (void) sprintf msg, ERRreport(id, severity)

#define ERR_ERROR_AND_COUNT(id, err_no, max, severity, msg) \
   (void) sprintf msg, ERRreport_and_count(id, err_no, max, severity)

/* errno, sys_errlist, and sys_nerr are unix provided globals */
#define ERR_PERROR_MSG	(errno > sys_nerr ? "BAD ERRNO:" : sys_errlist[errno])

/* Format choice for ERR_REPORT_ERROR */
#define ERR_B		ERRbuffer
#define ERR_S		ERR_B, "%s"
#define ERR_SI		ERR_B, "%s %d"
#define ERR_SX		ERR_B, "%s %x"
#define ERR_SD		ERR_B, "%s %g"
#define ERR_SS		ERR_B, "%s %s"
#define ERR_SSS 	ERR_B, "%s %s %s"
#define ERR_SSI 	ERR_B, "%s %s %d"
#define ERR_SVEC	ERR_B, "%s (%g, %g, %g)"
#define ERR_SFILE	ERR_B, "%s file \"%s\""
#define ERR_SFILE_LINE	ERR_B, "%s at line %d in file \"%s\""
#define ERR_ALLOC1	ERR_B, "Unable to allocate space for %s"
#define ERR_ALLOC2	ERR_B, "Unable to allocate space for %s in routine %s"
#define ERR_REALLOC1	ERR_B, "Unable to reallocate space for %s"
#define ERR_REALLOC2	ERR_B, "Unable to reallocate space for %s in routine %s"
#define ERR_PERRORS	ERR_B, "%s %s", ERR_PERROR_MSG
#define ERR_PERROR	ERR_B, "%s", ERR_PERROR_MSG
#define ERR_ARB 	ERR_B

/* Global variable in ERR.c */
extern char		ERRbuffer[];

/* In ERR.c */
extern ERRid 		*ERRregister_package();
extern void		ERRroute_errors();
extern void		ERRclear_count();
extern void		ERRclear_total();
extern void		ERRset_core_dump_level();
extern void		ERRreport();
extern void		ERRreport_and_count();
extern void		ERRroute_errors_package();

extern char  *sys_errlist[];
extern int   sys_nerr, errno;
#endif
