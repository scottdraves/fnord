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

#include "met/defs.h"

#ifndef lint
static char Version[] =
   "$Id: met_tags.c,v 1.4 1992/06/22 17:05:01 mas Exp $";
#endif /* lint */

/*
 * Clear out any bad data in a set of tags.
 */

void MET_tags_initialize(ts)
   METtag_set	*ts;
{
   MME(MET_tags_initialize);
   
   ts->num_tags = 0;
   ts->tags = (METtag *) ts->static_tags;
}

/*
 * Free any data in a set of tags.
 */

void MET_tags_free(ts)
   METtag_set	*ts;
{
   int		i;
   MME(MET_tags_free);

   for (i = 0; i < ts->num_tags; i++)
      MET_CONNEX_FREE(ts->tags[i].connex);

   if (ts->tags != ts->static_tags) 
      FREEN(ts->tags, METtag, ts->num_tags);

}

/*
 * Add a copy of the passed tag in the right
 * order to the passed tag set.
 */

void MET_tags_add(ts, tag, scalar)
   METtag_set	*ts;
   METtag	*tag;
   FOboolean	scalar;
{
   int		n, i, j;
   METtag	*new_tags, *t;
   MME(MET_tags_add);

   /*
    * make space for another tag
    */
   n = ts->num_tags;
   if (ts->tags != ts->static_tags) {
      REALLOCNM(ts->tags, METtag, n + 1, n);
   } else if (MET_TAGS_STATIC_SIZE == n) {
      ALLOCNM(new_tags, METtag, n + 1);
      for (i = 0; i < MET_TAGS_STATIC_SIZE; i++) {
	 TAG_COPY(new_tags[i], ts->tags[i]);
	 MET_CONNEX_FREE(ts->tags[i].connex);
      }
      ts->tags = new_tags;
   } 

   /*
    * tags are kept sorted by id.  find where this one goes.
    */
   t = ts->tags;
   for (i = 0; i < n; i++) 
      if (t[i].id > tag->id)
	 break;

   /*
    * move those behind it back a slot
    */
   for (j = n; j > i; j--) {
      TAG_COPY(t[j], t[j - 1]);
      MET_CONNEX_FREE(t[j - 1].connex);
   }
   /*
    * drop it in
    */
   TAG_COPY(t[i], *tag);
   t[i].scalar = scalar;

   ts->num_tags++;
}

/*
 * Take all the tags on the src set and make sure that they are repeated
 * in the dst set (doesn't duplicate tags, though).
 */

void MET_tags_copy(dst, src)
   METtag_set	*dst, *src;
{
   int 		i, j, k, need;
   METtag	*new_tags, *old_tags, space[MET_TAGS_STATIC_SIZE];
   MME(MET_tags_copy);

   /*
    * count how many you need.
    */
   for (need = i = j = 0;
	i < src->num_tags && j < dst->num_tags; need++) 
      if (dst->tags[j].id == src->tags[i].id) {
	 dst->tags[j].scalar &= src->tags[i].scalar;
	 i++, j++;
      } else if (dst->tags[j].id > src->tags[i].id)
	 i++;
      else
	 j++;

   need += (src->num_tags - i) + (dst->num_tags - j);

   /* 
    * I think this will happen fairly often:
    */
   if (need == dst->num_tags)
      return;

   if (need <= MET_TAGS_STATIC_SIZE) {
      old_tags = space;
      for (i = 0; i < dst->num_tags; i++) {
	 TAG_COPY(space[i], dst->tags[i]);
	 MET_CONNEX_FREE(dst->tags[i].connex);
      }
      new_tags = dst->static_tags;
   } else {
      old_tags = dst->tags;
      ALLOCNM(new_tags, METtag, need);
   }
   dst->tags = new_tags;

   /*
    * Merge.
    */
   for (i = j = k = 0; 
	i < src->num_tags && j < dst->num_tags; k++) {
      if (old_tags[j].id < src->tags[i].id) {
	 TAG_COPY(new_tags[k], old_tags[j]);
	 MET_CONNEX_FREE(old_tags[j].connex);
	 j++;
      } else if (old_tags[j].id == src->tags[i].id) {
	 TAG_COPY(new_tags[k], old_tags[j]);
	 MET_CONNEX_FREE(old_tags[j].connex);
	 j++;  i++;
      } else {
	 TAG_COPY(new_tags[k], src->tags[i]);
	 i++;
      }
   }

   while (i < src->num_tags) {
      TAG_COPY(new_tags[k], src->tags[i]);
      i++; k++;
   }

   while (j < dst->num_tags) {
      TAG_COPY(new_tags[k], old_tags[j]);
      MET_CONNEX_FREE(old_tags[j].connex);
      j++; k++;
   }

   if (old_tags != dst->static_tags && old_tags != space)
      FREEN(old_tags, METtag, dst->num_tags);

   dst->num_tags = need;

   for (i = 0; i < need; i++)
      if (dst->tags[i].size < 1)
	 printf("Ack!\n");
}

/*
 * Add to dst set those tags in src which are not noted
 * in compare.
 */

