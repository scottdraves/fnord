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
   "$Id: met_type.c,v 1.22 1992/06/22 17:05:04 mas Exp $";
#endif

#if 1
int MET__type_cast_fields[MET_FIELD_MAX][MET_FIELD_MAX] = {
{  			/* MET_FIELD_ZTWO */
   MET_FIELD_ZTWO, 	MET_FIELD_UNKNOWN, 	MET_FIELD_UNKNOWN, 
   MET_FIELD_UNKNOWN, 	MET_FIELD_UNKNOWN, 	MET_FIELD_UNKNOWN,
   MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN, 	MET_FIELD_ZTWO
}, {			/* MET_FIELD_INTEGER */
   MET_FIELD_UNKNOWN, 	MET_FIELD_INTEGER,	MET_FIELD_UNKNOWN,
   MET_FIELD_REAL,	MET_FIELD_COMPLEX,	MET_FIELD_QUATERNION,
   MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,	MET_FIELD_INTEGER
}, {			/* MET_FIELD_ANGLE */
   MET_FIELD_UNKNOWN, 	MET_FIELD_UNKNOWN, 	MET_FIELD_ANGLE,
   MET_FIELD_UNKNOWN, 	MET_FIELD_UNKNOWN, 	MET_FIELD_UNKNOWN,
   MET_FIELD_UNKNOWN, 	MET_FIELD_UNKNOWN, 	MET_FIELD_ANGLE
}, {			/* MET_FIELD_REAL */
   MET_FIELD_UNKNOWN, 	MET_FIELD_REAL, 	MET_FIELD_UNKNOWN,
   MET_FIELD_REAL, 	MET_FIELD_COMPLEX, 	MET_FIELD_QUATERNION,
   MET_FIELD_UNKNOWN, 	MET_FIELD_UNKNOWN, 	MET_FIELD_REAL
}, {			/* MET_FIELD_COMPLEX */
   MET_FIELD_UNKNOWN, 	MET_FIELD_COMPLEX, 	MET_FIELD_UNKNOWN,
   MET_FIELD_COMPLEX,	MET_FIELD_COMPLEX,  	MET_FIELD_QUATERNION,
   MET_FIELD_UNKNOWN, 	MET_FIELD_UNKNOWN, 	MET_FIELD_COMPLEX
}, {			/* MET_FIELD_QUATERNION */
   MET_FIELD_UNKNOWN, 	MET_FIELD_QUATERNION, 	MET_FIELD_UNKNOWN,
   MET_FIELD_QUATERNION,MET_FIELD_QUATERNION,	MET_FIELD_QUATERNION,
   MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,	MET_FIELD_QUATERNION
}, {			/* MET_FIELD_MAP */
   MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,
   MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,
   MET_FIELD_MAP, 	MET_FIELD_UNKNOWN,	MET_FIELD_MAP
}, {			/* MET_FIELD_SET */
   MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,
   MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,	MET_FIELD_UNKNOWN,
   MET_FIELD_UNKNOWN,	MET_FIELD_SET,		MET_FIELD_SET
}, {			/* MET_FIELD_UNKNOWN */
   MET_FIELD_ZTWO,	MET_FIELD_INTEGER,	MET_FIELD_ANGLE,
   MET_FIELD_REAL, 	MET_FIELD_COMPLEX, 	MET_FIELD_QUATERNION,
   MET_FIELD_MAP,	MET_FIELD_SET, 		MET_FIELD_UNKNOWN
}};
#else
#if 0
/* should use symbols */ /* Ack! */
int MET__type_cast_fields[MET_FIELD_MAX][MET_FIELD_MAX] = {
{MET_FIELD_ZTWO, 8, 8, 8, 8, 8, 8, 8, MET_FIELD_ZTWO},
{8, 1, 8, 3, 4, 5, 8, 8, 1},
{8, 8, 2, 8, 8, 8, 8, 8, 2},
{8, 3, 8, 3, 4, 5, 8, 8, 3},
{8, 4, 8, 4, 4, 5, 8, 8, 4},
{8, 5, 8, 5, 5, 5, 8, 8, 5},
{8, 8, 8, 8, 8, 8, 6, 8, 6},
{8, 8, 8, 8, 8, 8, 8, 7, 7},
{MET_FIELD_ZTWO, 1, 2, 3, 4, 5, 6, 7, 8}};
#else
#endif
#endif

METtype *MET__type_unknown;
METtype *MET__type_set;
METtype *MET__type_bogus;
METtype *MET__type_field[MET_FIELD_MAX];
METtype *MET_type_generic_map;
METtype *MET_type_interval;
METtype *MET_type_access_text;
METtype *MET_type_access_marker;

void
MET_type_initialize()
{
   int		i;
   METtype	*t,*t1, *t2;
   MME(MET_type_initialize);

   MET_TYPE_ALLOC(MET__type_unknown);
   MET__type_unknown->type = MET_TYPE_TYPE_UNKNOWN;
   MET_TYPE_ALLOC(MET__type_set);
   MET__type_set->type = MET_TYPE_TYPE_SET;
   MET_TYPE_ALLOC(MET__type_bogus);
   MET__type_bogus->type = MET_TYPE_TYPE_BOGUS;

   MET_TYPE_ALLOC(MET_type_generic_map);
   MET_type_generic_map->type = MET_TYPE_TYPE_MAP;
   MET_TYPE_NEW_UNKNOWN(MET_type_generic_map->info.map.from);
   MET_TYPE_NEW_UNKNOWN(MET_type_generic_map->info.map.to);

   for (i = 0; i < MET_FIELD_MAX; i++) {
      MET_TYPE_ALLOC(MET__type_field[i]);
      MET__type_field[i]->type = MET_TYPE_TYPE_FIELD;
      MET__type_field[i]->info.field.type = i;
   }

   MET_TYPE_NEW_FIELD(t, MET_FIELD_REAL);
   MET_TYPE_NEW_SET_OF(MET_type_interval, t);
   MET_TYPE_FREE(t);

   MET_TYPE_NEW_UNKNOWN(t);
   MET_TYPE_NEW_FIELD(t1, MET_FIELD_INTEGER);
   MET_TYPE_NEW_PAIR(MET_type_access_marker, t, t1);
   MET_TYPE_FREE(t);
   MET_TYPE_FREE(t1);

   MET_TYPE_NEW_UNKNOWN(t);
   MET_TYPE_NEW_FIELD(t1, MET_FIELD_INTEGER);
   MET_TYPE_NEW_VECTOR(t2, t1, TYPE_DIMENSION_UNKNOWN, NO);
   MET_TYPE_FREE(t1);
   MET_TYPE_NEW_PAIR(MET_type_access_text, t, t2);
   MET_TYPE_FREE(t);
   MET_TYPE_FREE(t2);
}

