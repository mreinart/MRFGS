/*
 * fanet_struct.c
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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef bool boolean;
typedef unsigned char byte;

#ifndef __typedef_RADIODATA
#	define __typedef_RADIODATA
typedef struct {
    int8_t rssi;
    int8_t prssi;
    float psnr;
    char coding_rate[5];
    byte crc_err;
    int16_t rx_headers;
    int16_t rx_packets;
    int16_t tx_packets;
    int16_t tx_time;
    int32_t freq_err;
    uint32_t timestamp;
} sRadioData;
#endif


#ifndef __typedef_NAME
#	define __typedef_NAME
typedef struct {
    char name[255];
    byte n_length;
} sName;
#endif


#ifndef __typedef_FANETMAC
#	define __typedef_FANETMAC
typedef struct {
    boolean e_header;
    boolean forward;
    byte type;
    byte s_manufactur_id;
    uint16_t s_unique_id;
    byte ack;
    boolean cast;
    boolean signature_bit;
    byte d_manufactur_id;
    uint16_t d_unique_id;
    uint32_t signature;
    byte valid_bit;
} sFanetMAC;
#endif


#ifndef __typedef_RAW_MESSAGE
#	define __typedef_RAW_MESSAGE
typedef struct {
    char message[255];
    byte m_length;
    byte m_pointer;
} sRawMessage;
#endif

#ifndef __typedef_ACK
#	define __typedef_ACK
typedef struct {
    uint32_t message_id;
    uint32_t time;
    char subheader;
    byte s_address_manufactur_id;
    uint16_t s_address_unique_id;
    byte d_address_manufactur_id;
    uint16_t d_address_unique_id;
    char ack_req;
    char ack_status;
    byte send_events;
} sACK;
#endif

#ifndef __typedef_TRACKING
#	define __typedef_TRACKING
typedef struct {
    byte s_address_manufactur_id;
    uint16_t s_address_unique_id;
    float latitude;
    float longitude;
    boolean tracking;
    byte aircraft_type;
    char aircraft_type_char[16];
    uint16_t altitude;
    float speed;
    float climb;
    float heading;
    boolean turn_rate_on;
    float turn_rate;
    float distance;
} sAirTracking;
#endif

#ifndef __typedef_MESSAGE
#	define __typedef_MESSAGE
typedef struct {
    uint32_t message_id;
    uint32_t time;
    char subheader;
    byte s_address_manufactur_id;
    uint16_t s_address_unique_id;
    byte d_address_manufactur_id;
    uint16_t d_address_unique_id;
    char ack_req;
    char ack_status;
    byte send_events;
    char message_type;
    char message[255];
    byte m_length;
    byte m_pointer;
} sMessage;
#endif


#ifndef __typedef_WEATHER
#	define __typedef_WEATHER
typedef struct {
    char id_station[32];
    char name[64];
    char short_name[32];
    uint32_t time;
    boolean gateway;
    boolean temp;
    boolean wind;
    boolean humid;
    boolean barom;
    boolean e_header;
    byte extended_header;
    float latitude;
    float longitude;
    int16_t altitude;
    float temperature;
    float wind_heading;
    float wind_speed;
    float wind_gusts;
    float humidity;
    float barometric;
} sWeather;
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

#ifndef __typedef_ROUTING
#	define __typedef_ROUTING
typedef struct {
    byte address_manufactur_id;
    uint16_t address_unique_id;
    byte subnet_manufactur_id;
    uint16_t subnet_unique_id;
    byte next_hop_manufactur_id;
    uint16_t next_hop_unique_id;
    int metrik;
    long last_seen;
    float snr;
} sRouting;
#endif

#ifndef __typedef_ONLINE
#	define __typedef_ONLINE
typedef struct {
    long timestamp;
    byte address_manufactur_id[255];
    uint16_t address_unique_id[255];
    int online;
} sOnline;
#endif


/*
#ifndef __typedef_RX_RADIO_MESSAGES
#	define __typedef_RX_RADIO_MESSAGES
	typedef struct {
		char	rx_radio_messages[255];
		byte 	rx_radio_length;
	}sRxRadioMessages;
#endif

#ifndef __typedef_TX_RADIO_MESSAGES
#	define __typedef_TX_RADIO_MESSAGES
	typedef struct {
		char	tx_radio_messages[255];
		byte 	tx_radio_length;
	}sTxRadioMessages;
#endif
*/


