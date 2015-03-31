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

/* -------------------------------------------------------------------------
		       Public MET include file
   ------------------------------------------------------------------------- */

/* $Id: met.h,v 1.25 1992/06/22 17:04:27 mas Exp $ */

#ifndef MET_HAS_BEEN_INCLUDED
#define MET_HAS_BEEN_INCLUDED

#include "global/global.h"
#include "filoc/filoc.h"
#include "sym/sym.h"

#include <sys/types.h>
#include <math.h>

#ifdef DEBUG
#define MET__DEBUG
#endif

#define MET__DEBUG_PRINT
#ifdef MET__DEBUG
#define MET__DEBUG_MAGIC
#define MET__DEBUG_TRACE
#define MET__DEBUG_MEMORY
#endif

/*
 * Naming Conventions
 *
 * name			type		declared in
 * =========		=====		========
 * METobj_verb()	public		met.h
 * MET_obj_verb()	private		defs.h
 * MET__obj_verb()	static		obj.c
 * MET_OBJ_VERB		public		met.h
 * OBJ_VERB		private		defs.h
 * VERB			static		obj.c
 *
 * constants which fit in a particular field of an object should be named
 * MET_OBJ_FIELD_DESCRIPT
 *
 * some routines, although they are private (resp static), are declared in
 * met.h (resp defs.h or met.h).  This is because they may be needed for other
 * declarations, or they might be used in a macro at that level. eg
 * MET__MAX_SYM_KIDS is declared in met.h because it is needed to set the
 * sizes of arrays in public data structures.
 *
 * sometimes the conventions are broken in the name of consistency.  All data
 * types are declared globally, as are their memory management macros.
 */

/* -----------------------------  Constants  ------------------------------ */

/*
 * change the debugging string tables in METglobals.c if you change any of
 * these constant groups.
 */

/*
 * fundamental fields supported, also METverts->type values
 */
#define MET_FIELD_ZTWO			0
#define MET_FIELD_INTEGER		1
#define MET_FIELD_ANGLE			2
#define MET_FIELD_REAL			3
#define MET_FIELD_COMPLEX		4
#define MET_FIELD_QUATERNION		5
#define MET_FIELD_MAP			6
#define MET_FIELD_SET			7
#define MET_FIELD_UNKNOWN		8
#define MET_FIELD_MAX			9

#define MET_TYPE_TYPE_UNKNOWN		0
#define MET_TYPE_TYPE_MAP		1
#define MET_TYPE_TYPE_PAIR		2
#define MET_TYPE_TYPE_VECTOR		3
#define MET_TYPE_TYPE_SET		4
#define MET_TYPE_TYPE_FIELD		5
#define MET_TYPE_TYPE_SET_OF		6
#define MET_TYPE_TYPE_BOGUS		7

#define MET_SYM_TYPE_OP			0
#define MET_SYM_TYPE_CONTROL		1
#define MET_SYM_TYPE_VARIABLE		2
#define MET_SYM_TYPE_SET_REF		3
#define MET_SYM_TYPE_DATA		4

#define MET_SYM_VARIABLE_ID_FIRST	1
#define MET__MAX_SYM_KIDS		5

#define MET_ENV_TYPE_BIG		0
#define MET_ENV_TYPE_LITTLE		1
#define MET_ENV_TYPE_BINDING		2

#define MET_SET_TYPE_UNKNOWN		0
#define MET_SET_TYPE_BARE		1
#define MET_SET_TYPE_BLOCK		2
#define MET_SET_TYPE_SYMBOL		3
#define MET_SET_TYPE_TYPE		4
#define MET_SET_TYPE_SYM_CONST		5

#define MET_CONNEX_TYPE_MESH		0
#define MET_CONNEX_TYPE_TRIANGLE	1

#define MET_TAG_TYPE_DOUBLE_MAP		0
#define MET_TAG_TYPE_PRODUCT_MAP	1
#define MET_TAG_TYPE_ELEMENT		2
#define MET_TAG_NO_ID			0

#define MET__MM_TYPE_NONE		(-1)
#define MET__MM_TYPE_SET		0
#define MET__MM_TYPE_TYPE		1
#define MET__MM_TYPE_DATA		2
#define MET__MM_TYPE_VERTS		3
#define MET__MM_TYPE_SYM		4
#define MET__MM_TYPE_BLOCK		5
#define MET__MM_TYPE_CONNEX		6
#define MET__MM_TYPE_LIST		7
#define MET__MM_TYPE_ENV		8
#define MET__MM_TYPE_MAX		9

