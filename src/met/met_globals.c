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
   "$Id: met_globals.c,v 1.19 1992/06/22 17:04:41 mas Exp $";
#endif

#include "defs.h"

#define PACKAGE_NAME "MET"

extern Boolean MET__default_control();

static METdefaults default_storage = {
{ MET_error_handler_default },
{ 5, 1.0 },
{ 1e-5 },
{ FALSE, FALSE },
{ TRUE },
{ FALSE },
{ MET__default_control, CNULL},
{ TRUE, TRUE},
};
METdefaults *MET_defaults = &default_storage;

/*
 * sizes of the different fields
 */
int MET_field_size[MET_FIELD_MAX] = {
   sizeof(METztwo),
   sizeof(METinteger),
   sizeof(METangle),
   sizeof(METreal),
   sizeof(METcomplex),
   sizeof(METquaternion),
   sizeof(METmap),
   sizeof(METset *),
   0,
};

/*
 * heads of the free lists
 */
METset		*MET__set_free_list    = MET_NULL_SET;
METtype		*MET__type_free_list   = MET_NULL_TYPE;
METblock	*MET__block_free_list  = MET_NULL_BLOCK;
METsym		*MET__sym_free_list    = MET_NULL_SYM;
METdata		*MET__data_free_list   = MET_NULL_DATA;
METverts	*MET__verts_free_list  = MET_NULL_VERTS;
METconnex	*MET__connex_free_list = MET_NULL_CONNEX;
METlist		*MET__list_free_list   = MET_NULL_LIST;
METenv		*MET__env_free_list    = MET_NULL_ENV;

/*
 * routines to allocate more objects
 */
void (*MET_new_free_list[MET__MM_TYPE_MAX])() = {
   MET__set_new_free_list,
   MET__type_new_free_list,
   MET__data_new_free_list,
   MET__verts_new_free_list,
   MET__sym_new_free_list,
   MET__block_new_free_list,
   MET__connex_new_free_list,
   MET__list_new_free_list,
   MET__env_new_free_list,
};
/*
 * routines to deep free objects
 */
void (*MET_free_deep[MET__MM_TYPE_MAX])() = {
   MET__set_free_deep,
   MET__type_free_deep,
   MET__data_free_deep,
   MET__verts_free_deep,
   MET__sym_free_deep,
   MET__block_free_deep,
   MET__connex_free_deep,
   MET__list_free_deep,
   MET__env_free_deep,
};

/*
 * how many objects we put in a chunk.  this # gets rounded up to fit in a
 * power of two
 */
int MET_chunk_size[MET__MM_TYPE_MAX] = {
   40, 30, 40, 40, 200, 10, 10, 200, 10
};

int MET_obj_size[MET__MM_TYPE_MAX] = {
   sizeof(METset),
   sizeof(METtype),
   sizeof(METdata),
   sizeof(METverts),
   sizeof(METsym),
   sizeof(METblock),
   sizeof(METconnex),
   sizeof(METlist),
   sizeof(METenv),
};

MET_chunk_header *MET_chunk_list[MET__MM_TYPE_MAX] = {
   NULL_CHUNK_HEADER, NULL_CHUNK_HEADER,
   NULL_CHUNK_HEADER, NULL_CHUNK_HEADER, 
   NULL_CHUNK_HEADER, NULL_CHUNK_HEADER,
   NULL_CHUNK_HEADER, NULL_CHUNK_HEADER,
   NULL_CHUNK_HEADER,
};

#ifdef MET__DEBUG_MAGIC
/*
 * track # of allocated objects of different types
 */
int MET_count_allocs[MET__MM_TYPE_MAX] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int MET_count_copies[MET__MM_TYPE_MAX] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int MET_count_deep_copies[MET__MM_TYPE_MAX] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int MET_count_frees[MET__MM_TYPE_MAX] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int MET_count_deep_frees[MET__MM_TYPE_MAX] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int MET_count_chunks[MET__MM_TYPE_MAX] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0,
};

long MET_magic[MET__MM_TYPE_MAX] = {
   0x82a048f2L,
   0x7a010d33L,
   0x486abb71L,
   0x226cf08aL,
   0xb9089348L,
   0x9936a185L,
   0x8d32b425L,
   0xd6717e72L,
   0xaceda22eL,
};

