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
   "$Id: met_block.c,v 1.15 1992/06/22 17:04:30 mas Exp $";
#endif

void
MET__block_free_deep(block)
   METblock	*block;
{
   MME(MET__block_free_deep);
   CHECK_BLOCK(block);

   MET_BLOCK_FREE_NULL(block->next);

   MET_TYPE_FREE(block->type);
   MET_DATA_FREE(block->data);
   MET_CONNEX_FREE(block->connex);
   MET_CONNEX_FREE(block->data_connex);
   MET_tags_free(&block->tags_in);
   MET_tags_free(&block->tags_part);

   block->mminfo.next = MET__block_free_list;
   MET__block_free_list = block;
#ifdef MET__DEBUG_MAGIC
   block->magic = MET__MAGIC_FREE;
#endif
}

/*
 * all child structures are SHALLOW copied, except the following blocks in
 * the linked list, which are deep copied.
 */
METblock *
MET__block_copy_deep(block)
   METblock	*block;
{
   METblock	*baby;
   MME(MET__block_copy_deep);
   CHECK_BLOCK(block);

   MET_BLOCK_ALLOC(baby);
   if (MET_NULL_BLOCK != block->next) {
      MET_BLOCK_COPY_DEEP(baby->next, block->next);
   } else
      baby->next = MET_NULL_BLOCK;
   MET_TYPE_COPY(baby->type, block->type);
   MET_DATA_COPY(baby->data, block->data);
   MET_CONNEX_COPY(baby->connex, block->connex);
   MET_CONNEX_COPY(baby->data_connex, block->data_connex);
   MET_tags_initialize(&baby->tags_in);
   MET_tags_initialize(&baby->tags_part);
   MET_tags_copy(&baby->tags_in, &block->tags_in);
   MET_tags_copy(&baby->tags_part, &block->tags_part);
   return baby;
}

METset *
MET_block_to_polybare(block, id, type, size)
   METblock	*block;
   int		id;
   int		type;
   int		size;
{
   METset	*bare_set;
   METtag	tag;
   int		i, data_factor = 1;
   MME(MET_block_to_polybare);

   bare_set = MET_set_new(MET_SET_TYPE_BARE);
   MET_TYPE_COPY(bare_set->info.bare.type, block->type->info.set_of.of);
   MET_DATA_COPY(bare_set->info.bare.data, block->data);
   bare_set->info.bare.how_many = DATA_NUM_VERTS(block->data);

   MET_tags_initialize(&bare_set->info.bare.tags_in);
   MET_tags_initialize(&bare_set->info.bare.tags_bare);

   MET_tags_copy(&bare_set->info.bare.tags_bare,
		 &block->tags_part);
   
   for (i = 0; i < block->tags_in.num_tags; i++)
      if (NO == block->tags_in.tags[i].scalar)
	 data_factor *= block->tags_in.tags[i].size;
   bare_set->info.bare.how_many *= data_factor;
   
   if (MET_TAG_NO_ID == id) {
      MET_tags_copy(&bare_set->info.bare.tags_bare,
		    &block->tags_in);
   } else {
      MET_tags_copy(&bare_set->info.bare.tags_in,
		    &block->tags_in);
      tag.id = id;
      tag.type = type;
      tag.size = size;
      tag.index = 0;
      tag.scalar = FALSE;
      MET_CONNEX_COPY(tag.connex, block->data_connex);
      MET_tags_add(&bare_set->info.bare.tags_bare,
		   &tag, FALSE);
      MET_CONNEX_FREE(tag.connex);
   }

#ifdef MET__DEBUG
   if (1) {
     int csize = MET_connex_num_pts(block->connex) / data_factor;
     if (size !=  csize &&
	 -size != csize) {
       FATAL((ERR_SI, "connex doesn't match size", csize));
     }
   }
#endif
   return bare_set;
}

