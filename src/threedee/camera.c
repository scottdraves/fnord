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

#include "threedee/threedee.h"

#ifndef lint
static char Version[] = "$Id: camera.c,v 1.4 1992/06/22 17:11:24 mas Exp $";
#endif /* undef lint */

#define NORM(x,y,z) (SQRT((x)*(x)+(y)*(y)+(z)*(z)))
#define VECTOR_DISTANCE(v0,v1) NORM( \
	(v1)[0]-(v0)[0], (v1)[1]-(v0)[1], (v1)[2]-(v0)[2])
#define EPS (1e-7)

/* This is the quick and dirty function that resets the xform component of 
   a camera structure to reflect the changes in the more abstract
   quantities that are more useful to specify. */

void CameraBuildXform(cam)
   Camera cam;
{
   Real mat[3][3];
   ME(CameraBuildXform);

   spin_to_matrix(mat, cam->spin);
   
   cam->xform[0][0] = mat[0][0];
   cam->xform[0][1] = mat[0][1];
   cam->xform[0][2] = mat[0][2];
   cam->xform[1][0] = mat[1][0];
   cam->xform[1][1] = mat[1][1];
   cam->xform[1][2] = mat[1][2];
   cam->xform[2][0] = mat[2][0];
   cam->xform[2][1] = mat[2][1];
   cam->xform[2][2] = mat[2][2];
   
   cam->xform[0][3] = 
      (mat[0][0] * cam->pretrans[0] +
       mat[0][1] * cam->pretrans[1] +
       mat[0][2] * cam->pretrans[2]);
   cam->xform[1][3] = 
      (mat[1][0] * cam->pretrans[0] +
       mat[1][1] * cam->pretrans[1] +
       mat[1][2] * cam->pretrans[2]);
   cam->xform[2][3] = 
      (mat[2][0] * cam->pretrans[0] +
       mat[2][1] * cam->pretrans[1] +
       mat[2][2] * cam->pretrans[2]);
   
   cam->xform[0][0] *= cam->scale[0];
   cam->xform[0][1] *= cam->scale[0];
   cam->xform[0][2] *= cam->scale[0];
   cam->xform[0][3] *= cam->scale[0];
   cam->xform[1][0] *= cam->scale[1];
   cam->xform[1][1] *= cam->scale[1];
   cam->xform[1][2] *= cam->scale[1];
   cam->xform[1][3] *= cam->scale[1];
   cam->xform[2][0] *= cam->scale[2];
   cam->xform[2][1] *= cam->scale[2];
   cam->xform[2][2] *= cam->scale[2];
   cam->xform[2][3] *= cam->scale[2];
   
   cam->xform[0][3] += cam->posttrans[0];
   cam->xform[1][3] += cam->posttrans[1];
   cam->xform[2][3] += cam->posttrans[2];
}


/*
 * forward order of xforms:
 * first pretrans rotate scale posttrans last
 */
void
CameraGetInverseXform(cam, xform)
   Camera	cam;
   Real		xform[3][4];
{
   Real		mat[3][3];
   Real		a,b,c,d,e,f;
   Spin		spin;
   ME(CameraGetInverseXform);

   spin[0] = cam->spin[0];
   spin[1] = cam->spin[1];
   spin[2] = cam->spin[2];
   spin[3] = - cam->spin[3];

   spin_to_matrix(mat, spin);

   /*
    * these are the entries of the sparse matrix
    *   d00a
    *   0e0b
    *   00fc
    */
   d = 1.0 / cam->scale[0];
   e = 1.0 / cam->scale[1];
   f = 1.0 / cam->scale[2];
   a = - cam->posttrans[0] * d;
   b = - cam->posttrans[1] * e;
   c = - cam->posttrans[2] * f;

   /*
    * now multiply in the rotate matrix by hand, using sparsity
    */

   xform[0][0] = mat[0][0] * d;
   xform[0][1] = mat[0][1] * e;
   xform[0][2] = mat[0][2] * f;
   xform[0][3] = (mat[0][0] * a + mat[0][1] * b + mat[0][2] * c);
   
   xform[1][0] = mat[1][0] * d;
   xform[1][1] = mat[1][1] * e;
   xform[1][2] = mat[1][2] * f;
   xform[1][3] = (mat[1][0] * a + mat[1][1] * b + mat[1][2] * c);
   
   xform[2][0] = mat[2][0] * d;
   xform[2][1] = mat[2][1] * e;
   xform[2][2] = mat[2][2] * f;
   xform[2][3] = (mat[2][0] * a + mat[2][1] * b + mat[2][2] * c);

   /*
    * now add in the pretranslate
    */
   
   xform[0][3] -= cam->pretrans[0];
   xform[1][3] -= cam->pretrans[1];
   xform[2][3] -= cam->pretrans[2];
   
}