void
MET_type_exit()
{
   int		i;
   MME(MET_type_exit);

   for (i = 0; i < MET_FIELD_MAX; i++)
      MET_TYPE_FREE(MET__type_field[i]);

   MET_TYPE_FREE(MET__type_unknown);
   MET_TYPE_FREE(MET__type_set);
   MET_TYPE_FREE(MET__type_bogus);
   MET_TYPE_FREE(MET_type_generic_map);
   MET_TYPE_FREE(MET_type_interval);
   MET_TYPE_FREE(MET_type_access_marker);
   MET_TYPE_FREE(MET_type_access_text);
}

void
MET__type_free_deep(type)
   METtype	*type;
{
   MME(MET__type_free_deep);
   CHECK_TYPE(type);

   switch (type->type) {
      
    case MET_TYPE_TYPE_UNKNOWN:
      break;

    case MET_TYPE_TYPE_MAP:
      MET_TYPE_FREE_NULL(type->info.map.from);
      MET_TYPE_FREE_NULL(type->info.map.to);
      break;
      
    case MET_TYPE_TYPE_PAIR:
      MET_TYPE_FREE_NULL(type->info.pair.left);
      MET_TYPE_FREE_NULL(type->info.pair.right);
      break;
      
    case MET_TYPE_TYPE_VECTOR:
      MET_TYPE_FREE_NULL(type->info.vector.of);
      break;

    case MET_TYPE_TYPE_SET:
      break;
      
    case MET_TYPE_TYPE_FIELD:
      break;
      
    case MET_TYPE_TYPE_SET_OF:
      MET_TYPE_FREE_NULL(type->info.set_of.of);
      break;
      
    case MET_TYPE_TYPE_BOGUS:
      break;
      
    default:
      DEFAULT_ERR(type->type);
   }

   type->mminfo.next = MET__type_free_list;
   MET__type_free_list = type;
#ifdef MET__DEBUG_MAGIC
   type->magic = MET__MAGIC_FREE;
#endif
}


METtype *
MET__type_copy_deep(type)
   METtype	*type;
{
   METtype	*baby_type;
   MME(MET__type_copy_deep);
   CHECK_TYPE(type);

   MET_TYPE_ALLOC(baby_type);
   baby_type->type = type->type;

   switch (type->type) {
      
    case MET_TYPE_TYPE_UNKNOWN:
      break;

    case MET_TYPE_TYPE_MAP:
      MET_TYPE_COPY_DEEP(baby_type->info.map.from, type->info.map.from);
      MET_TYPE_COPY_DEEP(baby_type->info.map.to,   type->info.map.to);
      break;
      
    case MET_TYPE_TYPE_PAIR:
      MET_TYPE_COPY_DEEP(baby_type->info.pair.left,  type->info.pair.left);
      MET_TYPE_COPY_DEEP(baby_type->info.pair.right, type->info.pair.right);
      break;
      
    case MET_TYPE_TYPE_VECTOR:
      MET_TYPE_COPY_DEEP(baby_type->info.vector.of, type->info.vector.of);
      baby_type->info.vector.dimension = type->info.vector.dimension;
      baby_type->info.vector.is_row = type->info.vector.is_row;
      break;
      
    case MET_TYPE_TYPE_FIELD:
      break;
      
    case MET_TYPE_TYPE_SET_OF:
      MET_TYPE_COPY_DEEP(baby_type->info.set_of.of, type->info.set_of.of);
      break;
      
    case MET_TYPE_TYPE_BOGUS:
      break;
      
    default:
      DEFAULT_ERR(type->type);
   }
      
   return baby_type;
}

/*
 * return YES if the types are the same, or if one is a subtype of two.
 * Two types of casting are always permitted:  up from basic and 
 * up to universal. In addition, if allow_casts is YES,
 * then two fields are considered equivalent if one can
 * be cast to the other.  With this enabled, ONE may be
 * cast up to TWO, but not the other way around.
 */
Boolean
MET_type_compatible(one, two, allow_casts)
   METtype	*one, *two;
   Boolean	allow_casts;	/* YES/NO */
{
   int		result_of_cast;
   MME(MET_type_compatible);
   CHECK_TYPE(one);
   CHECK_TYPE(two);

   if (one == two)
      return YES;

   if (one->type == MET_TYPE_TYPE_UNKNOWN ||
       two->type == MET_TYPE_TYPE_UNKNOWN)
      return YES;

   if (one->type == MET_TYPE_TYPE_BOGUS ||
       two->type == MET_TYPE_TYPE_BOGUS)
      return NO;

   if (one->type != two->type)
      return NO;

   switch (one->type) {

    case MET_TYPE_TYPE_MAP:
      return MET_type_compatible(one->info.map.from,
				 two->info.map.from, allow_casts) &&
	     MET_type_compatible(one->info.map.to,
				 two->info.map.to, allow_casts);    

    case MET_TYPE_TYPE_PAIR:
      return MET_type_compatible(one->info.pair.left, two->info.pair.left,
				 allow_casts) &&
	 MET_type_compatible(one->info.pair.right, two->info.pair.right,
			     allow_casts);

    case MET_TYPE_TYPE_VECTOR:
      return MET_type_compatible(one->info.vector.of,
				 two->info.vector.of,
				 allow_casts) &&
	 (one->info.vector.is_row == two->info.vector.is_row) &&
	 (one->info.vector.dimension == two->info.vector.dimension ||
	  one->info.vector.dimension == TYPE_DIMENSION_UNKNOWN ||
	  two->info.vector.dimension == TYPE_DIMENSION_UNKNOWN);

    case MET_TYPE_TYPE_SET:
      return YES;

    case MET_TYPE_TYPE_FIELD:
      if (YES == allow_casts) {
	 result_of_cast = TYPE_CAST_FIELDS(one->info.field.type,
					   two->info.field.type);
	 return MET_FIELD_UNKNOWN != result_of_cast &&
	    two->info.field.type == result_of_cast;
      } else
	 return one->info.field.type == two->info.field.type;

    case MET_TYPE_TYPE_SET_OF:
      return MET_type_compatible(one->info.set_of.of, two->info.set_of.of,
				 allow_casts);

    default:
      DEFAULT_ERR(one->type);
   }

   /* make lint happy */
   return NO;
}

