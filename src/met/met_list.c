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
   "$Id: met_list.c,v 1.12 1992/06/22 17:04:45 mas Exp $";
#endif

/*
 * these are some primes to use for the size of the hash table.  planning
 * for the future...
 */
#define HASH_TABLE_INITIAL_SIZE 2003
static int MET__list_hash_table_sizes[] = {
   HASH_TABLE_INITIAL_SIZE, 4001, 8009, 16001, 32003,
   64007, 128021, 256019, 512009, 1024021, 2048003, 4096013,
   8192003, 16384001, 32768011, 65536043, 131072003};


typedef unsigned long MET__flag_bits;
/*
 * the next few defs support virtual hash tables inside the global table.
 * must be a power of 2.
 */
#define MET__LIST_HASH_INITIAL_NUM_IDS 32


#define MET__LIST_HASH_BITS          (0x20)
#define MET__LIST_HASH_BITS_MASK     (0x1f)
#define MET__LIST_HASH_BITS_SHIFT     5


/*
 * which entry in the above table we are using.  if the table gets full and
 * we increase its size, we increment this.
 */
static int MET__list_hash_size_index = 0;

#define NUM_HASH_TABLE_SIZES \
(sizeof(MET__list_hash_table_sizes)/sizeof(int))

typedef struct {
   METlist    **table;
   int		size;		/* size of table */
   int		num_ids;	/* max # virtual tables */
   Boolean	*id_in_use;	/* which vir.tabs. are active */
   MET__flag_bits *flags;
   int		flag_len;	/* minor index of prev. array */
#if 0
   int		object_count;	/* how many objects are in the table */
   int		full_count;	/* expand table when object_count > */
				/* full_count */
#endif
} MET__list_hash_table;

/*
 * when this many of the entries in the hash table are full,
 * increase its size
 */
#define HASH_TABLE_FULL_PERCENTAGE	0.5

static METlist *MET__list_static_table_table[HASH_TABLE_INITIAL_SIZE];
static MET__list_hash_table MET__list_static_global_hash_table;
static MET__list_hash_table *MET__list_global_hash_table =
   &MET__list_static_global_hash_table;

/*
 * compile with HASH_STATS defined to collect and report statistics on the
 * hash table.
 */
#ifdef HASH_STATS
#define HASH_STATS_COLLECT MET_list_hash_stats(FNULL)
/*
 * averages are only kept when there are objects in the table.
 */
static struct MET__list_hash_stats {
   int		longest;
   int		most_objects;
   int		objects;
   int		entries;
   int		num_samples;
   int		per_entry[HASH_TABLE_INITIAL_SIZE];
} MET__list_hash_stats_totals;
#else
#define HASH_STATS_COLLECT
#endif


void
MET_list_initialize()
{
   int		i;
   MET__list_hash_table *t = MET__list_global_hash_table;
   MME(MET_list_initialize);

   t->table = MET__list_static_table_table;
   t->size = MET__list_hash_table_sizes[MET__list_hash_size_index];
   for (i = 0; i < t->size; i++)
      t->table[i] = MET_NULL_LIST;
#if 0
   t->object_count = 0;
   t->full_count = (int) (t->size * HASH_TABLE_FULL_PERCENTAGE);
#endif
   t->num_ids = MET__LIST_HASH_INITIAL_NUM_IDS;

   ALLOCNM(t->id_in_use, Boolean, t->num_ids);
   for (i = 0; i < t->num_ids; i++)
      t->id_in_use[i] = NO;
   t->flag_len = (t->size + MET__LIST_HASH_BITS_MASK) >>
      MET__LIST_HASH_BITS_SHIFT;

   ALLOCNM(t->flags, MET__flag_bits, t->num_ids * t->flag_len);
   
#ifdef HASH_STATS
{
   struct MET__list_hash_stats *totals = &MET__list_hash_stats_totals;

   totals->longest = totals->most_objects = totals->objects =
      totals->entries = totals->num_samples = 0;
   for (i = 0; i < t->size; i++)
      totals->per_entry[i] = 0;
}
#endif
}

