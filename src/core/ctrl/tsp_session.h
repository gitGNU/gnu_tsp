/*

$Header: /sources/tsp/tsp/src/core/ctrl/tsp_session.h,v 1.21 2008/03/24 23:56:20 deweerdt Exp $

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
Component : Provider

-----------------------------------------------------------------------

Purpose   : Interface for the object TSP_session_t that embody the
opened session from a client

-----------------------------------------------------------------------
 */

#ifndef _TSP_SESSION_H
#define _TSP_SESSION_H

#include <tsp_prjcfg.h>

#include <tsp_datastruct.h>
#include <tsp_glu.h>

/**
 * Add a new session.
 * @param new_channel_id The created session number
 * @param glu_h The GLU handled that will be linked to the session
 * @return TRUE or FALSE. TRUE = OK.
 */
int32_t TSP_add_session(channel_id_t* new_channel_id, GLU_handle_t* glu_h);

/**
 * Session initialisation.
 */
void TSP_session_init();

/**
 * Close a session.
 * @param channel_id The channel id that must be close
 */
void TSP_session_close_session_by_channel(channel_id_t channel_id);

/**
 * Calculate the symbol table.
 * FIXME : remove thoses RPC struct for this function
 * @param req_sample The symbols list
 * @param ans_sample The computed answer
 * @return TRUE or FALSE. TRUE = OK.
 */
int32_t 
TSP_session_create_symbols_table_by_channel(const TSP_request_sample_t* req_sample,
					    TSP_answer_sample_t* ans_sample);


/**
 * destroy a session.
 * @param channel_id The channel id that must be destroyed
 */
void TSP_session_destroy_symbols_table_by_channel(channel_id_t channel_id);

/**
 * Send data to all connected clients.
 * @param t current time stamp sent with the data
 */
void TSP_session_all_session_send_data(time_stamp_t t);


/**
 * Send control message to all connected clients.
 * @param msg_ctrl Control message to that must be sent
 */
void TSP_session_all_session_send_msg_ctrl(TSP_msg_ctrl_t msg_ctrl);

/**
 * Send control message to one client.
 * @param channel_id Session id for this client
 * @param msg_ctrl Control message to that must be sent
 * @return TRUE or FALSE. TRUE = OK
 */
int TSP_session_send_msg_ctrl_by_channel(channel_id_t channel_id, TSP_msg_ctrl_t msg_ctrl);

/**
 * Get the address string to which the client must connect
 * to receive data.(format depends on the used protocol ; for
 * TCP/IP it is 'hostname : port')
 * @param channel_id The session id matching the asked adresse
 * @return data address. An error return 0;
 */
const char* TSP_session_get_data_address_string_by_channel(channel_id_t channel_id);

/**
 * Create the data sender object for a session.
 * @param channel_id The session id for which the data sender must be created.
 * @return TSP_STATUS_OK on success.
 */
int32_t 
TSP_session_create_data_sender_by_channel(channel_id_t channel_id);

/**
 * Destroy the data sender object for a session.
 * @param channel_id The session id for which the data sender must be destroyed.
 * @return TSP_STATUS_OK on success.
 */
int32_t 
TSP_session_destroy_data_sender_by_channel(channel_id_t channel_id);

/**
 * Get the whole list of symbols managed by the GLU.
 * @param channel_id The session id for the list must be known
 * @param symbol_list The GLU symbol list for this session
 * @return TRUE or FALSE. TRUE = OK
 */
int  TSP_session_get_sample_symbol_info_list_by_channel(channel_id_t channel_id,
							TSP_sample_symbol_info_list_t* symbol_list);

/**
 * Get the provider global index for a list of symbols.
 * The each symbol will be recongnized with its string name,
 * and the provider global id will be set in the symbol structure
 * @param channel_id The session id
 * @param symbol_list The list of symbol that must be updated
 * @return TRUE or FALSE. TRUE = OK
 */
int TSP_session_get_symbols_global_index_by_channel(channel_id_t channel_id,
						   TSP_sample_symbol_info_list_t* symbol_list);

/**
 * Is a consumer already connected for the session ?
 * @return TRUE or FALSE. TRUE = A consumer is connected
 */
int TSP_session_is_consumer_connected_by_channel(channel_id_t channel_id);

/**
 * Get the GLU handle of the specified sessions.
 * @param channel_id the TSP channel id
 * @return the GLU handle attached to the session
 *         may be NULL if session is invalid
 */
GLU_handle_t* TSP_session_get_GLU_by_channel(channel_id_t channel_id);

/**
 * Returns a session that must be destroyed.
 * For each call this function returns a channel_id for which the 
 * session must be destoyed (ex : consumer disconnected)
 * @param channel_id The session that must be destroyed
 * @return FALSE = there isn't any session that must be destroyed
 */
int TSP_session_get_garbage_session(channel_id_t* channel_id);


/**
 * Get current total number of session.
 * @return Total number of session
 */
int TSP_session_get_nb_session();

/**
 * Change session state to target newState.
 * @param channel_id the TSP channel id
 * @param newState the new state for the concerned session
 *  @return TSP_STATUS_OK on success TSP_STATUS_ERROR_xxxx on failure.
 */ 
int32_t 
TSP_session_change_stateTo_byChannel(channel_id_t channel_id, TSP_session_state_t newState);

TSP_session_state_t 
TSP_session_get_state(channel_id_t channel_id);

#endif /* _TSP_SESSION_H */
