/*****************************************************************************

	PixelEditor.h

	This file contains is the public .h file for the Pixel Editor
	widget.

	October 14, 1990 by Brian Totty

******************************************************************************/

/*
 * Copyright 1990 Brian Totty
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Brian Totty or
 * University of Illinois not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  Brian Totty and University of Illinois make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Brian Totty and University of Illinois disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall Brian Totty or
 * University of Illinois be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 * 
 * 	totty@cs.uiuc.edu
 * 	
 */ 

#ifndef _PIXEL_EDITOR_H_
#define _PIXEL_EDITOR_H_

#define	TOOL_PENCIL		0
#define	TOOL_BRUSH		1
#define	TOOL_ERASER		2
#define	TOOL_REGION		3
#define	TOOL_PASTER		4

#define	BRUSH_SMALL_BOX		0
#define	BRUSH_MEDIUM_BOX	1
#define	BRUSH_LARGE_BOX		2
#define	BRUSH_SMALL_DIAMOND	3
#define	BRUSH_MEDIUM_DIAMOND	4
#define	BRUSH_LARGE_DIAMOND	5
#define	BRUSH_VERT_LINE		6
#define	BRUSH_HOR_LINE		7
#define	BRUSH_NEG_DIAG_LINE	8
#define	BRUSH_POS_DIAG_LINE	9
#define	BRUSH_CIRCLE		10
#define	BRUSH_USER_DEFINED	-1

/*---------------------------------------------------------------------------*

         E X P O R T E D    F U N C T I O N    D E C L A R A T I O N S

 *---------------------------------------------------------------------------*/

void				PixelEditorRegionFlipHorizontally();
void				PixelEditorRegionFlipVertically();
void				PixelEditorRegionFill();
Boolean				PixelEditorGetGrid();
int				PixelEditorGetZoom();
Pixel				PixelEditorGetFGColor();
Pixel				PixelEditorGetBGColor();
Boolean				PixelEditorRegionCopy();
void				PixelEditorRegionCut();
Boolean				PixelEditorChangeTool();
void				PixelEditorSetImage();
void				PixelEditorResizeImage();
Pixel				*PixelEditorBitmapDataToPixels();
unsigned char			*PixelEditorPixelsToBitmapData();
void				PixelEditorSetBitmapData();
Boolean				PixelEditorLoadBitmapFile();
Boolean				PixelEditorSaveBitmapFile();

/*---------------------------------------------------------------------------*

                 C O M P A T I B I L I T Y    D E F I N E S

 *---------------------------------------------------------------------------*/

#define	XtNcolor			XtNdrawColor
#define	PixelEditorGetColor		PixelEditorGetFGColor

/*---------------------------------------------------------------------------*

             R E S O U R C E    N A M E    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

#define	XtNzoom				"zoom"
#define	XtNdrawColor			"drawColor"
#define	XtNeraseColor			"eraseColor"
#define	XtNleftClipper			"leftClipper"
#define	XtNrightClipper			"rightClipper"
#define	XtNtopClipper			"topClipper"
#define	XtNbottomClipper		"bottomClipper"
#define	XtNclipperWidth			"clipperWidth"
#define	XtNclipperHeight		"clipperHeight"
#define	XtNgrid				"grid"
#define	XtNclippers			"clippers"
#define	XtNautoScale			"autoScale"
#define	XtNopaquePaste			"opaquePaste"
#define	XtNtool				"tool"
#define	XtNbrush			"brush"
#define	XtNdataChangeCallback		"dataChangeCallback"
#define	XtNregionChangeCallback		"regionChangeCallback"
#define	XtNpasteBufferChangeCallback	"pasteBufferChangeCallback"
#define	XtNimageWidth			"iW"
#define	XtNimageHeight			"imageHeight"
#define	XtNimagePixels			"imagePixels"
#define	XtNimageShared			"imageShared"

#define	XtCTool				"Tool"
#define	XtCBrush			"Brush"
#define	XtCPixelArray			"PixelArray"

#define	XtRTool				"Tool"
#define	XtRBrush			"Brush"

#define	XtNtoolPencil			"Pencil"
#define	XtNtoolBrush			"Brush"
#define	XtNtoolEraser			"Eraser"
#define	XtNtoolRegion			"Region"
#define	XtNtoolPaster			"Paster"

#define	XtNbrushSmallBox		"SmallBox"
#define	XtNbrushMediumBox		"MediumBox"
#define	XtNbrushLargeBox		"LargeBox"
#define	XtNbrushSmallDiamond		"SmallDiamond"
#define	XtNbrushMediumDiamond		"MediumDiamond"
#define	XtNbrushLargeDiamond		"LargeDiamond"
#define	XtNbrushVertLine		"VertLine"
#define	XtNbrushHorizLine		"HorizLine"
#define	XtNbrushNegDiagLine		"NegDiagLine"
#define	XtNbrushPosDiagLine		"PosDiagLine"
#define	XtNbrushCircle			"Circle"
#define	XtNbrushUserDefined		"UserDefined"

/*---------------------------------------------------------------------------*

              W I D G E T    &    C L A S S    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

typedef struct _PixelEditorRec		*PixelEditorWidget;
typedef struct _PixelEditorClassRec	*PixelEditorWidgetClass;

extern WidgetClass			pixelEditorWidgetClass;

/*---------------------------------------------------------------------------*

             C A L L B A C K    R E T U R N    S T R U C T U R E

 *---------------------------------------------------------------------------*/

typedef struct _PixelEditorReturnStruct
{
	int dummy;
} PixelEditorReturnStruct;

#endif

