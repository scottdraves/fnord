/****************************************************************************

	PixelEditorP.h

	This file is the private include file for the Pixel Editor widget.

	October 14, 1990, Brian Totty

 ****************************************************************************/

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

#ifndef	_PIXEL_EDITORP_H_
#define	_PIXEL_EDITORP_H_

#define	CURSOR_STACK_ENTRIES	8

/*===========================================================================

                   T Y P E    D E F I N I T I O N S

  ===========================================================================*/

typedef	struct
{
	String name;
	int value;
} Binding;

typedef struct
{
	int x;
	int y;
	int w;
	int h;
} Box;

typedef struct
{
	int x;
	int y;
} Point;

typedef struct
{
	int zoom_factor;
	int left_x_clipper;
	int right_x_clipper;
	int top_y_clipper;
	int bottom_y_clipper;
	int clipper_width;
	int clipper_height;
	Boolean grid_on;
	Boolean clippers_on;
	Boolean opaque_paste;
	Boolean auto_scale;
	int tool;
	int brush_shape;
	Pixel draw_color;
	Pixel erase_color;
	XtCallbackList data_change_callback;
	XtCallbackList region_change_callback;
	XtCallbackList paste_buffer_change_callback;
	int image_width;
	int image_height;
	Boolean image_shared;
	Pixel *image_pixels;
} Parameters;

typedef struct
{
	unsigned short red;
	unsigned short green;
	unsigned short blue;
} COLOR;

typedef struct
{
	int entries;
	COLOR *bindings;
} ColorBindingTable;

typedef struct
{
	int width;
	int height;
	char *bits;
} Bits;

typedef struct
{
	int numcolors;
	int width;
	int height;
	int hot_x,hot_y;
	Boolean shared;
	Pixel *pixels;
} ImageData;

typedef struct
{
	Box box;
	int phase;
	Boolean valid;
	Boolean pasted;
	ImageData image;
	XtIntervalId timer;
} PasteBuffer;

typedef struct
{
	Display *display;
	Window root;
	Screen *screen;
	int screen_number;
	Visual *visual;
	int cells;
	Pixel white,black;
	int dynamic_color_flag;
	Colormap std_cmap;
} DisplayData;

typedef struct
{
	Box window;
	Box pixel_area;
	Box clip_hor;
	Box clip_ver;
	Box pad_bottom;
	Box pad_right;
	Box gap;
	int max_pixels_across,max_pixels_down;	/* Image "Pixels" We Can Fit */
	int pixels_across,pixels_down;		/* Actual Image Pixels Seen */
	int pixel_size;				/* Drawn pixel size */
} Coords;

typedef struct
{
	GC gc;
	GC erase_gc;
	GC dashed_gc;
	GC grid_gc;
	GC pad_gc;
	GC pixel_gc;
	GC selection_box_gc;
	GC paste_box_gc;
	GC xor_gc;
	Cursor grab_cursor;
	Cursor pencil_cursor;
	Cursor epencil_cursor;
	Cursor brush_cursor;
	Cursor eraser_cursor;
	Cursor pointing_hand_cursor;
	Cursor flat_hand_cursor;
	Cursor upper_left_cursor;
	Cursor upper_right_cursor;
	Cursor lower_left_cursor;
	Cursor lower_right_cursor;
	Cursor crosshair_cursor;
	Cursor cursor_stack[CURSOR_STACK_ENTRIES];
	int cursor_sp;
	Colormap cmap;
	int window_mapped;
	ImageData *image;
	PasteBuffer paste_buffer;
	Bits user_brush_bits;
	Boolean region_selected;
	Box selection;
} EditorState;

/*===========================================================================

                             M A C R O S

  ===========================================================================*/

#define	BoxX(b)			((b)->x)
#define	BoxY(b)			((b)->y)
#define	BoxX1(b)		BoxX(b)
#define	BoxY1(b)		BoxY(b)
#define	BoxW(b)			((b)->w)
#define	BoxH(b)			((b)->h)
#define	BoxX2(b)		(BoxX1(b) + BoxW(b) - 1)
#define	BoxY2(b)		(BoxY1(b) + BoxH(b) - 1)