int
MET_type_count_data(type)
   METtype	*type;
{
   MME(MET_type_count_data);

   switch (type->type) {
    case MET_TYPE_TYPE_MAP:
      return 1;

    case MET_TYPE_TYPE_PAIR:
      return MET_type_count_data(type->info.pair.left) +
	 MET_type_count_data(type->info.pair.right);

    case MET_TYPE_TYPE_VECTOR:
      return MET_type_count_data(type->info.vector.of);

    case MET_TYPE_TYPE_SET:
      return 1;

    case MET_TYPE_TYPE_FIELD:
      return 1;

    case MET_TYPE_TYPE_SET_OF:
      return 1;

    default:
      DEFAULT_ERR(type->type);
   }

   /* make lint happy */
   return 0;
}
/*
 * return YES if there are unknown nodes in the passed type
 */
Boolean
MET_type_unknown(type)
   METtype	*type;
{
   MME(MET_type_unknown);

   if (MET_TYPE_TYPE_UNKNOWN == type->type)
      return YES;

   switch (type->type) {

    case MET_TYPE_TYPE_MAP:
      return MET_type_unknown(type->info.map.from) ||
	 MET_type_unknown(type->info.map.to);

    case MET_TYPE_TYPE_PAIR:
      return MET_type_unknown(type->info.pair.left) ||
	 MET_type_unknown(type->info.pair.right);

    case MET_TYPE_TYPE_VECTOR:
      return MET_type_unknown(type->info.vector.of) ||
	 type->info.vector.dimension == TYPE_DIMENSION_UNKNOWN;

    case MET_TYPE_TYPE_SET:
    case MET_TYPE_TYPE_BOGUS:
      return NO;

    case MET_TYPE_TYPE_FIELD:
      return type->info.field.type == MET_FIELD_UNKNOWN;

    case MET_TYPE_TYPE_SET_OF:
      return MET_type_unknown(type->info.set_of.of);

    default:
      DEFAULT_ERR(type->type);
   }

   /* make lint happy */
   return NO;
}
/*
 * return NO if there are bogus nodes in TYPE
 */
Boolean
MET_type_bogus(type)
   METtype	*type;
{
   MME(MET_type_unknown);

   if (MET_TYPE_TYPE_BOGUS == type->type)
      return YES;

   switch (type->type) {

    case MET_TYPE_TYPE_MAP:
      return MET_type_bogus(type->info.map.from) ||
	 MET_type_bogus(type->info.map.to);

    case MET_TYPE_TYPE_PAIR:
      return MET_type_bogus(type->info.pair.left) ||
	 MET_type_bogus(type->info.pair.right);

    case MET_TYPE_TYPE_VECTOR:
      return MET_type_bogus(type->info.vector.of);

    case MET_TYPE_TYPE_UNKNOWN:
    case MET_TYPE_TYPE_SET:
    case MET_TYPE_TYPE_FIELD:
      return NO;

    case MET_TYPE_TYPE_SET_OF:
      return MET_type_unknown(type->info.set_of.of);

    default:
      DEFAULT_ERR(type->type);
   }

   /* make lint happy */
   return NO;
}
/*
 * Create the smallest type larger than TWO that ONE can be cast into.
 * casting presently includes only bumping fields up the Z -> R
 * -> C ladder, and bumping types up the ? -> anything -> $ ladder.
 * In the future it might include converting between ordered
 * tuples of identical types and vectors.  Boolean returns true 
 * unless a new $ node appears in final output.  In case of conflict, 
 * action taken depends on the passed integer code;  the function will
 * either return a copy of the first node, return a copy of the second,
 * or build a bogus node that "spans" the two.  Pass one of 
 * TYPE_CAST_TAKE_FIRST, TYPE_CAST_TAKE_SECOND, & TYPE_CAST_MAKE_BOGUS.
 */
Boolean
MET_type_cast(one, two, result, conflict_res)
   METtype	*one, *two;
   METtype	**result;
   int		conflict_res;
{
   Boolean	rv;
   MME(MET_type_cast);
   CHECK_TYPE(one);
   CHECK_TYPE(two);
   
   if (one == two) {
      MET_TYPE_COPY(*result, one);
      return YES;
   }

   if (one->type == MET_TYPE_TYPE_UNKNOWN) {
      MET_TYPE_COPY(*result, two);
      return YES;
   }

   if (two->type == MET_TYPE_TYPE_UNKNOWN) {
      MET_TYPE_COPY(*result, one);
      return YES;
   }

   if (one->type != two->type) 
      goto problem;

   if (one->type != MET_TYPE_TYPE_FIELD) {
      MET_TYPE_ALLOC(*result);
      (*result)->type = one->type;
   }

   switch (one->type) {
      
    case MET_TYPE_TYPE_MAP:
      rv = MET_type_cast(one->info.map.from, two->info.map.from,
			 &(*result)->info.map.from, conflict_res);
      rv &= MET_type_cast(one->info.map.to, two->info.map.to,
			  &(*result)->info.map.to, conflict_res);
      if (one->info.map.to == (*result)->info.map.to &&
	  one->info.map.from == (*result)->info.map.from) {
	 MET_TYPE_FREE(*result);
	 MET_TYPE_COPY(*result, one);
      }
      return rv;
      
    case MET_TYPE_TYPE_PAIR:
      rv = MET_type_cast(one->info.pair.left, two->info.pair.left,
			 &(*result)->info.pair.left, conflict_res);
      rv &= MET_type_cast(one->info.pair.right, two->info.pair.right,
			  &(*result)->info.pair.right, conflict_res);
      if (one->info.pair.left == (*result)->info.pair.left &&
	  one->info.pair.right == (*result)->info.pair.right) {
	 MET_TYPE_FREE(*result);
	 MET_TYPE_COPY(*result, one);
      }
      return rv;
      
    case MET_TYPE_TYPE_VECTOR:
      if (TYPE_DIMENSION_UNKNOWN == one->info.vector.dimension) {
	 (*result)->info.vector.dimension = two->info.vector.dimension;
	 (*result)->info.vector.is_row = two->info.vector.is_row;
	 return MET_type_cast(one->info.vector.of, two->info.vector.of,
			      &(*result)->info.vector.of, conflict_res);
      }
      if (one->info.vector.dimension == two->info.vector.dimension ||
	  TYPE_DIMENSION_UNKNOWN == two->info.vector.dimension) {
	 (*result)->info.vector.dimension = one->info.vector.dimension;
	 (*result)->info.vector.is_row = one->info.vector.is_row;
	 rv = MET_type_cast(one->info.vector.of, two->info.vector.of,
			    &(*result)->info.vector.of, conflict_res);
	 if ((*result)->info.vector.of == one->info.vector.of) {
	    MET_TYPE_FREE(*result);
	    MET_TYPE_COPY(*result, one);
	 }
	 return rv;
      } 
      (*result)->info.vector.of = MET_NULL_TYPE;
      MET_TYPE_FREE(*result);
      break;
      
    case MET_TYPE_TYPE_SET:
      MET_TYPE_COPY(*result, one);
      return YES;
      
    case MET_TYPE_TYPE_FIELD:
      MET_TYPE_NEW_FIELD(*result, TYPE_CAST_FIELDS(one->info.field.type,
						   two->info.field.type));
      if (MET_FIELD_UNKNOWN != (*result)->info.field.type)
	 return YES;

      MET_TYPE_FREE(*result);
      break;
      
    case MET_TYPE_TYPE_SET_OF:
      return MET_type_cast(one->info.set_of.of, two->info.set_of.of,
			   &(*result)->info.set_of.of, conflict_res);
      
    default:
      DEFAULT_ERR(one->type);
   }

 problem:
   switch(conflict_res) {
    case TYPE_CAST_TAKE_FIRST:
      MET_TYPE_COPY(*result, one);
      return YES;
    case TYPE_CAST_TAKE_SECOND:
      MET_TYPE_COPY(*result, two);
      return YES;
    case TYPE_CAST_MAKE_BOGUS:
      MET_TYPE_NEW_BOGUS(*result);
      return NO;
    default:
      DEFAULT_ERR(conflict_res);
   }

   /* make lint happy */
   return YES;
}

