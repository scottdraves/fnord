#
# Copyright 1990, 1991, 1992, Brown University, Providence, RI
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# $Id: Echo-Help.make,v 1.7 1993/06/03 14:37:25 rch Exp $

echo:
	@echo "options"
	@echo "======="
	@echo "program    = $(program)"
	@echo "mode       = $(mode)"
	@echo "echo       = $(echo)"
	@echo "subset     = $(subset)"
	@echo ""
	@echo "directories"
	@echo "==========="
	@echo "gen_dir    = $(gen_dir)"
	@echo "src_dir    = $(src_dir)"
	@echo ""
	@echo "commands"
	@echo "========"
	@echo "cc	  = $(cc)"
	@echo "ld	  = $(ld)"
	@echo "yacc	  = $(yacc)"
	@echo "lex	  = $(lex)"
	@echo "dep	  = $(dep)"
	@echo "picc	  = $(picc)"
	@echo "icc	  = $(icc)"
	@echo "lint1st	  = $(lint_first)"
	@echo "lint2nd	  = $(lint_second)"
	@echo ""
	@echo "files"
	@echo "====="
	@echo "objs       = $(objs)"
	@echo "libs       = $(libs)"
	@echo "cfiles     = $(cfiles)"
	@echo "ldlibs     = $(ldlibs)"

help:
	@echo ""
	@echo "usage: gnumake -rk target option=value ..."
	@echo ""
	@echo "target       meaning"
	@echo "======       ======="
	@echo "all          all executables (me*)"
	@echo "program      an executable (mpe)"
	@echo "depend       update dependencies (mpe)"
	@echo "lint         source for an executable (mpe)"
	@echo "lint_subset  lint specified files (mes*)"
	@echo "clean        remove all generated files (m)"
	@echo "wc           counts lines of all source (*)"
	@echo "tags         gnuemacs tags file (*)"
	@echo "tar          make a distribution"
	@echo "static       make a distribution of static exec's + src/fnorse"
	@echo "patch        make incremental distribution"
	@echo "patch-list   list what would do if patch were made"
	@echo "run          build and run the program (mp)"
	@echo "config       print $$(mode) and $$(program) (mp)"
	@echo "echo         print many variable values (mpes)"
	@echo "help         print this message"
	@echo ""
	@echo "option       value"
	@echo "======       ====="
	@echo "mode         debug, optimize, profile, etc."
	@echo "program      the executable to work with"
	@echo "echo         doesn't matter, shows build commands"
	@echo "subset       files for lint_subset"
	@echo "cppsrc       doesn't matter, compiles .i files"
	@echo ""
	@echo "The options used by each target are identified by"
	@echo "their first letters in ()s.  A * means that that"
	@echo "target is unimplemented.  Options take default"
	@echo "values from config.make.  The default target is"
	@echo "program."
