/****************************************************************************

	PixelEditor.c

	This file implements the Pixel Editor widget.

	October 14, 1990, Brian Totty

 ****************************************************************************/

#ifndef lint
static char Version[]=
   "$Id: PixelEditor.c,v 1.4 1991/09/30 14:38:35 mas Exp $";
#endif

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

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/cursorfont.h>
#include "PixelEditorP.h"
#include "PixelEditor.h"

/*===========================================================================*

           D E F I N I T I O N S    A N D    D E C L A R A T I O N S

 *===========================================================================*/

#define XtRFloat	"Float"

#define	FontAscent(f)	((f)->max_bounds.ascent)
#define	FontDescent(f)	((f)->max_bounds.descent)
#define	FontH(f)	(FontAscent(f) + FontDescent(f))
#define	FontW(f,s)	XTextWidth(f,s,strlen(s))
#define	FontMaxCharW(f)	((f)->max_bounds.rbearing-(f)->min_bounds.lbearing+1)

#define max(a,b)	((a) > (b) ? (a) : (b))
#define min(a,b)	((a) < (b) ? (a) : (b))
#define abs(a)		((a) < 0 ? -(a) : (a))
#define	even(x)		((x) % 2 == 0)
#define	odd(x)		((x) % 2 == 1)
#define	sign(x)		((x) < 0 ? -1 : 1)
#define	square(x)	((x) * (x))
#define	swap(x,y)	{int t; t = x; x = y; y = t;}

#define CLIPPERW	20
#define CLIPPERH	20
#define	FLASH_TIME	250

/*===========================================================================*

                           L O C A L    D A T A

 *===========================================================================*/

static char pad_stipple_data[32] =
{
	85, 85, 170, 170, 85, 85, 170, 170,
	85, 85, 170, 170, 85, 85, 170, 170,
	85, 85, 170, 170, 85, 85, 170, 170,
	85, 85, 170, 170, 85, 85, 170, 170
};
static char clipped_pixel_stipple_data[32] =
{
	136, 136, 0, 0, 34, 34, 0, 0,
	136, 136, 0, 0, 34, 34, 0, 0,
	136, 136, 0, 0, 34, 34, 0, 0,
	136, 136, 0, 0, 34, 34, 0, 0
};

static char dash_list[] = {8,4};

/*===========================================================================*

              C U R S O R    I M A G E    D E F I N I T I O N S

 *===========================================================================*/

#define draw_pencil_width 16
#define draw_pencil_height 16
#define draw_pencil_x_hot 1
#define draw_pencil_y_hot 1

static char draw_pencil_bits[] =
{
	0x00, 0x00, 0x06, 0x00, 0x1e, 0x00, 0x64, 0x00, 0xc4, 0x00, 0x28, 0x01,
	0x18, 0x02, 0x10, 0x04, 0x20, 0x08, 0x40, 0x18, 0x80, 0x3c, 0x00, 0x5f,
	0x00, 0x4e, 0x00, 0x44, 0x00, 0x38, 0x00, 0x00
};

static char draw_pencil_mask_bits[] =
{
	0x0f, 0x00, 0x3f, 0x00, 0xff, 0x00, 0xff, 0x01, 0xfe, 0x03, 0xfe, 0x07,
	0xfc, 0x0f, 0xfc, 0x1f, 0xf8, 0x3f, 0xf0, 0x7f, 0xe0, 0xff, 0xc0, 0xff,
	0x80, 0xff, 0x00, 0xff, 0x00, 0xfe, 0x00, 0x7c
};

#define erase_pencil_width 16
#define erase_pencil_height 16
#define erase_pencil_x_hot 1
#define erase_pencil_y_hot 1

static char erase_pencil_bits[] =
{
	0x00, 0x00, 0x1c, 0x00, 0x22, 0x00, 0x72, 0x00, 0xfa, 0x00, 0x3c, 0x01,
	0x18, 0x02, 0x10, 0x04, 0x20, 0x08, 0x40, 0x14, 0x80, 0x12, 0x00, 0x21,
	0x00, 0x26, 0x00, 0x78, 0x00, 0x60, 0x00, 0x00
};

static char erase_pencil_mask_bits[] =
{
	0x3e, 0x00, 0x7f, 0x00, 0xff, 0x00, 0xff, 0x01, 0xff, 0x03, 0xff, 0x07,
	0xfe, 0x0f, 0xfc, 0x1f, 0xf8, 0x3f, 0xf0, 0x3f, 0xe0, 0x7f, 0xc0, 0x7f,
	0x80, 0xff, 0x00, 0xff, 0x00, 0xfc, 0x00, 0xf0
};

#define brush_width 16
#define brush_height 16
#define brush_x_hot 2
#define brush_y_hot 1

static char brush_bits[] =
{
	0x00, 0x00, 0x04, 0x00, 0x0c, 0x00, 0x3c, 0x00, 0x58, 0x00, 0xa8, 0x00,
	0x90, 0x00, 0xe0, 0x01, 0x80, 0x03, 0x00, 0x07, 0x00, 0x0e, 0x00, 0x1c,
	0x00, 0x38, 0x00, 0x70, 0x00, 0x60, 0x00, 0x00
};

static char brush_mask_bits[] =
{
	0x0e, 0x00, 0x1e, 0x00, 0x7e, 0x00, 0xfe, 0x00, 0xfe, 0x01, 0xfc, 0x01,
	0xfc, 0x03, 0xf8, 0x07, 0xf0, 0x0f, 0xc0, 0x1f, 0x80, 0x3f, 0x00, 0x7f,
	0x00, 0xfe, 0x00, 0xfc, 0x00, 0xf8, 0x00, 0xf0
};

#define hand_width 16
#define hand_height 16
#define hand_x_hot 8
#define hand_y_hot 8

static char hand_bits[] =
{
	0x00, 0x00, 0x00, 0x01, 0x40, 0x05, 0xa0, 0x0a, 0xa0, 0x1a, 0xa0, 0x2a,
	0xa0, 0x2a, 0x2e, 0x20, 0x12, 0x20, 0x04, 0x20, 0x08, 0x10, 0x10, 0x10,
	0x20, 0x10, 0x20, 0x10, 0xe0, 0x1f, 0x00, 0x00
};

static char hand_mask_bits[] =
{
	0x80, 0x03, 0xe0, 0x0f, 0xf0, 0x1f, 0xf0, 0x3f, 0xf0, 0x7f, 0xf0, 0x7f,
	0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xfe, 0x7f, 0xfc, 0x3f,
	0xf8, 0x3f, 0xf0, 0x3f, 0xf0, 0x3f, 0xf0, 0x3f
};

#define eraser_width 16
#define eraser_height 16
#define eraser_x_hot 3
#define eraser_y_hot 2

static char eraser_bits[] =
{
	0x00, 0x00, 0x40, 0x00, 0xf0, 0x00, 0x9c, 0x01, 0x06, 0x03, 0x06, 0x06,
	0x0c, 0x0f, 0xd8, 0x1b, 0x70, 0x35, 0xe0, 0x6a, 0xc0, 0x75, 0x80, 0x1b,
	0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00
};

static char eraser_mask_bits[] =
{
	0xf8, 0x00, 0xfc, 0x01, 0xfe, 0x03, 0xff, 0x07, 0xff, 0x0f, 0xff, 0x1f,
	0xff, 0x3f, 0xfe, 0x7f, 0xfc, 0xff, 0xf8, 0xff, 0xf0, 0xff, 0xe0, 0x7f,
	0xc0, 0x3f, 0x80, 0x0f, 0x00, 0x07, 0x00, 0x00
};

/*===========================================================================*

                         B R U S H    S H A P E S

 *===========================================================================*/

#include "widgets/PixelEditor.brushes.h"

/*===========================================================================*

   R E S O U R C E    T Y P E S,    F I E L D S,    A N D    D E F A U L T S

 *===========================================================================*/

#define PEFieldOffset(field)	XtOffset(PixelEditorWidget,pixelEditor.field)
#define	ParamOffset(field)	PEFieldOffset(parameters.field)
#define CoreFieldOffset(field)	XtOffset(Widget,core.field)

static XtResource resources[] =
{
	{XtNzoom,XtCValue,XtRInt,sizeof(int),
		ParamOffset(zoom_factor),XtRString,"23"},
	{XtNleftClipper,XtCValue,XtRInt,sizeof(int),
		ParamOffset(left_x_clipper),XtRString,"0"},
	{XtNrightClipper,XtCValue,XtRInt,sizeof(int),
		ParamOffset(right_x_clipper),XtRString,"-1"},
	{XtNtopClipper,XtCValue,XtRInt,sizeof(int),
		ParamOffset(top_y_clipper),XtRString,"0"},
	{XtNbottomClipper,XtCValue,XtRInt,sizeof(int),
		ParamOffset(top_y_clipper),XtRString,"-1"},
	{XtNclipperWidth,XtCWidth,XtRInt,sizeof(int),
		ParamOffset(clipper_width),XtRImmediate,(caddr_t)CLIPPERW},
	{XtNclipperHeight,XtCHeight,XtRInt,sizeof(int),
		ParamOffset(clipper_height),XtRImmediate,(caddr_t)CLIPPERH},
	{XtNgrid,XtCBoolean,XtRBoolean,sizeof(Boolean),
		ParamOffset(grid_on),XtRString,"True"},
	{XtNclippers,XtCBoolean,XtRBoolean,sizeof(Boolean),
		ParamOffset(clippers_on),XtRString,"False"},
	{XtNautoScale,XtCBoolean,XtRBoolean,sizeof(Boolean),
		ParamOffset(auto_scale),XtRString,"False"},
	{XtNopaquePaste,XtCBoolean,XtRBoolean,sizeof(Boolean),
		ParamOffset(opaque_paste),XtRString,"True"},
	{XtNtool,XtCTool,XtRTool,sizeof(int),
		ParamOffset(tool),XtRString,XtNtoolPencil},
	{XtNbrush,XtCBrush,XtRBrush,sizeof(int),
		ParamOffset(brush_shape),XtRString,XtNbrushSmallBox},
	{XtNdataChangeCallback,XtCCallback,XtRCallback,sizeof(caddr_t),
		ParamOffset(data_change_callback),XtRCallback,NULL},
	{XtNregionChangeCallback,XtCCallback,XtRCallback,sizeof(caddr_t),
		ParamOffset(region_change_callback),XtRCallback,NULL},
	{XtNpasteBufferChangeCallback,XtCCallback,XtRCallback,sizeof(caddr_t),
		ParamOffset(paste_buffer_change_callback),XtRCallback,NULL},
	{XtNdrawColor,XtCColor,XtRPixel,sizeof(Pixel),
		ParamOffset(draw_color),XtRString,"Black"},
	{XtNeraseColor,XtCColor,XtRPixel,sizeof(Pixel),
		ParamOffset(erase_color),XtRString,"White"},
	{XtNimageHeight,XtCHeight,XtRInt,sizeof(int),
		ParamOffset(image_height),XtRString,"16"},
	{XtNimageWidth,XtCWidth,XtRInt,sizeof(int),
		ParamOffset(image_width),XtRString,"16"},
	{XtNimagePixels,XtCPixelArray,XtRPointer,sizeof(Pixel *),
		ParamOffset(image_pixels),XtRPointer,NULL},
	{XtNimageShared,XtCBoolean,XtRBoolean,sizeof(Boolean),
		ParamOffset(image_shared),XtRString,"False"}
};

#undef PEFieldOffset
#undef CoreFieldOffset

/*===========================================================================*

                F U N C T I O N    D E C L A R A T I O N S

 *===========================================================================*/

static void			ClassInitialize();
static void			CvtStringToTool();
static void			CvtStringToBrush();
static int			BindingLookup();
static void			Initialize();
static void			Realize();
static void			Destroy();
static void			Resize();
static Boolean			SetValues();
static XtGeometryResult		QueryGeometry();
static void			Redisplay();
static Boolean			Layout();
static void			DumpCoords();
static void			RedrawAll();
static void			RedrawRegion();
static void			RedrawRegionOfPixels();
static void			RedrawPixelArea();
static void			RedrawGrid();
static void			RedrawSelectionBox();
static void			RedrawFloatingPasteBox();
static void			RedrawFloatingPasteOutline();
static void			RedrawPadding();
static void			RedrawClippers();
static void			RedrawHorizontalClippers();
static void			RedrawVerticalClippers();
static void			RedrawUpDownClipLines();
static void			RedrawLeftRightClipLines();
static void			RedrawGap();
static void			FlashPasteOutline();
static void			CancelSelectionBox();
static void			CancelPasteBox();
static void			Click();
static void			EventHandleButtonPress();
static void			DoClickInPixelArea();
static void			DoClickInClipperArea();
static void			DoClickInGapArea();
static void			ToolHandleClick();
static void			ToolPaste();
static void			ToolDraw();
static void			ToolRegion();
static Boolean			PixelsPixelInsideClippers();
static Boolean			PixelsPointToPixel();
static Boolean			PixelsPixelToPoint();
static Boolean			PixelsXPixelToXPoint();
static Boolean			PixelsYPixelToYPoint();
static Pixel			PixelsGetPixel();
static void			PixelsSetPixel();
static void			PixelsDrawOneToolBlob();
static void			PixelsDrawOneBrushBlob();
static void			PixelsDrawStroke();
static void			PixelsRedrawRegion();
static void			ImageDrawPixels();
static Pixel	 		*PixelEditorGetSubImageData();
static void			PixelEditorPutSubImageData();
static int			DragConstrainedBox();
static int			DragRubberBox();
static void			BoxDump();
static Boolean			RectIntersect();
static Boolean			BoxIntersect();
static Boolean			PointInRect();
static Boolean			PointInBox();
static Colormap			ColormapNew();
static void			ColormapSetDefaultColors();
static void			SetNewImageFromParams();
static ImageData 		*ImageDataNew();
static void			ImageResize();
static Pixmap			PixmapCreate();
static Pixmap			BitmapCreate();
static GC			GCCreate();
static Cursor			CreateCursorFromBitmapData();
static void			CursorStackDump();
static Boolean			CursorPush();
static Cursor			CursorPop();
static void			CursorChange();
static Boolean			FileReadBits();
static void			NotifyDataChange();
static void			NotifyRegionChange();
static void			NotifyPasteBufferChange();

/*===========================================================================*

        A C T I O N    A N D    T R A N S L A T I O N    T A B L E S

 *===========================================================================*/

static char defaultTranslations[] =
"	<BtnDown>:			Click()";


static XtActionsRec actions[] =
{
	{"Click", Click},
	{NULL,NULL}
};

/*===========================================================================*

                 S T A T I C    A L L O C A T I O N    A N D

        I N I T I A L I Z A T I O N    O F    C L A S S    R E C O R D

 *===========================================================================*/

#define	SUPERCLASS (&widgetClassRec)