#define MET_NULL_SET			((METset    *) NULL)
#define MET_NULL_TYPE			((METtype   *) NULL)
#define MET_NULL_DATA			((METdata   *) NULL)
#define MET_NULL_VERTS			((METverts  *) NULL)
#define MET_NULL_SYM			((METsym    *) NULL)
#define MET_NULL_BLOCK			((METblock  *) NULL)
#define MET_NULL_CONNEX			((METconnex *) NULL)
#define MET_NULL_LIST			((METlist   *) NULL)
#define MET_NULL_ENV			((METenv    *) NULL)

#define MET_DEFAULTS_ERROR_HANDLER		0
#define MET_DEFAULTS_INTERVAL_RESOLUTION	1
#define MET_DEFAULTS_RELOP_EPSILON		2
#define MET_DEFAULTS_COMPLEX_FLAGS		3
#define MET_DEFAULTS_PREPROCESS			4
#define MET_DEFAULTS_TRACE    			5
#define MET_DEFAULTS_CONTROLS			6
#define MET_DEFAULTS_OPTIMIZATIONS		7

#define MET_CONNEX_STATIC_SIZE			3
#define MET_TAGS_STATIC_SIZE			2

#define MET__MAGIC_FREE			0x5fb02a8dL

#ifdef M_PI
#define MET_CONSTANT_PI      M_PI
#define MET_CONSTANT_E       M_E
#define MET_CONSTANT_SQRT1_2 M_SQRT1_2
#else
#define MET_CONSTANT_PI      3.14159265358979323846
#define MET_CONSTANT_E       2.7182818284590452354
#define MET_CONSTANT_SQRT1_2 0.70710678118654752440
#endif

#define MET_CONTROL_REQUEST_VALUE	0
#define MET_CONTROL_REQUEST_CHANGED	1
#define MET_CONTROL_REQUEST_TYPE	2

#define MET_ACCESS_VERTICES_SPECIAL_NONE	0
#define MET_ACCESS_VERTICES_SPECIAL_TEXT	1
#define MET_ACCESS_VERTICES_SPECIAL_MARKER	2

#define MET_CLEAR_NOT_AT_ALL		0
#define MET_CLEAR_JUST_ONCE		1
#define MET_CLEAR_LATER			2
#define MET_CLEAR_ONE_OF_MANY		3
#define MET_CLEAR_IN_PROGRESS		4

/* ------------------------------  Types  --------------------------------- */


typedef struct METtype      METtype;
typedef struct METdata      METdata;
typedef struct METverts     METverts;
typedef struct METconnex    METconnex;
typedef struct METblock     METblock;
typedef struct METlist	    METlist;
typedef struct METenv	    METenv;
typedef struct METtag	    METtag;
typedef struct METtag_set   METtag_set;
typedef union  MET_object   MET_object;
typedef union  MET_object_p MET_object_p;

typedef struct METcontrol_request METcontrol_request;
typedef struct METdefaults  METdefaults;
typedef struct METsym       METsym;
typedef struct METset       METset;


typedef int		(*METfunc_int)();
typedef void		(*METfunc_void)();
   
/*
 * the different fields
 */
typedef unsigned char	  METztwo;
typedef int		  METinteger;
typedef u_long		  METangle;
typedef Real		  METreal;
typedef struct {
   Real		real, imaginary;
} 		METcomplex;
typedef struct {
   Real		real, i, j, k;
}		METquaternion;
typedef struct {
   METsym	*sym;
   METtype	*loose_from;
   METenv	*env;
} METmap;

union MET_object_p {
   METset		*set;
   METtype		*type;
   METdata		*data;
   METverts		*verts;
   METsym		*sym;
   METblock		*block;
   METconnex		*connex;
   METlist		*list;
   METenv		*env;
};

/*
 * vertices in PLAs, 
 */
typedef union {
   METztwo		  ztwo;
   METinteger		  integer;
   METangle		  angle;
   METreal		  real;
   METcomplex		  complex;
   METquaternion	  quaternion;
   METset		  *set;
   METmap		  map;
}			  METnumber;

