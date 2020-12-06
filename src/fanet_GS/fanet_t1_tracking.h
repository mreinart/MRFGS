/*
 * fanet_t1_tracking.h
 * 
 * Copyright 2019  <pi@RPi3B_FANET_2>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#ifndef FANET_T1_TRACKING_H
#define FANET_T1_TRACKING_H

#include <stdio.h>
#include "fanet_mac.h"
#include "fanet_struct.h"
#include "fanet_terminal.h"
#include "fanet_global.h"

// TODO:get from configuration 

//#define STATION_LAT  46.684681			// Position of Ground Statation -> Is needed to calculate the distance to the tracked object 
//#define STATION_LON  7.867658			// Modfied if necessary

/*
void decode_tracking (sRawMessage *_rx_message, sAirTracking *_tracking);

void decode_aircraft_type (sRawMessage *_rx_message, sAirTracking *_tracking);

void decode_alitude (sRawMessage *_rx_message, sAirTracking *_tracking);
void encode_alitude (sRawMessage *_sx_message, sAirTracking *_tracking);

void decode_speed (sRawMessage *_rx_message, sAirTracking *_tracking);

void decode_climb (sRawMessage *_rx_message, sAirTracking *_tracking);

void decode_heading (sRawMessage *_rx_message, sAirTracking *_tracking);
*/

void type_1_tracking_decoder(sRawMessage *_rx_payload, sFanetMAC *_fanet_mac, sAirTracking *_rx_tracking);

void type_1_tracking_coder(sRawMessage *_tx_message, sAirTracking *_tx_tracking);

void type_1_tracking_receiver(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_rx_payload);

#endif