/*
 * from two types create and return a third type which both types can be
 * cast into.  casting presently includes only bumping fields up the Z -> R
 * -> C ladder.  In the future it might include converting between ordered
 * tuples of identical types and vectors.  If the types are not compatible
 * at all, then return MET_NULL_TYPE.
 */
METtype *
MET_type_cast_maps(type, map_type)
   METtype	*type, *map_type;
{
   METtype	*result_type, *t1, *t2;
   MME(MET_type_cast_maps);
   CHECK_TYPE(type);
   CHECK_TYPE(map_type);
   
   if (type == map_type) {
      MET_TYPE_COPY(result_type, map_type);
      return result_type;
   }
   
   switch (type->type) {
      
    case MET_TYPE_TYPE_MAP:
      if (NO == MET_type_cast(type, map_type, &result_type, 
			      TYPE_CAST_MAKE_BOGUS)) {
	 MET_TYPE_FREE(result_type);
	 return MET_NULL_TYPE;
      }
      return result_type;
      
    case MET_TYPE_TYPE_PAIR:
      t1 = MET_type_cast_maps(type->info.pair.left, map_type);
      t2 = MET_type_cast_maps(type->info.pair.right, map_type);
      if (MET_NULL_TYPE == t1 || MET_NULL_TYPE == t2)
	 return MET_NULL_TYPE;
      else if (t1 == type->info.pair.left && t2 == type->info.pair.right) {
	 MET_TYPE_COPY(result_type, type);
      } else 
	 MET_TYPE_NEW_PAIR(result_type, t1, t2);
      MET_TYPE_FREE(t1);
      MET_TYPE_FREE(t2);
      return result_type;
      
    case MET_TYPE_TYPE_VECTOR:
      t1 = MET_type_cast_maps(type->info.vector.of, map_type);
      if (MET_NULL_TYPE == t1)
	 return MET_NULL_TYPE;
      if (t1 == type->info.vector.of) {
	 MET_TYPE_COPY(result_type, type);
      } else
	 MET_TYPE_NEW_VECTOR(result_type, t1,
			     type->info.vector.dimension,
			     type->info.vector.is_row);
      MET_TYPE_FREE(t1);
      return result_type;
      
    case MET_TYPE_TYPE_SET:
    case MET_TYPE_TYPE_FIELD:
    case MET_TYPE_TYPE_UNKNOWN:
    case MET_TYPE_TYPE_BOGUS:
      return MET_NULL_TYPE;

    case MET_TYPE_TYPE_SET_OF:
      t1 = MET_type_cast_maps(type->info.set_of.of, map_type);
      if (MET_NULL_TYPE == t1)
	 return MET_NULL_TYPE;
      else if (t1 == type->info.set_of.of) {
	 MET_TYPE_COPY(result_type, type);
      } else
	 MET_TYPE_NEW_SET_OF(result_type, t1);
      MET_TYPE_FREE(t1);
      return result_type;
      
    default:
      DEFAULT_ERR(type->type);
   }

   /* make lint happy */
   return MET_NULL_TYPE;
}

/*
 * see MET_type_to_data
 */
static METdata *
MET__type_to_data(type, num_verts, multiplicity)
   METtype	*type;
   int		num_verts;
   int		multiplicity;
{
   METverts	*verts;
   METdata	*left, *right, **chase;
   MME(MET__type_to_data);

   switch (type->type) {

    case MET_TYPE_TYPE_MAP:
      verts = MET_verts_new(MET_FIELD_MAP, num_verts, multiplicity);
      left = MET_data_new(verts, multiplicity, 0);
      MET_VERTS_FREE(verts);
      return left;
      
    case MET_TYPE_TYPE_PAIR:
      left = MET__type_to_data(type->info.pair.left, num_verts,
			       multiplicity);
      right = MET__type_to_data(type->info.pair.right, num_verts,
				multiplicity);
      /*
       * add right to the end of the list in left
       */
      chase = &left->next;
      while (MET_NULL_DATA != *chase)
	 chase = &(*chase)->next;
      
      MET_DATA_COPY(*chase, right);
      MET_DATA_FREE(right);

      return left;
      
    case MET_TYPE_TYPE_VECTOR:
      return MET__type_to_data(type->info.vector.of, num_verts,
			       multiplicity * type->info.vector.dimension);
      
    case MET_TYPE_TYPE_FIELD:
      verts = MET_verts_new(type->info.field.type, num_verts,
			    multiplicity);
      left = MET_data_new(verts, multiplicity, 0);
      MET_VERTS_FREE(verts);
      return left;
      
    case MET_TYPE_TYPE_SET_OF:
      verts = MET_verts_new(MET_FIELD_SET, num_verts, multiplicity);
      left = MET_data_new(verts, multiplicity, 0);
      MET_VERTS_FREE(verts);
      return left;
      
    case MET_TYPE_TYPE_UNKNOWN:
    case MET_TYPE_TYPE_BOGUS:
      FATALS("cannot convert unkown types to data");
      
    default:
      DEFAULT_ERR(type->type);
   }

   /* make lint happy */
   return MET_NULL_DATA;
}

/*
 * create a linked list of METdatas, each one with its own METverts.  The
 * types of the datas are controlled by the type.
 *
 * So, traverse the type.  map, field, and set_of nodes generate METdatas,
 * the other ones just affect the field at the METdata when it gets
 * generated.  row and column nodes multiply the width of the verts, pair
 * nodes cause recursion.
 */
