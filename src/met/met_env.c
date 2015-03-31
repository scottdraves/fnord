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

#include "defs.h"

#ifndef lint
static char Version[] =
   "$Id: met_env.c,v 1.6 1992/06/22 17:04:38 mas Exp $";
#endif

#define ENV_SYMTAB_SIZE 601

void
MET_env_initialize()
{
   MME(MET_env_initialize);
}

static void
MET__env_free_big_binding(sym, set)
   Sym		sym;
   METset	**set;
{
   MME(MET__env_free_big_binding);

   MET_SET_FREE(*set);
}

void
MET__env_free_deep(env)
   METenv	*env;
{
   MME(MET__env_free_deep);
   CHECK_ENV(env);

   switch (env->type) {
    case MET_ENV_TYPE_BIG:
      MET_ENV_FREE_NULL(env->info.big.parent);
      METlist_hash_free_id(env->info.big.id);
      break;

    case MET_ENV_TYPE_LITTLE:
      MET_ENV_FREE_NULL(env->info.little.parent);
      MET_ENV_FREE_NULL(env->info.little.bindings);
      break;

    case MET_ENV_TYPE_BINDING:
      MET_ENV_FREE_NULL(env->info.binding.next);
      MET_SET_FREE(env->info.binding.set);
      break;

    default:
      DEFAULT_ERR(env->type);
   }

   env->mminfo.next = MET__env_free_list;
   MET__env_free_list = env;
#ifdef MET__DEBUG_MAGIC
   env->magic = MET__MAGIC_FREE;
#endif
}


METenv *
MET__env_copy_deep(env)
   METenv	*env;
{
   MME(MET__env_copy_deep);
   CHECK_ENV(env);
   NYET;

   return MET_NULL_ENV;
}

METenv *
METenv_new(type, parent)
   int		type;
   METenv	*parent;
{
   METenv	*baby;
   MME(METenv_new);

   MET_ENV_ALLOC(baby);
   baby->type = type;
   switch (type) {
    case MET_ENV_TYPE_BIG:
      baby->info.big.id = METlist_hash_new_id();
      MET_ENV_COPY_NULL(baby->info.big.parent, parent);
      break;

    case MET_ENV_TYPE_LITTLE:
      baby->info.little.bindings = MET_NULL_ENV;
      MET_ENV_COPY_NULL(baby->info.little.parent, parent);
      break;

    default:
      DEFAULT_ERR(baby->type);
   }

   return baby;
}

/*
 * bind VALUE to SYM in ENV.
 * if OVERRIDE is YES, then the current binding, if any, is replaced.
 * if it is NO, then any current is left intact.
 *
 * NO is returned if there was no previous binding,
 * YES if there was one, regardless of whether it was
 * overriden or not.
 */
Boolean
METenv_add_binding(env, sym, value, override)
   METenv	*env;
   Sym		sym;
   METset	*value;
   Boolean	override;
{
   Boolean	result;
   METenv	*binding, *scan;
   MME(METenv_add_binding);
   CHECK_ENV(env);
   CHECK_SET(value);

   switch (env->type) {
    case MET_ENV_TYPE_BIG:
      if (NO == override)
	 NYET;
      result = METlist_hash_add_set(env->info.big.id, sym, value);
      break;

    case MET_ENV_TYPE_LITTLE:
      /*
       * see if it is already there
       */
      result = NO;
      scan = env->info.little.bindings;
      while (MET_NULL_ENV != scan) {
	 if (SYM_EQ(scan->info.binding.symbol, sym)) {
	    if (YES == override) {
	       MET_SET_FREE(scan->info.binding.set);
	       MET_SET_COPY(scan->info.binding.set, value);
	    }
	    result = YES;
	    break;
	 }
	 scan = scan->info.binding.next;
      }
      if (NO == result) {
	 /*
	  * create it
	  */
	 MET_ENV_ALLOC(binding);
	 binding->type = MET_ENV_TYPE_BINDING;
	 binding->info.binding.symbol = sym;
	 MET_SET_COPY(binding->info.binding.set, value);
	 /*
	  * link it in
	  */
	 MET_ENV_COPY_NULL(binding->info.binding.next,
			   env->info.little.bindings);
	 MET_ENV_FREE_NULL(env->info.little.bindings);
	 MET_ENV_COPY(env->info.little.bindings, binding);
      }
      break;

    default:
      DEFAULT_ERR(env->type);
   }

   return result;
}