METblock *
MET_block_lose_tags(block, tags)
   METblock	*block;
   METtag_set	*tags;
{
   METblock	*baby;
   MME(MET_block_lose_tags);

   if (MET_NULL_BLOCK == block)
      return MET_NULL_BLOCK;

   MET_BLOCK_ALLOC(baby);
   if (MET_NULL_BLOCK != block->next) {
      baby->next = MET_block_lose_tags(block->next, tags);
   } else
      baby->next = MET_NULL_BLOCK;
   MET_TYPE_COPY(baby->type, block->type);
   MET_DATA_COPY(baby->data, block->data);
   MET_CONNEX_COPY(baby->connex, block->connex);
   MET_CONNEX_COPY(baby->data_connex, block->data_connex);
   MET_tags_initialize(&baby->tags_in);
   MET_tags_initialize(&baby->tags_part);
   MET_tags_copy(&baby->tags_in, &block->tags_in);
   MET_tags_copy_new(&baby->tags_part, &block->tags_part, tags);
   return baby;
}

#define FUNC MET__block_new_free_list
#define LIST MET__block_free_list
#define TYPE METblock
#define MM_TYPE MET__MM_TYPE_BLOCK
#include "new_free_list.c"
#undef FUNC
#undef LIST
#undef TYPE
#undef MM_TYPE

void
METblock_print_pretty(stream, block, s, slen)
   FILE		*stream;
   METblock	*block;
   char		*s;
   int		slen;
{
   char		buf[8192];
   char         shortie[24];
   MME(METblock_print_pretty);

   buf[0] = '\0';

   METtype_print_pretty(FNULL, block->type, buf, 0);
   (void) strcat(buf, " : {");
   METdata_print_pretty(FNULL, block->data,
			block->type->info.set_of.of, buf, 0);
   
   (void) strcat(buf, "}");

   if (FNULL != stream)
      (void) fprintf(stream, "%s", buf);
   if (CNULL != s)
      (void) strcat(s, buf);
}

Boolean
METblock_write(block, buf, len)
   METblock		*block;
   unsigned char	**buf;
   int			*len;
{
   MME(METblock_write);

   if (FAILURE == METtype_write(block->type, buf, len))
      return FAILURE;

   if (FAILURE == METdata_write(block->data, buf, len))
      return FAILURE;

   if (FAILURE == METconnex_write(block->connex, buf, len))
      return FAILURE;

   return SUCCESS;
}

