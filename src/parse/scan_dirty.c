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

/* This file is meant to be included in parse.c after scan.c has
 * been included there.  This is so that these functions can use
 * information that is defined in scan.c after the user declarations 
 * are imported.  This nastiness will allow for a mechanism to copy
 * the state of the scanner and put it on the stack, so that the
 * scanner can sensibly be called recursively.
 */

#ifndef INSIDE_PARSER

#ifndef LINT
static char Version = "$Id: scan_dirty.c,v 1.2 1992/06/22 17:08:10 mas Exp $";
#endif /* undef LINT */

#define INSIDE_SCANNER_ADJUNCT

#include "scan.c"

#endif /* undef INSIDE_PARSER */

/* The routines in this file must treat LEX and FLEX separately,
 * since they mess with information that is very different in the
 * two program's output.  The flex section comes first.
 */

#ifdef FLEX_SCANNER

/* the information that flex needs to know */
struct scan_globals_rec {
   YY_BUFFER_STATE buffer_state;
   YY_CHAR yy_hold_char;
   int yy_n_chars;
   yy_state_type yy_last_accepting_state;
   YY_CHAR *yy_last_accepting_cpos;
   YY_CHAR *yy_c_buf_p;
};

/* save all of flex's foolishly global information, and switch
   all relevant information so that we will start fresh when
   new stuff arrives. */

static void scan_protect_globals ()
{
   YY_BUFFER_STATE temp;
   ME (scan_protect_globals);
   
   if (NULL == scan_saved)
   {
      ALLOCM (scan_saved, struct scan_globals_rec);
      scan_saved->buffer_state = yy_current_buffer;
   }
   
   else if (scan_saved->buffer_state != yy_current_buffer)
   {
      temp = yy_current_buffer;
      yy_delete_buffer (scan_saved->buffer_state);
      yy_current_buffer = temp;
      scan_saved->buffer_state = yy_current_buffer;
   }

   scan_saved->yy_hold_char = yy_hold_char;
   scan_saved->yy_n_chars = yy_n_chars;
   scan_saved->yy_last_accepting_state = yy_last_accepting_state;
   scan_saved->yy_c_buf_p = yy_c_buf_p;

   yy_current_buffer = yy_create_buffer (yyin, YY_BUF_SIZE);
   yy_load_buffer_state();
}

/* restore all of flex's foolishly global information,
   based on the status of the top of the context stack */

static void scan_restore_globals ()
{
   ME(scan_restore_globals);

   if (NULL == scan_saved)
      return;

   if (scan_saved->buffer_state != yy_current_buffer)
   {
      yy_delete_buffer (yy_current_buffer);
      yy_current_buffer = scan_saved->buffer_state;
   }

   yy_hold_char = scan_saved->yy_hold_char;
   yy_n_chars = scan_saved->yy_n_chars;
   yy_last_accepting_state = scan_saved->yy_last_accepting_state;
   yy_c_buf_p = scan_saved->yy_c_buf_p;
   /* flex is about to do : yy_c_buf_p = yytext + YY_MORE_ADJ */
   yytext = yy_c_buf_p - YY_MORE_ADJ;
}

/* Now, for lex..... */

#else 

#endif /* def FLEX_SCANNER */

/* wrapper on yyparse for recursive calls */

int fnord_parse (file, fd, input_reader, parse_flags, mask)
   Sym file;
   int fd;
   int (*input_reader)();
   int parse_flags;
   int mask;
{
   int old_flags;
   int rv;
   ME(fnord_parse);

   scan_context_push (file, fd, input_reader, YES);
   old_flags = parse_global_flags;
   parse_global_flags = (old_flags & ~mask) | (parse_flags & mask);
   parse_global_flags &= ~PARSE_NEW_DEFINITION_FLAG;
   time_begin();
   rv = yyparse();
   time_end (FNULL, SYM_STRING(file));
   parse_global_flags = old_flags | 
      (parse_global_flags & PARSE_NEW_DEFINITION_FLAG);
   return rv;
}
