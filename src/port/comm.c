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

#ifndef lint
static char Version[] =
   "$Id: comm.c,v 1.2 1992/06/22 17:08:21 mas Exp $";
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <memory.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

/* valid SunOS ONLY! */
#define MAX_FD		256

#define ME(A)

#include "comm.h"

extern int errno;

int CommSetupRead(port)
   unsigned long	port;
{
   int			sock;
   int			result;
   struct sockaddr_in	server;
   ME(CommSetupRead);
   
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0)
      return sock;

   result = fcntl(sock, F_SETFL, FNDELAY);
   if (result < 0) {
      shutdown(sock, 2);
      close(sock);
      return -1;
   }

   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = htons(port);

   result = bind(sock, (struct sockaddr *) &server, sizeof(server));
   if (result < 0) {
      shutdown(sock, 2);
      close(sock);
      return -1;
   }

   listen(sock, 1);
   return sock;
}

int CommConnectRead(old_sock)
   int		*old_sock;
{
   int		new_sock;
   ME(CommConnectRead);

   new_sock = accept(*old_sock, (struct sockaddr *) NULL, (int *) NULL);
   if (new_sock < 0 && EWOULDBLOCK == errno)
      return new_sock;

   shutdown(*old_sock, 2);
   close(*old_sock);
   *old_sock = -1;

   return new_sock;
}

int CommSetupWrite()
{
   int			sock;
   int			result;
   static int		proto = 0;
   struct protoent	*pe;
   ME(CommSetupWrite);

   if (0 == proto) {
      pe = getprotobyname("TCP");
      proto = pe->p_proto;
   }

   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0)
      return sock;

   result = fcntl(sock, F_SETFL, FNDELAY);
   if (result < 0) {
      shutdown(sock, 2);
      close(sock);
      return -1;
   }

#if 0
   (void) setsockopt(sock, proto, TCP_NODELAY, &proto, sizeof(int));
#endif

   return sock;
}

   
int CommConnectWrite(sock, host, port)
   int			*sock;
   char			*host;
   unsigned long	port;
{
   int			result;
   struct sockaddr_in	server;
   struct hostent 	*hp, *gethostbyname();
   ME(CommConnectWrite);

   server.sin_family = AF_INET;
   hp = gethostbyname(host);
   
   if (hp == (struct hostent *) NULL)
      return -1;

   memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);
   server.sin_port = htons(port);

   result = connect(*sock, (struct sockaddr *) &server, sizeof(server));

   /* this will only happen if connect blocks */
   if (result == 0)
      return 1;

   switch (errno) {
    case EALREADY:
    case EINPROGRESS:
    case EINTR:
    case ETIMEDOUT:
      /* Try again later */
      return 0;

    case EISCONN:
      /* The last one must have worked */
      return 1;

    default:
      /* major problem */
      shutdown(*sock, 2);
      close(*sock);
      *sock = CommSetupWrite();
      return 0;
   }
}
