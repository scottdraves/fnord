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

/* $Id: td_top.h,v 1.2 1992/06/22 17:10:49 mas Exp $ */

#ifndef TD_TOP_H_INCLUDED
#define TD_TOP_H_INCLUDED

/* Highest level widget */

#if	defined(X_TWO_D)

/* All the information required to work things in X */
typedef struct fnw_top {
   XtAppContext		app;
   XtInputId		x_input;
   int			running;
   DrawEnv		env;
   int			text_input;
   int			length;
   int			desired;
   int			ready;
   char			*text;
   int			x_subdiv;
   int			y_subdiv;
} fnw_top;

#if	defined(MOTIF_WIDGETS)
extern XtTranslations td_text_translations;
#endif  /* defined(MOTIF_WIDGETS) */

#else

typedef struct fnw_call {
   int			fd;
   FOboolean		alive;
   void			(*readfunc)();
   void			(*writefunc)();
   char			*closure;
} fnw_call;


/* All the information required to work things in ttyfnord */
typedef struct fnw_top {
   fnw_call		*calls;
   int			nfds;
   FOboolean		destroy_fds;
   int			running;
   int			text_input;
   int			length;
   int			desired;
   int			ready;
   char			*text;
} fnw_top;

extern void td_wfd_add();
extern void td_wfd_remove();
extern void td_wfd_clean();

#endif /* defined(X_TWO_D) */

extern fnw_type fnw_type_top;
extern void td_init(/* argc, argv */);
extern void td_register_running( /* is_on */ );

#endif /* defined TD_TOP_H_INCLUDED */
