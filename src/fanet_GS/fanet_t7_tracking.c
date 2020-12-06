/*
 * fanet_t7_tracking.c
 * 
 * Copyright 2019  <pi@pi4>
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

#ifndef FANET_T7_TRACKING_C
#define FANET_T7_TRACKING_C

#include <stdio.h>
#include "fanet_mac.h"
#include "fanet_struct.h"
#include "fanet_terminal.h"
#include "fanet_global.h"
#include "fanet_t1_tracking.h"

void decode_ground_tracking(sRawMessage *_rx_message, sGroundTracking *_tracking) {
    if (_rx_message->message[_rx_message->m_pointer] & 0x01)
        _tracking->tracking = 1;
    else
        _tracking->tracking = 0;
}

void decode_ground_type(sRawMessage *_rx_message, sGroundTracking *_tracking) {
    _tracking->ground_type = (_rx_message->message[_rx_message->m_pointer] & 0xF0);
    _tracking->ground_type >>= 4;

    switch (_tracking->ground_type) {
        case 0:
            strcpy(_tracking->ground_type_char, "Other");
            break;
        case 1:
            strcpy(_tracking->ground_type_char, "Walking");
            break;
        case 2:
            strcpy(_tracking->ground_type_char, "Vehicle");
            break;
        case 3:
            strcpy(_tracking->ground_type_char, "Bike");
            break;
        case 4:
            strcpy(_tracking->ground_type_char, "Boat");
            break;
        case 7:
            strcpy(_tracking->ground_type_char, "Ground Station");
            break;
        case 8:
            strcpy(_tracking->ground_type_char, "Need a ride");
            break;
        case 9:
            strcpy(_tracking->ground_type_char, "Landed well");
            break;
        case 12:
            strcpy(_tracking->ground_type_char, "Need technical support");
            break;
        case 13:
            strcpy(_tracking->ground_type_char, "Need medical support");
            break;
        case 14:
            strcpy(_tracking->ground_type_char, "Distress call");
            break;
        case 15:
            strcpy(_tracking->ground_type_char, "Distress call automatically");
            break;
        default:
            sprintf(_tracking->ground_type_char, "UNK ground type %d", _tracking->ground_type);
            break;
    }
}


void type_7_tracking_decoder(sRawMessage *_rx_payload, sFanetMAC *_fanet_mac, sGroundTracking *_rx_tracking) {
    float _latitude;
    float _longitude;

    _rx_payload->m_pointer = 0;

    _rx_tracking->s_address_manufactur_id = _fanet_mac->s_manufactur_id;
    _rx_tracking->s_address_unique_id = _fanet_mac->s_unique_id;

    decode_abs_coordinates(&_rx_payload->message[_rx_payload->m_pointer], &_latitude, &_longitude);
    _rx_tracking->latitude = _latitude;
    _rx_tracking->longitude = _longitude;
    _rx_payload->m_pointer += 6;

    _rx_tracking->distance = distance(own_lat, own_lon, _rx_tracking->latitude, _rx_tracking->longitude, 'K');

    decode_ground_tracking(_rx_payload, _rx_tracking);
    decode_ground_type(_rx_payload, _rx_tracking);
}


void type_7_tracking_coder(sRawMessage *_tx_message, sGroundTracking *_tx_tracking) {
    memset(_tx_message->message, 0, 256);
    _tx_message->m_length = 0;
    _tx_message->m_pointer = 0;

    encode_abs_coordinates(_tx_message, _tx_tracking->latitude, _tx_tracking->longitude);
    _tx_message->m_pointer += 6;
    uint8_t bbb = (_tx_tracking->ground_type & 0x0F) << 4;
    (_tx_tracking->tracking ? bbb |= 0x01 : 0x00 );
    _tx_message->message[_tx_message->m_pointer] = bbb;
    _tx_message->m_length++;
    _tx_message->m_pointer++;
}

void type_7_tracking_receiver(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_rx_payload) {
    sGroundTracking _rx_tracking;
    type_7_tracking_decoder(_rx_payload, _fanet_mac, &_rx_tracking);
    terminal_message_7(0, 0, _radiodata, _fanet_mac, &_rx_tracking);
}

#endif
