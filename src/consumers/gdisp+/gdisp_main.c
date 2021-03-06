/*

$Id: gdisp_main.c,v 1.16 2006/09/21 20:19:59 esteban Exp $

-----------------------------------------------------------------------

GDISP+ - Graphic Tool based on GTK+,
         for being used upon the generic Transport Sampling Protocol.

Copyright (c) 2003 - Euskadi.

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
Maintainer: tsp@astrium.eads.net
Component : Graphic Tool

-----------------------------------------------------------------------

Purpose   : Graphic Tool based on GTK+ that provide several kinds of
            plot. This tool is to be used with the generic TSP protocol.

File      : Graphic Tool main part.

-----------------------------------------------------------------------
*/


/*
 * System includes.
 */
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <libgen.h> /* basename function */


/*
 * GDISP+ includes.
 */
#include "gdisp_kernel.h"
#include "gdisp_prototypes.h"

/**
 * @defgroup TSP_Targa Targa
 * @ingroup  TSP_Consumers
 * targa is the second generation TSP consumer GUI.
 * This a gtk+1.2 (which should evolve to gtk+2.x), 
 * GUI which may be used to efficiently draw or view TSP sample symbol. 
 * Targa includes following features:
 * <ul>
 *   <li> Multi Y vs X graphing capability </li>
 *   <li> Graph snapshot </li>
 *   <li> Textual Viewing with rendering (hex, binary, etc...) </li>
 *   <li> Interactive drag'n'drop </li>
 *   <li> Save/Restore sampling configuration </li>
 *   <li> many more to come... </li>
 * </ul>
 * The targa command lines options are the following:
 * \par  targa [-u TSPurl ] [-h host ] [-x config.xml]
 * <ul>
 *   <li> \b -u TSPurl the TSP URL @ref TSP_URL_FORMAT_USAGE</li>   
 *   <li> \b -h host the hostname or IP address </li>
 *   <li> \b -x config.xml the Targa xml configuration file </li>
 * </ul>
 * @section targa_Design Targa Design
 * Targa is a TSP consumer which is designed as a modular graphical object renderer.
 * You can display the TSP symbols in XY Graph or textual view or with
 * the other graphical object renderer.
 * 
 * @subsection targa_plugins Graphical plugins
 * Targa map TSP symbols onto graphical object renderer which are 
 * structured as Targa plugins.
 * @subsubsection targa_plottext Text Plot
 * The text viewer module is able to display textual values
 * of symbols with different rendering:
 *       <ul>
 *         <li> hexadecimal </li>
 *         <li> octal       </li>
 *         <li> scientific  </li>
 *         <li> ... </li>
 *       </ul>
 * @subsubsection targa_graph 2D Graph Plot
 * The 2D grapher module is able to plot Y versus X curves with
 * mutiple Y. You may drag'n'drop TSP symbols onto the graph widget
 * in order to chose X and Y's data.
 */

/**
 * @defgroup TSP_TargaLib Targa Library
 * The Targa libraries.
 * @ingroup TSP_Targa
 */

/*
 --------------------------------------------------------------------
                             STATIC ROUTINES
 --------------------------------------------------------------------
*/

/*
 * Usage.
 */
static void
gdisp_usage ( Kernel_T *kernel,
	      gchar    *applicationName )
{

  /*
   * Print application usage.
   */
  fprintf(stdout,
	  "------------------------------------------------------------\n");

  fprintf(stdout,
	  "Usage : %s [ -x cnf.xml ] [ -r] [ -w ]\n",
	  basename(applicationName));

  fprintf(stdout,
	  "              [ -u url1 [ -u url2 ... ] ] [ -h host1 [ -h host2 ... ] ]\n");

  fprintf(stdout,
	  "              [ -p path-to-graphic-modules ]\n");

  fprintf(stdout,
	  "  -x : Automatically load the specified configuration.\n");

  fprintf(stdout,
	  "  -r : Runtime mode. Edition is not available\n");

  fprintf(stdout,
	  "  -w : Allow asynchronous write operations on symbols.\n");

  fprintf(stdout,
	  "  -u : Insert one or several specific URLs.\n");

  fprintf(stdout,
	  "  -h : Insert one or several additional hosts.\n");

  fprintf(stdout,
	  "       'localhost' is always taken into account if no URL specified.\n");

  fprintf(stdout,
	  "------------------------------------------------------------\n");

  fflush (stdout);

}