void
MET_list_exit()
{
   MET__list_hash_table *t = MET__list_global_hash_table;
   int		i;
   Boolean	first = YES;
   MME(MET_list_exit);

#ifdef MET__DEBUG
   /*
    * any objects left over in the table must be bogus
    */
   for (i = 0; i < t->size; i++)
      if (MET_NULL_LIST != t->table[i]) {
	 if (YES == first) {
	    PRINT(stdout, "Objects left in hash table:\n");
	    first = NO;
	 }
	 METlist_print(FNULL, t->table[i], 0);
      }
#endif MET__DEBUG

   if (t->table != MET__list_static_table_table) {
      FREEN(t->table, METlist*, t->size);
   }
}

/*
 * put LIST into the hash table.  This frees list.
 * list->id and list->key must be set, the others are optional.
 * any current value is overshadowed.
 */
void
MET_list_hash_add(list)
   METlist	*list;
{
   MET__list_hash_table *t = MET__list_global_hash_table;
   int		i = list->key % t->size;
   MET__flag_bits *bits;
   MME(MET_list_hash_add);
   HASH_STATS_COLLECT;

   MET_LIST_COPY_NULL(list->next, t->table[i]);
   MET_LIST_FREE_NULL(t->table[i]);
   MET_LIST_COPY(t->table[i], list);
   MET_LIST_FREE(list);

   bits = t->flags + list->id * t->flag_len;
   bits[i >> MET__LIST_HASH_BITS_SHIFT] |=
      1 << (i &  MET__LIST_HASH_BITS_MASK);
}

/*
 * associate SYM with I in virtual hash table ID
 */
void
METlist_hash_add_int(id, symbol, i)
   int		id;
   Sym		symbol;
   int		i;
{
   METlist	*list;
   MME(MET_list_hash_add_int);

   MET_LIST_ALLOC(list);
   list->id = id;
   list->key = (long) symbol;
   list->data.i = i;
   list->mm_type = MET__MM_TYPE_NONE;
   list->aux_mm_type = MET__MM_TYPE_NONE;
   MET_list_hash_add(list);
}

/*
 * associate SYMBOL with SET in virtual hash table ID.  return YES if it
 * already had a definition (but this defn is replaced).
 */
Boolean
METlist_hash_add_set(id, symbol, set)
   int		id;
   Sym		symbol;
   METset	*set;
{
   METlist	*list;
   MME(MET_list_hash_add_set);
   CHECK_SET(set);

   /*
    * try to find it
    */
   list = MET_list_hash_find((long) symbol, id,
			     MET_list_hash_find_object,
			     (char *) symbol);
   if (MET_NULL_LIST == list) {
      /*
       * not found -- add it
       */
      MET_LIST_ALLOC(list);
      list->id = id;
      list->key = (long) symbol;
      list->aux_mm_type = MET__MM_TYPE_NONE;
      list->mm_type = MET__MM_TYPE_SET;
      MET_SET_COPY(list->object.set, set);
      MET_list_hash_add(list);
      return NO;
   }
   /*
    * found -- just replace the set
    */
   MET_SET_FREE(list->object.set);
   MET_SET_COPY(list->object.set, set);
   return YES;
}

/*
 * associate SYMBOL with SYM in virtual hash table ID.  return YES if it
 * already had a definition (but this defn is replaced).
 */
Boolean
METlist_hash_add_sym(id, symbol, metsym)
   int		id;
   Sym		symbol;
   METsym	*metsym;
{
   METlist	*list;
   MME(MET_list_hash_add_sym);
   CHECK_SYM(metsym);

   /*
    * try to find it
    */
   list = MET_list_hash_find((long) symbol, id,
			     MET_list_hash_find_object,
			     (char *) symbol);
   if (MET_NULL_LIST == list) {
      /*
       * not found -- add it
       */
      MET_LIST_ALLOC(list);
      list->id = id;
      list->key = (long) symbol;
      list->aux_mm_type = MET__MM_TYPE_NONE;
      list->mm_type = MET__MM_TYPE_SYM;
      MET_SYM_COPY(list->object.sym, metsym);
      MET_list_hash_add(list);
      return NO;
   }
   /*
    * found -- just replace the sym
    */
   MET_SYM_FREE(list->object.sym);
   MET_SYM_COPY(list->object.sym, metsym);
   return YES;
}