Boolean (*MET_has_ref[MET__MM_TYPE_MAX])() = {
   MET__set_has_ref,
   MET__type_has_ref,
   MET__data_has_ref,
   MET__verts_has_ref,
   MET__sym_has_ref,
   MET__block_has_ref,
   MET__connex_has_ref,
   MET__list_has_ref,
   MET__env_has_ref,
};

void (*MET_print[MET__MM_TYPE_MAX])() = {
   METset_print,
   METtype_print,
   METdata_print,
   METverts_print,
   METsym_print,
   METblock_print,
   METconnex_print,
   METlist_print,
   METenv_print,
};

#endif MET__DEBUG_MAGIC


char *MET_obj_name[MET__MM_TYPE_MAX] = {
   "set",
   "type",
   "data",
   "verts",
   "sym",
   "block",
   "connex",
   "list",
   "env",
};

void
MET_error_handler_default(region, text, severity)
   FILOCloc	*region;
   char		*text;
   int		severity;
{
   FILOCloc_print(region, stdout, CNULL, 0);
   PRINT(stdout, ": %s\n", text);
}

void
MET_error_handler_ignore(region, text, severity)
   FILOCloc	*region;
   char		*text;
   int		severity;
{
   /* do nothing */
}

/*
 * A client using MET to compute sets with controls should do this:
 * 1) freeze the values of its controls
 * 2) evaluate the sets it needs
 * 3) wait for the value of a control to change
 * 4) goto step 1
 *
 * met uses a callback to get the values of the controls, it is set via a
 * default.   This function shows how it should be declared.  What it
 * should do:  if request->code is TYPE, it should place the type that the
 * control's value will be in request->type.  If it it is CHANGED, then it
 * should set request->changed to YES if its value has changed since the
 * last time it was frozen.  If it is VALUE, then it should put the value
 * of the control at the last time it was frozen in request->value.  It can
 * return FAILURE whenever it wants, in which case it should not set any of
 * the fields.  It is not responsible for freeing anything it puts in the
 * request. 
 * 
 */
/*ARGSUSED*/
static Boolean
MET__default_control(request)
   METcontrol_request	*request;
{
   MME(MET__default_control);

   return FAILURE;
}

/*
 * this writes garbage all over the heap and stack so that memory is no all
 * zeros when we first see it.  this flushes out some bugs
 */
#ifdef MET__DEBUG
#define DIRTY_SIZE_HEAP    1000000
#define DIRTY_SIZE_STACK   1000
#define DIRTY_STACK_LEVELS 100
static void
MET__dirty_stack(level, with)
   int		level;
   long		with;
{
   long stack[DIRTY_SIZE_STACK];
   int  i;

   if (level > 0) {
      /* avoid tail recursion */
      MET__dirty_stack(level - 1, with);
      
      for (i = 0; i < DIRTY_SIZE_STACK; i++)
	 stack[i] = with;
   }
}
   
static void
MET__dirty_memory()
{
   long *heap, t;
   int  i;

   if (CNULL != getenv("DIRTY")) {
#ifdef lint
      heap = (long *) 0;
#else
      heap = (long *) malloc(DIRTY_SIZE_HEAP * sizeof(long));
#endif
      if (0 == heap)
	 return;
      t = (CNULL != getenv("DIRTYZERO")) ? 0 : 0xabcd1234;
      for (i = 0; i < DIRTY_SIZE_HEAP; i++)
	 heap[i] = t;
      free((char *) heap);
      
      t = (CNULL != getenv("DIRTYZERO")) ? 0 : 0xdcba4321;
      MET__dirty_stack(DIRTY_STACK_LEVELS, t);
   }
}
#endif MET__DEBUG


void
METinitialize()
{
   int		i;

#ifdef MET__DEBUG
   MET__dirty_memory();
#endif

   for (i = 0; i < MET__MM_TYPE_MAX; i++)
      MET_chunk_size[i] =
	 MET_malloc_round_up(MET_chunk_size[i],
			     MET_obj_size[i]);
      
   MET_list_initialize(); /* this one must be first */
   MET_type_initialize();
   MET_sym_initialize();
   MET_set_initialize();
   MET_env_initialize();
}

