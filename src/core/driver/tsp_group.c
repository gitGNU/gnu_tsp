/*

$Header: /sources/tsp/tsp/src/core/driver/tsp_group.c,v 1.13 2008/02/05 18:54:10 rhdv Exp $

-----------------------------------------------------------------------

TSP Library - core components for a generic Transport Sampling Protocol.

Copyright (c) 2002 Yves DUFRENNE, Stephane GALLES, Eric NOULARD, Robert PAGNOT and Arnaud MORVAN

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

Purpose   : Implementation for the groups management

-----------------------------------------------------------------------
 */

#include "tsp_sys_headers.h"

#include "tsp_group_data.h"
#include "tsp_group.h"
#include <tsp_decoder.h>

#include "tsp_data_receiver.h"


void TSP_group_delete_group_table(TSP_groups_t groups)
{

   TSP_group_table_t* table = (TSP_group_table_t*)groups;
   int32_t i;

   if(table)
     {
       free(table->items_table);table->items_table = 0;
       /* free decode buffer for each group */
       for (i=0;i<table->table_len;++i) {
	 free(table->groups[i].decode_buffer);
       }
       free(table->groups);table->groups = 0;
       free(table);
     }

}


TSP_groups_t
TSP_group_create_group_table(const TSP_sample_symbol_info_list_t* symbols, int group_number)
{
    
  int group_id;
  int i;
    
  TSP_group_table_t* table = 0;
  TSP_group_item_t* items_table = 0;
    
  assert(symbols);
    
  table = (TSP_group_table_t*)calloc(1, sizeof(TSP_group_table_t));
  TSP_CHECK_ALLOC(table, 0);
    
  /* Get total number of groups */
  table->table_len = group_number;
    
  /*Allocate room for all groups */
  table->groups = (TSP_group_t*)calloc(table->table_len, sizeof(TSP_group_t));
  TSP_CHECK_ALLOC(table->groups, 0);
    
    /*Allocate room for all group items*/
  table->groups_summed_size = symbols->TSP_sample_symbol_info_list_t_len;
  table->items_table = (TSP_group_item_t*)calloc(table->groups_summed_size, sizeof(TSP_group_item_t));
  items_table = table->items_table;
  TSP_CHECK_ALLOC(items_table, 0);
        
  /*Initialize groups items*/
  /* And make them point at the right place in the item list */
  i = 0;
  table->max_group_len = 0;
  for(group_id = 0; group_id < group_number; group_id++) 
    {
     
      /* Rank for the symbol in a group*/
      int rank = 0;
      /* The maximum size of the decode buffer 
       * to be used for this group.
       * Should be resetted and recomputed for each group.
       */
      int32_t decode_buffer_maxsize=0;
     
      /* Correct items pointer */
      table->groups[group_id].items = items_table;
        
      while(i < symbols->TSP_sample_symbol_info_list_t_len)
	{
    
	  if(group_id == symbols->TSP_sample_symbol_info_list_t_val[i].provider_group_index)
	    {
	      
	      memcpy(&table->groups[group_id].items[rank].symbol_info, &symbols->TSP_sample_symbol_info_list_t_val[i], sizeof(symbols->TSP_sample_symbol_info_list_t_val[i]));
	      table->groups[group_id].items[rank].provider_global_index = symbols->TSP_sample_symbol_info_list_t_val[i].provider_global_index;

	      /*find the dat type and decode and load  the data */
	      table->groups[group_id].items[rank].data_decoder = TSP_data_channel_get_decoder(symbols->TSP_sample_symbol_info_list_t_val[i].type);
	      table->groups[group_id].items[rank].sizeof_encoded_item = 
		                                             TSP_data_channel_get_encoded_size(symbols->TSP_sample_symbol_info_list_t_val[i].type)
		                                             * symbols->TSP_sample_symbol_info_list_t_val[i].dimension;
	      table->groups[group_id].sizeof_encoded_group += table->groups[group_id].items[rank].sizeof_encoded_item;
	      /* update decode buffer maximum size which is the size
	       * of the longest encoded size of all items in the group
	       */
	      decode_buffer_maxsize = (table->groups[group_id].items[rank].sizeof_encoded_item > decode_buffer_maxsize ?
				       table->groups[group_id].items[rank].sizeof_encoded_item :
				       decode_buffer_maxsize);
	      
	      STRACE_DEBUG("Added to group table Id=%d, Gr=%d, Rank=%d", 
			    table->groups[group_id].items[rank].provider_global_index,
			    group_id,
			    rank);
	      rank++; 
	      i++;
	    }  
	  else
	    {
	      break;
	    }
        
            
	}
      /* set group size */
      table->groups[group_id].group_len = rank;
      /* allocate group decode buffer */
      table->groups[group_id].decode_buffer = calloc(1,decode_buffer_maxsize);
      TSP_CHECK_ALLOC(table->groups[group_id].decode_buffer,0);

      /* step thrue table */
      items_table += rank;
      
      /* calculate max group size*/
      if( table->groups[group_id].group_len > table->max_group_len)
	{
	  table->max_group_len = table->groups[group_id].group_len;
	
	}

    }
            
  return table;
}
                                                             
