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

/* $Id: defs.h,v 1.29 1992/06/22 17:04:24 mas Exp $ */

#ifndef METDEFS_HAS_BEEN_INCLUDED
#define METDEFS_HAS_BEEN_INCLUDED

#include "met/met.h"
#include "err/err.h"

/* The X include files define this already.  
   ALWAYS include this file AFTER X11/Intrinsic.h! */

#ifndef _XtIntrinsic_h
typedef FOboolean Boolean;
#endif

/* -----------------------------  Constants  ------------------------------ */

#define TYPE_DIMENSION_UNKNOWN	(-1)
#define SYM_VARIABLE_ID_PARM	(MET_SYM_VARIABLE_ID_FIRST - 1)
#define SYM_VARIABLE_ID_NONE	(MET_SYM_VARIABLE_ID_FIRST - 2)

#if SYM_VARIABLE_ID_PARM < 0
SYM_VARIABLE_ID_PARM must be non-negative
#endif

#define RNULL			((FILOCloc *) NULL)
#define BNULL			((Boolean *) NULL)
#define NULL_CHUNK_HEADER	((MET_chunk_header *) NULL)

#define TYPE_CAST_TAKE_FIRST	0
#define TYPE_CAST_TAKE_SECOND	1
#define TYPE_CAST_MAKE_BOGUS	2

/* ------------------------------  Types   -------------------------------- */

typedef struct MET_chunk_header MET_chunk_header;

struct MET_chunk_header {
   MET_chunk_header	*next;
   MET_object		*first_of_chunk;
};
   

/* ------------------------------  Macros  -------------------------------- */

#ifdef MET__DEBUG_TRACE
#define MME(NAME)			\
   ME(NAME);				\
   MET_periodic();
#else /* undef(MET__DEBUG_TRACE) */
#define MME(NAME)	ME(NAME);
#endif /* defined(MET__DEBUG_TRACE) */

#define ERROR(REGION, TEXT, SEVERITY) \
   (MET_defaults->error_handler.handler)(REGION, TEXT, SEVERITY)

#define SEV_FATAL 0
#define SEV_WARNING 1

#define TAG_COPY(DEST, SRC) {\
   (DEST).type = (SRC).type;  \
   (DEST).id   = (SRC).id  ;    \
   (DEST).scalar = (SRC).scalar; \
   (DEST).size = (SRC).size;      \
   (DEST).index = (SRC).index;     \
   MET_CONNEX_COPY((DEST).connex, (SRC).connex); }

#define NUMBER_COPY(DEST, SRC, FIELD) {\
   switch(FIELD) {\
    case MET_FIELD_ZTWO: \
      * (METztwo *)(DEST) = * (METztwo *)(SRC); \
      break; \
    case MET_FIELD_INTEGER: \
      * (METinteger *)(DEST) = * (METinteger *)(SRC); \
      break; \
    case MET_FIELD_ANGLE: \
      * (METangle *)(DEST) = * (METangle *)(SRC); \
      break; \
    case MET_FIELD_REAL: \
      * (METreal *)(DEST) = * (METreal *)(SRC); \
      break; \
    case MET_FIELD_COMPLEX: \
      * (METcomplex *)(DEST) = * (METcomplex *)(SRC); \
      break; \
    case MET_FIELD_QUATERNION: \
      * (METquaternion *)(DEST) = * (METquaternion *)(SRC); \
      break; \
    case MET_FIELD_MAP: \
      MET_SYM_COPY(* (METsym **)(DEST), * (METsym **)(SRC)); \
      break; \
    case MET_FIELD_SET: \
      MET_SET_COPY(* (METset **)(DEST), * (METset **)(SRC)); \
      break; \
    default: \
      DEFAULT_ERR(FIELD); \
   } \
}


