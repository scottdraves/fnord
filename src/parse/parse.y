%{
/*
 * Copyright 1990, 1991, 1992, Brown University, Providence, RI
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

/*************************************************************
 *
 *	parse.y : grammar description file for fnord
 *
 *************************************************************/

#ifndef lint
static char Version[] =
   "$Id: parse.y,v 1.45 1992/06/22 17:08:00 mas Exp $";
#endif



/*************************************************************
 *
 *	C header code
 *
 *************************************************************/

#include <sys/types.h>
#include "global/global.h"
#include "port/time.h"
#include "sym/sym.h"
#include "parse/parse.h"
#include "ops/ops.h"
#include "filoc/filoc.h"
#include "parse/scan.h"
#include "td/td.h"

/* bison needs alloca() */
#ifdef BISON
#include <alloca.h>
#endif /* def(BISON) */

#define RNULL ((FILOCloc *) NULL)
#define PARSE_NAME_OF_LAST "_fnord_last_expr"
#define PARSE_NEW_DEFINITION_FLAG (PARSE_ALL_FLAGS + 1)

/* if this is defined, the scanner knows that it has been #included
 * into the file rather than compiled separately */
#define	INSIDE_PARSER

/* enable debugging with yydebug */
#ifndef YYDEBUG
#define YYDEBUG	1
#endif /* undef YYDEBUG */

#ifdef DEBUG
extern void parse_hook();
#define PARSE_HOOK ME(parser); parse_hook()
#else
#define PARSE_HOOK ME(parser)
#endif

/* define the type to use to pass position information up the tree */
typedef FILOCloc yyltype;
#define YYLTYPE yyltype

YYLTYPE yylloc;

static METsym
   *parse_apply_op_0(),
   *parse_apply_op_1(),
   *parse_apply_op_2(),
   *parse_apply_op_3(),
   *parse_apply_pair();

static int parse_global_flags;

static FILOCpool *parse_filoc_pool;
static FILOCloc *parse_span_filocs();

static METsym **used_syms = NULL;
static int used_syms_size = 0;
static int used_syms_index = 0;

static void parse_new_metsym();
static void parse_free_metsyms();

static void parse_err_cleanup();
static void parse_reset();

static METset *parse_set_from_expr();

static void parse_sym_define();

static int func_id;

METenv *parse_root_environment;
static METenv *parse_perm_environment;
static Sym name_of_last;

/* to keep track of what files we've read in --
 * referenced in the scanner, so if we take the scanner
 * out of this file we'll have to change this def...
 */
static struct parse_file_list {
   Sym file;
   struct parse_file_list *next;
} *parse_files_kept = NULL, *parse_files_read = NULL;

extern void clean_up();

%}

%pure_parser

/*************************************************************
 *
 *	POSSIBLE TYPE DECLARATION
 *
 *************************************************************/
%union {
   struct {
      Sym sym;
      FILOCloc *loc;
   } sym;
   struct {
      double real;
      FILOCloc *loc;
   } real;
   struct {
      int integer;
      FILOCloc *loc;
   } integer;
   struct {
      char *string;
      FILOCloc *loc;
   } string;
   struct {
      METsym *expr;
      FILOCloc *loc;
   } expr;
   struct {
      METsym *expr;
      METsym *right;
      FILOCloc *loc;
   } rel;
   struct {
      FILOCloc *loc;
   } token;
   int none;
}

/*************************************************************
 *
 *	TERMINAL TYPE DECLARATIONS
 *
 *************************************************************/
%token <token>
	TOKEN_COMMA TOKEN_POP TOKEN_BANG
	TOKEN_AT TOKEN_SHARP TOKEN_DOLLAR TOKEN_PERCENT TOKEN_CARET
	TOKEN_AMPERSAND TOKEN_UNDERSCORE TOKEN_BAR TOKEN_BACKSLASH
	TOKEN_BACKQUOTE TOKEN_TILDE
   	TOKEN_LESSLESS TOKEN_LESSPLUS
   	TOKEN_COLON TOKEN_SEMICOLON TOKEN_COLONEQUAL
	TOKEN_QUESTION TOKEN_PLUS TOKEN_STAR TOKEN_SLASH TOKEN_DOT
	TOKEN_DASH TOKEN_LPAREN TOKEN_RPAREN TOKEN_LBRACE TOKEN_RBRACE
	TOKEN_LBRACKET TOKEN_RBRACKET TOKEN_LANGLE TOKEN_RANGLE
	TOKEN_ARROW TOKEN_BACKARROW TOKEN_CROSS TOKEN_DOUBLESTAR
	TOKEN_DOTDOT TOKEN_COLONCOLON TOKEN_MOD
	TOKEN_EQUAL TOKEN_EXACTEQ TOKEN_NEQUAL TOKEN_GEQUAL TOKEN_LEQUAL
	TOKEN_AND TOKEN_OR TOKEN_NOT TOKEN_UNION
	TOKEN_IF TOKEN_ELSE TOKEN_IN TOKEN_LET TOKEN_BE
	TOKEN_FIX TOKEN_CLEAR TOKEN_QUIT

%token <token>
	TOKEN_BOZO

%token <sym>
	TOKEN_SYM
%token <real>
	TOKEN_REAL
%token <integer>
	TOKEN_INTEGER
%token <string>
	TOKEN_STRING

%token <integer>
	TOKEN_BUILTIN

%token <token>
	TOKEN_CMD_INCLUDE TOKEN_CMD_REQUIRE
   	TOKEN_CMD_WINDOW TOKEN_CMD_CAMERA
	TOKEN_CMD_EVAL TOKEN_CMD_PRINT TOKEN_CMD_DEBUG
	TOKEN_CMD_WIDGET


/*************************************************************
 *
 *	NONTERMINAL TYPE DECLARATIONS
 *
 *************************************************************/