PixelEditorClassRec pixelEditorClassRec =
{
	{
		/* superclass		*/	SUPERCLASS,
		/* class_name		*/	"PixelEditor",
		/* widget_size		*/	sizeof(PixelEditorRec),
		/* class_initialize	*/	ClassInitialize,
		/* class_part_initialize*/	NULL,
		/* class_inited		*/	FALSE,
		/* initialize		*/	Initialize,
		/* initialize_hook	*/	NULL,
		/* realize		*/	Realize,
		/* actions		*/	actions,
		/* num_actions		*/	XtNumber(actions),
		/* resources		*/	resources,
		/* resource_count	*/	XtNumber(resources),
		/* xrm_class		*/	NULL,
		/* compress_motion	*/	TRUE,
		/* compress_exposure	*/	TRUE,
		/* compress_enterleave	*/	TRUE,
		/* visible_interest	*/	FALSE,
		/* destroy		*/	Destroy,
		/* resize		*/	Resize,
		/* expose		*/	Redisplay,
		/* set_values		*/	SetValues,
		/* set_values_hook	*/	NULL,
		/* set_values_almost	*/	XtInheritSetValuesAlmost,
		/* get_values_hook	*/	NULL,
		/* accept_focus		*/	NULL,
		/* version		*/	XtVersion,
		/* callback_private	*/	NULL,
		/* tm_table		*/	defaultTranslations,
		/* query_geometry       */	QueryGeometry,
		/* display_accelerator  */	XtInheritDisplayAccelerator,
		/* extension            */	NULL
	}, /* Core Part */
	{
		/* dummy variable	*/	0
	}  /* Pixel Editor Part */
};

WidgetClass pixelEditorWidgetClass = (WidgetClass) &pixelEditorClassRec;

/*===========================================================================*

                      T O O L K I T    M E T H O D S

 *===========================================================================*/

/*---------------------------------------------------------------------------*

	static void ClassInitialize()

	This procedure is called by the X toolkit to initialize the class.
	The class is only initialized once, and is shared among all
	instances of the class.  The hook to this routine is in the
	class_initialize part of the core part of the class.  We use this
	as a place to add resource type converters for the pixelEditor class.

 *---------------------------------------------------------------------------*/

static void ClassInitialize()
{
	XtAddConverter(XtRString,XtRTool,CvtStringToTool,NULL,0);
	XtAddConverter(XtRString,XtRBrush,CvtStringToBrush,NULL,0);
} /* End ClassInitialize */


/* ARGSUSED */
static void CvtStringToTool(args,arg_count,source,dest)
XrmValuePtr args;
int *arg_count;
XrmValuePtr source,dest;
{
	char message[80];
	static int value;
	static Binding bindings[] =
	{
		{XtNtoolPencil,TOOL_PENCIL},
		{XtNtoolBrush,TOOL_BRUSH},
		{XtNtoolEraser,TOOL_ERASER},
		{XtNtoolRegion,TOOL_REGION},
		{XtNtoolPaster,TOOL_PASTER},
		{NULL,0}
	};

	value = BindingLookup(bindings,source->addr);
	if (value < 0)
	{
		sprintf(message,"String '%s' can't be converted to a Tool\n",
			source->addr);
		XtWarning(message);
	}
	    else
	{
		dest->addr = (char *)&value;
		dest->size = sizeof(int);
	}
} /* End CvtStringToTool */


/* ARGSUSED */
static void CvtStringToBrush(args,arg_count,source,dest)
XrmValuePtr args;
int *arg_count;
XrmValuePtr source,dest;
{
	char message[80];
	static int value;
	static Binding bindings[] =
	{
		{XtNbrushSmallBox,BRUSH_SMALL_BOX},
		{XtNbrushMediumBox,BRUSH_MEDIUM_BOX},
		{XtNbrushLargeBox,BRUSH_LARGE_BOX},
		{XtNbrushSmallDiamond,BRUSH_SMALL_DIAMOND},
		{XtNbrushMediumDiamond,BRUSH_MEDIUM_DIAMOND},
		{XtNbrushLargeDiamond,BRUSH_LARGE_DIAMOND},
		{XtNbrushVertLine,BRUSH_VERT_LINE},
		{XtNbrushHorizLine,BRUSH_HOR_LINE},
		{XtNbrushNegDiagLine,BRUSH_NEG_DIAG_LINE},
		{XtNbrushPosDiagLine,BRUSH_POS_DIAG_LINE},
		{XtNbrushCircle,BRUSH_CIRCLE},
		{XtNbrushUserDefined,BRUSH_USER_DEFINED},
		{NULL,0}
	};

	value = BindingLookup(bindings,source->addr);
	if (value < 0)
	{
		sprintf(message,"String '%s' can't be converted to a Brush\n",
			source->addr);
		XtWarning(message);
	}
	    else
	{
		dest->addr = (char *)&value;
		dest->size = sizeof(int);
	}
} /* End CvtStringToBrush */


static int BindingLookup(bindings,string)
Binding *bindings;
String string;
{
	char *s,*t,sc,tc;
	int index;

	index = 0;
	if (string == NULL) return(-1);
	while (bindings[index].name != NULL)
	{
		s = bindings[index].name;
		t = string;
		while (1)
		{
			sc = (isupper(*s) ? tolower(*s) : *s);
			tc = (isupper(*t) ? tolower(*t) : *t);
			if (sc == tc)
			{
				if (sc == '\0') return(index);
			}
			    else
			{
				break;
			}
			++ s;
			++ t;
		}
		++ index;
	}
	return(-1);
} /* End BindingLookup */


/*---------------------------------------------------------------------------*

	static void Initialize(old,new)

	This procedure is called by the X toolkit to initialize the widget
	instance.  The widget will not be realized yet (no window will have
	been created), so only non-graphic values will be initialized.  The
	hook to this routine is in the initialize part of the core part of
	the class.

	The parameters <old> and <new> are widgets where <old> represents
	the widget before the superclass modified any of the parameters
	and <new> is the current state of the widget.  <old> can be used
	to determine the original requested data before the superclass
	changed anything.  The widget <new> will become the new widget.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static void Initialize(old,new)
PixelEditorWidget old,new;
{
	DisplayData *dd;
	EditorState *es;

	dd = PEPartDisplayData(new);
	es = PEPartEditorState(new);
	DDDisplay(dd) = XtDisplay(new);
	DDScreen(dd) = XtScreen(new);
	DDScreenNumber(dd) = XScreenNumberOfScreen(DDScreen(dd));
	DDRoot(dd) = XRootWindow(DDDisplay(dd),DDScreenNumber(dd));
	DDVisual(dd) = XDefaultVisual(DDDisplay(dd),DDScreenNumber(dd));
	DDCells(dd) = XDisplayCells(DDDisplay(dd),DDScreenNumber(dd));
	DDWhite(dd) = XWhitePixel(DDDisplay(dd),DDScreenNumber(dd));
	DDBlack(dd) = XBlackPixel(DDDisplay(dd),DDScreenNumber(dd));
	DDCmap(dd) = XDefaultColormap(DDDisplay(dd),DDScreenNumber(dd));
	DDDynamicColorFlag(dd) = False;

	ESImage(es) = NULL;
	ESCursorSP(es) = 0;
	ImagePixels(PasteBufferImage(ESPasteBuffer(es))) = NULL;
	PasteBufferValid(ESPasteBuffer(es)) = False;
	PasteBufferPasted(ESPasteBuffer(es)) = False;
	PasteBufferPhase(ESPasteBuffer(es)) = 0;
	SetNewImageFromParams(new);
	Layout(new,(Core(new)->width == 0),(Core(new)->height == 0),
	       (Dimension *)NULL,(Dimension *)NULL);
} /* End Initialize */


/*---------------------------------------------------------------------------*

	static void Realize()

	This function is called to realize a PixelEditor widget.

 *---------------------------------------------------------------------------*/

static void Realize(pew,requested_window_gc_value_mask,requested_window_attrs)
PixelEditorWidget pew;
XtValueMask *requested_window_gc_value_mask;
XSetWindowAttributes *requested_window_attrs;
{
	Window w;
	Widget widget;
	Pixmap pad_pixmap,clipped_pixel_bitmap;
	EditorState *es;
	DisplayData *dd;

	widget = (Widget)pew;
	*requested_window_gc_value_mask |= CWEventMask;
	requested_window_attrs->event_mask |= PointerMotionMask;
	XtCreateWindow(widget,InputOutput,(Visual *)CopyFromParent,
		*requested_window_gc_value_mask,requested_window_attrs);
	w = XtWindow(pew);

	es = PEPartEditorState(pew);
	dd = PEPartDisplayData(pew);

	ESWindowMapped(es) = False;
	ESGrabCursor(es) = XCreateFontCursor(DDDisplay(dd),XC_hand1);
	ESPencilCursor(es) = CreateCursorFromBitmapData(DDDisplay(dd),w,
		DDBlack(dd),DDWhite(dd),
		draw_pencil_bits,draw_pencil_mask_bits,
		draw_pencil_width,draw_pencil_height,
		draw_pencil_x_hot,draw_pencil_y_hot);
	ESEPencilCursor(es) = CreateCursorFromBitmapData(DDDisplay(dd),w,
		DDBlack(dd),DDWhite(dd),
		erase_pencil_bits,erase_pencil_mask_bits,
		erase_pencil_width,erase_pencil_height,
		erase_pencil_x_hot,erase_pencil_y_hot);
	ESBrushCursor(es) = CreateCursorFromBitmapData(DDDisplay(dd),w,
		DDBlack(dd),DDWhite(dd),
		brush_bits,brush_mask_bits,
		brush_width,brush_height,
		brush_x_hot,brush_y_hot);
	ESEraserCursor(es) = CreateCursorFromBitmapData(DDDisplay(dd),w,
		DDBlack(dd),DDWhite(dd),
		eraser_bits,eraser_mask_bits,
		eraser_width,eraser_height,
		eraser_x_hot,eraser_y_hot);
	ESFlatHandCursor(es) = CreateCursorFromBitmapData(DDDisplay(dd),w,
		DDBlack(dd),DDWhite(dd),
		hand_bits,hand_mask_bits,
		hand_width,hand_height,
		hand_x_hot,hand_y_hot);

	ESPointingCursor(es) = XCreateFontCursor(DDDisplay(dd),XC_hand2);
	ESUpperLeftCursor(es) = XCreateFontCursor(DDDisplay(dd),XC_ul_angle);
	ESUpperRightCursor(es) = XCreateFontCursor(DDDisplay(dd),XC_ur_angle);
	ESLowerLeftCursor(es) = XCreateFontCursor(DDDisplay(dd),XC_ll_angle);
	ESLowerRightCursor(es) = XCreateFontCursor(DDDisplay(dd),XC_lr_angle);
	ESCrosshairCursor(es) = XCreateFontCursor(DDDisplay(dd),XC_crosshair);

	ESCMap(es) = ColormapNew(dd);
	XSetWindowColormap(DDDisplay(dd),w,ESCMap(es));

	pad_pixmap = PixmapCreate(DDDisplay(dd),w,pad_stipple_data,
				  16,16,DDBlack(dd),DDWhite(dd));
	clipped_pixel_bitmap = BitmapCreate(DDDisplay(dd),w,
					    clipped_pixel_stipple_data,16,16);

	ESPixelGC(es) = GCCreate(DDDisplay(dd),w,DDBlack(dd),DDWhite(dd),
		LineSolid,1,FillStippled,clipped_pixel_bitmap);
	ESGC(es) = GCCreate(DDDisplay(dd),w,DDBlack(dd),DDWhite(dd),
		LineSolid,1,FillSolid,None);
	ESEraseGC(es) = GCCreate(DDDisplay(dd),w,DDWhite(dd),DDWhite(dd),
		LineSolid,1,FillSolid,None);
	ESDashedGC(es) = GCCreate(DDDisplay(dd),w,DDBlack(dd),DDWhite(dd),
		LineDoubleDash,1,FillSolid,None);
	ESGridGC(es) = GCCreate(DDDisplay(dd),w,DDBlack(dd),DDWhite(dd),
		LineSolid,1,FillSolid,None);
	ESPadGC(es) = GCCreate(DDDisplay(dd),w,DDBlack(dd),DDWhite(dd),
		LineSolid,1,FillTiled,pad_pixmap);
	ESSelectionBoxGC(es) = GCCreate(DDDisplay(dd),w,DDBlack(dd),
		DDWhite(dd),LineDoubleDash,1,FillSolid,None);
	ESPasteBoxGC(es) = GCCreate(DDDisplay(dd),w,DDBlack(dd),
		DDWhite(dd),LineDoubleDash,1,FillSolid,None);
	ESXorGC(es) = GCCreate(DDDisplay(dd),w,DDBlack(dd),DDWhite(dd),
		LineSolid,1,FillSolid,None);

	XSetDashes(DDDisplay(dd),ESSelectionBoxGC(es),0,dash_list,2);
	XSetDashes(DDDisplay(dd),ESPasteBoxGC(es),0,dash_list,2);

	XSetFunction(DDDisplay(dd),ESXorGC(es),GXxor);

	XGrabButton(DDDisplay(dd),AnyButton,AnyModifier,XtWindow(widget),
		    False,
		    (ButtonReleaseMask | ButtonPressMask | PointerMotionMask),
		    GrabModeAsync,GrabModeAsync,None,None);

	ESCursorSP(es) = 0;
	CursorPush(pew,None);
	PixelEditorChangeTool(pew,TOOL_PENCIL);
} /* End Realize */


/* ARGSUSED */
static void Destroy(gw)
Widget gw;
{
} /* End Destroy */


static void Resize(pew)
PixelEditorWidget pew;
{
	Layout(pew,False,False,(Dimension *)NULL,(Dimension *)NULL);
} /* End Resize */


/* ARGSUSED */
static Boolean SetValues(c_pew,r_pew,n_pew)
PixelEditorWidget c_pew,r_pew,n_pew;
{
	Boolean redraw,relayout;
	Parameters *cp,*np;
	ImageData *image;

	redraw = False;
	relayout = False;
	cp = PEPartParams(c_pew);
	np = PEPartParams(n_pew);
	image = ESImage(PEPartEditorState(n_pew));
	if ((ParamsZoom(cp) != ParamsZoom(np)))
	{
		ParamsZoom(np) = max(1,ParamsZoom(np));
		relayout = True;
		redraw = True;
	}
	if ((ParamsGridOn(cp) != ParamsGridOn(np)) ||
	    (ParamsClippersOn(cp) != ParamsClippersOn(np)))
	{
		relayout = True;
		redraw = True;
	}		
	if ((ParamsImageWidth(cp) != ParamsImageWidth(np)) ||
	    (ParamsImageHeight(cp) != ParamsImageHeight(np)) ||
	    (ParamsImagePixels(cp) != ParamsImagePixels(np)) ||
	    (ParamsImageShared(cp) != ParamsImageShared(np)))
	{
		if (ParamsImagePixels(np) == NULL)
		{
			XtWarning("Ignoring NULL imagePixels value");
			ParamsImagePixels(np) = ParamsImagePixels(cp);
		}
		if (ParamsImagePixels(cp) != ParamsImagePixels(np))
		{
			SetNewImageFromParams(n_pew);
		}
		    else if (ParamsImageShared(cp) != ParamsImageShared(np))
		{
			if (ParamsImageShared(np) == True)
			{
				XtWarning("Ignoring imageSharing change\n");
				ParamsImageShared(np) = False;
			}
			    else
				SetNewImageFromParams(n_pew);
		}
		    else			/* Only A Size Change */
		{
			if (ParamsImageShared(np) == False)
			{
				ImageResize(image,0,0,
					    ParamsImageWidth(np),
					    ParamsImageHeight(np));
			}
		}
		relayout = True;
		redraw = True;
	}
	if (relayout)
	{
		Layout(n_pew,True,True,(Dimension *)NULL,(Dimension *)NULL);
	}
	if (!XtIsRealized(c_pew))
		return(False);
	    else
		return(redraw);
} /* End SetValues */