/*
 * get the integer associated with SYMBOL, as placed into hash table ID with
 * METlist_hash_add_int().  return SUCCESS iff it is found.
 */
Boolean
METlist_hash_find_int(id, symbol, i)
   int		id;
   Sym		symbol;
   int		*i;
{
   METlist	*list;
   MME(METlist_hash_find_int);
   CHECK_PTR(i);

   list = MET_list_hash_find((long) symbol, id,
			     MET_list_hash_find_object,
			     (char *) symbol);
   if (MET_NULL_LIST == list)
      return FAILURE;

   *i = list->data.i;
   return SUCCESS;
}

/*
 * get the set associated with SYMBOL, returns NULL if not found
 */
METset *
METlist_hash_find_set(id, symbol)
   int		id;
   Sym		symbol;
{
   METlist	*list;
   METset	*result;
   MME(METlist_hash_find_set);

   list = MET_list_hash_find((long) symbol, id,
			     MET_list_hash_find_object,
			     (char *) symbol);
   if (MET_NULL_LIST == list)
      return MET_NULL_SET;

   MET_SET_COPY(result, list->object.set);
   return result;
}


/*
 * get the sym associated with SYMBOL, returns NULL if not found
 */
METsym *
METlist_hash_find_sym(id, symbol)
   int		id;
   Sym		symbol;
{
   METlist	*list;
   METsym	*result;
   MME(METlist_hash_find_sym);

   list = MET_list_hash_find((long) symbol, id,
			     MET_list_hash_find_object,
			     (char *) symbol);
   if (MET_NULL_LIST == list)
      return MET_NULL_SYM;

   MET_SYM_COPY(result, list->object.sym);
   return result;
}

/*
 * the usual foundp, arg should be the same as key.
 */
Boolean
MET_list_hash_find_object(list, arg)
   METlist	*list;
   char		*arg;
{
   MME(MET_list_hash_find_object);

   return list->key == (long) arg;
}

/*
 * foundp is called (with the potential list and arg) to identify the METlist
 * that we really want.  the pointer returned is transient.
 */
METlist *
MET_list_hash_find(key, id, foundp, arg)
   long		key;
   int		id;
   Boolean	(*foundp)();
   char		*arg;
{
   MET__list_hash_table *t = MET__list_global_hash_table;
   int		i = key % t->size;
   METlist	*list;
   MME(MET_list_hash_find);
   HASH_STATS_COLLECT;

   list = t->table[i];
   while (MET_NULL_LIST != list) {
      if (list->id == id &&
	  YES == (foundp)(list, arg))
	 return list;
      list = list->next;
   }
   return MET_NULL_LIST;
}

static int MET__list_hash_last_id = 0;
int
METlist_hash_new_id()
{
   MET__list_hash_table *t = MET__list_global_hash_table;
   int this_id, first_try, mask = t->num_ids - 1;
   int full_count;
   MET__flag_bits *bits;
   MME(METlist_hash_new_id);

   this_id = MET__list_hash_last_id;

   if (NO == t->id_in_use[this_id])
      goto done;

   first_try = this_id;
   full_count = 0;
   do {
      this_id = (this_id + 1) & mask;
      full_count++;
      if (NO == t->id_in_use[this_id])
	 goto done;
      if (t->num_ids == full_count) {
	 FATALS("too many simultaneous virtual hash tables.");
      }
   } while (first_try != this_id);

   FATALS("too many nested virtual hash tables");
   return LIST_HASH_BAD_ID;

 done:
   t->id_in_use[this_id] = YES;
   bits = t->flags + this_id * t->flag_len;
   UNROLL4(t->flag_len, *bits++ = 0;);
   MET__list_hash_last_id = (MET__list_hash_last_id + 1) & mask;
   
   return this_id;
}