#ifdef MET__DEBUG_TRACE
void
METtron()
{
   MME(METtron);

   global_tron();
}

void
METtroff()
{
   MME(METtroff);

   global_troff();
}

#endif MET__DEBUG_TRACE

static void
MET__chase_free_lists()
{
   METset	*set;
   METtype	*type;
   METblock	*block;
   METsym	*sym;
   METdata	*data;
   METverts	*verts;
   METconnex	*connex;
   METlist	*list;
   METenv	*env;

   set = MET__set_free_list;
   while (MET_NULL_SET != set) {
      set = set->mminfo.next;
   }
   type = MET__type_free_list;
   while (MET_NULL_TYPE != type) {
      type = type->mminfo.next;
   }
   block = MET__block_free_list;
   while (MET_NULL_BLOCK != block) {
      block = block->mminfo.next;
   }
   sym = MET__sym_free_list;
   while (MET_NULL_SYM != sym) {
      sym = sym->mminfo.next;
   }
   list = MET__list_free_list;
   while (MET_NULL_LIST != list) {
      list = list->mminfo.next;
   }
   connex = MET__connex_free_list;
   while (MET_NULL_CONNEX != connex) {
      connex = connex->mminfo.next;
   }
   data = MET__data_free_list;
   while (MET_NULL_DATA != data) {
      data = data->mminfo.next;
   }
   verts = MET__verts_free_list;
   while (MET_NULL_VERTS != verts) {
      verts = verts->mminfo.next;
   }
   env = MET__env_free_list;
   while (MET_NULL_ENV != env) {
      env = env->mminfo.next;
   }
}

/*
 * this gets called (if DEBUG is defined)
 * every time a function is called (except itself, of
 * course).  It checks all the global free lists for consistency.
 *
 * note that because we don't have a MME(), we can't use FO.
 */
void
MET_periodic()
{
   static int entrance_count = 0;

   entrance_count++;

   if ((entrance_count % 1000) == 0)
      MET__chase_free_lists();

}

static void
MET__free_chunks()
{
   MET_chunk_header	*chunk, *next;
   int			i, size;
   MME(MET__free_chunks);

   for (i = 0; i < MET__MM_TYPE_MAX; i++) {
      chunk = MET_chunk_list[i];
      size = MET_chunk_size[i] * MET_obj_size[i] +
	 sizeof(MET_chunk_header);
      while (NULL_CHUNK_HEADER != chunk) {
	 next = chunk->next;
	 FREE(chunk, size);
	 chunk = next;
      }
   }
}


void
METexit()
{
   MME(METexit);
   
   MET_type_exit();
   MET_sym_exit();
   MET_set_exit();
   MET_list_exit();
   MET__free_chunks();

#ifdef MET__DEBUG_MAGIC
#ifdef MET__DEBUG_PRINT
   METprint_all_objects(FNULL);
   METprint_memory_stats(FNULL);
   MET_list_hash_total_stats(FNULL);
#endif
#endif
}

/*
 * the error handler should be declared 
 *
 * void
 * handler(FILOCloc *region, char *text, int severity)
 *
 * where region is a best estimate of what caused the error and text is a
 * NULL terminated ascii string describing what is going on, and severity
 * is one of ERR_{INFO,WARNING,SEVERE,FATAL}.
 */