%type <expr> atom sym papply p_has_applied papply_lhs
	     paren_exp apply_rhs apply
	     extract extract_lhs extract_rhs
	     pow pow_rhs
	     term term_rhs
	     sum
	     bool_expr and_expr or_expr union_expr expr
	     list comma_list list_item cast map
%type <rel>  rel_chain
%type <none> stmts stmt defn

/*************************************************************
 *
 *	TERMINAL AND RULE PRECEDENCE DECLARATIONS
 *		are all gone!
 *
 *************************************************************/


%start stmts

%%

stmts
	: /* empty */
	{
	   PARSE_HOOK;
	}
	| stmts stmt
	{
	   PARSE_HOOK;
	}
	;

stmt
	: list TOKEN_SEMICOLON
	{ 
	   METset *set;
	   PARSE_HOOK;
		
	   if (!(PARSE_NEW_DEFINITION_FLAG & parse_global_flags))
	      parse_sym_define(name_of_last, $1.expr, NO);

	   parse_global_flags |= PARSE_NEW_DEFINITION_FLAG;

	   if (PARSE_PRINT_EXPR_FLAG & parse_global_flags)
	      if (MET_NULL_SET != (set = parse_set_from_expr($1.expr))) {
	         METset_print_pretty(stdout, set, CNULL, 0);
	         (void) fprintf(stdout, "\n");
	         MET_SET_FREE(set);
	      }

	   /* reset everything for the next stmt */
	   parse_reset();
	}
	| defn TOKEN_SEMICOLON
	{
	   PARSE_HOOK;

	   /* reset everything for the next stmt */
	   parse_reset();
	}
	| command TOKEN_SEMICOLON
	{
	   PARSE_HOOK;

	   /* reset everything for the next stmt */
	   parse_reset();
	}
	| error TOKEN_SEMICOLON
	{
	   PARSE_HOOK;

	   FILOCloc_print($2.loc, stderr, (char *) NULL, 0);
	   fprintf(stderr, ": bad statement detected\n");

	   parse_reset();
	}
	| error
	{
	   PARSE_HOOK;

	   fprintf(stderr, ": bad statement--semicolon missing?\n");
	   parse_reset();
	}
	;

defn
	: TOKEN_SYM TOKEN_COLONEQUAL list
	{
	   PARSE_HOOK;

	   if (PARSE_DEFINE_SYMBOLS_FLAG & parse_global_flags)
	      parse_sym_define($1.sym, $3.expr, YES);
	   else {
	      FILOCloc_print($3.loc, stderr, (char *)NULL, 0);
	      fprintf(stderr, "definitions not possible now\n");
	   }
	}
	| error TOKEN_COLONEQUAL list
	{
	   PARSE_HOOK;
	   FILOCloc_print($2.loc, stderr, (char *) NULL, 0);
	   fprintf(stderr, ": bad identifier detected?\n");
	}
	| TOKEN_SYM TOKEN_COLONEQUAL TOKEN_CMD_WIDGET list
	{
	   METset *set;
	   METsym *sym;
	   PARSE_HOOK;
	   
	   if (PARSE_USE_WIDGETS_FLAG & parse_global_flags) {
	      if (PARSE_DEFINE_SYMBOLS_FLAG & parse_global_flags) {
		 if (MET_NULL_SET != (set = parse_set_from_expr($4.expr))) {
		    sym = widget(SYM_STRING($1.sym), set,
				 parse_root_environment, $4.expr);
		    if (sym != MET_NULL_SYM) {
		       parse_sym_define($1.sym, sym, YES);
		       MET_SYM_FREE(sym);
		    }
		    MET_SET_FREE(set);
		 }
	      } else {
		 FILOCloc_print($3.loc, stderr, (char *)NULL, 0);
		 fprintf(stderr, "definitions not possible now\n");
	      }
	   } 
	   else {
	      FILOCloc_print($3.loc, stderr, (char *)NULL, 0);
	      fprintf(stderr, "widgets not available now\n");
	   }
	}
	| error TOKEN_COLONEQUAL TOKEN_CMD_WIDGET list
	{
	   PARSE_HOOK;
	   FILOCloc_print($2.loc, stderr, (char *)NULL, 0);
	   fprintf(stderr, ": bad identifier detected?\n");
	}
	;	

