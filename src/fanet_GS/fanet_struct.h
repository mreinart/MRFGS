/*
 * fanet_struct.h
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

#ifndef FANET_STRUCT_H
#define FANET_STRUCT_H

#include <stdbool.h>
#include <stdint.h>

typedef bool boolean;

typedef unsigned char byte;

#ifndef __typedef_RADIODATA
#	define __typedef_RADIODATA
typedef struct {
    int8_t rssi;
    int8_t prssi;
    float psnr;
    char coding_rate[5];
    unsigned char crc_err;
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
    unsigned char n_length;
} sName;
#endif


#ifndef __typedef_FANETMAC
#	define __typedef_FANETMAC
typedef struct {
    boolean e_header;
    boolean forward;
    unsigned char type;
    unsigned char s_manufactur_id;
    uint16_t s_unique_id;
    unsigned char ack;
    boolean cast;
    boolean signature_bit;
    unsigned char d_manufactur_id;
    uint16_t d_unique_id;
    uint32_t signature;
    unsigned char valid_bit;
} sFanetMAC;
#endif


#ifndef __typedef_ACK
#	define __typedef_ACK
typedef struct {
    uint32_t message_id;
    uint32_t time;
    char subheader;
    unsigned char s_address_manufactur_id;
    uint16_t s_address_unique_id;
    unsigned char d_address_manufactur_id;
    uint16_t d_address_unique_id;
    char ack_req;
    char ack_status;
    unsigned char send_events;
} sACK;
#endif

#ifndef __typedef_TRACKING
#	define __typedef_TRACKING
typedef struct {
    unsigned char s_address_manufactur_id;
    uint16_t s_address_unique_id;
    float latitude;
    float longitude;
    boolean tracking;
    unsigned char aircraft_type;
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

#ifndef __typedef_GROUND_TRACKING
#	define __typedef_GROUND_TRACKING
typedef struct {
    unsigned char s_address_manufactur_id;
    uint16_t s_address_unique_id;
    float latitude;
    float longitude;
    boolean tracking;
    unsigned char ground_type;
    char ground_type_char[20];
    float distance;
} sGroundTracking;
#endif

#ifndef __typedef_THERMAL
#	define __typedef_THERMAL
typedef struct {
    uint8_t  s_address_manufactur_id;
    uint16_t s_address_unique_id;
    float latitude;
    float longitude;
    uint8_t confidence;  // 0..7
    uint16_t altitude;
    float avgClimb;
    float avgWind;
    float avgHeading;
} sThermalInfo;
#endif

#ifndef __typedef_HWINFO
#	define __typedef_HWINFO
typedef struct {
    uint8_t  s_address_manufactur_id;
    uint16_t s_address_unique_id;
    uint8_t  device_type;
    boolean experimental;
    uint16_t fw_build_year;
    uint8_t  fw_build_month;
    uint8_t  fw_build_day;
    uint8_t  add1;
    uint8_t  add2;
    char     device_type_str[30];
} sHardwareInfo;
#endif

#ifndef __typedef_LANDMARK
#	define __typedef_LANDMARK
typedef struct {
    unsigned char s_address_manufactur_id;
    uint16_t s_address_unique_id;
    uint8_t subtype;
    uint8_t ttl;
    uint8_t layer;
    bool wind_dep;
    uint8_t wind_bits;
    float latitude;
    float longitude;
    char landmark_text[50];
    uint8_t diameter;
    int16_t altMin;
    int16_t altMax;
} sLandmark;
#endif

#ifndef __typedef_MESSAGE
#	define __typedef_MESSAGE
typedef struct {
    uint32_t message_id;
    uint32_t time;
    char subheader;
    unsigned char s_address_manufactur_id;
    uint16_t s_address_unique_id;
    unsigned char d_address_manufactur_id;
    uint16_t d_address_unique_id;
    char ack_req;
    char ack_status;
    unsigned char send_events;
    char message_type;
    char message[255];
    unsigned char m_length;
    unsigned char m_pointer;
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
    unsigned char extended_header;
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


#ifndef __typedef_ROUTING
#	define __typedef_ROUTING
typedef struct {
    unsigned char address_manufactur_id;
    uint16_t address_unique_id;
    unsigned char subnet_manufactur_id;
    uint16_t subnet_unique_id;
    unsigned char next_hop_manufactur_id;
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
    unsigned char address_manufactur_id[255];
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

#endif

