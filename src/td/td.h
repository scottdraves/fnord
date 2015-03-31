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
 * $Id: td.h,v 1.5 1992/06/22 17:09:32 mas Exp $ */

extern METsym *widget();
extern void destroy_new_widgets();
extern void fix_widgets();
extern void destroy_widgets();
extern void td_fd_add();
extern void td_fd_remove();
extern int  td_fd_check();
extern int  td_fd_get_string();
extern void td_work();
extern void td_destroy();
extern void td_init();
extern void td_update();
extern void td_add_active();
extern FOboolean td_control_callback();
