/*!  \file 

$Id: gdisp_plot2D.c,v 1.6 2004/05/19 15:10:57 dufy Exp $

-----------------------------------------------------------------------

GDISP+ - Graphic Tool based on GTK+,
         for being used upon the generic Transport Sampling Protocol.

Copyright (c) 2004 - Euskadi, Yves DUFRENNE.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

-----------------------------------------------------------------------

Project   : TSP
Maintainer: tsp@astrium-space.com
Component : Graphic Tool

-----------------------------------------------------------------------

Purpose   : Graphic Tool based on GTK+ that provide several kinds of
            plot. This tool is to be used with the generic TSP protocol.

File      : 2D plot system.

-----------------------------------------------------------------------
*/


/*
 * System includes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <math.h>


/*
 * For key manipulation.
 */
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>


/*
 * GDISP+ includes.
 */
#include "gdisp_kernel.h"
#include "gdisp_prototypes.h"

#include "gdisp_plot2D.h"

#undef DEBUG_2D

/*
 --------------------------------------------------------------------
                             TODO : Hard coded VALUES 
 --------------------------------------------------------------------
*/
#define GDISP_2D_MARGIN_RATIO   0.1 /* We anticipate the next 10% of futur */
#define GDISP_2D_MAX_TITLE     50   /* Nb char in title  */

#define GDISP_2D_BACK_COLOR _BLACK_

/*
 --------------------------------------------------------------------
                             STATIC ROUTINES
 --------------------------------------------------------------------
*/

static void
gdisp_manageSymbolNameWindow ( Kernel_T *kernel,
			       Plot2D_T *plot,
			       GList    *symbolList);

/*
 * Verbosity management.
 */
static int gdispVerbosity = 1;

#if defined(DEBUG_2D)	
#  define GDISP_TRACE(level,txt) \
	if (level <= gdispVerbosity) fprintf(stderr,txt); 
#else
#  define GDISP_TRACE(level,txt) \
 	/* nothing to be done */
#endif

#define EVENT_METHOD(widget,event) \
        GTK_WIDGET_CLASS(GTK_OBJECT(widget)->klass)->event

/*
 * SAMPLE is in physical unit,
 * PLOT is in logical pixel, (0,0) is bottom left,
 * WIN is in pixel with "good" GTK direction, (0,0) is upper left.
 */

#define X_WIN_TO_PLOT(plot,x) (x)
#define Y_WIN_TO_PLOT(plot,y) ((plot)->p2dAreaHeight - (y))

#define X_PLOT_TO_WIN(plot,x) (x)
#define Y_PLOT_TO_WIN(plot,y) ((plot)->p2dAreaHeight - (y))

#define X_SYMBOL_OFFSET  5
#define Y_SYMBOL_OFFSET 15

#define X_SAMPLE_TO_PLOT(plot,xx) \
        ( ((xx) - (plot)->p2dPtMin.x) * (plot)->p2dPtSlope.x)
#define Y_SAMPLE_TO_PLOT(plot,yy) \
        ( ((yy) - (plot)->p2dPtMin.y) * (plot)->p2dPtSlope.y)

#define X_SAMPLE_TO_WIN(plot,x) (X_PLOT_TO_WIN(plot,X_SAMPLE_TO_PLOT(plot,x)))
#define Y_SAMPLE_TO_WIN(plot,y) (Y_PLOT_TO_WIN(plot,Y_SAMPLE_TO_PLOT(plot,y)))


/*
 * Free memory of all sample point table.
 */
static void
gdisp_freeSampledPointTables( Kernel_T *kernel,
			      Plot2D_T *plot,
			      gboolean  freeAll)
{

  guint    nbCurves  = 0;
  guint    sArrayCpt = 0;
  gpointer pdata     = (gpointer)NULL;

  nbCurves = plot->p2dSampleArray->len;

  for (sArrayCpt=0; sArrayCpt<nbCurves; sArrayCpt++) {

    pdata = g_ptr_array_remove_index_fast(plot->p2dSampleArray,
                                          0 /* always remove first one */);

    dparray_freeSampleArray((DoublePointArray_T*)pdata);

  }

  if (freeAll == TRUE) {

    g_ptr_array_free(plot->p2dSampleArray,FALSE);
    plot->p2dSampleArray = (GPtrArray*)NULL;

  }

}


/*
 * Loop over all elements of the list.
 * Each element is a TSP symbol the 'sReference' parameter of
 * which must be decremented.
 */
static void
gdisp_dereferenceSymbolList ( GList *symbolList )
{

  GList    *symbolItem =    (GList*)NULL;
  Symbol_T *symbol     = (Symbol_T*)NULL;

  /*
   * Loop over all elements of the list.
   * Do not forget to decrement the 'sReference' of each symbol.
   */
  if (symbolList != (GList*)NULL) {

    symbolItem = g_list_first(symbolList);

    while (symbolItem != (GList*)NULL) {

      symbol = (Symbol_T*)symbolItem->data;

      symbol->sReference--;

      symbolItem = g_list_next(symbolItem);

    }

    g_list_free(symbolList);

  }

}


/*
 * Are the drop coordinates inside the X zone ?
 * 'x' and 'y' are in plot coordinates.
 */
static gboolean
gdisp_isInsideXZone (Plot2D_T *plot,
		     guint     x,
		     guint     y)
{

  /*
   * Y
   * |       Y zone
   * |
   * | /--------------------
   * |/        X zone
   * +-----------------------> X
   *
   */

  guint coefficient = 2; /* 50 % */

  /*
   * Apply filter.
   */
  if (x < plot->p2dAreaWidth / coefficient) {

    if (y < (plot->p2dAreaHeight * x / plot->p2dAreaWidth)) {

      return TRUE;

    }

  }
  else {

    if (y < plot->p2dAreaHeight / coefficient) {

      return TRUE;

    }

  }

  return FALSE;

}


/*
 * Draw the symbol list.
 */
static void
gdisp_drawSymbolName ( Kernel_T *kernel,
		       Plot2D_T *plot,
		       gboolean  xAxis)
{

  GList    *symbolItem   =    (GList*)NULL;
  Symbol_T *symbol       = (Symbol_T*)NULL;
  guint     xPosition    =               0;
  guint     yPosition    =               0;
  gint      lBearing     =               0;
  gint      rBearing     =               0;
  gint      width        =               0;
  gint      ascent       =               0;
  gint      descent      =               0;
  gint      windowX      =               0;
  gint      windowY      =               0;
  gint      windowWidth  =               0;
  gint      windowHeight =               0;
  gint      windowDepth  =               0;


  /*
   * Clear window content.
   */
  gdk_gc_set_foreground(plot->p2dGContext,
			&kernel->colors[_GREY_]);

  gdk_window_get_geometry(xAxis == TRUE ?
			  plot->p2dXSymbolWindow : plot->p2dYSymbolWindow,
			  &windowX,
			  &windowY,
			  &windowWidth,
			  &windowHeight,
			  &windowDepth);

  gdk_draw_rectangle(xAxis == TRUE ?
		     plot->p2dXSymbolWindow : plot->p2dYSymbolWindow,
		     plot->p2dGContext,
		     TRUE, /* rectangle is filled */
		     0,
		     0,
		     windowWidth,
		     windowHeight);

  /*
   * TSP Symbol list attached to the X and Y axis.
   */
  if (xAxis == TRUE) {

    xPosition = X_SYMBOL_OFFSET;
    yPosition = Y_SYMBOL_OFFSET;

    symbol = (Symbol_T*)plot->p2dXSymbolList->data;

    gdk_gc_set_foreground(plot->p2dGContext,
			  &kernel->colors[_BLACK_]);

    gdk_draw_string(plot->p2dXSymbolWindow,
		    plot->p2dFont,
		    plot->p2dGContext,
		    xPosition,
		    yPosition,
		    symbol->sInfo.name);

    /*
     * White rectangle around window.
     */
    gdk_window_get_geometry(plot->p2dXSymbolWindow,
			    &windowX,
			    &windowY,
			    &windowWidth,
			    &windowHeight,
			    &windowDepth);

    gdk_gc_set_foreground(plot->p2dGContext,
			  &kernel->colors[_WHITE_]);

    gdk_draw_rectangle(plot->p2dXSymbolWindow,
		       plot->p2dGContext,
		       FALSE, /* rectangle is not filled */
		       0,
		       0,
		       windowWidth  - 1,
		       windowHeight - 1);

  }
  else /* Y axis */ {

    xPosition = X_SYMBOL_OFFSET;
    yPosition = Y_SYMBOL_OFFSET;

    gdk_gc_set_foreground(plot->p2dGContext,
			  &kernel->colors[_BLACK_]);

    symbolItem = g_list_first(plot->p2dYSymbolList);
    while (symbolItem != (GList*)NULL) {

      symbol = (Symbol_T*)symbolItem->data;

      gdk_draw_string(plot->p2dYSymbolWindow,
		      plot->p2dFont,
		      plot->p2dGContext,
		      xPosition,
		      yPosition,
		      symbol->sInfo.name);

      /*
       * Underline selected symbol.
       */
      if (symbolItem == plot->p2dSelectedSymbol) {

	gdk_string_extents(plot->p2dFont,
			   symbol->sInfo.name,
			   &lBearing,
			   &rBearing,
			   &width,
			   &ascent,
			   &descent);

	gdk_draw_line(plot->p2dYSymbolWindow,
		      plot->p2dGContext,
		      xPosition,                /* x1 */
		      yPosition + descent,      /* y1 */
		      xPosition + width,        /* x2 */
		      yPosition + descent);     /* y2 */

      }

      yPosition += Y_SYMBOL_OFFSET;

      symbolItem = g_list_next(symbolItem);

    }

    /*
     * White rectangle around window.
     */
    gdk_gc_set_foreground(plot->p2dGContext,
			  &kernel->colors[_WHITE_]);

    gdk_draw_rectangle(plot->p2dYSymbolWindow,
		       plot->p2dGContext,
		       FALSE, /* rectangle is not filled */
		       0,
		       0,
		       windowWidth  - 1,
		       windowHeight - 1);

  }

}