METdata *
MET_type_to_data(type, num_verts)
   METtype	*type;
   int		num_verts;
{
   MME(MET_type_to_data);

   return MET__type_to_data(type, num_verts, 1);
}

/*
 * create and return a polybare set of the passed type with num_verts
 * in each of its METverts.  The work is really done by MET_type_to_data().
 */
METset *
MET_type_to_polybare_set(type, num_verts, tag_sets, num_sets)
   METtype	*type;
   int		num_verts;
   METset	**tag_sets;
   int		num_sets;
{
   int		i;
   METset	*result;
   MME(MET_type_to_polybare_set);

   result = MET_set_new(MET_SET_TYPE_BARE);
   result->info.bare.data = MET_type_to_data(type, num_verts);
   MET_TYPE_COPY(result->info.bare.type, type);
   result->info.bare.how_many = num_verts;
   MET_tags_initialize(&result->info.bare.tags_bare);
   MET_tags_initialize(&result->info.bare.tags_in);
   for (i=0; i < num_sets; i++) 
      if (MET_SET_TYPE_BARE == tag_sets[i]->type) {
	 MET_tags_copy(&result->info.bare.tags_bare, 
		       &tag_sets[i]->info.bare.tags_bare);
	 MET_tags_copy(&result->info.bare.tags_in, 
		       &tag_sets[i]->info.bare.tags_in);
      }

   return result;
}

/*
 * find and return the field at the root of the passed type.  if there is
 * more than one, return MET_FIELD_UNKNOWN.  If all_the_way is true,
 * return the field at the root of any set data, otherwise, just return
 * `set' in such cases.
 */
int
MET_type_base_field(type, all_the_way)
   METtype	*type;
   Boolean	all_the_way;
{
   MME(MET_type_base_field);

   switch(type->type) {

    case MET_TYPE_TYPE_MAP:
      return MET_FIELD_MAP;

    case MET_TYPE_TYPE_PAIR:
      return MET_FIELD_UNKNOWN;

    case MET_TYPE_TYPE_VECTOR:
      return MET_type_base_field(type->info.vector.of, all_the_way);

    case MET_TYPE_TYPE_SET:
      return MET_FIELD_SET;

    case MET_TYPE_TYPE_FIELD:
      return type->info.field.type;

    case MET_TYPE_TYPE_SET_OF:
      if (YES == all_the_way)
	 return MET_type_base_field(type->info.set_of.of, all_the_way);
      else
	 return MET_FIELD_SET;
      
    case MET_TYPE_TYPE_UNKNOWN:
      return MET_FIELD_UNKNOWN;

    case MET_TYPE_TYPE_BOGUS:
      return MET_FIELD_UNKNOWN;

    default:
      DEFAULT_ERR(type->type);
   }

   /* make lint happy */
   return MET_FIELD_UNKNOWN;
}

/*
 * call func with type-data pairs that correspond
 * return the sum of the return values
 */
int
MET_type_traverse_data(type, data, func, ptr)
   METtype	*type;
   METdata	**data;
   int		(*func)();
   char		*ptr;
{
   int		t;
   MME(MET_type_traverse_data);
 
  switch (type->type) {
    case MET_TYPE_TYPE_PAIR:
     t  = MET_type_traverse_data(type->info.pair.left,  data, func, ptr);
     t += MET_type_traverse_data(type->info.pair.right, data, func, ptr);
     return t;

    case MET_TYPE_TYPE_VECTOR:
    case MET_TYPE_TYPE_MAP:
    case MET_TYPE_TYPE_SET:
    case MET_TYPE_TYPE_FIELD:
    case MET_TYPE_TYPE_SET_OF:
      t = (func)(type, *data, ptr);
      *data = (*data)->next;
      return t;

    default:
      DEFAULT_ERR(type->type);
   }

   /* make lint happy */
   return 0;
}   

/*
 * create and return a block set of the passed type with num_verts in its
 * METverts.  Again, the work is done by MET_type_to_data();
 */
METset *
MET_type_to_block_set(type, num_verts, connex)
   METtype	*type;
   int		num_verts;
   METconnex	*connex;
{
   METset	*result;
   METblock	*block;
   MME(MET_type_to_block_set);
#ifdef MET__DEBUG
   if (MET_TYPE_TYPE_SET_OF != type->type)
      FATALS("type must be set_of");
#endif

   MET_BLOCK_ALLOC(block);
   MET_tags_initialize(&block->tags_in);
   MET_tags_initialize(&block->tags_part);
   block->next = MET_NULL_BLOCK;
   block->data = MET_type_to_data(type->info.set_of.of, num_verts);
   MET_TYPE_COPY(block->type, type);

   MET_CONNEX_COPY(block->connex, connex);
   MET_CONNEX_COPY(block->data_connex, connex);

   result = MET_set_new(MET_SET_TYPE_BLOCK);
   result->info.block.block_list = block;
   return result;
}

/*
 * cast components of base type to field
 * and return a new type.  It tries to be smart -- if the children don't
 * change, then it just shallow copies.
 */
