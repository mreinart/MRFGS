/*
 * fanet_terminal.h
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

#ifndef FANET_TERMINAL_H
#define FANET_TERMINAL_H

#include "fanet_radio.h"
#include "fanet_struct.h"

void terminal_set_file(FILE *_output_file);

void terminal_start_screen(byte _sf, int _bandwith, int _freq);

void terminal_rf_info(boolean _rxtx, boolean _integrity, sRadioData *_radio_data, sFanetMAC *_mac_data);

void terminal_mac_info(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data);

void terminal_message_raw(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                          sRawMessage *_raw_message);

void terminal_message_crc_err(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data);

void terminal_message_mac_err(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data);

void terminal_message_0(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data, sACK *_ack);

void terminal_message_1(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sAirTracking *_tracking);

void terminal_message_2(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data, sName *_name);

void
terminal_message_3(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data, sMessage *_message);

void terminal_message_4(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sWeather *_weather_data);

void terminal_message_5(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sLandmark *_landmark_data);

void terminal_message_7(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sGroundTracking *_tracking);

void terminal_message_8(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sHardwareInfo *_hwinfo);

void terminal_message_9(boolean _rxtx, boolean _integrity, sRadioData *_radiodata, sFanetMAC *_mac_data,
                        sThermalInfo *_thermalinfo);

#endif
