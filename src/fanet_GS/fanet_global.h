/*
 * fanet_global.h
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

#ifndef FANET_GLOBAL_H
#define FANET_GLOBAL_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "fanet_struct.h"
#include "fanet_radio.h"

extern sWeather *this_station_data;

double deg2rad(double);

double rad2deg(double);

double distance(double lat1, double lon1, double lat2, double lon2, char unit);

void decode_abs_coordinates(char *_input, float *_latitude, float *_longitude);

void encode_abs_coordinates(sRawMessage *_tx_message, float _latitude, float _longitude);

void address_int(char *_address_string, byte *_manuf, uint16_t *_id);

boolean fanet_type_check(byte _fanet_type);

boolean fanet_manufacturer_check(byte _manufactur_id);

boolean fanet_own_id_checker(sFanetMAC *_fanet_mac);

extern double own_lat;
extern double own_lon;
extern double own_alt;

void set_position(double lat, double lon, double alt);

#endif

