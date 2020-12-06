/*
 * fanet_terminal.c
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

//#ifndef FANET_TERMINAL_C
//#define FANET_TERMINAL_C

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "fanet_struct.h"
#include "fanet_terminal.h"

#define KNRM  "\x1B[0m"        // Color: Normal
#define KRED  "\x1B[31m"    // Color: Red
#define KGRN  "\x1B[32m"    // Color: Green
#define KYEL  "\x1B[33m"    // Color: Yellow
#define KBLU  "\x1B[34m"    // Color: Blue
#define KMAG  "\x1B[35m"    // Color: Mangenta
#define KCYN  "\x1B[36m"    // Color: Cyan
#define KWHT  "\x1B[37m"    // Color: White

#define BOLD        "\e[1m"        // Bold/Bright
#define DIM            "\e[2m"        // Dim
#define UNDERLINE    "\e[4m"        // Underline
#define BLINK        "\e[5m"        // Blink
#define INVERT        "\e[7m"        // Reverse (invert the foreground and background colors)

#define RESET_ALL    "\e[0m"        // Reset all attributs
#define R_BOLD        "\e[21m"    // Reset Bold/Bright
#define R_DIM        "\e[22m"    // Reset Dim
#define R_UNDERLINE    "\e[24m"    // Reset Underline
#define R_BLINK        "\e[25m"    // Reset Blink
#define R_INVERT    "\e[27m"    // Reset Reverse (invert the foreground and background colors)


//#define clear() printf("\033[H\033[J")
//#define gotoxy(x, y) fprintf(output_file, "\033[%d;%dH", (x), (y))

typedef unsigned char byte;

FILE *output_file = 0;

void terminal_set_file(FILE *_output_file) {
    output_file = _output_file;
}

/***********************************************************************
 * Shows the FANET start screen
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |01.10.2018|  bet | Init Version
 **********************************************************************/