/*---------------------------------------------------------------------------*

	QueryGeometry(plw,parent_idea,our_idea)

	This routine is called by the parent to tell us about the
	parent's idea of our width and/or height.  We then suggest
	our preference through <our_idea> and return the information
	to the parent.

 *---------------------------------------------------------------------------*/

static XtGeometryResult QueryGeometry(pew,parent_idea,our_idea)
PixelEditorWidget pew;
XtWidgetGeometry *parent_idea,*our_idea;
{
	Dimension nw,nh;
	Boolean parent_wants_w,parent_wants_h,we_changed_size;
    
	parent_wants_w = (parent_idea->request_mode) & CWWidth;
	parent_wants_h = (parent_idea->request_mode) & CWHeight;
	our_idea->request_mode = 0;
	if (!parent_wants_w && !parent_wants_h) return(XtGeometryYes);
	we_changed_size = Layout(pew,!parent_wants_w,!parent_wants_h,&nw,&nh);
	our_idea->request_mode |= (CWWidth | CWHeight);
	our_idea->width = Core(pew)->width;
	our_idea->height = Core(pew)->height;
	if ((parent_wants_w && (nw != parent_idea->width)) ||
	    (parent_wants_h && (nh != parent_idea->height)))
		return(XtGeometryAlmost);
	    else
		return(XtGeometryYes);
} /* End QueryGeometry */


static void Redisplay(pew,event,region)
PixelEditorWidget pew;
XEvent *event;
Region region;
{
	Box box;

	if (!XtIsRealized(pew)) return;
	if (event == NULL)
	{
		BoxX(&box) = 0;
		BoxY(&box) = 0;
		BoxW(&box) = Core(pew)->width;
		BoxH(&box) = Core(pew)->height;
	}
	    else
	{
		BoxX(&box) = event->xexpose.x;
		BoxY(&box) = event->xexpose.y;
		BoxW(&box) = event->xexpose.width + 1;
		BoxH(&box) = event->xexpose.height + 1;
	}
	RedrawRegion(pew,&box);
} /* End Redisplay */

/*===========================================================================*

                      G E O M E T R Y    L A Y O U T

 *===========================================================================*/

static Boolean Layout(pew,w_changeable,h_changeable,new_w_ptr,new_h_ptr)
PixelEditorWidget pew;
Boolean w_changeable,h_changeable;
Dimension *new_w_ptr,*new_h_ptr;
{
	ImageData *i;
	int pixel_size;
	Coords *coords;
	Parameters *params;
	EditorState *es;
	Dimension old_w,old_h;

	coords = PEPartCoords(pew);
	params = PEPartParams(pew);
	es = PEPartEditorState(pew);
	i = ESImage(es);

	old_w = Core(pew)->width;
	old_h = Core(pew)->height;

		/* Set Clipper Width & Height */

	if (ParamsClippersOn(params))
	{
		BoxW(CoordsClipVer(coords)) = ParamsClipperWidth(params);
		BoxH(CoordsClipHor(coords)) = ParamsClipperHeight(params);
	}
	    else
	{
		BoxW(CoordsClipVer(coords)) = 0;
		BoxH(CoordsClipHor(coords)) = 0;
	}

		/* Modify Width/Height If Allowed */

	pixel_size = PEWPixelSize(pew);
	if (w_changeable)
	{
		Core(pew)->width = ImageWidth(ESImage(es)) * pixel_size +
			(ParamsGridOn(params) == True) +
				BoxW(CoordsClipVer(coords));
	}
	if (h_changeable)
	{
		Core(pew)->height = ImageHeight(ESImage(es)) * pixel_size +
			(ParamsGridOn(params) == True) +
				BoxH(CoordsClipHor(coords));
	}

		/* Widget Window Coords */

	BoxX(CoordsWindow(coords)) = 0;
	BoxY(CoordsWindow(coords)) = 0;
	BoxW(CoordsWindow(coords)) = Core(pew)->width;
	BoxH(CoordsWindow(coords)) = Core(pew)->height;

		/* Handle Any Auto Scaling */

	if (ParamsAutoScale(params))
	{
		ParamsZoom(params) = max(1,
			min(((BoxW(CoordsWindow(coords)) -
			      BoxW(CoordsClipVer(coords))) / ImageWidth(i)),
			    ((BoxH(CoordsWindow(coords)) -
			      BoxH(CoordsClipHor(coords))) / ImageHeight(i))));
		pixel_size = PEWPixelSize(pew);
	}				  

		/* Pixel Area Coords */

	BoxX(CoordsPixelArea(coords)) = 0;
	BoxY(CoordsPixelArea(coords)) = 0;
	BoxW(CoordsPixelArea(coords)) = ParamsGridOn(params) + 
		pixel_size * ImageWidth(i);
	BoxH(CoordsPixelArea(coords)) = ParamsGridOn(params) + 
		pixel_size * ImageHeight(i);

	CoordsMaxHPixels(coords) = (BoxW(CoordsWindow(coords)) -
				    ParamsGridOn(params) + pixel_size - 1) /
					    pixel_size;
	CoordsMaxVPixels(coords) = (BoxH(CoordsWindow(coords)) -
				    ParamsGridOn(params) + pixel_size - 1) /
					    pixel_size;

	CoordsHPixels(coords) = min(CoordsMaxHPixels(coords),ImageWidth(i));
	CoordsVPixels(coords) = min(CoordsMaxVPixels(coords),ImageHeight(i));

		/* Right Padding Coords */

	BoxX(CoordsPadRight(coords)) = BoxX2(CoordsPixelArea(coords)) + 1;
	BoxY(CoordsPadRight(coords)) = BoxY(CoordsPixelArea(coords));
	BoxW(CoordsPadRight(coords)) = max(0,BoxW(CoordsWindow(coords)) -
		BoxW(CoordsClipVer(coords)) - BoxW(CoordsPixelArea(coords)));
	BoxH(CoordsPadRight(coords)) = BoxH(CoordsPixelArea(coords));
	
		/* Bottom Padding Coords */

	BoxX(CoordsPadBottom(coords)) = BoxX(CoordsPixelArea(coords));
	BoxY(CoordsPadBottom(coords)) = BoxY2(CoordsPixelArea(coords)) + 1;
	BoxW(CoordsPadBottom(coords)) = BoxW(CoordsPixelArea(coords)) +
		BoxW(CoordsPadRight(coords));
	BoxH(CoordsPadBottom(coords)) = max(0,BoxH(CoordsWindow(coords)) -
		BoxH(CoordsClipHor(coords)) - BoxH(CoordsPixelArea(coords)));

		/* Vertical Clipper Coords */

	BoxX(CoordsClipVer(coords)) = BoxX2(CoordsPadRight(coords)) + 1;
	BoxY(CoordsClipVer(coords)) = BoxY(CoordsPadRight(coords));
	BoxH(CoordsClipVer(coords)) = BoxH(CoordsPadRight(coords)) +
		BoxH(CoordsPadBottom(coords)) + 1;

		/* Horizontal Clipper Coords */

	BoxX(CoordsClipHor(coords)) = BoxX(CoordsPadBottom(coords));
	BoxY(CoordsClipHor(coords)) = BoxY2(CoordsPadBottom(coords)) + 1;
	BoxW(CoordsClipHor(coords)) = BoxW(CoordsPadBottom(coords)) + 1;

		/* Gap Left At Lower Right */

	BoxX(CoordsGap(coords)) = BoxX(CoordsClipVer(coords));
	BoxY(CoordsGap(coords)) = BoxY(CoordsClipHor(coords));
	BoxW(CoordsGap(coords)) = BoxW(CoordsClipVer(coords));
	BoxH(CoordsGap(coords)) = BoxH(CoordsClipHor(coords));

	if (new_w_ptr) *new_w_ptr = Core(pew)->width;
	if (new_h_ptr) *new_h_ptr = Core(pew)->height;
	if ((old_w != Core(pew)->width) || (old_h != Core(pew)->height))
		return(True);
	    else
		return(False);
} /* End Layout */


static void DumpCoords(pew)
PixelEditorWidget pew;
{
	printf("PSize:%d\n",PEWPixelSize(pew));
	printf("W:    "); BoxDump(CoordsWindow(PEPartCoords(pew)));
	printf("PA:   "); BoxDump(CoordsPixelArea(PEPartCoords(pew)));
	printf("PR:   "); BoxDump(CoordsPadRight(PEPartCoords(pew)));
	printf("PB:   "); BoxDump(CoordsPadBottom(PEPartCoords(pew)));
	printf("CH:   "); BoxDump(CoordsClipHor(PEPartCoords(pew)));
	printf("CV:   "); BoxDump(CoordsClipVer(PEPartCoords(pew)));
	printf("GAP:  "); BoxDump(CoordsGap(PEPartCoords(pew)));
	printf("MaxH: %d\n",CoordsMaxHPixels(PEPartCoords(pew)));
	printf("MaxV: %d\n",CoordsMaxVPixels(PEPartCoords(pew)));
	printf("H:    %d\n",CoordsHPixels(PEPartCoords(pew)));
	printf("V:    %d\n",CoordsVPixels(PEPartCoords(pew)));
} /* End DumpCoords */

/*===========================================================================*

                     R E D R A W    R O U T I N E S

 *===========================================================================*/

static void RedrawAll(pew)
PixelEditorWidget pew;
{
	Region dummy;

	Redisplay(pew, (XEvent *)NULL, dummy);
} /* End RedrawAll */


static void RedrawRegion(pew,box)
PixelEditorWidget pew;
Box *box;
{
	RedrawPadding(pew,box);
	RedrawPixelArea(pew,box);
	RedrawClippers(pew,box);
	RedrawGap(pew,box);
} /* End RedrawRegion */


static void RedrawRegionOfPixels(pew,box)
PixelEditorWidget pew;
Box *box;
{
	Box xarea;
	Parameters *params;
	int pixel_size,x1,y1,x2,y2;

	params = PEPartParams(pew);
	pixel_size = PEWPixelSize(pew);
	PixelsPixelToPoint(pew,BoxX1(box),BoxY1(box),&x1,&y1);
	PixelsPixelToPoint(pew,BoxX2(box),BoxY2(box),&x2,&y2);
	x2 = x2 + pixel_size - 1 + (ParamsGridOn(params) == True);
	y2 = y2 + pixel_size - 1 + (ParamsGridOn(params) == True);
	BoxX(&xarea) = x1;
	BoxY(&xarea) = y1;
	BoxW(&xarea) = x2 - x1 + 1;
	BoxH(&xarea) = y2 - y1 + 1;
	RedrawRegion(pew,&xarea);
} /* End RedrawRegionOfPixels */


static void RedrawPixelArea(pew,box)
PixelEditorWidget pew;
Box *box;
{
	Box ibox,*pa_box;
	Coords *coords;
	int startpx,endpx,startpy,endpy;

	coords = PEPartCoords(pew);
	pa_box = CoordsPixelArea(coords);
	if (!BoxIntersect(box,pa_box,&ibox)) return;
	if (!PixelsPointToPixel(pew,BoxX(&ibox),BoxY(&ibox),&startpx,&startpy))
		printf("UL too small\n");
	if (!PixelsPointToPixel(pew,BoxX2(&ibox),BoxY2(&ibox),&endpx,&endpy))
		printf("LR too big\n");
	RedrawGrid(pew,startpx,startpy,endpx,endpy);
	endpx = min(endpx,CoordsHPixels(coords) - 1);
	endpy = min(endpy,CoordsVPixels(coords) - 1);
	PixelsRedrawRegion(pew,startpx,startpy,endpx,endpy);
	RedrawFloatingPasteBox(pew,startpx,startpy,endpx,endpy);
	RedrawSelectionBox(pew,startpx,startpy,endpx,endpy);
} /* End RedrawPixelArea */


static void RedrawGrid(pew,spx,spy,epx,epy)
PixelEditorWidget pew;
int spx,spy,epx,epy;
{
	Box *pa_box;
	Parameters *p;
	Coords *coords;
	EditorState *es;
	static XSegment *s = NULL;
	static int old_num_segs = 0;
	int px,py,size;
	int num_segs,i,pacross,pdown;

	p = PEPartParams(pew);
	coords = PEPartCoords(pew);
	pa_box = CoordsPixelArea(coords);
	es = PEPartEditorState(pew);

	if (ParamsGridOn(p))
	{
		size = PEWPixelSize(pew);
		pacross = epx - spx + 1;
		pdown = epy - spy + 1;
		num_segs = pacross + pdown;
		if (old_num_segs < num_segs)
		{
			if (s) free((char *) s);
			s = (XSegment *)XtMalloc(sizeof(XSegment) * num_segs);
			old_num_segs = num_segs;
		}
		i = 0;
		for (px = spx; px <= epx; px++)
		{
			s[i].x1 = BoxX(pa_box) + (size * px);
			s[i].y1 = BoxY(pa_box) + (size * spy);
			s[i].x2 = s[i].x1;
			s[i].y2 = BoxY(pa_box) + (size * (epy + 1)) - 1;
			s[i].y2 = min(s[i].y2,BoxY2(pa_box)) + 1;
			++ i;
		}
		for (py = spy; py <= epy; py++)
		{
			s[i].x1 = BoxX(pa_box) + (size * spx);
			s[i].y1 = BoxY(pa_box) + (size * py);
			s[i].x2 = BoxX(pa_box) + (size * (epx + 1)) - 1;
			s[i].y2 = s[i].y1;
			s[i].x2 = min(s[i].x2,BoxX2(pa_box)) + 1;
			++ i;
		}
		XDrawSegments(XtDisplay(pew),XtWindow(pew),
			      ESGridGC(es),s,num_segs);
	}
} /* End RedrawGrid */


/* ARGSUSED */
static void RedrawSelectionBox(pew,rx1,ry1,rx2,ry2)
PixelEditorWidget pew;
int rx1,ry1,rx2,ry2;
{
	Parameters *p;
	EditorState *es;
	Box *sel_box;
	int size,x1,y1,x2,y2,w,h;

	p = PEPartParams(pew);
	es = PEPartEditorState(pew);
	if (ESRegionSelected(es))
	{
		sel_box = ESSelectionBox(es);
		size = PEWPixelSize(pew);
		PixelsPixelToPoint(pew,BoxX1(sel_box),BoxY1(sel_box),&x1,&y1);
		PixelsPixelToPoint(pew,BoxX2(sel_box),BoxY2(sel_box),&x2,&y2);
		x2 = x2 + size - 1 + (ParamsGridOn(p) == True);
		y2 = y2 + size - 1 + (ParamsGridOn(p) == True);
		w = x2 - x1;
		h = y2 - y1;
		XDrawRectangle(XtDisplay(pew),XtWindow(pew),
			       ESSelectionBoxGC(es),x1,y1,w,h);
	}
} /* End RedrawSelectionBox */


static void RedrawFloatingPasteBox(pew,rx1,ry1,rx2,ry2)
PixelEditorWidget pew;
int rx1,ry1,rx2,ry2;
{
	Parameters *p;
	Box *paste_box;
	PasteBuffer *pb;
	int x1,y1,x2,y2,w,h,off_x,off_y;

	p = PEPartParams(pew);
	pb = ESPasteBuffer(PEPartEditorState(pew));
	paste_box = PasteBufferBox(pb);
	if ((!PasteBufferValid(pb)) || (!PasteBufferPasted(pb))) return;
	x1 = max(BoxX1(paste_box),rx1);
	y1 = max(BoxY1(paste_box),ry1);
	x2 = min(BoxX2(paste_box),rx2);
	y2 = min(BoxY2(paste_box),ry2);
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	off_x = max(0,rx1 - BoxX1(paste_box));
	off_y = max(0,ry1 - BoxY1(paste_box));
	ImageDrawPixels(pew,PasteBufferImage(pb),
			off_x,off_y,w,h,x1,y1,ParamsOpaquePaste(p));
	RedrawFloatingPasteOutline(pew,rx1,ry1,rx2,ry2);
} /* End RedrawFloatingPasteBox */


