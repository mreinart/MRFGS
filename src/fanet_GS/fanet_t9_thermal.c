/*
 * fanet_t9_thermal.c
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

//#ifndef FANET_T9_THERMAL_C
//#define FANET_T9_THERMAL_C

#include <stdio.h>
#include "fanet_mac.h"
#include "fanet_struct.h"
#include "fanet_terminal.h"
#include "fanet_global.h"
//#include "fanet_db.h"
#include "fanet_t9_thermal.h"

/*
------------

Thermal (Type = 9) (EXPERIMENTAL)
[recommended intervall: floor((#neighbors/10 + 1) * 30s), if a thermal is detected]

[Byte 0-2]	Position of thermal	(Little Endian, 2-Complement)
bit 0-23	Latitude 		(Absolute, see below)
[Byte 3-5]	Position of thermal	(Little Endian, 2-Complement)
bit 0-23	Longitude 		(Absolute, see below)

[Byte 6-7]	Type			(Little Endian)
bit 15		TBD, leave as 0
bit 14-12	confidence/quality	(0 = 0%, 7= 100%)
bit 11		Thermal Altitude Scaling 1->4x, 0->1x
bit 0-10	Thermal Altitude in m

[Byte 8]	Avg climb of thermal	(max +/- 31.5m/s, 2-Complement, climb of air NOT the paraglider)
bit 7		Scaling 	1->5x, 0->1x
bit 0-6		Value		in 0.1m/s

[Byte 9]	Avg wind speed at thermal (max 317.5km/h)
bit 7		Scaling 	1->5x, 0->1x
bit 0-6		Value		in 0.5km/h

[Byte 10]	Avg wind heading at thermal (attention: 90degree means the wind is coming from east and blowing towards west)
bit 0-7		Value		in 360/256 deg

------------
 */


void type_9_thermal_decoder(sRawMessage *_rx_payload, sFanetMAC *_fanet_mac, sThermalInfo *_rx_thermalinfo) {
    float _latitude;
    float _longitude;

    _rx_payload->m_pointer = 0;

    _rx_thermalinfo->s_address_manufactur_id = _fanet_mac->s_manufactur_id;
    _rx_thermalinfo->s_address_unique_id = _fanet_mac->s_unique_id;

    decode_abs_coordinates(&_rx_payload->message[_rx_payload->m_pointer], &_latitude, &_longitude);
    _rx_thermalinfo->latitude = _latitude;
    _rx_thermalinfo->longitude = _longitude;
    _rx_payload->m_pointer += 6;

    uint16_t altBytes =
    altBytes = (_rx_payload->message[_rx_payload->m_pointer] & 0xFF) + ((_rx_payload->message[_rx_payload->m_pointer+1] & 0xFF) << 8);
    _rx_thermalinfo->confidence = (altBytes & 0x7000) >> 12;
    _rx_thermalinfo->altitude = altBytes & 0x03FF;
    if (altBytes & 0x0400) {
        _rx_thermalinfo->altitude *= 4;
    }
    _rx_payload->m_pointer += 2;

    uint8_t avgByte = _rx_payload->message[_rx_payload->m_pointer++];
    _rx_thermalinfo->avgClimb = (avgByte & 0x7F) / 10;
    if (avgByte & 0x80) {
        _rx_thermalinfo->avgClimb *= 5;
    }

    avgByte = _rx_payload->message[_rx_payload->m_pointer++];
    _rx_thermalinfo->avgWind = (avgByte & 0x7F) / 2;
    if (avgByte & 0x80) {
        _rx_thermalinfo->avgWind *= 5;
    }

    avgByte = _rx_payload->message[_rx_payload->m_pointer++];
    _rx_thermalinfo->avgHeading = avgByte * 360 / 256;
}


void type_9_thermal_coder(sRawMessage *_tx_message, sFanetMAC *_fanet_mac, sThermalInfo *_tx_thermalinfo) {
    memset(_tx_message, 0, 255);
    _tx_message->m_length = 0;
    _tx_message->m_pointer = 0;
    //  [Byte 0-2]	Position of thermal	(Little Endian, 2-Complement)
    //  bit 0-23	Latitude 		(Absolute, see below)
    //  [Byte 3-5]	Position of thermal	(Little Endian, 2-Complement)
    //  bit 0-23	Longitude 		(Absolute, see below)
    encode_abs_coordinates(_tx_message, _tx_thermalinfo->latitude, _tx_thermalinfo->longitude);

    //  Byte 6-7]	Type			(Little Endian)
    //  bit 15		TBD, leave as 0
    //  bit 14-12	confidence/quality	(0 = 0%, 7= 100%)
    //  bit 11		Thermal Altitude Scaling 1->4x, 0->1x
    //  bit 0-10	Thermal Altitude in m
    uint16_t altiBytes = 0x0000;
    if (_tx_thermalinfo->altitude > 1023) { // scaling 4
        altiBytes = (_tx_thermalinfo->altitude / 4 ) & 0x03FF ;
        altiBytes |= 0x40;
    } else {
        altiBytes = _tx_thermalinfo->altitude & 0x03FF; // mask 10 bit 0..1023
    }
    uint16_t confiBytes = (_tx_thermalinfo->confidence & 0x07) << 12;
    altiBytes |= confiBytes;
    _tx_message->message[_tx_message->m_length++] = altiBytes & 0xFF;
    _tx_message->message[_tx_message->m_length++] = (altiBytes >> 8) & 0xFF;

    //  [Byte 8]	Avg climb of thermal	(max +/- 31.5m/s, 2-Complement, climb of air NOT the paraglider)
    //  bit 7		Scaling 	1->5x, 0->1x
    //  bit 0-6		Value		in 0.1m/s
    uint8_t avgByte = 0x00;
    avgByte = roundf(_tx_thermalinfo->avgClimb * 10);
    _tx_message->message[_tx_message->m_length++] = avgByte;

    //  [Byte 9]	Avg wind speed at thermal (max 317.5km/h)
    //  bit 7		Scaling 	1->5x, 0->1x
    //  bit 0-6		Value		in 0.5km/h
    avgByte = roundf(_tx_thermalinfo->avgWind * 2);
    _tx_message->message[_tx_message->m_length++] = avgByte;

    //  [Byte 10]	Avg wind heading at thermal (attention: 90degree means the wind is coming from east and blowing towards west)
    //  bit 0-7		Value		in 360/256 deg
    avgByte = (int)roundf(_tx_thermalinfo->avgHeading * 256 / 360) & 0xFF;
    _tx_message->message[_tx_message->m_length++] = avgByte;
    _tx_message->m_pointer = _tx_message->m_length;
}


void type_9_thermal_receiver(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_rx_payload) {
    sThermalInfo _rx_thermal;

    type_9_thermal_decoder(_rx_payload, _fanet_mac, &_rx_thermal);
    terminal_message_9(0, 0, _radiodata, _fanet_mac, &_rx_thermal);

//TBD    write_object_ground_tracking(_radiodata, _fanet_mac, &_rx_tracking);
}


//#endif
