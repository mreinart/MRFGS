/*
 * fanet_t4_service.h
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

#ifndef FANET_T4_SERVICE_H
#define FANET_T4_SERVICE_H

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "fanet_mac.h"
#include "fanet_struct.h"
#include "fanet_global.h"
#include "fanet_radio.h"
#include "fanet_t2_name.h"
#include "fanet_terminal.h"

/*******************************************************************
 * Temperature (+1byte in 0.5 degree, 2-Complement) 
 * ****************************************************************/
void decode_temperature(sRawMessage *_rx_message, sWeather *_weather_data);

void code_temperature(sRawMessage *_tx_message, sWeather *_weather_data);

/*******************************************************************
 * Wind (+3byte: 1byte Heading in 360/256 degree, 1byte speed and 1byte gusts in 0.2km/h (each: bit 7 scale 5x or 1x, bit 0-6))
 ******************************************************************/
void decode_wind(sRawMessage *_rx_message, sWeather *_weather_data);

void code_wind(sRawMessage *_tx_message, sWeather *_weather_data);

/*******************************************************************
 * Humidity (+1byte: in 0.4% (%rh*10/4)) 
 * ****************************************************************/
void decode_humidity(sRawMessage *_rx_message, sWeather *_weather_data);

void code_humidity(sRawMessage *_tx_message, sWeather *_weather_data);

/*******************************************************************
 * Barometric pressure (+2byte: in 1Pa, offseted by 430hPa, unsigned little endian (hPa-430)*100) 
 * ****************************************************************/
void decode_barometric(sRawMessage *_rx_message, sWeather *_weather_data);

void code_barometric(sRawMessage *_tx_message, sWeather *_weather_data);


void type_4_service_decoder(sRawMessage *_rx_message, sWeather *_weather_data);

void type_4_service_coder(sRawMessage *_tx_message, sWeather *_weather_data);

void type_4_service_receiver(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_rx_payload);


#endif