void terminal_start_screen(byte _sf, int _bandwith, int _freq) {
    //clear();
    fprintf(output_file, " FANET Monitor V0.1\n");
    fprintf(output_file, "---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    fprintf(output_file, "| Listening at SF%i on %.6lf Mhz with a BW of %d kHz.\n", _sf, (double) _freq / 1000000, _bandwith);
    fprintf(output_file, "|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    fprintf(output_file, "| Time                  | RSSI   | P.RSSI | SNR    | CR | +-frq   | Lenght| Raw Data \n");
    fprintf(output_file, "|                                                                 | E F  T| Source | A C S| Destina| Signatur| Latitude  | Longitude |On|A-Typ| Alt  | Speed    | Climb   | Headin|   T-Rate |  Distance|\n");
    fprintf(output_file, "|                                                                 | E F  T| Source | A C S| Destina| Signatur| Latitude  | Longitude | Temp   |Heading| Speed    | Gusts    | Humidity| Barometric|\n");
    fprintf(output_file, "|                                                                 | E F  T| Source | A C S| Destina| Signatur| Name\n");
    fprintf(output_file, "|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

/***********************************************************************
 * Shows the RF information
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |01.10.2018|  bet | Init Version
 **********************************************************************/
void terminal_rf_info(boolean _rxtx, boolean _integrity, sRadioData *_radio_data, sFanetMAC *_mac_data) {
    struct timeval tv;
    struct tm *ptm;
    char _time_string[40];
    long _milliseconds;

    gettimeofday(&tv, NULL);
    ptm = localtime(&tv.tv_sec);
    _milliseconds = tv.tv_usec / 1000;
    strftime(_time_string, sizeof(_time_string), "%Y-%m-%d %H:%M:%S", ptm);

    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    // Timestamp
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radio_data->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    fprintf(output_file, "%s.%03ld", _time_string, _milliseconds);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // RSSI
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radio_data->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    fprintf(output_file, " %+4ddBm", _radio_data->rssi);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // P.RSSI
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radio_data->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    fprintf(output_file, " %+4ddBm", _radio_data->prssi);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // P.RSNR
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radio_data->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    if (_rxtx) fprintf(output_file, "   -.-dB"); else fprintf(output_file, " %+5.1fdB", _radio_data->psnr);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Coding rate (CR)
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radio_data->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    fprintf(output_file, " %s", _radio_data->coding_rate);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Frequency error
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radio_data->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    if (_rxtx) fprintf(output_file, "      -Hz"); else fprintf(output_file, " %+6dHz", _radio_data->freq_err);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

}

/***********************************************************************
 * Shows the MAC information
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |25.12.2018|  bet | Init Version
 **********************************************************************/
void terminal_mac_info(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data) {
    // MAC Header
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    fprintf(output_file, " %d %d %2d", _mac_data->e_header, _mac_data->forward, _mac_data->type);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // MAC Source Address
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    fprintf(output_file, " %02x:%04x", _mac_data->s_manufactur_id, _mac_data->s_unique_id);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // MAC Extended Header
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    if (_mac_data->e_header) fprintf(output_file, " %d", _mac_data->ack); else fprintf(output_file, " -");
    if (_mac_data->e_header) fprintf(output_file, " %d", _mac_data->cast); else fprintf(output_file, " -");
    if (_mac_data->e_header) fprintf(output_file, " %d", _mac_data->signature_bit); else fprintf(output_file, " -");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // MAC Destination Address
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    if (_mac_data->cast) fprintf(output_file, " %02x:%04x", _mac_data->d_manufactur_id, _mac_data->d_unique_id);
    else
        fprintf(output_file, " --:----");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // MAC Signature
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    if (_mac_data->signature_bit) fprintf(output_file, " %08x", _mac_data->signature); else fprintf(output_file, " --------");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
}


/***********************************************************************
 * Shows the FANET message as HEX on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |13.01.2019|  bet | Init Version
 **********************************************************************/
void terminal_message_raw(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                          sRawMessage *_raw_message) {
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    //terminal_mac_info (_rxtx, _integrity, _mac_data);

    // Raw Data
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    //fprintf(output_file, BOLD);
    fprintf(output_file, "len:%3d", _raw_message->m_length);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err || !_mac_data->valid_bit) fprintf(output_file, KYEL);
    //fprintf(output_file, BOLD);
    _raw_message->m_pointer = 0;
    while (_raw_message->m_pointer < (_raw_message->m_length)) {
        fprintf(output_file, " %02x", _raw_message->message[_raw_message->m_pointer]);
        _raw_message->m_pointer++;
    }
    //fprintf(output_file, R_BOLD);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    fprintf(output_file, "\n");
    fflush(output_file);
}


/***********************************************************************
 * Shows the CRC error on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |26.01.2019|  bet | Init Version
 **********************************************************************/
void terminal_message_crc_err(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data) {
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);

    // Name
    fprintf(output_file, KYEL);
    fprintf(output_file, INVERT);
    fprintf(output_file, " CRC error");
    fprintf(output_file, R_INVERT);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    fprintf(output_file, "\n");
    fflush(output_file);
}

/***********************************************************************
 * Shows the MAC error on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |27.01.2019|  bet | Init Version
 **********************************************************************/
void terminal_message_mac_err(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data) {
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);

    fprintf(output_file, KYEL);
    fprintf(output_file, INVERT);
    fprintf(output_file, " MAC error");
    fprintf(output_file, R_INVERT);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    fprintf(output_file, "\n");
    fflush(output_file);
}


/***********************************************************************
 * Shows the FANET message 0 (ACK) on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |13.01.2019|  bet | Init Version
 **********************************************************************/
void terminal_message_0(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data, sACK *_ack) {
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    terminal_mac_info(_rxtx, _integrity, _radiodata, _mac_data);

    // Name
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, INVERT);
    fprintf(output_file, " ACK");
    fprintf(output_file, R_INVERT);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    fprintf(output_file, "\n");
    fflush(output_file);
}


/***********************************************************************
 * Shows the FANET message 1 (Tracking) on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |26.01.2019|  bet | Init Version
 **********************************************************************/
void terminal_message_1(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sAirTracking *_tracking) {
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    terminal_mac_info(_rxtx, _integrity, _radiodata, _mac_data);

    // Latitude
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %+9.5f°", _tracking->latitude);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Longitude
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %+9.5f°", _tracking->longitude);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Online Tracking
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %d", _tracking->tracking);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Aircraft type
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, "    %d", _tracking->aircraft_type);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    fprintf(output_file, " %s |", _tracking->aircraft_type_char);

    // Altitude
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %4dm", _tracking->altitude);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Speed
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %5.1fkm/h", _tracking->speed);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Climb
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %+5.1fm/s", _tracking->climb);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Heading
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %5.1f°", _tracking->heading);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Turm rate
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    if (_tracking->turn_rate_on) fprintf(output_file, " %+6.2f°/s", _tracking->turn_rate); else fprintf(output_file, "   -.--°/s");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Distance
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, "  %6.2fkm", _tracking->distance);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Tracking - text
    if (_tracking->tracking) {
        fprintf(output_file, " online track allowed |");
    } else {
        fprintf(output_file, " NO online tracking  |");
    }

    fprintf(output_file, "\n");
    fflush(output_file);
}


/***********************************************************************
 * Shows the FANET message 2 (Name) on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |25.12.2018|  bet | Init Version
 **********************************************************************/
void terminal_message_2(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data, sName *_name) {
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    terminal_mac_info(_rxtx, _integrity, _radiodata, _mac_data);

    // Name
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    _name->name[_name->n_length] = 0;            // Add EOF at last
    fprintf(output_file, " %s ", _name->name);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    fprintf(output_file, "\n");
    fflush(output_file);
}


