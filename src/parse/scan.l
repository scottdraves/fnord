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

/* -*- C -*- */
/* Lexical analyzer description file for Fnord */
/* Compatible with both lex and flex. */

/* $Id: scan.l,v 1.26 1992/06/22 17:08:07 mas Exp $ */

%{

#include <fcntl.h>

#include "parse/scan.h"

/***********************************************************************
 *
 * include the token definitions from bison -d, unless we have been
 * #included in the parser already
 *
 ***********************************************************************/
#ifndef INSIDE_PARSER
#ifndef INSIDE_SCANNER_ADJUNCT

#ifndef lint
static char Version[] =
   "$Id: scan.l,v 1.26 1992/06/22 17:08:07 mas Exp $";
#endif /* undef lint */

#endif /* ndef INSIDE_SCANNER_ADJUNCT */

#include "parse.tab.h"

#endif /* ndef INSIDE_PARSER */

/***********************************************************************
 *
 * miscellaneous declarations
 *
 ***********************************************************************/

/* conversion routines to use on yytext */
extern double atof();
extern int    atoi();

/* our static functions */
static int nothing_there();
static void cmd_init();
static int fill_buffer();
static void scan_context_push();
static void scan_protect_globals();
static void scan_restore_globals();
static FOboolean scan_include();

#define SCAN_HOOK ME(scanner)

/* input context definition & related declarations */
typedef struct scan_globals_rec *scan_globals;

/* complete input context */
struct scan_context {
   int lineno;		/* current line number */
   Sym file;		/* Sym for current file */
   int fd;		/* descriptor for file if open, else -1 */
   int isatty;
   int (*read)();	/* how to access "file" */
   FOboolean hard;	/* true if can be popped */

   char *buf;		/* buffer for file */
   int bsize;		/* size of buffer */
   char *bptr;		/* current position in buffer */
   char *bstop;		/* position at which to stop and read again */
   char *bnext;		/* next position to read into */
   
   scan_globals globals;/* what to restore when we pop to */

   struct scan_context *last;
};

/* current context: top of a stack containing nested contexts caused
 * by #include, etc. */
struct scan_context *scan_current_context = NULL;

#define scan_buf	(scan_current_context->buf)
#define scan_bsize	(scan_current_context->bsize)
#define scan_bptr	(scan_current_context->bptr)
#define scan_bstop	(scan_current_context->bstop)
#define scan_bnext	(scan_current_context->bnext)
#define scan_read	(scan_current_context->read)
#define scan_saved	(scan_current_context->globals)

/***********************************************************************
 *
 * Modify the input routines to our taste.  Since flex and lex have
 * different internals, we have to modify them differently.  First is
 * the flex section.
 * 
 ***********************************************************************/
#ifdef FLEX_SCANNER

#undef yywrap

/* use header as befits bison %pure_parser */
#undef YY_DECL
#define YY_DECL int yylex (lvalp, llocp) \
                           YYSTYPE *lvalp;\
			   YYLTYPE *llocp;
#define YYLVAL lvalp ->
#define YYLLOC llocp ->


/* redefine YY_INPUT to use our input-snarfing routine */
/* just in case */
#ifndef min
#define min(a,b) (a)<(b)?(a):(b)
#endif /* ndef min */

#ifdef YY_INPUT
#undef YY_INPUT
#endif /* def YY_INPUT */

/* we can give flex all the characters up to bstop or max_size,
 * whichever is less */
#define YY_INPUT(buf,result,max_size)			    \
   if (((result=min(max_size,scan_bstop-scan_bptr)) == 0) && \
       ((result=fill_buffer()) == 0))			      \
      result = 0;					       \
   else bcopy(scan_bptr,buf,result), scan_bptr+=result


/***********************************************************************
 *
 * Now the lex section:
 * Change lex's input and unput macros so they don't use yylineno...
 * we will keep track of that ourselves with an action for "\n".
 * Also, use our input snarfing routine to get more characters.
 * 
 ***********************************************************************/
#else /* not flex, must be lex */

/* extra data for a token */
extern YYSTYPE yylval;

/* location data for a token */
extern YYLTYPE yylloc;

#define YYLVAL yylval.
#define YYLLOC yylloc.

#undef input
#undef unput

/* input() must return the next character from the either the unput
 * buffer, the input stream, or 0 for the end of input */
#define input() (yytchar=		 \
		 yysptr>yysbuf ?	  \
		  U(*--yysptr) :	   \
		  scan_bptr != scan_bstop ? \
		   yytchar = *scan_bptr++ :  \
		   fill_buffer()!=0 ?	      \
		    yytchar = *scan_bptr++ :   \
		    0)