/*
 * Try to delete one or several symbols on Y axis.
 */
static void
gdisp_deleteSelectedSymbolName ( Kernel_T *kernel,
				 Plot2D_T *plot,
				 gint      xMouse,
				 gint      yMouse,
				 gboolean  isShifted)
{

  DoublePointArray_T *pArray   = (DoublePointArray_T*)NULL;
  guint               position =                         0;

  /*
   * 'button3' removes the selected symbol from the list.
   * 'shift' + 'button3' removes all symbols.
   */
  if (isShifted == TRUE) {

    /*
     * Do not forget to decrement the reference of the Y symbol.
     */
    gdisp_dereferenceSymbolList(plot->p2dXSymbolList);
    gdisp_dereferenceSymbolList(plot->p2dYSymbolList);
    plot->p2dXSymbolList    = (GList*)NULL;
    plot->p2dYSymbolList    = (GList*)NULL;
    plot->p2dSelectedSymbol = (GList*)NULL;

    /*
     * Free data for sampled points.
     */
    gdisp_freeSampledPointTables(kernel,plot,FALSE /* freeAll */);

    /*
     * Reset bounding box.
     */
    plot->p2dPtMin.x = 0;
    plot->p2dPtMin.y = 0;
    plot->p2dPtMax.x = 0;
    plot->p2dPtMax.y = 0;

    /*
     * Destroy all Gdk windows.
     */
    if (plot->p2dXSymbolWindow != (GdkWindow*)NULL)
      gdk_window_destroy(plot->p2dXSymbolWindow);
    if (plot->p2dYSymbolWindow != (GdkWindow*)NULL)
      gdk_window_destroy(plot->p2dYSymbolWindow);

    plot->p2dXSymbolWindow = (GdkWindow*)NULL;
    plot->p2dYSymbolWindow = (GdkWindow*)NULL;

  }
  else {

    if (plot->p2dSelectedSymbol != (GList*)NULL) {

      position = g_list_position(plot->p2dYSymbolList,
				 plot->p2dSelectedSymbol);

      plot->p2dYSymbolList = g_list_remove_link(plot->p2dYSymbolList,
						plot->p2dSelectedSymbol);

      /*
       * Do not forget to decrement the reference of the Y symbol.
       */
      gdisp_dereferenceSymbolList(plot->p2dSelectedSymbol);
      plot->p2dSelectedSymbol = (GList*)NULL;

      /*
       * Free corresponding memory.
       */
      pArray = g_ptr_array_remove_index(plot->p2dSampleArray,
					position);
      dparray_freeSampleArray(pArray);

      /*
       * Update Y symbol name window.
       */
      if (g_list_length(plot->p2dYSymbolList) == 0) {

	if (plot->p2dYSymbolWindow != (GdkWindow*)NULL)
	  gdk_window_destroy(plot->p2dYSymbolWindow);
	plot->p2dYSymbolWindow = (GdkWindow*)NULL;

      }
      else {

	gdisp_manageSymbolNameWindow(kernel,
				     plot,
				     plot->p2dYSymbolList);

      }

      /*
       * FIXME : Duf.
       * We have just removed one symbol.
       * How do we recompute the bounding box (min, max) so that the
       * remaining symbols use the whole graphic area ?
       */

    }

  }

}


/*
 * Draw selected symbol on Y axis.
 */
static void
gdisp_underlineSelectedSymbolName ( Kernel_T *kernel,
				    Plot2D_T *plot,
				    gint      xMouse,
				    gint      yMouse)
{

  GList    *symbolItem =    (GList*)NULL;
  Symbol_T *symbol     = (Symbol_T*)NULL;
  guint     xPosition  =               0;
  guint     yPosition  =               0;
  gint      lBearing   =               0;
  gint      rBearing   =               0;
  gint      width      =               0;
  gint      ascent     =               0;
  gint      descent    =               0;


  /*
   * Return if no symbol available.
   */
  if (plot->p2dYSymbolList == (GList*)NULL) {

    return;

  }


  /*
   * Try to get the symbol the mouse is over.
   */
  xPosition = X_SYMBOL_OFFSET;
  yPosition = Y_SYMBOL_OFFSET;

  symbolItem = g_list_first(plot->p2dYSymbolList);
  while (symbolItem != (GList*)NULL) {

    symbol = (Symbol_T*)symbolItem->data;

    /*
     * Compute surrounding rectangle dimensions.
     */
    gdk_string_extents(plot->p2dFont,
		       symbol->sInfo.name,
		       &lBearing,
		       &rBearing,
		       &width,
		       &ascent,
		       &descent);

    if (xPosition          < xMouse && xMouse < xPosition + width &&
	yPosition - ascent < yMouse && yMouse < yPosition + descent  ) {

      if (symbolItem != plot->p2dSelectedSymbol) {

	/*
	 * This symbol becomes the new selected one, refresh graphic area.
	 */
	plot->p2dSelectedSymbol = symbolItem;

	gdisp_drawSymbolName (kernel,
			      plot,
			      FALSE /* TRUE for xAxis, FALSE otherwise */);

      }

      /*
       * Stop here investigations, because selected symbol is found.
       */
      return;

    } /* Inside rectangle */

    yPosition += Y_SYMBOL_OFFSET;

    symbolItem = g_list_next(symbolItem);

  }

  /*
   * No symbol is selected, because the mouse is not inside any rectangle.
   * Refresh graphic area.
   */
  if (plot->p2dSelectedSymbol != (GList*)NULL) {

    plot->p2dSelectedSymbol = (GList*)NULL;

    gdisp_drawSymbolName (kernel,
			  plot,
			  FALSE /* TRUE for xAxis, FALSE otherwise */);

  }

}


/*
 * Manage symbol name window.
 */
static GdkFilterReturn
gdisp_symbolNameWindowEvent ( GdkXEvent *xevent,
			      GdkEvent  *event,
			      gpointer   data )
{

  Kernel_T        *kernel       =   (Kernel_T*)data;
  Plot2D_T        *plot         =   (Plot2D_T*)NULL;
  GdkWindow       *nameWindow   =  (GdkWindow*)NULL;
  GdkFilterReturn  returnFilter = GDK_FILTER_REMOVE;
  XEvent          *xEvent       =     (XEvent*)NULL;
  GdkRectangle     exposeArea;

  GDISP_TRACE(3,"Symbol name window management.\n");

  /*
   * Manage all cases.
   */
  gdk_window_get_user_data(event->any.window,
			   (gpointer)&plot);


  /*
   * CAUTION !!!! CAUTION !!!! CAUTION !!!! CAUTION !!!! CAUTION !!!!
   *
   * Due to a strange way of processing events within Gdk, the second
   * parameter "event" (a GdkEvent pointer) must be used carefully.
   * The only field that is correct is the "window" field. All other
   * fields must not be used, because they are UNINITIALIZED !!!
   * All other information must be taken from the first argument "xevent"
   * that must be casted to a X11 native "XEvent" structure.
   */
  nameWindow = event->any.window; /* Symbol window */
  xEvent     = (XEvent*)xevent;

  switch (xEvent->type) {

  case Expose :

    GDISP_TRACE(3,"Expose on symbol name window.\n");

    exposeArea.x      = xEvent->xexpose.x;
    exposeArea.y      = xEvent->xexpose.y;
    exposeArea.width  = xEvent->xexpose.width;
    exposeArea.height = xEvent->xexpose.height;

    gdk_gc_set_clip_rectangle(plot->p2dGContext,
			      &exposeArea);

    gdisp_drawSymbolName(kernel,
			 plot,
			 nameWindow == plot->p2dXSymbolWindow ? TRUE : FALSE);

    gdk_gc_set_clip_rectangle(plot->p2dGContext,
			      (GdkRectangle*)NULL);

    break;

  case MotionNotify :

    GDISP_TRACE(3,"MotionNotify on symbol name window.\n");

    if (plot->p2dYSymbolWindow     == nameWindow &&
	plot->p2dSignalsAreBlocked == FALSE         ) {

      gdisp_underlineSelectedSymbolName(kernel,
					plot,
					xEvent->xmotion.x,
					xEvent->xmotion.y);

    }

    break;

  case ButtonPress :

    GDISP_TRACE(3,"ButtonPress on symbol name window.\n");

    if (plot->p2dYSymbolWindow     == nameWindow &&
	plot->p2dSignalsAreBlocked == FALSE         ) {

      if (xEvent->xbutton.button == Button3) {

	gdisp_deleteSelectedSymbolName(kernel,
				       plot,
				       xEvent->xbutton.x,
				       xEvent->xbutton.y,
				       (xEvent->xbutton.state & ShiftMask) ?
				       TRUE : FALSE /* isShifted */);

      }

    }

    break;

  default :
    break;

  }

  return returnFilter;

}