void
METdefaults_set(new_defaults, which)
   METdefaults	*new_defaults;
   int		which;
{
   MME(METdefaults_set);

   switch (which) {
    case MET_DEFAULTS_ERROR_HANDLER:
      MET_defaults->error_handler.handler =
	 new_defaults->error_handler.handler;
      break;

    case MET_DEFAULTS_INTERVAL_RESOLUTION:
      MET_defaults->interval_resolution.resolution =
	 new_defaults->interval_resolution.resolution;
      MET_defaults->interval_resolution.scale =
	 new_defaults->interval_resolution.scale;
      break;

    case MET_DEFAULTS_RELOP_EPSILON:
      MET_defaults->relop_epsilon.epsilon =
	 new_defaults->relop_epsilon.epsilon;
      break;

    case MET_DEFAULTS_COMPLEX_FLAGS:
      MET_defaults->complex_flags.pow =
	 new_defaults->complex_flags.pow;
      MET_defaults->complex_flags.sqrt =
	 new_defaults->complex_flags.sqrt;
      break;

    case MET_DEFAULTS_PREPROCESS:
      MET_defaults->preprocess.on =
	 new_defaults->preprocess.on;
      break;

    case MET_DEFAULTS_TRACE:
      MET_defaults->trace.on = 
	 new_defaults->trace.on;
      break;
      
    case MET_DEFAULTS_CONTROLS:
      MET_defaults->controls.callback =
	 new_defaults->controls.callback;
      MET_defaults->controls.common_data =
	 new_defaults->controls.common_data;
      break;

    case MET_DEFAULTS_OPTIMIZATIONS:
      MET_defaults->optimizations.peephole =
	 new_defaults->optimizations.peephole;
      MET_defaults->optimizations.cse_elimination =
	 new_defaults->optimizations.cse_elimination;
      break;

    default:
      DEFAULT_ERR(which);
   }
}

METdefaults *
METdefaults_get()
{
   MME(METdefaults_get);

   return MET_defaults;
}



/*
 * round num_objects up for malloc efficiency
 * this is OS dependent.  12 allows for malloc's block header.
 */
int
MET_malloc_round_up(num_objects, object_size)
   int		num_objects;
   int		object_size;
{
   int		size, bits_used = 0;
   MME(MET_malloc_round_up);
   
   size = num_objects * object_size;
   /*
    * count # of bits in size.  this is so we can round it
    * up to the nearest power of two.
    */
   while (size) {
      bits_used++;
      size >>= 1;
   }
   size = 1 << bits_used;
   return (size - (12 + sizeof(MET_chunk_header))) / object_size;
}

#ifdef MET__DEBUG_MAGIC
void
METprint_memory_stats(stream)
   FILE		*stream;
{
   static char	*title_format = "%8s%8s%8s%8s%8s%8s%8s%8s%8s%8s%8s%8s\n";
   static char	*row_format   = "%8s%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d\n";
   static char  *total_format = "%8s%16d%16d\n";
   int		total_bytes = 0, total_chunks = 0, i, t;
   extern void	mallocmap();
   MME(METprint_memory_stats);
   
   if (FNULL == stream)
      stream = stdout;

   /*
    * summery of columns;
    *
    * name	- ascii name of the object
    * size	- sizeof that type in bytes
    * chunks	- how many large arrays of them have been mallocked
    * total	- total number of objects in those chunks (grain * chunks)
    * bytes	- total bytes of that object
    *                    (grain * size + sizeof(MET_chunk_header) * chunks)
    * allocs	- how many times MET_XXX_ALLOC() was called
    * copies	- how many times MET_XXX_COPY() was called
    * frees	- how many times MET_XXX_FREE() was called
    * dcopies	- how many times MET_XXX_COPY_DEEP() was called
    * dfrees	- how many times MET_XXX_FREE() resulted in a DEEP free
    * refs	- total references to all objects (allocs + copies - frees)
    * objs	- total objects in use (allocs - dfrees)
    */
   
   PRINT(stream, "\nMET memory statistics\n");
   PRINT(stream, title_format, "name", "size", "chunks", "total", "bytes",
	 "allocs", "copies", "frees", "dcopies", "dfrees", "refs", "objs");
   PRINT(stream, title_format, "----", "----", "------", "-----", "-----",
	 "------", "------", "-----", "-------", "------", "----", "----");
   for (i = 0; i < MET__MM_TYPE_MAX; i++) {
      PRINT(stream, row_format,
	    MET_obj_name[i],
	    MET_obj_size[i],
	    MET_count_chunks[i],
	    MET_count_chunks[i] * MET_chunk_size[i],
	    t = MET_count_chunks[i] *
	    (MET_chunk_size[i] * MET_obj_size[i] + sizeof(MET_chunk_header)),
	    MET_count_allocs[i],
	    MET_count_copies[i],
	    MET_count_frees[i],
	    MET_count_deep_copies[i],
	    MET_count_deep_frees[i],
	    MET_count_allocs[i] + MET_count_copies[i] - MET_count_frees[i],
	    MET_count_allocs[i] - MET_count_deep_frees[i]);
      total_bytes += t;
      total_chunks += MET_count_chunks[i];
   }
   PRINT(stream, title_format, "----", "----", "------", "-----", "-----",
	 "------", "------", "-----", "-------", "------", "----", "----");
   PRINT(stream, total_format, "total", total_chunks, total_bytes);
   
   PRINT(stream, "\n");
   if (CNULL != getenv("MALLOCMAP")) /* Ack! */
      global_print_memory_statistics(stream);
   PRINT(stream, "\n");
}