command
	: TOKEN_CMD_EVAL list
	{
	   METset *set;
	   PARSE_HOOK;

	   if (PARSE_EXEC_COMMANDS_FLAG & parse_global_flags &&
	       MET_NULL_SET != (set = parse_set_from_expr($2.expr))) {
	      MET_SET_FREE(set);
	   }

	   parse_reset();
	}
	| TOKEN_CMD_PRINT list
	{
	   METset *set;
	   PARSE_HOOK;

	   if (PARSE_EXEC_COMMANDS_FLAG & parse_global_flags &&
	       MET_NULL_SET != (set = parse_set_from_expr($2.expr))) {
	      METset_print_pretty(stdout, set, CNULL, 0);
	      (void) fprintf(stdout, "\n");
	      MET_SET_FREE(set);
	   }

	   /* clean up after ourselves */
	   parse_reset();
	}
	| TOKEN_CMD_DEBUG list
	{
	   METset *set;
	   PARSE_HOOK;

	   if (PARSE_EXEC_COMMANDS_FLAG & parse_global_flags &&
	       MET_NULL_SET != (set = parse_set_from_expr($2.expr))) {
	      METset_print(FNULL, set, 0);
	      MET_SET_FREE(set);
	   }

	   /* clean up after ourselves */
	   parse_reset();
	}
	| TOKEN_CMD_INCLUDE list
	{
	   METset *set;
	   char file[200];
	   int val = 200;
	   Sym sym;
	   extern int my_read();
	   PARSE_HOOK;

	   if (PARSE_EXEC_COMMANDS_FLAG & parse_global_flags) 
	      if (MET_NULL_SET != (set = parse_set_from_expr($2.expr)) &&
		  SUCCESS == METset_get_string(set, file, &val)) {
		 val = scan_open(file, &sym);
		 if (-1 == val) {
		    FILOCloc_print($2.loc, stderr, (char *) NULL, 0);
		    perror("can't include file");
		 } else {
		    (void) fnord_parse(sym, val, my_read,
				       PARSE_NO_FLAGS, PARSE_NO_FLAGS);
		 }
		 MET_SET_FREE(set);
	      } else {
		 MET_SET_FREE_NULL(set);
		 FILOCloc_print($2.loc, stderr, (char *) NULL, 0);
		 fprintf(stderr, "bad file specification\n");
	      }
	   else {
	      FILOCloc_print($2.loc, stderr, (char *) NULL, 0);
	      fprintf(stderr, "cannot include files now\n");
	   }
	   parse_reset();
	}
        | TOKEN_CMD_REQUIRE list
	{
	   METset *set;
	   char file[200];
	   int val = 200;
	   Sym sym;
	   struct parse_file_list *curr;
	   FOboolean found = NO;
	   extern int my_read();
	   PARSE_HOOK;

	   if (PARSE_EXEC_COMMANDS_FLAG & parse_global_flags)
	      if (MET_NULL_SET != (set = parse_set_from_expr($2.expr)) &&
		  SUCCESS == METset_get_string(set, file, &val)) {
		 val = scan_open(file, &sym);
		 if (-1 == val) {
		    FILOCloc_print($2.loc, stderr, (char *) NULL, 0);
		    perror("can't include file");
		 } else {
		    for (curr = parse_files_kept; curr != NULL; 
			 curr = curr->next) 
		       if (SYM_EQ(curr->file, sym)) {
			  found = YES;
			  break;
		       }
		    if (NO == found) {
		       for (curr = parse_files_read; curr != NULL;
			    curr = curr->next)
			  if (SYM_EQ(curr->file, sym)) {
			     found = YES;
			     break;
			  }
		       if (NO == found) 
			  (void) fnord_parse(sym, val, my_read,
					     PARSE_NO_FLAGS, PARSE_NO_FLAGS);
		    }
		 }
		 MET_SET_FREE(set);
	      } else {
		 MET_SET_FREE_NULL(set);
		 FILOCloc_print($2.loc, stderr, (char *) NULL, 0);
		 fprintf(stderr, "bad file specification\n");
	      }
	   else {
	      FILOCloc_print($2.loc, stderr, (char *) NULL, 0);
	      fprintf(stderr, "cannot include files now\n");
	   }

	   parse_reset();

	}
   	| TOKEN_CMD_WIDGET list
	{
	   METset *set;
	   METsym *sym;
	   PARSE_HOOK;

	   if (parse_global_flags & PARSE_USE_WIDGETS_FLAG &&
	       parse_global_flags & PARSE_EXEC_COMMANDS_FLAG) {
	      if (MET_NULL_SET != (set = parse_set_from_expr($2.expr))) {
		 sym = widget("default", set, parse_root_environment, $2.expr);
		 if (sym != MET_NULL_SYM) {
		    MET_SYM_FREE(sym);
		 } else {
		    printf("Unable to create widget.\n");
		 }
		 MET_SET_FREE(set);
	      }
	   } else {
	      FILOCloc_print($1.loc, stderr, (char *)NULL, 0);
	      fprintf(stderr, "widgets not available now\n");
	   }
	}
	| TOKEN_FIX
	{
	   PARSE_HOOK;
		
	   if (PARSE_EXEC_COMMANDS_FLAG & parse_global_flags)
	      parse_fix();
	   parse_reset();
	}
	| TOKEN_CLEAR
	{
	   PARSE_HOOK;
	   
	   if (PARSE_EXEC_COMMANDS_FLAG & parse_global_flags)
	      parse_clear();
	   parse_reset();
	}
	| TOKEN_QUIT
	{
	   PARSE_HOOK;

	   if (PARSE_EXEC_COMMANDS_FLAG & parse_global_flags) {
	      parse_reset();
	      clean_up();
	      (void) exit(0);
	   }
	   parse_reset();
	}
	;