static void
MET__list_clear_entry(list, id)
   METlist	**list;
   int		id;
{
   METlist	*tlist;
   MME(MET__list_clear_entry);
   
   while (*list != MET_NULL_LIST) {
      if ((*list)->id == id) {
	 if (MET_NULL_LIST == (*list)->next) {
	    MET_LIST_FREE(*list);
	    *list = MET_NULL_LIST;
	 } else {
	    MET_LIST_COPY(tlist, (*list)->next);
	    MET_LIST_FREE(*list);
	    MET_LIST_COPY(*list, tlist);
	    MET_LIST_FREE(tlist);
	 }
      } else {
	 list = &(*list)->next;
      }
   }
}

/*
 * clear out entries with 1s in them.  search the
 * bits in groups of 8.
 *
 * this is hardwired for >= 32 bit longs.  sorry.
 */
static void
MET__list_hash_free_bits(bits, id, entry)
   MET__flag_bits bits;
   int		id;
   int		entry;
{
   MET__list_hash_table *t = MET__list_global_hash_table;
   MET__flag_bits mask;
   int		i;
   MME(MET__list_hash_free_bits);

   if (0 != (bits & 0x000000ff)) {
      mask = 0x00000001;
      for (i = 0; i < 8; i++) {
	 if (0 != (mask & bits))
	    MET__list_clear_entry(t->table + entry + 0 + i, id);
	 mask <<= 1;
      }
   }
   if (0 != (bits & 0x0000ff00)) {
      mask = 0x00000100;
      for (i = 0; i < 8; i++) {
	 if (0 != (mask & bits))
	    MET__list_clear_entry(t->table + entry + 8 + i, id);
	 mask <<= 1;
      }
   }
   if (0 != (bits & 0x00ff0000)) {
      mask = 0x00010000;
      for (i = 0; i < 8; i++) {
	 if (0 != (mask & bits))
	    MET__list_clear_entry(t->table + entry + 16 + i, id);
	 mask <<= 1;
      }
   }
   if (0 != (bits & 0xff000000)) {
      mask = 0x01000000;
      for (i = 0; i < 8; i++) {
	 if (0 != (mask & bits))
	    MET__list_clear_entry(t->table + entry + 24 + i, id);
	 mask <<= 1;
      }
   }
}

/*
 * remove everything from the table with id ID.
 */
void
METlist_hash_free_id(id)
   int		id;
{
   MET__list_hash_table *t = MET__list_global_hash_table;
   int		entry;
   MET__flag_bits *bits;
   MME(METlist_hash_free_id);

   entry = 0;
   bits = t->flags + id * t->flag_len;

   UNROLL4(t->flag_len,
	   if (0 != *bits) MET__list_hash_free_bits(*bits, id, entry);
	   entry += MET__LIST_HASH_BITS;
	   bits++;);

   t->id_in_use[id] = NO;
   MET__list_hash_last_id--;
}


void
MET__list_free_deep(list)
   METlist	*list;
{
   MME(MET__list_free_deep);

   MET_LIST_FREE_NULL(list->next);

   MET_OBJECT_P_FREE(list->object, list->mm_type);
   MET_OBJECT_P_FREE(list->aux_object, list->aux_mm_type);

   /*
    * put it on the free list and cook up some bad magic
    */
   list->mminfo.next = MET__list_free_list;
   MET__list_free_list = list;
#ifdef MET__DEBUG_MAGIC
   list->magic = MET__MAGIC_FREE;
#endif
}

METlist *
MET__list_copy_deep(list)
   METlist	*list;
{
   MME(MET__list_copy_deep);
   CHECK_LIST(list);
   NYET;

   return MET_NULL_LIST;
}

/*
 * allocate more lists for the free list.
 */
#define FUNC MET__list_new_free_list
#define LIST MET__list_free_list
#define TYPE METlist
#define MM_TYPE MET__MM_TYPE_LIST
#include "new_free_list.c"
#undef FUNC
#undef LIST
#undef TYPE
#undef MM_TYPE

/* NB: reads *using* a list---does not read list. */

