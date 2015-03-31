%{
%}
/*
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
 *	POSSIBLE TYPE DECLARATION
 *
 *************************************************************/
%union {
   int sym;
   int real;
   int integer;
   int string;
   int expr;
   int rel;
   int token;
   int win;
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
	TOKEN_BACKQUOTE TOKEN_TILDE TOKEN_COLON TOKEN_SEMICOLON
	TOKEN_QUESTION TOKEN_PLUS TOKEN_STAR TOKEN_SLASH TOKEN_DOT
	TOKEN_DASH TOKEN_LPAREN TOKEN_RPAREN TOKEN_LBRACE TOKEN_RBRACE
	TOKEN_LBRACKET TOKEN_RBRACKET TOKEN_LANGLE TOKEN_RANGLE
	TOKEN_ARROW TOKEN_CROSS TOKEN_DOUBLESTAR TOKEN_PARENSTAR
	TOKEN_ROOT TOKEN_DOTDOT TOKEN_MOD
	TOKEN_EQUAL TOKEN_EXACTEQ TOKEN_NEQUAL
	TOKEN_AND TOKEN_OR TOKEN_NOT
	TOKEN_IF TOKEN_ELSE
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
	TOKEN_CMD_INCLUDE TOKEN_CMD_WINDOW TOKEN_CMD_CAMERA
	TOKEN_CMD_SHOW TOKEN_CMD_EVAL TOKEN_CMD_PRINT


/*************************************************************
 *
 *	NONTERMINAL TYPE DECLARATIONS
 *
 *************************************************************/
%type <expr> atom
	     apply_rhs apply
	     extract extract_lhs extract_rhs
	     pow pow_rhs
	     term term_rhs
	     sum
	     bool_expr and_expr or_expr expr
	     expr_list expr_comma_list expr_colon_list map
%type <rel>  rel_chain
%type <none> stmts stmt defn
%type <win> sym_list

%start stmts

%%

stmts
	: /* empty */ {}
	| stmts stmt {}
	;

stmt
	: defn {}
	| command {}
	;

defn
	: TOKEN_SYM TOKEN_COLON map TOKEN_SEMICOLON {}
	| TOKEN_SYM TOKEN_COLON map TOKEN_COLON map TOKEN_SEMICOLON {}
	;

sym_list
	: TOKEN_SYM {}
	| TOKEN_SYM sym_list {}
	;
   
command
	: TOKEN_CMD_EVAL TOKEN_SYM TOKEN_SEMICOLON {}
	| TOKEN_CMD_PRINT TOKEN_SYM TOKEN_SEMICOLON {}
	| TOKEN_CMD_SHOW sym_list TOKEN_SEMICOLON {}
	;

atom
	: TOKEN_LBRACKET expr_list TOKEN_RBRACKET {}
	| TOKEN_LBRACKET expr_colon_list TOKEN_RBRACKET {}
	| TOKEN_LBRACE expr_list TOKEN_RBRACE {}
	| TOKEN_QUESTION {}
	| TOKEN_REAL {}
	| TOKEN_INTEGER {}
	;

sym
	: TOKEN_SYM {}
	;

papply
	: atom {}
	| sym TOKEN_LPAREN map TOKEN_RPAREN {}
	| TOKEN_LPAREN map TOKEN_RPAREN {}
	| sym {}
	;

extract_lhs
	: extract {}
	| extract_lhs TOKEN_BANG {}
	| extract_lhs TOKEN_POP {}
	;

extract_rhs
	: papply {}
	| TOKEN_DASH extract_rhs {}
	| TOKEN_PLUS extract_rhs {}
	| TOKEN_NOT extract_rhs {}
	;

extract
	: papply {}
	| extract_lhs TOKEN_UNDERSCORE extract_rhs {}
	| extract_lhs TOKEN_CARET extract_rhs {}
	;

pow_rhs
	: pow {}
	| TOKEN_DASH pow_rhs {}
	| TOKEN_PLUS pow_rhs {}
	| TOKEN_NOT pow_rhs {}
	;

pow
	: extract_lhs {}
	| extract_lhs TOKEN_DOUBLESTAR pow_rhs {}
	;

apply_rhs
	: pow {}
	| TOKEN_NOT apply_rhs {}
	;

apply
	: pow {}
	| apply apply_rhs {}
	;

term_rhs
	: apply {}
	| TOKEN_DASH term_rhs {}
	| TOKEN_PLUS term_rhs {}
	| TOKEN_NOT term_rhs {}
	;


term
	: term_rhs {}
	| term TOKEN_STAR term_rhs {}
	| term TOKEN_SLASH term_rhs {}
	| term TOKEN_DOT term_rhs {}
	| term TOKEN_CROSS term_rhs {}
	| term TOKEN_MOD term_rhs {}
	;

sum
	: term {}
	| sum TOKEN_PLUS term {}
	| sum TOKEN_DASH term {}
	;

rel_chain
	: sum {}
	| rel_chain TOKEN_EQUAL sum {}
	| rel_chain TOKEN_EXACTEQ sum {}
	| rel_chain TOKEN_NEQUAL sum {}
	| rel_chain TOKEN_LANGLE sum {}
	| rel_chain TOKEN_RANGLE sum {}
	| rel_chain TOKEN_LANGLE TOKEN_EQUAL sum {}
	| rel_chain TOKEN_RANGLE TOKEN_EQUAL sum {}
	;

bool_expr
	: rel_chain {}
	;

and_expr
	: bool_expr {}
	| and_expr TOKEN_AND bool_expr {}
	;

or_expr
	: and_expr {}
	| or_expr TOKEN_OR and_expr {}
	;

expr
	: or_expr {}
	| or_expr TOKEN_IF or_expr {}
	| or_expr TOKEN_IF or_expr TOKEN_ELSE expr {}
	;

/*
	| TOKEN_BAR expr TOKEN_BAR {}
*/

expr_comma_list
	: expr TOKEN_COMMA expr {}
	| expr_comma_list TOKEN_COMMA expr {}
	;

expr_colon_list
	: expr TOKEN_COLON expr {}
	| expr_colon_list TOKEN_COLON expr {}
	;

expr_list
	: expr {}
	| expr_comma_list {}
	;

map
	: map TOKEN_ARROW expr_list {}
	| expr_list {}
	;

%%

/* $Id: grammar.y,v 1.3 1992/06/22 17:07:54 mas Exp $ */