/*
 * Analyse all arguments given by the user.
 * Returns TRUE in case of error.
 */
static gboolean
gdisp_analyseUserArguments ( Kernel_T *kernel )
{

  gint     opt      = 0;
  gboolean mustStop = FALSE;

  /*
   * The user can specify several host to be looked at, with the '-h' option.
   */
  while ((opt = getopt(kernel->argCounter,
		       kernel->argTable,
		       "h:u:x:wrp:")) != EOF) {

    switch (opt) {

    case 'h' :
      gdisp_addHost(kernel,optarg);
      break;

    case 'u' :
      gdisp_addUrl(kernel,optarg);
      break;

    case 'w' :
      kernel->asyncWriteIsAllowed = TRUE;
      break;

    case 'r' :
      kernel->editionIsAllowed = FALSE;
      break;

    case 'p' :
      kernel->pathToGraphicModules = gdisp_strDup(optarg);
      break;

    case 'x' :
      if (kernel->ioFilename != (gchar*)NULL) {
	g_free(kernel->ioFilename);
      }
      kernel->ioFilename = gdisp_strDup(optarg);
      break;

    default :
      mustStop = TRUE;
      break;

    } /* end switch */

  } /* end while */

  return mustStop;

}


/*
 --------------------------------------------------------------------
                             PUBLIC ROUTINES
 --------------------------------------------------------------------
*/


/*
 * GDISP+ main part.
 */
gint
main (int argc, char **argv) 
{

  gint      retVal      = 0;
  Kernel_T *gdispKernel = (Kernel_T*)NULL;
  gboolean  mustStop    = FALSE;


  /*
   * Set up GTK+ things, such as the default visual and color map.
   * Initialize the library for use, sets up default signal handlers, and
   * checks the arguments passed to the application on the command line.
   */
  gtk_init(&argc, &argv);


  /*
   * Show splash screen.
   */
  gdisp_showSplashScreen();


  /*
   * Allocate memory for managing GDISP+ kernel.
   * Take into account user defined arguments passed to the application
   * on the command line.
   */
  gdispKernel = gdisp_createKernel(argc,argv);


  /*
   * Discover all options given by the user.
   */
  mustStop = gdisp_analyseUserArguments(gdispKernel);
  if (mustStop == TRUE) {

    /*
     * Write the usage.
     */
    gdisp_usage(gdispKernel,argv[0]);

    /*
     * Destroy kernel, and exit.
     */
    gdisp_destroyKernel(gdispKernel);

    retVal = -1;
    return retVal;

  }


  /*
   * Create GDISP+ main board.
   * The main board is a little window that offers all GDISP+ functions.
   */
  gdisp_createMainBoard(gdispKernel);


  /*
   * Write few information into the output window before starting.
   */
  gdisp_writeInitialInformation(gdispKernel);


  /*
   * Create our colormap.
   */
  gdisp_createColormap(gdispKernel);


  /*
   * Initialise all plot systems.
   * Each plot system that is supported may provide several functions.
   */
  gdisp_loadGraphicModules(gdispKernel);


  /*
   * Restore configuration if a configuration in
   * file was provided on command line.
   * We may not restore it before GUI is built.
   */
  if ((gdispKernel->ioFilename != (gchar*)NULL) &&
      (gdisp_openConfigurationFile(gdispKernel) == FALSE)) {

    /*
     * Write the usage.
     */
    gdisp_usage(gdispKernel,argv[0]);

  }
  else {

#if defined(GD_NEED_CONSUMMING_AT_STARTUP)
  
    /*
     * Initialize consuming environment.
     */
    gdisp_consumingInit(gdispKernel);

#endif

    /*
     * Enter GTK main processing loop.
     * Sleep waiting for X events (such as button or key presses), timeouts,
     * file IO notifications to occur.
     */
    gtk_main();

  }

  /*
   * Close consuming environment.
   */
  gdisp_consumingEnd(gdispKernel);


  /*
   * Destroy our colormap.
   */
  gdisp_destroyColormap(gdispKernel);


  /*
   * End of GDISP+ main part, by destroying kernel.
   */
  gdisp_destroyKernel(gdispKernel);

  return retVal;

}