atom
	: TOKEN_LBRACKET list TOKEN_RBRACKET
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_COLUMN_BUILD,
				      $2.expr, $$.loc);
	}

	| TOKEN_LBRACKET error TOKEN_RBRACKET
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = METsym_new_op(MET_SYM_OP_BAD_ARGS, $$.loc);
	   parse_new_metsym($$.expr);
	   FILOCloc_print($$.loc, stderr, (char *) NULL, 0);
	   fprintf(stderr, ": bad expression in [ ]\n");
	}

	| TOKEN_LBRACE TOKEN_RBRACE
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_0(MET_SYM_OP_SET_NULL, $$.loc);
	}

	| TOKEN_LBRACE list TOKEN_RBRACE
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_SET_OF, $2.expr, $$.loc);
	}

	| TOKEN_LBRACE error TOKEN_RBRACE
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = METsym_new_op(MET_SYM_OP_BAD_ARGS, $$.loc);
	   parse_new_metsym($$.expr);
	   FILOCloc_print($$.loc, stderr, (char *) NULL, 0);
	   fprintf(stderr, ": bad expression in { }\n");
	}

	| TOKEN_LBRACE list TOKEN_IN list TOKEN_BAR list TOKEN_RBRACE
	{
	   METset	*one;
	   METsym	*t1, *t2, *t3;
	   METnumber	num;
	   PARSE_HOOK;
	   /*
	    * { decl in set | expr } parses into
	    * implicit(set, (set -> (type 1=1) : decl -> expr))
	    */
	   $$.loc = parse_span_filocs($1.loc, $7.loc);
	   num.integer = 1;
	   one = METset_new_number(MET_FIELD_INTEGER, &num);
	   t1 = METsym_new_set_ref(one, $$.loc);
	   MET_SET_FREE(one);
	   parse_new_metsym(t1);
	   t2 = parse_apply_op_2(MET_SYM_OP_EXACT_EQUAL, t1, t1, $$.loc);
	   t1 = parse_apply_op_1(MET_SYM_OP_TYPE, t2, $$.loc);
	   t2 = parse_apply_op_2(MET_SYM_OP_MAP, $4.expr, t1, $$.loc);
	   t1 = parse_apply_op_2(MET_SYM_OP_MAP, $2.expr, $6.expr, $$.loc);
	   t3 = parse_apply_op_2(MET_SYM_OP_TYPE_AND_CAST, t2, t1, $$.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_IMPLICIT, $4.expr,
				      t3, $$.loc);
	}

	| TOKEN_QUESTION
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = METsym_new_op(MET_SYM_OP_TYPE_UNKNOWN, $$.loc);
	   parse_new_metsym($$.expr);
	}
	
	| TOKEN_BACKQUOTE TOKEN_SYM
	{
	   METset *set;
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   set = METsym_new_sym_const_set($2.sym);
	   $$.expr = METsym_new_set_ref(set, $$.loc);
	   MET_SET_FREE(set);
	}

	| TOKEN_STRING
	{
	   METset   *set;
	   PARSE_HOOK;

	   $$.loc = $1.loc;

	   set = METset_new_string($1.string);
	   $$.expr = METsym_new_set_ref(set, $$.loc);
	   parse_new_metsym($$.expr);
	   MET_SET_FREE(set);
	   /* free the string data allocated by the scanner */
	   FREE_STR($1.string);
	}

	| TOKEN_REAL
	{
	   METnumber num;
	   METset   *set;
	   PARSE_HOOK;

	   $$.loc = $1.loc;

	   num.real = $1.real;
	   set = METset_new_number(MET_FIELD_REAL, &num);
	   $$.expr = METsym_new_set_ref(set, $$.loc);
	   parse_new_metsym($$.expr);
	   MET_SET_FREE(set);
	}

	| TOKEN_INTEGER
	{
	   METnumber num;
	   METset   *set;
	   PARSE_HOOK;

	   $$.loc = $1.loc;

	   num.integer = $1.integer;
	   set = METset_new_number(MET_FIELD_INTEGER, &num);
	   $$.expr = METsym_new_set_ref(set, $$.loc);
	   parse_new_metsym($$.expr);
	   MET_SET_FREE(set);
	}
	;

sym
	: TOKEN_SYM
	{
	   int op_id;
	   PARSE_HOOK;

	   $$.loc = $1.loc;
	   /*
	    * if the symbol was in the table of standard functions, than
	    * generate an op node, otherwise generate a reference to
	    * that symbol
	    */
	   if (SUCCESS == METlist_hash_find_int(func_id, $1.sym, &op_id))
	      $$.expr = METsym_new_op(op_id, $$.loc);
	   else
	      $$.expr = METsym_new_sym($1.sym, $$.loc);

	   parse_new_metsym($$.expr);
	}
	;

papply_lhs
	: sym
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| papply_lhs TOKEN_BANG
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_FACTORIAL, $1.expr,
				      $$.loc);
	}
	| papply_lhs TOKEN_POP
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_PRIME, $1.expr,
				      $$.loc);
	}
	;

paren_exp
	: TOKEN_LPAREN list TOKEN_RPAREN
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = $2.expr;
	}
	| TOKEN_LPAREN error TOKEN_RPAREN
	{
	   PARSE_HOOK;
 	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = METsym_new_op(MET_SYM_OP_BAD_ARGS, $$.loc);
	   parse_new_metsym($$.expr);
           FILOCloc_print($$.loc, stderr, (char *) NULL, 0);
           fprintf(stderr, ": bad expression in ( )\n");
	}
	;

p_has_applied
	: papply_lhs 
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| p_has_applied paren_exp
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = METsym_new_op(MET_SYM_OP_APPLY, $$.loc);
	   parse_new_metsym($$.expr);
	   METsym_add_kid($$.expr, $1.expr);
	   METsym_add_kid($$.expr, $2.expr);
	}
	;
	
papply
	: atom
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| paren_exp
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| p_has_applied
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	;

extract_lhs
	: extract
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| extract_lhs TOKEN_BANG
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_FACTORIAL, $1.expr,
				      $$.loc);
	}
	| extract_lhs TOKEN_POP
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_PRIME, $1.expr,
				      $$.loc);
	}
	;


extract_rhs
	: papply
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| TOKEN_DASH extract_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_OPPOSITE, $2.expr, $$.loc);
	}
	| TOKEN_SHARP extract_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_CARDINALITY, $2.expr, $$.loc);
	}
	| TOKEN_PLUS extract_rhs
	{
	   PARSE_HOOK;
	   $$.loc = $2.loc;
	   $$.expr = $2.expr;
	}
	| TOKEN_NOT extract_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_NOT, $2.expr, $$.loc);
	}
	;

extract
	: papply
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}

	| extract_lhs TOKEN_UNDERSCORE extract_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_ROW_EXTRACT, $1.expr,
				      $3.expr, $$.loc);
	}

	| extract_lhs TOKEN_CARET extract_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_COLUMN_EXTRACT,
				      $1.expr, $3.expr, $$.loc);
	}
	;

pow_rhs
	: pow
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| TOKEN_DASH pow_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_OPPOSITE, $2.expr, $$.loc);
	}
	| TOKEN_SHARP pow_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_CARDINALITY, $2.expr, $$.loc);
	}
	| TOKEN_PLUS pow_rhs
	{
	   PARSE_HOOK;
	   $$.loc = $2.loc;
	   $$.expr = $2.expr;
	}
	| TOKEN_NOT pow_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_NOT, $2.expr, $$.loc);
	}
	;