#define unput(c) *yysptr++ =(yytchar=(c))

#endif /* lex */


/***********************************************************************
 *
 * My input fetching routine.  Since the main loop is the scanner, we
 * use scan_read as a place to put things like window handling and
 * the like
 * 
 ***********************************************************************/
extern int read(/* int fd, char *buf, int count */);

static int cmd_id;

struct cmd_entry {
   char *name;
   int token;
} cmd_table[] = {
   { "eval", TOKEN_CMD_EVAL },
   { "print", TOKEN_CMD_PRINT },
   { "debug", TOKEN_CMD_DEBUG },
   { "include", TOKEN_CMD_INCLUDE },
   { "require", TOKEN_CMD_REQUIRE },
   { "mod", TOKEN_MOD },
   { "and", TOKEN_AND },
   { "or", TOKEN_OR },
   { "not", TOKEN_NOT },
   { "if", TOKEN_IF },
   { "else", TOKEN_ELSE },
   { "in", TOKEN_IN },
   { "be", TOKEN_BE },
   { "let", TOKEN_LET },
   { "union", TOKEN_UNION },
   { "widget", TOKEN_CMD_WIDGET},
   { "FnordFix", TOKEN_FIX},
   { "FnordClear", TOKEN_CLEAR},
   { "FnordQuit", TOKEN_QUIT},
};

#define NCMDS	(sizeof (cmd_table) / sizeof (struct cmd_entry))

static void
cmd_init()
{
   int i;
   ME(cmd_init);

   cmd_id = METlist_hash_new_id();

   for (i = 0; i < NCMDS; i++)
   {
       (void) METlist_hash_add_int(cmd_id, sym_from_string(cmd_table[i].name),
				   cmd_table[i].token);
   }
}

static int nothing_there ()
{
   return 0;
}

void
scan_init()
{
   ME(scan_init);

   cmd_init();
   scan_context_push (sym_from_string("BOZO"), -1, nothing_there, YES);
}
   

void
scan_exit()
{
   ME(scan_exit);
   METlist_hash_free_id(cmd_id);
}

void
scan_prompt(fd)
   int fd;
{
   extern char *getenv();
   static char *prompt = (char *) -1;

   if ((char *) -1 == prompt)
      prompt = getenv("FNORD_PROMPT");

   if (CNULL == prompt)
      prompt = "fnord >";

   if (-1 != fd && isatty(fd)) {
      (void) printf("%s ", prompt);
      fflush(stdout);
   }
}

int
scan_open (file, sym)
   char *file;
   Sym  *sym;
{
   extern char *getenv();
   extern char *strchr();
   extern char *strtok();
   static char **dirs = (char **) -1;
   static char zero = 0;
   char *temp;
   char room[80];
   int i;
   int fd;
   int len;
   int max = 80;

   ME(scan_open);
   
   if ((char **) -1 == dirs)
   {
      char *envar = getenv("FNORD_PATH");

      for (i = 1, temp = envar; CNULL != temp; i++)
	 temp = strchr(temp+1, ':');
	       
      ALLOCNM(dirs, char *, i+1);
      if (CNULL != envar)
      {
	 ALLOCNM(temp, char, strlen (envar) + 1);
      }
      
      i = 0;
      dirs[i++] = &zero;
      if (CNULL == temp)
	 dirs[i] = temp;
      else 
      {
         strcpy (temp, envar);
	 for (dirs[i++] = strtok (temp, ":"); CNULL != dirs[i-1]; i++)
	    dirs[i] = strtok (CNULL,":");
      }
   }

   *sym = (Sym) NULL;
   fd = -1;
   for (i=0; CNULL != dirs[i] && -1 == fd; i++)
   {
      len = strlen(dirs[i]) + strlen(file);
      if (len > 71)
      {
	 if (80 < max) {
	    REALLOCNM (temp, char, len + 9, max);
	 } else {
	    ALLOCNM(temp, char, len + 9);
	 } max = len + 1;
      }
      else 
	temp = room;
      if (dirs[i][0] == 0)
	 strcpy(temp, file);
      else
	 (void) sprintf(temp, "%s/%s", dirs[i], file);
      fd = open (temp, O_RDONLY, 0);
      if (fd == -1)
      {
	 (void) strcat (temp, ".fnorse");
	 fd = open (temp, O_RDONLY, 0);
      }
   }

   *sym = sym_from_string(temp);
   
   if (80 < max)
      FREEN (temp, char, max);

   return fd;
}

