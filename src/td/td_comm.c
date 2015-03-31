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

#include "td/defs.h"
#include "port/comm.h"
#include <fcntl.h>
#include <errno.h> /* Why haven't we done this anywhere else? */

#ifndef lint
static char Version[] =
   "$Id: td_comm.c,v 1.3 1993/06/03 14:16:30 rch Exp $";
#endif

static METsym	*td_read_create();
static FOboolean td_read_request();
static void      td_read_destroy();

fnw_type fnw_type_read = {
   WIDGET_TYPE_READ,  	/* type */
   STD_ARG_READ,	/* name arg */
   td_read_create,	/* create */
   td_read_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_read_request,	/* request */
};

static METsym	*td_write_create();
static FOboolean td_write_request();
static void      td_write_destroy();
static void	 td_write_clear();
static void	 td_write_update();

fnw_type fnw_type_write = {
   WIDGET_TYPE_WRITE,  	/* type */
   STD_ARG_WRITE,	/* name arg */
   td_write_create,	/* create */
   td_write_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   td_write_update,	/* update */
   NULL,		/* post update */
   td_write_request,	/* request */
};

/************************************************************************/
/*									*/
/*				READ					*/
/*									*/
/************************************************************************/

/*
 *  This function is called by X when there is read input on
 * a connected read socket.  The function snarfs as much as it can 
 * into its buffer.  (If there is an EOF condition, it shuts things
 * down, by the way.)  Then it calls the appropriate reading function,
 * until the reading functions cannot absorb any more information.  
 * The reading function either finishes reading the object begun
 * in list or starts a new one, and results either in a new object, 
 * or the creation of a new list recording a partial object.  If
 * the function gets a new set, it updates the display.
 */

#ifdef X_TWO_D
/* ARGSUSED */
static void
td_read_info(client_data, source, id)
   caddr_t 	client_data;
   int		*source;
   XtInputId	*id;
#else
static void 
td_read_info(client_data)
   caddr_t	client_data;
#endif
{
   fnord_widget *reader = CAST(fnord_widget *, client_data);
   int		i;
   FOboolean	first = TRUE;
   MET_object_p object;
   int		mm_type;
   ME(td_read_info);

   if (reader->info.read.size == 0) {
      reader->info.read.size = 2032;
      ALLOCNM(reader->info.read.buf, char, reader->info.read.size);
      reader->info.read.len = 0;
   }

   do {
      i = read(reader->info.read.socket, 
	       reader->info.read.buf + reader->info.read.len,
	       reader->info.read.size - reader->info.read.len);
      
      if (i < 0)
	 return;

      if (i == 0 && YES == first) {
	 shutdown(reader->info.read.socket, 2);
	 close(reader->info.read.socket);
#ifdef X_TWO_D
	 XtRemoveInput(reader->info.read.x_input);
	 reader->info.read.x_input = None;
#else
	 td_wfd_remove(reader->info.read.socket);
#endif
	 reader->info.read.socket = -1;
	 reader->info.read.connected = FALSE;
	 return;
      } 

      first = NO;

      reader->info.read.len += i;

      if (i < reader->info.read.size - reader->info.read.len) 
	 break;
      
      REALLOCNM(reader->info.read.buf, char, reader->info.read.size,
		reader->info.read.size << 1);
      reader->info.read.size <<= 1;
   } while (1);

   do {
      i = 0;
      object.set = MET_NULL_SET;
      if (MET_NULL_LIST == reader->info.read.list) 
	 object.set = METset_read(reader->info.read.buf, 
				  reader->info.read.len,
				  &i, &reader->info.read.list, 
				  object, FALSE);

      
      else
	 object = MET_list_read(reader->info.read.buf, 
				reader->info.read.len, 
				&i, &reader->info.read.list, &mm_type);
      
      memcpy(reader->info.read.buf,
	     reader->info.read.buf + i, reader->info.read.len - i);
      reader->info.read.len -= i;
      if (MET_NULL_SET != object.set) {
	 MET_SET_FREE_NULL(reader->info.read.set);
	 MET_SET_COPY(reader->info.read.set, object.set);
	 MET_SET_FREE(object.set);
	 reader->state |= FNW_STATE_CHANGED_NOW;
      }
   } while (object.set != MET_NULL_SET);

   if (FNW_IS_CHANGED(reader))
      td_update();
}