#define MET_TYPE_NEW_MAP(DEST, FROM, TO) {   \
   MET_TYPE_ALLOC(DEST);		      \
   (DEST)->type = MET_TYPE_TYPE_MAP;	       \
   MET_TYPE_COPY((DEST)->info.map.to, (TO));    \
   MET_TYPE_COPY((DEST)->info.map.from, (FROM)); \
}
#define MET_TYPE_NEW_PAIR(DEST, LEFT, RIGHT) { \
   MET_TYPE_ALLOC(DEST);			\
   (DEST)->type = MET_TYPE_TYPE_PAIR;	         \
   MET_TYPE_COPY((DEST)->info.pair.left, (LEFT));  \
   MET_TYPE_COPY((DEST)->info.pair.right, (RIGHT)); \
}
#define MET_TYPE_NEW_SET_OF(DEST, OF) {      \
   MET_TYPE_ALLOC(DEST);		      \
   (DEST)->type = MET_TYPE_TYPE_SET_OF;	       \
   MET_TYPE_COPY((DEST)->info.set_of.of, (OF)); \
}
#define MET_TYPE_NEW_VECTOR(DEST, OF, DIM, IS_ROW) { \
   MET_TYPE_ALLOC(DEST);		              \
   (DEST)->type = MET_TYPE_TYPE_VECTOR;	    	       \
   MET_TYPE_COPY((DEST)->info.vector.of, (OF)); 	\
   (DEST)->info.vector.dimension = (DIM);        	 \
   (DEST)->info.vector.is_row = (IS_ROW);        	  \
}

#define MET_TYPE_NEW_UNKNOWN(DEST) MET_TYPE_COPY((DEST), MET__type_unknown)
#define MET_TYPE_NEW_BOGUS(DEST)   MET_TYPE_COPY((DEST), MET__type_bogus)
#define MET_TYPE_NEW_SET(DEST)     MET_TYPE_COPY((DEST), MET__type_set)

#define MET_TYPE_NEW_FIELD(DEST, FIELD) \
   MET_TYPE_COPY((DEST), MET__type_field[FIELD])

/* etc.  add the others as needed */


#define CHECK_PTR(PTR)	     ALLOC_CHECK_PTR(PTR)

#define CHECK_NULL_PTR(PTR)  ALLOC_CHECK_NULL_PTR(PTR)

#define CHECK_TYPE(TYPE)     ALLOC_CHECK_MAGIC((TYPE), \
					 MET_magic[MET__MM_TYPE_TYPE])
#define CHECK_DATA(DATA)     ALLOC_CHECK_MAGIC((DATA), \
					 MET_magic[MET__MM_TYPE_DATA])
#define CHECK_SYM(SYM)       ALLOC_CHECK_MAGIC((SYM), \
					 MET_magic[MET__MM_TYPE_SYM])
#define CHECK_CONNEX(CONNEX) ALLOC_CHECK_MAGIC((CONNEX), \
					 MET_magic[MET__MM_TYPE_CONNEX])
#define CHECK_BLOCK(BLOCK)   ALLOC_CHECK_MAGIC((BLOCK), \
					 MET_magic[MET__MM_TYPE_BLOCK])
#define CHECK_SET(SET)       ALLOC_CHECK_MAGIC((SET), \
					 MET_magic[MET__MM_TYPE_SET])
#define CHECK_LIST(LIST)     ALLOC_CHECK_MAGIC((LIST), \
					 MET_magic[MET__MM_TYPE_LIST])
#define CHECK_VERTS(VERTS)   ALLOC_CHECK_MAGIC((VERTS), \
					 MET_magic[MET__MM_TYPE_VERTS])
#define CHECK_ENV(ENV)       ALLOC_CHECK_MAGIC((ENV), \
					 MET_magic[MET__MM_TYPE_ENV])

#define CHECK_NULL_TYPE(TYPE)     ALLOC_CHECK_NULL_MAGIC((TYPE), \
					 MET_magic[MET__MM_TYPE_TYPE])
#define CHECK_NULL_DATA(DATA)     ALLOC_CHECK_NULL_MAGIC((DATA), \
					 MET_magic[MET__MM_TYPE_DATA])