/*
 * Create or resize the window associated to X and Y symbol names.
 */
static void
gdisp_manageSymbolNameWindow ( Kernel_T *kernel,
			       Plot2D_T *plot,
			       GList    *symbolList)
{

  GdkWindow     **windowPointer  =  (GdkWindow**)NULL;
  GdkVisual      *visual         =   (GdkVisual*)NULL;
  GdkColormap    *colormap       = (GdkColormap*)NULL;
  GList          *symbolItem     =       (GList*)NULL;
  Symbol_T       *symbol         =    (Symbol_T*)NULL;
  gint            windowAttrMask =                  0;
  gint16          windowX        =                  0;
  gint16          windowY        =                  0;
  gint16          windowWidth    =                  0;
  gint16          windowHeight   =                  0;
  gint            lBearing       =                  0;
  gint            rBearing       =                  0;
  gint            width          =                  0;
  gint            ascent         =                  0;
  gint            descent        =                  0;
  GdkWindowAttr   windowAttr;


  GDISP_TRACE(3,"Creating or resizing symbol name window.\n");


  /*
   * Which axis ?
   */
  if (symbolList == plot->p2dYSymbolList) {

    if (g_list_length(symbolList) == 0)
      return;

    windowPointer = &plot->p2dYSymbolWindow;

    windowX       = X_SYMBOL_OFFSET * 3;
    windowY       = Y_SYMBOL_OFFSET;

    windowWidth   = 0; /* by default */
    windowHeight  = g_list_length(symbolList) * Y_SYMBOL_OFFSET;
    windowHeight += Y_SYMBOL_OFFSET / 2;

    symbolItem = g_list_first(symbolList);
    while (symbolItem != (GList*)NULL) {

      symbol = (Symbol_T*)symbolItem->data;

      gdk_string_extents(plot->p2dFont,
			 symbol->sInfo.name,
			 &lBearing,
			 &rBearing,
			 &width,
			 &ascent,
			 &descent);

      windowWidth = MAX(windowWidth,width);

      symbolItem = g_list_next(symbolItem);

    }

    windowWidth += 2 * X_SYMBOL_OFFSET;

  }
  else /* X */ {

    if (g_list_length(symbolList) == 0)
      return;

    windowPointer = &plot->p2dXSymbolWindow;

    windowX      = plot->p2dAreaWidth  - 3 * X_SYMBOL_OFFSET;
    windowY      = plot->p2dAreaHeight - 2 * Y_SYMBOL_OFFSET;
    windowWidth  = 0; /* by default */
    windowHeight = Y_SYMBOL_OFFSET + Y_SYMBOL_OFFSET / 2;

    symbolItem = g_list_first(symbolList);
    symbol = (Symbol_T*)symbolItem->data;

    gdk_string_extents(plot->p2dFont,
		       symbol->sInfo.name,
		       &lBearing,
		       &rBearing,
		       &width,
		       &ascent,
		       &descent);

    windowWidth  = width + 2 * X_SYMBOL_OFFSET;
    windowX     -= windowWidth;

  }


  /*
   * Get back information from parent window.
   */
  if (*windowPointer == (GdkWindow*)NULL) {

    visual   = gdk_window_get_visual  (plot->p2dArea->window);
    colormap = gdk_window_get_colormap(plot->p2dArea->window);

    memset(&windowAttr,0,sizeof(GdkWindowAttr));

    windowAttr.event_mask  = GDK_EXPOSURE_MASK       |
                             GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK;
    windowAttr.x           = windowX;
    windowAttr.y           = windowY;
    windowAttr.width       = windowWidth;
    windowAttr.height      = windowHeight;
    windowAttr.window_type = GDK_WINDOW_CHILD;
    windowAttr.wclass      = GDK_INPUT_OUTPUT;
    windowAttr.visual      = visual;
    windowAttr.colormap    = colormap;
    windowAttrMask         = GDK_WA_X        | GDK_WA_Y      |
                             GDK_WA_COLORMAP | GDK_WA_VISUAL;

    *windowPointer = gdk_window_new(plot->p2dArea->window,
				    &windowAttr,
				    windowAttrMask);

    assert(*windowPointer);

    gdk_window_set_user_data(*windowPointer,
			     (gpointer)plot);

    gdk_window_add_filter(*windowPointer,
			  gdisp_symbolNameWindowEvent,
			  (gpointer)kernel);

    gdk_window_show(*windowPointer);

  }
  else /* resize */ {

    gdk_window_move_resize(*windowPointer,
			   windowX,
			   windowY,
			   windowWidth,
			   windowHeight);

  }

}


/*
 * Copy back buffer into graphic area (front buffer).
 */
static void
gdisp_plot2DSwapBuffers (Kernel_T       *kernel,
			 Plot2D_T       *plot,
			 KindOfRedraw_T  drawType)
{

  GDISP_TRACE(3,"Swaping front and back buffers\n");

  /*
   * Take care of draw type.
   */
  if (drawType == GD_2D_ADD_NEW_SAMPLES) {

    /*
     * Should only repaint the area of buffer
     * that has been updated => OverPowerfull. Thanks Duf.
     */
    gdk_draw_pixmap(plot->p2dArea->window,
		    plot->p2dGContext,
		    plot->p2dBackBuffer,
		    plot->p2dPtRedrawMin.x,
		    plot->p2dPtRedrawMin.y,
		    plot->p2dPtRedrawMin.x,
		    plot->p2dPtRedrawMin.y,
		    plot->p2dPtRedrawMax.x - plot->p2dPtRedrawMin.x + 1,
		    plot->p2dPtRedrawMax.y - plot->p2dPtRedrawMin.y + 1);

  }
  else {

    /*
     * Copy back buffer into front buffer.
     */
    gdk_draw_pixmap(plot->p2dArea->window,
		    plot->p2dGContext,
		    plot->p2dBackBuffer,
		    0,
		    0,
		    0,
		    0,
		    plot->p2dArea->allocation.width,
		    plot->p2dArea->allocation.height);

  }

}


/*
 * Redraw back buffer content. 2 parts :
 *
 * - gdisp_plot2DDrawBackBufferBackground : 
 *	Only for background earsing, titles and Axis.
 *	For X_SCROLL, use 'gdk_draw_pixmap' for translating 90% of bitmap.
 *
 * - gdisp_plot2DDrawBackBufferCurves
 *	Plot the lines 
 *
 */