#ifdef X_TWO_D
/* ARGSUSED */
static void
td_read_connect(client_data, source, id)
   caddr_t 	client_data;
   int		*source;
   XtInputId	*id;
#else
static void
td_read_connect(client_data)
   caddr_t	client_data;
#endif
{
   fnord_widget *reader = CAST(fnord_widget *, client_data);
   int		new_socket;
#ifndef X_TWO_D
   int		old_socket = reader->info.read.socket;
#endif
   ME(td_read_connect);

   new_socket = CommConnectRead(&reader->info.read.socket);
   if (new_socket < 0)
      return;

#ifdef X_TWO_D
   XtRemoveInput(reader->info.read.x_input);
   reader->info.read.x_input = 
      XtAppAddInput(widget__top->info.top.app,  new_socket, 
		    XtInputReadMask, (XtInputCallbackProc) td_read_info,
		    (caddr_t) reader);
#else
   td_wfd_remove(old_socket);
   td_wfd_add(new_socket, td_read_info, FUNC_NULL, (caddr_t) reader);
#endif
   reader->info.read.socket = new_socket;
   reader->info.read.connected = TRUE;
}

static FOboolean td_read_request(reader, req)
   fnord_widget		*reader;
   METcontrol_request	*req;
{
   fd_set		fds;
   ME(td_read_request);

   /* If we don't have anything to report, block until
      we get something. */

   if (MET_NULL_SET == reader->info.read.set) {
      FD_ZERO(&fds);
      FD_SET(reader->info.read.socket, &fds);
      TD_MSG(stdout, "Waiting for information over network.\n");
      fcntl(reader->info.read.socket, F_SETFL, 0);
      while (MET_NULL_SET == reader->info.read.set) {
	 select(256 /* ACK! */, &fds, (fd_set *) NULL, 
		(fd_set *) NULL, (struct timeval *) NULL);
	 if (TRUE == reader->info.read.connected) {
#ifdef X_TWO_D
	    td_read_info((caddr_t) reader, INULL, (XtInputId *) NULL);
	 } else {
	    td_read_connect((caddr_t) reader, INULL, (XtInputId *) NULL);
#else
	    td_read_info((caddr_t) reader);
	 } else {
	    td_read_connect((caddr_t) reader);
#endif
	 }
      }
      fcntl(reader->info.read.socket, F_SETFL, FNDELAY);
   }

   switch(req->code) {
    case MET_CONTROL_REQUEST_TYPE:
      req->type = MET_NULL_TYPE;
      break;

    case MET_CONTROL_REQUEST_CHANGED:
      FNW_SET_REQ_CHANGED(req, reader);
      break;

    case MET_CONTROL_REQUEST_VALUE:
      MET_SET_COPY_NULL(req->value, reader->info.read.set);
      break;
      
    default:
      DEFAULT_ERR(req->code);
   }

   return SUCCESS;
}

static void td_read_destroy(reader)
   fnord_widget		*reader;
{
   ME(td_read_destroy);

   if (reader->info.read.socket != -1) {
      shutdown(reader->info.read.socket, 2);
      close(reader->info.read.socket);
#ifndef X_TWO_D
      td_wfd_remove(reader->info.read.socket);
#else
      if (None != reader->info.read.x_input)
	 XtRemoveInput(reader->info.read.x_input);
#endif
   }

   MET_SET_FREE_NULL(reader->info.read.set);
   MET_LIST_FREE_NULL(reader->info.read.list);
   if ((char *) NULL != reader->info.read.buf) {
      FREE(reader->info.read.buf, reader->info.read.size);
   }
}

