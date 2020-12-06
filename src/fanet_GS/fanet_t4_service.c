/*
 * fanet_t4_service.c
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
#include "fanet_t2_name.h"
#include "fanet_terminal.h"
#include "fanet_t4_service.h"


/*******************************************************************
 * Temperature (+1byte in 0.5 degree, 2-Complement) 
 * ****************************************************************/
void decode_temperature(sRawMessage *_rx_message, sWeather *_weather_data) {
    _weather_data->temperature = (signed char) _rx_message->message[_rx_message->m_pointer] *
                                 0.5;            // Return range will be -64.0...+63.5°C
    _rx_message->m_pointer += 1;

    _weather_data->temp = true;
}

void code_temperature(sRawMessage *_tx_message, sWeather *_weather_data) {
    float _temperature;

    _temperature = _weather_data->temperature;

    if (_temperature < -64)                            // Check if _temperature is not < -64.0°C
        _temperature = -64;
    if (_temperature > 63.5)                            // Check if _temperature is not > +63.5°C
        _temperature = 63.5;
    _tx_message->message[_tx_message->m_length] = (signed char) (round(
            _temperature * 2));    // Round temp value for 0.5°C grid

    _tx_message->m_length += 1;
}

/*******************************************************************
 * Wind (+3byte: 1byte Heading in 360/256 degree, 1byte speed and 1byte gusts in 0.2km/h (each: bit 7 scale 5x or 1x, bit 0-6))
 ******************************************************************/
void decode_wind(sRawMessage *_rx_message, sWeather *_weather_data) {
    _weather_data->wind_heading = (float) (unsigned char) _rx_message->message[_rx_message->m_pointer] * 360 / 255;
    _rx_message->m_pointer += 1;

    _weather_data->wind_speed = (signed char) _rx_message->message[_rx_message->m_pointer] &
                                0x7F;    // Return range will be 26...127 km/h in 1 kmh/h steps
    if (!(_rx_message->message[_rx_message->m_pointer] & 0x80))
        _weather_data->wind_speed *= 0.2;                                                        // Return range will be 0...25.4 km/h in 0.2 kmh/h steps
    _rx_message->m_pointer += 1;

    _weather_data->wind_gusts = (signed char) _rx_message->message[_rx_message->m_pointer] &
                                0x7F;    // Return range will be 26...127 km/h in 1 kmh/h steps
    if (!(_rx_message->message[_rx_message->m_pointer] & 0x80))
        _weather_data->wind_gusts *= 0.2;                                                        // Return range will be 0...25.4 km/h in 0.2 kmh/h steps
    _rx_message->m_pointer += 1;

    _weather_data->wind = true;
}

void code_wind(sRawMessage *_tx_message, sWeather *_weather_data) {
    float _wind_heading;
    float _wind_speed;
    float _wind_gusts;

    _wind_heading = _weather_data->wind_heading;
    _wind_speed = _weather_data->wind_speed;
    _wind_gusts = _weather_data->wind_gusts;

    if (_wind_heading < 0 || _wind_heading > 360)                // Check if _heading is between 0...360°
        _wind_heading = 0;
    _wind_heading = _wind_heading * 255 / 360;
    _tx_message->message[_tx_message->m_length] = (unsigned char) (_wind_heading);


    if (_wind_speed <= 25.4)                                // Check which scale (5x or 1x) is necessary
    {
        if (_wind_speed < 0)                                // Check if _speed is not < 0 km/h
            _wind_speed = 0;
        _wind_speed = _wind_speed / 0.2;
        _tx_message->message[_tx_message->m_length + 1] = (unsigned char) (_wind_speed);
    } else {
        if (_wind_speed > 127.0)                            // Check if _speed is not > 127 km/h
            _wind_speed = 127.0;
        _tx_message->message[_tx_message->m_length + 1] = (unsigned char) (round(_wind_speed));
        _tx_message->message[_tx_message->m_length +
                             1] |= 0x80;                            // Set scale 5x bit for _speed
    }

    if (_wind_gusts <= 25.4)                                // Check which scale (5x or 1x) is necessary
    {
        if (_wind_gusts < 0)                                // Check if _gusts is not < 0 km/h
            _wind_gusts = 0;
        _wind_gusts = _wind_gusts / 0.2;
        _tx_message->message[_tx_message->m_length + 2] = (unsigned char) (_wind_gusts);
    } else {
        if (_wind_gusts > 127.0)                            // Check if _gusts is not > 127 km/h
            _wind_gusts = 127.0;
        _tx_message->message[_tx_message->m_length + 2] = (unsigned char) (round(_wind_gusts));
        _tx_message->message[_tx_message->m_length + 2] |= 0x80;            // Set scale 5x bit for _gusts
    }

    _tx_message->m_length += 3;
}

/*******************************************************************
 * Humidity (+1byte: in 0.4% (%rh*10/4)) 
 * ****************************************************************/