pow
	: extract_lhs
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}

	| extract_lhs TOKEN_DOUBLESTAR pow_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_POW, $1.expr,
				      $3.expr, $$.loc);
	}
	;

apply_rhs
	: pow
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| TOKEN_NOT apply_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_NOT, $2.expr, $$.loc);
	}
	;

apply
	: pow
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| apply apply_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = METsym_new_op(MET_SYM_OP_APPLY, $$.loc);
	   parse_new_metsym($$.expr);
	   METsym_add_kid($$.expr, $1.expr);
	   METsym_add_kid($$.expr, $2.expr);
	}
	| apply TOKEN_LESSLESS apply_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_DOUBLE_MAP, $1.expr,
				      $3.expr, $$.loc);
	}
	| apply TOKEN_LESSPLUS apply_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_OUT_PROD_MAP, $1.expr,
				      $3.expr, $$.loc);
	}
	;

term_rhs
	: apply
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| TOKEN_DASH term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_OPPOSITE, $2.expr, $$.loc);
	}
	| TOKEN_SHARP term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_CARDINALITY, $2.expr, $$.loc);
	}
	| TOKEN_PLUS term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = $2.loc;
	   $$.expr = $2.expr;
	}
	| TOKEN_NOT term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $2.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_NOT, $2.expr, $$.loc);
	}
	;


term
	: term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| term TOKEN_STAR term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_MULTIPLY, $1.expr,
				      $3.expr, $$.loc);
	}
	| term TOKEN_SLASH term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_DIVIDE, $1.expr,
				      $3.expr, $$.loc);
	}
	| term TOKEN_DOT term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_DOT, $1.expr,
				      $3.expr, $$.loc);
	}
	| term TOKEN_CROSS term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_CROSS, $1.expr,
				      $3.expr, $$.loc);
	}
	| term TOKEN_MOD term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_MODULO, $1.expr,
				      $3.expr, $$.loc);
	}
	| term TOKEN_COLONCOLON term_rhs
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_CONCATENATE, $1.expr,
				      $3.expr, $$.loc);
	}
	;

sum
	: term
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| sum TOKEN_PLUS term
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_ADD, $1.expr,
				      $3.expr, $$.loc);
	}
	| sum TOKEN_DASH term
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_SUBTRACT, $1.expr,
				      $3.expr, $$.loc);
	}
	;

rel_chain
	: sum
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = MET_NULL_SYM;
	   $$.right = $1.expr;
	}
	| rel_chain TOKEN_EQUAL sum
	{
	   METsym *rel;
	   PARSE_HOOK;

	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   rel = parse_apply_op_2(MET_SYM_OP_EPSILON_EQUAL,
				  $1.right, $3.expr,
				  parse_span_filocs($2.loc, $3.loc));
	   if ($1.expr != MET_NULL_SYM)
	      $$.expr = parse_apply_op_2(MET_SYM_OP_AND, $1.expr,
					 rel, $$.loc);
	   else
	      $$.expr = rel;

	   $$.right = $3.expr;
	}
	| rel_chain TOKEN_EXACTEQ sum
	{
	   METsym *rel;
	   PARSE_HOOK;

	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   rel = parse_apply_op_2(MET_SYM_OP_EXACT_EQUAL,
				  $1.right, $3.expr,
				  parse_span_filocs($2.loc, $3.loc));
	   if ($1.expr != MET_NULL_SYM)
	      $$.expr = parse_apply_op_2(MET_SYM_OP_AND, $1.expr,
					 rel, $$.loc);
	   else
	      $$.expr = rel;

	   $$.right = $3.expr;
	}
	| rel_chain TOKEN_NEQUAL sum
	{
	   METsym *rel;
	   PARSE_HOOK;

	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   rel = parse_apply_op_2(MET_SYM_OP_NOTEQUAL,
				  $1.right, $3.expr,
				  parse_span_filocs($2.loc, $3.loc));
	   if ($1.expr != MET_NULL_SYM)
	      $$.expr = parse_apply_op_2(MET_SYM_OP_AND, $1.expr,
					 rel, $$.loc);
	   else
	      $$.expr = rel;

	   $$.right = $3.expr;
	}
	| rel_chain TOKEN_LANGLE sum
	{
	   METsym *rel;
	   PARSE_HOOK;

	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   rel = parse_apply_op_2(MET_SYM_OP_LESS,
				  $1.right, $3.expr,
				  parse_span_filocs($2.loc, $3.loc));
	   if ($1.expr != MET_NULL_SYM)
	      $$.expr = parse_apply_op_2(MET_SYM_OP_AND, $1.expr,
					 rel, $$.loc);
	   else
	      $$.expr = rel;

	   $$.right = $3.expr;
	}
	| rel_chain TOKEN_RANGLE sum
	{
	   METsym *rel;
	   PARSE_HOOK;

	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   rel = parse_apply_op_2(MET_SYM_OP_GREATER,
				  $1.right, $3.expr,
				  parse_span_filocs($2.loc, $3.loc));
	   if ($1.expr != MET_NULL_SYM)
	      $$.expr = parse_apply_op_2(MET_SYM_OP_AND, $1.expr,
					 rel, $$.loc);
	   else
	      $$.expr = rel;

	   $$.right = $3.expr;
	}
	| rel_chain TOKEN_LEQUAL sum
	{
	   METsym *rel;
	   PARSE_HOOK;

	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   rel = parse_apply_op_2(MET_SYM_OP_LESSOREQUAL,
				  $1.right, $3.expr,
				  parse_span_filocs($2.loc, $3.loc));
	   if ($1.expr != MET_NULL_SYM)
	      $$.expr = parse_apply_op_2(MET_SYM_OP_AND, $1.expr,
					 rel, $$.loc);
	   else
	      $$.expr = rel;

	   $$.right = $3.expr;
	}
	| rel_chain TOKEN_GEQUAL sum
	{
	   METsym *rel;
	   PARSE_HOOK;

	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   rel = parse_apply_op_2(MET_SYM_OP_GREATEROREQUAL,
				  $1.right, $3.expr,
				  parse_span_filocs($2.loc, $3.loc));
	   if ($1.expr != MET_NULL_SYM)
	      $$.expr = parse_apply_op_2(MET_SYM_OP_AND, $1.expr,
					 rel, $$.loc);
	   else
	      $$.expr = rel;

	   $$.right = $3.expr;
	}
	;

