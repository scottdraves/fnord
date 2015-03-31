/*LINTLIBRARY*/
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

#include "err.h"
#include "defs.h"

#ifndef lint
static char Version[] =
   "$Id: err.c,v 1.3 1992/06/22 17:03:18 mas Exp $";
#endif

char ERRbuffer[MAX_LEN];

static FILE *err_fp = stderr;

static char *sev_msgs[ERR_NUM_SEV] =
		  {"", " warning", " error", " fatal error"};

/* Create an ERRid, store the name in it, and return it to the user.
*/
ERRid *
ERRregister_package(name)
  register char *name;
{
  ERRid *id;

  if (name == NULL || name[0]=='\0')
     FPRINTF(stderr, "ERR: NULL or blank passed to ERRregister_package.\n");

  MALLOC(id, ERRid, 1, "ERRid", "ERRregister_package");
  MALLOC(ERR_PACKAGE_NAME(id), char, strlen(name) + 1, "package name",
		  "ERRregister_package");

  (void) strcpy(ERR_PACKAGE_NAME(id), name);
  MAGIC_NUM(id)       = MAGIC;
  NUM_COUNTS(id)      = 0;
  CORE_DUMP_LEVEL(id) = ERR_NO_CORE_DUMP;
  id->error_stream    = ERR_GLOBAL_FILE;

  ERRclear_total(id, ERR_ALL);

  return id;
}


/* Change the file pointers on which to write messages.
** Normally stderr.
*/
void
ERRroute_errors(fp)
  FILE *fp;
{
  if (fp != NULL) err_fp = fp;
}


/* Clear the count for a particular error number. No message is printed
** if the error number isn't found, because it just might not have
** ever been reported.
*/
void
ERRclear_count(id, err_num)
  ERRid *id;
  int	err_num;
{
  int i;

  CHECK_MAGIC(id, "ERRclear_count");

  for (i = 0; i < NUM_COUNTS(id); i++)
     if (NUMS(id)[i] == err_num) COUNTS(id)[i] = 0;
}

int
ERRget_count(id, err_num)
  ERRid *id;
  int	err_num;
{
  int i;

  CHECK_MAGIC(id, "ERRget_count");

  for (i = 0; i < NUM_COUNTS(id); i++)
     if (NUMS(id)[i] == err_num) return COUNTS(id)[i];

  return 0;
}

/* Clear the error totals in an ERRid.
*/
void
ERRclear_total(id, sev)
  ERRid *id;
  int	sev;
{
  int i;

  CHECK_MAGIC(id, "ERRclear_total");

  if (sev != ERR_ALL) ERR_TOTAL_ERRORS(id, sev) = 0;

  else for (i = 0; i < ERR_NUM_SEV; i++) ERR_TOTAL_ERRORS(id, i) = 0;
}

/* Set the minimum severity required to force a core dump */
void
ERRset_core_dump_level(id, sev)
   ERRid *id;
   int	 sev;
{
  CHECK_MAGIC(id, "ERRclear_total");

  CORE_DUMP_LEVEL(id) = sev;
}


/* Report an error. This is called by the ERR_ERROR macro after
** the message is sprintf'ed into the ERRbuffer.
*/

void
ERRreport(id, sev)
  ERRid *id;
  int	sev;
{
   FILE *stream;
  /* Make sure the error message interleaves correctly */
  (void) fflush(stdout);

  if (id == NULL || MAGIC_NUM(id) != MAGIC) {
    FPRINTF(stderr, "ERR: invalid ERRid passed to ERRreport.\n");
    FPRINTF(stderr, "ERR: message was '%s'.\n", ERRbuffer);
    exit(1);
  }

  if (ERR_NO_FILE != id->error_stream) {
     stream = (ERR_GLOBAL_FILE == id->error_stream) ?
	err_fp : id->error_stream;
     (void) fprintf(stream, "%s%s: %s.\n",
		    ERR_PACKAGE_NAME(id), sev_msgs[sev], ERRbuffer);
  }

  ERR_TOTAL_ERRORS(id, sev) ++;

  if (sev >= CORE_DUMP_LEVEL(id)) {
    FPRINTF(stderr, "ERR: Dumping core.\n");
    (void) abort();
  }

  if (sev == ERR_FATAL) exit(1);
}


/* Report an error and keep track of the error number.
*/
void
ERRreport_and_count(id, err_num, max, sev)
  ERRid *id;
  int	err_num, max, sev;
{
  int idx, found = FALSE;

  for (idx = 0; idx < NUM_COUNTS(id); idx++)
     if (NUMS(id)[idx] == err_num) {
       COUNTS(id)[idx]++;
       found = TRUE;
       break;
     }

  if (! found) {
     idx = NUM_COUNTS(id);

     /* If this is the first error number to count, malloc arrays */
     if (idx == 0) {
       MALLOC(NUMS(id), int, 1, "err nums", "ERRreport_and_count");
       MALLOC(COUNTS(id), int, 1, "err cnts", "ERRreport_and_count");
     }

     /* Otherwise, resize them with realloc */
     else {
       REALLOC(NUMS(id), int, idx + 1, "err nums", "ERRreport_and_count");
       REALLOC(COUNTS(id), int, idx + 1, "err cnts", "ERRreport_and_count");
     }

     /* Count this message for the first time */
     NUMS(id)[idx]   = err_num;
     COUNTS(id)[idx] = 1;

     NUM_COUNTS(id) ++;
  }

  /* Report the error if it hasn't been seen too many times already */
  if (max == ERR_NO_MAX || COUNTS(id)[idx] <= max) ERRreport(id, sev);
}

void
ERRroute_errors_package(id, stream)
   ERRid	*id;
   FILE		*stream;
{
   id->error_stream = stream;
}
   