/* ARGSUSED */
static void RedrawFloatingPasteOutline(pew,rx1,ry1,rx2,ry2)
PixelEditorWidget pew;
int rx1,ry1,rx2,ry2;
{
	Parameters *p;
	Box *paste_box;
	PasteBuffer *pb;
	EditorState *es;
	int px1,py1,px2,py2;
	int size,x1,y1,x2,y2,w,h;

	p = PEPartParams(pew);
	es = PEPartEditorState(pew);
	pb = ESPasteBuffer(es);
	paste_box = PasteBufferBox(pb);
	if ((!PasteBufferValid(pb)) || (!PasteBufferPasted(pb))) return;
	size = PEWPixelSize(pew);
	px1 = max(0,BoxX1(paste_box));
	py1 = max(0,BoxY1(paste_box));
	px2 = min(BoxX2(paste_box),CoordsHPixels(PEPartCoords(pew)) - 1);
	py2 = min(BoxY2(paste_box),CoordsVPixels(PEPartCoords(pew)) - 1);
	PixelsPixelToPoint(pew,px1,py1,&x1,&y1);
	PixelsPixelToPoint(pew,px2,py2,&x2,&y2);
	x2 = x2 + size - 1 + (ParamsGridOn(p) == True);
	y2 = y2 + size - 1 + (ParamsGridOn(p) == True);
	w = x2 - x1;
	h = y2 - y1;
	XDrawRectangle(XtDisplay(pew),XtWindow(pew),
		       ESPasteBoxGC(es),x1,y1,w,h);
} /* End RedrawFloatingPasteOutline */


/* ARGSUSED */
static void RedrawPadding(pew,box)
PixelEditorWidget pew;
Box *box;
{
	Coords *coords;
	EditorState *es;

	coords = PEPartCoords(pew);
	es = PEPartEditorState(pew);

	if (BoxW(CoordsPadRight(coords)) && BoxH(CoordsPadRight(coords)))
	{
		XFillRectangle(XtDisplay(pew),XtWindow(pew),ESPadGC(es),
			BoxX(CoordsPadRight(coords)),
			BoxY(CoordsPadRight(coords)),
			BoxW(CoordsPadRight(coords)),
			BoxH(CoordsPadRight(coords)));
		XDrawLine(XtDisplay(pew),XtWindow(pew),ESGC(es),
			  BoxX(CoordsPadRight(coords)),
			  BoxY(CoordsPadRight(coords)),
			  BoxX(CoordsPadRight(coords)),
			  BoxY2(CoordsPadRight(coords)) + 1);
	}
	if (BoxW(CoordsPadBottom(coords)) && BoxH(CoordsPadBottom(coords)))
	{
		XFillRectangle(XtDisplay(pew),XtWindow(pew),ESPadGC(es),
			BoxX(CoordsPadBottom(coords)),
			BoxY(CoordsPadBottom(coords)),
			BoxW(CoordsPadBottom(coords)),
			BoxH(CoordsPadBottom(coords)));
		XDrawLine(XtDisplay(pew),XtWindow(pew),ESGC(es),
			  BoxX(CoordsPadBottom(coords)),
			  BoxY(CoordsPadBottom(coords)),
			  BoxX(CoordsPadRight(coords)),
			  BoxY(CoordsPadBottom(coords)));

	}
} /* End RedrawPadding */


static void RedrawClippers(pew,box)
PixelEditorWidget pew;
Box *box;
{
	RedrawHorizontalClippers(pew,box);
	RedrawVerticalClippers(pew,box);
} /* End RedrawClippers */


static void RedrawHorizontalClippers(pew,box)
PixelEditorWidget pew;
Box *box;
{
	Box ibox;
	Box *ch_box;
	Parameters *p;
	Coords *coords;
	EditorState *es;
	int height,base;
	XPoint boundary[5];
	XPoint up_triangle[4];
	int ccenter,cx,cy;

	p = PEPartParams(pew);
	if (!ParamsClippersOn(p)) return;
	coords = PEPartCoords(pew);
	ch_box = CoordsClipHor(coords);
	es = PEPartEditorState(pew);
	if (!BoxIntersect(box,ch_box,&ibox)) return;
	XClearArea(XtDisplay(pew),XtWindow(pew),BoxX(&ibox),BoxY(&ibox),
		   BoxW(&ibox),BoxH(&ibox),FALSE);
	RedrawUpDownClipLines(pew,box);
	if (BoxH(ch_box) > 1)
	{
		height = BoxH(ch_box) - 1;
		base = (height * 2) / 5;
		if (odd(base)) ++ base;

		up_triangle[1].x = base / 2;	/* Relative Coordinates */
		up_triangle[1].y = height;
		up_triangle[2].x = - base;
		up_triangle[2].y = 0;
		up_triangle[3].x = base / 2;
		up_triangle[3].y = - height;

		if (PixelsXPixelToXPoint(pew,ParamsHClipLeft(p),&ccenter))
		{
			cx = ccenter;
			cy = BoxY(CoordsClipHor(coords)) + 1;
			up_triangle[0].x = cx;
			up_triangle[0].y = cy;
			XFillPolygon(XtDisplay(pew),XtWindow(pew),ESGC(es),
				     up_triangle,4,Convex,CoordModePrevious);
		}
		if (PixelsXPixelToXPoint(pew,ParamsHClipRight(p),&ccenter))
		{
			cx = ccenter;
			cy = BoxY(CoordsClipHor(coords)) + 1;
			up_triangle[0].x = cx;
			up_triangle[0].y = cy;
			XFillPolygon(XtDisplay(pew),XtWindow(pew),ESGC(es),
				     up_triangle,4,Convex,CoordModePrevious);
		}
	}
	boundary[0].x = BoxX(ch_box);
	boundary[0].y = BoxY(ch_box);
	boundary[1].x = BoxX2(ch_box);
	boundary[1].y = BoxY(ch_box);
	boundary[2].x = BoxX2(ch_box);
	boundary[2].y = BoxY2(ch_box);
	boundary[3].x = BoxX(ch_box);
	boundary[3].y = BoxY2(ch_box);
	boundary[4].x = BoxX(ch_box);
	boundary[4].y = BoxY(ch_box);
	XDrawLines(XtDisplay(pew),XtWindow(pew),ESGC(es),
		   boundary,5,CoordModeOrigin);
} /* End RedrawHorizontalClippers */


static void RedrawVerticalClippers(pew,box)
PixelEditorWidget pew;
Box *box;
{
	Box ibox;
	Box *cv_box;
	Parameters *p;
	EditorState *es;
	int height,base;
	XPoint boundary[5];
	XPoint left_triangle[4];
	int ccenter,cx,cy;

	p = PEPartParams(pew);
	if (!ParamsClippersOn(p)) return;
	cv_box = CoordsClipVer(PEPartCoords(pew));
	es = PEPartEditorState(pew);

	if (!BoxIntersect(box,cv_box,&ibox)) return;
	XClearArea(XtDisplay(pew),XtWindow(pew),BoxX(&ibox),BoxY(&ibox),
		   BoxW(&ibox),BoxH(&ibox),FALSE);
	RedrawLeftRightClipLines(pew,box);
	if (BoxW(cv_box) > 1)
	{
		height = BoxW(cv_box) - 1;
		base = (height * 2) / 5;
		if (odd(base)) ++ base;

		left_triangle[1].x = height;
		left_triangle[1].y = base / 2;
		left_triangle[2].x = 0;
		left_triangle[2].y = - base;
		left_triangle[3].x = - height;
		left_triangle[3].y = base / 2;

		if (PixelsYPixelToYPoint(pew,ParamsVClipTop(p),&ccenter))
		{
			cx = BoxX(cv_box) + 1;
			cy = ccenter;
			left_triangle[0].x = cx;
			left_triangle[0].y = cy;
			XFillPolygon(XtDisplay(pew),XtWindow(pew),ESGC(es),
				     left_triangle,4,Convex,CoordModePrevious);
		}
		if (PixelsYPixelToYPoint(pew,ParamsVClipBottom(p),&ccenter))
		{
			cx = BoxX(cv_box) + 1;
			cy = ccenter;
			left_triangle[0].x = cx;
			left_triangle[0].y = cy;
			XFillPolygon(XtDisplay(pew),XtWindow(pew),ESGC(es),
				     left_triangle,4,Convex,CoordModePrevious);
		}
	}
	boundary[0].x = BoxX(cv_box);
	boundary[0].y = BoxY(cv_box);
	boundary[1].x = BoxX2(cv_box);
	boundary[1].y = BoxY(cv_box);
	boundary[2].x = BoxX2(cv_box);
	boundary[2].y = BoxY2(cv_box);
	boundary[3].x = BoxX(cv_box);
	boundary[3].y = BoxY2(cv_box);
	boundary[4].x = BoxX(cv_box);
	boundary[4].y = BoxY(cv_box);
	XDrawLines(XtDisplay(pew),XtWindow(pew),ESGC(es),
		   boundary,5,CoordModeOrigin);
} /* End RedrawVerticalClippers */


static void RedrawUpDownClipLines(pew,box)
PixelEditorWidget pew;
Box *box;
{
	XSegment lines[2];
	Parameters *p;
	int cx1,cx2,num_lines;
	EditorState *es;
	Box ibox,*pa_box;

	p = PEPartParams(pew);
	es = PEPartEditorState(pew);
	pa_box = CoordsPixelArea(PEPartCoords(pew));
	if (ParamsGridOn(p) == False) return;
	if (!BoxIntersect(box,pa_box,&ibox)) return;
	num_lines = 0;
	if ((ParamsHClipLeft(p) != 0) &&
	    PixelsXPixelToXPoint(pew,ParamsHClipLeft(p),&cx1) &&
	    (cx1 >= BoxX(&ibox) && cx1 <= BoxX2(&ibox)))
	{
/*
		lines[num_lines].x1 = cx1;
		* The following line corrects the period 8 dashing phase *
		lines[num_lines].y1 = iy & ~7 + (ESDims(es)->pay1 % 8);
		lines[num_lines].x2 = lines[num_lines].x1;
		lines[num_lines].y2 = iy + ih - 1;
*/
		lines[num_lines].x1 = cx1;
		lines[num_lines].y1 = BoxY(pa_box);
		lines[num_lines].x2 = lines[num_lines].x1;
		lines[num_lines].y2 = BoxY2(pa_box);
		++ num_lines;
	}
	if ((ParamsHClipRight(p) != CoordsHPixels(PEPartCoords(pew))-1) &&
	    PixelsXPixelToXPoint(pew,ParamsHClipRight(p),&cx2) &&
	    (cx2 >= BoxX(&ibox) && cx2 <= BoxX2(&ibox)))
	{
/*
		lines[num_lines].x1 = cx2;
		lines[num_lines].y1 = iy & ~7 + (ESDims(es)->pay1 % 8);
		lines[num_lines].x2 = lines[num_lines].x1;
		lines[num_lines].y2 = iy + ih - 1;
*/
		lines[num_lines].x1 = cx2;
		lines[num_lines].y1 = BoxY(pa_box);
		lines[num_lines].x2 = lines[num_lines].x1;
		lines[num_lines].y2 = BoxY2(pa_box);
		++ num_lines;
	}
	if (num_lines > 0)
	{
		XDrawSegments(XtDisplay(pew),XtWindow(pew),ESDashedGC(es),
			      lines,num_lines);
	}
} /* End RedrawUpDownClipLines */


static void RedrawLeftRightClipLines(pew,box)
PixelEditorWidget pew;
Box *box;
{
	Parameters *p;
	XSegment lines[2];
	int cy1,cy2,num_lines;
	EditorState *es;
	Box ibox,*pa_box;

	p = PEPartParams(pew);
	es = PEPartEditorState(pew);
	pa_box = CoordsPixelArea(PEPartCoords(pew));
	if (ParamsGridOn(p) == False) return;
	if (!BoxIntersect(box,pa_box,&ibox)) return;
	num_lines = 0;

	if ((ParamsVClipTop(p) != 0) &&
	    PixelsYPixelToYPoint(pew,ParamsVClipTop(p),&cy1) &&
	    (cy1 >= BoxY(pa_box) && cy1 <= BoxY2(pa_box)))
	{
		/* The following line corrects the period 8 dashing phase */
/*
		lines[num_lines].x1 = ix & ~7 + (ESDims(es)->pax1 % 8);
		lines[num_lines].y1 = cy1;
		lines[num_lines].x2 = ix + iw - 1;
		lines[num_lines].y2 = lines[num_lines].y1;
*/
		lines[num_lines].x1 = BoxX(pa_box);
		lines[num_lines].y1 = cy1;
		lines[num_lines].x2 = BoxX2(pa_box);
		lines[num_lines].y2 = lines[num_lines].y1;
		++ num_lines;
	}
	if ((ParamsVClipBottom(p) != CoordsVPixels(PEPartCoords(pew))-1) &&
	    PixelsYPixelToYPoint(pew,ParamsVClipBottom(p),&cy2) &&
	    (cy2 >= BoxY(pa_box) && cy2 <= BoxY2(pa_box)))
	{
/*
		lines[num_lines].x1 = ix & ~7 + (ESDims(es)->pax1 % 8);
		lines[num_lines].y1 = cy2;
		lines[num_lines].x2 = ix + iw - 1;
		lines[num_lines].y2 = lines[num_lines].y1;
*/
		lines[num_lines].x1 = BoxX(pa_box);
		lines[num_lines].y1 = cy2;
		lines[num_lines].x2 = BoxX2(pa_box);
		lines[num_lines].y2 = lines[num_lines].y1;
		++ num_lines;
	}
	if (num_lines > 0)
	{
		XDrawSegments(XtDisplay(pew),XtWindow(pew),ESDashedGC(es),
			      lines,num_lines);
	}
} /* End RedrawLeftRightClipLines */


