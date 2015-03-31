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
######################################################################
# $Id: Rules.make,v 1.28 1993/06/03 14:37:24 rch Exp $
#
# rules for building files
#
# before this file is included, the following variables must be defined:
# program mode gen_dir src_dir $(program)_objs $(program)_libs


################################################################
#		Definitions
################################################################


cfiles := $($(program)_objs:.o=.c)
objs   := $(addprefix $(gen_dir)/,$($(program)_objs))
libs   := $($(program)_libs)

ifeq "$(os)" "BSD"
  upcase := tr a-z A-Z
else
  upcase := tr [a-z] [A-Z]
endif

cppflags := $(cppflags) $($(program)_cpp) -I$(src_dir) \
            -D$(shell echo $(mode) | $(upcase)) \
            -DPROGRAM_$(shell echo $(program) | $(upcase)) \
            -DSITE_$(shell echo $(site) | $(upcase)) \
	    -D$(os) -D$(osv) -D$(arch)

lint_first_pass_flag := -i

ccc := cc
ifeq "$(arch)" "rt"
  ccc := pcc
endif

cpp   := $(ccc) -E $(cppflags)
ccom  := $(ccc)
ld    := $(ccc)

dep   := $(cpp) -M
cc    := $(ccom) $(ccflags) $(cppflags)
ccpp  := CC +p +w
picc  := $(cpp) -P
icc   := $(ccom) $(ccflags)
ld    := $(ld) $(ldflags)
yacc  := bison -y $(yaccflags)
lex   := flex -t $(lexflags)
lint_first  := lint $(lint_first_pass_flag) $(lintflags) $(cppflags)
lint_second := lint $(lintflags)

full_program := $(gen_dir)/$(program)

#	define a shell command to make any necessary
#	directories for the target file in a rule
#
define mkdirs
test -d $(dir $@) || { \
dirlist="$(subst /, /,$(dir $@))" ; \
cur_dir="" ; \
for f in $$dirlist ; do \
	cur_dir=$$cur_dir$$f ; \
	test -d $$cur_dir || \
		{ echo "making $$cur_dir" ; mkdir $$cur_dir ; } ; \
done ; \
}
endef

# shell command to print what file is being made
define making
@echo making $(subst $(gen_dir)/,,$@)
endef

# this filters the output of cpp -M into the .dep files
# it removes files we don't care about, redundancies, and
# adds the path to the generated files to the .o files
depend_filter_one = \
	egrep -v '/include/' | sort | uniq | \
	sed -e 's/^/$(subst /,\/,$(dir $@))/'

# this replaces a dependency for a .o file with one for each 
# of the sorts of generated files we have:  .o, .ln, and .dep
depend_filter_two = \
	sed -e p -e 's/\.o:/\.dep:/' \
	    -e p -e 's/\.dep:/\.ln:/' | \
	sed -e 's/\.o:/.o  :/' -e 's/\.ln:/.ln :/'


depend_file := $(gen_dir)/dep.$(program)


tar_tmp_file := /tmp/fnord.$(shell echo $$$$).tar
tar_x_file   := /tmp/fnord.$(shell echo $$$$).X

# this removes files that we don't want to distribute, 
# like backup files and the tarfile itself
tar_filter := egrep '(/~$$|/\#|/\.\#|fnord\.tar\.Z|/lib/.)'

ifeq "$(os)" "SunOS"
 define make_tar
 cd .. ;\
 find . -print | $(tar_filter) > $(tar_x_file) ;\
 tar cfX $(tar_tmp_file) $(tar_x_file) . ;\
 rm -f $(tar_x_file) ;\
 compress $(tar_tmp_file) ;\
 mv $(tar_tmp_file).Z ./fnord.tar.Z
endef
else
 define make_tar
 cd .. ;\
 tar cf $(tar_tmp_file) . ;\
 compress $(tar_tmp_file) ;\
 mv $(tar_tmp_file).Z ./fnord.tar.Z
endef
endif

# for a distrubution that has only the .fnorse libraries
# and the statically linked executable, xmfnord.static. 
# rch 
tar_static_filter = grep exec | grep -v dynamic | sed -e 's/:.*//'

ifeq "$(os)" "SunOS"
 define make_static_tar
 cd .. ;\
 find ./src/fnorse -print > $(tar_x_file) ;\
 find ./doc -print >> $(tar_x_file) ;\
 echo  "./lib/$(arch)-$(mode)/xmfnord.static" >> $(tar_x_file)  ;\
 tar cf $(tar_tmp_file) -I $(tar_x_file) ;\
 rm -f $(tar_x_file) ;\
 compress $(tar_tmp_file) ;\
 mv $(tar_tmp_file).Z ./fnord.static.tar.Z