static void
gdisp_plot2DDrawBackBufferBackground (Kernel_T       *kernel,
				      Plot2D_T       *plot,
				      KindOfRedraw_T  drawType)
{

#undef WANT_TITLES

#if defined(WANT_TITLES)
  gint      lBearing   =               0;
  gint      rBearing   =               0;
  gint      width      =               0;
  gint      ascent     =               0;
  gint      descent    =               0;
  gchar     title   [GDISP_2D_MAX_TITLE];
  gchar     subTitle[GDISP_2D_MAX_TITLE];
#endif


  /*
   * We never enter this routine with 'drawType' GD_2D_ADD_NEW_SAMPLES.
   */

  /*
   * Take care of the type of the drawing process.
   */
  if (drawType == GD_2D_FULL_REDRAW) {

    GDISP_TRACE(2,"Drawing into back buffer -> background : FULL_REDRAW\n");

    gdk_gc_set_foreground(plot->p2dGContext,
			  &kernel->colors[GDISP_2D_BACK_COLOR]);

    gdk_draw_rectangle(plot->p2dBackBuffer,
		       plot->p2dGContext,
		       TRUE, /* rectangle is filled */
		       0,
		       0,
		       plot->p2dArea->allocation.width,
		       plot->p2dArea->allocation.height);

  } /* GD_2D_FULL_REDRAW */


  /*
   * We need to scroll the X axis.
   */
  else if (drawType == GD_2D_SCROLL_X_AXIS) {

    GDISP_TRACE(2,"Drawing into back buffer -> background : SCROLL_X\n");

    /*
     * Scroll the 90% of the backbuffer to the left.
     */
    gdk_draw_pixmap(plot->p2dBackBuffer, /* destination */
		    plot->p2dGContext,
		    plot->p2dBackBuffer, /* source      */
		    (gint)ceil(plot->p2dArea->allocation.width *
                                                     GDISP_2D_MARGIN_RATIO),
		    0,
		    0,
		    0,
		    (gint)ceil(plot->p2dArea->allocation.width *
                                               (1 - GDISP_2D_MARGIN_RATIO)), 
		    plot->p2dArea->allocation.height);

    /*
     * Erase the 10% left on the right.
     */
    gdk_gc_set_foreground(plot->p2dGContext,
			  &kernel->colors[GDISP_2D_BACK_COLOR]);

    gdk_draw_rectangle(plot->p2dBackBuffer,
		       plot->p2dGContext,
		       TRUE, /* rectangle is filled */
		       (gint)floor(plot->p2dArea->allocation.width *
                                                (1 - GDISP_2D_MARGIN_RATIO)), 
		       0,
		       (gint)ceil(plot->p2dArea->allocation.width *
                                                      GDISP_2D_MARGIN_RATIO), 
		       plot->p2dArea->allocation.height);

  }
  else /* GD_2D_ADD_NEW_SAMPLES */ {

    GDISP_TRACE(2,
		"Drawing into back buffer -> background : ADD_NEW_SAMPLES\n");

    return;

  }


  /*
   * Title
   */
#if defined(WANT_TITLES)

  gdk_gc_set_foreground(plot->p2dGContext,
			&kernel->colors[_WHITE_]);

  snprintf(title,
	   GDISP_2D_MAX_TITLE,
	   "Y=Sin(t) : {%g,%g}", 
	   plot->p2dPtLast.x,
	   plot->p2dPtLast.y);

  gdk_string_extents(plot->p2dFont,
		     title,
		     &lBearing,
		     &rBearing,
		     &width,
		     &ascent,
		     &descent);

  gdk_draw_string(plot->p2dBackBuffer,
		  plot->p2dFont,
		  plot->p2dGContext,
		  (plot->p2dArea->allocation.width - width) / 2,
		  Y_SYMBOL_OFFSET,
		  title);
      
  snprintf(subTitle,
	   GDISP_2D_MAX_TITLE,
	   "Bornes X={%g<%g} Y={%g<%g}", 
	   plot->p2dPtMin.x,
	   plot->p2dPtMax.x, 
	   plot->p2dPtMin.y,
	   plot->p2dPtMax.y);
      
  gdk_string_extents(plot->p2dFont,
		     subTitle,
		     &lBearing,
		     &rBearing,
		     &width,
		     &ascent,
		     &descent);

  gdk_draw_string(plot->p2dBackBuffer,
		  plot->p2dFont,
		  plot->p2dGContext,
		  (plot->p2dArea->allocation.width - width) / 2,
		  Y_SYMBOL_OFFSET * 2,
		  subTitle);

#endif
      

  /*
   * X & Y axis lines around the graphic area.
   */
  gdk_gc_set_foreground(plot->p2dGContext,
			&kernel->colors[_GREY_]);


  /* Axis on X=0 */
  gdk_draw_line(plot->p2dBackBuffer,
		plot->p2dGContext,
		X_SAMPLE_TO_WIN(plot,0.0),                /* x1 */
		Y_SAMPLE_TO_WIN(plot,plot->p2dPtMin.y),   /* y1 */
		X_SAMPLE_TO_WIN(plot,0.0),                /* x2 */
		Y_SAMPLE_TO_WIN(plot,plot->p2dPtMax.y) ); /* y2 */
      
  /* Axis on Y=0 */
  gdk_draw_line(plot->p2dBackBuffer,
		plot->p2dGContext,
		X_SAMPLE_TO_WIN(plot,plot->p2dPtMin.x),   /* x1 */
		Y_SAMPLE_TO_WIN(plot,0.0),                /* y1 */
		X_SAMPLE_TO_WIN(plot,plot->p2dPtMax.x),   /* x2 */
		Y_SAMPLE_TO_WIN(plot,0.0) );              /* y2 */


  /*
   * Black lines around the graphic area.
   */
  gdk_gc_set_foreground(plot->p2dGContext,
			&kernel->colors[_BLACK_]);

  gdk_draw_line(plot->p2dBackBuffer,
		plot->p2dGContext,
		0,                                   /* x1 */
		0,                                   /* y1 */
		plot->p2dArea->allocation.width - 1, /* x2 */
		0);                                  /* y2 */

  gdk_draw_line(plot->p2dBackBuffer,
		plot->p2dGContext,
		plot->p2dArea->allocation.width  - 1,  /* x1 */
		0,                                     /* y1 */
		plot->p2dArea->allocation.width  - 1,  /* x2 */
		plot->p2dArea->allocation.height - 1); /* y2 */


  /*
   * Axis
   * FIXME : This piece of code must be put elsewhere,
   * but NOT here while drawing into the back-buffer.
   */
  if (plot->p2dPtMin.x != plot->p2dPtMax.x) {

    gtk_ruler_set_range(GTK_RULER(plot->p2dHRuler),
			plot->p2dPtMin.x,
			plot->p2dPtMax.x,
			plot->p2dPtMin.x,
			GDISP_WIN_T_DURATION);

  }

  if (plot->p2dPtMin.y != plot->p2dPtMax.y) {

    gtk_ruler_set_range(GTK_RULER(plot->p2dVRuler),
			-plot->p2dPtMax.y,
			-plot->p2dPtMin.y,
			0,
			plot->p2dPtMax.y - plot->p2dPtMin.y);

  }
      
}


static void
gdisp_plot2DDrawBackBufferCurves (Kernel_T       *kernel,
				  Plot2D_T       *plot,
				  KindOfRedraw_T  drawType)
{

  guint               nbCurves   =                         0;
  guint               cptCurve   =                         0;
  guint               cptPoint   =                         0;
  DoublePointArray_T *pArray     = (DoublePointArray_T*)NULL;
  DoublePoint_T      *pSample    =      (DoublePoint_T*)NULL;
  guint               startIndex =                         0;
  guint               lastIndex  =                         0;
  guint               nbPoints   =                         0;
  ShortPoint_T        lastPixel;
  ShortPoint_T        currentPixel;

  GDISP_TRACE(3,"Drawing into back buffer -> curves\n");

  /*
   * Compute bounding box to be redrawn.
   * Inversion for Y minimum and maximum, cause (0,0) is (up,left).
   */
  plot->p2dPtRedrawMin.x = X_SAMPLE_TO_WIN(plot,plot->p2dPtMax.x);
  plot->p2dPtRedrawMax.x = X_SAMPLE_TO_WIN(plot,plot->p2dPtMin.x);
  plot->p2dPtRedrawMin.y = Y_SAMPLE_TO_WIN(plot,plot->p2dPtMin.y);
  plot->p2dPtRedrawMax.y = Y_SAMPLE_TO_WIN(plot,plot->p2dPtMax.y);


  /*
   * Plot the sample lines
   */
  nbCurves = plot->p2dSampleArray->len;
  for (cptCurve=0; cptCurve<nbCurves; cptCurve++) {

    pArray = plot->p2dSampleArray->pdata[cptCurve]; 

    if (drawType == GD_2D_FULL_REDRAW) {

      /*
       * Plot all for GDISP_2D_FULL_REDRAW & GDISP_2D_SCROLL_X_AXIS.
       */
      startIndex = dparray_getFirstIndex(pArray);
      nbPoints   = dparray_getNbSamples (pArray);

    }
    else {

      /*
       * GDISP_2D_ADD_NEW_SAMPLES => restart from where we stopped earlier.
       */
      startIndex = dparray_getMarkerIndex(pArray);
      nbPoints = dparray_getLeftSamplesFromPos(pArray,
					       startIndex);

    }
    lastIndex = startIndex;
      

    /*
     * TODO: A color per draw ?
     */
    gdk_gc_set_foreground(plot->p2dGContext,
			  &kernel->colors[_YELLOW_ + cptCurve * 7]);
      
    for (cptPoint=startIndex; cptPoint<startIndex+nbPoints; cptPoint++) {

      lastIndex = cptPoint;

      pSample        = DP_ARRAY_GET_SAMPLE_PTR(pArray,cptPoint);
      currentPixel.x = X_SAMPLE_TO_WIN(plot,pSample->x);
      currentPixel.y = Y_SAMPLE_TO_WIN(plot,pSample->y);

      /*
       * Store the area of backbuffer
       * that we'll need to redraw on front buffer.
       */
      if (currentPixel.x < plot->p2dPtRedrawMin.x) {
	plot->p2dPtRedrawMin.x = currentPixel.x;
      }
      if (currentPixel.y < plot->p2dPtRedrawMin.y) {
	plot->p2dPtRedrawMin.y = currentPixel.y;
      }
      if (currentPixel.x > plot->p2dPtRedrawMax.x) {
	plot->p2dPtRedrawMax.x = currentPixel.x;
      }
      if (currentPixel.y > plot->p2dPtRedrawMax.y) {
	plot->p2dPtRedrawMax.y = currentPixel.y;
      }

      if ((cptPoint > startIndex ) &&
	  (lastPixel.x != currentPixel.x || lastPixel.y != currentPixel.y)) {

	gdk_draw_line(plot->p2dBackBuffer,
		      plot->p2dGContext,
		      lastPixel.x,
		      lastPixel.y,
		      currentPixel.x,
		      currentPixel.y);

      }

      lastPixel = currentPixel;

    } /* end of loop for drawing on curve */

    /*
     * Mark the last plotted position, for next drawBackBuffer optimisation.
     */
    if (nbPoints > 0) {

      /*
       * Must Redtart from the previous last point
       */
      dparray_setMarkerIndex(pArray,
			     lastIndex -1);

    }

  } /* end of loop on every curves */

}


