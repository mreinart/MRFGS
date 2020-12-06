/*
 * fanet_t1_tracking.c
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

#ifndef FANET_T1_TRACKING_C
#define FANET_T1_TRACKING_C

#include <stdio.h>
#include "fanet_mac.h"
#include "fanet_struct.h"
#include "fanet_terminal.h"
#include "fanet_global.h"
#include "fanet_t1_tracking.h"

extern sWeather *this_station_data;

void decode_tracking(sRawMessage *_rx_message, sAirTracking *_tracking) {
    if (_rx_message->message[_rx_message->m_pointer + 1] & 0x80)
        _tracking->tracking = 1;
    else
        _tracking->tracking = 0;
}

void decode_aircraft_type(sRawMessage *_rx_message, sAirTracking *_tracking) {
    _tracking->aircraft_type = (_rx_message->message[_rx_message->m_pointer + 1] & 0x70);
    _tracking->aircraft_type >>= 4;

    memset(_tracking->aircraft_type_char, 0, 16);
    switch (_tracking->aircraft_type) {
        case 0:
            strcpy(_tracking->aircraft_type_char, "Other");
            break;
        case 1:
            strcpy(_tracking->aircraft_type_char, "Paraglider");
            break;
        case 2:
            strcpy(_tracking->aircraft_type_char, "Hangglider");
            break;
        case 3:
            strcpy(_tracking->aircraft_type_char, "Balloon");
            break;
        case 4:
            strcpy(_tracking->aircraft_type_char, "Glider");
            break;
        case 5:
            strcpy(_tracking->aircraft_type_char, "Aircraft");
            break;
        case 6:
            strcpy(_tracking->aircraft_type_char, "Helicopter");
            break;
        case 7:
            strcpy(_tracking->aircraft_type_char, "UAV");
            break;
        default:
            strcpy(_tracking->aircraft_type_char, "---");
            break;
    }
}

void decode_alitude(sRawMessage *_rx_message, sAirTracking *_tracking) {
    _tracking->altitude = (_rx_message->message[_rx_message->m_pointer + 1] & 0x07);
    _tracking->altitude <<= 8;
    _tracking->altitude |= _rx_message->message[_rx_message->m_pointer];    // Return range will be 0...2'047 m in 1 m steps
    if ((_rx_message->message[_rx_message->m_pointer + 1] & 0x08))
        _tracking->altitude <<= 2;                                            // Return range will be 2'047...8'188 m in 4 m steps

    _rx_message->m_pointer += 2;
}

void encode_alitude(sRawMessage *_tx_message, sAirTracking *_tracking) {
    signed int altitude = _tracking->altitude & 0x07ff;;
    if (_tracking->altitude >= 2048) {
        altitude = _tracking->altitude / 4;
        _tx_message->message[_tx_message->m_length + 1] |= 0x08;       // Set Altitude Scaling Bit: 1->4x;
    }
    _tx_message->message[_tx_message->m_length + 1] |= (altitude >> 8) & 0x07;   // 7
    _tx_message->message[_tx_message->m_length + 0] = (altitude & 0xFF);         // 6

    _tx_message->m_length += 2;
}

void decode_speed(sRawMessage *_rx_message, sAirTracking *_tracking) {
    _tracking->speed = (_rx_message->message[_rx_message->m_pointer] & 0x7F) *
                       0.5;        // Return range will be 0...63.5 km/h m in 0.5 km/h steps
    if ((_rx_message->message[_rx_message->m_pointer] & 0x80))
        _tracking->speed *= 5;                                                        // Return range will be 63.5...317.5 km/h in 2.5 km/h steps

    _rx_message->m_pointer += 1;
}

void encode_speed(sRawMessage *_tx_message, sAirTracking *_tracking) {
    int speed = _tracking->speed * 2;
    if (_tracking->speed > 63.5) {
        _tx_message->message[_tx_message->m_length] |= 0x80;    // Speed scaling bit
        speed = _tracking->speed / 2.5;
    }
    _tx_message->message[_tx_message->m_length] |= (speed & 0x7F);
    _tx_message->m_length += 1;
}

void decode_climb(sRawMessage *_rx_message, sAirTracking *_tracking) {
    signed char _climb_char;

    _climb_char = (_rx_message->message[_rx_message->m_pointer] & 0x7F);
    if (_climb_char & 0x40) {       // Check, if minus (-) bit is set
        _climb_char |= 0x80;
    }
    _tracking->climb = (float) _climb_char;

    if ((_rx_message->message[_rx_message->m_pointer] & 0x80)) {
        _tracking->climb /= 2;     // Return range will be -32.5...-6.5 / +6.5...+31.5 m/s in +-0.5m/s steps
    } else {
        _tracking->climb /= 10;   // Return range will be -6.4...+6.3 m/s in +-0.1m/s steps
    }
    _rx_message->m_pointer += 1;
}

void encode_climb(sRawMessage *_tx_message, sAirTracking *_tracking) {
    int climb = _tracking->climb * 10;
    if ((_tracking->climb > 6.3) || (_tracking->climb < -6.4)) {
        _tx_message->message[_tx_message->m_length] |= 0x80;    // Climb scaling bit
        climb = _tracking->climb * 2;
    }
    _tx_message->message[_tx_message->m_length] |= (climb & 0x7F);
    _tx_message->m_length += 1;
}

void decode_turnrate(sRawMessage *_rx_message, sAirTracking *_tracking) {
    signed char _turnrate_char = (_rx_message->message[_rx_message->m_pointer] & 0x7F);
    if (_turnrate_char & 0x40) {                                                // Check, if minus (-) bit is set
        _turnrate_char |= 0x80;
    }
    _tracking->turn_rate = (float) _turnrate_char;

    if ((_rx_message->message[_rx_message->m_pointer] & 0x80)) {
        _tracking->turn_rate *= 0.1;                                        // Return range will be -64...-16 / +16...+64 deg/s in +-0.25 deg/s steps
    } else {
        _tracking->turn_rate *= 0.5;                                        // Return range will be -15.75...+15.5 deg/s in +-0.1deg/s steps
    }
    _rx_message->m_pointer += 1;
}

void encode_turnrate(sRawMessage *_tx_message, sAirTracking *_tracking) {
    int turnrate = _tracking->turn_rate * 2;
    if (_tracking->turn_rate >= 16.0 | _tracking->turn_rate <= -16.0) {
        _tx_message->message[11] |= 0x80;    // Climb scaling bit
        turnrate = _tracking->turn_rate * 10;
    }
    _tx_message->message[11] |= (turnrate & 0x7F);
    _tx_message->m_length += 1;
}

void decode_heading(sRawMessage *_rx_message, sAirTracking *_tracking) {
    _tracking->heading = 360.0 / 256 *
                         (_rx_message->message[_rx_message->m_pointer]);    // Return range will be 0...360° in 1.4° steps

    _rx_message->m_pointer += 1;
}

void encode_heading(sRawMessage *_tx_message, sAirTracking *_tracking) {
//    _tracking->heading = 360.0/256*(
    int heading = (_tracking->heading * 256 / 360);
    _tx_message->message[10] = heading;    //  0...360° in 1.4° steps

    _tx_message->m_pointer += 1;
}


void type_1_tracking_decoder(sRawMessage *_rx_payload, sFanetMAC *_fanet_mac, sAirTracking *_rx_tracking) {
    float _latitude;
    float _longitude;
    memset(_rx_tracking, 0, sizeof(*_rx_tracking));

    _rx_payload->m_pointer = 0;

    _rx_tracking->s_address_manufactur_id = _fanet_mac->s_manufactur_id;
    _rx_tracking->s_address_unique_id = _fanet_mac->s_unique_id;

    decode_abs_coordinates(&_rx_payload->message[_rx_payload->m_pointer], &_latitude, &_longitude);
    _rx_tracking->latitude = _latitude;
    _rx_tracking->longitude = _longitude;
    _rx_payload->m_pointer += 6;

    _rx_tracking->distance = distance(own_lat, own_lon, _rx_tracking->latitude, _rx_tracking->longitude, 'K');

    decode_tracking(_rx_payload, _rx_tracking);
    decode_aircraft_type(_rx_payload, _rx_tracking);
    decode_alitude(_rx_payload, _rx_tracking);
    decode_speed(_rx_payload, _rx_tracking);
    decode_climb(_rx_payload, _rx_tracking);
    decode_heading(_rx_payload, _rx_tracking);

    _rx_tracking->turn_rate_on = 1;        // No decode routine for turn rate
    decode_turnrate(_rx_payload, _rx_tracking);
}


void type_1_tracking_coder(sRawMessage *_tx_message, sAirTracking *_tx_tracking) {
    // for completeness - needed to relay otherwised received air traffic via FANET
    // assume MAC date are already encoded in message buffer

    memset(_tx_message->message, 0, 256);
    _tx_message->m_length = 0;
    _tx_message->m_pointer = 0;

    encode_abs_coordinates(_tx_message, _tx_tracking->latitude, _tx_tracking->longitude);
    encode_alitude(_tx_message, _tx_tracking);
    _tx_message->message[7] |= (_tx_tracking->aircraft_type << 4);
    encode_speed(_tx_message, _tx_tracking);
    encode_climb(_tx_message, _tx_tracking);
    encode_heading(_tx_message, _tx_tracking);
    encode_turnrate(_tx_message, _tx_tracking);

    _tx_message->m_pointer = 16;
    _tx_message->m_length = 15;
}

void type_1_tracking_receiver(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_rx_payload) {
    sAirTracking _rx_tracking;
    memset(&_rx_tracking, 0, sizeof(_rx_tracking));
    type_1_tracking_decoder(_rx_payload, _fanet_mac, &_rx_tracking);
    terminal_message_1(0, 0, _radiodata, _fanet_mac, &_rx_tracking);
}

#endif
