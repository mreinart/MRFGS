/*
 * fanet_radio.h
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

#ifndef FANET_RADIO_H
#define FANET_RADIO_H

#include <stdbool.h>
#include <stdint.h>

typedef bool boolean;

typedef unsigned char byte;

#ifndef __typedef_RADIODATA
#	define __typedef_RADIODATA
typedef struct {
    int8_t 	rssi;
    int8_t 	prssi;
    float 	psnr;
    char  	coding_rate[5];
    byte	crc_err;
    int16_t rx_headers;
    int16_t	rx_packets;
    int16_t	tx_packets;
    int16_t	tx_time;
    int32_t freq_err;
    uint32_t timestamp;
}sRadioData;
#endif


// FANET center frequency - EU area
// unsigned int  freq = 868200000; // in Hz! (868.2)

#ifndef __typedef_RAW_MESSAGE
#	define __typedef_RAW_MESSAGE
typedef struct {
    char message[255];
    byte m_length;
    byte m_pointer;
} sRawMessage;
#endif

#ifndef __typedef_SYSTEM
#	define __typedef_SYSTEM
typedef struct {
    long timestamp_1min;
    long timestamp_15min;
    float rssi_avg_1min;
    float rssi_avg_15min;
    int rssi_max_1min;
    int rssi_max_15min;
    int16_t rx_headers_1min;
    int16_t rx_headers_15min;
    int16_t rx_packets_1min;
    int16_t rx_packets_15min;
    int16_t tx_packets_1min;
    int16_t tx_packets_15min;
    int16_t tx_time_1min;
    int32_t tx_time_15min;
} sSystem;
#endif

extern sRadioData radio_data;

void show_register(void);

void read_radio_data(sRadioData *radiodata);

void radio_info(sSystem *_system_data);

boolean read_rx_data(sRawMessage *_rx_radio, sRadioData *_radiodata);

void tx_radio_values(sRadioData *_radiodata);

void write_tx_data(sRadioData *_radiodata, sRawMessage *_tx_message);

void init_fanet_radio(int _delay, boolean _show_msg, int _power);

void led_blink(unsigned char led);

#endif