/*
 * Redraw back buffer content.
 */
static void
gdisp_plot2DDrawBackBuffer (Kernel_T       *kernel,
			    Plot2D_T       *plot,
			    KindOfRedraw_T  drawType)
{

  GDISP_TRACE(3,"Drawing into back buffer\n");

  /*
   * Do not redraw background when simply adding new samples.
   */
  if (drawType != GD_2D_ADD_NEW_SAMPLES) {

    plot->p2dPtSlope.x =
      plot->p2dAreaWidth  / (plot->p2dPtMax.x - plot->p2dPtMin.x);
    plot->p2dPtSlope.y =
      plot->p2dAreaHeight / (plot->p2dPtMax.y - plot->p2dPtMin.y);

    gdisp_plot2DDrawBackBufferBackground(kernel,
					 plot,
					 drawType);
  }

  /*
   * Always plot the new lines for every curve.
   */
  gdisp_plot2DDrawBackBufferCurves(kernel,
				   plot,
				   drawType);

}


/*
 * Treat 'expose' X event.
 * What shall I do when the graphic area has to be refreshed ?
 */
static gboolean
gdisp_plot2DExpose (GtkWidget       *area,
		    GdkEventExpose  *event,
		    gpointer         data)
{

  Kernel_T *kernel = (Kernel_T*)data;
  Plot2D_T *plot   = (Plot2D_T*)NULL;

  GDISP_TRACE(3,"Expose event\n");

  /*
   * Graphic area has now to be repainted.
   */
  plot = (Plot2D_T*)gtk_object_get_data(GTK_OBJECT(area),
					"plotPointer");

  gdk_gc_set_clip_rectangle(plot->p2dGContext,
			    &event->area);

  gdisp_plot2DSwapBuffers(kernel,
			  plot,
			  GD_2D_FULL_REDRAW);

  gdk_gc_set_clip_rectangle(plot->p2dGContext,
			    (GdkRectangle*)NULL);

  return TRUE;

}


/*
 * Treat 'configure' X event.
 * What shall I do when the graphic area has to be re-configured ?
 */
static gboolean
gdisp_plot2DConfigure (GtkWidget         *area,
		       GdkEventConfigure *event,
		       gpointer           data)
{

  Kernel_T *kernel = (Kernel_T*)data;
  Plot2D_T *plot   = (Plot2D_T*)NULL;

  GDISP_TRACE(1,"Configure event\n");

  /*
   * Get plot private data.
   */
  plot = (Plot2D_T*)gtk_object_get_data(GTK_OBJECT(area),
					"plotPointer");


  /*
   * Don't know why, but GTK generates a 'configure' event when
   * showing widgets before returning into the main X loop. Strange.
   */
  if (event->width == 1 && event->height == 1) {

    GDISP_TRACE(3,"Configure event returns suddendly.\n");

    return TRUE;

  }


  /*
   * Graphic area has now to be resized
   */
  plot->p2dAreaWidth  = event->width;
  plot->p2dAreaHeight = event->height;


  /*
   * Some difficulties to create the back buffer in the 'create' procedure,
   * because some window information are not yet available.
   * So create it here.
   */
  if (plot->p2dBackBuffer != (GdkPixmap*)NULL) {

    gdk_pixmap_unref(plot->p2dBackBuffer);

  }


  /*
   * Create a pixmap for double buffering.
   * Deduce depth from graphic area window.
   */
  GDISP_TRACE(3,"Creating back buffer.\n");

  plot->p2dBackBuffer = gdk_pixmap_new(plot->p2dArea->window,
				       plot->p2dAreaWidth,
				       plot->p2dAreaHeight,
				       -1 /* same as window */);

  assert(plot->p2dBackBuffer);


  /*
   * The configure operation is done after a resize request.
   * So refresh back buffer.
   */
  gdisp_plot2DDrawBackBuffer(kernel,
			     plot,
			     GD_2D_FULL_REDRAW);


  /*
   * Take care of X symbol name window.
   * No need to move/resize Y symbol list.
   */
  gdisp_manageSymbolNameWindow(kernel,
			       plot,
			       plot->p2dXSymbolList);

  return TRUE;

}


/*
 * Treat 'enter-notify' X event.
 * What shall I do when the mouse enters the graphic area ?
 */
static gboolean
gdisp_plot2DEnterNotify (GtkWidget        *area,
			 GdkEventCrossing *event,
			 gpointer          data)
{

  Plot2D_T *plot = (Plot2D_T*)NULL;

  GDISP_TRACE(2,"Enter Notify Event\n");

  /*
   * Graphic area has now the focus.
   */
  plot = (Plot2D_T*)gtk_object_get_data(GTK_OBJECT(area),
					"plotPointer");

  plot->p2dHasFocus = TRUE;

  return TRUE;

}


/*
 * Treat 'leave-notify' X event.
 * What shall I do when the mouse leaves the graphic area ?
 */
static gboolean
gdisp_plot2DLeaveNotify (GtkWidget        *area,
			 GdkEventCrossing *event,
			 gpointer          data)
{

  Plot2D_T *plot = (Plot2D_T*)NULL;

  GDISP_TRACE(2,"Leave Notify Event.\n");

  /*
   * Graphic area has lost the focus.
   */
  plot = (Plot2D_T*)gtk_object_get_data(GTK_OBJECT(area),
					"plotPointer");

  plot->p2dHasFocus = FALSE;

  return TRUE;

}


/*
 * Treat 'button-press' X event.
 * What shall I do when the user press a button on the graphic area ?
 */
static gboolean
gdisp_plot2DButtonPress (GtkWidget      *area,
			 GdkEventButton *event,
			 gpointer        data)
{

  Kernel_T *kernel = (Kernel_T*)data;
  Plot2D_T *plot   = (Plot2D_T*)NULL;

  GDISP_TRACE(3,"Button press event.\n");

  /*
   * Graphic area has lost the focus.
   */
  plot = (Plot2D_T*)gtk_object_get_data(GTK_OBJECT(area),
					"plotPointer");

  /*
   * Nothing to be done if button identity is not 3.
   */
  if (event->button != 3) {

    return TRUE;

  }

  /*
   * 'button3' removes the selected symbol from the list.
   * 'shift' + 'button3' removes all symbols.
   */
  if (event->state & GDK_SHIFT_MASK) {

  }
  else {

  }

  /*
   * Refresh graphic area.
   */
  gdisp_plot2DDrawBackBuffer(kernel,
			     plot,
			     GD_2D_FULL_REDRAW);

  gdisp_plot2DSwapBuffers   (kernel,
			     plot,
			     GD_2D_FULL_REDRAW);

  return TRUE;

}


/*
 * Treat 'motion-notify' X event.
 * What shall I do when the mouse moves over the graphic area ?
 */
static gboolean
gdisp_plot2DMotionNotify (GtkWidget      *area,
			  GdkEventMotion *event,
			  gpointer        data)
{

  Kernel_T *kernel     = (Kernel_T*)data;
  Plot2D_T *plot       = (Plot2D_T*)NULL;

  guint     xPosition  =               0;
  guint     yPosition  =               0;

  GDISP_TRACE(3,"Motion notify event.\n");


  /*
   * Graphic area has lost the focus.
   */
  plot = (Plot2D_T*)gtk_object_get_data(GTK_OBJECT(area),
					"plotPointer");


  /*
   * Take care of hints from X Server.
   */
  if (event->is_hint) {
  
    gdk_window_get_pointer(event->window,
			   &xPosition,
			   &yPosition,
			   &event->state);

    event->x = (gdouble)xPosition;
    event->y = (gdouble)yPosition;

  }

  gdisp_plot2DDrawBackBuffer(kernel,
			     plot,
			     GD_2D_FULL_REDRAW);

  gdisp_plot2DSwapBuffers   (kernel,
			     plot,
			     GD_2D_FULL_REDRAW);

  return TRUE;

}