static void RedrawGap(pew,box)
PixelEditorWidget pew;
Box *box;
{
	Parameters *p;
	XPoint points[5];
	XSegment lines[10];
	float ct_w,ct_h;
	int num_lines;
	Box *gap;
	
	p = PEPartParams(pew);
	if (ParamsGridOn(p) == False) return;
	gap = CoordsGap(PEPartCoords(pew));
	XClearArea(XtDisplay(pew),XtWindow(pew),BoxX(gap),BoxY(gap),
		   BoxW(gap),BoxH(gap),FALSE);
	points[0].x = BoxX(gap);
	points[0].y = BoxY(gap);
	points[1].x = BoxX2(gap);
	points[1].y = BoxY(gap);
	points[2].x = BoxX2(gap);
	points[2].y = BoxY2(gap);
	points[3].x = BoxX(gap);
	points[3].y = BoxY2(gap);
	points[4].x = BoxX(gap);
	points[4].y = BoxY(gap);
	XDrawLines(XtDisplay(pew),XtWindow(pew),ESGC(PEPartEditorState(pew)),
		   points,5,CoordModeOrigin);
	ct_w = (float)BoxW(gap);
	ct_h = (float)BoxH(gap);
	lines[0].x1 = BoxX(gap) + ct_w * (3.0 / 8.0);
	lines[0].y1 = BoxY(gap) + ct_h * (2.0 / 8.0);
	lines[0].x2 = lines[0].x1;
	lines[0].y2 = BoxY(gap) + ct_h * (6.0 / 8.0);
	lines[1].x1 = BoxX(gap) + ct_w * (5.0 / 8.0);
	lines[1].y1 = BoxY(gap) + ct_h * (2.0 / 8.0);
	lines[1].x2 = lines[1].x1;
	lines[1].y2 = BoxY(gap) + ct_h * (6.0 / 8.0);
	lines[2].x1 = BoxX(gap) + ct_w * (2.0 / 8.0);
	lines[2].y1 = BoxY(gap) + ct_h * (3.0 / 8.0);
	lines[2].x2 = BoxX(gap) + ct_w * (6.0 / 8.0);
	lines[2].y2 = lines[2].y1;
	lines[3].x1 = BoxX(gap) + ct_w * (2.0 / 8.0);
	lines[3].y1 = BoxY(gap) + ct_h * (5.0 / 8.0);
	lines[3].x2 = BoxX(gap) + ct_w * (6.0 / 8.0);
	lines[3].y2 = lines[3].y1;
	num_lines = 4;
	if (ParamsClippersOn(p))
	{
		lines[4].x1 = BoxX(gap);
		lines[4].y1 = BoxY(gap);
		lines[4].x2 = BoxX2(gap);
		lines[4].y2 = BoxY2(gap);
		lines[5].x1 = BoxX2(gap);
		lines[5].y1 = BoxY(gap);
		lines[5].x2 = BoxX(gap);
		lines[5].y2 = BoxY2(gap);
		num_lines = num_lines + 2;
	}
	XDrawSegments(XtDisplay(pew),XtWindow(pew),
		      ESGC(PEPartEditorState(pew)),
		      lines,num_lines);
} /* End RedrawGap */


/* ARGSUSED */
static void FlashPasteOutline(pew,id)
PixelEditorWidget pew;
XtIntervalId *id;
{
	EditorState *es;
	PasteBuffer *pb;
	DisplayData *dd;

	dd = PEPartDisplayData(pew);
	es = PEPartEditorState(pew);
	pb = ESPasteBuffer(es);
	PasteBufferPhase(pb) = (PasteBufferPhase(pb) + 2) % 12;
	XSetDashes(DDDisplay(dd),ESPasteBoxGC(es),PasteBufferPhase(pb),
		   dash_list,2);
	RedrawFloatingPasteOutline(pew,0,0,-1,-1);
	PasteBufferTimer(pb) = XtAddTimeOut(FLASH_TIME,FlashPasteOutline,
					    (char *) pew);
} /* End FlashPasteOutline */
	
/*===========================================================================*

          P I X E L    E D I T O R    S T A T E    M O D I F I E R S

 *===========================================================================*/

static void CancelSelectionBox(pew)
PixelEditorWidget pew;
{
	Box *sel_box;
	EditorState *es;

	es = PEPartEditorState(pew);
	if (ESRegionSelected(es))
	{
		ESRegionSelected(es) = False;
		sel_box = ESSelectionBox(es);
		RedrawRegionOfPixels(pew,sel_box);
		NotifyRegionChange(pew);
	}
} /* End CancelSelectionBox */


static void CancelPasteBox(pew)
PixelEditorWidget pew;
{
	Box *paste_box;
	PasteBuffer *pb;
	EditorState *es;

	es = PEPartEditorState(pew);
	pb = ESPasteBuffer(es);
	if (PasteBufferPasted(pb))
	{
		paste_box = PasteBufferBox(pb);
		PasteBufferPasted(pb) = False;
		XtRemoveTimeOut(PasteBufferTimer(pb));
		RedrawRegionOfPixels(pew,paste_box);
	}
} /* End CancelPasteBox */


static void CommitPaste(pew)
PixelEditorWidget pew;
{
	Parameters *p;
	Box *paste_box;
	PasteBuffer *pb;
	EditorState *es;
	ImageData *image;

	p = PEPartParams(pew);
	es = PEPartEditorState(pew);
	pb = ESPasteBuffer(es);
	if (PasteBufferPasted(pb))
	{
		image = PasteBufferImage(pb);
		paste_box = PasteBufferBox(pb);
		PixelEditorPutSubImageData(pew,image,
					   BoxX(paste_box),BoxY(paste_box),
					   ParamsOpaquePaste(p));
		CancelPasteBox(pew);
		NotifyDataChange(pew);
	}
} /* End CommitPaste */


/*===========================================================================*

                      A C T I O N    H A N D L E R S

 *===========================================================================*/

/* ARGSUSED */
static void Click(pew,event,params,num_params)
PixelEditorWidget pew;
XEvent *event;
char *params;
Cardinal *num_params;
{
	EventHandleButtonPress(pew,(XButtonPressedEvent *)event);
} /* End Click */

/*===========================================================================*

                       E V E N T    H A N D L E R S

 *===========================================================================*/

static void EventHandleButtonPress(pew,press)
PixelEditorWidget pew;
XButtonPressedEvent *press;
{
	int x,y;
	DisplayData *dd;
	Box *pa_box,*ch_box,*cv_box,*gap_box;

	x = press->x;
	y = press->y;
	dd = PEPartDisplayData(pew);
	pa_box = CoordsPixelArea(PEPartCoords(pew));
	ch_box = CoordsClipHor(PEPartCoords(pew));
	cv_box = CoordsClipVer(PEPartCoords(pew));
	gap_box = CoordsGap(PEPartCoords(pew));

	if (PointInBox(x,y,pa_box))
		DoClickInPixelArea(pew,press->button,press->x,press->y);
	    else if (PointInBox(x,y,ch_box))
		DoClickInClipperArea(pew,press->button,0,press->x,press->y);
	    else if (PointInBox(x,y,cv_box))
		DoClickInClipperArea(pew,press->button,1,press->x,press->y);
	    else if (PointInBox(x,y,gap_box))
		DoClickInGapArea(pew,press->button,press->x,press->y);

	
	XUngrabButton(DDDisplay(dd),AnyButton,AnyModifier,XtWindow(pew));
	XGrabButton(DDDisplay(dd),AnyButton,AnyModifier,XtWindow(pew),
		    False,
		    (ButtonReleaseMask | ButtonPressMask | PointerMotionMask),
		    GrabModeAsync,GrabModeAsync,None,None);
} /* End EventHandleButtonPress */


static void DoClickInPixelArea(pew,button,x,y)
PixelEditorWidget pew;
unsigned int button;
int x,y;
{
	ToolHandleClick(pew,button,x,y);
} /* End DoClickInPixelArea */


static void DoClickInClipperArea(pew,button,hor_or_ver,x,y)
PixelEditorWidget pew;
unsigned int button;
int hor_or_ver,x,y;
{
	Coords *c;
	Parameters *p;
	DisplayData *dd;
	Box outer,inner;
	int first_clipper,second_clipper,coord,base_coord,resulting_pixel;
	int half_pixel,final_x,final_y,status,closest_clipper;

	if (!ParamsClippersOn(PEPartParams(pew))) return;
	c = PEPartCoords(pew);
	p = PEPartParams(pew);
	dd = PEPartDisplayData(pew);
	CursorPush(pew,ESPointingCursor(PEPartEditorState(pew)));
	if (hor_or_ver == 0)				/* Horizontal */
	{
		PixelsXPixelToXPoint(pew,ParamsHClipLeft(p),&first_clipper);
		PixelsXPixelToXPoint(pew,ParamsHClipRight(p),&second_clipper);
		if (abs(x - first_clipper) <= abs(x - second_clipper))
			closest_clipper = 0;
		    else
			closest_clipper = 1;
		BoxX(&outer) = BoxX(CoordsClipHor(c));
		BoxY(&outer) = BoxY(CoordsPixelArea(c));
		BoxW(&outer) = BoxW(CoordsPixelArea(c)) + 1;
		BoxH(&outer) = BoxH(CoordsPixelArea(c)) +
			BoxH(CoordsPadBottom(c)) + BoxH(CoordsClipHor(c));
		BoxX(&inner) = x;
		BoxY(&inner) = BoxY(&outer) + 1;
		BoxW(&inner) = 1;
		BoxH(&inner) = BoxH(&outer) - 2;
	}
	    else					/* Vertical */
	{
		PixelsYPixelToYPoint(pew,ParamsVClipTop(p),&first_clipper);
		PixelsYPixelToYPoint(pew,ParamsVClipBottom(p),&second_clipper);
		if (abs(y - first_clipper) <= abs(y - second_clipper))
			closest_clipper = 0;
		    else
			closest_clipper = 1;
		BoxX(&outer) = BoxX(CoordsPixelArea(c));
		BoxY(&outer) = BoxY(CoordsClipVer(c));
		BoxW(&outer) = BoxW(CoordsPixelArea(c)) +
			BoxW(CoordsPadRight(c)) + BoxW(CoordsClipVer(c));
		BoxH(&outer) = BoxH(CoordsPixelArea(c)) + 1;
		BoxX(&inner) = BoxX(&outer) + 1;
		BoxY(&inner) = y;
		BoxW(&inner) = BoxW(&outer) - 2;
		BoxH(&inner) = 1;
	}
	status = DragConstrainedBox(DDDisplay(dd),XtWindow(pew),
				    ESGC(PEPartEditorState(pew)),
				    button,x,y,&outer,&inner,
				    &final_x,&final_y);
	CursorPop(pew);
	half_pixel = PEWPixelSize(pew) / 2;
	coord = (hor_or_ver == 0 ? final_x : final_y);
	base_coord = (hor_or_ver == 0 ? BoxX(CoordsClipHor(c)) :
		      BoxY(CoordsClipVer(c)));
	resulting_pixel = (coord - base_coord + half_pixel) /
		PEWPixelSize(pew);
	if (hor_or_ver == 0)
	{
		resulting_pixel = min(max(resulting_pixel,0),
				      CoordsHPixels(c) + 1);
		if (closest_clipper == 0)
			ParamsHClipLeft(p) = resulting_pixel;
		    else
			ParamsHClipRight(p) = resulting_pixel;
		if (ParamsHClipLeft(p) > ParamsHClipRight(p))
			swap(ParamsHClipLeft(p),ParamsHClipRight(p));
	}
	    else
	{
		resulting_pixel = min(max(resulting_pixel,0),
				      CoordsVPixels(c) + 1);
		if (closest_clipper == 0)
			ParamsVClipTop(p) = resulting_pixel;
		    else
			ParamsVClipBottom(p) = resulting_pixel;
		if (ParamsVClipTop(p) > ParamsVClipBottom(p))
			swap(ParamsVClipTop(p),ParamsVClipBottom(p));
	}
	RedrawAll(pew);
} /* End DoClickInClipperArea */


/* ARGSUSED */
static void DoClickInGapArea(pew,button,x,y)
PixelEditorWidget pew;
unsigned int button;
int x,y;
{
} /* End DoClickInGapArea */

/*===========================================================================*

                     T O O L    I N T E R A C T I O N

 *===========================================================================*/

static void ToolHandleClick(pew,button,x,y)
PixelEditorWidget pew;
unsigned int button;
int x,y;
{
	if (ESRegionSelected(PEPartEditorState(pew)))
	{
		CancelSelectionBox(pew);
	}
	switch (ParamsTool(PEPartParams(pew)))
	{
	    case TOOL_PENCIL:
		ToolDraw(pew,button,x,y,TOOL_PENCIL);
		break;
	    case TOOL_ERASER:
		ToolDraw(pew,button,x,y,TOOL_ERASER);
		break;
	    case TOOL_BRUSH:
		ToolDraw(pew,button,x,y,TOOL_BRUSH);
		break;
	    case TOOL_REGION:
		ToolRegion(pew,button,x,y);
		break;
	    case TOOL_PASTER:
		ToolPaste(pew,button,x,y);
		break;
	    default:
		fprintf(stderr,"Unknown Tool %d\n",
			ParamsTool(PEPartParams(pew)));
		break;
	}
} /* End ToolHandleClick */


static void ToolPaste(pew,button,x,y)
PixelEditorWidget pew;
unsigned int button;
int x,y;
{
	Boolean status;
	PasteBuffer *pb;
	EditorState *es;
	DisplayData *dd;
	Box *paste_box,inner,outer;
	int h_pixels,v_pixels,px,py,x1,y1,x2,y2;

	es = PEPartEditorState(pew);
	dd = PEPartDisplayData(pew);
	pb = ESPasteBuffer(es);
	if (PasteBufferValid(pb) == False) return;

	paste_box = PasteBufferBox(pb);
	h_pixels = CoordsHPixels(PEPartCoords(pew));
	v_pixels = CoordsVPixels(PEPartCoords(pew));

	PixelsPointToPixel(pew,x,y,&px,&py);
	if (PasteBufferPasted(pb) && PointInBox(px,py,paste_box))
	{
		outer = *CoordsPixelArea(PEPartCoords(pew));
		-- BoxX(&outer);
		-- BoxY(&outer);
		BoxW(&outer) += 2;
		BoxH(&outer) += 2;
		PixelsPixelToPoint(pew,BoxX1(paste_box),BoxY1(paste_box),
				   &x1,&y1);
		PixelsPixelToPoint(pew,BoxX2(paste_box)+1,BoxY2(paste_box)+1,
				   &x2,&y2);
		BoxX(&inner) = x1;
		BoxY(&inner) = y1;
		BoxW(&inner) = x2 - x1;
		BoxH(&inner) = y2 - y1;
		status = DragConstrainedBox(DDDisplay(dd),XtWindow(pew),
					    ESGC(es),button,x,y,&outer,&inner,
					    &x,&y);
		x = x + PEWPixelSize(pew) / 2;
		y = y + PEWPixelSize(pew) / 2;
		CancelPasteBox(pew);
		if (status == False) return;
	}
	if (PasteBufferPasted(pb)) CancelPasteBox(pew);
	if (PixelsPointToPixel(pew,x,y,&px,&py) == False) return;
	BoxX(paste_box) = max(0,min(px,h_pixels - BoxW(paste_box)));
	BoxY(paste_box) = max(0,min(py,v_pixels - BoxH(paste_box)));
	PasteBufferPasted(pb) = True;
	PasteBufferTimer(pb) = XtAddTimeOut(FLASH_TIME,FlashPasteOutline,
					    (char *) pew);
	RedrawRegionOfPixels(pew,paste_box);
} /* End ToolPaste */