static METsym *td_read_create(parent, name, alist, sym)
   fnord_widget		*parent;
   char			*name;
   METset		*alist;
   METsym		*sym;
{
   unsigned long	port;
   int			socket;
   fnord_widget		*reader;
   widget_list		*list;
   ME(td_read_create);

   if (FAILURE == td_lookup_int(alist, std_args[STD_ARG_PORT], &port,
				 0, (Sym) NULL, 0)) {
      TD_ERR(sym->origin, ": Must specify port to connect to\n");
      return MET_NULL_SYM;
   }
   
   socket = CommSetupRead(port);
   if (socket < 0) {
      TD_ERR(sym->origin, ": Unable to open socket.\n");
      return MET_NULL_SYM;
   }
   
   ALLOCM(reader, fnord_widget);

   reader->type = WIDGET_TYPE_READ;
   (void) strcpy(reader->name, name);
   reader->kids = WL_NULL;
   reader->dft_kid_type = WIDGET_TYPE_NONE;
   reader->state = FNW_STATE_START;

#ifdef X_TWO_D
   reader->xw.w = reader->xw.top = None;
#endif

   reader->info.read.socket = socket;
   reader->info.read.port = port;
   reader->info.read.set = MET_NULL_SET;
   reader->info.read.list = MET_NULL_LIST;
   reader->info.read.buf = (char *) NULL;
   reader->info.read.size = reader->info.read.len = 0;
   reader->info.read.connected = FALSE;

#ifdef X_TWO_D
   reader->info.read.x_input =
      XtAppAddInput(widget__top->info.top.app, socket, XtInputReadMask,
		    (XtInputCallbackProc) td_read_connect,
		    (caddr_t) reader);
#else
   td_wfd_add(socket, td_read_connect, FUNC_NULL, (caddr_t) reader);
#endif

   ALLOCM(list, widget_list);
   list -> next = widget__top->kids;
   list -> w = reader;
   widget__top->kids = list;
   return METsym_new_control((char *) reader, sym->origin);
}

/************************************************************************/
/*									*/
/*				WRITE					*/
/*									*/
/************************************************************************/

/*
 *  This function is called by X when there is read input 
 * on a writing connection.  If there is anything to be read,
 * we ignore it.  (What would we do with it?)  If there is
 * nothing there, it's unix's sly way of saying that the other
 * end of the socket has been closed.
 */

#ifdef X_TWO_D
/* ARGSUSED */
static void
td_off(client_data, source, id)
   caddr_t	client_data;
   int		*source;
   XtInputId	*id;
#else
static void
td_off(client_data)
   caddr_t	client_data;
#endif
{
   fnord_widget *writer = CAST(fnord_widget *, client_data);
   char		buf[10];
   int		i;
   FOboolean	first = TRUE;
   ME(td_off);

   do {
      i = read(writer->info.write.socket, buf, 10);
      
      if (i == 0 && YES == first) {
	 shutdown(writer->info.write.socket, 2);
	 close(writer->info.write.socket);
#ifdef X_TWO_D
	 XtRemoveInput(writer->info.write.x_input);
	 writer->info.write.x_input = None;
#else
	 td_wfd_remove(writer->info.write.socket);
#endif
	 writer->info.write.socket = -1;
	 writer->info.write.connected = FALSE;
      }
	 
   } while (i == 10) ;
}

/*
 *  This function is called when a writing widget's component set is
 * changed.  If the connection has been made, it writes the set to a  
 * buffer and sends the buffer (in pieces if necessary).
 */

static void
td_write_set(writer)
   fnord_widget	*writer;
{
   char 	buf[8192], *pos = buf;
   int		len = 8192;
   int		i;
   fd_set	fds;
   ME(td_write_set);

   if (TRUE != writer->info.write.connected)
      return;

   if (SUCCESS == METset_write(writer->info.write.set, &pos, &len)) {
      pos = buf;
      len = 8192 - len;
      do {
	 i = write(writer->info.write.socket, pos, len);
	 if (i == len)
	    return;
	 if (-1 == i) 
	    switch (errno) {
	     case EWOULDBLOCK:
	     case EAGAIN:
	       FD_ZERO(&fds);
	       FD_SET(writer->info.write.socket, &fds);
	       (void) select(256, (fd_set *) NULL, &fds,
			     (fd_set *) NULL, (struct timeval *) NULL);
	       break;
	     default:
	       return;
	    }
	 len -= i;
	 pos += i;
      } while (1);
   }
}