#define CHECK_NULL_SYM(SYM)       ALLOC_CHECK_NULL_MAGIC((SYM), \
					 MET_magic[MET__MM_TYPE_SYM])
#define CHECK_NULL_CONNEX(CONNEX) ALLOC_CHECK_NULL_MAGIC((CONNEX), \
					 MET_magic[MET__MM_TYPE_CONNEX])
#define CHECK_NULL_BLOCK(BLOCK)   ALLOC_CHECK_NULL_MAGIC((BLOCK), \
					 MET_magic[MET__MM_TYPE_BLOCK])
#define CHECK_NULL_SET(SET)       ALLOC_CHECK_NULL_MAGIC((SET), \
					 MET_magic[MET__MM_TYPE_SET])
#define CHECK_NULL_LIST(LIST)     ALLOC_CHECK_NULL_MAGIC((LIST), \
					 MET_magic[MET__MM_TYPE_LIST])
#define CHECK_NULL_VERTS(VERTS)   ALLOC_CHECK_NULL_MAGIC((VERTS), \
					 MET_magic[MET__MM_TYPE_VERTS])
#define CHECK_NULL_ENV(ENV)       ALLOC_CHECK_NULL_MAGIC((ENV), \
					 MET_magic[MET__MM_TYPE_ENV])

#define PRINT (void) fprintf
#define PUTC  (void) putc

#define DATA_FIRST_VERTEX(DATA, TYPE) \
   (CAST(TYPE *, (DATA)->verts->vertices) + (DATA)->first_col)
#define DATA_NUM_VERTS(DATA)  ((DATA)->verts->num_verts)
#define MAX(A,B) ((A) > (B) ? (A) : (B))

#define TYPE_CAST_FIELDS(F1, F2) (MET__type_cast_fields[F1][F2])

#define LIST_HASH_BAD_ID (-1)

#define LIST_HASH_ADD_SYM(LIST, KEY, OBJECT, ID) { \
   MET_LIST_ALLOC(LIST);			    \
   (LIST)->key = (long)(KEY);			     \
   (LIST)->id = (ID);				      \
   (LIST)->mm_type = MET__MM_TYPE_SYM;		       \
   MET_SYM_COPY((LIST)->object.sym, (OBJECT));	        \
   (LIST)->aux_mm_type = MET__MM_TYPE_SYM;		 \
   MET_SYM_COPY((LIST)->aux_object.sym, (KEY));		  \
   MET_list_hash_add(LIST);				   \
}

#define LIST_HASH_FIND_SYM(LIST, KEY, ID) {	       \
   (LIST) = MET_list_hash_find((long) (KEY),		\
			       (ID),			 \
			       MET_list_hash_find_object, \
			       (char *) (KEY));		   \
}

#define MET_OBJECT_P_FREE(OBJECT, MM_TYPE) { \
   switch(MM_TYPE) { \
    case MET__MM_TYPE_NONE:  break; \
    case MET__MM_TYPE_SET: \
      MET_SET_FREE((OBJECT).set);  break; \
    case MET__MM_TYPE_TYPE: \
      MET_TYPE_FREE((OBJECT).type);  break; \
    case MET__MM_TYPE_DATA: \
      MET_DATA_FREE((OBJECT).data);  break; \
    case MET__MM_TYPE_VERTS: \
      MET_VERTS_FREE((OBJECT).verts);  break; \
    case MET__MM_TYPE_SYM: \
      MET_SYM_FREE((OBJECT).sym);  break; \
    case MET__MM_TYPE_BLOCK: \
      MET_BLOCK_FREE((OBJECT).block);  break; \
    case MET__MM_TYPE_CONNEX: \
      MET_CONNEX_FREE((OBJECT).connex);  break; \
    case MET__MM_TYPE_LIST: \
      MET_LIST_FREE((OBJECT).list);  break; \
    case MET__MM_TYPE_ENV: \
      MET_ENV_FREE((OBJECT).env);  break; \
    default: \
      DEFAULT_ERR(MM_TYPE); \
   } \
}


