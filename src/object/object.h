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

/* $Id: object.h,v 1.3 1992/06/22 17:05:23 mas Exp $ */

#ifndef object_has_been_included
#define object_has_been_included

#include "global/global.h"

#ifdef DEBUG
#define object__debug
#endif

#define object__debug_print
#ifdef object__debug
#define object__debug_magic
#define object__debug_trace
#define object__debug_memory
#endif


/*
 * objects from all clients
 ************/
#define object_type_none		(-1)

#define object_type_met_set		0
#define object_type_met_type		1
#define object_type_met_data		2
#define object_type_met_verts		3
#define object_type_met_sym		4
#define object_type_met_block		5
#define object_type_met_connex		6
#define object_type_met_list		7

#define object_type_bsp_node		8

#define object_type_max			9
/*************
 * end objects from all clients
 */


/*
 * general list based memory management macros
 */
#ifdef object__debug_magic
#define object_copy(ptr, obj, mm_type) { \
   (obj)->mminfo.ref_count++;      	  \
   (ptr) = (obj);		    	   \
   object__count_copies[mm_type]++;         \
}
#define object_copy_deep(ptr, obj, func, mm_type) { \
   (ptr) = (func)(obj);        			     \
   object__count_deep_copies[mm_type]++;      	      \
}
#define object_alloc(ptr, list, func, mm_type) { \
   if ((list) == NULL)		                  \
      (func)();     			           \
   (ptr) = (list);			            \
   (list) = (list)->mminfo.next;	             \
   (ptr)->mminfo.ref_count = 1;		              \
   (ptr)->magic = object__magic[mm_type];	       \
   object__count_allocs[mm_type]++;		        \
}
#define object_free(obj, func, mm_type) {           \
   if (object__magic_free == (obj)->magic) {         \
      (void) printf("obj has already been freed.\n"); \
      abort(); 					       \
   } 							\
  object__count_frees[mm_type]++;	\
   if (--(obj)->mminfo.ref_count == 0) { \
      (func)(obj);			  \
      object__count_deep_frees[mm_type]++; \
   }					    \
}

#else

#define object_copy(ptr, obj, mm_type) { \
   (obj)->mminfo.ref_count++;      	  \
   (ptr) = (obj);		    	   \
}
#define object_copy_deep(ptr, obj, func, mm_type) { \
   (ptr) = (func)(obj);        			     \
}
#define object_alloc(ptr, list, func, mm_type) { \
   if ((list) == NULL)		                  \
      (func)();     				   \
   (ptr) = (list);			            \
   (list) = (list)->mminfo.next;	             \
   (ptr)->mminfo.ref_count = 1;		              \
}
#define object_free(obj, func, mm_type) { \
   if (--(obj)->mminfo.ref_count == 0) {   \
      (func)(obj);	  		    \
   }					     \
}
#endif object__debug_magic


#define object_copy_null(ptr, obj, mm_type) { \
   if (NULL == (obj)) {			       \
      (ptr) = NULL;				\
   } else {					 \
      object_copy(ptr, obj, mm_type);		  \
   }						   \
}
#define object_free_null(obj, mm_type) { \
   if (NULL != (obj)) {			  \
      object_free(obj, mm_type);	   \
   }					    \
}


#ifdef object__debug_magic
#define object_check_magic(obj, mm_type) {     \
   ALLOC_CHECK_PTR(obj);			\
   if ((int)(obj) % MEM_ALIGN_GENERIC) {	 \
      FATAL((ERR_SS, "obj", "is misaligned"))     \
   }						   \
   if ((obj)->magic != object__magic[mm_type]) {    \
      if ((ptr)->magic != object__magic_free) {	     \
	 FATAL((ERR_SS, "obj", "has bad magic"));     \
      } else {					       \
	 FATAL((ERR_SS, "obj", "has free magic"));      \
      }						         \
   }						          \
}
#define object_check_null_magic(obj, mm_type) { \
   if (NULL != (obj)) {			         \
      object_check_magic(obj, mm_type);	          \
   }					           \
}
#else
#define object_check_magic(obj, mm_type)
#define object_check_null_magic(obj, mm_type)
#endif object__debug_magic


#define object__magic_free		0x8da42b28L

#define object__null_chunk	((object__chunk_header *) NULL)
typedef struct object__chunk_header object__chunk_header;
struct object__chunk_header {
   object__chunk_header	*next;
   char			*first_of_chunk;
};


extern int object__count_copies[object_type_max];
extern int object__count_deep_copies[object_type_max];
extern int object__count_allocs[object_type_max];
extern int object__count_frees[object_type_max];
extern int object__count_deep_frees[object_type_max];
extern int object__count_chunks[object_type_max];
extern long object__magic[object_type_max];
extern object__chunk_header *object__chunk_list[object_type_max];


#endif  object_has_been_included
