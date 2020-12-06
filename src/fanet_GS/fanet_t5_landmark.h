/*
 * fanet_t5_landmark.h
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
 */

#ifndef FANET_T5_LANDMARK_H
#define FANET_T5_LANDMARK_H

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "fanet_mac.h"
#include "fanet_struct.h"
#include "fanet_global.h"
#include "fanet_radio.h"
#include "fanet_terminal.h"

void type_5_landmark_decoder(sRawMessage *_rx_message, sLandmark *_landmark_data);

void type_5_landmark_coder(sRawMessage *_tx_message, sLandmark *_landmark_data);

void type_5_landmark_receiver(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_rx_payload);

#endif // FANET_T5_LANDMARK_H