void decode_humidity(sRawMessage *_rx_message, sWeather *_weather_data) {
    _weather_data->humidity = (unsigned char) _rx_message->message[_rx_message->m_pointer] * 0.4;
    if (_weather_data->humidity > 100)                            // Check if _humidity is not > 100%rh
        _weather_data->humidity = 100;
    _rx_message->m_pointer += 1;

    _weather_data->humid = true;
}

void code_humidity(sRawMessage *_tx_message, sWeather *_weather_data) {
    float _humidity;

    _humidity = _weather_data->humidity;

    if (_humidity < 0)                                // Check if _humidity is not < 0%rh
        _humidity = 0;
    if (_humidity > 100)                            // Check if _humidity is not > 100%rh
        _humidity = 100;
    _humidity = round(_humidity * 2.5);
    _tx_message->message[_tx_message->m_length] = (unsigned char) (_humidity);

    _tx_message->m_length += 1;
}


/*******************************************************************
 * Barometric pressure (+2byte: in 1Pa, offseted by 430hPa, unsigned little endian (hPa-430)*100) 
 * ****************************************************************/
void decode_barometric(sRawMessage *_rx_message, sWeather *_weather_data) {
    unsigned int barometric_int;

    barometric_int = _rx_message->message[_rx_message->m_pointer + 1];
    barometric_int <<= 8;
    barometric_int += _rx_message->message[_rx_message->m_pointer];
    _weather_data->barometric = (barometric_int / (float) 100) + 430;            // Return range: 430...1085.35hPa
    _rx_message->m_pointer += 2;

    _weather_data->barom = true;
}

void code_barometric(sRawMessage *_tx_message, sWeather *_weather_data) {
    float _barometric;
    unsigned int _barometric_int;

    _barometric = _weather_data->barometric;

    if (_barometric < 430)                                    // Check if _barometric is not < 430hPa
        _barometric = 430;
    if (_barometric > 1085.35)                                // Check if _baromteric is not >1085.35hPa
        _barometric = 1085.35;

    _barometric_int = (float) (_barometric - 430) * 100;
    _tx_message->message[_tx_message->m_length] = _barometric_int & 0x00FF;
    _tx_message->message[_tx_message->m_length + 1] = (_barometric_int & 0xFF00) >> 8;

    _tx_message->m_length += 2;
}

void type_4_service_decoder(sRawMessage *_rx_message, sWeather *_weather_data) {
    memset(_weather_data, 0, sizeof(sWeather));
    float _latitude;
    float _longitude;

    _rx_message->m_pointer = 1;

    _weather_data->temp = false;
    _weather_data->wind = false;
    _weather_data->humid = false;
    _weather_data->barom = false;

    if (_rx_message->message[0] & 0x80)
        _weather_data->gateway = true;

    if (_rx_message->message[0] & 0x01)        // Bit E-Header is set
        _rx_message->m_pointer += 1;

    if (_rx_message->m_length > 1) {
        // Skytraxx WLAN GWs also send Type-4 packets with Bis 7 set and only length of 5 in total
        //  - no coordinates - a bit inconsistent with the spec in protocol.txt where it is stated that
        //    0x80 still requires coordinates in paylod
        decode_abs_coordinates(&_rx_message->message[_rx_message->m_pointer], &_latitude, &_longitude);
        _weather_data->latitude = _latitude;
        _weather_data->longitude = _longitude;
        _rx_message->m_pointer += 6;

        if (_rx_message->message[0] & 0x40)
            decode_temperature(_rx_message, _weather_data);

        if (_rx_message->message[0] & 0x20)
            decode_wind(_rx_message, _weather_data);

        if (_rx_message->message[0] & 0x10)
            decode_humidity(_rx_message, _weather_data);

        if (_rx_message->message[0] & 0x08)
            decode_barometric(_rx_message, _weather_data);
    }
}


void type_4_service_coder(sRawMessage *_tx_message, sWeather *_weather_data) {
    _tx_message->message[_tx_message->m_length] = 0;
    if (_weather_data->temp) _tx_message->message[_tx_message->m_length] |= 0x40;
    if (_weather_data->wind) _tx_message->message[_tx_message->m_length] |= 0x20;
    if (_weather_data->humid) _tx_message->message[_tx_message->m_length] |= 0x10;
    if (_weather_data->barom) _tx_message->message[_tx_message->m_length] |= 0x08;
    _tx_message->m_length += 1;

    encode_abs_coordinates(_tx_message, _weather_data->latitude, _weather_data->longitude);

    if (_weather_data->temp)
        code_temperature(_tx_message, _weather_data);

    if (_weather_data->wind)
        code_wind(_tx_message, _weather_data);

    if (_weather_data->humid)
        code_humidity(_tx_message, _weather_data);

    if (_weather_data->barom)
        code_barometric(_tx_message, _weather_data);
}

void type_4_service_receiver(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_rx_payload) {
    sWeather _rx_weather_data;
    type_4_service_decoder(_rx_payload, &_rx_weather_data);
    terminal_message_4(0, 0, _radiodata, _fanet_mac, &_rx_weather_data);
}