void MET_tags_copy_new(dst, src, compare)
   METtag_set	*dst, *src, *compare;
{
   int 		i, j, k, l, need;
   METtag	*new_tags, *old_tags, space[MET_TAGS_STATIC_SIZE];
   MME(MET_tags_copy_new);

   /*
    * count how many you need.
    */
   for (need = i = j = k = 0;
	i < src->num_tags && j < dst->num_tags;)  {

      /* 
       * do you need to skip this one?
       */
      while (k < compare->num_tags &&
	     compare->tags[k].id < src->tags[i].id)
	 k++;
      if (k < compare->num_tags &&
	  compare->tags[k].id == src->tags[i].id) {
	 k++; i++;
	 continue;
      }
      
      /*
       * very well then
       */
      if (dst->tags[j].id == src->tags[i].id) {
	 dst->tags[j].scalar &= src->tags[i].scalar;
	 i++, j++;
      } else if (dst->tags[j].id > src->tags[i].id)
	 i++;
      else
	 j++;
      need++;
   }

   /*
    * make sure that any left in src are OK.
    */
   if (i < src->num_tags)
      while (i < src->num_tags) {
	 while (k < compare->num_tags &&
		compare->tags[k].id < src->tags[i].id)
	    k++;
	 if (k < compare->num_tags &&
	     compare->tags[k].id == src->tags[i].id) 
	    k++;
	 else 
	    need++;
	 i++;
      }
   else
      need += (dst->num_tags - j);

   /* 
    * I think this will happen fairly often:
    */
   if (need == dst->num_tags)
      return;

   if (need <= MET_TAGS_STATIC_SIZE) {
      old_tags = space;
      for (i = 0; i < dst->num_tags; i++) {
	 TAG_COPY(space[i], dst->tags[i]);
	 MET_CONNEX_FREE(dst->tags[i].connex);
      }
      new_tags = dst->static_tags;
   } else {
      old_tags = dst->tags;
      ALLOCNM(new_tags, METtag, need);
   }
   dst->tags = new_tags;

   /*
    * Merge.
    */
   for (i = j = l = k = 0; 
	i < src->num_tags && j < dst->num_tags;) {

      /*
       * do we copy this guy?
       */
      while (k < compare->num_tags &&
	     compare->tags[k].id < src->tags[i].id)
	 k++;
      if (k < compare->num_tags &&
	  compare->tags[k].id == src->tags[i].id) {
	 k++; i++;
	 continue;
      }

      /*
       * very well then
       */
      if (old_tags[j].id < src->tags[i].id) {
	 TAG_COPY(new_tags[l], old_tags[j]);
	 MET_CONNEX_FREE(old_tags[j].connex);
	 j++;
      } else if (old_tags[j].id == src->tags[i].id) {
	 TAG_COPY(new_tags[l], old_tags[i]);
	 MET_CONNEX_FREE(old_tags[j].connex);
	 j++;  i++;
      } else {
	 TAG_COPY(new_tags[l], src->tags[i]);
	 i++;
      }
      l++;
   }

   while (i < src->num_tags) {
      while (k < compare->num_tags &&
	     compare->tags[k].id < src->tags[i].id)
	 k++;
      if (k < compare->num_tags &&
	  compare->tags[k].id == src->tags[i].id) 
	 k++;
      else {
	 TAG_COPY(new_tags[l], src->tags[i]);
	 l++;
      }
      i++;
   }

   while (j < dst->num_tags) {
      TAG_COPY(new_tags[l], old_tags[j]);
      MET_CONNEX_FREE(old_tags[j].connex);
      j++; l++;
   }

   if (old_tags != dst->static_tags && old_tags != space)
      FREEN(old_tags, METtag, dst->num_tags);

   dst->num_tags = need;
}

/*
 * Returns true if the passed tagsets are as identical as they can be.
 */

Boolean MET_tags_equal(ts1, ts2)
   METtag_set	*ts1, *ts2;
{
   int		i;
   MME(MET_tags_equal);

   if (ts1->num_tags != ts2->num_tags)
      return NO;

   for (i = 0; i < ts1->num_tags; i++)
      if (ts1->tags[i].id != ts2->tags[i].id || 
	  ts2->tags[i].scalar != ts1->tags[i].scalar)
	 return NO;

   return YES;
}

/*
 * Change all of the types of the tags in the passed tag set to partial
 * and compute the indices of each tag, corresponding to the passed row
 * number.
 */

void MET_tags_partialize(tags, row)
   METtag_set 	*tags;
{
   int		i, remain, size;
   MME(MET_tags_partialize);
   
   for (i = 0, size = 1; i < tags->num_tags; i++)
      size *= (tags->tags[i].scalar == YES) ? 1 : tags->tags[i].size;

   for (i = 0, remain = row; i < tags->num_tags; i++) {
      tags->tags[i].type = MET_TAG_TYPE_ELEMENT;
      if (tags->tags[i].scalar == YES) {
	 tags->tags[i].index = 0;
      } else {
	 size /= tags->tags[i].size;
	 tags->tags[i].index = remain / size;
	 remain = remain % size;
      }
   }
}