#define	PointX(p)		((p)->x)
#define	PointY(p)		((p)->y)

#define	ParamsZoom(p)		((p)->zoom_factor)
#define	ParamsHClipLeft(p)	((p)->left_x_clipper)
#define	ParamsHClipRight(p)	((p)->right_x_clipper)
#define	ParamsVClipTop(p)	((p)->top_y_clipper)
#define	ParamsVClipBottom(p)	((p)->bottom_y_clipper)
#define	ParamsClipperWidth(p)	((p)->clipper_width)
#define	ParamsClipperHeight(p)	((p)->clipper_height)
#define	ParamsGridOn(p)		((p)->grid_on)
#define	ParamsClippersOn(p)	((p)->clippers_on)
#define	ParamsAutoScale(p)	((p)->auto_scale)
#define	ParamsOpaquePaste(p)	((p)->opaque_paste)
#define	ParamsTool(p)		((p)->tool)
#define	ParamsBrushShape(p)	((p)->brush_shape)
#define	ParamsDrawColor(p)	((p)->draw_color)
#define	ParamsEraseColor(p)	((p)->erase_color)
#define	ParamsDataChangeCB(p)	((p)->data_change_callback)
#define	ParamsRegionChangeCB(p)	((p)->region_change_callback)
#define	ParamsImageWidth(p)	((p)->image_width)
#define	ParamsImageHeight(p)	((p)->image_height)
#define	ParamsImagePixels(p)	((p)->image_pixels)
#define	ParamsImageShared(p)	((p)->image_shared)

#define	BitsWidth(b)		((b)->width)
#define	BitsHeight(b)		((b)->height)
#define	BitsData(b)		((b)->bits)

#define	ImageNumColors(i)	((i)->numcolors)
#define	ImageWidth(i)		((i)->width)
#define	ImageHeight(i)		((i)->height)
#define	ImageHotX(i)		((i)->hot_x)
#define	ImageHotY(i)		((i)->hot_y)
#define	ImageShared(i)		((i)->shared)
#define	ImagePixels(i)		((i)->pixels)
#define	ImageNthPixel(i,n)	(ImagePixels(i)[n])
#define	ImagePixel(i,x,y)	ImageNthPixel((i),(y) * ImageWidth(i) + (x))

#define	PasteBufferBox(pb)	(&((pb)->box))
#define	PasteBufferPhase(pb)	((pb)->phase)
#define	PasteBufferValid(pb)	((pb)->valid)
#define	PasteBufferPasted(pb)	((pb)->pasted)
#define	PasteBufferImage(pb)	(&((pb)->image))
#define	PasteBufferTimer(pb)	((pb)->timer)

#define	DDDisplay(dd)		((dd)->display)
#define	DDRoot(dd)		((dd)->root)
#define	DDScreen(dd)		((dd)->screen)
#define	DDScreenNumber(dd)	((dd)->screen_number)
#define	DDVisual(dd)		((dd)->visual)
#define	DDCells(dd)		((dd)->cells)
#define	DDWhite(dd)		((dd)->white)
#define	DDBlack(dd)		((dd)->black)
#define	DDDynamicColorFlag(dd)	((dd)->dynamic_color_flag)
#define	DDCmap(dd)		((dd)->std_cmap)

#define	CoordsWindow(c)		(&((c)->window))
#define	CoordsPixelArea(c)	(&((c)->pixel_area))
#define	CoordsClipHor(c)	(&((c)->clip_hor))
#define	CoordsClipVer(c)	(&((c)->clip_ver))
#define	CoordsPadBottom(c)	(&((c)->pad_bottom))
#define	CoordsPadRight(c)	(&((c)->pad_right))
#define	CoordsGap(c)		(&((c)->gap))
#define	CoordsMaxHPixels(c)	((c)->max_pixels_across)
#define	CoordsMaxVPixels(c)	((c)->max_pixels_down)
#define	CoordsHPixels(c)	((c)->pixels_across)
#define	CoordsVPixels(c)	((c)->pixels_down)