#define UNROLL4(len, st) \
        { \
        register unroll_count = (len + 3) >> 2; \
        if (unroll_count > 0) { \
                switch (len & 3) { \
                        case 0: do { st \
                        case 3:  st \
                        case 2:  st \
                        case 1:  st \
                    } while (--unroll_count) ; \
                } \
        } \
}

#define UNROLL2(len, st) \
        { \
        register unroll_count = (len + 1) >> 1; \
        if (unroll_count > 0) { \
                switch (len & 1) { \
                        case 0: do { st \
                        case 1:  st \
                    } while (--unroll_count) ; \
                } \
        } \
}

/* ------------------------------  Externs  -------------------------------- */

/* globals */
extern void		 MET_indent();
extern int		 MET_chunk_size[MET__MM_TYPE_MAX];
extern int		 MET_obj_size[MET__MM_TYPE_MAX];
extern char		*MET_obj_name[MET__MM_TYPE_MAX];
extern MET_chunk_header	*MET_chunk_list[MET__MM_TYPE_MAX];
extern int		 MET_field_size[MET_FIELD_MAX];
extern METdefaults	*MET_defaults;
extern void    		 MET_error_handler_default();
extern void    		 MET_error_handler_ignore();
#ifdef MET__DEBUG_TRACE
extern void		 MET_periodic();
#endif

/* set */
extern void		 MET_set_initialize();
extern void		 MET_set_exit();
extern void		 MET__set_new_free_list();
extern void		 MET__set_free_deep();
extern METset		*MET__set_copy_deep();
extern Boolean		 MET_set_have_rep();
extern METset		*MET_set_new();
extern void		 MET_set_get_type();
extern METset		*MET_set_extract_row();
extern METset		*MET_set_extract_set_row();
extern METset		*MET_set_extract_tagged_row();
extern void		 MET_set_add_block();
extern METset		*MET_set_cast_to_type();
extern METset		*MET_set_zeroed();
extern METlist		*MET_set_block_to_polybare();
extern void		 MET_set_add_dependency();
extern METset		*MET_set_from_data_and_type();
extern METset		*MET_set_canonize();
extern METdata		*MET_set_to_data();
extern METset 		*MET_set_de_canonize();
extern METset		*MET_set_remove_tag();
extern METset		*MET_set_polybare_merge();
extern METset		*MET_set_zero;
extern METset		*MET_set_one;
extern METset		*MET_set_two;
extern METset		*MET_set_null;

/* list */
extern void		 MET_list_initialize();
extern void		 MET_list_exit();
extern void		 MET__list_new_free_list();
extern void		 MET__list_free_deep();
extern METlist		*MET__list_copy_deep();
extern Boolean		 MET_list_hash_find_object();
extern void		 MET_list_hash_add();
extern METlist		*MET_list_hash_find();
#ifdef MET__DEBUG_PRINT
extern void		 MET_list_hash_stats();
extern void		 MET_list_hash_total_stats();
extern void		 MET_list_hash_print();
#endif

/* sym */
extern void		 MET_sym_initialize();
extern void		 MET_sym_exit();
extern void		 MET__sym_new_free_list();
extern void		 MET__sym_free_deep();
extern METsym		*MET__sym_copy_deep();
extern METsym		*MET_sym_dup_with_kids();
extern void		 MET_sym_force_set_ref();
extern void		 MET_sym_force_to_sym();
extern METset		*MET_sym_be_set_ref();
extern void	         MET_sym_find_dependencies();
extern METsym		*MET_sym_preprocess_definition();
extern void		 MET_sym_clear_seen_flags();
extern METsym		*MET_sym_new_variable();
extern METset		*MET_sym_eval_set_ref();