bool_expr
	: rel_chain
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   if ($1.expr == MET_NULL_SYM)
	      $$.expr = $1.right;
	   else
	      $$.expr = $1.expr;
	}
	;

and_expr
	: bool_expr
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| and_expr TOKEN_AND bool_expr
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_AND, $1.expr,
				      $3.expr, $$.loc);
	}
	;

or_expr
	: and_expr
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| or_expr TOKEN_OR and_expr
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_OR, $1.expr,
				      $3.expr, $$.loc);
	}
	;

union_expr 
	: or_expr
	{
	   PARSE_HOOK;
   	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| union_expr TOKEN_UNION or_expr
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_UNION, $1.expr,
				      $3.expr, $$.loc);
	}
	;

expr
	: union_expr
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	| union_expr TOKEN_IF union_expr
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_IF, $1.expr,
				      $3.expr, $$.loc);
	}
	| union_expr TOKEN_IF union_expr TOKEN_ELSE expr
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_3(MET_SYM_OP_IF_ELSE, $1.expr,
				      $3.expr, $5.expr, $$.loc);
	}
	| union_expr TOKEN_DOTDOT union_expr
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_ENUMERATE, $1.expr,
				      $3.expr, $$.loc);
	}
	| TOKEN_LET expr TOKEN_BE expr TOKEN_IN map
	{
	   METsym	*t1;
	   PARSE_HOOK;
	   /*
	    * let decl be expr1 in expr2   parses into
	    * ((decl -> expr2)(expr1))
	    */

	   $$.loc = parse_span_filocs($1.loc, $6.loc);
	   t1 = parse_apply_op_2(MET_SYM_OP_MAP, $2.expr, $6.expr,
				 $$.loc);
	   $$.expr = METsym_new_op(MET_SYM_OP_APPLY, $$.loc);
	   parse_new_metsym($$.expr);
	   METsym_add_kid($$.expr, t1);
	   METsym_add_kid($$.expr, $4.expr);
	}
/*
	| TOKEN_BAR expr TOKEN_BAR
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_1(MET_SYM_OP_BARS, $2.expr, $$.loc);
	}
*/
	;

map
	: expr TOKEN_ARROW map
	{
	   PARSE_HOOK;

	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_MAP, $1.expr, $3.expr,
				      $$.loc);
	}
	| expr
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	;

cast
	: cast TOKEN_COLON map
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_op_2(MET_SYM_OP_TYPE_AND_CAST,
                                      $1.expr, $3.expr, $$.loc);
	}
	| map 
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	;

list_item
	: TOKEN_SYM TOKEN_BACKARROW list_item
	{
	   METsym *sym;
	   METsym *temp;
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   
	   temp = METsym_new_sym($1.sym, $$.loc);
	   parse_new_metsym(temp);
	   sym = parse_apply_op_1(MET_SYM_OP_QUOTE, temp, $$.loc);
	   $$.expr = parse_apply_pair(sym, $3.expr, $$.loc);
	}
	| cast
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	;
	   
comma_list
	: list_item TOKEN_COMMA list_item
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_pair($1.expr, $3.expr, $$.loc);
	}

	| comma_list TOKEN_COMMA list_item
	{
	   PARSE_HOOK;
	   $$.loc = parse_span_filocs($1.loc, $3.loc);
	   $$.expr = parse_apply_pair($1.expr, $3.expr, $$.loc);
	}
	;

list
	: list_item
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	
	| comma_list
	{
	   PARSE_HOOK;
	   $$.loc = $1.loc;
	   $$.expr = $1.expr;
	}
	;

%%

void
parse_init(flags)
   int flags;
{
   ME(parse_init);

   parse_global_flags = flags;
   
   sym_init(601);

   scan_init();
   func_init();

   parse_filoc_pool = FILOCpool_new();

   parse_root_environment = METenv_new(MET_ENV_TYPE_BIG, MET_NULL_ENV);
   MET_ENV_COPY(parse_perm_environment, parse_root_environment);
   name_of_last = sym_from_string (PARSE_NAME_OF_LAST);

}

void
parse_exit()
{
   struct parse_file_list *next, *temp;
   ME(parse_exit);

   parse_reset();
   MET_ENV_FREE(parse_root_environment);
   MET_ENV_FREE(parse_perm_environment);

   temp = parse_files_read;
   while (NULL != temp) {
      next = temp->next;
      FREE(temp, sizeof(struct parse_file_list));
      temp = next;
   }
   temp = parse_files_kept;
   while (NULL != temp) {
      next = temp->next;
      FREE(temp, sizeof(struct parse_file_list));
      temp = next;
   }
   
   METlist_hash_free_id(func_id);

   if ((METsym **) NULL != used_syms) {
      FREEN(used_syms, METsym*, used_syms_size);
   }

   scan_exit();
   FILOCpool_free(parse_filoc_pool);
}

void 
parse_fix()
{
   if (parse_root_environment == parse_perm_environment) {
      MET_ENV_FREE( parse_root_environment );
      parse_root_environment = METenv_new( MET_ENV_TYPE_BIG,
					  parse_perm_environment);
      fix_widgets();
   } else {
      fprintf(stderr, "Error : cannot fix more than once.\n");
   }
   parse_files_kept = parse_files_read;
   parse_files_read = (struct parse_file_list *) NULL;
}