#define BLOW_CHUNKS(MM_TYPE, TYPE) {	      \
   chunk = MET_chunk_list[MM_TYPE];		       \
   while (NULL_CHUNK_HEADER != chunk) {			\
      first = chunk->first_of_chunk;			 \
      for (i = 0; i < MET_chunk_size[MM_TYPE]; i++)  \
	 if (((TYPE *) first)[i].magic != MET__MAGIC_FREE) \
	    (MET_print[MM_TYPE])(stream, ((TYPE *) first) + i, 0);    \
      chunk = chunk->next;			      	     \
   }							      \
}

void
METprint_all_objects(stream)
   FILE		*stream;
{
   MET_chunk_header	*chunk;
   MET_object		*first;
   int			i;
   MME(METprint_all_objects);
   
   if (FNULL == stream)
      stream = stdout;

   PRINT(stream, "\n");
   PRINT(stream, "All objects in use:\n");

   BLOW_CHUNKS(MET__MM_TYPE_SET,    METset);
   BLOW_CHUNKS(MET__MM_TYPE_TYPE,   METtype);
   BLOW_CHUNKS(MET__MM_TYPE_DATA,   METdata);
   BLOW_CHUNKS(MET__MM_TYPE_VERTS,  METverts);
   BLOW_CHUNKS(MET__MM_TYPE_SYM,    METsym);
   BLOW_CHUNKS(MET__MM_TYPE_BLOCK,  METblock);
   BLOW_CHUNKS(MET__MM_TYPE_CONNEX, METconnex);
   BLOW_CHUNKS(MET__MM_TYPE_LIST,   METlist);
   BLOW_CHUNKS(MET__MM_TYPE_ENV,    METenv);
}

#define BACKTRACE_CHUNKS(MM_TYPE, TYPE) {	      \
   chunk = MET_chunk_list[MM_TYPE];		       \
   while (NULL_CHUNK_HEADER != chunk) {			\
      first = chunk->first_of_chunk;			 \
      for (i = 0; i < MET_chunk_size[MM_TYPE]; i++)       \
	 if (((TYPE *) first)[i].magic != MET__MAGIC_FREE && \
	     (MET_has_ref[MM_TYPE](((TYPE *) first) + i,	\
				   mm_type, object))) { 		\
		(MET_print[MM_TYPE])(stream, ((TYPE *) first) + i, 0); \
		METbacktrace_object(stream,				  \
				    (MET_object *) (((TYPE *) first) + i), \
				    MM_TYPE, levels - 1); 		\
	     }								\
      chunk = chunk->next;			      	     \
   }							      \
}

void
METbacktrace_object(stream, object, mm_type, levels)
   FILE		*stream;
   MET_object	*object;
   int		mm_type;
   int		levels;
{
   MET_chunk_header	*chunk;
   MET_object		*first;
   int			i;
   MME(METbacktrace_object);

   if (0 == levels)
      return;

   if (FNULL == stream)
      stream = stdout;

   PRINT(stream, "backtrace of %s at %#x:\n",
	 MET_obj_name[mm_type], object);

   BACKTRACE_CHUNKS(MET__MM_TYPE_SET,    METset);
   BACKTRACE_CHUNKS(MET__MM_TYPE_TYPE,   METtype);
   BACKTRACE_CHUNKS(MET__MM_TYPE_DATA,   METdata);
   BACKTRACE_CHUNKS(MET__MM_TYPE_VERTS,  METverts);
   BACKTRACE_CHUNKS(MET__MM_TYPE_SYM,    METsym);
   BACKTRACE_CHUNKS(MET__MM_TYPE_BLOCK,  METblock);
   BACKTRACE_CHUNKS(MET__MM_TYPE_CONNEX, METconnex);
   BACKTRACE_CHUNKS(MET__MM_TYPE_LIST,   METlist);
   BACKTRACE_CHUNKS(MET__MM_TYPE_ENV,    METenv);
}

   
#endif MET__DEBUG_MAGIC