struct METtype {
   int			type;		/* one of MET_TYPE_TYPE_XXX */
   union {
      struct {
	 METtype	*from;
	 METtype	*to;
      } 		map;
      struct {
	 METtype	*left;
	 METtype	*right;
      } 		pair;
      struct {
	 METtype	*of;
	 int		dimension;
	 FOboolean	is_row;
      } 		vector;
      struct {
	 int		type;
      } 		field;
      struct {
	 METtype	*of;
      } 		set_of;
   }			info;
   union {
      int		ref_count;
      METtype		*next;
   }			mminfo;
#ifdef MET__DEBUG_MAGIC
   long			magic;
#endif
};

/*
 * the vertices pointer is from malloc()
 */
struct METverts {
   int			type;		/* one of MET_FIELD_XXX */
   int			num_verts;
   int			total_cols;
   METnumber		*vertices;
   union {
      int		ref_count;
      METverts		*next;
   }			mminfo;
#ifdef MET__DEBUG_MAGIC
   long			magic;
#endif
};

/*
 * these refer to a portion of a METverts
 */
struct METdata {
   METdata		*next;
   int			num_cols;
   int			first_col;
   METverts		*verts;
   union {
      int		ref_count;
      METdata		*next;
   }			mminfo;
#ifdef MET__DEBUG_MAGIC
   long			magic;
#endif
};

/*
 * this is an environment.  It is either big (a hash table), little (a
 * list of bindings), or it is a binding.
 */
struct METenv {
   int			type;
   union {
      struct {
	 METenv		*parent;
	 int		id;	/* of virtual hash table */
      }			big;
      struct {
	 METenv		*parent;
	 METenv		*bindings;
      }			little;
      struct {
	 METenv		*next;
	 Sym		symbol;
	 METset		*set;
      }			binding;
   }			info;
   union {
      int		ref_count;
      METenv		*next;
   }			mminfo;
#ifdef MET__DEBUG_MAGIC
   long			magic;
#endif
};



/*
 * Symbolic dags are used in two places: in set definitions, where they are
 * the direct output of the parser, and to represent functions that we want to
 * apply to something.
 *
 * The dag itself contains five types of nodes: apply, op, control ops, 
 * set refs, and variables.  All internal nodes are applies, all leaves are ops or
 * references.  References are to other defined sets.  Ops are really like
 * references, but they refer to built-in functions like + or sin. Control ops
 * are like ops, but they have a built-in parameter.
 *
 * Any node in the definition can have a type tree.  This means that,
 * when the simplification of the tree reaches this node, the type of the
 * set should match the type at this node.
 *
 * Every node in the dag must have a type tree.  When the dag is being
 * used to evaluate a set, it has data too.
 *
 * Every apply has at least one child (two?).
 */
struct METsym {
   short		type;		/* one of MET_SYM_TYPE_XXX */
   short		num_kids;
   METsym		*kids[MET__MAX_SYM_KIDS];
   FILOCloc		*origin;	/* a place in the source code */
   union {
      int	flags;
   }			seen;	/* when traversing as a dag */
   union {
      struct {
	 METset		*set;
      } 		set_ref;
      struct {
	 int		id;
      } 		variable;
      struct {
	 int		code;
      } 		op;
      struct {
	 char		*data;
      } 		control;
#if 0
      struct {
	 int		size;
	 char		*ptr;
      } 		data;
#endif
   }			info;
   union {
      int		ref_count;
      METsym		*next;
   }			mminfo;
#ifdef MET__DEBUG_MAGIC
   long			magic;
#endif
};

/*
 * identitfy the origin and size of a polybare.  ID is the unique
 * identifier of some apply.  SIZE is the number of elements in the
 * collection at the time of that apply.  The current size of the
 * collection is stored in HOW_MANY (in METset) and is the product
 * of the sizes of all the tags.
 *
 * Finally type indicates the kind of resolution the tag wants.
 * (Either double map or outer product map.)
 */

struct METtag {
   int			type;
   int			id;
   int			size;
   int			index;
   FOboolean		scalar;
   METconnex		*connex;
};

struct METtag_set {
   int			num_tags;
   METtag		*tags;
   METtag		static_tags[MET_TAGS_STATIC_SIZE];
};

