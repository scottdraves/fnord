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

/**********************************************************************
 *
 *	float.h : defines and declarations which enable the use of
 *		either single or double precision floating point
 *		numbers
 *
 **********************************************************************/
#ifndef _FLOAT_H_
#define _FLOAT_H_

/* $Id: float.h,v 1.8 1992/06/22 17:03:53 mas Exp $ */

#ifndef FLOAT_TYPE
#define FLOAT_TYPE double
#endif

typedef FLOAT_TYPE Real;

/*
 * math stuff
 */
#include <math.h>

#if defined(IRIX) && (FLOAT_TYPE == float)

#define ANINT(X)	(ffloor((float)(X)+0.5))
#define IRINT(X)	((int)rint((double)(X)))
#define FABS(X)		((float)fabs((double)(X)))
#define RINT(X)		((float)rint((double)(X)))
#define FLOOR(X)	(ffloor((float)(X)))
#define CEIL(X)		(fceil((float)(X)))
#define SQRT(X)		(fsqrt((float)(X)))
#define CBRT(X)		((float)cbrt((double)(X)))
#define ERF(X)		((float)erf((double)(X)))
#define ERFC(X)		((float)erfc((double)(X)))

#define SIN(X)		(fsin((float)(X)))
#define COS(X)		(fcos((float)(X)))
#define TAN(X)		(ftan((float)(X)))
#define ASIN(X)		(fasin((float)(X)))
#define ACOS(X)		(facos((float)(X)))
#define ATAN(X)		(fatan((float)(X)))
#define SINH(X)		(fsinh((float)(X)))
#define COSH(X)		(fcosh((float)(X)))
#define TANH(X)		(ftanh((float)(X)))
#define ASINH(X)	((float)asinh((double)(X)))
#define ACOSH(X)	((float)acosh((double)(X)))
#define ATANH(X)	((float)atanh((double)(X)))

#define EXP(X)		(fexp((float)(X)))
#define EXPM1(X)	(expm1((float)(X)))
#define EXP2(X)		((float)exp2((double)(X)))
#define EXP10(X)	((float)exp10((double)(X)))
#define LOG(X)		(flog((float)(X)))
#define LOG1P(X)	(flog1p((float)(X)))
#define LOG2(X)		((float)log2((double)(X)))
#define LOG10(X)	(flog10((float)(X)))

#define POW(X,Y)	((float)pow((double)(X),(double)(Y)))
#define HYPOT(X,Y)	(fhypot((float)(X),(float)(Y)))
#define ATAN2(X,Y)	(fatan2((float)(X),(float)(Y)))

#else

#ifdef SunOS
#define ANINT(X)	((FLOAT_TYPE)anint((double)(X)))
#define IRINT(X)	(irint((double)(X)))
#else
#define ANINT(X)	((FLOAT_TYPE)floor((double)(X)+0.5))
#define IRINT(X)	((int)rint((double)(X)))
#endif

#define FABS(X)		((FLOAT_TYPE)fabs((double)(X)))
#define RINT(X)		((FLOAT_TYPE)rint((double)(X)))
#define FLOOR(X)	((FLOAT_TYPE)floor((double)(X)))
#define CEIL(X)		((FLOAT_TYPE)ceil((double)(X)))
#define SQRT(X)		((FLOAT_TYPE)sqrt((double)(X)))
#define CBRT(X)		((FLOAT_TYPE)cbrt((double)(X)))
#define ERF(X)		((FLOAT_TYPE)erf((double)(X)))
#define ERFC(X)		((FLOAT_TYPE)erfc((double)(X)))

#define SIN(X)		((FLOAT_TYPE)sin((double)(X)))
#define COS(X)		((FLOAT_TYPE)cos((double)(X)))
#define TAN(X)		((FLOAT_TYPE)tan((double)(X)))
#define ASIN(X)		((FLOAT_TYPE)asin((double)(X)))
#define ACOS(X)		((FLOAT_TYPE)acos((double)(X)))
#define ATAN(X)		((FLOAT_TYPE)atan((double)(X)))
#define SINH(X)		((FLOAT_TYPE)sinh((double)(X)))
#define COSH(X)		((FLOAT_TYPE)cosh((double)(X)))
#define TANH(X)		((FLOAT_TYPE)tanh((double)(X)))
#define ASINH(X)	((FLOAT_TYPE)asinh((double)(X)))
#define ACOSH(X)	((FLOAT_TYPE)acosh((double)(X)))
#define ATANH(X)	((FLOAT_TYPE)atanh((double)(X)))

#define EXP(X)		((FLOAT_TYPE)exp((double)(X)))
#define EXPM1(X)	((FLOAT_TYPE)expm1((double)(X)))
#define EXP2(X)		((FLOAT_TYPE)exp2((double)(X)))
#define EXP10(X)	((FLOAT_TYPE)exp10((double)(X)))
#define LOG(X)		((FLOAT_TYPE)log((double)(X)))
#define LOG1P(X)	((FLOAT_TYPE)log1p((double)(X)))
#define LOG2(X)		((FLOAT_TYPE)log2((double)(X)))
#define LOG10(X)	((FLOAT_TYPE)log10((double)(X)))

#define POW(X,Y)	((FLOAT_TYPE)pow((double)(X),(double)(Y)))
#define HYPOT(X,Y)	((FLOAT_TYPE)hypot((double)(X),(double)(Y)))
#define ATAN2(X,Y)	((FLOAT_TYPE)atan2((double)(X),(double)(Y)))

#endif /* IRIX && FLOAT_TYPE */

#endif /* undef(_FLOAT_H_) */