static FOboolean
scan_include (name_in_quotes)
   char *name_in_quotes;
{
   extern int my_read();
   int fd;
   char *quote, *unquote;
   
   for (quote = name_in_quotes; *quote && *quote != '\"'; quote++);
   if (*quote)
   {
      for (unquote = ++quote; *unquote && *unquote != '\"'; unquote++);
      if (*unquote)
      {
	 Sym filesym;
	 *unquote = 0;
	 fd = scan_open (quote, &filesym);
	 if (fd > -1)
	 {
	    scan_context_push (filesym, fd, my_read, NO);
	    return SUCCESS;
	 }
	 else
	 {
	    perror (quote);
	    (void) fprintf(stderr, "File included from file %s:\n",
			   SYM_STRING(scan_current_context->file));
	 }
      }
      else
	 (void) fprintf(stderr, "Illegal #include in file %s at line %d.\n",
			SYM_STRING(scan_current_context->file),
			scan_current_context->lineno);
   }
   else
      (void) fprintf(stderr, "Illegal #include in file %s at line %d.\n",
		     SYM_STRING(scan_current_context->file),
		     scan_current_context->lineno);

   return FAILURE;
}

static void
scan_context_push(sym, fd, readfunc, hard)
   Sym sym;
   int fd;
   int (*readfunc)();
   FOboolean hard;

{
   struct scan_context *ctx;
   struct parse_file_list *temp;
   ME(scan_context_push);

   /* allocate a new context */
   ALLOCM(ctx, struct scan_context);
   ctx->lineno = 0;
   ctx->file = sym;
   ctx->fd = fd;
   ctx->isatty = isatty(fd);
   ctx->globals = NULL;
   ctx->read  = readfunc;
   ctx->hard = hard;

   /* allocate the buffer for the context */
   ctx->bsize = 80;			/* default buffer size */
   ALLOCNM(ctx->buf, char, ctx->bsize);
   ctx->bptr = ctx->bstop = ctx->bnext = ctx->buf;

   /* save state of scanner */
   if (NULL != scan_current_context && hard)
      scan_protect_globals();

   /* push the context onto the stack */
   ctx->last = scan_current_context;
   scan_current_context = ctx;

   /* update the files read list */
   ALLOCM (temp, struct parse_file_list);
   temp->file = scan_current_context->file;
   temp->next = parse_files_read;
   parse_files_read = temp;
}

/* fill the buffer as far as possible.  May switch the scan_context.
 * returns scan_bstop - scan_bptr, or 0 for end of file */
static int
fill_buffer()
{
   int cursize = scan_bnext - scan_bstop;
   char *oldbuf, *beol;
   int count;
   ME(fill_buffer);

   /* increment the line number.  This routine is called once per
    * line. */
   scan_current_context->lineno++;

   /* all the characters up to bptr have been consumed already, we can
    * throw them away.  Note that this copy may be overlapping */
   if (cursize > 0)
   {
      char *src = scan_bstop, *dest = scan_buf;

      /* could use bcopy here, but the length is probably
       * insignificant */
      while (cursize--)
	 *dest++ = *src++;

      scan_bnext = dest;
   }
   else
      scan_bnext = scan_buf;

   /* if we go directly into scan_read, we are in danger of blocking:
    * check to see if we have enough to act on first */

   /* if there is a '\n' in the input, act on it */
   /* otherwise, add more to the buffer */

   beol = scan_buf;
   while (beol < scan_bnext && *beol != '\n')
      beol++;

   while (beol >= scan_bnext)
   {
      /* if the buffer is full, enlarge it */
      if (scan_bnext >= scan_buf + scan_bsize)
      {
	 /* double the buffer size */
	 oldbuf = scan_buf;
	 if ((scan_buf = (char *) realloc(scan_buf,
					  (unsigned) scan_bsize * 2)) == 0)
	 {
	    fprintf(stderr,
		    "Out of memory. Input file \"%s\" line %d too long.\n",
		    SYM_STRING(scan_current_context->file),
		    scan_current_context->lineno);
	    exit(1);
	 }
	 scan_bsize *= 2;
	 scan_bnext += scan_buf - oldbuf;

	 beol += scan_buf - oldbuf;
      }

      /* read more into the buffer */
      count = (*scan_read)(scan_current_context->fd, scan_bnext,
			   scan_buf + scan_bsize - scan_bnext);

      if (count == 0)
	 /* set bstop to bnext, bptr to buf, and resume scan */
	 return (scan_bstop = scan_bnext) - (scan_bptr = scan_buf);

      else	/* increase scan_bnext */
	 scan_bnext += count;

      /* look for newlines in the new area */
      while (beol < scan_bnext && *beol != '\n')
	 beol++;
   }

   /* if this is a command, process it */
   /* possible commands: #include, #line, cpp # line information */
   if (*scan_buf == '#')
   {
      if (0 == strncmp (scan_buf + 1, "include", 7))
      {
	 scan_bptr = scan_bstop = beol + 1;
	 if (SUCCESS == scan_include (scan_buf + 8))
	    return fill_buffer();
      }
      else if (0 == strncmp (scan_buf + 1, "line", 4))
      {
	 int line = atoi (scan_buf + 5);
	 if (line > 0)
	    scan_current_context->lineno = line - 1;
      }
      else 
      {
	 (void) fprintf(stderr, "unknown cpp directive in file \"%s\" on line %d\n",
		SYM_STRING(scan_current_context->file),
		scan_current_context->lineno);
	 (void) fprintf(stderr, "ignored\n");
      }

      scan_bstop = beol + 1;
      return fill_buffer();
   }

   /* otherwise, scan as much as we can from the buffer */

   scan_bstop = ++beol;

   return scan_bstop - (scan_bptr = scan_buf);
}