static void ToolDraw(pew,button,x,y,tool)
PixelEditorWidget pew;
unsigned int button;
int x,y,tool;
{
	EditorState *es;
	DisplayData *dd;
	Parameters *params;
	int px,py,old_px,old_py;
	Pixel color,old_color;
	Boolean cursor_pushed;
	XEvent event;
	XMotionEvent *motion_event;
	Display *dpy;

	params = PEPartParams(pew);
	es = PEPartEditorState(pew);
	dd = PEPartDisplayData(pew);
	dpy = DDDisplay(dd);
	PixelsPointToPixel(pew,x,y,&px,&py);
	old_color = PixelsGetPixel(pew,px,py);
	cursor_pushed = False;
	switch(tool)
	{
	    case TOOL_PENCIL:
		if (button == Button1)				/* Draw */
		{
			color = ParamsDrawColor(params);
		}
		    else if (button == Button2)			/* Toggle */
		{
			if (old_color != ParamsEraseColor(params))
			{
				color = ParamsEraseColor(params);
				CursorPush(pew,ESEPencilCursor(es));
				cursor_pushed = True;
			}
			    else
			{
				color = ParamsDrawColor(params);
			}
		}
		    else					/* Erase */
		{
			color = ParamsEraseColor(params);
			CursorPush(pew,ESEPencilCursor(es));
			cursor_pushed = True;
		}
		break;
	    case TOOL_ERASER:
		color = ParamsEraseColor(params);
		break;
	    case TOOL_BRUSH:
		color = ParamsDrawColor(params);
		break;
	}
	old_px = -1;				/* Just To Make Old != New */
	old_py = -1;
	while (1)
	{
		if (px != old_px || py != old_py)
		{
			if (old_px < 0 || old_py < 0)
			{
				old_px = px;
				old_py = py;
			}
			PixelsDrawStroke(pew,old_px,old_py,px,py,color,tool);
			old_px = px;
			old_py = py;
		}
		XNextEvent(dpy,&event);
		switch(event.type)
		{
		case MotionNotify:
			motion_event = (XMotionEvent *)(&event);
			x = motion_event->x;
			y = motion_event->y;
			PixelsPointToPixel(pew,x,y,&px,&py);
			break;
		case ButtonRelease:
			if (cursor_pushed) CursorPop(pew);
			NotifyDataChange(pew);
			return;
		default:
			printf("Unknown event %d\n",event.type);
			break;
		}
	}
} /* End ToolDraw */


static void ToolRegion(pew,button,x,y)
PixelEditorWidget pew;
unsigned int button;
int x,y;
{
	int status;
	Coords *coords;
	EditorState *es;
	int sx,sy,ex,ey;
	int px1,py1,px2,py2;
	Box constrain_box,result_box,*sel_box;

	coords = PEPartCoords(pew);
	es = PEPartEditorState(pew);
	BoxX(&constrain_box) = BoxX(CoordsPixelArea(coords));
	BoxY(&constrain_box) = BoxY(CoordsPixelArea(coords));
	BoxW(&constrain_box) = BoxW(CoordsPixelArea(coords));
	BoxH(&constrain_box) = BoxH(CoordsPixelArea(coords));
	CursorPush(pew,ESPointingCursor(es));
	status = DragRubberBox(DDDisplay(PEPartDisplayData(pew)),XtWindow(pew),
			       ESGC(PEPartEditorState(pew)),button,x,y,
			       &constrain_box,&result_box);
	CursorPop(pew);
	px1 = BoxX1(&result_box) + PEWPixelSize(pew) / 2;
	py1 = BoxY1(&result_box) + PEWPixelSize(pew) / 2;
	px2 = BoxX2(&result_box) - PEWPixelSize(pew) / 2;
	py2 = BoxY2(&result_box) - PEWPixelSize(pew) / 2;
	PixelsPointToPixel(pew,px1,py1,&sx,&sy);
	PixelsPointToPixel(pew,px2,py2,&ex,&ey);
	if (ex >= sx && ey >= sy)
	{
		sel_box = ESSelectionBox(es);
		BoxX(sel_box) = sx;
		BoxY(sel_box) = sy;
		BoxW(sel_box) = ex - sx + 1;
		BoxH(sel_box) = ey - sy + 1;
		ESRegionSelected(es) = True;
		RedrawSelectionBox(pew,sx,sy,ex,ey);
		NotifyRegionChange(pew);
	}
} /* End ToolRegion */

/*===========================================================================*

               P I X E L  /  P O I N T    R O U T I N E S

 *===========================================================================*/

static Boolean PixelsPixelInsideClippers(pew,px,py)
PixelEditorWidget pew;
int px,py;
{
	if (px < ParamsHClipLeft(PEPartParams(pew)) ||
	    px >= ParamsHClipRight(PEPartParams(pew)) ||
	    py < ParamsVClipTop(PEPartParams(pew)) ||
	    py >= ParamsVClipBottom(PEPartParams(pew)))
	{
		return(False);
	}
	    else
	{
		return(True);
	}
} /* End PixelsPixelInsideClippers */


static Boolean PixelsPointToPixel(pew,x,y,px,py)
PixelEditorWidget pew;
int x,y,*px,*py;
{
	Box *pa_box;
	Coords *coords;
	int pixel_size;
	Boolean status;

	status = True;
	coords = PEPartCoords(pew);
	pa_box = CoordsPixelArea(coords);
	pixel_size = PEWPixelSize(pew);
	if (x < BoxX(pa_box) || x > BoxX2(pa_box))
	{
		*px = (x < BoxX(pa_box) ? 0 : CoordsHPixels(coords) - 1);
		status = False;
	}
	    else
	{
		*px = (x - BoxX(pa_box)) / pixel_size;
	}
	if (y < BoxY(pa_box) || y > BoxY2(pa_box))
	{
		*py = (y < BoxY(pa_box) ? 0 : CoordsVPixels(coords) - 1);
		status = False;
	}
	    else
	{
		*py = (y - BoxY(pa_box)) / pixel_size;
	}
	return(status);
} /* End PixelsPointToPixel */


static Boolean PixelsPixelToPoint(pew,px,py,x,y)
PixelEditorWidget pew;
int px,py,*x,*y;
{
	Boolean status;

	status = ((PixelsXPixelToXPoint(pew,px,x) == True) &&
		  (PixelsYPixelToYPoint(pew,py,y) == True));
	return(status);
} /* End PixelsPixelToPoint */

	
static Boolean PixelsXPixelToXPoint(pew,px,x)
PixelEditorWidget pew;
int px,*x;
{
	Box *pa_box;
	int pixel_size;

	pa_box = CoordsPixelArea(PEPartCoords(pew));
	pixel_size = PEWPixelSize(pew);
	*x = BoxX(pa_box) + px * pixel_size;
	if (px < 0 || px > CoordsHPixels(PEPartCoords(pew)))
		return(False);
	    else
		return(True);
} /* End PixelsXPixelToXPoint */
	

static Boolean PixelsYPixelToYPoint(pew,py,y)
PixelEditorWidget pew;
int py,*y;
{
	Box *pa_box;
	int pixel_size;

	pa_box = CoordsPixelArea(PEPartCoords(pew));
	pixel_size = PEWPixelSize(pew);
	*y = BoxY(pa_box) + py * pixel_size;
	if (py < 0 || py > CoordsVPixels(PEPartCoords(pew)))
		return(False);
	    else
		return(True);
} /* End PixelsYPixelToYPoint */

/*===========================================================================*

               P I X E L    E D I T I N G    R O U T I N E S

 *===========================================================================*/

static Pixel PixelsGetPixel(pew,px,py)
PixelEditorWidget pew;
int px,py;
{
	register ImageData *image;
	
	image = ESImage(PEPartEditorState(pew));
	return(ImagePixels(image)[ImageWidth(image) * py + px]);
} /* End PixelsGetPixel */


static void PixelsSetPixel(pew,px,py,color)
PixelEditorWidget pew;
int px,py;
Pixel color;
{
	register ImageData *image;
	
	image = ESImage(PEPartEditorState(pew));
	ImagePixels(image)[ImageWidth(image) * py + px] = color;
} /* End PixelsSetPixel */


static void PixelsDrawOneToolBlob(pew,px,py,color,tool)
PixelEditorWidget pew;
int px,py,tool;
Pixel color;
{
	switch(tool)
	{
	    case TOOL_PENCIL:
		PixelsSetPixel(pew,px,py,color);
		PixelsRedrawRegion(pew,px,py,px,py);
		break;
	    case TOOL_ERASER:
	    case TOOL_BRUSH:
		PixelsDrawOneBrushBlob(pew,px,py,color);
		break;
	    default:
		fprintf(stderr,"Drawing tool unimplemented\n");
		return;
	}
} /* End PixelsDrawOneToolBlob */


static void PixelsDrawOneBrushBlob(pew,px,py,color)
PixelEditorWidget pew;
int px,py;
Pixel color;
{
	Coords *c;
	Bits *brush_bits;
	EditorState *es;
	Parameters *params;
	int brush_number,x,y,cx,cy,sx,sy,ex,ey,x_index,y_index;
	int rx1,ry1,rx2,ry2,bit,byte,bytes_across;
	static unsigned char bit_masks[] = {1,2,4,8,16,32,64,128};

	c = PEPartCoords(pew);
	params = PEPartParams(pew);
	es = PEPartEditorState(pew);

	brush_number = ParamsBrushShape(params);
	if (brush_number == BRUSH_USER_DEFINED)
	{
		brush_bits = ESUserBrushBits(es);
		if (BitsData(brush_bits) == NULL)
		{
			brush_bits = &(brush_shapes[BRUSH_SMALL_BOX]);
		}
	}
	    else if (brush_number < 0 || brush_number >= NUM_BRUSHES)
	{
		brush_bits = &(brush_shapes[BRUSH_SMALL_BOX]);
	}
	    else
	{
		brush_bits = &(brush_shapes[brush_number]);
	}

	bytes_across = (BitsWidth(brush_bits) + 7) / 8;
	cx = (BitsWidth(brush_bits) - 1) / 2;
	cy = (BitsHeight(brush_bits) - 1) / 2;
	sx = px - cx;
	sy = py - cy;
	ex = sx + BitsWidth(brush_bits) - 1;
	ey = sy + BitsHeight(brush_bits) - 1;
	rx1 = ex;
	ry1 = ey;
	rx2 = sx;
	ry2 = sy;
	
	for (y = sy; y <= ey; y++)
	{
		for (x = sx; x <= ex; x++)
		{
			if (x < 0 || x >= CoordsHPixels(c) ||
			    y < 0 || y >= CoordsVPixels(c)) continue;
			x_index = x - sx;
			y_index = y - sy;
			byte = bytes_across * y_index + x_index / 8;
			bit = x_index % 8;
			if (BitsData(brush_bits)[byte] & bit_masks[bit])
			{
				PixelsSetPixel(pew,x,y,color);
				rx1 = min(x,rx1);
				ry1 = min(y,ry1);
				rx2 = max(x,rx2);
				ry2 = max(y,ry2);
			}
		}
	}
	if (rx1 <= rx2 && ry1 <= ry2) PixelsRedrawRegion(pew,rx1,ry1,rx2,ry2);
} /* End PixelsDrawOneBrushBlob */


static void PixelsDrawStroke(pew,px1,py1,px2,py2,color,tool)
PixelEditorWidget pew;
int px1,py1,px2,py2,tool;
Pixel color;
{
	float ratio;
	int x,y,dx,dy,abs_dx,abs_dy,sign_dx,sign_dy;

	dx = px2 - px1;
	dy = py2 - py1;
	abs_dx = abs(dx);
	abs_dy = abs(dy);
	sign_dx = sign(dx);
	sign_dy = sign(dy);
	if (dx == 0 && dy == 0)
	{
		PixelsDrawOneToolBlob(pew,px1,py1,color,tool);
		return;
	}
	if (abs_dx > abs_dy)
	{
		ratio = (float)dy / (float)dx;
		for (x = px1; x != px2 + sign_dx; x = x + sign_dx)
		{
			y = (float)py1 + (float)(x - px1) * ratio;
			PixelsDrawOneToolBlob(pew,x,y,color,tool);
		}
	}
	    else
	{
		ratio = (float)dx / (float)dy;
		for (y = py1; y != py2 + sign_dy; y = y + sign_dy)
		{
			x = (float)px1 + (float)(y - py1) * ratio;
			PixelsDrawOneToolBlob(pew,x,y,color,tool);
		}
	}
} /* End PixelsDrawStroke */


static void PixelsRedrawRegion(pew,startpx,startpy,endpx,endpy)
PixelEditorWidget pew;
int startpx,startpy,endpx,endpy;
{
	EditorState *es;

	es = PEPartEditorState(pew);
	ImageDrawPixels(pew,ESImage(es),startpx,startpy,
			endpx - startpx + 1,endpy - startpy + 1,
			startpx,startpy,True);
} /* End PixelsRedrawRegion */


static void ImageDrawPixels(pew,image,image_x,image_y,
			    image_w,image_h,dest_x,dest_y,opaque)
PixelEditorWidget pew;
ImageData *image;
int image_x,image_y,image_w,image_h;
int dest_x,dest_y;
Boolean opaque;
{
	GC gc;
	Display *dpy;
	EditorState *es;
	DisplayData *dd;
	Parameters *params;
	unsigned char pixel;
	Box p,ibox,*pa_box;
	int px,py,dx,dy,previous_pixel,size;

	es = PEPartEditorState(pew);
	dd = PEPartDisplayData(pew);
	dpy = DDDisplay(dd);
	gc = ESPixelGC(es);
	params = PEPartParams(pew);
	pa_box = CoordsPixelArea(PEPartCoords(pew));
	size = ParamsZoom(params);
	BoxW(&p) = size;
	BoxH(&p) = size;
	XSetForeground(dpy,gc,ParamsEraseColor(params));
	previous_pixel = ParamsEraseColor(params);
	for (py = image_y; py < image_y + image_h; py++)
	{
		for (px = image_x; px < image_x + image_w; px++)
		{
			dx = px - image_x + dest_x;
			dy = py - image_y + dest_y;
			if (!PixelsPixelToPoint(pew,dx,dy,&BoxX(&p),&BoxY(&p)))
			{
				continue;
			}
			pixel = ImagePixel(image,px,py);
			if (pixel == ParamsEraseColor(params) && !opaque)
				continue;
			if (previous_pixel != pixel)
			{
				XSetForeground(dpy,gc,pixel);
				previous_pixel = pixel;
			}
			if (!ParamsClippersOn(PEPartParams(pew)) ||
			    PixelsPixelInsideClippers(pew,dx,dy))
			{
				XSetFillStyle(DDDisplay(dd),gc,FillSolid);
			}
			    else
			{
				XSetFillStyle(DDDisplay(dd),gc,
					      FillOpaqueStippled);
			}
			if (ParamsGridOn(PEPartParams(pew))) ++ BoxX(&p);
			if (ParamsGridOn(PEPartParams(pew))) ++ BoxY(&p);
			if (BoxIntersect(&p,pa_box,&ibox))
			{
				XFillRectangle(dpy,XtWindow(pew),gc,
					       BoxX(&ibox),BoxY(&ibox),
					       BoxW(&ibox),BoxH(&ibox));
			}
		}
	}			
} /* End ImageDrawPixels */


static Pixel *PixelEditorGetSubImageData(pew,box)
PixelEditorWidget pew;
Box *box;
{
	Pixel *data;
	ImageData *image;
	int x,y,size,index;

	size = sizeof(Pixel) * BoxW(box) * BoxH(box);
	data = (Pixel *)XtMalloc(size);
	image = ESImage(PEPartEditorState(pew));
	index = 0;
	for (y = BoxY1(box); y <= BoxY2(box); y++)
	{
		for (x = BoxX1(box); x <= BoxX2(box); x++)
		{
			data[index++] = ImagePixel(image,x,y);
		}
	}
	return(data);
} /* End PixelEditorGetSubImageData */