METtype *
MET_type_cast_to_field(type, field)
   METtype     	*type;
   int		field;
{
   METtype	*baby_type, *t1, *t2;
   int		t;
   MME(MET_type_cast_to_field);

   switch (type->type) {
    case MET_TYPE_TYPE_MAP:
      t1 = MET_type_cast_to_field(type->info.map.from, field);
      t2 = MET_type_cast_to_field(type->info.map.to  , field);
      if (MET_NULL_TYPE == t1 || MET_NULL_TYPE == t2) {
	 baby_type = MET_NULL_TYPE;
	 break;
      } else if (type->info.map.from == t1 &&
	  type->info.map.to   == t2) {
	 MET_TYPE_COPY(baby_type, type);
      } else {
	 MET_TYPE_NEW_MAP(baby_type, t1, t2);
      }
      MET_TYPE_FREE(t1);
      MET_TYPE_FREE(t2);
      break;

    case MET_TYPE_TYPE_PAIR:
      t1 = MET_type_cast_to_field(type->info.pair.left, field);
      t2 = MET_type_cast_to_field(type->info.pair.right, field);
      if (MET_NULL_TYPE == t1 || MET_NULL_TYPE == t2) {
	 baby_type = MET_NULL_TYPE;
	 break;
      } else if (type->info.pair.left  == t1 &&
	  type->info.pair.right == t2) {
	 MET_TYPE_COPY(baby_type, type);
      } else {
	 MET_TYPE_NEW_PAIR(baby_type, t1, t2);
      }
      MET_TYPE_FREE(t1);
      MET_TYPE_FREE(t2);
      break;

    case MET_TYPE_TYPE_VECTOR:
      t1 = MET_type_cast_to_field(type->info.vector.of, field);
      if (MET_NULL_TYPE == t1)
	 baby_type = MET_NULL_TYPE;
      else if (type->info.vector.of == t1) {
	 MET_TYPE_COPY(baby_type, type);
      } else {
	 MET_TYPE_NEW_VECTOR(baby_type, t1,
			     type->info.vector.dimension,
			     type->info.vector.is_row);
      }
      MET_TYPE_FREE(t1);
      break;

    case MET_TYPE_TYPE_SET:
      baby_type = MET_NULL_TYPE;
      break;

    case MET_TYPE_TYPE_FIELD:
      if (type->info.field.type == field ||
	  type->info.field.type ==
	  (t = TYPE_CAST_FIELDS(type->info.field.type,
				field))) {
	 MET_TYPE_COPY(baby_type, type);
      } else if (MET_FIELD_UNKNOWN != t) {
	 MET_TYPE_NEW_FIELD(baby_type, t);
      } else 
	 baby_type = MET_NULL_TYPE;
      break;
      
    case MET_TYPE_TYPE_SET_OF:
      t1 = MET_type_cast_to_field(type->info.set_of.of, field);
      if (MET_NULL_TYPE == t1)
	 baby_type = MET_NULL_TYPE;
      else if (type->info.set_of.of == t1) {
	 MET_TYPE_COPY(baby_type, type);
      } else {
	 MET_TYPE_NEW_SET_OF(baby_type, t1);
      }
      MET_TYPE_FREE(t1);
      break;
      
    case MET_TYPE_TYPE_UNKNOWN:
      MET_TYPE_COPY(baby_type, type);
      break;

    case MET_TYPE_TYPE_BOGUS:
      MET_TYPE_COPY(baby_type, type);
      break;

    default:
      DEFAULT_ERR(type->type);
   }
   
   return baby_type;
}

METtype *
MET_type_of_type(type)
   METtype	*type;
{
   METtype	*result, *left, *right;
   MME(MET_type_of_type);

   switch (type->type) {
    case MET_TYPE_TYPE_MAP:
      left = MET_type_of_type(type->info.map.from);
      right = MET_type_of_type(type->info.map.to);
      break;
    case MET_TYPE_TYPE_PAIR:
      left = MET_type_of_type(type->info.pair.left);
      right = MET_type_of_type(type->info.pair.right);
      break;
    case MET_TYPE_TYPE_VECTOR:
      left = MET_type_of_type(type->info.vector.of);
      MET_TYPE_NEW_FIELD(right, MET_FIELD_INTEGER);
      break;
    case MET_TYPE_TYPE_SET_OF:
      left = MET_type_of_type(type->info.set_of.of);
      MET_TYPE_NEW_BOGUS(right);
      break;
    case MET_TYPE_TYPE_SET:
    case MET_TYPE_TYPE_FIELD:
    case MET_TYPE_TYPE_UNKNOWN:
    case MET_TYPE_TYPE_BOGUS:
      MET_TYPE_NEW_SET(result);
      return result;
    default:
      DEFAULT_ERR(type->type);
   }

   MET_TYPE_NEW_PAIR(result, left, right);
   MET_TYPE_FREE(left);
   MET_TYPE_FREE(right);
   return result;
}

static void
MET__type_print_pretty(buf, type, prec, is_right)
   char		*buf;
   METtype	*type;
   int		prec;
   Boolean	is_right;
{
   static int	precs[] =  {1, 0, 2, 3, 3, 3, 3};
   static int	assocs[] = {0, 1, 1, 1, 1, 1, 1};
   char		t[8192];
   Boolean	need_parens;
   int		my_prec;
   MME(MET__type_print_pretty);

   my_prec = precs[type->type];
   need_parens = my_prec < prec || (my_prec == prec &&
				    assocs[type->type] == is_right);
   if (YES == need_parens)
      (void) strcat(buf, "(");

   switch (type->type) {
    case MET_TYPE_TYPE_MAP:
      MET__type_print_pretty(buf, type->info.map.from, my_prec, NO);
      (void) strcat(buf, " -> ");
      MET__type_print_pretty(buf, type->info.map.to, my_prec, YES);
      break;

    case MET_TYPE_TYPE_PAIR:
      MET__type_print_pretty(buf, type->info.pair.left, my_prec, NO);
      (void) strcat(buf, ", ");
      MET__type_print_pretty(buf, type->info.pair.right, my_prec, YES);
      break;

    case MET_TYPE_TYPE_VECTOR:
      MET__type_print_pretty(buf, type->info.vector.of, my_prec, NO);
      if (TYPE_DIMENSION_UNKNOWN == type->info.vector.dimension) {
	 if (YES == type->info.vector.is_row)
	    (void) sprintf(t, "_?");
	 else
	    (void) sprintf(t, "^?");
      } else {
	 if (YES == type->info.vector.is_row)
	    (void) sprintf(t, "_%d", type->info.vector.dimension);
	 else
	    (void) sprintf(t, "^%d", type->info.vector.dimension);
      }
      (void) strcat(buf, t);
      break;

    case MET_TYPE_TYPE_SET:
      (void) strcat(buf, "SET");
      break;

    case MET_TYPE_TYPE_FIELD:
      switch(type->info.field.type) {
       case MET_FIELD_ZTWO:
	 (void) strcat(buf, "B");
	 break;
       case MET_FIELD_INTEGER:
	 (void) strcat(buf, "Z");
	 break;
       case MET_FIELD_REAL:
	 (void) strcat(buf, "R");
	 break;
       case MET_FIELD_COMPLEX:
	 (void) strcat(buf, "C");
	 break;
       case MET_FIELD_MAP:
	 (void) strcat(buf, "map");
	 break;
       case MET_FIELD_SET:
	 (void) strcat(buf, "set");
	 break;
       default:
	 DEFAULT_ERR(type->info.field.type);
      }
      break;

    case MET_TYPE_TYPE_SET_OF:
      (void) strcat(buf, "{");
      MET__type_print_pretty(buf, type->info.set_of.of, my_prec, NO);
      (void) strcat(buf, "}");
      break;

    case MET_TYPE_TYPE_UNKNOWN:
      (void) strcat(buf, "?");
      break;

    case MET_TYPE_TYPE_BOGUS:
      (void) strcat(buf, "$");
      break;

    default:
      DEFAULT_ERR(type->type);
   }
   if (YES == need_parens)
      (void) strcat(buf, ")");
}