MET_object_p
MET_list_read(buf, len, used, list, mm_type)
   unsigned char	*buf;
   int			len;
   int			*used;
   METlist		**list;
   int			*mm_type;
{
   int		last_mm_type = MET__MM_TYPE_NONE;
   int		this_mm_type = MET__MM_TYPE_NONE;
   int		used_here = 0;
   MET_object_p op;
   MET_object_p last;
   MME(MET_list_read);

   *mm_type = MET__MM_TYPE_NONE;
   op.set = MET_NULL_SET;
   do {
      MET_OBJECT_P_FREE(op, last_mm_type);
      switch (this_mm_type) {
       case MET__MM_TYPE_NONE:
	 break;
       case MET__MM_TYPE_TYPE:
	 MET_TYPE_COPY(op.type, last.type);
	 MET_TYPE_FREE(last.type);
	 break;
       case MET__MM_TYPE_SET:
	 MET_SET_COPY(op.set, last.set);
	 MET_SET_FREE(last.set);
	 break;
       case MET__MM_TYPE_BLOCK:
	 MET_BLOCK_COPY(op.block, last.block);
	 MET_BLOCK_FREE(last.block);
	 break;
       case MET__MM_TYPE_DATA:
	 MET_DATA_COPY(op.data, last.data);
	 MET_DATA_FREE(last.data);
	 break;
      }
      last_mm_type = this_mm_type;
      this_mm_type = (MET_NULL_LIST == *list) ? 
	 MET__MM_TYPE_SET : (*list)->mm_type;
      switch (this_mm_type) {
       case MET__MM_TYPE_SET:
	 last.set = METset_read(buf + used_here, len - used_here,
				&used_here, list, op,
				MET_NULL_LIST != *list);
	 break;
       case MET__MM_TYPE_TYPE:
	 last.type = METtype_read(buf + used_here, len - used_here,
				  &used_here, list, op,
				  MET_NULL_LIST != *list);
	 break;
       case MET__MM_TYPE_BLOCK:
	 last.block = METblock_read(buf + used_here, len - used_here,
				    &used_here, list, op,
				    MET_NULL_LIST != *list);
	 break;
       case MET__MM_TYPE_DATA:
	 last.data = METdata_read(buf + used_here, len - used_here,
				  &used_here, list, op,
				  MET_NULL_LIST != *list);
	 break;
      }
   } while (MET_NULL_LIST != *list);

   MET_OBJECT_P_FREE(op, last_mm_type);
   
   *mm_type = this_mm_type;
   *used += used_here;
   return last;
}

#ifdef MET__DEBUG_PRINT

static int MET__list_ickey_id = LIST_HASH_BAD_ID;

void
MET_list_hash_print(stream, id)
   FILE		*stream;
   int		id;
{
   MET__list_hash_table *t = MET__list_global_hash_table;
   int		i;
   MME(MET_list_hash_print);

   if (FNULL == stream)
      stream = stdout;

   if (LIST_HASH_BAD_ID == id) {
      PRINT(stream, "\nthe complete hash table:\n");
   } else {
      PRINT(stream, "\nvirtual hash table %d:\n", id);
   }
   MET__list_ickey_id = id;
   for (i = 0; i < t->size; i++)
      if (MET_NULL_LIST != t->table[i]) {
	 PRINT(stream, "hash table entry %d:\n", i);
	 METlist_print(stream, t->table[i], 0);
      }
   MET__list_ickey_id = LIST_HASH_BAD_ID;
}

/*
 * note lack of MME() (this is often called from MET_periodic)
 */
#ifdef HASH_STATS
#define MAXIFY(A,B) A = MAX(A,B)
void
MET_list_hash_stats(stream)
   FILE		*stream;
{
   MET__list_hash_table *t = MET__list_global_hash_table;
   struct MET__list_hash_stats *totals = &MET__list_hash_stats_totals;
   int		i, objects, longest, this_entry, entries;
   METlist	*list;
   
   entries = objects = longest = 0;
   for (i = 0; i < t->size; i++) {
      list = t->table[i];
      this_entry = 0;
      while (MET_NULL_LIST != list) {
	 objects++;
	 this_entry++;
	 list = list->next;
      }
      if (this_entry > 0) {
	 entries++;
	 totals->per_entry[i] += this_entry;
	 MAXIFY(longest, this_entry);
      }
   }

   if (FNULL != stream)
      PRINT(stream, "hash stats:\t%d\t%d\t%d\n",
	    t->size, objects, longest);

   if (objects > 0) {
      totals->num_samples++;
      totals->entries += entries;
      totals->objects += objects;
      MAXIFY(totals->longest, longest);
      MAXIFY(totals->most_objects, objects);
   }
}