/*
 * A block stores elements of a block set with the same type.  The type field
 * contains a type tree describing the elements of the block.  The data field
 * has an array of METdata's that have the pointers to the array of elements
 * of the set.  Each row of the array of elements is one element of the set,
 * these are called vertices.  The METconnex lets some of the elements
 * represent many others, eg a block of type real with vertices 1.0, 2.0, and
 * 3.0 might represent the set {1.0} U [2.0, 3.0].
 *
 * In order to track the "set of similar functions" optimization where a
 * set of functions with the same dag are stored as one dag with polybare
 * constants, we store a list of tags here.
 */
struct METblock {
   METblock		*next;
   METtype		*type;
   METdata		*data;
   METconnex		*connex;
   METconnex		*data_connex;
   METtag_set		tags_part;
   METtag_set		tags_in;
   union {
      int		ref_count;
      METblock		*next;
   }			mminfo;
#ifdef MET__DEBUG_MAGIC
   long			magic;
#endif
};

struct METconnex {
   int			ndims;		/* number of dimensions */
   int			*dims;		/* ptr to dimension sizes */
   int 			*types;   	/* ptr to dimension types */
   /* static array of dims and types for small ndims */
   int			static_dims[MET_CONNEX_STATIC_SIZE];
   int			static_types[MET_CONNEX_STATIC_SIZE];
   union {
      int		ref_count;
      METconnex		*next;
   }			mminfo;
#ifdef MET__DEBUG_MAGIC
   long			magic;
#endif
};

struct METlist {
   METlist		*next;
   int			id;
   long			key;
   union {
      FOboolean		b;
      int		i;
      long		l;
      char		*ptr;
      Sym		sym;
   }			data;
   short		mm_type;
   short		aux_mm_type;
   MET_object_p		object;
   MET_object_p		aux_object;
   union {
      int		ref_count;
      METlist		*next;
   }			mminfo;
#ifdef MET__DEBUG_MAGIC
   long			magic;
#endif
};


struct METset {
   int			type;
   METsym		*definition;
   METlist		*dep_list;
   METenv		*env;
   union {
      struct {
	 METtype	*type;
	 METdata	*data;
	 int		how_many;
	 METtag_set	tags_bare;
	 METtag_set	tags_in;
      }			bare;
      struct {
	 METblock	*block_list;
      }			block;
      struct {
	 Sym		sym;
      }			symbol;
      struct {
	 Sym		sym;
      }			sym_const;
      struct {
	 METenv		*env;
      }			env;
      struct {
	 METtype	*type;
      }			type;
   }			info;
   union {
      int		ref_count;
      METset		*next;
   }			mminfo;
#ifdef MET__DEBUG_MAGIC
   long			magic;
#endif
};

union MET_object {
   METset		set;
   METtype		type;
   METdata		data;
   METverts		verts;
   METsym		sym;
   METblock		block;
   METconnex		connex;
   METlist		list;
};

struct METdefaults {
   struct {
      void		(*handler)();
   } error_handler;
   struct {
      int		resolution;
      Real		scale;
   } interval_resolution;
   struct {
      Real		epsilon;
   } relop_epsilon;
   struct {
      FOboolean		pow;
      FOboolean		sqrt;
   } complex_flags;
   struct {
      FOboolean		on;
   } preprocess;
   struct {
      FOboolean		on;
   } trace;
   struct {
      FOboolean		(*callback)();
      char		*common_data;
   } controls;
   struct {
      FOboolean		peephole;
      FOboolean		cse_elimination;
   } optimizations;
};

struct METcontrol_request {
   int		code;
   char		*common_data;
   char		*data;
   FOboolean	changed;	/* YES/NO */
   METset	*value;
   METtype	*type;
};

/* ------------------------------  Macros  -------------------------------- */

/*
 * access to MET base fields.
 * they take pointers for input and output.
 */
#define MET_PUT_ZTWO(DEST, I)    { *(DEST) = ((METztwo)((*I) != 0)); }
#define MET_PUT_INTEGER(DEST, I) { *(DEST) = ((METinteger)(*I)); }
#define MET_PUT_REAL(DEST, R)    { *(DEST) = ((METreal)(*R)); }
#define MET_PUT_COMPLEX(DEST, R1, R2) { \
   (DEST)->real = ((Real)(*R1));	 \
   (DEST)->imaginary = ((Real)(*R2));	  \
}

