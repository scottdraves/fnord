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


/* $Id: threedee.h,v 1.9 1992/06/22 17:11:26 mas Exp $ */

#ifndef THREEDEEH
#define THREEDEEH

#include "global/global.h"
#include "met/met.h"
#include "widgets/spin.h"

typedef struct _DrawEnvRec *DrawEnv;

extern DrawEnv EnvCreate ( /* data specific to type */ );

extern void EnvDrawAdd ( /* env, pict, mode */ );
#define ENV_NO_MODE 0
#define ENV_PIXMAP_MODE 1
#define ENV_COLORMAP_MODE 2
#define ENV_PIXMAP_COLORMAP_MODE 3
#define ENV_BW_POSTSCRIPT_MODE 4
#define ENV_GRAY_POSTSCRIPT_MODE 5
#define ENV_COLOR_POSTSCRIPT_MODE 6
#define ENV_DUMB_MODE 7

typedef struct _CameraRec {
   Real pretrans[3];
  
   Spin spin;
   Real scale[3];
   Real posttrans[3];

   Real xform[3][4];
} CameraRec, *Camera;


extern void EnvFlush ( /* env */ );
extern void EnvDestroy ( /* env */ );
extern FOboolean EnvRealizeColors ( /* env, model */ );
extern FOboolean EnvFreeColors ( /* env, model */ );

typedef struct _PictRec *Pict;

extern Pict PictCreate ( /* env, data */ );
extern void PictAddObject ( /* pict, obj */ );
extern void PictRemoveObject ( /* pict, obj */ );
extern FOboolean PictUpdate ( /* pict */ );
extern void PictRescale ( /* pict, mins, maxs */ );
extern void PictDefaultScale ( /* pict */ );
extern void PictSort ( /* pict */ );
extern void PictOutput ( /* pict */ );
extern void PictChangeDrawMode ( /* pict */ );
extern void PictSetDrawStyle ( /* pict, style */);
extern void PictSetSpin ( /* pict, spin */ );
extern void PictDestroy ( /* pict */ );
extern Camera PictGetCamera ( /* pict */ );

typedef struct _ViewObjRec *ViewObj;

extern ViewObj VOCreate ( /* set, string */ );
extern FOboolean VOUpdate ( /* view */ );
extern void VODestroy ( /* view */ );


extern void CameraBuildXform ( /* cam */ );
extern void CameraSetInterest ( /* cam, pt */ );
extern void CameraSetSpin ( /* cam, spin */ );
extern void CameraRescale ( /* cam, omin, omax, bmin, bmax */ );
extern void CameraGetInterest ( /* cam, pt */ );
extern void CameraGetSpin ( /* cam, spin */ );
extern void CameraGetInverseXform( /* cam, xform */ );

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)   ((a>0)?a:-(a))
#endif

#endif