/*
 * Create a '2D plot' by providing an opaque structure to the
 * caller. This opaque structure will be given as an argument to all
 * plot function. These functions remain generic.
 */
static void*
gdisp_createPlot2D (Kernel_T *kernel)
{

#define FROZEN_RULER

  Plot2D_T *plot     = (Plot2D_T*)NULL;
  guint     signalID = 0;

  /*
   * Dynamic allocation.
   */
  plot = g_malloc0(sizeof(Plot2D_T));
  assert(plot);

  /*
   * Few initialisations.
   */
  plot->p2dHasFocus  = FALSE;
  plot->p2dIsWorking = FALSE;
  plot->p2dType      = GD_PLOT_2D;
  plot->p2dSubType   = GD_2D_F2T;

  /*
   * Create a table : dimension 2 x 2, homogeneous.
   * Insert a graphic area and two rulers in it.
   */
  plot->p2dTable = gtk_table_new(2,
				 2,
				 FALSE);

  plot->p2dArea = gtk_drawing_area_new();

  plot->p2dSignalsAreBlocked = FALSE;
  plot->p2dSignalIdentities  = g_array_new(FALSE /* zero terminated */,
					   TRUE  /* clear           */,
					   sizeof(guint));

  gtk_table_attach(GTK_TABLE(plot->p2dTable),
		   plot->p2dArea,
		   1,
		   2,
		   0,
		   1,
		   GTK_EXPAND | GTK_FILL,
		   GTK_EXPAND | GTK_FILL,
		   0,
		   0 );

#if defined(GD_AREA_WANT_EXTRA_EVENTS)

  gtk_widget_set_events(plot->p2dArea,
			GDK_POINTER_MOTION_MASK      |
			GDK_POINTER_MOTION_HINT_MASK |
			GDK_ENTER_NOTIFY_MASK        |
			GDK_LEAVE_NOTIFY_MASK        |
			GDK_BUTTON_PRESS_MASK          );

#endif

  gtk_signal_connect(GTK_OBJECT(plot->p2dArea),
		     "expose_event",
		     (GtkSignalFunc)gdisp_plot2DExpose,
		     (gpointer)kernel);

  gtk_signal_connect (GTK_OBJECT(plot->p2dArea),
		      "configure_event",
		      (GtkSignalFunc)gdisp_plot2DConfigure,
		      (gpointer)kernel); 

  /*
   * Store few signals in order to block them afterwards.
   */
  signalID = gtk_signal_connect(GTK_OBJECT(plot->p2dArea),
				"enter_notify_event",
				(GtkSignalFunc)gdisp_plot2DEnterNotify,
				(gpointer)kernel);
  g_array_append_val(plot->p2dSignalIdentities,signalID);

  signalID = gtk_signal_connect(GTK_OBJECT(plot->p2dArea),
				"leave_notify_event",
				(GtkSignalFunc)gdisp_plot2DLeaveNotify,
				(gpointer)kernel);
  g_array_append_val(plot->p2dSignalIdentities,signalID);

  signalID = gtk_signal_connect(GTK_OBJECT(plot->p2dArea),
				"button_press_event",
				(GtkSignalFunc)gdisp_plot2DButtonPress,
				(gpointer)kernel);
  g_array_append_val(plot->p2dSignalIdentities,signalID);

  signalID = gtk_signal_connect(GTK_OBJECT(plot->p2dArea),
				"motion_notify_event",
				(GtkSignalFunc)gdisp_plot2DMotionNotify,
				(gpointer)kernel);
  g_array_append_val(plot->p2dSignalIdentities,signalID);

  gtk_object_set_data(GTK_OBJECT(plot->p2dArea),
		      "plotPointer",
		      (gpointer)plot);

  /*
   * The horizontal ruler goes on bottom.
   * As the mouse moves across the drawing area, a 'motion_notify_event'
   * is passed to the appropriate event handler for the ruler.
   */
  plot->p2dHRuler = gtk_hruler_new();

  gtk_ruler_set_metric(GTK_RULER(plot->p2dHRuler),
		       GTK_PIXELS);

  gtk_ruler_set_range(GTK_RULER(plot->p2dHRuler),
		      0,
		      10,
		      0,
		      10);

#if !defined(FROZEN_RULER)
  gtk_signal_connect_object(GTK_OBJECT(plot->p2dArea),
			    "motion_notify_event",
			    (GtkSignalFunc)EVENT_METHOD(plot->p2dHRuler,
							motion_notify_event),
			    GTK_OBJECT(plot->p2dHRuler));
#endif

#if defined(FROZEN_RULER)
  gtk_widget_set_sensitive(plot->p2dHRuler,
			   FALSE /* no sensitive */);
#endif

  gtk_table_attach(GTK_TABLE(plot->p2dTable),
		   plot->p2dHRuler,
		   1,
		   2,
		   1,
		   2,
		   GTK_EXPAND | GTK_SHRINK | GTK_FILL,
		   GTK_FILL,
		   0,
		   0);
    
  /*
   * The vertical ruler goes on the left.
   * As the mouse moves across the drawing area, a 'motion_notify_event'
   * is passed to the appropriate event handler for the ruler.
   */
  plot->p2dVRuler = gtk_vruler_new();

  gtk_ruler_set_metric(GTK_RULER(plot->p2dVRuler),
		       GTK_PIXELS);

  gtk_ruler_set_range(GTK_RULER(plot->p2dVRuler),
		      -10,
		      0,
		      0,
		      10);

#if !defined(FROZEN_RULER)
  gtk_signal_connect_object(GTK_OBJECT(plot->p2dArea),
			    "motion_notify_event",
			    (GtkSignalFunc)EVENT_METHOD(plot->p2dVRuler,
							motion_notify_event),
			    GTK_OBJECT(plot->p2dVRuler));
#endif

#if defined(FROZEN_RULER)
  gtk_widget_set_sensitive(plot->p2dVRuler,
			   FALSE /* no sensitive */);
#endif

  gtk_table_attach(GTK_TABLE(plot->p2dTable),
		   plot->p2dVRuler,
		   0,
		   1,
		   0,
		   1,
		   GTK_FILL,
		   GTK_EXPAND | GTK_SHRINK | GTK_FILL,
		   0,
		   0);

  /*
   * Create a graphic context specific to this plot.
   */
  plot->p2dGContext =
    gdk_gc_new(GTK_WIDGET(kernel->widgets.mainBoardWindow)->window);
  plot->p2dFont     = kernel->fonts[GD_FONT_SMALL][GD_FONT_FIXED];


  /*
   * Initialize sampled points.
   * Allocation is done when dropping symbols onto the plot.
   */
  plot->p2dSampleArray = g_ptr_array_new();


  /*
   * Return the opaque structure.
   */
  return (void*)plot;

}


/*
 * Destroy a '2D' plot opaque structure.
 */
static void
gdisp_destroyPlot2D(Kernel_T *kernel,
		    void     *data)
{

  Plot2D_T *plot = (Plot2D_T*)data;

  /*
   * Free symbol list.
   */
  gdisp_dereferenceSymbolList(plot->p2dXSymbolList);
  gdisp_dereferenceSymbolList(plot->p2dYSymbolList);

  /*
   * Now destroy everything.
   */
  if (plot->p2dXSymbolWindow != (GdkWindow*)NULL)
    gdk_window_destroy(plot->p2dXSymbolWindow      );
  if (plot->p2dYSymbolWindow != (GdkWindow*)NULL)
    gdk_window_destroy(plot->p2dYSymbolWindow      );
  gdk_gc_destroy    (plot->p2dGContext             );
  gdk_pixmap_unref  (plot->p2dBackBuffer           );
  gtk_widget_destroy(plot->p2dArea                 );
  gtk_widget_destroy(plot->p2dHRuler               );
  gtk_widget_destroy(plot->p2dVRuler               );
  gtk_widget_destroy(plot->p2dTable                );
  g_array_free      (plot->p2dSignalIdentities,TRUE);

  /*
   * Free data for sampled points.
   */
  gdisp_freeSampledPointTables(kernel,plot,TRUE /* freeAll */);

  /*
   * Free opaque structure.
   */
  memset(plot,0,sizeof(Plot2D_T));
  g_free(plot);

}


/*
 * Record the parent widget.
 */
static void
gdisp_setPlot2DParent (Kernel_T  *kernel,
		       void      *data,
		       GtkWidget *parent)
{

  Plot2D_T *plot = (Plot2D_T*)data;

  /*
   * Remember my parent.
   */
  plot->p2dParent = parent;

}


/*
 * Record initial dimensions provided by the calling process.
 */