static void PixelEditorPutSubImageData(pew,source,x,y,opaque)
PixelEditorWidget pew;
ImageData *source;
int x,y;
Boolean opaque;
{
	Pixel src_pixel;
	ImageData *dest;
	DisplayData *dd;
	Parameters *params;
	int sx,sy,dx,dy,final_sx,final_sy;

	dd = PEPartDisplayData(pew);
	params = PEPartParams(pew);
	dest = ESImage(PEPartEditorState(pew));
	final_sx = min(max(ImageWidth(dest) - x - 1,0),ImageWidth(source)-1);
	final_sy = min(max(ImageHeight(dest) - y - 1,0),ImageHeight(source)-1);
	for (sy = 0; sy <= final_sy; sy++)
	{
		for (sx = 0; sx <= final_sx; sx++)
		{
			dx = sx + x;
			dy = sy + y;
			src_pixel = ImagePixel(source,sx,sy);
			if ((src_pixel == ParamsEraseColor(params)) &&
			    (opaque == False))
			{
				continue;
			}
			ImagePixel(dest,dx,dy) = src_pixel;
		}
	}
} /* End PixelEditorPutSubImageData */

/*===========================================================================*

               U S E R    I N T E R F A C E    S U P P O R T

 *===========================================================================*/

/* ARGSUSED */
static int DragConstrainedBox(display,window,gc,button,imx,imy,
			      outer,inner,xp,yp)
Display *display;
Window window;
GC gc;
unsigned int button;
int imx,imy;				/* Initial X & Y Mouse Coordinates */
Box *outer,*inner;
int *xp,*yp;
{
	XEvent event;
	XMotionEvent *motion_event;
	XButtonEvent *button_event;
	int newx,newy;
	int delta_x,delta_y;		/* Offset From Pointer To Upper Left */
	int ul_x,ul_y;			/* Requested Upper Left For Box */
	int wx,wy;			/* Window Coordinates */
	unsigned int ww,wh;
	unsigned int dummy;
	GC xor_gc;			/* GC To Do XORing With */
	Window root;

	*xp = BoxX(inner);
	*yp = BoxY(inner);
	if (BoxX(inner) <= BoxX(outer) || BoxX2(inner) >= BoxX2(outer) ||
		BoxY(inner) <= BoxY(outer) || BoxY2(inner) >= BoxY2(outer))
	{
		return(False);
	}
	xor_gc = XCreateGC(display,window,(long)0,(XGCValues *)NULL);
	XCopyGC(display,gc,-1L,xor_gc);
	XSetFunction(display,xor_gc,GXxor);
	delta_x = BoxX(inner) - imx;	/* Offsets From Mouse To Upper Left */
	delta_y = BoxY(inner) - imy;
	XDrawRectangle(display,window,xor_gc,
		*xp,*yp,BoxW(inner) - 1,BoxH(inner) - 1);
	while (1)
	{
		XNextEvent(display,&event);
		switch(event.type)
		{
		case MotionNotify:
			motion_event = (XMotionEvent *)(&event);
			ul_x = motion_event->x + delta_x;
			ul_y = motion_event->y + delta_y;
			newx = min(BoxX2(outer) - BoxW(inner),ul_x);
			newx = max(newx,BoxX(outer) + 1);
			newy = min(BoxY2(outer) - BoxH(inner),ul_y);
			newy = max(newy,BoxY(outer) + 1);
			if (*xp != newx || *yp != newy)
			{
				XDrawRectangle(display,window,xor_gc,
					       *xp,*yp,
					       BoxW(inner) - 1,
					       BoxH(inner) - 1);
				XDrawRectangle(display,window,xor_gc,
					       newx,newy,
					       BoxW(inner) - 1,
					       BoxH(inner) - 1);
				*xp = newx;
				*yp = newy;
			}
			break;
		case ButtonRelease:
			button_event = (XButtonEvent *)(&event);
			XDrawRectangle(display,window,xor_gc,
				*xp,*yp,BoxW(inner) - 1,BoxH(inner) - 1);
			XGetGeometry(display,window,&root,&wx,&wy,&ww,&wh,
				&dummy,&dummy);
			ul_x = button_event->x + delta_x;
			ul_y = button_event->y + delta_y;
			*xp = min(BoxX2(outer) - BoxW(inner),ul_x);
			*xp = max(*xp,BoxX(outer) + 1);
			*yp = min(BoxY2(outer) - BoxH(inner),ul_y);
			*yp = max(*yp,BoxY(outer) + 1);
			XFreeGC(display,xor_gc);
			if (PointInRect(button_event->x,button_event->y,
				wx,wy,ww,wh))
			{
				return(True);
			}
			    else
			{
				return(False);
			}
		case ButtonPress:
			XDrawRectangle(display,window,xor_gc,
				*xp,*yp,BoxW(inner) - 1,BoxH(inner) - 1);
			*xp = BoxX(inner);
			*yp = BoxY(inner);
			XFreeGC(display,xor_gc);
			return(False);
		default:
			printf("Unexpected event %d\n",event.type);
			break;
		}
	}
} /* End DragConstrainedBox */


static int DragRubberBox(dpy,w,gc,button,ix,iy,constrain_box,result_box)
Display *dpy;
Window w;
GC gc;
unsigned int button;
int ix,iy;
Box *constrain_box,*result_box;
{
	XEvent event;
	XMotionEvent *motion_event;
	XButtonEvent *button_event;
	int x,y,rx1,ry1,rx2,ry2,rw,rh;
	int wx,wy;
	unsigned int ww,wh;
	unsigned int dummy;
	GC xor_gc;			/* GC To Do XORing With */
	Window root;

	xor_gc = XCreateGC(dpy,w,(long)0,(XGCValues *)NULL);
	XCopyGC(dpy,gc,-1L,xor_gc);
	XSetFunction(dpy,xor_gc,GXxor);
	x = ix;
	y = iy;
	while (1)
	{
		rx1 = max(BoxX1(constrain_box),min(ix,x));
		ry1 = max(BoxY1(constrain_box),min(iy,y));
		rx2 = min(BoxX2(constrain_box),max(ix,x));
		ry2 = min(BoxY2(constrain_box),max(iy,y));
		rw = rx2 - rx1 + 1;
		rh = ry2 - ry1 + 1;
		XDrawRectangle(dpy,w,xor_gc,rx1,ry1,rw - 1,rh - 1);

		XNextEvent(dpy,&event);
		switch(event.type)
		{
		case MotionNotify:
			motion_event = (XMotionEvent *)(&event);
			XDrawRectangle(dpy,w,xor_gc,rx1,ry1,rw - 1,rh - 1);
			x = motion_event->x;
			y = motion_event->y;
			break;
		case ButtonRelease:
			button_event = (XButtonEvent *)(&event);
			XDrawRectangle(dpy,w,xor_gc,rx1,ry1,rw - 1,rh - 1);
			XGetGeometry(dpy,w,&root,&wx,&wy,&ww,&wh,
				&dummy,&dummy);
			x = button_event->x;
			y = button_event->y;
			XFreeGC(dpy,xor_gc);
			BoxX(result_box) = rx1;
			BoxY(result_box) = ry1;
			BoxW(result_box) = rx2 - rx1 + 1;
			BoxH(result_box) = ry2 - ry1 + 1;
			if (PointInRect(x,y,wx,wy,ww,wh))
				return(True);
			    else
				return(False);
		default:
			printf("Unexpected event %d\n",event.type);
			break;
		}
	}
} /* End DragConstrainedBox */

/*===========================================================================*

                        S U P P O R T    C O D E

 *===========================================================================*/

static void BoxDump(box)
Box *box;
{
	printf("%d,%d,%d,%d,%d,%d\n",BoxX(box),BoxY(box),
	       BoxW(box),BoxH(box),BoxX2(box),BoxY2(box));
} /* End BoxDump */


static Boolean RectIntersect(x1,y1,w1,h1,x2,y2,w2,h2,xp,yp,wp,hp)
int x1,y1,w1,h1,x2,y2,w2,h2,*xp,*yp,*wp,*hp;
{
	int left,right,top,bottom;

	left = max(x1,x2);
	right = min(x1 + w1,x2 + w2);
	top = max(y1,y2);
	bottom = min(y1 + h1,y2 + h2);
	if (xp)
	{
		*xp = left;
		*yp = top;
		*wp = right - left;
		*hp = bottom - top;
	}
	return((right > left) && (bottom > top));
} /* End RectIntersect */


static Boolean BoxIntersect(box1,box2,ibox)
Box *box1,*box2,*ibox;
{
	return(RectIntersect(BoxX(box1),BoxY(box1),BoxW(box1),BoxH(box1),
			     BoxX(box2),BoxY(box2),BoxW(box2),BoxH(box2),
			     &BoxX(ibox),&BoxY(ibox),&BoxW(ibox),&BoxH(ibox)));
} /* End BoxIntersect */


static Boolean PointInRect(x,y,rx,ry,rw,rh)
int x,y,rx,ry;
unsigned int rw,rh;
{
	if (x >= rx && x < rx + rw && y >= ry && y < ry + rh)
		return(True);
	    else
		return(False);
} /* End PointInRect */


static Boolean PointInBox(x,y,b)
int x,y;
Box *b;
{
	if (x >= BoxX(b) && x <= BoxX2(b) && y >= BoxY(b) && y <= BoxY2(b))
		return(True);
	    else
		return(False);
} /* End PointInBox */


static Colormap ColormapNew(dd)
DisplayData *dd;
{
	Colormap cmap;

	if (DDDynamicColorFlag(dd))
	{
		cmap = XCreateColormap(DDDisplay(dd),DDRoot(dd),
				       DDVisual(dd),AllocAll);
		ColormapSetDefaultColors(dd,cmap);
		return(cmap);
	}
	    else
	{
		return(DDCmap(dd));
	}
} /* End ColormapNew */


static void ColormapSetDefaultColors(dd,cmap)
DisplayData *dd;
Colormap cmap;
{
	int i;
	XColor *colors;

	if (!DDDynamicColorFlag(dd)) return;
	colors = (XColor *)XtMalloc(sizeof(XColor) * DDCells(dd));
	for (i = 0; i < DDCells(dd); i++)
	{
		colors[i].pixel = i;
		colors[i].flags = DoRed | DoGreen | DoBlue;
	}
	XQueryColors(DDDisplay(dd),DDCmap(dd),colors,DDCells(dd));
	XStoreColors(DDDisplay(dd),cmap,colors,DDCells(dd));
	free(colors);
} /* End ColormapSetDefaultColors */
		

static void SetNewImageFromParams(pew)
PixelEditorWidget pew;
{
	EditorState *es;
	Parameters *params;

	es = PEPartEditorState(pew);
	params = PEPartParams(pew);
	if (ESImage(es) != NULL)
	{
		if (ImagePixels(ESImage(es)) != NULL)
		{
			if (ImageShared(ESImage(es)) == False)
			{
				free(ImagePixels(ESImage(es)));
			}
		}
		free(ESImage(es));
	}
	ESImage(es) = ImageDataNew(ParamsImageWidth(params),
				   ParamsImageHeight(params),
				   ParamsImageShared(params),
				   ParamsImagePixels(params),
				   256);
	if (ParamsImageShared(params) == False)
		ParamsImagePixels(params) = ImagePixels(ESImage(es));
} /* End SetNewImageFromParams */


static ImageData *ImageDataNew(w,h,shared,pixels,colors)
int w,h,colors;
Boolean shared;
Pixel *pixels;
{
	int p_size;
	ImageData *image;

	image = (ImageData *)XtMalloc(sizeof(ImageData));
	if (image == NULL) return(NULL);
	ImageNumColors(image) = colors;
	p_size = sizeof(Pixel) * w * h;
	if (shared == False)
	{
		ImagePixels(image) = (Pixel *)XtCalloc(1,p_size);
		if (pixels) memcpy(ImagePixels(image),pixels,p_size);
		ImageShared(image) = False;
	}
	    else
	{
		ImagePixels(image) = pixels;
		ImageShared(image) = True;
	}
	ImageWidth(image) = w;
	ImageHeight(image) = h;
	return(image);
} /* End ImageDataNew */


/*---------------------------------------------------------------------------*

	void ImageResize(image,ox,oy,w,h)

	This routine changes the width and height of the pixel storage
	in the image pointed to by <image> to be <w> and <h>.  The old data
	is copied into the new storage starting at the offsets <ox> and
	<oy> into the pixel matrix.  If the resized size is smaller, data
	is lost.  If it is bigger, any unfilled space will be set to zero.

 *---------------------------------------------------------------------------*/

static void ImageResize(image,ox,oy,w,h)
ImageData *image;
int ox,oy,w,h;
{
	Pixel *pixels;
	int x,y,s_index,d_index;

	pixels = (Pixel *)XtCalloc(1,sizeof(Pixel) * w * h);
	for (y = 0; y < min(h,ImageHeight(image) - oy); y++)
	{
		for (x = 0; x < min(w,ImageWidth(image) - ox); x++)
		{
			d_index = y * h + x;
			s_index = (y + oy) * h + (x + ox);
			if (s_index < 0 ||
			    s_index >= ImageWidth(image) * ImageHeight(image))
			{
				continue;
			}
			pixels[d_index] = ImagePixels(image)[s_index];
		}
	}
	free(ImagePixels(image));
	ImagePixels(image) = pixels;
	ImageWidth(image) = w;
	ImageHeight(image) = h;
} /* End ImageResize */


static Pixmap PixmapCreate(display,d,data,width,height,fg,bg)
Display *display;
Drawable d;
char *data;
unsigned int width, height;
Pixel fg,bg;
{
	Pixmap pixmap;
	int depth,screen;

	screen = XDefaultScreen(display);
	depth = XDefaultDepth(display,screen);
	pixmap = XCreatePixmapFromBitmapData(display,d,data,
		width,height,fg,bg,depth);
	return(pixmap);
} /* End PixmapCreate */


static Pixmap BitmapCreate(display,d,data,width,height)
Display *display;
Drawable d;
char *data;
unsigned int width, height;
{
	return(XCreateBitmapFromData(display,d,data,width,height,0,0));
} /* End BitmapCreate */


static GC GCCreate(display,d,fg,bg,line_style,line_width,fill_type,fill_pixmap)
Display *display;
Drawable d;
Pixel fg,bg;
int line_style,line_width,fill_type;
Pixmap fill_pixmap;
{
	XGCValues gcvals;
	long gc_mask;
	GC gc;

	gc_mask = (GCForeground | GCBackground | GCLineStyle | GCLineWidth);
	gcvals.background = bg;
	gcvals.foreground = fg;
	gcvals.line_style = line_style;
	gcvals.line_width = line_width;
	if (fill_type == FillTiled)
	{
		gc_mask |= (GCTile | GCFillStyle);
		gcvals.tile = fill_pixmap;
		gcvals.fill_style = FillTiled;
	}
	else if (fill_type == FillStippled)
	{
		gc_mask |= (GCStipple | GCFillStyle);
		gcvals.stipple = fill_pixmap;
		gcvals.fill_style = FillStippled;
	}
	gc = XCreateGC(display,d,gc_mask,&gcvals);
	return(gc);
} /* End GCCreate */


static Cursor CreateCursorFromBitmapData(dpy,d,fg,bg,data,mask,
					 width,height,xhot,yhot)
Display *dpy;
Drawable d;
Pixel fg,bg;
char *data,*mask;
unsigned int width,height;
int xhot,yhot;
{
	Cursor cursor;
	Colormap cmap;
	XColor fg_rgb,bg_rgb;
	Pixmap data_pmap,mask_pmap;

	cmap = XDefaultColormap(dpy,XDefaultScreen(dpy));
	data_pmap = XCreateBitmapFromData(dpy,d,data,width,height,0,0);
	mask_pmap = XCreateBitmapFromData(dpy,d,mask,width,height,0,0);
	fg_rgb.pixel = fg;
	bg_rgb.pixel = bg;
	XQueryColor(dpy,cmap,&fg_rgb);
	XQueryColor(dpy,cmap,&bg_rgb);
	cursor = XCreatePixmapCursor(dpy,data_pmap,mask_pmap,
				     &fg_rgb,&bg_rgb,xhot,yhot);
	return(cursor);
} /* End CreateCursorFromBitmapData */