endef	
endif

patch_file := src/admin/Patch-Dates
patch_date = `tail -1 $(patch_file) | colrm 29`

define make_patch
cd .. ;\
cvs patch -D "$(patch_date)" world > ./fnord.patch ;\
echo "`date`   $(note)" >> $(patch_file)
endef

define make_patch_list
cd .. ; cvs patch -s -D "$(patch_date)" world
endef

define make_lex
rm -f $@ ; $(lex) $< > $@
endef

define lint_header
@echo "lint second pass"
@echo "================"
endef

# this allows one to link into /tmp and then mv the executable
# to where it belongs.  This hides a bug in ibm NFS.
# note use of := vs = is important.
ifeq "$(arch)" "IBM6000"
link_dest := /tmp/fnord.$(shell echo $$$$).link_dest
else
link_dest = $@.new
endif

# this avoids a bug in Sun's NFS.  bad things can happen if you
# replace a running executable otherwise.
define replace_executable
test -f $@ && mv $@ $@.old ;\
mv $(link_dest) $@ ; rm -f $@.old
endef

ifeq "stellar" "stellar"
# no cb :(
clean_i_code := cat -s
else
clean_i_code := cat -s | cb -s -l 60 | expand -3
endif

# shell code to compile .c to .o via a .i
define make_cppsrc
cd $(@D) ; \
$(picc) $< | $(clean_i_code) > $(@F:.o=.c) ; \
$(icc) -c $(src_dir)/$(@:.o=.c)
endef

hang := while sleep 1; do sleep 1; done

################################################################
#		Targets
################################################################

.PHONY: program depend clean tar run
.NOEXPORT:

# bozo files are used in several places as files that will never be
# created to work around the bug in gnumake that phony targets can
# not be dependents of other targets.

# build an executable
program-bozo program: config-bozo $(full_program)

#generate dependencies
depend: config-bozo depend-bozo

lint: config-bozo lint-bozo

# target to print the configuration
# makes sure mode and program have recognized values
ifeq "$(filter $(mode),$(modes))" ""
config config-bozo:
	@echo "mode = $(mode), program = $(program)"
	@echo "*******************************"
	@echo "***** mode is unrecognized ****"
	@echo "*******************************"
	@exit 1
	@$(hang)
else
ifeq "$(filter $(program),$(programs))" ""
config config-bozo:
	@echo "mode = $(mode), program = $(program)"
	@echo "**********************************"
	@echo "***** program is unrecognized ****"
	@echo "**********************************"
	@exit 1
	@$(hang)
else
config config-bozo:
	@echo "mode = $(mode), program = $(program)"
endif
endif

# generates the config file from the current configuration
config.make:
	$(making)
	@echo "mode    = $(mode)"    >  $@
	@echo "program = $(program)" >> $@


# remove all generated files
ifdef echo
clean: config-bozo
	rm -rf $(gen_dir)
tar:
	$(make_tar)
static:
	$(make_static_tar)
patch:
	$(make_patch)
patch-list:
	$(make_patch_list)

else

clean: config-bozo
	$(making)
	@rm -rf $(gen_dir)
tar:
	@$(make_tar)
static:
	@$(make_static_tar)
patch:
	@$(make_patch)
patch-list:
	@$(make_patch_list)
endif

run: program-bozo
	@file $(full_program)
	@ls -gilsFL $(full_program)
	@size $(full_program)
	@$(full_program) fnorse/std.fnorse fnorse/editor.fnorse

include admin/Echo-Help.make
include $(depend_file)

################################################################
#		Rules
################################################################



# generates an object from c source
ifdef cppsrc
ifdef echo
$(gen_dir)/%.o : $(src_dir)/%.c
	$(mkdirs)
	$(make_cppsrc)
else
$(gen_dir)/%.o : $(src_dir)/%.c
	$(making)
	@$(mkdirs)
	@$(make_cppsrc)
endif
else
ifdef echo
$(gen_dir)/%.o : $(src_dir)/%.c
	$(mkdirs)
	cd $(@D); $(cc) -c $<
else
$(gen_dir)/%.o : $(src_dir)/%.c
	$(making)
	@$(mkdirs)
	@cd $(@D); $(cc) -c $<
endif
endif

# generates an object from generated c files
ifdef cppsrc
ifdef echo
$(gen_dir)/%.o : $(gen_dir)/%.c
	cd $(@D); $(cc) -c $(<F)
else
$(gen_dir)/%.o : $(gen_dir)/%.c
	$(making)
	@cd $(@D); $(cc) -c $(<F)
endif
else
ifdef echo
$(gen_dir)/%.o : $(gen_dir)/%.c
	cd $(@D); $(cc) -c $(<F)