static void
gdisp_setPlot2DInitialDimensions (Kernel_T *kernel,
				  void     *data,
				  guint     width,
				  guint     height)
{

  Plot2D_T *plot = (Plot2D_T*)data;

  /*
   * Remember my initial width and height.
   */
  plot->p2dAreaWidth  = width;
  plot->p2dAreaHeight = height;

}


/*
 * Give back to the calling process the top level widget
 * in order to be inserted in a possible container for further
 * dynamic X management.
 */
static GtkWidget*
gdisp_getPlot2DTopLevelWidget (Kernel_T  *kernel,
			       void      *data)
{

  Plot2D_T *plot = (Plot2D_T*)data;

  GDISP_TRACE(3,"Getting back plot 2D top level widget.\n");

  return (GtkWidget*)plot->p2dTable;

}


/*
 * By now, the '2D plot' widgets are created, but not shown yet.
 * Show them here.
 */
static void
gdisp_showPlot2D (Kernel_T  *kernel,
		  void      *data)
{

  Plot2D_T *plot = (Plot2D_T*)data;

  GDISP_TRACE(3,"Showing plot 2D.\n");

  /*
   * Now show everything.
   */
  gtk_widget_show(plot->p2dArea  );
  gtk_widget_show(plot->p2dHRuler);
  gtk_widget_show(plot->p2dVRuler);
  gtk_widget_show(plot->p2dTable );

  /*
   * These 'show' operations generate 'configure' events. Strange.
   */

}


/*
 * Return to calling process what king of plot we are.
 */
static PlotType_T
gdisp_getPlot2DType (Kernel_T *kernel,
		     void     *data)
{

  Plot2D_T *plot = (Plot2D_T*)data;

  GDISP_TRACE(3,"Getting back plot type (2D).\n");

  /*
   * Must be GD_PLOT_2D. See 'create' routine.
   */
  return plot->p2dType;

}


/*
 * Record any incoming symbols.
 * The drop coordinate (0,0) is at the lower left corner of the graphic area.
 */
static void
gdisp_addSymbolsToPlot2D (Kernel_T *kernel,
			  void     *data,
			  GList    *symbolList,
			  guint     xDrop, /* plot coordinates */
			  guint     yDrop  /* plot coordinates */ )
{

  Plot2D_T *plot       = (Plot2D_T*)data;
  GList    *symbolItem =    (GList*)NULL;
  Symbol_T *symbol     = (Symbol_T*)NULL;
  guint     maxSamples =               0;

  GDISP_TRACE(3,"Adding symbols to plot 2D.\n");

  /*
   * If drop coordinates are in the X zone, the symbol has to be
   * attached to the X axis.
   * CAUTION : we bet the X will be monotonic increasing 
   * try plot y=f(t). Realtime will warn us if not.
   */
  if (gdisp_isInsideXZone(plot,
			  xDrop,
			  yDrop) == TRUE) {

    /*
     * We cannot change X symbol while plot is been working...
     * It is a choice. Technically, it is possible, but what for ?
     */
    if (plot->p2dIsWorking == FALSE) {

      plot->p2dSubType = GD_2D_F2T;

      /*
       * Do not forget to decrement the reference of the X symbol.
       */
      gdisp_dereferenceSymbolList(plot->p2dXSymbolList);
      plot->p2dXSymbolList = (GList*)NULL;

      /*
       * Only one symbol on X axis.
       * Take the first symbol of the incoming list. DO NOT LOOP.
       */
      symbolItem = g_list_first(symbolList);
      if (symbolItem != (GList*)NULL) {

	plot->p2dXSymbolList = g_list_append(plot->p2dXSymbolList,
					     symbolItem->data);

	symbol = (Symbol_T*)symbolItem->data;
	symbol->sReference++;

      }

      gdisp_manageSymbolNameWindow(kernel,
				   plot,
				   plot->p2dXSymbolList);

    } /* p2dIsWorking == FALSE */

  }
  else {

    /*
     * Incoming symbols are to be attached to the Y axis.
     */

    /*
     * Loop over all incoming symbols and store only those that
     * are not already in the final list.
     */
    symbolItem = g_list_first(symbolList);
    while (symbolItem != (GList*)NULL) {

      if (g_list_find(plot->p2dYSymbolList,symbolItem->data) == (GList*)NULL) {

	plot->p2dYSymbolList = g_list_append(plot->p2dYSymbolList,
					     symbolItem->data);

	/*
	 * Do not forget to increment the reference of the Y symbol.
	 */
	symbol = (Symbol_T*)symbolItem->data;
	symbol->sReference++;


	/*
	 * Allocate memory for receiving sampled values of the symbol.
	 */
	maxSamples = TSP_PROVIDER_FREQ * GDISP_WIN_T_DURATION;
	g_ptr_array_add(plot->p2dSampleArray,
			(gpointer)dparray_newSampleArray(maxSamples));

      }

      symbolItem = g_list_next(symbolItem);

    }

    gdisp_manageSymbolNameWindow(kernel,
				 plot,
				 plot->p2dYSymbolList);

  }


  /*
   * Refresh graphic area only if back buffer exists.
   */
  if (plot->p2dBackBuffer != (GdkPixmap*)NULL) {

    gdisp_plot2DDrawBackBuffer(kernel,
			       plot,
			       GD_2D_FULL_REDRAW);

    gdisp_plot2DSwapBuffers   (kernel,
			       plot,
			       GD_2D_FULL_REDRAW);

  }

}


/*
 * Broadcast all symbols.
 */
static GList*
gdisp_getSymbolsFromPlot2D (Kernel_T *kernel,
			    void     *data,
			    gchar     axis)
{

  Plot2D_T *plot = (Plot2D_T*)data;

  GDISP_TRACE(3,"Give back the list of symbols handled by the plot 2D.\n");

  switch (axis) {

  case 'X' :
    return plot->p2dXSymbolList;
    break;

  case 'Y' :
    return plot->p2dYSymbolList;
    break;

  default :
    return (GList*)NULL;
    break;

  }

}


/*
 * Real time Starting Step Action.
 */
static gboolean
gdisp_startStepOnPlot2D (Kernel_T *kernel,
			 void     *data)
{

  Plot2D_T *plot     = (Plot2D_T*)data;
  guint     nbEvents = 0;
  guint     cptEvent = 0;
  guint     eventID  = 0;

  GDISP_TRACE(3,"Performing start-step on plot 2D.\n");

  /*
   * Performs actions before starting steps.
   *
   * BUT we must return TRUE to the calling procedure in order to allow
   * the general step management to proceed.
   *
   * Returning FALSE means that our plot is not enabled to perform its
   * step operations, because of this or that...
   */

  /*
   * Checkout X and Y symbol lists.
   * There must be at least one symbol affected to the Y axis.
   * The X symbol list must not be empty.
   */
  if (g_list_length(plot->p2dYSymbolList) == 0 ||
      g_list_length(plot->p2dXSymbolList) == 0    ) {

    plot->p2dIsWorking = FALSE;

    return FALSE;

  }


  /*
   * Before starting steps, block few X signals.
   * Those signals have been recorded into a table.
   * These signals are : CrossingEvents, ButtonPressEvents, MotionEvents.
   */
  if (plot->p2dSignalsAreBlocked == FALSE) {

    nbEvents = plot->p2dSignalIdentities->len;
    for (cptEvent=0; cptEvent<nbEvents; cptEvent++) {

      eventID = (guint)g_array_index(plot->p2dSignalIdentities,
				     guint,
				     cptEvent);

      gtk_signal_handler_block(GTK_OBJECT(plot->p2dArea),
			       eventID);

    }

    plot->p2dSignalsAreBlocked = TRUE;

  }


  /*
   * Tell the plot it has been started by the application kernel.
   */
  plot->p2dIsWorking = TRUE;

  return TRUE /* everything's ok */;

}


/*
 * Real time Step Action.
 */