static void CursorStackDump(pew)
PixelEditorWidget pew;
{
	int i;
	EditorState *es;

	es = PEPartEditorState(pew);
	for (i = 0; i < ESCursorSP(es); i++)
		printf("Cursor[%d] = %d\n",i,ESCursorStack(es)[i]);
} /* End CursorStackDump */


static Boolean CursorPush(pew,cursor)
PixelEditorWidget pew;
Cursor cursor;
{
	EditorState *es;
	DisplayData *dd;

	es = PEPartEditorState(pew);
	dd = PEPartDisplayData(pew);
	if (ESCursorSP(es) == CURSOR_STACK_ENTRIES) return(False);
	ESCursorStack(es)[ESCursorSP(es)++] = cursor;
	XDefineCursor(DDDisplay(dd),XtWindow(pew),cursor);
	return(True);
} /* End CursorPush */


static Cursor CursorPop(pew)
PixelEditorWidget pew;
{
	EditorState *es;
	DisplayData *dd;
	Cursor current_cursor;

	es = PEPartEditorState(pew);
	dd = PEPartDisplayData(pew);
	if (ESCursorSP(es) == 0) return(None);
	current_cursor = ESCursorStack(es)[--ESCursorSP(es)];
	if (ESCursorSP(es) == 0)
		XUndefineCursor(DDDisplay(dd),XtWindow(pew));
	    else
		XDefineCursor(DDDisplay(dd),XtWindow(pew),
			      ESCursorStack(es)[ESCursorSP(es) - 1]);
	return(current_cursor);
} /* End CursorPop */


static void CursorChange(pew,cursor)
PixelEditorWidget pew;
Cursor cursor;
{
	EditorState *es;
	DisplayData *dd;

	es = PEPartEditorState(pew);
	dd = PEPartDisplayData(pew);
	if (ESCursorSP(es) == 0)
	{
		fprintf(stderr,"CursorChange: sp = 0\n");
		return;
	}
	ESCursorStack(es)[ESCursorSP(es) - 1] = cursor;
	XDefineCursor(DDDisplay(dd),XtWindow(pew),cursor);
} /* End CursorChange */


static Boolean FileReadBits(fp,bits_ptr,w_ptr,h_ptr,hot_x_ptr,hot_y_ptr)
FILE *fp;
unsigned char **bits_ptr;
unsigned int *w_ptr,*h_ptr;
int *hot_x_ptr,*hot_y_ptr;
{
	int stat,hot_x,hot_y;

	stat = XmuReadBitmapData(fp,w_ptr,h_ptr,bits_ptr,&hot_x,&hot_y);
	if (hot_x_ptr) *hot_x_ptr = hot_x;
	if (hot_y_ptr) *hot_y_ptr = hot_y;
	if (stat == BitmapSuccess)
		return(True);
	    else
		return(False);
} /* End FileReadBits */


static Boolean FileWriteBits(fp,name,bits,w,h,hot_x,hot_y)
FILE *fp;
char *name;
unsigned char *bits;
int w,h,hot_x,hot_y;
{
	int i,bytes;
	unsigned char c;

	fprintf(fp,"#define %s_width %d\n",name,w);
	fprintf(fp,"#define %s_height %d\n",name,h);
	if (hot_x >= 0) fprintf(fp,"#define %s_x_hot %d\n",name,hot_x);
	if (hot_y >= 0) fprintf(fp,"#define %s_y_hot %d\n",name,hot_y);
	fprintf(fp,"static char %s_bits[] = {",name);
	bytes = (w / 8) * h;
	for (i = 0; i < bytes; i++)
	{
		c = bits[i];
		if (i % 12 == 0)
		{
			fprintf(fp,"\n  ");
		}
		fprintf(fp," 0x%02x",c);
		if (i == bytes - 1)
			fprintf(fp,"};\n");
		    else
			fprintf(fp,",");
	}
	return(True);
} /* End FileWriteBits */


static void NotifyDataChange(pew)
PixelEditorWidget pew;
{
	XtCallCallbacks((Widget)pew,XtNdataChangeCallback,(caddr_t)NULL);
} /* End NotifyDataChange */


static void NotifyRegionChange(pew)
PixelEditorWidget pew;
{
	static Boolean region_selected;

	region_selected = ESRegionSelected(PEPartEditorState(pew));
	XtCallCallbacks((Widget)pew,XtNregionChangeCallback,
			(caddr_t)&region_selected);
} /* End NotifyRegionChange */


static void NotifyPasteBufferChange(pew)
PixelEditorWidget pew;
{
	static Boolean buffer_valid;

	buffer_valid = PasteBufferValid(ESPasteBuffer(PEPartEditorState(pew)));
	XtCallCallbacks((Widget)pew,XtNpasteBufferChangeCallback,
			(caddr_t)&buffer_valid);
} /* End NotifyPasteBufferChange */

/*===========================================================================*

          W I D G E T  /  A P P L I C A T I O N    I N T E R F A C E

 *===========================================================================*/

void PixelEditorRegionFlipHorizontally(pew)
PixelEditorWidget pew;
{
	int i,y;
	Box *sel_box;
	EditorState *es;
	ImageData *image;

	es = PEPartEditorState(pew);
	image = ESImage(es);
	if (ESRegionSelected(es) == False) return;
	sel_box = ESSelectionBox(es);
	for (y = BoxY1(sel_box); y <= BoxY2(sel_box); y++)
	{
		for (i = 0; i < BoxW(sel_box) / 2; i++)
		{
			swap(ImagePixel(image,BoxX1(sel_box) + i,y),
			     ImagePixel(image,BoxX2(sel_box) - i,y));
		}
	}
	RedrawRegionOfPixels(pew,sel_box);
	NotifyDataChange(pew);
} /* End PixelEditorRegionFlipHorizontally */


void PixelEditorRegionFlipVertically(pew)
PixelEditorWidget pew;
{
	int i,x;
	Box *sel_box;
	EditorState *es;
	ImageData *image;

	es = PEPartEditorState(pew);
	image = ESImage(es);
	if (ESRegionSelected(es) == False) return;
	sel_box = ESSelectionBox(es);
	for (x = BoxX1(sel_box); x <= BoxX2(sel_box); x++)
	{
		for (i = 0; i < BoxH(sel_box) / 2; i++)
		{
			swap(ImagePixel(image,x,BoxY1(sel_box) + i),
			     ImagePixel(image,x,BoxY2(sel_box) - i));
		}
	}
	RedrawRegionOfPixels(pew,sel_box);
	NotifyDataChange(pew);
} /* End PixelEditorRegionFlipVertically */


void PixelEditorRegionFill(pew,color)
PixelEditorWidget pew;
Pixel color;
{
	Box *sel_box;
	EditorState *es;
	ImageData *image;
	register int x,y;

	es = PEPartEditorState(pew);
	image = ESImage(es);
	if (ESRegionSelected(es) == False) return;
	sel_box = ESSelectionBox(es);
	for (y = BoxY1(sel_box); y <= BoxY2(sel_box); y++)
	{
		for (x = BoxX1(sel_box); x <= BoxX2(sel_box); x++)
		{
			ImagePixel(image,x,y) = color;
		}
	}
	RedrawRegionOfPixels(pew,sel_box);
	NotifyDataChange(pew);
} /* End PixelEditorRegionFill */


Boolean PixelEditorGetGrid(pew)
PixelEditorWidget pew;
{
	return(ParamsGridOn(PEPartParams(pew)));
} /* End PixelEditorGetGrid */


int PixelEditorGetZoom(pew)
PixelEditorWidget pew;
{
	return(ParamsZoom(PEPartParams(pew)));
} /* End PixelEditorGetZoom */


Pixel PixelEditorGetFGColor(pew)
PixelEditorWidget pew;
{
	return(ParamsDrawColor(PEPartParams(pew)));
} /* End PixelEditorGetFGColor */


Pixel PixelEditorGetBGColor(pew)
PixelEditorWidget pew;
{
	return(ParamsEraseColor(PEPartParams(pew)));
} /* End PixelEditorGetBGColor */


Boolean PixelEditorRegionCopy(pew)
PixelEditorWidget pew;
{
	Box *region;
	PasteBuffer *pb;
	EditorState *es;

	es = PEPartEditorState(pew);
	region = ESSelectionBox(es);
	pb = ESPasteBuffer(es);
	if (ESRegionSelected(es) == False) return(False);
	if (PasteBufferValid(pb))
	{
		if (PasteBufferPasted(pb))
		{
			PasteBufferPasted(pb) = False;
		}
		free(ImagePixels(PasteBufferImage(pb)));
		PasteBufferValid(pb) = False;
	}
	ImagePixels(PasteBufferImage(pb)) =
		PixelEditorGetSubImageData(pew,region);
	ImageWidth(PasteBufferImage(pb)) = BoxW(region);
	ImageHeight(PasteBufferImage(pb)) = BoxH(region);
	BoxW(PasteBufferBox(pb)) = BoxW(region);
	BoxH(PasteBufferBox(pb)) = BoxH(region);
	PasteBufferValid(pb) = True;
	PasteBufferPasted(pb) = False;
	NotifyPasteBufferChange(pew);
	return(True);
} /* End PixelEditorRegionCopy */


void PixelEditorRegionCut(pew)
PixelEditorWidget pew;
{
	PixelEditorRegionCopy(pew);
	PixelEditorRegionFill(pew,ParamsEraseColor(PEPartParams(pew)));
} /* End PixelEditorRegionCut */


Boolean PixelEditorChangeTool(pew,tool)
PixelEditorWidget pew;
int tool;
{
	EditorState *es;

	es = PEPartEditorState(pew);
	if (PasteBufferValid(ESPasteBuffer(es)) &&
	    PasteBufferPasted(ESPasteBuffer(es)))
	{
		if (tool == TOOL_PASTER)
			return(True);
		    else
			CommitPaste(pew);
	}
	if (ESRegionSelected(es))
	{
		if (tool == TOOL_REGION)
			return(True);
		    else
			CancelSelectionBox(pew);
	}
	ParamsTool(PEPartParams(pew)) = tool;
	switch(tool)
	{
	    case TOOL_PENCIL:
		CursorChange(pew,ESPencilCursor(es));
		break;
	    case TOOL_BRUSH:
		CursorChange(pew,ESBrushCursor(es));
		break;
	    case TOOL_ERASER:
		CursorChange(pew,ESEraserCursor(es));
		break;
	    case TOOL_REGION:
		CursorChange(pew,ESUpperLeftCursor(es));
		break;
	    case TOOL_PASTER:
		CursorChange(pew,ESCrosshairCursor(es));
		break;
	}
	return(True);
} /* End PixelEditorChangeTool */


void PixelEditorSetImage(pew,pixels,w,h,shared)
PixelEditorWidget pew;
Pixel *pixels;
unsigned int w,h;
Boolean shared;
{
	Arg args[4];

	XtSetArg(args[0],XtNimageWidth,w);
	XtSetArg(args[1],XtNimageHeight,h);
	XtSetArg(args[2],XtNimagePixels,pixels);
	XtSetArg(args[3],XtNimageShared,shared);
	XtSetValues((Widget)pew,args,4);
} /* End PixelEditorSetImage */


void PixelEditorResizeImage(pew,w,h)
PixelEditorWidget pew;
unsigned int w,h;
{
	Arg args[2];

	XtSetArg(args[0],XtNimageWidth,w);
	XtSetArg(args[1],XtNimageHeight,h);
	XtSetValues((Widget)pew,args,2);
} /* End PixelEditorResizeImage */


/* ARGSUSED */
Pixel *PixelEditorBitmapDataToPixels(pew,bits,w,h,fg_color,bg_color)
PixelEditorWidget pew;
unsigned char *bits;
unsigned int w,h;
Pixel fg_color,bg_color;
{
	Pixel *pixels;
	register int x,y,src_index,dest_index,bytes_across;
	static unsigned char masks[] =
	{
		1, 2, 4, 8, 16, 32, 64, 128
	};

	pixels = (Pixel *)XtMalloc(sizeof(Pixel) * w * h);
	bytes_across = (w + 7) / 8;
	dest_index = 0;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			src_index = (x / 8) + (y * bytes_across);
			if (bits[src_index] & masks[x % 8])
				pixels[dest_index++] = fg_color;
			    else
				pixels[dest_index++] = bg_color;
		}
	}
	return(pixels);
} /* End PixelEditorBitmapDataToPixels */


/* ARGSUSED */
unsigned char *PixelEditorPixelsToBitmapData(pew,pixels,w,h,bg_color)
PixelEditorWidget pew;
Pixel *pixels;
int w,h;
Pixel bg_color;
{
	unsigned char *bits;
	register int x,y,src_index,dest_index,bytes_across;
	static unsigned char masks[] =
	{
		1, 2, 4, 8, 16, 32, 64, 128
	};

	bytes_across = (w + 7) / 8;
	bits = (unsigned char *)XtCalloc(sizeof(unsigned char),
					 bytes_across * h);
	src_index = 0;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			dest_index = (x / 8) + (y * bytes_across);
			if (pixels[src_index++] != bg_color)
				bits[dest_index] |= masks[x % 8];
		}
	}
	return(bits);
} /* End PixelEditorPixelsToBitmapData */


void PixelEditorSetBitmapData(pew,bits,w,h,fg,bg,shared)
PixelEditorWidget pew;
unsigned char *bits;
unsigned int w,h;
Pixel fg,bg;
Boolean shared;
{
	Pixel *pixels;

	pixels = PixelEditorBitmapDataToPixels(pew,bits,w,h,fg,bg);
	PixelEditorSetImage(pew,pixels,w,h,shared);
} /* End PixelEditorSetBitmapData */


Boolean PixelEditorLoadBitmapFile(pew,filename,fg_color,bg_color)
PixelEditorWidget pew;
char *filename;
Pixel fg_color,bg_color;
{
	unsigned int w,h;
	FILE *fp;
	unsigned char *bits;

	fp = fopen(filename,"r");
	if (fp == NULL) return(False);
	if (FileReadBits(fp,&bits,&w,&h,(int *)NULL,(int *)NULL))
	{
		PixelEditorSetBitmapData(pew,bits,w,h,fg_color,bg_color,False);
		return(True);
	}
	    else
	{
		return(False);
	}
} /* End PixelEditorLoadBitmapFile */


Boolean PixelEditorSaveBitmapFile(pew,base_name,filename,bg_color)
PixelEditorWidget pew;
char *base_name;
char *filename;
Pixel bg_color;
{
	FILE *fp;
	ImageData *image;
	EditorState *es;
	unsigned char *bits;

	fp = fopen(filename,"w");
	if (fp == NULL) return(False);
	es = PEPartEditorState(pew);
	image = ESImage(es);
	bits = PixelEditorPixelsToBitmapData(pew,ImagePixels(image),
		ImageWidth(image),ImageHeight(image),bg_color);
	FileWriteBits(fp,base_name,bits,ImageWidth(image),
		      ImageHeight(image),-1,-1);
	fclose(fp);
	free((char *)bits);
	return(True);
} /* End PixelEditorSaveBitmapFile */