#define MET_GET_ZTWO(I, M)    { *(I) = ((int)(*M)); }
#define MET_GET_INTEGER(I, M) { *(I) = ((int)(*M)); }
#define MET_GET_REAL(R, M)    { *(R) = ((Real)(*M); }
#define MET_GET_COMPLEX(R1, R2, M) { \
   *(R1) = ((Real)((M)->real));     \
   *(R2) = ((Real)((M)->complex));   \
}

/*
 * general list based memory management macros
 */
#ifdef MET__DEBUG_MAGIC
#define MET__OBJ_COPY(PTR, OBJ, MM_TYPE) { \
   (OBJ)->mminfo.ref_count++;      \
   (PTR) = (OBJ);		    \
   MET_count_copies[MM_TYPE]++;      \
}
#define MET__OBJ_COPY_DEEP(PTR, OBJ, FUNC, MM_TYPE) { \
   (PTR) = (FUNC)(OBJ);        \
   MET_count_deep_copies[MM_TYPE]++;      \
}
#define MET__OBJ_ALLOC(PTR, LIST, MM_TYPE) {\
   if ((LIST) == NULL)		             \
      (MET_new_free_list[MM_TYPE])();	      \
   (PTR) = (LIST);			       \
   (LIST) = (LIST)->mminfo.next;	        \
   (PTR)->mminfo.ref_count = 1;		         \
   (PTR)->magic = MET_magic[MM_TYPE];		  \
   MET_count_allocs[MM_TYPE]++;			   \
}
#define MET__OBJ_FREE(OBJ, MM_TYPE) {   \
   if (MET__MAGIC_FREE == (OBJ)->magic) { \
      (void) printf("OBJ has already been freed.\n"); \
      abort(); \
   } \
  MET_count_frees[MM_TYPE]++;	   \
   if (--(OBJ)->mminfo.ref_count == 0) { \
      (MET_free_deep[MM_TYPE])(OBJ);	  \
      MET_count_deep_frees[MM_TYPE]++;	   \
   }					     \
}
#else
#define MET__OBJ_COPY(PTR, OBJ, MM_TYPE) { \
   (OBJ)->mminfo.ref_count++;      \
   (PTR) = (OBJ);		    \
}
#define MET__OBJ_COPY_DEEP(PTR, OBJ, FUNC, MM_TYPE) { \
   (PTR) = (FUNC)(OBJ);        \
}
#define MET__OBJ_ALLOC(PTR, LIST, MM_TYPE) { \
   if ((LIST) == NULL)		              \
      (MET_new_free_list[MM_TYPE])();	       \
   (PTR) = (LIST);			        \
   (LIST) = (LIST)->mminfo.next;	         \
   (PTR)->mminfo.ref_count = 1;		          \
}
#define MET__OBJ_FREE(OBJ, MM_TYPE) {   \
   if (--(OBJ)->mminfo.ref_count == 0) { \
      (MET_free_deep[MM_TYPE])(OBJ);	  \
   }					   \
}
#endif MET__DEBUG_MAGIC

#define MET__OBJ_COPY_NULL(PTR, OBJ, MM_TYPE) { \
   if (NULL == (OBJ)) {				 \
      (PTR) = NULL;				  \
   } else {					   \
      MET__OBJ_COPY(PTR, OBJ, MM_TYPE);		    \
   }						     \
}
#define MET__OBJ_FREE_NULL(OBJ, MM_TYPE) { \
   if (NULL != (OBJ)) {			    \
      MET__OBJ_FREE(OBJ, MM_TYPE);	     \
   }					      \
}


#define MET_LIST_COPY(PTR, LIST) \
   MET__OBJ_COPY(PTR, LIST, MET__MM_TYPE_LIST)
#define MET_LIST_COPY_NULL(PTR, LIST) \
   MET__OBJ_COPY_NULL(PTR, LIST, MET__MM_TYPE_LIST)
#define MET_LIST_COPY_DEEP(PTR, LIST) \
   MET__OBJ_COPY_DEEP(PTR, LIST, MET__list_copy_deep, MET__MM_TYPE_LIST)