/*
 * lookup and return a copy of the value of SYM in ENV.
 * NULL is returned if it is unbound.  ENV may be NULL.
 */
METset *
METenv_lookup(env, sym)
   METenv	*env;
   Sym		sym;
{
   METset	*value;
   MME(METenv_lookup);

   if (MET_NULL_ENV == env)
      return MET_NULL_SET;

   switch (env->type) {
    case MET_ENV_TYPE_BIG:
      if (MET_NULL_SET ==
	  (value = METlist_hash_find_set(env->info.big.id, sym))) {
	 return METenv_lookup(env->info.big.parent, sym);
      } else {
	 return value;
      }

    case MET_ENV_TYPE_LITTLE:
      if (MET_NULL_SET ==
	  (value = METenv_lookup(env->info.little.bindings, sym))) {
	 return METenv_lookup(env->info.little.parent, sym);
      } else {
	 return value;
      }

    case MET_ENV_TYPE_BINDING:
      if (SYM_EQ(sym, env->info.binding.symbol)) {
	 MET_SET_COPY(value, env->info.binding.set);
	 return value;
      } else
	 return METenv_lookup(env->info.binding.next, sym);
      break;

    default:
      DEFAULT_ERR(env->type);
   }

   /* make lint happy */
   return MET_NULL_SET;
}
      

#define FUNC MET__env_new_free_list
#define LIST MET__env_free_list
#define TYPE METenv
#define MM_TYPE MET__MM_TYPE_ENV
#include "new_free_list.c"
#undef FUNC
#undef LIST
#undef TYPE
#undef MM_TYPE

#ifdef MET__DEBUG_PRINT
void
METenv_print(stream, env, indent_level)
   FILE		*stream;
   METenv	*env;
   int		indent_level;
{
   MME(METenv_print);
   CHECK_NULL_ENV(env);

   if (FNULL == stream)
      stream = stdout;

   MET_indent(stream, indent_level);
   if (MET_NULL_ENV == env) {
      PRINT(stream, "env is null\n");
      return;
   }
   PRINT(stream,
	 "env at %#x: type = %s, ref_count = %d, ",
	 env, MET_names_env_type[env->type], env->mminfo.ref_count);
   
   switch (env->type) {
    case MET_ENV_TYPE_BIG:
      PRINT(stream, "parent = %#x\n", env->info.big.parent);
      MET_list_hash_print(stream, env->info.big.id);
      break;

    case MET_ENV_TYPE_LITTLE:
      PRINT(stream, "parent = %#x\n", env->info.little.parent);
      METenv_print(stream, env->info.little.bindings, indent_level + 1);
      break;

    case MET_ENV_TYPE_BINDING:
      PRINT(stream, "symbol = %s, set = %#x\n",
	    SYM_STRING(env->info.binding.symbol),
	    env->info.binding.set);
      METenv_print(stream, env->info.binding.next, indent_level);
      break;
    default:
      DEFAULT_ERR(env->type);
   }
      
}


Boolean
MET__env_has_ref(env, mm_type, object)
   METenv	*env;
   int		mm_type;
   MET_object	*object;
{
   MME(MET__env_has_ref);

   switch(mm_type) {
    case MET__MM_TYPE_SET:
    case MET__MM_TYPE_TYPE:
    case MET__MM_TYPE_DATA:
    case MET__MM_TYPE_VERTS:
    case MET__MM_TYPE_SYM:
    case MET__MM_TYPE_BLOCK:
    case MET__MM_TYPE_CONNEX:
    case MET__MM_TYPE_LIST:
      break;

    default:
      DEFAULT_ERR(mm_type);
   }

   return NO;
}
#endif MET__DEBUG_PRINT
