/*!  \file 

$Id: tsp_common.h,v 1.1.2.1 2005/09/17 17:35:04 erk Exp $

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

Project    : TSP
Maintainer : tsp@astrium.eads.net
Component  : Common

-----------------------------------------------------------------------

Purpose   : Main interface for the producer module

-----------------------------------------------------------------------
 */

#ifndef _TSP_COMMON_H
#define _TSP_COMMON_H

#include "tsp_prjcfg.h"
#include "tsp_datastruct.h"

void
TSP_common_sample_symbol_info_list_copy(TSP_sample_symbol_info_list_t* dest_symbols, 
					TSP_sample_symbol_info_list_t  src_symbols);
void
TSP_common_sample_symbol_copy(TSP_sample_symbol_info_t* dest_symbol, 
			      TSP_sample_symbol_info_t src_symbol);
#endif /* _TSP_COMMON_H */
