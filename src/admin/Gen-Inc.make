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
# $Id: Gen-Inc.make,v 1.4 1992/06/22 17:02:53 mas Exp $

# include this file whenever there are generated include files

# gen_src is the generated file
# gen_obj is the root of the file that eventually includes it
# gen_pkg is the package containing gen_src

$(gen_dir)/$(gen_pkg)/$(gen_obj).o   : $(gen_dir)/$(gen_pkg)/$(gen_src)
$(gen_dir)/$(gen_pkg)/$(gen_obj).dep : $(gen_dir)/$(gen_pkg)/$(gen_src)
$(gen_dir)/$(gen_pkg)/$(gen_obj).ln  : $(gen_dir)/$(gen_pkg)/$(gen_src)