void 
parse_clear()
{
   struct parse_file_list *temp, *next;
   ME(parse_clear);

   temp = parse_files_read;
   while (NULL != temp) {
      next = temp->next;
      FREE(temp, sizeof(struct parse_file_list));
      temp = next;
   }
   parse_files_read = NULL;

   if (parse_root_environment == parse_perm_environment) {	
      MET_ENV_FREE(parse_perm_environment);
      MET_ENV_FREE(parse_root_environment);
      parse_root_environment = METenv_new(MET_ENV_TYPE_BIG,
					  MET_NULL_ENV);
      MET_ENV_COPY(parse_perm_environment,
		   parse_root_environment);
   } else {
      MET_ENV_FREE( parse_root_environment );
      parse_root_environment = METenv_new( MET_ENV_TYPE_BIG,
					  parse_perm_environment );
   }
   destroy_new_widgets();
}

static
FILOCloc *
parse_span_filocs(first, last)
   FILOCloc *first, *last;
{
   FILOCloc *loc;
   ME(parse_span_filocs);

   if (FILOCloc_file_begin(first) == FILOCloc_file_begin(last)) {
      loc = FILOCloc_new(parse_filoc_pool, FILOCloc_file_begin(first),
			 FILOCloc_line_begin(first),
			 FILOCloc_col_begin(first),
			 FILOCloc_line_end(last),
			 FILOCloc_col_end(last));
   }

   /* otherwise, what do we do?  best would be create a union
    * containing the two files, but how do we figure out the last line
    * of the first file and the first line of the last file? */

   return loc;
}

FILOCloc *
parse_new_filoc(file, first_line, last_line)
   char *file;
   int first_line, last_line;
{
   FILOCloc *loc;
   ME(parse_new_filoc);

   loc = FILOCloc_new(parse_filoc_pool, file,
		      first_line, 0, last_line, 0);

   return loc;
}

static METsym *
parse_apply_op_0(op, loc)
   int op;
   FILOCloc *loc;
{
   METsym *sym;
   ME(parse_apply_op_0);

   sym = METsym_new_op(op, loc);
   parse_new_metsym(sym);

   return sym;
}


static METsym *
parse_apply_op_1(op, kid, loc)
   int op;
   METsym *kid;
   FILOCloc *loc;
{
   METsym *apply, *opsym;
   ME(parse_apply_op_1);

   apply = METsym_new_op(MET_SYM_OP_APPLY, loc);
   parse_new_metsym(apply);

   opsym = METsym_new_op(op, loc);
   parse_new_metsym(opsym);
   METsym_add_kid(apply,opsym);

   METsym_add_kid(apply, kid);

   return apply;
}


static METsym *
parse_apply_op_2(op, kid1, kid2, loc)
   int op;
   METsym *kid1, *kid2;
   FILOCloc *loc;
{
   METsym *apply, *opsym, *pair;
   ME(parse_apply_op_2);

   apply = METsym_new_op(MET_SYM_OP_APPLY, loc);
   parse_new_metsym(apply);

   opsym = METsym_new_op(op, loc);
   METsym_add_kid(apply, opsym);
   MET_SYM_FREE(opsym);

   pair = parse_apply_pair(kid1, kid2, loc);
   METsym_add_kid(apply, pair);

   return apply;
}

static METsym *
parse_apply_op_3(op, kid1, kid2, kid3, loc)
   int op;
   METsym *kid1, *kid2, *kid3;
   FILOCloc *loc;
{
   METsym *apply, *opsym, *pair1, *pair2;
   ME(parse_apply_op_3);

   apply = METsym_new_op(MET_SYM_OP_APPLY, loc);
   parse_new_metsym(apply);

   opsym = METsym_new_op(op, loc);
   METsym_add_kid(apply, opsym);
   MET_SYM_FREE(opsym);

   pair1 = parse_apply_pair(kid1, kid2, loc);
   pair2 = parse_apply_pair(pair1, kid3, loc);
/*   MET_SYM_FREE(pair1);*/
   METsym_add_kid(apply, pair2);

   return apply;
}

static METsym *
parse_apply_pair(kid1, kid2, loc)
   METsym *kid1, *kid2;
   FILOCloc *loc;
{
   METsym *apply, *pair;
   ME(parse_apply_pair);

   apply = METsym_new_op(MET_SYM_OP_APPLY, loc);
   parse_new_metsym(apply);

   pair = METsym_new_op(MET_SYM_OP_PAIR_BUILD, loc);
   METsym_add_kid(apply, pair);
   MET_SYM_FREE(pair);

   METsym_add_kid(apply, kid1);

   METsym_add_kid(apply, kid2);

   return apply;
}

static void
parse_new_metsym(metsym)
   METsym *metsym;
{
   ME(parse_new_metsym);

   if (used_syms_index >= used_syms_size) {
      if (used_syms_size == 0)
	 used_syms_size = 64;
      else
	 used_syms_size *= 2;

      if (used_syms == NULL) {
	 ALLOCNM(used_syms, METsym*, used_syms_size);
      } else {
	 REALLOCNM(used_syms, METsym*, used_syms_size,
		   used_syms_size / 2);
      }
   }

   used_syms[used_syms_index++] = metsym;
}

static void
parse_free_metsyms()
{
   ME(parse_free_metsyms);

   while (used_syms_index) {
      used_syms_index--;
      MET_SYM_FREE(used_syms[used_syms_index]);
   }
}

static void
parse_err_cleanup()
{
   ME(parse_err_cleanup);

   parse_free_metsyms();
}


/* This routine resets the state of the parser, freeing all MET stuff
 *  which was on the expression stack and resetting the variable index
 *  to its initial value.  It should be called after any production
 *  that has an expr in it but does not pass the expr up, including
 *  definitions, commands, and properties that take expr values. */
static void
parse_reset()
{
   ME(parse_reset);

   /* free outstanding symbols */
   parse_free_metsyms();
}