#define	ESGC(es)		((es)->gc)
#define	ESEraseGC(es)		((es)->erase_gc)
#define	ESDashedGC(es)		((es)->dashed_gc)
#define	ESGridGC(es)		((es)->grid_gc)
#define	ESPadGC(es)		((es)->pad_gc)
#define	ESPixelGC(es)		((es)->pixel_gc)
#define	ESSelectionBoxGC(es)	((es)->selection_box_gc)
#define	ESPasteBoxGC(es)	((es)->paste_box_gc)
#define	ESXorGC(es)		((es)->xor_gc)
#define	ESGrabCursor(es)	((es)->grab_cursor)
#define	ESPencilCursor(es)	((es)->pencil_cursor)
#define	ESEPencilCursor(es)	((es)->epencil_cursor)
#define	ESBrushCursor(es)	((es)->brush_cursor)
#define	ESEraserCursor(es)	((es)->eraser_cursor)
#define	ESPointingCursor(es)	((es)->pointing_hand_cursor)
#define	ESFlatHandCursor(es)	((es)->flat_hand_cursor)
#define	ESUpperLeftCursor(es)	((es)->upper_left_cursor)
#define	ESUpperRightCursor(es)	((es)->upper_right_cursor)
#define	ESLowerLeftCursor(es)	((es)->lower_left_cursor)
#define	ESLowerRightCursor(es)	((es)->lower_right_cursor)
#define	ESCrosshairCursor(es)	((es)->crosshair_cursor)
#define	ESCursorStack(es)	((es)->cursor_stack)
#define	ESCursorSP(es)		((es)->cursor_sp)
#define	ESWindowMapped(es)	((es)->window_mapped)
#define	ESCMap(es)		((es)->cmap)
#define	ESImage(es)		((es)->image)
#define	ESPasteBuffer(es)	(&((es)->paste_buffer))
#define	ESUserBrushBits(es)	(&((es)->user_brush_bits))
#define	ESRegionSelected(es)	((es)->region_selected)
#define	ESSelectionBox(es)	(&((es)->selection))

#define	Core(w)			(&((w)->core))
#define	PEPart(w)		(&((w)->pixelEditor))

#define	PETitle(pe)		((pe)->title)
#define	PECoords(pe)		(&((pe)->coords))
#define	PEDisplayData(pe)	(&((pe)->display_data))
#define	PEParams(pe)		(&((pe)->parameters))
#define	PEEditorState(pe)	(&((pe)->editor_state))

#define	PEPartTitle(w)		(PETitle(PEPart(w)))
#define	PEPartCoords(w)		(PECoords(PEPart(w)))
#define	PEPartDisplayData(w)	(PEDisplayData(PEPart(w)))
#define	PEPartParams(w)		(PEParams(PEPart(w)))
#define	PEPartEditorState(w)	(PEEditorState(PEPart(w)))

#define	PEWPixelSize(pew)	(ParamsZoom(PEPartParams(pew)) +             \
				 (ParamsGridOn(PEPartParams(pew)) == True))

/*===========================================================================*

          W I D G E T    S T R U C T U R E    D E F I N I T I O N

 *===========================================================================*/

typedef struct
{
	String			title;
	Coords			coords;
	DisplayData		display_data;
	Parameters		parameters;
	EditorState		editor_state;
} PixelEditorPart;

typedef struct _PixelEditorClassPart
{
	int    empty;
} PixelEditorClassPart;

typedef struct _PixelEditorClassRec
{
	CoreClassPart		core_class;
	PixelEditorClassPart	pixelEditor_class;
} PixelEditorClassRec;

	/* This Is What A Widget Instance Points To */

typedef struct _PixelEditorRec
{
	CorePart		core;
	PixelEditorPart		pixelEditor;
} PixelEditorRec;

extern PixelEditorClassRec pixelEditorClassRec;

#endif