#ifdef X_TWO_D
/*ARGSUSED*/
static void
td_write_connect(client_data, source, id)
   caddr_t 	client_data;
   int		*source;
   XtInputId	*id;
#else
static void
td_write_connect(client_data)
   caddr_t	client_data;
#endif
{
   fnord_widget *writer = CAST(fnord_widget *, client_data);
   int		result;
#ifndef X_TWO_D
   int		oldsock = writer->info.write.socket;
#endif
   ME(td_write_connect);

   result = CommConnectWrite(&writer->info.write.socket,
			     SYM_STRING(writer->info.write.host),
			     writer->info.write.port);

   switch(result) {
    case -1:
      /* Ack! */
      break;

    case 0:
#ifdef X_TWO_D
      if (None != writer->info.write.timer)
	 XtRemoveTimeOut(writer->info.write.timer);
      writer->info.write.timer = 
	 XtAppAddTimeOut(widget__top->info.top.app, 1000, 
			 (XtTimerCallbackProc) td_write_connect,
			 (caddr_t) writer);
#else
      td_wfd_remove(oldsock);
      td_wfd_add(writer->info.write.socket, FUNC_NULL,
		 td_write_connect, (caddr_t) writer);
#endif
      break;

    case 1:
#ifdef X_TWO_D
      if (None != writer->info.write.timer) {
	 XtRemoveTimeOut(writer->info.write.timer);
	 writer->info.write.timer = None;
      }
      if (None == writer->info.write.x_input)
	 writer->info.write.x_input =
	    XtAppAddInput(widget__top->info.top.app, 
			  writer->info.write.socket, XtInputReadMask,
			  (XtInputCallbackProc) td_off,
			  (caddr_t) writer);
#else
      td_wfd_add(writer->info.write.socket, td_off,
		 FUNC_NULL, (caddr_t) writer);
#endif
      writer->info.write.connected = TRUE;
      td_write_set(writer);	
      break;
   }
}

static void td_write_update(writer)
   fnord_widget		*writer;
{
   int		changed;
   ME(td_write_update);

   if (MET_NULL_SET == writer->info.write.set ||
       NO == writer->info.write.connected)
      return;

   if (!FNW_IS_TESTED(writer)) {
      changed = METset_clear_rep(writer->info.write.set);
      (void) METset_have_rep(writer->info.write.set, (FOboolean *) NULL);
      FNW_SET_CHANGED_MET(writer, changed);
   }

   if (FNW_IS_CHANGED(writer))
      td_write_set(writer);
}

static FOboolean td_write_request(writer, req)
   fnord_widget		*writer;
   METcontrol_request	*req;
{
   int		changed;
   ME(td_write_request);

   if (FALSE == writer->info.write.connected && 
       -1 != writer->info.write.socket) 
      do {
#ifdef X_TWO_D
	 td_write_connect((caddr_t) writer, INULL, (XtInputId *) NULL);
#else
	 td_write_connect((caddr_t) writer);
#endif
	 if (FALSE == writer->info.write.connected)
	    usleep(100000);
	 else
	    break;
      } while (1);

   if (!FNW_IS_TESTED(writer) && MET_NULL_SET != writer->info.write.set) {
      changed = METset_clear_rep(writer->info.write.set);
      (void) METset_have_rep(writer->info.write.set, (FOboolean *) NULL);
      FNW_SET_CHANGED_MET(writer, changed);
      writer->state |= FNW_STATE_TESTED;
   }

   switch(req->code) {
    case MET_CONTROL_REQUEST_TYPE:
      req->type = MET_NULL_TYPE;
      break;

    case MET_CONTROL_REQUEST_CHANGED:
      FNW_SET_REQ_CHANGED(req, writer);
      break;

    case MET_CONTROL_REQUEST_VALUE:
      MET_SET_COPY_NULL(req->value, writer->info.write.set);
      break;
      
    default:
      DEFAULT_ERR(req->code);
   }
}