/***********************************************************************
 * Shows the FANET message 3 (Message data) on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |29.12.2018|  bet | Init Version
 **********************************************************************/
void terminal_message_3(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sMessage *_message) {
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    terminal_mac_info(_rxtx, _integrity, _radiodata, _mac_data);

    // Message
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    _message->message[_message->m_length] = 0;            // Add EOF at last
    fprintf(output_file, " %s", _message->message);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    fprintf(output_file, "\n");
    fflush(output_file);
}


/***********************************************************************
 * Shows the FANET message 4 (Weather data) on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Describtion
 * ---------------------------------------------------------------------
 * 0.1 |01.10.2018|  bet | Init Version
 **********************************************************************/
void terminal_message_4(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sWeather *_weather_data) {
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    terminal_mac_info(_rxtx, _integrity, _radiodata, _mac_data);

    if (_weather_data->gateway) {
        if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
        if (_radiodata->crc_err) fprintf(output_file, KYEL);
        fprintf(output_file, " Gateway " );
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");
        if ((_weather_data->latitude != 0.0) &&  (_weather_data->latitude != 0.0)) {
            // Latitude
            if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
            if (_radiodata->crc_err) fprintf(output_file, KYEL);
            fprintf(output_file, " %+9.5f°", _weather_data->latitude);
            fprintf(output_file, KNRM);
            fprintf(output_file, "|");

            // Longitude
            if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
            if (_radiodata->crc_err) fprintf(output_file, KYEL);
            fprintf(output_file, " %+9.5f°", _weather_data->longitude);
            fprintf(output_file, KNRM);
            fprintf(output_file, "|");
        } else {
            if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
            fprintf(output_file, " no coordinates " );
            fprintf(output_file, KNRM);
            fprintf(output_file, "|");
        }
    } else {
        // Latitude
        if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
        if (_radiodata->crc_err) fprintf(output_file, KYEL);
        fprintf(output_file, " %+9.5f°", _weather_data->latitude);
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");

        // Longitude
        if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
        if (_radiodata->crc_err) fprintf(output_file, KYEL);
        fprintf(output_file, " %+9.5f°", _weather_data->longitude);
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");

        // Temperature
        if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
        if (_radiodata->crc_err) fprintf(output_file, KYEL);
        if (_weather_data->temp) fprintf(output_file, " %+5.1f°C", _weather_data->temperature); else fprintf(output_file, "   -.-°C");
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");

        // Wind heading
        if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
        if (_radiodata->crc_err) fprintf(output_file, KYEL);
        if (_weather_data->wind) fprintf(output_file, " %5.1f°", _weather_data->wind_heading); else fprintf(output_file, "   -.-°");
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");

        // Wind speed
        if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
        if (_radiodata->crc_err) fprintf(output_file, KYEL);
        if (_weather_data->wind) fprintf(output_file, " %5.1fkm/h", _weather_data->wind_speed); else fprintf(output_file, "   -.-km/h");
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");

        // Winde gusts
        if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
        if (_radiodata->crc_err) fprintf(output_file, KYEL);
        if (_weather_data->wind) fprintf(output_file, " %5.1fkm/h", _weather_data->wind_gusts); else fprintf(output_file, "   -.-km/h");
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");

        // Humidity
        if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
        if (_radiodata->crc_err) fprintf(output_file, KYEL);
        if (_weather_data->humid) fprintf(output_file, " %5.1f%%rh", _weather_data->humidity); else fprintf(output_file, "   -.-%%rh");
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");

        // Barometric
        if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
        if (_radiodata->crc_err) fprintf(output_file, KYEL);
        if (_weather_data->barom) fprintf(output_file, " %7.2fhPa", _weather_data->barometric); else fprintf(output_file, "    -.--hPa");
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");
    }
    fprintf(output_file, "\n");
    fflush(output_file);
}

/***********************************************************************
 * Shows the FANET message 5 (Landmark) on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Description
 * ---------------------------------------------------------------------
 * 0.1 |2002-03-26|  mr  | Init Version
 **********************************************************************/
void terminal_message_5(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sLandmark *_landmark_data){
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    terminal_mac_info(_rxtx, _integrity, _radiodata, _mac_data);

    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    // subtype
    fprintf(output_file, "%d", _landmark_data->subtype);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    // ttl
    fprintf(output_file, "%d", _landmark_data->ttl);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    // layer
    fprintf(output_file, "%d", _landmark_data->layer);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    if (_landmark_data->wind_dep) {
        fprintf(output_file, "Wind: %02X", _landmark_data->wind_bits);
        fprintf(output_file, KNRM);
        fprintf(output_file, "|");
    }
    // Latitude
    fprintf(output_file, " %+9.5f°", _landmark_data->latitude);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    // Longitude
    fprintf(output_file, " %+9.5f°", _landmark_data->longitude);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    // Text
    fprintf(output_file, KNRM);
    fprintf(output_file, " '%s' ", _landmark_data->landmark_text);
    fprintf(output_file, "|");
    fprintf(output_file, "\n");
    fflush(output_file);
}