void
MET_list_hash_total_stats(stream)
   FILE		*stream;
{
   MET__list_hash_table *t = MET__list_global_hash_table;
   struct MET__list_hash_stats *totals = &MET__list_hash_stats_totals;
   int	i;
   MME(MET_list_hash_total_stats);
   
   if (FNULL == stream)
      stream = stdout;

   PRINT(stream, "total hash stats\n");
   PRINT(stream, "      longest %8d\n", totals->longest);
   PRINT(stream, " most objects %8d\n", totals->most_objects);
   PRINT(stream, "      objects %8d\n", totals->objects);
   PRINT(stream, "      entries %8d\n", totals->entries);
   PRINT(stream, "      samples %8d\n", totals->num_samples);
   PRINT(stream, "\n");
   for (i = 0; i < t->size; i++)
      PRINT(stream, "hits on entry %8d = %8d\n", i, totals->per_entry[i]);
}

#else

/*ARGSUSED*/
void
MET_list_hash_stats(stream)
   FILE		*stream;
{
   /* do nothing */
}
/*ARGSUSED*/
void
MET_list_hash_total_stats(stream)
   FILE		*stream;
{
   /* do nothing */
}
#endif HASH_STATS

void
METlist_print(stream, list, indent_level)
   FILE		*stream;
   METlist	*list;
   int		indent_level;
{
   MME(METlist_print);
   CHECK_LIST(list);

   /*
    * ickey_id is used to keep the interface to the print functions uniform
    */
   if (MET_NULL_LIST == list ||
       (LIST_HASH_BAD_ID != MET__list_ickey_id &&
	list->id != MET__list_ickey_id))
      return;

   if (FNULL == stream)
      stream = stdout;

   MET_indent(stream, indent_level);
   PRINT(stream,
	 "list at %#x: ref_count = %d, ",
	 list, list->mminfo.ref_count);
   PRINT(stream,
	 "next = %#x, id = %d, key = %#x, data = %d, ",
	 list->next, list->id, list->key, list->data);
   if (MET__MM_TYPE_NONE != list->mm_type) {
      PRINT(stream,
	    "mm_type = %s, object = %#x, ",
	    MET_obj_name[list->mm_type], list->object.set);
   }

   if (MET__MM_TYPE_NONE != list->aux_mm_type) {
      PRINT(stream,
	    "aux_mm_type = %s, aux_object = %#x, ",
	    MET_obj_name[list->aux_mm_type], list->aux_object.set);
   }
   
   PRINT(stream, "\n");

   if (MET_NULL_LIST != list->next)
      METlist_print(stream, list->next, indent_level);
}

Boolean
MET__list_has_ref(list, mm_type, object)
   METlist	*list;
   int		mm_type;
   MET_object	*object;
{
   MME(MET__list_has_ref);

   if (mm_type == MET__MM_TYPE_LIST &&
       list->next == (METlist *) object)
      return YES;

   switch(mm_type) {
    case MET__MM_TYPE_SET:
      if (list->object.set == (METset *) object)
	 return YES;
    case MET__MM_TYPE_TYPE:
      if (list->object.type == (METtype *) object)
	 return YES;
    case MET__MM_TYPE_DATA:
      if (list->object.data == (METdata *) object)
	 return YES;
    case MET__MM_TYPE_VERTS:
      if (list->object.verts == (METverts *) object)
	 return YES;
    case MET__MM_TYPE_SYM:
      if (list->object.sym == (METsym *) object)
	 return YES;
    case MET__MM_TYPE_BLOCK:
      if (list->object.block == (METblock *) object)
	 return YES;
    case MET__MM_TYPE_CONNEX:
      if (list->object.connex == (METconnex *) object)
	 return YES;
    case MET__MM_TYPE_LIST:
      if (list->object.list == (METlist *) object)
	 return YES;
      break;
      
    default:
      DEFAULT_ERR(mm_type);
   }

   return NO;
}
#endif MET__DEBUG_PRINT