/***********************************************************************
 *
 * Macros for use in scanner actions
 *
 ***********************************************************************/
/*
 * set the yylloc to tell the parser where this token is
 */
#define yylloc_set(type) (YYLVAL type.loc=scan_current_context	\
	? parse_new_filoc(SYM_STRING(scan_current_context->file),\
			scan_current_context->lineno,		  \
			scan_current_context->lineno)		   \
	: parse_new_filoc("bozo", -1, -1))
						
						 


/*
 (YYLLOC first_line=YYLLOC last_line \
		      =scan_current_context ? \
		       scan_current_context->lineno : -1)
 */

/*
 * use this to return tokens to the parser.
 * it sets up the range of the token before returning it.
 */
#define return_token(type, token) { yylloc_set(type); return (token); }

/*
 * if not returning any token, use this so location information will be
 * maintained.
 */
#define no_token()


/***********************************************************************
 *
 * Utility functions called by generated scanner
 *
 ***********************************************************************/
yywrap()
{
   struct scan_context *old_context;
   int rv = scan_current_context->hard;
   ME(yywrap);

   /* otherwise, we have an end of file.  Pop the input context
    * stack and free the current context */
   /* pop the stack */
   old_context = scan_current_context;
   scan_current_context = scan_current_context->last;
   
   /* free the buffer */
   FREEN(old_context->buf, char, old_context->bsize);
   
   /* close the file */
   /* should we check to see if it is stdin first? */
   if (old_context->fd != -1)
      close(old_context->fd);
   
   /* free the context */
   FREE(old_context, sizeof(struct scan_context));
   
   /* if the input context stack is now empty, return 0 */
   if (scan_current_context == NULL)
      return 1;
   
   /* otherwise, restore relevant information for the scanner
      and try to fill the new buffer */
   if (YES == rv)
   {
      scan_restore_globals();
   }
   return rv;
}

/*static void*/
yyerror(s)
   char *s;
{
   ME(yyerror);
/*
   fprintf(stderr, "error: %s\n", s);
   parse_err_cleanup();
 */
}



%}


any		(.|\n)
digit		[0-9]
sign		[-+]
sym		[A-Za-z][A-Za-z0-9]*
integer		{digit}+
scale		[Ee]{sign}?{integer}
realdot		{digit}*\.{digit}+({scale})?
realexp		{digit}+{scale}
real		{realdot}|{realexp}
white		[ \t\r\n\f]*

%%