/***********************************************************************
 * Shows the FANET message 7 (Ground Tracking) on the terminal
 *
 * *********************************************************************
 * Ver | Date     | Sign | Description
 * ---------------------------------------------------------------------
 * 0.3 |28.09.2019|  mr  | Init Version
 **********************************************************************/
void terminal_message_7(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sGroundTracking *_tracking) {
    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    terminal_mac_info(_rxtx, _integrity, _radiodata, _mac_data);

    // Latitude
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %+9.5f°", _tracking->latitude);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Longitude
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %+9.5f°", _tracking->longitude);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Online Tracking
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %d", _tracking->tracking);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Ground type
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    switch (_tracking->ground_type) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            fprintf(output_file, KGRN);
            break;
        case 7:                    // Ground station
        case 8:
            fprintf(output_file, KYEL);
            break;
        case 9:
            fprintf(output_file, KGRN);
            break;
        case 12:
        case 13:
        case 14:
            fprintf(output_file, KRED);
            break;
        case 15:
            fprintf(output_file, KMAG);
            break;
        default:
            fprintf(output_file, KGRN);
            break;
    }
    fprintf(output_file, "    %d", _tracking->ground_type);
    fprintf(output_file, "|");
    fprintf(output_file, " %s ", _tracking->ground_type_char);
    fprintf(output_file, "|");
    fprintf(output_file, KWHT);

    // Distance
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, "  %6.2fkm", _tracking->distance);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Tracking
    if (_tracking->tracking) {
        fprintf(output_file, " online track allowed |");
    } else {
        fprintf(output_file, " NO online tracking  |");
    }
    fprintf(output_file, "\n");
    fflush(output_file);
}

void terminal_message_8(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sHardwareInfo *_hwinfo) {

    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    terminal_mac_info(_rxtx, _integrity, _radiodata, _mac_data);

    // Device type
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    if (_hwinfo->device_type)
        fprintf(output_file, " T: %02X", _hwinfo->device_type);
    else
        fprintf(output_file, "   - ");
    if (strlen(_hwinfo->device_type_str) > 0)
        fprintf(output_file, " %s", _hwinfo->device_type_str);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // FW build date
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    if (_hwinfo->fw_build_year)
        fprintf(output_file, " %04d-%02d-%02d ", _hwinfo->fw_build_year, _hwinfo->fw_build_month, _hwinfo->fw_build_day);
    else
        fprintf(output_file, "  UNK ");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    fprintf(output_file, " add: %02X %02X ", _hwinfo->add1, _hwinfo->add2);
    fprintf(output_file, "|");

    if (_hwinfo->experimental)
        fprintf(output_file, " experimental ");
    else
        fprintf(output_file, " released     +");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    fprintf(output_file, "\n");
    fflush(output_file);
}

void terminal_message_9(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sThermalInfo *_thermalinfo) {

    terminal_rf_info(_rxtx, _integrity, _radiodata, _mac_data);
    terminal_mac_info(_rxtx, _integrity, _radiodata, _mac_data);

    // Latitude
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %+9.5f°", _thermalinfo->latitude);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Longitude
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    fprintf(output_file, " %+9.5f°", _thermalinfo->longitude);
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Confidence
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    if (_thermalinfo->confidence) fprintf(output_file, " %d ", _thermalinfo->confidence); else fprintf(output_file, "   - ");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Altitude
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    if (_thermalinfo->altitude) fprintf(output_file, " %d m", _thermalinfo->altitude); else fprintf(output_file, "   x m");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Avg Thermal Climb
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    if (_thermalinfo->avgClimb) fprintf(output_file, "Cl: %f m/s", _thermalinfo->avgClimb); else fprintf(output_file, " Cl:  - m/s");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Wind speed
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    if (_thermalinfo->avgWind) fprintf(output_file, " %5.1f m/s", _thermalinfo->avgWind); else fprintf(output_file, "   -.- m/s");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");

    // Wind heading
    if (_rxtx) fprintf(output_file, KCYN); else fprintf(output_file, KGRN);
    if (_radiodata->crc_err) fprintf(output_file, KYEL);
    if (_thermalinfo->avgHeading) fprintf(output_file, " %5.1f°", _thermalinfo->avgHeading); else fprintf(output_file, "   -.-°");
    fprintf(output_file, KNRM);
    fprintf(output_file, "|");
    fprintf(output_file, "\n");
    fflush(output_file);
}


//#endif