static void td_write_destroy(writer)
   fnord_widget		*writer;
{
   ME(td_write_destroy);

   if (writer->info.write.socket != -1) {
      shutdown(writer->info.write.socket, 2);
      close(writer->info.write.socket);
#ifndef X_TWO_D
      td_wfd_remove(writer->info.write.socket);
#endif
   }
#ifdef X_TWO_D
   if (None != writer->info.write.x_input)
      XtRemoveInput(writer->info.write.x_input);
   if (None != writer->info.write.timer)
      XtRemoveTimeOut(writer->info.write.timer);
#endif

   MET_SET_FREE_NULL(writer->info.write.set);
}

static METsym *td_write_create(parent, name, alist, sym)
   fnord_widget		*parent;
   char			*name;
   METset		*alist;
   METsym		*sym;
{
   unsigned long	port;
   int			socket;
   fnord_widget		*writer;
   widget_list		*list;
   METset		*set;
   char			buf[256];
   int			length = 255, connect;
   Sym			host;
   ME(td_write_create);

   if (FAILURE == td_lookup_int(alist, std_args[STD_ARG_PORT], (int *) &port,
				0, (Sym) NULL, 0)) {
      TD_ERR(sym->origin, "Must specify port to connect to\n");
      return MET_NULL_SYM;
   }

   set = METset_assoc(alist, std_args[STD_ARG_HOST]);
   if (MET_NULL_SET == set || 
       FAILURE == METset_get_string(set, buf, &length)) {
      TD_ERR(sym->origin, "Must specify string for host to write to\n");
      MET_SET_FREE_NULL(set);
      return MET_NULL_SYM;
   }
   MET_SET_FREE(set);
   buf[length] = 0;
   host = sym_from_string(buf);
   
   set = td_make_set_recalc(alist, STD_ARG_SET, sym->origin);
   if (MET_NULL_SET == set) {
      TD_ERR(sym->origin, "Must specify set to write.\n");
      return MET_NULL_SYM;
   }
   
   socket = CommSetupWrite();
   
   if (socket < 0) {
      TD_ERR(sym->origin, "Unable to open socket.\n");
      MET_SET_FREE(set);
      return MET_NULL_SYM;
   }

   connect = CommConnectWrite(&socket, SYM_STRING(host), port);

   ALLOCM(writer, fnord_widget);
   
   writer->type = WIDGET_TYPE_WRITE;
   (void) strcpy(writer->name, name);
   writer->kids = WL_NULL;
   writer->dft_kid_type = WIDGET_TYPE_NONE;
   writer->state = FNW_STATE_START;

#ifdef X_TWO_D
   writer->xw.w = writer->xw.top = (Widget) None;
#endif

   writer->info.write.socket = socket;
   writer->info.write.port = port;
   writer->info.write.host = host;
   writer->info.write.connected = FALSE;

   MET_SET_COPY_NULL(writer->info.write.set, set);
   MET_SET_FREE_NULL(set);

   if (connect == 0) {
#ifdef X_TWO_D
      writer->info.write.x_input = (XtInputId) None;
      writer->info.write.timer = 
	 XtAppAddTimeOut(widget__top->info.top.app, 1000, 
			 (XtTimerCallbackProc) td_write_connect,
			 (caddr_t) writer);
#else
      td_wfd_add(socket, FUNC_NULL, td_write_connect, (caddr_t) writer);
#endif /* X_TWO_D */
   } else {
#ifdef X_TWO_D
      writer->info.write.connected = TRUE;
      writer->info.write.x_input =
	 XtAppAddInput(widget__top->info.top.app, 
		       writer->info.write.socket, XtInputReadMask,
		       (XtInputCallbackProc) td_off,
		       (caddr_t) writer);
      writer->info.write.timer = (XtIntervalId) None;
#else
      td_wfd_add(socket, td_off, FUNC_NULL, (caddr_t) writer);
#endif /* X_TWO_D */
      td_write_set(writer);
   }

   ALLOCM(list, widget_list);
   list -> next = widget__top->kids;
   list -> w = writer;
   widget__top->kids = list;
   return METsym_new_control((char *) writer, sym->origin);
}

