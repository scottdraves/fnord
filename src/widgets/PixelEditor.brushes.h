/****************************************************************************

	PixelEditor.brushes

	This file contains the brush shape defininitions and brush shape
	table included by the PixelEditor.c file.

	October 1990, Brian Totty

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

#ifndef _PIXEL_EDITOR_BRUSHES_
#define	_PIXEL_EDITOR_BRUSHES_

/*===========================================================================*

                         B R U S H    B I T M A P S

 *===========================================================================*/

#define brush_small_box_width 3
#define brush_small_box_height 3
static char brush_small_box_bits[] = {
   0x07, 0x07, 0x07};

#define brush_medium_box_width 5
#define brush_medium_box_height 5
static char brush_medium_box_bits[] = {
   0x1f, 0x1f, 0x1f, 0x1f, 0x1f};

#define brush_large_box_width 7
#define brush_large_box_height 7
static char brush_large_box_bits[] = {
   0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f};

#define brush_small_diamond_width 3
#define brush_small_diamond_height 3
static char brush_small_diamond_bits[] = {
   0x02, 0x07, 0x02};

#define brush_medium_diamond_width 5
#define brush_medium_diamond_height 5
static char brush_medium_diamond_bits[] = {
   0x04, 0x0e, 0x1f, 0x0e, 0x04};

#define brush_large_diamond_width 7
#define brush_large_diamond_height 7
static char brush_large_diamond_bits[] = {
   0x08, 0x1c, 0x3e, 0x7f, 0x3e, 0x1c, 0x08};

#define brush_circle_width 7
#define brush_circle_height 7
static char brush_circle_bits[] = {
   0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c};

#define brush_horizontal_line_width 5
#define brush_horizontal_line_height 1
static char brush_horizontal_line_bits[] = {
   0x1f};

#define brush_vertical_line_width 1
#define brush_vertical_line_height 5
static char brush_vertical_line_bits[] = {
   0x01, 0x01, 0x01, 0x01, 0x01};

#define brush_pos_diag_line_width 5
#define brush_pos_diag_line_height 5
static char brush_pos_diag_line_bits[] = {
   0x10, 0x08, 0x04, 0x02, 0x01};

#define brush_neg_diag_line_width 5
#define brush_neg_diag_line_height 5
static char brush_neg_diag_line_bits[] = {
   0x01, 0x02, 0x04, 0x08, 0x10};

/*===========================================================================*

                    B R U S H    S H A P E    T A B L E

 *===========================================================================*/

static Bits brush_shapes[] =
{
	{brush_small_box_width,brush_small_box_height,
		 brush_small_box_bits},
	{brush_medium_box_width,brush_medium_box_height,
		 brush_medium_box_bits},
	{brush_large_box_width,brush_large_box_height,
		 brush_large_box_bits},
	{brush_small_diamond_width,brush_small_diamond_height,
		 brush_small_diamond_bits},
	{brush_medium_diamond_width,brush_medium_diamond_height,
		 brush_medium_diamond_bits},
	{brush_large_diamond_width,brush_large_diamond_height,
		 brush_large_diamond_bits},
	{brush_vertical_line_width,brush_vertical_line_height,
		 brush_vertical_line_bits},
	{brush_horizontal_line_width,brush_horizontal_line_height,
		 brush_horizontal_line_bits},
	{brush_neg_diag_line_width,brush_neg_diag_line_height,
		 brush_neg_diag_line_bits},
	{brush_pos_diag_line_width,brush_pos_diag_line_height,
		 brush_pos_diag_line_bits},
	{brush_circle_width,brush_circle_height,
		 brush_circle_bits},
};

#define	NUM_BRUSHES		11

#endif