static METset *
parse_set_lookup(sym)
   Sym sym;
{
   METset *set;
   FOboolean t;
   ME(parse_set_lookup);

   if (NULL != (set = METenv_lookup(parse_root_environment, sym))) {
      time_begin();
      t = METset_have_rep(set, (FOboolean *) NULL);
      time_end(FNULL, "set");
      if (0 == t)
	 printf("\nfailed * *  * *  * *  * *  * *  * *  * *\n\n");
      else
	 return set;
   } else {
      printf("Unknown set \"%s\"\n", SYM_STRING(sym));
      return NULL;
   }

   return set;
}

METset *
parse_last_expr()
{
   int flag = PARSE_NEW_DEFINITION_FLAG & parse_global_flags;
   ME(parse_last_expr);

   parse_global_flags &= ~PARSE_NEW_DEFINITION_FLAG;
   if (flag)
      return parse_set_lookup (name_of_last);
   else
      return (METset *) NULL;
}

static METset *
parse_set_from_expr(expr)
   METsym *expr;
{
   METset *set;
   FOboolean t;
   ME(parse_set_from_expr);

   set = METset_new_with_definition(expr, parse_root_environment);

   time_begin();
   t = METset_have_rep(set, (FOboolean *) NULL);
   time_end(FNULL, "set");

   if (0 == t) {
      printf("\nfailed * *  * *  * *  * *  * *  * *  * *\n\n");
      MET_SET_FREE(set);
      return MET_NULL_SET;
   } else {
      return set;
   }
}


static void
parse_sym_define(sym, metsym, warn_if_redef)
   Sym sym;
   METsym *metsym;
   FOboolean warn_if_redef;
{
   METset *set;
   ME(parse_sym_define);
   
   set = METset_new_with_definition(metsym, parse_root_environment);
   
   if (YES == METenv_add_binding(parse_root_environment, sym, set, YES)) {
      if (YES == warn_if_redef)
	 fprintf(stderr, "Warning: redeclaration of set \"%s\"\n",
		 SYM_STRING(sym));
   }
   MET_SET_FREE(set);
}

/*
 * this is called by the PARSE_HOOK macro that is placed at the beginning
 * of the code for each rule.  it is a good place to stop when debugging.
 */
static void
parse_hook()
{
   ME(parse_hook);
   
   /* do nothing */
}

struct func_entry {
   char *name;
   int op_id;
} func_table[] = {
   { "sin",		MET_SYM_OP_SIN },
   { "cos",		MET_SYM_OP_COS },
   { "tan",		MET_SYM_OP_TAN },
   { "asin",		MET_SYM_OP_ASIN },
   { "acos",		MET_SYM_OP_ACOS },
   { "atan",		MET_SYM_OP_ATAN },
   { "sinh",		MET_SYM_OP_SINH },
   { "cosh",		MET_SYM_OP_COSH },
   { "tanh",		MET_SYM_OP_TANH },
   { "asinh",		MET_SYM_OP_ASINH },
   { "acosh",		MET_SYM_OP_ACOSH },
   { "atanh",		MET_SYM_OP_ATANH },
   { "ln",		MET_SYM_OP_LN },
   { "square",		MET_SYM_OP_SQUARE },
   { "sqrt",		MET_SYM_OP_SQRT },
   { "exp",		MET_SYM_OP_EXP },
   { "interval",	MET_SYM_OP_INTERVAL },
   { "complex",		MET_SYM_OP_COMPLEX_BUILD },
   { "left",		MET_SYM_OP_PAIR_LEFT },
   { "right",		MET_SYM_OP_PAIR_RIGHT },
   { "binomial",	MET_SYM_OP_CHOOSE },
   { "bitand",		MET_SYM_OP_BITAND },
   { "bitor",		MET_SYM_OP_BITOR },
   { "bitxor",		MET_SYM_OP_BITXOR },
   { "bitnot",		MET_SYM_OP_BITNOT },
   { "fragment",	MET_SYM_OP_VECTOR_TO_PAIRS },
   { "nop",		MET_SYM_OP_NOP },
   { "spew",		MET_SYM_OP_PRINT },
   { "sample",		MET_SYM_OP_SAMPLE },
   { "cxfrag",		MET_SYM_OP_COMPLEX_FRAG },
   { "polynomial",	MET_SYM_OP_POLYNOMIAL },
   { "type",		MET_SYM_OP_TYPE },
   { "triangle",	MET_SYM_OP_TRIANGLE },
   { "assoc",		MET_SYM_OP_ASSOC },
   { "realtoint",	MET_SYM_OP_REAL_TO_INTEGER },
   { "typematch",	MET_SYM_OP_TYPE_MATCH },
   { "typematchx",	MET_SYM_OP_TYPE_MATCH_X },
   { "localeffect",	MET_SYM_OP_LOCAL_EFFECT },
   { "zero",		MET_SYM_OP_ZERO },
   { "identity",	MET_SYM_OP_IDENTITY },
   { "reduce",		MET_SYM_OP_REDUCE },
   { "min",		MET_SYM_OP_MIN },
   { "max",		MET_SYM_OP_MAX },
   { "implicit",	MET_SYM_OP_IMPLICIT },
   { "shift",		MET_SYM_OP_SHIFT },
   { "trace",		MET_SYM_OP_TRACE_ON },
   { "untrace",         MET_SYM_OP_TRACE_OFF },
};

#define NFUNCS	(sizeof (func_table) / sizeof (struct func_entry))

/* add the builtin functions from the table into the func_symtab */
func_init()
{
   int i;
   ME(func_init);

   func_id = METlist_hash_new_id();

   for (i = 0; i < NFUNCS; i++) {
      METlist_hash_add_int(func_id, sym_from_string(func_table[i].name),
			   func_table[i].op_id);
   }
}


/* include the scanner.  This is the gross but easy way of doing
 * things */
#include "scan.c"
#include "parse/scan_dirty.c"