void
METtype_print_pretty(stream, type, s, slen)
   FILE		*stream;
   METtype	*type;
   char		*s;
   int		slen;
{
   char		buf[8192];
   MME(METtype_print_pretty);

   buf[0] = '\0';
   MET__type_print_pretty(buf, type, -1, NO);

   if (FNULL != stream)
      (void) fprintf(stream, "%s", buf);
   if (CNULL != s)
      (void) strcat(s, buf);
}

Boolean
METtype_write(type, buf, len)
   METtype		*type;
   unsigned char	**buf;
   int			*len;
{
   MME(METtype_write);
   CHECK_TYPE(type);

   if (1 + sizeof(int) > *len)
      return FAILURE;

   WRITE_INTEGER(*buf, type->type);
   *len -= sizeof(int);

   switch(type->type) {
    case MET_TYPE_TYPE_MAP:
      return (SUCCESS == METtype_write(type->info.map.from, buf, len) &&
	      SUCCESS == METtype_write(type->info.map.to, buf, len));
    case MET_TYPE_TYPE_PAIR:
      return (SUCCESS == METtype_write(type->info.pair.left, buf, len) &&
	      SUCCESS == METtype_write(type->info.pair.right, buf, len));
    case MET_TYPE_TYPE_VECTOR:
      if (FAILURE == METtype_write(type->info.vector.of, buf, len) ||
	  sizeof(int) + sizeof(int) + 1 > *len)
	 return FAILURE;
      WRITE_INTEGER(*buf, type->info.vector.dimension);
      WRITE_INTEGER(*buf, type->info.vector.is_row);
      (*len) -= sizeof(int) + sizeof(int);
      break;
    case MET_TYPE_TYPE_FIELD:
      if (sizeof(int) > *len)
	 return FAILURE;
      WRITE_INTEGER(*buf, type->info.field.type);
      *len -= sizeof(int);
      break;
    case MET_TYPE_TYPE_SET_OF:
      return METtype_write(type->info.set_of.of, buf, len);
    case MET_TYPE_TYPE_SET:
    case MET_TYPE_TYPE_UNKNOWN:
    case MET_TYPE_TYPE_BOGUS:
      break;
    default:
      DEFAULT_ERR(type->type);
   }

   **buf = 0;
   return SUCCESS;
}

static void
MET__type_blank(type)
   METtype	*type;
{
   MME(MET__type_blank);
   switch (type->type) {
    case MET_TYPE_TYPE_MAP:
      type->info.map.from = type->info.map.to = MET_NULL_TYPE;
      break;
    case MET_TYPE_TYPE_PAIR:
      type->info.pair.left = type->info.pair.right = MET_NULL_TYPE;
      break;
    case MET_TYPE_TYPE_VECTOR:
      type->info.vector.of = MET_NULL_TYPE;
      break;
    case MET_TYPE_TYPE_SET:
      break;
    case MET_TYPE_TYPE_FIELD:
      break;
    case MET_TYPE_TYPE_SET_OF:
      type->info.set_of.of = MET_NULL_TYPE;
      break;
    case MET_TYPE_TYPE_UNKNOWN:
      break;
    case MET_TYPE_TYPE_BOGUS:
      break;
    default:
      DEFAULT_ERR(type->type);
   }
}
static METtype **
MET__type_next(type)
   METtype	*type;
{
   MME(MET__type_next);
   switch (type->type) {
    case MET_TYPE_TYPE_MAP:
      if (MET_NULL_TYPE == type->info.map.from)
	 return &type->info.map.from;
      else if (MET_NULL_TYPE == type->info.map.to)
	 return &type->info.map.to;
      else
	 break;
    case MET_TYPE_TYPE_PAIR:
      if (MET_NULL_TYPE == type->info.pair.left) 
	 return &type->info.pair.left;
      else if (MET_NULL_TYPE == type->info.pair.right)
	 return &type->info.pair.right;
      else
	 break;
    case MET_TYPE_TYPE_VECTOR:
      if (MET_NULL_TYPE == type->info.vector.of)
	 return &type->info.vector.of;
      else
	 break;
    case MET_TYPE_TYPE_SET:
      break;
    case MET_TYPE_TYPE_FIELD:
      break;
    case MET_TYPE_TYPE_SET_OF:
      if (MET_NULL_TYPE == type->info.set_of.of)
	 return &type->info.set_of.of;
      break;
    case MET_TYPE_TYPE_UNKNOWN:
      break;
    case MET_TYPE_TYPE_BOGUS:
      break;
    default:
      DEFAULT_ERR(type->type);
   }
   return (METtype **) NULL;
}
METtype *
METtype_read(buf, len, used, list, last, cont)
   unsigned char	*buf;
   int			len;
   int			*used;
   METlist		**list;
   MET_object_p		last;
   Boolean		cont;
{
   int		type;
   int		eaten;
   METtype	*result_type;
   METtype	**fill;
   METlist	*old_list;
   MET_object_p	little;
   MME(METtype_read);
   CHECK_NULL_LIST(*list);

   little.type = MET_NULL_TYPE;

   if (sizeof(int) > len)
      return MET_NULL_TYPE;

   if (NO == cont) {
      READ_INTEGER(buf, type);
      len -= sizeof(int);
      *used += sizeof(int);
   } else if (MET_NULL_LIST == *list) {
      MET_TYPE_COPY(result_type, last.type);
      return result_type;
   } else {
      type = (*list)->object.type->type;
   }

   switch (type) {
    case MET_TYPE_TYPE_MAP:
    case MET_TYPE_TYPE_PAIR:
    case MET_TYPE_TYPE_SET_OF:
    case MET_TYPE_TYPE_VECTOR:
      if (NO == cont) {
	 MET_TYPE_ALLOC(result_type);
	 result_type->type = type;
	 MET__type_blank(result_type);

	 if (MET_NULL_LIST != *list) {
	    MET_LIST_COPY(old_list, *list);
	    MET_LIST_FREE(*list);
	 } else
	    old_list = MET_NULL_LIST;
	 MET_LIST_ALLOC(*list);
	 if (MET_NULL_LIST != old_list) {
	    MET_LIST_COPY((*list)->next, old_list);
	    MET_LIST_FREE(old_list);
	 } else 
	    (*list)->next = MET_NULL_LIST;

	 (*list)->mm_type = MET__MM_TYPE_TYPE;
	 (*list)->aux_mm_type = MET__MM_TYPE_NONE;
	 MET_TYPE_COPY((*list)->object.type, result_type);
      } else {
	 MET_TYPE_COPY(result_type, (*list)->object.type);
      }
      
      while ((METtype **) NULL != (fill = MET__type_next(result_type))) {
	 if (MET_NULL_TYPE != last.type) {
	    MET_TYPE_COPY(*fill, last.type);
	    last.type = MET_NULL_TYPE;
	    continue;
	 }
	 eaten = 0;
	 *fill = METtype_read(buf, len, &eaten, list, little, FALSE);
	 *used += eaten;
	 buf += eaten;
	 len -= eaten;
	 if (MET_NULL_TYPE == *fill) {
	    MET_TYPE_FREE(result_type);
	    return MET_NULL_TYPE;
	 } 	
      }

      if (MET_TYPE_TYPE_VECTOR == result_type->type) {
	 if (sizeof(int) + sizeof(int) > len) {
	    MET_TYPE_FREE(result_type);
	    return MET_NULL_TYPE;
	 }
	 READ_INTEGER(buf, result_type->info.vector.dimension);
	 READ_INTEGER(buf, result_type->info.vector.is_row);
	 len -= sizeof(int) + sizeof(int);
	 *used += sizeof(int) + sizeof(int);
      }
	 
      if (MET_NULL_LIST != (*list)->next) {
	 MET_LIST_COPY(old_list, (*list)->next);
	 MET_LIST_FREE(*list);
	 MET_LIST_COPY(*list, old_list);
	 MET_LIST_FREE(old_list);
      } else {
	 MET_LIST_FREE(*list);
	 *list = MET_NULL_LIST;
      }

      return result_type;
	 
    case MET_TYPE_TYPE_SET:
      if (YES == cont || MET_NULL_TYPE != last.type)
	 break;
      MET_TYPE_NEW_SET(result_type);
      return result_type;
    case MET_TYPE_TYPE_FIELD:
      if (YES == cont || MET_NULL_TYPE != last.type)
	 break;
      if (sizeof(int) > len) {
	 (*used) -= sizeof(int);
	 return MET_NULL_TYPE;
      }
      READ_INTEGER(buf, type);
      MET_TYPE_NEW_FIELD(result_type, type);
      len -= sizeof(int);
      *used += sizeof(int);
      return result_type;
    case MET_TYPE_TYPE_UNKNOWN:
      if (YES == cont || MET_NULL_TYPE != last.type)
	 break;
      MET_TYPE_NEW_UNKNOWN(result_type);
      return result_type;
    case MET_TYPE_TYPE_BOGUS:
      if (YES == cont || MET_NULL_TYPE != last.type)
	 break;
      MET_TYPE_NEW_BOGUS(result_type);
      return result_type;
    default:
      break;
   }

   MET_LIST_FREE_NULL(*list);
   *list = MET_NULL_LIST;
   return MET_NULL_TYPE;

}