METblock *
METblock_read(buf, len, used, list, last, cont)
   unsigned char	*buf;
   int			len;
   int			*used;
   METlist		**list;
   MET_object_p		last;
   Boolean		cont;
{
   METblock		*block;
   METlist		*oldlist;
   int			count;
   MET_object_p		op;
   MME(METtype_read);
   
   if (NO == cont) {
      if (MET_NULL_LIST != *list) {
	 MET_LIST_COPY(oldlist, *list);
	 MET_LIST_FREE(*list);
      } else 
	 oldlist = MET_NULL_LIST;

      MET_LIST_ALLOC(*list);

      if (MET_NULL_LIST != oldlist) {
	 MET_LIST_COPY((*list)->next, oldlist);
	 MET_LIST_FREE(oldlist);
      } else
	 (*list)->next = MET_NULL_LIST;

      (*list)->mm_type = MET__MM_TYPE_BLOCK;
      (*list)->aux_mm_type = MET__MM_TYPE_NONE;
      MET_BLOCK_ALLOC(block);
      MET_BLOCK_COPY((*list)->object.block, block);

      MET_tags_initialize(&block->tags_in);
      MET_tags_initialize(&block->tags_part);
      block->connex = MET_NULL_CONNEX;
      block->data_connex = MET_NULL_CONNEX;
      block->type = MET_NULL_TYPE;
      block->data = MET_NULL_DATA;
   } else if (MET_NULL_LIST == *list)
      return MET_NULL_BLOCK;
   else {
      MET_BLOCK_COPY(block, (*list)->object.block);
   }

   if (MET_NULL_TYPE == block->type) {
      if (YES == cont && MET_NULL_TYPE != last.type) {
	 MET_TYPE_COPY(block->type, last.type);
	 last.type = MET_NULL_TYPE;
      } else {
	 count = 0;
	 op.type = MET_NULL_TYPE;
	 block->type = METtype_read(buf, len, &count, list, op, FALSE);
	 *used += count;
	 len -= count;
	 buf += count;
	 if (MET_NULL_TYPE == block->type) {
	    MET_BLOCK_FREE(block);
	    return MET_NULL_BLOCK;
	 }
      }
   }

   if (MET_NULL_DATA == block->data) {
      if (YES == cont && MET_NULL_DATA != last.data) {
	 MET_DATA_COPY(block->data, last.data);
	 last.data = MET_NULL_DATA;
      } else {
	 count = 0;
	 op.data = MET_NULL_DATA;
	 block->data = METdata_read(buf, len, &count, list, op, FALSE);
	 *used += count;
	 len -= count;
	 buf += count;
	 if (MET_NULL_DATA == block->data) {
	    MET_BLOCK_FREE(block);
	    return MET_NULL_BLOCK;
	 }
      }
   }

   if (MET_NULL_CONNEX == block->connex) {
      count = 0;
      block->connex = METconnex_read(buf, len, &count);
      *used += count;
      len -= count;
      buf += count;
      if (MET_NULL_CONNEX == block->connex) {
	 MET_BLOCK_FREE(block);
	 return MET_NULL_BLOCK;
      }
      MET_CONNEX_COPY(block->data_connex, block->connex);
   }

   if (MET_NULL_LIST == (*list)->next) {
      MET_LIST_FREE(*list);
      *list = MET_NULL_LIST;
   } else {
      MET_LIST_COPY(oldlist, (*list)->next);
      MET_LIST_FREE(*list);
      MET_LIST_COPY(*list, oldlist);
      MET_LIST_FREE(oldlist);
   }

   return block;
}

#ifdef MET__DEBUG_PRINT
void
METblock_print(stream, block, indent_level)
   FILE		*stream;
   METblock	*block;
   int		indent_level;
{
   MME(METblock_print);
   CHECK_NULL_BLOCK(block);

   if (FNULL == stream)
      stream = stdout;

   MET_indent(stream, indent_level);
   if (MET_NULL_BLOCK == block) {
      PRINT(stream, "null block\n");
      return;
   }
   PRINT(stream, "block at %#x: ref_count = %d, false_scalar = %d",
	 block, block->mminfo.ref_count, 
	 0 == block->tags_in.num_tags);
   PRINT(stream, "\n");
   METtype_print(stream, block->type, indent_level + 1);
   METconnex_print(stream, block->connex, indent_level + 1);
   METconnex_print(stream, block->data_connex, indent_level + 1);
   METdata_print(stream, block->data, indent_level + 1);
   if (MET_NULL_BLOCK != block->next)
      METblock_print(stream, block->next, indent_level);
   
}
Boolean
MET__block_has_ref(block, mm_type, object)
   METblock	*block;
   int		mm_type;
   MET_object	*object;
{
   MME(MET__block_has_ref);

   switch(mm_type) {
    case MET__MM_TYPE_SET:
      break;

    case MET__MM_TYPE_TYPE:
      if (block->type == (METtype *) object)
	 return YES;
      break;

    case MET__MM_TYPE_DATA:
      if (block->data == (METdata *) object)
	 return YES;
      break;
      
    case MET__MM_TYPE_VERTS:
    case MET__MM_TYPE_SYM:
      break;

    case MET__MM_TYPE_BLOCK:
      if (block->next == (METblock *) object)
	 return YES;
      break;

    case MET__MM_TYPE_CONNEX:
      if (block->connex == (METconnex *) object)
	 return YES;
      if (block->data_connex == (METconnex *) object)
	 return YES;
      break;

    case MET__MM_TYPE_LIST:
      break;
      
    default:
      DEFAULT_ERR(mm_type);
   }

   return NO;
}
#endif MET__DEBUG_PRINT