#define MET_LIST_ALLOC(PTR) \
   MET__OBJ_ALLOC(PTR, MET__list_free_list, MET__MM_TYPE_LIST)
#define MET_LIST_FREE(LIST) \
   MET__OBJ_FREE(LIST, MET__MM_TYPE_LIST)
#define MET_LIST_FREE_NULL(LIST) \
   MET__OBJ_FREE_NULL(LIST, MET__MM_TYPE_LIST)

#define MET_SET_COPY(PTR, SET) \
   MET__OBJ_COPY(PTR, SET, MET__MM_TYPE_SET)
#define MET_SET_COPY_NULL(PTR, SET) \
   MET__OBJ_COPY_NULL(PTR, SET, MET__MM_TYPE_SET)
#define MET_SET_COPY_DEEP(PTR, SET) \
   MET__OBJ_COPY_DEEP(PTR, SET, MET__set_copy_deep, MET__MM_TYPE_SET)
#define MET_SET_ALLOC(PTR) \
   MET__OBJ_ALLOC(PTR, MET__set_free_list, MET__MM_TYPE_SET)
#define MET_SET_FREE(SET) \
   MET__OBJ_FREE(SET, MET__MM_TYPE_SET)
#define MET_SET_FREE_NULL(SET) \
   MET__OBJ_FREE_NULL(SET, MET__MM_TYPE_SET)

#define MET_TYPE_COPY(PTR, TYPE) \
   MET__OBJ_COPY(PTR, TYPE, MET__MM_TYPE_TYPE)
#define MET_TYPE_COPY_NULL(PTR, TYPE) \
   MET__OBJ_COPY_NULL(PTR, TYPE, MET__MM_TYPE_TYPE)
#define MET_TYPE_COPY_DEEP(PTR, TYPE) \
   MET__OBJ_COPY_DEEP(PTR, TYPE, MET__type_copy_deep, MET__MM_TYPE_TYPE)
#define MET_TYPE_ALLOC(PTR) \
   MET__OBJ_ALLOC(PTR, MET__type_free_list, MET__MM_TYPE_TYPE)
#define MET_TYPE_FREE(TYPE) \
   MET__OBJ_FREE(TYPE, MET__MM_TYPE_TYPE)
#define MET_TYPE_FREE_NULL(TYPE) \
   MET__OBJ_FREE_NULL(TYPE, MET__MM_TYPE_TYPE)

#define MET_DATA_COPY(PTR, DATA) \
   MET__OBJ_COPY(PTR, DATA, MET__MM_TYPE_DATA)
#define MET_DATA_COPY_NULL(PTR, DATA) \
   MET__OBJ_COPY_NULL(PTR, DATA, MET__MM_TYPE_DATA)
#define MET_DATA_COPY_DEEP(PTR, DATA) \
   MET__OBJ_COPY_DEEP(PTR, DATA, MET__data_copy_deep, MET__MM_TYPE_DATA)
#define MET_DATA_ALLOC(PTR) \
   MET__OBJ_ALLOC(PTR, MET__data_free_list, MET__MM_TYPE_DATA)
#define MET_DATA_FREE(DATA) \
   MET__OBJ_FREE(DATA, MET__MM_TYPE_DATA)
#define MET_DATA_FREE_NULL(DATA) \
   MET__OBJ_FREE_NULL(DATA, MET__MM_TYPE_DATA)

#define MET_VERTS_COPY(PTR, VERTS) \
   MET__OBJ_COPY(PTR, VERTS, MET__MM_TYPE_VERTS)
#define MET_VERTS_COPY_NULL(PTR, VERTS) \
   MET__OBJ_COPY_NULL(PTR, VERTS, MET__MM_TYPE_VERTS)
#define MET_VERTS_COPY_DEEP(PTR, VERTS) \
   MET__OBJ_COPY_DEEP(PTR, VERTS, MET__verts_copy_deep, MET__MM_TYPE_VERTS)
#define MET_VERTS_ALLOC(PTR) \
   MET__OBJ_ALLOC(PTR, MET__verts_free_list, MET__MM_TYPE_VERTS)
#define MET_VERTS_FREE(VERTS) \
   MET__OBJ_FREE(VERTS, MET__MM_TYPE_VERTS)
