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
# $Id: GNUmakefile,v 1.48 1992/06/22 17:02:40 mas Exp $

# set default mode and program
include config.make

# use this if no target is specified on the command line
default: program-bozo


# hardware architecture (eg sun4 or vax),
# os name (eg SunOS or BSD),
# and os version (eg OSV_4_1 or OSV_4_3)
arch := $(shell sysinfo -m)
os   := $(shell sysinfo -s)
osv  := OSV_$(shell sysinfo -r)
site := $(shell sysinfo -i)

clean_pwd := sed -e 's/\/tmp_mnt\//\//'
src_dir := $(shell pwd | $(clean_pwd))
gen_dir := ../lib/$(arch)-$(mode)

# switches for basic libraries
ifeq "$(site)" "brown"
  ldlibs := $(ldlibs) -L/cs/lib/gnu -lfl
endif
# !spot! help with the flex libraries here

x_cpp :=  -DX_TWO_D=1 -DX_THREE_D=1
ifeq "$(site)" "sgi"
  x_ld  := -lXt_s -lXmu -lX11_s
else
  x_ld  := -lXt -lXmu -lX11
endif

ifeq "$(site)" "brown"
  motif_cpp := -I/cs/include/motif -DMOTIF_WIDGETS=1 -D_NO_PROTO \
               -DMOTIF_1_1 $(x_cpp)
  motif_ld  := -L/cs/lib/motif -lXm $(x_ld)
endif

ifeq "$(site)" "sgi"
  cppflags  := -DFLOAT_TYPE=float
  motif_cpp := -DMOTIF_WIDGETS=1 -DMOTIF_1_1 $(x_cpp)
  motif_ld  := -lXm_s $(x_ld) -lPW
endif

ifeq "$(site)" "cmu"
  motif_cpp :=  -DMOTIF_WIDGETS=1 -D_NO_PROTO -DMOTIF_1_1 $(x_cpp)
  motif_ld  :=  -lXm $(x_ld)
endif

ifeq "$(site)" "andrew"
  motif_cpp := -I/usr/local/depot/motif/include -DMOTIF_WIDGETS=1 $(x_cpp)
  motif_ld  := -L/usr/local/depot/motif/lib -lXm \
                 /usr/local/depot/motif/lib/libXt.a -lXmu -lX11
endif

#########################################################################

# Don't complain about structure pointers to structures
# you haven't defined yet.

lintflags := -z

ifeq "$(arch)" "IBM6000"
  cppflags := -D_NO_PROTO $(cppflags)
endif

ifeq "$(arch)" "sun3"
  ccflags := -f68881 $(ccflags)
  ldflags := -f68881 $(ldflags)
endif

ifeq "$(arch)" "mips"
  ccflags := -float
endif

ifeq "$(os)" "IRIX"
 ifeq "$(osv)" "OSV_4_0"
  ccflags  := $(ccflags) -cckr
  cppflags := $(cppflags) -cckr
 endif
endif

ifeq "$(mode)" "debug"
 ccflags := $(ccflags) -g
 ifeq "$(os)" "SunOS"
  ldflags := $(ldflags) -n
  ldlibs := $(ldlibs) /usr/lib/debug/malloc.o /usr/lib/debug/mallocmap.o
 endif
else

ifeq  "$(mode)" "optimize"
 ccflags := $(ccflags) -g
 ifeq "$(os)" "SunOS"
  ldflags := $(ldflags) -n
  ldlibs := $(ldlibs) /usr/lib/debug/malloc.o /usr/lib/debug/mallocmap.o
 endif
# ldflags := $(ldflags) -s 
# ifeq "$(arch)" "SunOS"
#  ccflags := $(ccflags) -O4
# else
# ifeq "$(arch)" "IBM6000"
#  ccflags :=  $(ccflags) -O -Q
# else
#  ccflags := $(ccflags) -O
# endif
# endif
else

ifeq  "$(mode)" "profile"
 ifeq "$(arch)" "sun4"
  ifeq "$(site)" "brown"
   ldlibs  := $(ldlibs) -liprof
   ldflags := $(ldflags) -N -L/pro/lib
   ccflags := $(ccflags) -O4
  else
   ccflags := $(ccflags) -pg
   ldflags := $(ldflags) -pg
  endif
 else
  ccflags := $(ccflags) -pg
  ldflags := $(ldflags) -pg
 endif
endif
endif
endif

ifeq "$(os)" "IRIX"
 ldlibs := $(ldlibs) -lm -lc_s
else
 ldlibs := $(ldlibs) -lm 
endif

#########################################################################

# all packages
packages := global err filoc met ops parse port \
	    sym widgets object threedee \
	    main_fnord main_xfnord main_xmfnord main_implicit \
            sni_implicit sni_plain \
	    td_x td_motif

# a list of all valid modes
modes := debug optimize profile

# a list of all valid programs.  ttyfnord is the text-only version, 
# xfnord uses Xt, xmfnord uses Xm
programs := ttyfnord xfnord xmfnord implicit

defs_file := $(gen_dir)/Defs
include $(defs_file)

####

ttyfnord_pkgs := td_tty main_fnord global err filoc met ops parse \
	         port sym object sni_plain 
ttyfnord_libs :=
ttyfnord_cpp  := 
ttyfnord_objs := $(foreach pkg, $(ttyfnord_pkgs), \
	$(addprefix $(pkg)/,$($(pkg)_objs)))

####

implicit_pkgs := main_xmfnord global err filoc met ops parse \
	        port sym widgets object td_motif threedee sni_implicit
implicit_libs := $(motif_ld)
implicit_cpp  := $(motif_cpp)
implicit_objs := $(foreach pkg, $(implicit_pkgs), \
	$(addprefix $(pkg)/,$($(pkg)_objs)))

####

xfnord_pkgs := main_xfnord global err filoc met ops parse \
	       port sym widgets object td_x threedee sni_plain
xfnord_libs := $(x_ld)
xfnord_cpp  := $(x_cpp)
xfnord_objs := $(foreach pkg, $(xfnord_pkgs), \
	$(addprefix $(pkg)/,$($(pkg)_objs)))

####

xmfnord_pkgs := threedee td_motif main_xmfnord global err filoc met ops parse \
	        port sym widgets object sni_plain
xmfnord_libs := $(motif_ld)
xmfnord_cpp  := $(motif_cpp)
xmfnord_objs := $(foreach pkg, $(xmfnord_pkgs), \
	$(addprefix $(pkg)/,$($(pkg)_objs)))

###

include admin/Rules.make
