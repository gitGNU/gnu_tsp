/*!  \file 

$Header: /sources/tsp/tsp/src/providers/bb_provider/bb_tsp_provider_main.c,v 1.2 2004/09/20 20:55:59 erk Exp $

-----------------------------------------------------------------------

TSP Library - core components for a generic Transport Sampling Protocol.

Copyright (c) 2002 Yves DUFRENNE, Stephane GALLES, Eric NOULARD and Robert PAGNOT 

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
Maintainer : tsp@astrium-space.com
Component : Consumer

-----------------------------------------------------------------------

Purpose   : Blackboard TSP Provider

-----------------------------------------------------------------------
 */
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <bb_tsp_provider.h>

int 
main (int argc, char ** argv) {

/*   if (bb_attach()) { */
/*     exit(-1); */
/*   } */
  if (argc<2) {
    fprintf(stderr,"%s : argument missing\n",argv[0]);
    fprintf(stderr,"Usage: %s <bbname>\n",argv[0]);
    exit(-1);
  }
  
  bb_tsp_provider_initialise(&argc,&argv,1,argv[1]);

/*   return bb_detach(); */
  return 0;
}