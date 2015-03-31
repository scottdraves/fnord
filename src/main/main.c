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

#include "global/global.h"
#include "port/time.h"
#include "met/met.h"
#include "sym/sym.h"
#include "parse/scan.h"
#include "parse/parse.h"
#ifdef X_TWO_D
#include "td/td.h"
#endif

#include <fcntl.h>
#include <signal.h>
#include <errno.h>

extern char *getenv();

#define PRINT (void) printf



#ifndef lint
static char Version[] =
   "$Id: main.c,v 1.9 1992/06/22 17:04:11 mas Exp $";
#endif /* undef lint */

extern int yydebug;

int
my_read(fd, buf, count)
   int fd;
   char *buf;
   int count;
{
   int ready;
   ME(my_read);
   
   td_fd_add(fd, count);
   ready = td_fd_check();
   
   scan_prompt(fd);
   
   /* do window system tasks until some input shows up */
   while (! ready) {
      td_work();
      ready = td_fd_check();
   }
   return td_fd_get_string(buf, count);
}

static void
set_met_defaults()
{
   METdefaults d;
   ME(set_met_defaults);

   if(getenv("FNORD_OPTIMIZE_OFF")) {
      d.optimizations.peephole = NO;
      d.optimizations.cse_elimination = NO;
      METdefaults_set(&d, MET_DEFAULTS_OPTIMIZATIONS);
   }
}

static void
initialize(argc, argv)
   int *argc;
   char *argv[];
{
   ME(initialize);

   time_begin();
   METinitialize();
   set_met_defaults();
   FILOCinitialize();
   parse_init(PARSE_ALL_FLAGS);
   td_init(argc, argv);

   if (NULL != getenv("YYDEBUG"))
      yydebug = 1;

}

void
clean_up()
{
   ME(clean_up);

   td_destroy();
   parse_exit();

   METexit();

#if defined(DEBUG) && defined(SunOS)
   if (getenv("MALLOCMAP")) {
      PRINT("malloc memory map\n"); 
     mallocmap();
   }
#endif

   printf("\nend of input \n");

   time_end(FNULL, "program");
}

static void
handle_int()
{
   ME(handle_int);

   PRINT("interrupted, exiting.\n");
   (void) signal(SIGINT, SIG_DFL);
   clean_up();
   (void) exit(0);
}

static void
parse_files(count, names)
   int count;
   char *names[];
{
   int fd;
   int i;
   Sym filesym;
   ME(parse_files);

   for(i = 0; i < count; i++)
      if ('-' != names[i][0]) {
	 if (0 == strcmp(names[i], "stdin")) {
	    fd = 0;
	    filesym = sym_from_string("stdin");
	 } else {
	    if ((fd = scan_open(names[i], &filesym)) == -1)
	    {
	       (void) fprintf(stderr, "File %s ", names[i]);
	       perror("could not be opened");
	       continue;
	    }
	 }
	 (void) fnord_parse (filesym, fd, my_read,
			     PARSE_NO_FLAGS, PARSE_NO_FLAGS);
	 td_fd_remove(fd);
      }
}

int
main(argc, argv)
   int		argc;
   char		*argv[];
{
   char		*file_list;

#if defined(DEBUG) && defined(SunOS)
   malloc_debug(2);
#endif

   PRINT("fnord 1.1a\n");
   PRINT("initializing...\n");
   global_init();
   initialize(&argc, argv);
   PRINT("done\n");

#if defined(sparc) && defined(PROFILE) && defined(SITE_BROWN)
   if (getenv("IPROF_RUN")) {
      IPROFsetup(0,0);
      PRINT("profiling on\n");
   }
#endif
   if (1 == argc) {
      PRINT("no files specified, reading from stdin\n");
      file_list = "stdin";
      parse_files(1, &file_list);
   } else
      parse_files(argc - 1, argv + 1);

   (void) signal(SIGINT, handle_int);

   for (;;) {
      td_work();
   }
}
