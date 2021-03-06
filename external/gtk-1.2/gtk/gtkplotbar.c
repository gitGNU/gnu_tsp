/* gtkplotbar - bar plots widget for gtk+
 * Copyright 1999-2001  Adrian E. Feiguin <feiguin@ifir.edu.ar>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include "gtkplot.h"
#include "gtkplot3d.h"
#include "gtkplotdata.h"
#include "gtkplotbar.h"
#include "gtkplotpc.h"
#include "gtkplotps.h"
#include "gtkpsfont.h"

static void gtk_plot_bar_class_init 	(GtkPlotBarClass *klass);
static void gtk_plot_bar_init 		(GtkPlotBar *data);
static void gtk_plot_bar_draw_symbol	(GtkPlotData *data, 
					 gdouble x, 
                                         gdouble y, 
                                         gdouble z,
                                         gdouble a,
					 gdouble dx, 
                                         gdouble dy, 
                                         gdouble dz, 
                                         gdouble da);
static void gtk_plot_bar_draw_legend	(GtkPlotData *data, 
					 gint x, gint y);

static gint roundint (gdouble x);


static GtkPlotDataClass *parent_class = NULL;

GtkType
gtk_plot_bar_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      GtkTypeInfo data_info =
      {
	"GtkPlotBar",
	sizeof (GtkPlotBar),
	sizeof (GtkPlotBarClass),
	(GtkClassInitFunc) gtk_plot_bar_class_init,
	(GtkObjectInitFunc) gtk_plot_bar_init,
	/* reserved 1*/ NULL,
        /* reserved 2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_plot_data_get_type(), &data_info);
    }
  return data_type;
}

static void
gtk_plot_bar_class_init (GtkPlotBarClass *klass)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkPlotDataClass *data_class;

  parent_class = gtk_type_class (gtk_plot_data_get_type ());

  object_class = (GtkObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;
  data_class = (GtkPlotDataClass *) klass;

  data_class->draw_legend = gtk_plot_bar_draw_legend;
  data_class->draw_symbol = gtk_plot_bar_draw_symbol;
}


static void
gtk_plot_bar_init (GtkPlotBar *dataset)
{
  GtkWidget *widget;
  GdkColor black, white;
  GdkColormap *colormap;

  widget = GTK_WIDGET(dataset);

  colormap = gdk_colormap_get_system();

  gdk_color_black(colormap, &black);
  gdk_color_white(colormap, &white);

  GTK_PLOT_DATA(dataset)->symbol.symbol_style = GTK_PLOT_SYMBOL_FILLED;
  GTK_PLOT_DATA(dataset)->symbol.color = white;
  GTK_PLOT_DATA(dataset)->line.line_style = GTK_PLOT_LINE_SOLID;
  GTK_PLOT_DATA(dataset)->line.line_width = 1;
  GTK_PLOT_DATA(dataset)->line.color = black;

  dataset->width = .05;
}

GtkWidget*
gtk_plot_bar_new (GtkOrientation orientation)
{
  GtkWidget *widget;

  widget = gtk_type_new (gtk_plot_bar_get_type ());

  gtk_plot_bar_construct(GTK_PLOT_BAR(widget), orientation);

  return (widget);
}

void
gtk_plot_bar_construct(GtkPlotBar *bar, GtkOrientation orientation)
{
  bar->orientation = orientation;
}

static void
gtk_plot_bar_draw_symbol(GtkPlotData *dataset, 
                         gdouble x, gdouble y, gdouble z, gdouble a,
                         gdouble dx, gdouble dy, gdouble dz, gdouble da)
{
  GtkPlot *plot;
  GtkPlotBar *bar;
  GdkRectangle area, clip_area;
  gdouble px, py, px0, py0;
  gdouble x1 = 0.0, y1 = 0.0, width = 0.0, height = 0.0;

  bar = GTK_PLOT_BAR(dataset);
  plot = dataset->plot;

  area.x = GTK_WIDGET(plot)->allocation.x;
  area.y = GTK_WIDGET(plot)->allocation.y;
  area.width = GTK_WIDGET(plot)->allocation.width;
  area.height = GTK_WIDGET(plot)->allocation.height;

  clip_area.x = area.x + roundint(plot->x * area.width);
  clip_area.y = area.y + roundint(plot->y * area.height);
  clip_area.width = roundint(plot->width * area.width);
  clip_area.height = roundint(plot->height * area.height);

  gtk_plot_pc_clip(plot->pc, &clip_area);

  if(GTK_IS_PLOT3D(plot)){
  } else {
    switch(bar->orientation){
      case GTK_ORIENTATION_VERTICAL:    
        gtk_plot_get_pixel(plot, x-bar->width, y, &px, &py);
        gtk_plot_get_pixel(plot, x+bar->width, MAX(0., plot->ymin), &px0, &py0);
        break;
      case GTK_ORIENTATION_HORIZONTAL:    
        gtk_plot_get_pixel(plot, y, x+bar->width, &px, &py);
        gtk_plot_get_pixel(plot, MAX(0., plot->xmin), x-bar->width, &px0, &py0);
        break;
    }
    x1 = MIN(px, px0);
    y1 = MIN(py, py0);
    if(GTK_IS_PLOT_PS(plot->pc)){
      width = fabs(px - px0);
      height = fabs(py0 - py);
    }
    else
    {
      width = abs(roundint(px - px0));
      height = abs(roundint(py0 - py));
    }
  
    if(dataset->symbol.symbol_style == GTK_PLOT_SYMBOL_OPAQUE){
      gtk_plot_pc_set_color(plot->pc, &plot->background);
      gtk_plot_pc_draw_rectangle (plot->pc,
                                  TRUE,
                                  x1, y1, width, height);
    }
  
    gtk_plot_pc_set_lineattr (plot->pc, dataset->symbol.border.line_width, 
                              0, 0, 0);
    gtk_plot_pc_set_dash (plot->pc, 0, 0, 0); 

    if(dataset->symbol.symbol_style == GTK_PLOT_SYMBOL_FILLED){
      gtk_plot_pc_set_color(plot->pc, &dataset->symbol.color);
      gtk_plot_pc_draw_rectangle (plot->pc,
                                  TRUE,
                                  x1, y1, width, height);
    }
  
    gtk_plot_pc_set_color(plot->pc, &dataset->symbol.border.color);
    gtk_plot_pc_draw_rectangle (plot->pc,
                                FALSE,
                                x1, y1, width, height);
  
  }

  gtk_plot_pc_clip(plot->pc, NULL);
}


static void
gtk_plot_bar_draw_legend(GtkPlotData *data, gint x, gint y)
{
  GtkPlotBar *bar;
  GtkPlot *plot = NULL;
  GtkPlotText legend;
  GdkRectangle area;
  gint lascent, ldescent, lheight, lwidth;
  gdouble m;

  bar = GTK_PLOT_BAR(data);

  g_return_if_fail(data->plot != NULL);
  g_return_if_fail(GTK_IS_PLOT(data->plot));
  g_return_if_fail(GTK_WIDGET_REALIZED(data->plot));

  plot = data->plot;
  area.x = GTK_WIDGET(plot)->allocation.x;
  area.y = GTK_WIDGET(plot)->allocation.y;
  area.width = GTK_WIDGET(plot)->allocation.width;
  area.height = GTK_WIDGET(plot)->allocation.height;

  m = plot->magnification;
  legend = plot->legends_attr;

  if(data->legend)
    legend.text = data->legend;
  else
    legend.text = "";

  gtk_plot_text_get_size(legend.text, legend.angle, legend.font,
                         roundint(legend.height * m), 
                         &lwidth, &lheight,
                         &lascent, &ldescent);


  legend.x = (gdouble)(area.x + x + roundint((plot->legends_line_width + 4) * m))
             / (gdouble)area.width;
  legend.y = (gdouble)(area.y + y + lascent) / (gdouble)area.height;

  gtk_plot_draw_text(plot, legend);


  if(data->symbol.symbol_style == GTK_PLOT_SYMBOL_OPAQUE){
    gtk_plot_pc_set_color(plot->pc, &plot->background);
    gtk_plot_pc_draw_rectangle(plot->pc, TRUE, 
                               x, y,
                               roundint(plot->legends_line_width * m), 
                               lascent + ldescent);
  }

  gtk_plot_pc_set_lineattr (plot->pc, data->symbol.border.line_width, 0, 0, 0);
  gtk_plot_pc_set_dash(plot->pc, 0, 0, 0);

  if(data->symbol.symbol_style == GTK_PLOT_SYMBOL_FILLED){
    gtk_plot_pc_set_color(plot->pc, &data->symbol.color);
    gtk_plot_pc_draw_rectangle(plot->pc, TRUE, 
                               x, y,
                               roundint(plot->legends_line_width * m), 
                               lascent + ldescent);
  }

  gtk_plot_pc_set_color(plot->pc, &data->symbol.border.color);
  gtk_plot_pc_draw_rectangle(plot->pc, FALSE, 
                             x, y,
                             roundint(plot->legends_line_width * m), 
                             lascent + ldescent);

}

void
gtk_plot_bar_set_width (GtkPlotBar *bar, gdouble width)
{
  if(width < 0.0) return;

  bar->width = width;
}

gdouble
gtk_plot_bar_get_width (GtkPlotBar *bar)
{
  return(bar->width);
}


static gint
roundint (gdouble x)
{
 gint sign = 1;

/* if(x <= 0.) sign = -1; 
*/
 return (x+sign*.50999999471);
}