#define MET_VERTS_FREE_NULL(VERTS) \
   MET__OBJ_FREE_NULL(VERTS, MET__MM_TYPE_VERTS)

#define MET_SYM_COPY(PTR, SYM) \
   MET__OBJ_COPY(PTR, SYM, MET__MM_TYPE_SYM)
#define MET_SYM_COPY_NULL(PTR, SYM) \
   MET__OBJ_COPY_NULL(PTR, SYM, MET__MM_TYPE_SYM)
#define MET_SYM_COPY_DEEP(PTR, SYM) \
   MET__OBJ_COPY_DEEP(PTR, SYM, MET__sym_copy_deep, MET__MM_TYPE_SYM)
#define MET_SYM_ALLOC(PTR) \
   MET__OBJ_ALLOC(PTR, MET__sym_free_list, MET__MM_TYPE_SYM)
#define MET_SYM_FREE(SYM) \
   MET__OBJ_FREE(SYM, MET__MM_TYPE_SYM)
#define MET_SYM_FREE_NULL(SYM) \
   MET__OBJ_FREE_NULL(SYM, MET__MM_TYPE_SYM)

#define MET_BLOCK_COPY(PTR, BLOCK) \
   MET__OBJ_COPY(PTR, BLOCK, MET__MM_TYPE_BLOCK)
#define MET_BLOCK_COPY_NULL(PTR, BLOCK) \
   MET__OBJ_COPY_NULL(PTR, BLOCK, MET__MM_TYPE_BLOCK)
#define MET_BLOCK_COPY_DEEP(PTR, BLOCK) \
   MET__OBJ_COPY_DEEP(PTR, BLOCK, MET__block_copy_deep, MET__MM_TYPE_BLOCK)
#define MET_BLOCK_ALLOC(PTR) \
   MET__OBJ_ALLOC(PTR, MET__block_free_list, MET__MM_TYPE_BLOCK)
#define MET_BLOCK_FREE(BLOCK) \
   MET__OBJ_FREE(BLOCK, MET__MM_TYPE_BLOCK)
#define MET_BLOCK_FREE_NULL(BLOCK) \
   MET__OBJ_FREE_NULL(BLOCK, MET__MM_TYPE_BLOCK)

#define MET_CONNEX_COPY(PTR, CONNEX) \
   MET__OBJ_COPY(PTR, CONNEX, MET__MM_TYPE_CONNEX)
#define MET_CONNEX_COPY_NULL(PTR, CONNEX) \
   MET__OBJ_COPY_NULL(PTR, CONNEX, MET__MM_TYPE_CONNEX)
#define MET_CONNEX_COPY_DEEP(PTR, CONNEX) \
   MET__OBJ_COPY_DEEP(PTR, CONNEX, MET__connex_copy_deep, MET__MM_TYPE_CONNEX)
#define MET_CONNEX_ALLOC(PTR) \
   MET__OBJ_ALLOC(PTR, MET__connex_free_list, MET__MM_TYPE_CONNEX)
#define MET_CONNEX_FREE(CONNEX) \
   MET__OBJ_FREE(CONNEX, MET__MM_TYPE_CONNEX)
#define MET_CONNEX_FREE_NULL(CONNEX) \
   MET__OBJ_FREE_NULL(CONNEX, MET__MM_TYPE_CONNEX)

#define MET_ENV_COPY(PTR, ENV) \
   MET__OBJ_COPY(PTR, ENV, MET__MM_TYPE_ENV)
#define MET_ENV_COPY_NULL(PTR, ENV) \
   MET__OBJ_COPY_NULL(PTR, ENV, MET__MM_TYPE_ENV)
#define MET_ENV_COPY_DEEP(PTR, ENV) \
   MET__OBJ_COPY_DEEP(PTR, ENV, MET__env_copy_deep, MET__MM_TYPE_ENV)
#define MET_ENV_ALLOC(PTR) \
   MET__OBJ_ALLOC(PTR, MET__env_free_list, MET__MM_TYPE_ENV)
#define MET_ENV_FREE(ENV) \
   MET__OBJ_FREE(ENV, MET__MM_TYPE_ENV)
#define MET_ENV_FREE_NULL(ENV) \
   MET__OBJ_FREE_NULL(ENV, MET__MM_TYPE_ENV)


/* ------------------------------  Entries  ------------------------------- */