","		return_token(token, TOKEN_COMMA);
"'"		return_token(token, TOKEN_POP);
"!"		return_token(token, TOKEN_BANG);
"@"		return_token(token, TOKEN_AT);
"#"		return_token(token, TOKEN_SHARP);
"$"		return_token(token, TOKEN_DOLLAR);
"^"		return_token(token, TOKEN_CARET);
"&"		return_token(token, TOKEN_AMPERSAND);
"_"		return_token(token, TOKEN_UNDERSCORE);
"|"		return_token(token, TOKEN_BAR);
"\\"		return_token(token, TOKEN_BACKSLASH);
"`"		return_token(token, TOKEN_BACKQUOTE);
"~"		return_token(token, TOKEN_TILDE);
":"		return_token(token, TOKEN_COLON);
";"		return_token(token, TOKEN_SEMICOLON);
":="		return_token(token, TOKEN_COLONEQUAL);
"?"		return_token(token, TOKEN_QUESTION);
"+"		return_token(token, TOKEN_PLUS);
"*"		return_token(token, TOKEN_STAR);
"/"		return_token(token, TOKEN_SLASH);
"."		return_token(token, TOKEN_DOT);
"-"		return_token(token, TOKEN_DASH);
"("		return_token(token, TOKEN_LPAREN);
")"		return_token(token, TOKEN_RPAREN);
"{"		return_token(token, TOKEN_LBRACE);
"}"		return_token(token, TOKEN_RBRACE);
"["		return_token(token, TOKEN_LBRACKET);
"]"		return_token(token, TOKEN_RBRACKET);
"<"		return_token(token, TOKEN_LANGLE);
">"		return_token(token, TOKEN_RANGLE);
"->"		return_token(token, TOKEN_ARROW);
"<-"		return_token(token, TOKEN_BACKARROW);
"><"		return_token(token, TOKEN_CROSS);
"**"		return_token(token, TOKEN_DOUBLESTAR);
"="		return_token(token, TOKEN_EQUAL);
"==="		return_token(token, TOKEN_EXACTEQ);
"!="		return_token(token, TOKEN_NEQUAL);
">="		return_token(token, TOKEN_GEQUAL);
"<="		return_token(token, TOKEN_LEQUAL);
".."		return_token(token, TOKEN_DOTDOT);
"::"		return_token(token, TOKEN_COLONCOLON);
"<<"		return_token(token, TOKEN_LESSLESS);
"<+"		return_token(token, TOKEN_LESSPLUS);

{sym}		{
		   Sym sym;
		   int i;
		   SCAN_HOOK;

		   sym = sym_from_string(yytext);

		   if (SUCCESS == METlist_hash_find_int(cmd_id, sym, &i))
		   {
		      return_token(token, i);
		   }
		   else
		   {
		      YYLVAL sym.sym = sym;
		      return_token(sym, TOKEN_SYM);
		   }
		}

{real}		{
		   SCAN_HOOK;
		   YYLVAL real.real = atof(yytext);
		   return_token(real, TOKEN_REAL);
		}

{integer}	{
		   SCAN_HOOK;
		   YYLVAL integer.integer = atoi(yytext);
		   return_token(integer, TOKEN_INTEGER);
		}

\"[^"]*\"	{
		   int len = strlen(yytext) - 2;
		   /* might want to use a sym instead */
		   /* make room for a copy of the string */
		   char *s;
		   SCAN_HOOK;

		   ALLOCNM(s, char, len + 1);

		   /* don't copy the quotes over */
		   strncpy(s, yytext + 1, len);
		   s[len] = '\0';

		   YYLVAL string.string = s;
		   return_token(string, TOKEN_STRING);
		}


"/*"		{
		   /*************************************************
		    *
		    *   Basically, this action enters a tiny
		    * hand-written lexical analyzer for nested
		    * comments, and does yylloc processing by hand.
		    * If we were sure that we would be using flex,
		    * we could do this nicely.  Unfortunately, we
		    * don't have that guarantee, so we have to
		    * perpetrate this little hack.
		    *
		    *   We modify yylloc by hand here.  Sorry.
		    *
		    *************************************************/
		   int depth = 1;	/* current comment depth */
		   int state = 0;	/* current scan state */
		   int c;		/* current scan character */
		   static int comment_fsa[5][3] = {
		    /* '/'  '*' other */
		      { 1,   2,   0 },
		      { 1,   3,   0 },
		      { 4,   2,   0 },
		      { 1,   2,   0 },
		      { 1,   2,   0 },
		   };
		   SCAN_HOOK;

		   /* set the initial value(s) in yylloc */
              /*   YYLLOC first_line = scan_current_context->lineno;*/

		   /* keep going until we are at depth 0 */
		   while (depth)
		   {
		      /* get a character */
		      c = input();

		      if (0 == c) {
			 printf("reached EOF in comment, skipping...\n");
			 if (yywrap())
			    return(0);
			 break;
		      }

		      /* do state transition */
		      switch (c)
		      {
		       case '/':
			 state = comment_fsa[state][0];
			 break;
		       case '*':
			 state = comment_fsa[state][1];
			 break;
		       case '\n':
		         /* scan_current_context->lineno++;*/
			 state = comment_fsa[state][2];
			 break;
		       default:
			 state = comment_fsa[state][2];
			 break;
		      }

		      /* based on current state, modify depth */
		      if (state == 3)
			 depth++;
		      else if (state == 4)
			 depth--;
		   }

		   /* set the final value(s) in yylloc */
              /*   YYLLOC last_line = scan_current_context->lineno;*/
		}

{white}		no_token();

{any}		return_token(token, TOKEN_BOZO);