#ifdef MET__DEBUG_PRINT
void
METtype_print(stream, type, indent_level)
   FILE		*stream;
   METtype	*type;
   int		indent_level;
{
   MME(METtype_print);
   CHECK_NULL_TYPE(type);

   if (FNULL == stream)
      stream = stdout;

   MET_indent(stream, indent_level);
   if (MET_NULL_TYPE == type) {
      PRINT(stream, "type is null\n");
      return;
   }
   PRINT(stream, "type at %#x: type = %s, ref_count = %d, ",
	 type, MET_names_type_type[type->type], type->mminfo.ref_count);
   switch(type->type) {
      
    case MET_TYPE_TYPE_MAP:
      PRINT(stream, "from = %#x, to = %#x\n",
	    type->info.map.from, type->info.map.to);
      METtype_print(stream, type->info.map.from, indent_level + 1);
      METtype_print(stream, type->info.map.to,   indent_level + 1);
      break;
      
    case MET_TYPE_TYPE_PAIR:
      PRINT(stream, "left = %#x, right = %#x\n",
	    type->info.pair.left, type->info.pair.right);
      METtype_print(stream, type->info.pair.left,  indent_level + 1);
      METtype_print(stream, type->info.pair.right, indent_level + 1);
      break;
      
    case MET_TYPE_TYPE_VECTOR:
      if (TYPE_DIMENSION_UNKNOWN == type->info.vector.dimension)
	 PRINT(stream, "of = %#x, dimension = ?, %s\n",
	       type->info.vector.of,
	       type->info.vector.is_row ? "row" : "column");
      else
	 PRINT(stream, "of = %#x, dimension = %d, %s\n",
	       type->info.vector.of, type->info.vector.dimension,
	       type->info.vector.is_row ? "row" : "column");
      METtype_print(stream, type->info.vector.of, indent_level + 1);
      break;
      
    case MET_TYPE_TYPE_SET:
      PRINT(stream, "\n");
      break;
      
    case MET_TYPE_TYPE_FIELD:
      PRINT(stream, "type = %s\n",
	    MET_names_field_type[type->info.field.type]);
      break;
      
    case MET_TYPE_TYPE_SET_OF:
      PRINT(stream, "of = %#x\n", type->info.set_of.of);
      METtype_print(stream, type->info.set_of.of, indent_level + 1);
      break;
      
    case MET_TYPE_TYPE_UNKNOWN:
      PRINT(stream, "\n");
      break;
      
    case MET_TYPE_TYPE_BOGUS:
      PRINT(stream, "\n");
      break;

    default:
      DEFAULT_ERR(type->type);
   }
}

Boolean
MET__type_has_ref(type, mm_type, object)
   METtype	*type;
   int		mm_type;
   MET_object	*object;
{
   MME(MET__type_has_ref);

   switch(mm_type) {
    case MET__MM_TYPE_SET:
      break;
      
    case MET__MM_TYPE_TYPE:
      switch (type->type) {

       case MET_TYPE_TYPE_MAP:
	 if ((METtype *) object == type->info.map.from ||
	     (METtype *) object == type->info.map.to)
	    return YES;
	 break;

       case MET_TYPE_TYPE_PAIR:
	 if ((METtype *) object == type->info.pair.left ||
	     (METtype *) object == type->info.pair.right)
	    return YES;
	 break;

       case MET_TYPE_TYPE_VECTOR:
	 if ((METtype *) object == type->info.vector.of)
	    return YES;
	 break;

       case MET_TYPE_TYPE_SET:
       case MET_TYPE_TYPE_FIELD:
	 break;

       case MET_TYPE_TYPE_SET_OF:
	 if ((METtype *) object == type->info.set_of.of)
	    return YES;
	 break;

       case MET_TYPE_TYPE_UNKNOWN:
       case MET_TYPE_TYPE_BOGUS:
	 break;

       default:
	 DEFAULT_ERR(type->type);
	 
      }
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


#define FUNC MET__type_new_free_list
#define LIST MET__type_free_list
#define TYPE METtype
#define MM_TYPE MET__MM_TYPE_TYPE
#include "new_free_list.c"
#undef FUNC
#undef LIST
#undef TYPE
#undef MM_TYPE