static void
gdisp_stepOnPlot2D (Kernel_T *kernel,
		    void     *data)
{

  Plot2D_T           *plot           = (Plot2D_T*)data;

  guint               nbCurves       = 0;
  guint               cptCurve       = 0;
  guint               cptPoint       = 0;
  guint               startIndex     = 0;
  guint               nbPoints       = 0;
  KindOfRedraw_T      drawType       = GD_2D_ADD_NEW_SAMPLES;
  DoublePointArray_T *pArray         = (DoublePointArray_T*)NULL;
  DoublePoint_T      *pSample        =      (DoublePoint_T*)NULL;
  DoublePoint_T      *pLastPoint     = (DoublePoint_T*)NULL;
  guint		      totalNbSamples = 0;

  /*
   * Guess we got new TSP values at each refresh cycles.
   */
  nbCurves = plot->p2dSampleArray->len;
  for (cptCurve=0; cptCurve<nbCurves; cptCurve++) {

    pArray         = plot->p2dSampleArray->pdata[cptCurve]; 

    totalNbSamples = dparray_getNbSamples         (pArray);
    startIndex     = dparray_getMarkerIndex       (pArray);
    nbPoints       = dparray_getLeftSamplesFromPos(pArray,
						   startIndex);

    /*
     * Try to know whether F2T becomes F2X...
     */
    pLastPoint = DP_ARRAY_GET_SAMPLE_PTR(pArray,startIndex-1);

    for (cptPoint=startIndex; cptPoint<startIndex+nbPoints; cptPoint++) {

      pSample = DP_ARRAY_GET_SAMPLE_PTR(pArray,cptPoint);

      /*
       * Check if plot2D is still monotonic increasing on X.
       */
      if (plot->p2dSubType == GD_2D_F2T && totalNbSamples > 0) {

	if (pSample->x < pLastPoint->x) {

	  /*
	   * FIXME : What to do if X increase like a exponentiel ?
	   */
	  plot->p2dSubType = GD_2D_F2X;
	  drawType         = GD_2D_FULL_REDRAW;
	  GDISP_TRACE(1,"gdisp_stepOnPlot2D : changing type to F2X\n");

	}

      }

      /*
       * I Could do this check later, but might be speeder on the fly.
       */
      if (pSample->y < plot->p2dPtMin.y) {

	plot->p2dPtMin.y = pSample->y * (1 + GDISP_2D_MARGIN_RATIO);
	drawType         = GD_2D_FULL_REDRAW;

      }
      if (pSample->y > plot->p2dPtMax.y) {

	plot->p2dPtMax.y = pSample->y * (1 + GDISP_2D_MARGIN_RATIO);
	drawType         = GD_2D_FULL_REDRAW;

      }

      switch (plot->p2dSubType) {

      case GD_2D_F2T :
	if (pSample->x > plot->p2dPtMax.x) {

	  plot->p2dPtMax.x = pSample->x +
	                     GDISP_WIN_T_DURATION * GDISP_2D_MARGIN_RATIO; 
	  drawType         = GD_2D_FULL_REDRAW;

	}
	if (plot->p2dPtMax.x - plot->p2dPtMin.x > GDISP_WIN_T_DURATION) {

	  plot->p2dPtMin.x = plot->p2dPtMax.x - GDISP_WIN_T_DURATION;
	  drawType         = GD_2D_SCROLL_X_AXIS;

	}
	break;
 
      case GD_2D_F2X :
	if (pSample->x < plot->p2dPtMin.x) {

	  plot->p2dPtMin.x = pSample->x * (1 + GDISP_2D_MARGIN_RATIO);
	  drawType         = GD_2D_FULL_REDRAW;

	}
	if (pSample->x > plot->p2dPtMax.x) {

	  plot->p2dPtMax.x = pSample->x * (1 + GDISP_2D_MARGIN_RATIO);
	  drawType         = GD_2D_FULL_REDRAW;

	}
	break;

      default : 
	break;

      }

      plot->p2dPtLast = *pSample;
      *pLastPoint     = *pSample;

    } /* end loop upon new points */
	  
  } /* end loop upon curves */
  

  /*
   * Put here what must be done at each step.
   */
  gdisp_plot2DDrawBackBuffer(kernel,
			     plot,
			     drawType);

  gdisp_plot2DSwapBuffers   (kernel,
			     plot,
			     drawType);

}


/*
 * Real time Starting Step Action.
 */
static void
gdisp_stopStepOnPlot2D (Kernel_T *kernel,
			void     *data)
{

  Plot2D_T *plot     = (Plot2D_T*)data;
  guint     nbEvents = 0;
  guint     cptEvent = 0;
  guint     eventID  = 0;

  GDISP_TRACE(3,"Performing stop-step on plot 2D.\n");

  /*
   * Performs actions after stopping steps.
   */

  /*
   * After stopping steps, unblock few X signals.
   * Those signals have been recorded into a table.
   * These signals are : CrossingEvents, ButtonPressEvents, MotionEvents.
   */
  if (plot->p2dSignalsAreBlocked == TRUE) {

    nbEvents = plot->p2dSignalIdentities->len;
    for (cptEvent=0; cptEvent<nbEvents; cptEvent++) {

      eventID = (guint)g_array_index(plot->p2dSignalIdentities,
				     guint,
				     cptEvent);

      gtk_signal_handler_unblock(GTK_OBJECT(plot->p2dArea),
				 eventID);

    }

    plot->p2dSignalsAreBlocked = FALSE;

  }

  /*
   * Tell the plot it has been stopped by the application kernel.
   */
  plot->p2dIsWorking = FALSE;

}


/*
 * Get back to the calling procedure my period, expressed in milliseconds.
 * CAUTION : The period must be an exact multiple of 10.
 *           Should not be lower than 100.
 */
static guint
gdisp_getPlot2DPeriod (Kernel_T *kernel,
		       void     *data)
{

  /*
   * My period is 100 milli-seconds.
   * FIXME : This value is hardcoded. It should be easily modifiable.
   * But how ?
   */
  return 100;

}


/*
 * Get back to the calling procedure my information.
 */
static void
gdisp_getPlot2DInformation (Kernel_T         *kernel,
			    PlotSystemInfo_T *information)
{

#include "pixmaps/gdisp_2dLogo.xpm"

  /*
   *   - Name,
   *   - Formula,
   *   - Descripton for tooltip purpose.
   */
  information->psName        = "2D Plot";
  information->psFormula     = "Y = F ( X or T )";
  information->psDescription = "A typical 2D plot that shows the evolution "
    "of several symbols (Y axis) relatively to a single one (X axis).";
  information->psLogo        = gdisp_2dLogo;

}


/*
 * This procedure is called whenever all symbols have been time-tagged
 * by the corresponding provider sampling thread.
 * The last value of all symbols can now be retreived by the graphic plot.
 *
 * CAUTION : This procedure is called in another thread, compared to all
 * other procedures of the graphic plot that are called by GTK main thread.
 */
static void
gdisp_treatPlot2DSymbolValues (Kernel_T *kernel,
			       void     *data)
{

  Plot2D_T       *plot       = (Plot2D_T*)data;

  Symbol_T       *symbol     = (Symbol_T*)NULL;
  GList          *symbolItem = (GList*)NULL;
  guint           cptCurve   = 0;
  guint           nbCurves   = 0;
  DoublePoint_T   aPoint;


  /*
   * Get X (or T) last value.
   */
  symbol   = (Symbol_T*)plot->p2dXSymbolList->data;
  aPoint.x = symbol->sLastValue;


  /*
   * Get Y last values.
   */
  symbolItem = g_list_first(plot->p2dYSymbolList);
  nbCurves   = plot->p2dSampleArray->len;

  for (cptCurve=0; cptCurve<nbCurves; cptCurve++) {

    symbol   = (Symbol_T*)symbolItem->data;
    aPoint.y = symbol->sLastValue;

    dparray_addSample((DoublePointArray_T*)
		      plot->p2dSampleArray->pdata[cptCurve],
		      &aPoint);

    symbolItem = g_list_next(symbolItem);

  }

}


/*
 --------------------------------------------------------------------
                             PUBLIC ROUTINES
 --------------------------------------------------------------------
*/


void
gdisp_initPlot2DSystem (Kernel_T     *kernel,
			PlotSystem_T *plotSystem)
{

  gchar *trace = (gchar*)NULL;

  /*
   * We must here provide all 'Plot2D' private functions
   * that remain 'static' here, but accessible from everywhere
   * via the kernel.
   */
  plotSystem->psCreate            = gdisp_createPlot2D;
  plotSystem->psDestroy           = gdisp_destroyPlot2D;
  plotSystem->psSetParent         = gdisp_setPlot2DParent;
  plotSystem->psGetTopLevelWidget = gdisp_getPlot2DTopLevelWidget;
  plotSystem->psShow              = gdisp_showPlot2D;
  plotSystem->psGetType           = gdisp_getPlot2DType;
  plotSystem->psAddSymbols        = gdisp_addSymbolsToPlot2D;
  plotSystem->psGetSymbols        = gdisp_getSymbolsFromPlot2D;
  plotSystem->psSetDimensions     = gdisp_setPlot2DInitialDimensions;
  plotSystem->psStartStep         = gdisp_startStepOnPlot2D;
  plotSystem->psStep              = gdisp_stepOnPlot2D;
  plotSystem->psStopStep          = gdisp_stopStepOnPlot2D;
  plotSystem->psGetInformation    = gdisp_getPlot2DInformation;
  plotSystem->psTreatSymbolValues = gdisp_treatPlot2DSymbolValues;
  plotSystem->psGetPeriod         = gdisp_getPlot2DPeriod;

  /*
   * Manage debug traces.
   */
  trace = getenv("GDISP_STRACE");
  gdispVerbosity = (trace != (gchar*)NULL) ? atoi(trace) : 1;

}