#ifdef MET__DEBUG_PRINT
void
MET_indent(stream, indent_level)
   FILE		*stream;
   int		indent_level;
{
   int		i;
   MME(MET_indent);
   
   if (FNULL == stream)
      stream = stdout;

   for (i = 0; i < indent_level; i++)
      PRINT(stream, "|   ");
}

char *MET_names_field_type[] = {
   "ztwo",
   "integer",
   "angle",
   "real",
   "complex",
   "quaternion",
   "map",
   "set",
   "unknown",
};

char *MET_names_env_type[] = {
   "big",
   "little",
   "binding",
};

char *MET_names_sym_type[] = {
   "op",
   "control",
   "variable",
   "set_ref",
   "data",
};

char *MET_names_set_type[] = {
   "unknown",
   "bare",
   "block",
   "symbol",
   "type",
   "symbolic constant",
};

char *MET_names_type_type[] = {
   "unknown",
   "map",
   "pair",
   "vector",
   "set",
   "field",
   "set_of",
   "bogus",
};

#endif MET__DEBUG_PRINT

/*
 * some useful templates:
 */

#if 0
switch (type->type) {
 case MET_TYPE_TYPE_MAP:
   break;
 case MET_TYPE_TYPE_PAIR:
   break;
 case MET_TYPE_TYPE_VECTOR:
   break;
 case MET_TYPE_TYPE_SET:
   break;
 case MET_TYPE_TYPE_FIELD:
   break;
 case MET_TYPE_TYPE_SET_OF:
   break;
 case MET_TYPE_TYPE_UNKNOWN:
   break;
 case MET_TYPE_TYPE_BOGUS:
   break;
 default:
   DEFAULT_ERR(type->type);
}
switch (sym->type) {
 case MET_SYM_TYPE_OP:
   break;
 case MET_SYM_TYPE_CONTROL:
   break;
 case MET_SYM_TYPE_VARIABLE:
   break;
 case MET_SYM_TYPE_SET_REF:
   break;
 default:
   DEFAULT_ERR(sym->type);
}
switch(xxx) {
 case MET_FIELD_ZTWO:
   break;
 case MET_FIELD_INTEGER:
   break;
 case MET_FIELD_REAL:
   break;
 case MET_FIELD_COMPLEX:
   break;
 case MET_FIELD_MAP:
   break;
 case MET_FIELD_SET:
   break;
 default:
   DEFAULT_ERR(xxx);
}
switch(list->mm_type) {
 case MET__MM_TYPE_SET:
   break;
 case MET__MM_TYPE_TYPE:
   break;
 case MET__MM_TYPE_DATA:
   break;
 case MET__MM_TYPE_VERTS:
   break;
 case MET__MM_TYPE_SYM:
   break;
 case MET__MM_TYPE_BLOCK:
   break;
 case MET__MM_TYPE_CONNEX:
   break;
 case MET__MM_TYPE_LIST:
   break;
 case MET__MM_TYPE_ENV:
   break;
 default:
   DEFAULT_ERR(list->mm_type);
}
switch (set->type) {
 case MET_SET_TYPE_UNKNOWN:
   break;
 case MET_SET_TYPE_BARE:
   break;
 case MET_SET_TYPE_BLOCK:
   break;
 case MET_SET_TYPE_SYMBOL:
   break;
 case MET_SET_TYPE_ENV:
   break;
 case MET_SET_TYPE_TYPE:
   break;
 case MET_SET_TYPE_SYM_CONST:
   break;
 default:
   DEFAULT_ERR(set->type);
}
switch (env->type) {
 case MET_ENV_TYPE_BIG:
   break;
 case MET_ENV_TYPE_LITTLE:
   break;
 case MET_ENV_TYPE_BINDING:
   break;
 default:
   DEFAULT_ERR(env->type);
}
#endif
