/*
 * fanet_t5_landmark.c
 * 
 * Copyright 2018  <pi@RPi3B_FANET_2>
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

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "fanet_mac.h"
#include "fanet_struct.h"
#include "fanet_global.h"
#include "fanet_radio.h"
#include "fanet_terminal.h"
#include "fanet_t5_landmark.h"

void type_5_landmark_decoder(sRawMessage *_rx_message, sLandmark *_landmark_data) {
    memset(_landmark_data, 0, sizeof(sLandmark));
    float _latitude;
    float _longitude;
    _rx_message->m_pointer = 0;
    _landmark_data->subtype = _rx_message->message[_rx_message->m_pointer] & 0x0F;
    _landmark_data->ttl = (_rx_message->message[_rx_message->m_pointer] & 0xF0) >> 4;
    _rx_message->m_pointer +=1;
    _landmark_data->layer = _rx_message->message[_rx_message->m_pointer] & 0x0F;
    _landmark_data->wind_dep = _rx_message->message[_rx_message->m_pointer] & 0x10;
    _rx_message->m_pointer += 1;
    if (_landmark_data->wind_dep) {
        _landmark_data->wind_bits = _rx_message->message[_rx_message->m_pointer];
        _rx_message->m_pointer += 1;
    }
    decode_abs_coordinates(&_rx_message->message[_rx_message->m_pointer], &_latitude, &_longitude);
    _landmark_data->latitude = _latitude;
    _landmark_data->longitude = _longitude;
    _rx_message->m_pointer += 6;
    if (_landmark_data->subtype == 0) {
        memset(_landmark_data->landmark_text, 0, sizeof(_landmark_data->landmark_text));
        snprintf(_landmark_data->landmark_text, sizeof(_landmark_data->landmark_text)-1, "%s", (_rx_message->message + _rx_message->m_pointer));
        _rx_message->m_pointer += strlen(_landmark_data->landmark_text);
    } else {
        memset(_landmark_data->landmark_text, 0, sizeof(_landmark_data->landmark_text));
        strcpy(_landmark_data->landmark_text, "<compressed coordinates etc.>");
        _rx_message->m_pointer += strlen(_landmark_data->landmark_text);
    }
}


void type_5_landmark_coder(sRawMessage *_tx_message, sLandmark *_landmark_data) {
    memset(_tx_message, 0, sizeof(sRawMessage));
    _tx_message->message[_tx_message->m_length] = 0;

//    [Byte 0]
//    bit 4-7		Time to live +1 in 10min (bit 7 scale 6x or 1x, bit 4-6) (0->10min, 1->20min, ..., F->8h)
//    bit 0-3		Subtype:
//    0:     Text
//    1:     Line
//    2:     Arrow
//    3:     Area
//    4:     Area Filled
//    5:     Circle
//    6:     Circle Filled
//    7:     3D Line		suitable for cables
//    8:     3D Area		suitable for airspaces (filled if starts from GND=0)
//    9:     3D Cylinder	suitable for airspaces (filled if starts from GND=0)
//    10-15: TBD
    uint8_t byte0 = (((_landmark_data->ttl & 0x0f) << 4) |  (_landmark_data->subtype & 0x0f));
    _tx_message->message[_tx_message->m_length] = _landmark_data->subtype;
    _tx_message->m_length  += 1;
    _tx_message->m_pointer += 1;

//    [Byte 1]
//    bit 7-5		Reserved
//    bit 4		Internal wind dependency (+1byte wind sector)
//    bit 3-0		Layer:
//    0:     Info
//    1:     Warning
//    2:     Keep out
//    3:     Touch down
//    4:     No airspace warn zone		(not yet implemented)
//    5-14:  TBD
//    15:    Don't care

    _tx_message->message[_tx_message->m_length] = 0;//0x11; // wind bit set + Warning layer
    _tx_message->m_length  += 1;
    _tx_message->m_pointer += 1;

//    [Byte 2 only if internal wind bit is set] Wind sectors +/-22.5degree (only display landmark if internal wind is within one of the advertised sectors.
//            If byte 2 is present but is zero, landmark gets only displayed in case of no wind)
//    bit 7 		NW
//    bit 6		W
//    bit 5		SW
//    bit 4 		S
//    bit 3 		SE
//    bit 2 		E
//    bit 1 		NE
//    bit 0 		N
//
//    _tx_message->message[_tx_message->m_length] = 0x00; // NNNNN
//    _tx_message->m_length  += 1;
//    _tx_message->m_pointer += 1;

//    [n Elements]
//    Text (0): 		Position (Absolute) + String 				//(2 Byte aligned, zero-termination is optional)
//    Line/Arrow (1,2):	Position (1st absolute others compressed, see below, minimum 2 elements)
//    Area (filled)(3,4): 	Position (1st absolute others compressed, see below, minimum 3 elements)
//    Circle (filled)(5,6):	n times: Position (1st absolute others compressed, see below) + Radius (1Byte in 50m, bit 7 scale 8x or 1x, bit 0-6)
//    3D Line (7):		n times: Position (1st in packet absolute others compressed, see below) + Altitude (('1Byte signed'+109) * 25m (-127->-450m, 127->5900m))
//    3D Area (8):		Altitude bottom, top (each: ('1Byte signed'+109) * 25m (-127->-450m, 127->5900m), only once) +
//            n times: Position (1st absolute others compressed, see below)
//    3D Cylinder (9):	n times: Position (1st absolute others compressed, see below) + Radius (1Byte in 50m, bit 7 scale 8x or 1x, bit 0-6) +
//            Altitude bottom, top (each: ('1Byte signed'+109) * 25m (-127->-450m, 127->5900m), only once)

// Text
    encode_abs_coordinates(_tx_message, _landmark_data->latitude, _landmark_data->longitude);

    if (_landmark_data->subtype == 0) {
        strcpy(&_tx_message->message[_tx_message->m_length], (void*)&_landmark_data->landmark_text);
        _tx_message->m_length  += strlen(_landmark_data->landmark_text);
        _tx_message->m_pointer += strlen(_landmark_data->landmark_text);
    } else if (_landmark_data->subtype > 3) {
        _tx_message->message[_tx_message->m_length] = _landmark_data->diameter;
        _tx_message->m_length += 1;
        _tx_message->m_pointer += 1;
    }
}

void type_5_landmark_receiver(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_rx_payload) {
    sLandmark _rx_landmark_data;
    type_5_landmark_decoder(_rx_payload, &_rx_landmark_data);
    terminal_message_5(0, 0, _radiodata, _fanet_mac, &_rx_landmark_data);
}