/* type */
extern void		 MET_type_initialize();
extern void		 MET_type_exit();
extern void		 MET__type_new_free_list();
extern void		 MET__type_free_deep();
extern METtype		*MET__type_copy_deep();
extern Boolean		 MET_type_cast();
extern Boolean		 MET_type_compatible();
extern Boolean		 MET_type_unknown();
extern Boolean		 MET_type_bogus();
extern METset		*MET_type_to_polybare_set();
extern METset		*MET_type_to_block_set();
extern int		 MET_type_base_field();
extern int		 MET_type_count_data();
extern int		 MET_type_traverse_data();
extern METtype		*MET_type_cast_to_field();
extern METtype		*MET_type_cast_maps();
extern METtype		*MET_type_of_type();

extern int		 MET__type_cast_fields[MET_FIELD_MAX][MET_FIELD_MAX];
extern METtype		*MET__type_field[MET_FIELD_MAX];
extern METtype		*MET__type_unknown;
extern METtype		*MET__type_set;
extern METtype		*MET_type_generic_map;
extern METtype		*MET_type_interval;
extern METtype		*MET_type_access_text;
extern METtype		*MET_type_access_marker;

/* data */
extern void		 MET__data_new_free_list();
extern void		 MET__data_free_deep();
extern METdata		*MET__data_copy_deep();
extern METdata		*MET_data_new();
extern METdata		*MET_data_replicate();
extern METdata		*MET_data_extract_row();
extern METdata		*MET_data_extract_at();
extern METdata		*MET_data_copy_upto();
extern METdata		*MET_data_index();
extern void		 MET_data_append();
extern METblock		*MET_data_to_block();
extern METdata		*MET_data_cast_to_type();
extern METdata		*MET_data_copy_very_deep();

/* verts */
extern void		 MET__verts_new_free_list();
extern void		 MET__verts_free_deep();
extern METverts		*MET__verts_copy_deep();
extern METverts		*MET_verts_new();
extern void		 MET_verts_copy_vertices();
extern void		 MET_verts_zero();

/* block */
extern void		 MET__block_new_free_list();
extern void		 MET__block_free_deep();
extern METblock		*MET__block_copy_deep();
extern METset		*MET_block_to_polybare();
extern METblock		*MET_block_lose_tags();

/* connex */
extern void		 MET__connex_new_free_list();
extern void		 MET__connex_free_deep();
extern METconnex	*MET__connex_copy_deep();
extern METconnex	*MET_connex_new();
extern METconnex	*MET_connex_cross();
extern METconnex	*MET_connex_union();
extern int		 MET_connex_num_pts();
extern Boolean		 MET_connex_equal();

/* env */
extern void		 MET__env_new_free_list();
extern void		 MET__env_free_deep();
extern METenv		*MET__env_copy_deep();
extern void		 MET_env_initialize();

/* opt */
extern METsym		*MET_opt_inline();
extern METsym		*MET_opt_remove_constants();
extern METsym		*MET_opt_peephole();
extern METsym		*MET_opt_cse_eliminate();
extern METsym		*MET_opt_whopper();

#ifdef MET__DEBUG_PRINT
extern char		*MET_names_sym_type[];
extern char		*MET_names_type_type[];
extern char		*MET_names_field_type[];
extern char		*MET_names_env_type[];
extern char		*MET_names_set_type[];
#endif

extern Boolean		 MET__set_has_ref();
extern Boolean		 MET__type_has_ref();
extern Boolean		 MET__data_has_ref();
extern Boolean		 MET__verts_has_ref();
extern Boolean		 MET__sym_has_ref();
extern Boolean		 MET__block_has_ref();
extern Boolean		 MET__connex_has_ref();
extern Boolean		 MET__list_has_ref();
extern Boolean		 MET__env_has_ref();

/* tags */
extern void		 MET_tags_initialize();
extern void		 MET_tags_free();
extern void		 MET_tags_add();
extern void		 MET_tags_copy();
extern void 		 MET_tags_copy_new();
extern Boolean		 MET_tags_equal();
extern void		 MET_tags_partialize();

/*LINTLIBRARY*/


#endif METDEFS_HAS_BEEN_INCLUDED