/* globals */
extern void		 METinitialize();
extern void		 METexit();
extern void		 METdefaults_set();
extern METdefaults	*METdefaults_get();
extern void		 METprint_memory_stats();
extern void		 MET_periodic();
#ifdef MET__DEBUG_TRACE
extern void		 METtron();
extern void		 METtroff();
#endif

extern METlist		*MET__list_free_list;
extern METset		*MET__set_free_list;
extern METtype		*MET__type_free_list;
extern METblock		*MET__block_free_list;
extern METsym		*MET__sym_free_list;
extern METdata		*MET__data_free_list;
extern METconnex	*MET__connex_free_list;
extern METverts		*MET__verts_free_list;
extern METenv		*MET__env_free_list;

#ifdef MET__DEBUG_MAGIC
extern int		 MET_count_allocs[MET__MM_TYPE_MAX];
extern int		 MET_count_copies[MET__MM_TYPE_MAX];
extern int		 MET_count_deep_copies[MET__MM_TYPE_MAX];
extern int		 MET_count_frees[MET__MM_TYPE_MAX];
extern int		 MET_count_deep_frees[MET__MM_TYPE_MAX];
extern int		 MET_count_chunks[MET__MM_TYPE_MAX];
extern long		 MET_magic[MET__MM_TYPE_MAX];
#endif MET__DEBUG_MAGIC

extern void	       (*MET_new_free_list[MET__MM_TYPE_MAX])();
extern void	       (*MET_free_deep[MET__MM_TYPE_MAX])();
extern void	       (*MET_copy_deep[MET__MM_TYPE_MAX])();
#ifdef MET__DEBUG
extern char		*MET_obj_name[MET__MM_TYPE_MAX];
#endif

/* set */
extern METset		*METset_new_with_definition();
extern void		 METset_redefine();
extern METset		*METset_new_string();
extern METset		*METset_new_number();
extern METset		*METset_new_tensor();
extern FOboolean	 METset_get_number();
extern FOboolean	 METset_get_symbol();
extern METset		*METset_assoc();
extern FOboolean	 METset_get_string();
extern METtype		*METset_get_type();
extern int		 METset_clear_rep();
extern FOboolean	 METset_have_rep();
extern METconnex	*METset_access_vertices();

/* list */
extern FOboolean	 METlist_hash_find_int();
extern METset		*METlist_hash_find_set();
extern METsym		*METlist_hash_find_sym();
extern void		 METlist_hash_add_int();
extern FOboolean	 METlist_hash_add_set();
extern FOboolean	 METlist_hash_add_sym();
extern int		 METlist_hash_new_id();
extern void		 METlist_hash_free_id();

/* sym */
extern void		 METsym_add_kid();
extern METsym		*METsym_new_sym();
extern METsym		*METsym_new_op();
extern METsym		*METsym_new_set_ref();
extern METsym		*METsym_new_control();
extern METset		*METset_form_pair();	/* ugh */
extern METsym		*METsym_new_zero();
extern METset           *METsym_new_sym_const_set();

/* type */

/* data */

/* block */

/* env */
extern METenv		*METenv_new();
extern FOboolean	 METenv_add_binding();
extern METset		*METenv_lookup();

/* printing */
extern void		 METset_print_pretty();
extern void		 METdata_print_pretty();
extern void		 METblock_print_pretty();
extern void		 METtype_print_pretty();
#ifdef MET__DEBUG_PRINT
extern void		 METsym_print();
extern void		 METtype_print();
extern void		 METdata_print();
extern void		 METblock_print();
extern void		 METlist_print();
extern void		 METset_print();
extern void		 METverts_print();
extern void		 METconnex_print();
extern void		 METenv_print();

extern void		 METprint_all_objects();
#endif MET__DEBUG_PRINT

/* reading and writing */
extern FOboolean	METtype_write();
extern METtype          *METtype_read();
extern FOboolean        METdata_write();
extern METdata          *METdata_read();
extern FOboolean	METblock_write();
extern METblock         *METblock_read();
extern FOboolean        METset_write();
extern METset           *METset_read();
extern FOboolean        METconnex_write();
extern METconnex        *METconnex_read();
extern MET_object_p     MET_list_read();

#endif MET_HAS_BEEN_INCLUDED