else
$(gen_dir)/%.o : $(gen_dir)/%.c
	$(making)
	@cd $(@D); $(cc) -c $(<F)
endif
endif

# generates the .dep files from c source files
ifdef echo
$(gen_dir)/%.dep : $(src_dir)/%.c
	$(mkdirs)
	$(dep) $< | $(depend_filter_one) > $@
else
$(gen_dir)/%.dep : $(src_dir)/%.c
	$(making)
	@$(mkdirs)
	@$(dep) $< | $(depend_filter_one) > $@
endif

# generate the .dep (dependencies) files from generated c files
ifdef echo
$(gen_dir)/%.dep : $(gen_dir)/%.c
	$(dep) $< | $(depend_filter_one) > $@
else
$(gen_dir)/%.dep : $(gen_dir)/%.c
	$(making)
	@$(dep) $< | $(depend_filter_one) > $@
endif

# generate the .ln (lint first pass) files from c source
ifdef echo
$(gen_dir)/%.ln : $(src_dir)/%.c
	$(mkdirs)
	cd $(@D); $(lint_first) $<
else
$(gen_dir)/%.ln : $(src_dir)/%.c
	$(making)
	@$(mkdirs)
	@cd $(@D); $(lint_first) $<
endif

# generate the .ln (lint first pass) files from generated c files
ifdef echo
$(gen_dir)/%.ln : $(gen_dir)/%.c
	cd $(@D); $(lint_first) $(<F)
else
$(gen_dir)/%.ln : $(gen_dir)/%.c
	$(making)
	@cd $(@D); $(lint_first) $(<F)
endif

# generate a .c file from yacc source
ifdef echo
$(gen_dir)/%.c : $(src_dir)/%.y
	$(mkdirs)
	cd $(@D); $(yacc) $<
	mv -f $(@D)y.tab.c $@
else
$(gen_dir)/%.c : $(src_dir)/%.y
	$(making)
	@$(mkdirs)
	@cd $(@D); $(yacc) $<
	@mv -f $(@D)y.tab.c $@
endif

# generate a .c file from lex source
ifdef echo
$(gen_dir)/%.c : $(src_dir)/%.l
	$(mkdirs)
	$(make_lex)
else
$(gen_dir)/%.c : $(src_dir)/%.l
	$(making)
	@$(mkdirs)
	@$(make_lex)
endif

# generates an executable
ifeq "$(arch)" "stellar"
# on the stellar the list of objects exceeds the exec's limit on 
# the length of the command line, so put all the objects into a 
# file and use the link editor.
obj_file := /tmp/fnord.$(shell echo $$$$).link
append_obj = $(shell echo $(obj) >> $(obj_file))
ifdef echo
$(gen_dir)/$(program): $(objs)
	echo > $(obj_file)
	echo $(foreach obj,$(objs),$(append_obj)) > /dev/null
	$(ld) $(obj_file) $(libs) $(ldlibs) -o $(link_dest)
	rm $(obj_file)
	$(replace_executable)
else
$(gen_dir)/$(program): $(objs)
	$(making)
	@echo > $(obj_file)
	@echo $(foreach obj,$(objs),$(append_obj)) > /dev/null
	@$(ld) $(obj_file) $(libs) $(ldlibs) -o $(link_dest)
	@rm $(obj_file)
	@$(replace_executable)
endif
else
ifdef echo
$(gen_dir)/$(program): $(objs)
	$(ld) $(objs) $(libs) $(ldlibs) -o $(link_dest) 
	$(replace_executable)
else
$(gen_dir)/$(program): $(objs)
	$(making)
	@$(ld) $(objs) $(libs) $(ldlibs) -o $(link_dest) 
	@$(replace_executable)
endif
endif

# generate the Dependencies file
ifdef echo
depend-bozo : $(objs:.o=.dep)
	cat $^ | $(depend_filter_two) > $(depend_file)
else
depend-bozo : $(objs:.o=.dep)
	@echo making $(notdir $(depend_file))
	@cat $^ | $(depend_filter_two) > $(depend_file)
endif

# lint second pass
ifdef echo
lint-bozo : $(objs:.o=.ln)
	$(lint_header)
	$(lint_second) $(lintlibs) $^
else
lint-bozo : $(objs:.o=.ln)
	$(lint_header)
	@$(lint_second) $(lintlibs) $^
endif

# all the package Defs files are catted into one file
ifdef echo
$(defs_file): $(addsuffix /Defs,$(packages))
	$(mkdirs)
	cat $^ > $(defs_file)
else
$(defs_file): $(addsuffix /Defs,$(packages))
	$(making)
	@$(mkdirs)
	@cat $^ > $(defs_file)
endif