/* Makes a new center of rotation for the object at the center of its */
/* extent. Neither this function nor the next ones will recompute the xform */
/* based on any changes to the pict, so be sure to call CameraBuildXform if */
/* you need the xform after any camera changing calls. */

void CameraSetInterest (cam, pt)
   Camera cam;
   Real pt[3];
{
   ME(CameraSetInterest);
   
   cam->pretrans[0] = - pt[0];
   cam->pretrans[1] = - pt[1];
   cam->pretrans[2] = - pt[2];
   
}

/* Get the point at which the camera is looking */

void CameraGetInterest (cam, pt)
   Camera cam;
   Real pt[3];
{
   ME(CameraGetInterest);
   
   pt[0] = - cam->pretrans[0];
   pt[1] = - cam->pretrans[1];
   pt[2] = - cam->pretrans[2];

}

/* Get the rotation matrix of the camera */

void CameraGetSpin (cam, spin)
   Camera cam;
   Spin spin;
{
   spin[0] = cam->spin[0];
   spin[1] = cam->spin[1];
   spin[2] = cam->spin[2];
   spin[3] = cam->spin[3];
}

/* Set the spin matrix of the camera */

void CameraSetSpin (cam, spin)
   Camera cam;
   Spin spin;
{
   cam->spin[0] = spin[0];
   cam->spin[1] = spin[1];
   cam->spin[2] = spin[2];
   cam->spin[3] = spin[3];
}

/* Rescales and repositions the object so that the extent of the object is
   as close as possible to the extent of the bounds.  Note that while it
   does adjust the size of the object, it will not change the point being
   displayed in the center of the bounds.  In other words, while making the
   picture bigger, it will not change what you're looking at. */

void CameraRescale (cam, object_min, object_max, bounds_min, bounds_max)
   Camera cam;
   Real object_min[3], object_max[3], bounds_min[3], bounds_max[3];
{
   Real center[3];
   Real scale[3];
   Real size;
   ME(CameraRescale);
   
   center[0] = 0.5 * (object_min[0] + object_max[0]);
   center[1] = 0.5 * (object_min[1] + object_max[1]);
   center[2] = 0.5 * (object_min[2] + object_max[2]);
   
   size = 2.0 * VECTOR_DISTANCE(center, object_min);

   if (size < EPS) size = EPS;
   
   scale[0] = (bounds_max[0] - bounds_min[0]) / size;
   scale[1] = (bounds_max[1] - bounds_min[1]) / size;
   scale[2] = (bounds_max[2] - bounds_min[2]) / size;

   /* use the smaller of the x and y scales */
   scale[0] = scale[1] = MIN(scale[0], scale[1]);

   cam->pretrans[0] = - center[0];
   cam->pretrans[1] = - center[1];
   cam->pretrans[2] = - center[2];

   cam->scale[0] = scale[0];
   cam->scale[1] = scale[1];
   cam->scale[2] = scale[2];
   
   cam->posttrans[0] = (bounds_max[0] + bounds_min[0]) / 2.0;
   cam->posttrans[1] = (bounds_max[1] + bounds_min[1]) / 2.0;
   cam->posttrans[2] = (bounds_max[2] + bounds_min[2]) / 2.0;

}
